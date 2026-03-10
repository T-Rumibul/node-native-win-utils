// patrially used code from https://github.com/octalmage/robotjs witch is under MIT License Copyright (c) 2014 Jason Stallings
#include <napi.h>
#include <windows.h>
#include <atomic>
#include <cmath>

HHOOK mouseHook;
Napi::ThreadSafeFunction tsfn;
static HANDLE hHookThread = NULL;
static DWORD hookThreadId = 0;
static std::atomic_flag listenerRunning = ATOMIC_FLAG_INIT;
static std::atomic<bool> listenerStopping = false;
static HANDLE hHookReady = NULL;

struct Position
{
    int x;
    int y;
};

Position calcAbsolutePosition(int x, int y)
{
    int vLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vWidth <= 1 || vHeight <= 1)
        return {0, 0}; // No virtual screen
    int absoluteX = static_cast<int>((static_cast<long long>(x - vLeft) * 65535) / (vWidth - 1));
    int absoluteY = static_cast<int>((static_cast<long long>(y - vTop) * 65535) / (vHeight - 1));
    return {absoluteX, absoluteY};
}

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

    Position pos = calcAbsolutePosition(posX, posY);
    int absoluteX = pos.x;
    int absoluteY = pos.y;

    // Move the mouse
    INPUT mouseInput = {0};
    mouseInput.type = INPUT_MOUSE;
    mouseInput.mi.dx = absoluteX;
    mouseInput.mi.dy = absoluteY;
    mouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
    mouseInput.mi.time = 0; // System will provide the timestamp
    UINT sent = SendInput(1, &mouseInput, sizeof(INPUT));
    if (sent == 0)
        return Napi::Boolean::New(env, false);
    return Napi::Boolean::New(env, true);
}

Napi::Value ClickMouse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::string button = "left";
    std::string type = "click";
    if (info.Length() >= 1 && info[0].IsString())
        button = info[0].As<Napi::String>();
    if(info.Length() >= 2 && info[1].IsString())
        type = info[1].As<Napi::String>();
        

    WORD downFlag = 0, upFlag = 0;

    if (button == "left")
    {
        downFlag = MOUSEEVENTF_LEFTDOWN;
        upFlag = MOUSEEVENTF_LEFTUP;
    }
    else if (button == "right")
    {
        downFlag = MOUSEEVENTF_RIGHTDOWN;
        upFlag = MOUSEEVENTF_RIGHTUP;
    }
    else if (button == "middle")
    {
        downFlag = MOUSEEVENTF_MIDDLEDOWN;
        upFlag = MOUSEEVENTF_MIDDLEUP;
    }
    else
    {
        Napi::TypeError::New(env, "Invalid button name").ThrowAsJavaScriptException();
        return env.Null();
    }
    UINT sent = 0;
    INPUT inputsClick[2] = {};
    inputsClick[0].type = INPUT_MOUSE;
    inputsClick[0].mi.dwFlags = downFlag;
    inputsClick[1].type = INPUT_MOUSE;
    inputsClick[1].mi.dwFlags = upFlag;
    inputsClick[1].mi.time = 0;
    inputsClick[0].mi.time = 0;
    INPUT singleInput[1] = {};
    singleInput[0].type = INPUT_MOUSE;
    WORD flag = (type == "down") ? downFlag : upFlag;
    singleInput[0].mi.dwFlags = flag;

    if(type == "click")
        sent = SendInput(2, inputsClick, sizeof(INPUT));
    else
        sent = SendInput(1, singleInput, sizeof(INPUT));
    if (sent == 0)
        return Napi::Boolean::New(env, false);
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
    bool success = true;
    int startX = info[0].As<Napi::Number>();
    int startY = info[1].As<Napi::Number>();
    int endX = info[2].As<Napi::Number>();
    int endY = info[3].As<Napi::Number>();
    int speed = 100;
    if (info.Length() > 4 && info[4].IsNumber())
    {
        speed = info[4].As<Napi::Number>();
    }
    // Use pixel coords for timing
    double pixelDistX = endX - startX;
    double pixelDistY = endY - startY;
    double pixelDistance = sqrt(pixelDistX * pixelDistX + pixelDistY * pixelDistY);
    if (speed <= 0)
        speed = 1; // guard div/0
    double duration = pixelDistance / speed;
    // Get the screen metrics

    Position startPos = calcAbsolutePosition(startX, startY);
    Position endPos = calcAbsolutePosition(endX, endY);
    // Convert coordinates to absolute values
    int absoluteStartX = startPos.x;
    int absoluteStartY = startPos.y;
    int absoluteEndX = endPos.x;
    int absoluteEndY = endPos.y;

    // Calculate the distance and duration based on speed
    double distanceX = absoluteEndX - absoluteStartX;
    double distanceY = absoluteEndY - absoluteStartY;
    // Move the mouse to the starting position
    INPUT startMouseInput = {0};
    startMouseInput.type = INPUT_MOUSE;
    startMouseInput.mi.dx = absoluteStartX;
    startMouseInput.mi.dy = absoluteStartY;
    startMouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
    startMouseInput.mi.time = 0; // System will provide the timestamp

    if (SendInput(1, &startMouseInput, sizeof(INPUT)) == 0)
        return Napi::Boolean::New(env, false);
    ;

    // Perform mouse button down event
    INPUT mouseDownInput = {0};
    mouseDownInput.type = INPUT_MOUSE;
    mouseDownInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    mouseDownInput.mi.time = 0; // System will provide the timestamp

    if (SendInput(1, &mouseDownInput, sizeof(INPUT)) == 0)
        return Napi::Boolean::New(env, false);
    ;

    // Calculate the number of steps based on the duration and desired speed
    const int steps = 100; // Adjust the number of steps for smoother movement

    // Calculate the incremental values for each step
    double stepX = distanceX / steps;
    double stepY = distanceY / steps;

    // Move the mouse in increments to simulate dragging with speed control
    for (int i = 0; i <= steps; ++i)
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

        if (SendInput(1, &mouseMoveInput, sizeof(INPUT)) == 0)
            return Napi::Boolean::New(env, false);
        ;

        // Sleep for a short duration to control the speed
        if (i < steps)
        {
            Sleep(static_cast<DWORD>(duration / steps));
        }
    }

    // Perform mouse button up event
    INPUT mouseUpInput = {0};
    mouseUpInput.type = INPUT_MOUSE;
    mouseUpInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    mouseUpInput.mi.time = 0; // System will provide the timestamp

    if (SendInput(1, &mouseUpInput, sizeof(INPUT)) == 0)
        return Napi::Boolean::New(env, false);
    return Napi::Boolean::New(env, success);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && !listenerStopping.load())
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

        napi_status status = tsfn.NonBlockingCall(callback);
        if (status != napi_ok)
        {
            // tsfn is already closing
            return CallNextHookEx(mouseHook, nCode, wParam, lParam);
        }
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

DWORD WINAPI HookThread(LPVOID)
{
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    SetEvent(hHookReady);
    if (mouseHook == NULL)
        return 1;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouseHook);
    mouseHook = NULL;
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
    // Bug fix: prevent double-start leaking a thread and hook
    if (listenerRunning.test_and_set())
    {
        Napi::TypeError::New(env, "Mouse listener already running").ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Function callback = info[0].As<Napi::Function>();

    tsfn = Napi::ThreadSafeFunction::New(
        env,
        callback,
        "MouseListener",
        0,
        1);

    // Store handle and thread ID so StopMouseListener can signal and wait
    hHookReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hHookReady == NULL)
    {
        tsfn.Release();
        listenerRunning.clear();
        Napi::Error::New(env, "Failed to create sync event").ThrowAsJavaScriptException();
        return env.Null();
    }
    hHookThread = CreateThread(NULL, 0, HookThread, NULL, 0, &hookThreadId);
    if (hHookThread == NULL)
    {
        tsfn.Release();
        CloseHandle(hHookReady);
        hHookReady = NULL;
        listenerRunning.clear(); // release the guard so caller can retry
        Napi::Error::New(env, "Failed to create hook thread").ThrowAsJavaScriptException();
        return env.Null();
    }
    WaitForSingleObject(hHookReady, 2000);
    CloseHandle(hHookReady);
    hHookReady = NULL;
    DWORD exitCode = 0;
    GetExitCodeThread(hHookThread, &exitCode);
    if (exitCode == 1)
    {
        WaitForSingleObject(hHookThread, 1000);
        CloseHandle(hHookThread);
        hHookThread = NULL;
        hookThreadId = 0;
        tsfn.Release();
        listenerRunning.clear();
        Napi::Error::New(env, "Failed to install mouse hook").ThrowAsJavaScriptException();
        return env.Null();
    }
    return Napi::Boolean::New(env, true);
}

Napi::Value StopMouseListener(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    if (hHookThread == NULL)
    {
        result.Set("success", false);
        result.Set("error", "No listener is running");
        return result;
    }

    listenerStopping.store(true);
    // PostThreadMessage with WM_QUIT breaks the GetMessage loop in HookThread,
    // which then runs UnhookWindowsHookEx and exits
    if (!PostThreadMessage(hookThreadId, WM_QUIT, 0, 0))
    {
        listenerStopping.store(false);
        listenerRunning.clear();
        result.Set("success", false);
        result.Set("error", "Failed to signal hook thread");
        result.Set("errorCode", Napi::Number::New(env, GetLastError()));
        return result;
    }

    // Wait for the thread to fully exit before releasing resources
    DWORD waitResult = WaitForSingleObject(hHookThread, 3000);
    if (waitResult != WAIT_OBJECT_0)
    {
        // Thread didn't exit in time - kill it
        TerminateThread(hHookThread, 1);
        // Best-effort: reduce chance a concurrent NonBlockingCall is still
        // in-flight before tsfn.Release() below. Not a hard guarantee.
        Sleep(50);
        result.Set("success", false);
        result.Set("error", "Hook thread did not exit cleanly, was forcefully terminated");
    }
    else
    {
        result.Set("success", true);
    }

    // Release the TSFN so Node.js can garbage collect the JS callback
    tsfn.Release();

    CloseHandle(hHookThread);
    hHookThread = NULL;
    hookThreadId = 0;
    listenerRunning.clear();
    listenerStopping.store(false);
    return result;
}