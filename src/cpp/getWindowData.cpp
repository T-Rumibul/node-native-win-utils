#include <napi.h>
#include <Windows.h>
#include <iostream>
#include <dwmapi.h>
#include <helpers.h>

Napi::Value GetWindowData(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "Window name must be provided as a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string windowName = info[0].As<Napi::String>().Utf8Value();

    HWND windowHandle = GetWindowByName(windowName.c_str());

    if (windowHandle == NULL)
    {
        Napi::Error::New(env, "Window not found").ThrowAsJavaScriptException();
        return env.Null();
    }

    RECT windowRect;
    DwmGetWindowAttribute(windowHandle, DWMWA_EXTENDED_FRAME_BOUNDS, &windowRect, sizeof(windowRect));
    HDC hdc = GetDC(windowHandle);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(windowHandle, hdc);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;
    int x = windowRect.left;
    int y = windowRect.top;

    Napi::Object result = Napi::Object::New(env);
    result.Set("width", Napi::Number::New(env, width));
    result.Set("height", Napi::Number::New(env, height));
    result.Set("x", Napi::Number::New(env, x));
    result.Set("y", Napi::Number::New(env, y));

    return result;
}

Napi::Value BringWindowToFront(const Napi::CallbackInfo &info)
{   
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    auto returnError = [&](const std::string& message, DWORD errorCode = 0) {
        result.Set("success", false);
        result.Set("error", message);
        if (errorCode != 0) {
            result.Set("errorCode", Napi::Number::New(env, errorCode));
        }
        return result;
    };

    if (info.Length() < 1 || !info[0].IsString())
    {
        return returnError("Window name must be provided as a string");
    }

    std::string windowName = info[0].As<Napi::String>().Utf8Value();

    HWND hWnd = GetWindowByName(windowName.c_str());
    if (!hWnd || !IsWindow(hWnd))
    {
        return returnError("Window not found or invalid: " + windowName);
    }

    // Restore if minimized
    if (IsIconic(hWnd))
    {
        ShowWindow(hWnd, SW_RESTORE);
        // Give the window a moment to restore before trying to focus it
        Sleep(50);
    }

    HWND fg = GetForegroundWindow();

    // Bug fix 1: must attach the TARGET window's thread, not just the foreground
    // window's thread. We need a triangle: cur <-> fg thread <-> target thread.
    DWORD fgThread  = fg ? GetWindowThreadProcessId(fg, nullptr) : 0;
    DWORD tgtThread = GetWindowThreadProcessId(hWnd, nullptr);
    DWORD curThread = GetCurrentThreadId();

    // Bug fix 2: attach all relevant thread pairs so input state is fully shared
    bool attachedFg  = (fgThread  && fgThread  != curThread) ? AttachThreadInput(curThread, fgThread,  TRUE) != 0 : false;
    bool attachedTgt = (tgtThread && tgtThread != curThread) ? AttachThreadInput(curThread, tgtThread, TRUE) != 0 : false;

    // Bug fix 3: use AllowSetForegroundWindow to explicitly grant permission,
    // which bypasses Windows' focus-stealing prevention for cross-process calls
    AllowSetForegroundWindow(ASFW_ANY);

    // Bring the window to front using the full sequence
    SetForegroundWindow(hWnd);
    SetActiveWindow(hWnd);
    BringWindowToTop(hWnd);
    // Bug fix 4: SetFocus only works reliably once input queues are attached
    SetFocus(hWnd);

    // Always detach — don't leave threads attached on any code path
    if (attachedFg)  AttachThreadInput(curThread, fgThread,  FALSE);
    if (attachedTgt) AttachThreadInput(curThread, tgtThread, FALSE);

    // Verify it actually worked
    HWND newFg = GetForegroundWindow();
    if (newFg != hWnd)
    {
        return returnError("Window did not come to foreground (focus-stealing prevention may be active)");
    }

    result.Set("success", true);
    return result;
}