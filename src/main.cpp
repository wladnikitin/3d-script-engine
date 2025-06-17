#include <iostream>
#include <filesystem>
//#include <thread>
#include "core.hpp"

#if defined(_WIN32)  // Если Windows
    #include <windows.h>

#elif defined(__linux__)  // Если Linux
    //#include <X11/Xlib.h>
    #error "This operating system is not supported yet."

#else
    #error "This operating system is not supported yet."
#endif

std::string baseDir;
std::string getExecutableDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path exePath(buffer);
    return exePath.parent_path().string();
}

App app;
int main() {
baseDir = getExecutableDir(); // = путь до папки Debug
std::filesystem::path base = std::filesystem::path(baseDir).parent_path().parent_path(); // ⬆⬆
std::filesystem::path cameraPath = base / "data" / "camera" / "target" / "camera.txt";
std::filesystem::path modelPath  = base / "data" / "3d" / "cube.json";

#if defined(_WIN32)
    // 🔹 Название класса окна
    const wchar_t CLASS_NAME[] = L"MyWinWindowClass";

    // 🔹 Структура, описывающая поведение окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;

    wc.hInstance = GetModuleHandle(NULL);  // 🔹 Получаем текущий процесс
    wc.lpszClassName = CLASS_NAME;         // 🔹 Назначаем имя класса
    RegisterClass(&wc);                    // 🔹 Регистрируем класс окна в системе

    // 🔹 Создаём само окно
    HWND hwnd = CreateWindowEx(
        0,                          // Расширенные стили
        CLASS_NAME,                 // Класс окна
        L"3d-script-engine",         // Заголовок окна
        WS_OVERLAPPEDWINDOW,        // Стиль (обычное окно)
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, // Положение и размер
        NULL, NULL, wc.hInstance, NULL // Родитель, меню и т.п.
    );

    ShowWindow(hwnd, SW_SHOW);     // 🔹 Показываем окно на экране

    HDC screen = GetDC(hwnd); // лучше брать dpi именно из окна, а не всего экрана
    int dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(hwnd, screen);

    //std::thread inputThread(&App:: , &app);

    app.scale = dpi / 2.54f * focalLengthMM / 10.0f; // считается один раз в начале программы
    app.loadCamera(cameraPath.string());
    std::cout << "Camera: (" << cam.x << ", " << cam.y << ", " << cam.z << "), horizontal angle: "<< cam.horizontalAngle << ", vertical angle: "<< cam.verticalAngle << "\n";
   Model model = app.loader(modelPath.string());
    HDC hdc = GetDC(hwnd);         // 🔹 Получаем доступ к окну
    for (const auto& polygon : model.polygons) {
        for (const auto& line : polygon.lines) {
            for (const auto& point : line.points) {
                app.draw3DPoint(hdc, point);       // 🔹 Рисуем что-то
                std::cout << "Point: (" << point.x << ", " << point.y << ", " << point.z << ")\n";
            }
        }
    }
    ReleaseDC(hwnd, hdc);          // 🔹 Отпускаем, освобождаем ресурс

    // 🔁 Запускаем цикл, который реагирует на события И выполняет свою логику
    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // здесь бесконечно что-то выполняется
        }
    }
    //inputThread.join();      // 🧵 Дождись завершения waitForLine()


#elif defined(__linux__)
    // 🔹 Открываем соединение с X-сервером
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "Failed to connect to X server\n";
        return 1;
    }

    // 🔹 Получаем номер экрана и корневое окно
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // 🔹 Создаём простое окно
    Window window = XCreateSimpleWindow(
        display,     // Соединение с X11
        root,        // Родительское окно (корневое)
        100, 100,    // Координаты появления окна
        640, 480,    // Размеры окна
        1,           // Толщина рамки
        BlackPixel(display, screen), // Цвет рамки
        WhitePixel(display, screen)  // Цвет фона
    );

    // 🔹 Отправляем событие: хотим закрытие окна
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &delWindow, 1);

    // 🔹 Показываем окно
    XMapWindow(display, window);
    XFlush(display);  // Обновляем экран


    std::thread inputThread(&App::waitForLine, &app);
    app.setLinuxDisplay(display, window)
    app.initGraphics();        // ✅ создать контекст рисования один раз
    // 🔹 Обрабатываем события
    app.loader("../data/3d/cube.json");
    while (true) {
        // 🔁 Обрабатываем события (если есть)
        while (XPending(display)) {
            XEvent e;
            XNextEvent(display, &e);
            if (e.type == ClientMessage && (Atom)e.xclient.data.l[0] == delWindow)
                goto cleanup;
        }
        app.update();
    }

    // 🔹 Закрываем и очищаем ресурсы
    app.cleanup();             // 🔒 освободить контекст перед выходом
    XDestroyWindow(display, window);
    XCloseDisplay(display);
#endif

    return 0;  // Завершение программы
}
