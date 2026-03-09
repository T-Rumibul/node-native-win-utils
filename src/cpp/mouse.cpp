// patrially used code from https://github.com/octalmage/robotjs witch is under MIT License Copyright (c) 2014 Jason Stallings
#include <napi.h>
#include <windows.h>

HHOOK mouseHook;
Napi::ThreadSafeFunction tsfn;
/**
 * Move the mouse to a specific point.
 * @param point The coordinates to move the mouse to (x, y).
 */

Napi::Value MoveMouse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber())
    {
        Napi::TypeError::New(env, "You should provide x and y position of type number").ThrowAsJavaScriptException();
        return env.Null();
    }

    int posX = info[0].As<Napi::Number>();
    int posY = info[1].As<Napi::Number>();

    // Get the screen metrics
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Convert coordinates to absolute values
    int absoluteX = static_cast<int>((65536 * posX) / screenWidth);
    int absoluteY = static_cast<int>((65536 * posY) / screenHeight);

    // Move the mouse
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    mouseInput.mi.dx = absoluteX;
    mouseInput.mi.dy = absoluteY;
    mouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
    mouseInput.mi.time = 0; // System will provide the timestamp

    SendInput(1, &mouseInput, sizeof(mouseInput));

    return Napi::Boolean::New(env, true);
}

Napi::Value ClickMouse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::string button;

    if (info.Length() < 1 || !info[0].IsString())
        button = "left";
    else
        button = info[0].As<Napi::String>();

    WORD mouseEvent = 0;

    if (button == "left")
    {
        mouseEvent = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
    }
    else if (button == "right")
    {
        mouseEvent = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
    }
    else if (button == "middle")
    {
        mouseEvent = MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP;
    }
    else
    {
        Napi::TypeError::New(env, "Invalid button name").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Perform the mouse click
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    mouseInput.mi.dwFlags = mouseEvent;
    mouseInput.mi.time = 0; // System will provide the timestamp

    SendInput(1, &mouseInput, sizeof(mouseInput));

    return Napi::Boolean::New(env, true);
}

Napi::Value DragMouse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 4 || !info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsNumber())
    {
        Napi::TypeError::New(env, "You should provide startX, startY, endX, endY").ThrowAsJavaScriptException();
        return env.Null();
    }

    int startX = info[0].As<Napi::Number>();
    int startY = info[1].As<Napi::Number>();
    int endX = info[2].As<Napi::Number>();
    int endY = info[3].As<Napi::Number>();
    int speed = 100;
    if (info.Length() > 4 && info[4].IsNumber())
    {
        speed = info[4].As<Napi::Number>();
    }

    // Get the screen metrics
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Convert coordinates to absolute values
    int absoluteStartX = static_cast<int>((65536 * startX) / screenWidth);
    int absoluteStartY = static_cast<int>((65536 * startY) / screenHeight);
    int absoluteEndX = static_cast<int>((65536 * endX) / screenWidth);
    int absoluteEndY = static_cast<int>((65536 * endY) / screenHeight);

    // Calculate the distance and duration based on speed
    double distanceX = absoluteEndX - absoluteStartX;
    double distanceY = absoluteEndY - absoluteStartY;
    double distance = sqrt(distanceX * distanceX + distanceY * distanceY);
    double duration = distance / speed;

    // Move the mouse to the starting position
    INPUT startMouseInput = {0};
    startMouseInput.type = INPUT_MOUSE;
    startMouseInput.mi.dx = absoluteStartX;
    startMouseInput.mi.dy = absoluteStartY;
    startMouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
    startMouseInput.mi.time = 0; // System will provide the timestamp

    SendInput(1, &startMouseInput, sizeof(startMouseInput));

    // Perform mouse button down event
    INPUT mouseDownInput = {0};
    mouseDownInput.type = INPUT_MOUSE;
    mouseDownInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    mouseDownInput.mi.time = 0; // System will provide the timestamp

    SendInput(1, &mouseDownInput, sizeof(mouseDownInput));

    // Calculate the number of steps based on the duration and desired speed
    const int steps = 100; // Adjust the number of steps for smoother movement

    // Calculate the incremental values for each step
    double stepX = distanceX / steps;
    double stepY = distanceY / steps;

    // Move the mouse in increments to simulate dragging with speed control
    for (int i = 0; i < steps; ++i)
    {
        // Calculate the position for the current step
        int currentX = static_cast<int>(absoluteStartX + (stepX * i));
        int currentY = static_cast<int>(absoluteStartY + (stepY * i));

        // Move the mouse to the current position
        INPUT mouseMoveInput = {0};
        mouseMoveInput.type = INPUT_MOUSE;
        mouseMoveInput.mi.dx = currentX;
        mouseMoveInput.mi.dy = currentY;
        mouseMoveInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
        mouseMoveInput.mi.time = 0; // System will provide the timestamp

        SendInput(1, &mouseMoveInput, sizeof(mouseMoveInput));

        // Sleep for a short duration to control the speed
        Sleep(static_cast<DWORD>(duration / steps));
    }

    // Perform mouse button up event
    INPUT mouseUpInput = {0};
    mouseUpInput.type = INPUT_MOUSE;
    mouseUpInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    mouseUpInput.mi.time = 0; // System will provide the timestamp

    SendInput(1, &mouseUpInput, sizeof(mouseUpInput));

    return Napi::Boolean::New(env, true);
}


LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT *mouse = (MSLLHOOKSTRUCT *)lParam;

        int x = mouse->pt.x;
        int y = mouse->pt.y;

        std::string type = "move";

        switch (wParam)
        {
        case WM_LBUTTONDOWN:
            type = "leftDown";
            break;
        case WM_LBUTTONUP:
            type = "leftUp";
            break;
        case WM_RBUTTONDOWN:
            type = "rightDown";
            break;
        case WM_RBUTTONUP:
            type = "rightUp";
            break;
        case WM_MBUTTONDOWN:
            type = "middleDown";
            break;
        case WM_MBUTTONUP:
            type = "middleUp";
            break;
        case WM_MOUSEMOVE:
            type = "move";
            break;
        }

        auto callback = [x, y, type](Napi::Env env, Napi::Function jsCallback)
        {
            Napi::Object event = Napi::Object::New(env);
            event.Set("x", x);
            event.Set("y", y);
            event.Set("type", type);

            jsCallback.Call({event});
        };

        tsfn.BlockingCall(callback);
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

DWORD WINAPI HookThread(LPVOID)
{
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouseHook);
    return 0;
}

Napi::Value StartMouseListener(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Callback expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Function callback = info[0].As<Napi::Function>();

    tsfn = Napi::ThreadSafeFunction::New(
        env,
        callback,
        "MouseListener",
        0,
        1);

    CreateThread(NULL, 0, HookThread, NULL, 0, NULL);

    return Napi::Boolean::New(env, true);
}
