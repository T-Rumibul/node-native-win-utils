#include <napi.h>
#include <helpers.cpp>
#include <screenshot.cpp>
#include <getWindowData.cpp>
#include <keyboard.cpp>
#include <mouse.cpp>
#include <opencv.cpp>
#include <tesseract.cpp>

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getWindowData", Napi::Function::New(env, GetWindowData));
    exports.Set("captureWindowN", Napi::Function::New(env, CaptureWindow));
    exports.Set("captureScreenAsync", Napi::Function::New(env, CaptureScreenAsync));
    exports.Set("setKeyDownCallback", Napi::Function::New(env, SetKeyDownCallback));
    exports.Set("setKeyUpCallback", Napi::Function::New(env, SetKeyUpCallback));
    exports.Set("unsetKeyUpCallback", Napi::Function::New(env, UnsetKeyUpCallback));
    exports.Set("unsetKeyDownCallback", Napi::Function::New(env, UnsetKeyDownCallback));
    exports.Set("mouseMove", Napi::Function::New(env, MoveMouse));
    exports.Set("mouseClick", Napi::Function::New(env, ClickMouse));
    exports.Set("mouseDrag", Napi::Function::New(env, DragMouse));
    exports.Set("typeString", Napi::Function::New(env, TypeString));
    exports.Set("pressKey", Napi::Function::New(env, PressKey));
    exports.Set("imread", Napi::Function::New(env, Imread));
    exports.Set("imwrite", Napi::Function::New(env, Imwrite));
    exports.Set("matchTemplate", Napi::Function::New(env, MatchTemplate));
    exports.Set("blur", Napi::Function::New(env, Blur));
    exports.Set("bgrToGray", Napi::Function::New(env, BgrToGray));
    exports.Set("equalizeHist", Napi::Function::New(env, EqualizeHist));
    exports.Set("darkenColor", Napi::Function::New(env, DarkenColor));
    exports.Set("drawRectangle", Napi::Function::New(env, DrawRectangle));
    exports.Set("getRegion", Napi::Function::New(env, GetRegion));
    exports.Set("textRecognition", Napi::Function::New(env, TextRecognition));
    static GdiPlusInitializer gdiplusInitializer;
     // Register cleanup hook
    env.AddCleanupHook(CleanupHook);
    return exports;
}

NODE_API_MODULE(addon, Init)
