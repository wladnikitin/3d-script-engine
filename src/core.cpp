// core.cpp
#include "core.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "json.hpp"
#include <thread>
#include <chrono>
#if defined(_WIN32)  // Если Windows
    #include <windows.h>

#else defined(__linux__)  // Если Linux
    #include <X11/Xlib.h>

#endif

using json = nlohmann::json;

std::mutex coutMutex;

void safePrint(const std::string& message) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << message << std::endl;
}

// 🔁 Повороты по осям
Point3D rotateX(Point3D p, float t) {
    float c = cos(t), s = sin(t);
    return { p.x, p.y * c - p.z * s, p.y * s + p.z * c };
}
Point3D rotateY(Point3D p, float t) {
    float c = cos(t), s = sin(t);
    return { p.x * c + p.z * s, p.y, -p.x * s + p.z * c };
}
Point3D rotateZ(Point3D p, float t) {
    float c = cos(t), s = sin(t);
    return { p.x * c - p.y * s, p.x * s + p.y * c, p.z };
}

// ⌨️ Слушает команды
void App::waitForLine() {
    while (running) {
        std::string line;
        std::cout << "> ";
        std::getline(std::cin, line);

        std::lock_guard<std::mutex> lock(commandMutex);
        pendingCommand = line;
        hasNewCommand = true;
    }
}

// 🖼️ Рисует модель при получении команды
void App::update() {
    bool shouldDraw = false;
    std::string modelToDraw;
    float rx = 0, ry = 0, rz = 0;

    {
        std::lock_guard<std::mutex> lock(commandMutex);
        if (hasNewCommand) {
            std::istringstream iss(pendingCommand);
            std::string cmd, name;
            iss >> cmd >> name >> rx >> ry >> rz;

            if (cmd == "rotate" && models.count(name)) {
                models[name].rx += rx;
                models[name].ry += ry;
                models[name].rz += rz;
                lastModelToRender = name;
                shouldDraw = true;
                safePrint("Rotated the model: " + name);
            } else {
                safePrint("Console command or model not found");
            }
            hasNewCommand = false;
        }
    }

    if (shouldDraw && models.count(lastModelToRender)) {
        const auto& model = models[lastModelToRender];
        safePrint("We are drawing: " + lastModelToRender + " (" + std::to_string(model.points.size()) + " points) now");

        for (const Point3D& p : model.points) {
            Point2D screen = print3Dto2D(p, model.rx, model.ry, model.rz, 640, 480);
            drawPixelCrossPlatform(screen.x, screen.y);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

// 🔁 Перевод 3D → 2D
Point2D App::print3Dto2D(const Point3D& p, float rx, float ry, float rz, int w, int h) {
    Point3D r = rotateX(rotateY(rotateZ(p, rz), ry), rx);
    float aspect = (float)w / h;
    float x_ndc = (r.x / r.z) / aspect;
    float y_ndc = (r.y / r.z);
    return {
        (int)((x_ndc + 1.0f) * 0.5f * w),
        (int)((1.0f - y_ndc) * 0.5f * h)
    };
}

// 🧱 Загрузка модели из JSON
void App::loader(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    json j;
    file >> j;

    Model m;
    std::string name = j.value("name", "Unnamed");
    for (const auto& pt : j["points"]) {
        m.points.push_back({
            pt.value("x", 0.0f),
            pt.value("y", 0.0f),
            pt.value("z", 0.0f)
        });
    }

    models[name] = m;
    safePrint("Model loaded: " + name);
}

#ifdef _WIN32
void App::setHWND(HWND h) {
    hwnd = h;
}
void App::initGraphics() {
    if (hwnd && !hdc)
        hdc = GetDC(hwnd);
}
void App::cleanup() {
    if (hdc && hwnd) {
        ReleaseDC(hwnd, hdc);
        hdc = nullptr;
    }
}
#endif

#ifdef __linux__
void App::setLinuxDisplay(Display* d, Window w) {
    display = d;
    window = w;
}
void App::initGraphics() {
    if (display && window && !gc)
        gc = XCreateGC(display, window, 0, 0);
}
void App::cleanup() {
    if (display && gc) {
        XFreeGC(display, gc);
        gc = nullptr;
    }
}
#endif

void App::drawPixelCrossPlatform(int x, int y) {
#ifdef _WIN32
    if (hdc) SetPixel(hdc, x, y, RGB(255, 0, 0));
#elif defined(__linux__)
    if (display && gc) {
        XSetForeground(display, gc, 0xFF0000);
        XDrawPoint(display, window, gc, x, y);
        XFlush(display);
    }
#endif
}