// core.cpp
#include "core.hpp"
#include "json.hpp"

#include <unordered_map> // или <map>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#if defined(_WIN32)  // Если Windows
    #include <windows.h>

#else defined(__linux__)  // Если Linux
    #include <X11/Xlib.h>

#endif

using json = nlohmann::json;
using namespace std::chrono;

// глобальная переменная:
steady_clock::time_point lastFrameTime = steady_clock::now();

#define DEG2RAD(angleDegrees) ((angleDegrees) * 3.14159265f / 180.0f)

// ⚠️ Здесь создаются глобальные переменные
int windowWidth = 640;
int windowHeight = 480;

float focalLengthMM = 50.0f; // 👁️ Человеческий глаз

Camera cam; // 🔹 Здесь мы создаём саму переменную
Model model;;

bool isFullscreen = false;
WINDOWPLACEMENT windowPosBeforeFullscreen = { sizeof(windowPosBeforeFullscreen) };
DWORD windowStyleBeforeFullscreen = 0;


// 🎯 Функция для рисования 3D-точки
void App::draw3DPoint(HDC hdc, Point3D point) {
    float dx = point.x - cam.x;
    float dy = point.y - cam.y;
    float dz = point.z - cam.z;

    // 📌 Поворот по горизонтали (вокруг вертикальной оси)
    float x1 = dx * cos(DEG2RAD(-cam.horizontalAngle)) - dz * sin(DEG2RAD(-cam.horizontalAngle));
    float z1 = dx * sin(-cam.horizontalAngle) + dz * cos(DEG2RAD(-cam.horizontalAngle));

    // 📌 Поворот по вертикали (вокруг горизонтальной оси)
    float y1 = dy * cos(DEG2RAD(-cam.verticalAngle)) - z1 * sin(DEG2RAD(-cam.verticalAngle));
    float z2 = dy * sin(DEG2RAD(-cam.verticalAngle)) + z1 * cos(DEG2RAD(-cam.verticalAngle)); // глубина

    // ⚠️ Защита от деления на ноль или "отрицательной глубины"
    if (z2 <= 0.01f) return;

    // 📐 Проекция на экран (в логических координатах)
    float screenX = x1 / z2;
    float screenY = y1 / z2;

    // 🎯 Перевод в пиксели
    int pixelX = static_cast<int>(screenX * app.scale + windowWidth / 2);
    int pixelY = static_cast<int>(-screenY * app.scale + windowHeight / 2);

    // 🖌️ Рисуем пиксель
    SetPixel(hdc, pixelX, pixelY, RGB(point.r, point.g, point.b));
    SetPixel(hdc, pixelX+1, pixelY, RGB(point.r, point.g, point.b));
    SetPixel(hdc, pixelX, pixelY+1, RGB(point.r, point.g, point.b));
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            app.clear(hdc);

            for (const auto& polygon : model.polygons) {
                for (const auto& line : polygon.lines) {
                    for (const auto& point : line.points) {
                        app.draw3DPoint(hdc, point);
                        std::cout << "Point: (" << point.x << ", " << point.y << ", " << point.z << ")\n";
                    }
                }
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0); // Завершаем программу
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_F11) {
                if (!isFullscreen) {
                    // Сохраняем стиль и позицию
                    windowStyleBeforeFullscreen = GetWindowLong(hwnd, GWL_STYLE);
                    GetWindowPlacement(hwnd, &windowPosBeforeFullscreen);

                    // Убираем рамки
                    SetWindowLong(hwnd, GWL_STYLE, windowStyleBeforeFullscreen & ~WS_OVERLAPPEDWINDOW);

                    // Получаем размеры всего экрана
                    MONITORINFO mi = { sizeof(mi) };
                    if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
                        SetWindowPos(hwnd, HWND_TOP,
                            mi.rcMonitor.left, mi.rcMonitor.top,
                            mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom - mi.rcMonitor.top,
                            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                    }

                    isFullscreen = true;
                } else {
                    // Возврат в оконный режим
                    SetWindowLong(hwnd, GWL_STYLE, windowStyleBeforeFullscreen);
                    SetWindowPlacement(hwnd, &windowPosBeforeFullscreen);
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    isFullscreen = false;
                }
            }
            break;
        
        case WM_SIZE:
            windowWidth  = LOWORD(lParam);
            windowHeight = HIWORD(lParam);
            break;

        case WM_DPICHANGED: {
            // lParam — это указатель на RECT с новой рекомендованной областью окна
            RECT* suggestedRect = (RECT*)lParam;

            // Можешь переместить окно в рекомендуемую область
            SetWindowPos(hwnd,
                NULL,
                suggestedRect->left,
                suggestedRect->top,
                suggestedRect->right - suggestedRect->left,
                suggestedRect->bottom - suggestedRect->top,
                SWP_NOZORDER | SWP_NOACTIVATE);

            // Получи новый DPI
            int dpiX = LOWORD(wParam);
            int dpiY = HIWORD(wParam);

            // Обнови масштаб (если хочешь)
            int actualDPI = dpiX; // Обычно dpiX = dpiY, берём любой
            app.setDPI(actualDPI);

            break;
        }

    }

    return DefWindowProc(hwnd, msg, wParam, lParam); // Остальные сообщения — Windows сама
}

void App::setDPI(int dpiValue) {
    dpi = dpiValue;
    scale = dpi / 2.54f * focalLengthMM / 10.0f;
}

// ───── Загрузчик модели ─────

Model App::loader(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open file: " << path << '\n';
        return Model{}; // ← Вернёт "пустую" модель
    }

    json j;
    file >> j;

    Model model;
    model.modelName = j["modelName"];
    model.castShadow = j["castShadow"];

    for (const auto& jPoly : j["polygons"]) {
        Polygon3D poly;
        poly.roughness = jPoly["roughness"];
        poly.metallic = jPoly["metallic"];
        poly.lightTarget = jPoly["lightTarget"];
        poly.lightType = jPoly["lightType"];

        for (const auto& jLine : jPoly["lines"]) {
            Line line;

            for (const auto& jPoint : jLine["points"]) {
                Point3D p;
                p.x = jPoint["x"];
                p.y = jPoint["y"];
                p.z = jPoint["z"];
                p.r = jPoint["r"];
                p.g = jPoint["g"];
                p.b = jPoint["b"];
                p.opacity = jPoint["opacity"];
                p.lightIntensity = jPoint["lightIntensity"];
                line.points.push_back(p);
            }

            poly.lines.push_back(line);
        }

        model.polygons.push_back(poly);
    }

    // ───── Отладочный вывод ─────
    std::cout << "Model loaded: " << model.modelName << "\n";
    std::cout << "Shadows: " << (model.castShadow ? "enabled" : "disabled") << "\n";
    std::cout << "Polygon count: " << model.polygons.size() << "\n";

    for (size_t i = 0; i < model.polygons.size(); ++i) {
        const auto& p = model.polygons[i];
        std::cout << "  Polygon " << i << ": lines = " << p.lines.size()
                << ", roughness = " << p.roughness
                << ", metallic = " << p.metallic
                << ", lightType = " << p.lightType << "\n";
    }
    return model;
}

void App::clear(HDC hdc) {
    //HDC hdc = GetDC(hwnd);
    RECT rect = { 0, 0, windowWidth, windowHeight };

    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rect, blackBrush);
    DeleteObject(blackBrush);

    //ReleaseDC(hwnd, hdc);
}

void App::animate(Animation animation) {
    // вычисляем прошедшее время
    steady_clock::time_point now = steady_clock::now();
    float deltaTime = duration<float>(now - lastFrameTime).count();
    lastFrameTime = now;

    // 🎯 Пример: движение объекта
    //object.x += objectSpeed * deltaTime;

    // остальной код...
}

void App::loadCamera(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    std::string line;
    std::getline(file, line); // читаем строку из файла

    std::replace(line.begin(), line.end(), ';', ' '); // заменим ; на пробелы

    std::istringstream iss(line);
    std::string temp;

    while (iss >> temp) {
        if (temp == "x-coordinate:")      iss >> cam.x;
        else if (temp == "y-coordinate:") iss >> cam.y;
        else if (temp == "z-coordinate:") iss >> cam.z;
        else if (temp == "horizontal") {
            iss >> temp; // "angle:"
            iss >> cam.horizontalAngle;
        }
        else if (temp == "vertical") {
            iss >> temp; // "angle:"
            iss >> cam.verticalAngle;
        }
    }
}