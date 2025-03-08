#include <napi.h>
#include <Windows.h>
#include <thread>
#include <iostream>
#include <unordered_set>
#include <mutex>
#include <atomic>

// Global variables for JavaScript callback functions
Napi::ThreadSafeFunction globalJsCallbackKeyDown;
Napi::ThreadSafeFunction globalJsCallbackKeyUp;

// Thread-safe tracking of pressed keys
std::mutex pressedKeysMutex;
std::unordered_set<int> pressedKeys;

// Thread management
std::atomic<bool> handleKeyDown(false);
std::atomic<bool> handleKeyUp(false);
std::atomic<bool> monitorThreadRunning(false);
DWORD monitorThreadId = 0;


// Environment Cleanup Hook
void CleanupHook() {
    handleKeyDown = false;
    handleKeyUp = false;
    if(globalJsCallbackKeyDown) {
        globalJsCallbackKeyDown.Release();
    }
    if(globalJsCallbackKeyUp) {
        globalJsCallbackKeyUp.Release();
    }
    if (monitorThreadRunning) {
        PostThreadMessage(monitorThreadId, WM_QUIT, 0, 0);
        
    }
}


LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        int keyCode = kbdStruct->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            std::lock_guard<std::mutex> lock(pressedKeysMutex);
            if (pressedKeys.insert(keyCode).second) {
                if (handleKeyDown) {
                    // Allocate copy on heap for thread-safe transfer
                    int* keyCopy = new int(keyCode);
                    globalJsCallbackKeyDown.BlockingCall(keyCopy,
                        [](Napi::Env env, Napi::Function jsCallback, int* keyCode) {
                            jsCallback.Call({Napi::Number::New(env, *keyCode)});
                            delete keyCode; // Clean up allocated memory
                        });
                }
            }
        } 
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            std::lock_guard<std::mutex> lock(pressedKeysMutex);
            if (pressedKeys.erase(keyCode)) {
                if (handleKeyUp) {
                    // Allocate copy on heap for thread-safe transfer
                    int* keyCopy = new int(keyCode);
                    globalJsCallbackKeyUp.BlockingCall(keyCopy,
                        [](Napi::Env env, Napi::Function jsCallback, int* keyCode) {
                            jsCallback.Call({Napi::Number::New(env, *keyCode)});
                            delete keyCode; // Clean up allocated memory
                        });
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MonitorKeyboardEvents() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (!keyboardHook) {
        CoUninitialize();
        monitorThreadRunning = false;
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    CoUninitialize();
    monitorThreadRunning = false;
    monitorThreadId = 0;
}

Napi::Value SetKeyDownCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function jsCallback = info[0].As<Napi::Function>();

       // Release existing callback
    if (globalJsCallbackKeyDown) {
        globalJsCallbackKeyDown.Release();
    }
    globalJsCallbackKeyDown = Napi::ThreadSafeFunction::New(
        env, jsCallback, "KeyDownCallback", 0, 1);
    handleKeyDown = true;

    if (!monitorThreadRunning.exchange(true)) {
        std::thread monitorThread(MonitorKeyboardEvents);
        monitorThreadId = GetThreadId(monitorThread.native_handle());
        monitorThread.detach();
    }

    return env.Undefined();
}

Napi::Value SetKeyUpCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function jsCallback = info[0].As<Napi::Function>();
    

       // Release existing callback
    if (globalJsCallbackKeyUp) {
        globalJsCallbackKeyUp.Release();
    }

    globalJsCallbackKeyUp = Napi::ThreadSafeFunction::New(
        env, jsCallback, "KeyUpCallback", 0, 1);
    handleKeyUp = true;

    if (!monitorThreadRunning.exchange(true)) {
        std::thread monitorThread(MonitorKeyboardEvents);
        monitorThreadId = GetThreadId(monitorThread.native_handle());
        monitorThread.detach();
    }

    return env.Undefined();
}

Napi::Value UnsetKeyDownCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    handleKeyDown = false;
    globalJsCallbackKeyDown.Release();

    if (!handleKeyUp && monitorThreadRunning) {
        PostThreadMessage(monitorThreadId, WM_QUIT, 0, 0);
    }

    return env.Undefined();
}

Napi::Value UnsetKeyUpCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    handleKeyUp = false;
    globalJsCallbackKeyUp.Release();

    if (!handleKeyDown && monitorThreadRunning) {
        PostThreadMessage(monitorThreadId, WM_QUIT, 0, 0);
    }

    return env.Undefined();
}

Napi::Value TypeString(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "You should provide a string to type").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    int delay = 15;
    std::u16string text = info[0].As<Napi::String>().Utf16Value();
    if (info.Length() > 1 && info[1].IsNumber()) {
        delay = info[1].As<Napi::Number>().Int32Value();
    }

    for (const auto &character : text) {
        INPUT keyboardInput = {0};
        keyboardInput.type = INPUT_KEYBOARD;
        keyboardInput.ki.wVk = 0;
        keyboardInput.ki.wScan = character;
        keyboardInput.ki.dwFlags = KEYEVENTF_UNICODE;
        keyboardInput.ki.time = 0;
        keyboardInput.ki.dwExtraInfo = 0;

        // Press the key
        SendInput(1, &keyboardInput, sizeof(INPUT));

        // Release the key
        keyboardInput.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &keyboardInput, sizeof(INPUT));

        // Sleep for the specified delay between keystrokes
        Sleep(delay);
    }

    return Napi::Boolean::New(env, true);
}

// Function to simulate key press and release using provided key code
Napi::Value PressKey(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsNumber())
  {
    Napi::TypeError::New(env, "You should provide a key code to type").ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);;
  }

  int keyCode = info[0].As<Napi::Number>().Int32Value();
  int delay = 15;

  if (info.Length() > 1 && info[1].IsNumber())
  {
    delay = info[1].As<Napi::Number>().Int32Value();
  }

  // Simulate key press
  INPUT keyboardInput = {0};
  keyboardInput.type = INPUT_KEYBOARD;
  keyboardInput.ki.wVk = keyCode;
  keyboardInput.ki.dwFlags = 0;
  keyboardInput.ki.time = 0;

  SendInput(1, &keyboardInput, sizeof(keyboardInput));

  Sleep(delay);

  // Simulate key release
  keyboardInput.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &keyboardInput, sizeof(keyboardInput));

  return Napi::Boolean::New(env, true);
}