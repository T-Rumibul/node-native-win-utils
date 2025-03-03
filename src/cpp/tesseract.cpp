#include "tesseract/tesseract.h"
#include <napi.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

Napi::Value TextRecognition(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString())
    {
        Napi::TypeError::New(env, "Missing argument or wront type").ThrowAsJavaScriptException();
        return env.Null();
    }
    std::string trainedDataPath = info[0].ToString().Utf8Value();
    std::string dataLang = info[1].ToString().Utf8Value();
    std::string imagePath = info[2].ToString().Utf8Value();

    cv::Mat im = cv::imread(imagePath, cv::IMREAD_COLOR);

    // Create an instance of Tesseract API
    tesseract::TessBaseAPI *tess = Tesseract_Create();

    // Initialize with language and data path
    if (Tesseract_Init(tess, trainedDataPath.c_str(), dataLang.c_str()) != 0)
    {
        Napi::TypeError::New(env, "Could not initialize Tesseract!").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Set OCR mode
    Tesseract_SetPageSegMode(tess, 3);

    // Do OCR processing
    Tesseract_SetImage(tess, im.data, im.cols, im.rows, 3, im.step);

    // Get OCR result
    int len;
    const char *text = Tesseract_GetUTF8Text(tess, &len);
    std::string outText = text;

    // Free text memory
    Tesseract_FreeUTF8Text((char *&)text);

    // Clean up
    Tesseract_End(tess);
    Tesseract_Delete(tess);
    return Napi::String::New(env, outText);
}