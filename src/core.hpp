#pragma once
#include <string>
#include <vector>
#include <unordered_map> // или <map>

#if defined(_WIN32)
    #include <windows.h>

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef __linux__
    #include <X11/Xlib.h>
#endif


// 📌 Глобальные переменные — доступны во всех файлах, но создаются один раз
extern std::string baseDir;  // Глобальная переменная: путь к .exe
extern int windowWidth;
extern int windowHeight;

extern float focalLengthMM;

struct Animation {
    std::string name; // имя анимации

    // список параметров: вектор из пар ключ-значение
    std::vector<std::pair<std::string, float>> parameters;
};

struct Point3D {
    float x, y, z;       // координаты в см
    uint8_t r, g, b;     // Цвет
    float opacity;       // 0.0 — прозрачный, 1.0 — непрозрачный
    float lightIntensity;// 0.0 — тьма, 1.0 — ярко
};

struct Line {
    std::vector<Point3D> points; // Обычно 2 точки, но может быть и одна
};

struct Polygon3D {
    float roughness;
    float metallic;
    std::string lightTarget;
    std::string lightType;
    std::vector<Line> lines;
};

struct Model {
    std::string modelName;
    bool castShadow;
    std::vector<Polygon3D> polygons;
};

struct Camera {
    float x;  // Положение камеры по X
    float y;  // Положение камеры по Y
    float z;  // Положение камеры по Z (например, стоит позади центра сцены)

    float horizontalAngle; // Поворот влево-вправо (в радианах)
    float verticalAngle;   // Поворот вверх-вниз (в радианах)
};
extern Camera cam;  // 🔹 Объявляем, что переменная будет где-то создана

class App {
public:
    int dpi = 96;
    float scale = 1.0f;

    void loadCamera(const std::string& filename);
    void setDPI(int dpiValue); // функция для установки dpi
    Model loader(const std::string& path);   // 📦 Загрузка модели
    void draw3DPoint(HDC hdc, Point3D point);       // 🔹 Рисуем что-то
    void clear(HDC hdc);       // стереть всё
    void animate(Animation animation);

private:

};
extern App app;