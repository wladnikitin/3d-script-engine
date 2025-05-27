#pragma once
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

#if defined(_WIN32)
    #include <windows.h> // 👈 ВОТ СЮДА
#endif

#ifdef __linux__
    #include <X11/Xlib.h>
#endif

struct Point2D {
    int x, y;
};

struct Point3D {
    float x, y, z;
};

struct Model {
    std::vector<Point3D> points;
    float rx = 0, ry = 0, rz = 0; // углы поворота
};

class App {
public:
    std::atomic<bool> running = true;

    void update();                          // 🔁 Фоновая отрисовка
    void waitForLine();                     // ⌨️ Ожидает команду
    void loader(const std::string& path);   // 📦 Загрузка модели

    Point2D print3Dto2D(const Point3D& p, float rx, float ry, float rz, int w, int h);
    void drawPixelCrossPlatform(int x, int y);

    // ⬇️ Новое: инициализация и очистка графического контекста
    void initGraphics();                    // 🎨 Один раз создаёт графику
    void cleanup();                         // ❌ Освобождает ресурсы

#ifdef _WIN32
    void setHWND(HWND h);                   // Windows: передаём дескриптор окна
#endif

#ifdef __linux__
    void setLinuxDisplay(Display* d, Window w); // Linux: дисплей и окно
#endif

private:
    std::map<std::string, Model> models;    // 💾 все модели
    std::string lastModelToRender;          // имя модели для отрисовки

    std::mutex commandMutex;
    std::string pendingCommand;
    bool hasNewCommand = false;

#ifdef _WIN32
    HWND hwnd = nullptr;
    HDC hdc = nullptr;                      // 🎨 Контекст рисования (Windows)
#endif

#ifdef __linux__
    Display* display = nullptr;
    Window window;
    GC gc = 0;                              // 🎨 Графический контекст (Linux)
#endif
};