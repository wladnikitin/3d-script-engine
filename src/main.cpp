#include <iostream>
#include <thread>
#include "core.hpp"

#if defined(_WIN32)  // Если Windows
    #include <windows.h>

#elif defined(__linux__)  // Если Linux
    #include <X11/Xlib.h>

#else
    #error "This operating system is not supported yet."
#endif

int main() {
    App app;
#if defined(_WIN32)
    // 🔹 Название класса окна
    const wchar_t CLASS_NAME[] = L"MyWinWindowClass";

    // 🔹 Структура, описывающая поведение окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (msg == WM_DESTROY) {           // Если окно закрывается
            PostQuitMessage(0);            // Посылаем сообщение: "завершить программу"
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam); // Обработка прочих сообщений
    };
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

    std::thread inputThread(&App::waitForLine, &app);
    app.setHWND(hwnd);         // или app.setLinuxDisplay(...)
    app.initGraphics();        // ✅ создать контекст рисования один раз
    // 🔁 Запускаем цикл, который реагирует на события И выполняет свою логику
    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            app.update();
        }
    }
    app.cleanup();             // 🔒 освободить контекст перед выходом
    app.running = false;     // ⛔ Скажи: хватит слушать команды
    inputThread.join();      // 🧵 Дождись завершения waitForLine()


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
