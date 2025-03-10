#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <napi.h>

Napi::Value Imread(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String expected for image file path").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string filename = info[0].ToString().Utf8Value();

    int flags = cv::IMREAD_COLOR;
    if (info.Length() > 1 && info[1].IsNumber())
    {
        flags = info[1].ToNumber().Int32Value();
    }

    cv::Mat image = cv::imread(filename, flags);

    if (image.empty())
    {
        Napi::TypeError::New(env, "Failed to load image").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Create a new Uint8Array with the correct size

    size_t totalBytes = image.total() * image.elemSize();
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    // Copy the image data to the Uint8Array
    memcpy(uint8Array.Data(), image.data, totalBytes);
    // std::cout << totalBytes << std::endl;
    // std::cout << uint8Array.ByteLength() << std::endl;
    // Create a JavaScript object with 'width', 'height', and 'data' properties
    Napi::Object result = Napi::Object::New(env);

    result.Set("width", image.cols);
    result.Set("height", image.rows);
    result.Set("data", uint8Array);

    return result;
}

Napi::Value Imwrite(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: object").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object imageData = info[0].As<Napi::Object>();

    if (!imageData.Has("width") || !imageData.Has("height") || !imageData.Has("data"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'width', 'height', 'data'").ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = imageData.Get("width").ToNumber().Int32Value();
    int height = imageData.Get("height").ToNumber().Int32Value();

    cv::Mat image;
    if (imageData.Get("data").IsTypedArray())
    {
        Napi::TypedArray typedArray = imageData.Get("data").As<Napi::TypedArray>();
        int elementSize = typedArray.ByteLength() / (width * height);
        // std::cout << typedArray.ByteLength() << std::endl;
        // std::cout << elementSize << std::endl;
        int type = elementSize == 1 ? CV_8UC1 : CV_8UC3;

        image = cv::Mat(height, width, type);
        memcpy(image.data, typedArray.As<Napi::Uint8Array>().Data(), typedArray.ByteLength());
    }
    else
    {
        Napi::TypeError::New(env, "'data' property must be a TypedArray").ThrowAsJavaScriptException();
        return env.Null();
    }
    std::vector<uchar> buffer;
    cv::imencode(".png", image, buffer);

    Napi::Buffer<uchar> resultBuffer = Napi::Buffer<uchar>::Copy(env, buffer.data(), buffer.size());
    return resultBuffer;
}

Napi::Value MatchTemplate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsObject())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object, object[, object])").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object srcObj = info[0].As<Napi::Object>();
    Napi::Object templObj = info[1].As<Napi::Object>();
    int method = cv::TM_CCOEFF_NORMED;
    if (info.Length() > 2 && !info[2].IsNumber())
    {
        method = info[2].ToNumber().Int32Value();
    }

    if (!srcObj.Has("data") || !templObj.Has("data"))
    {
        Napi::TypeError::New(env, "Invalid image data objects. Expected properties: 'data'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!srcObj.Has("width") || !srcObj.Has("height") || !templObj.Has("width") || !templObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data objects. Expected properties: 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!srcObj.Get("data").IsTypedArray() || !templObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' properties").ThrowAsJavaScriptException();
        return env.Null();
    }

    int srcWidth = srcObj.Get("width").ToNumber().Int32Value();
    int srcHeight = srcObj.Get("height").ToNumber().Int32Value();
    int templWidth = templObj.Get("width").ToNumber().Int32Value();
    int templHeight = templObj.Get("height").ToNumber().Int32Value();

    Napi::ArrayBuffer srcBufferData = srcObj.Get("data").As<Napi::TypedArray>().ArrayBuffer();
    Napi::ArrayBuffer templBufferData = templObj.Get("data").As<Napi::TypedArray>().ArrayBuffer();

    cv::Mat src(srcHeight, srcWidth, CV_8UC1, srcBufferData.Data());
    cv::Mat templ(templHeight, templWidth, CV_8UC1, templBufferData.Data());

    cv::Mat mask;
    if (info.Length() >= 4 && info[2].IsObject())
    {
        Napi::Object maskObj = info[2].As<Napi::Object>();
        if (maskObj.Has("data") && maskObj.Get("data").IsTypedArray())
        {
            int maskWidth = maskObj.Get("width").ToNumber().Int32Value();
            int maskHeight = maskObj.Get("height").ToNumber().Int32Value();
            Napi::ArrayBuffer maskBufferData = maskObj.Get("data").As<Napi::TypedArray>().ArrayBuffer();
            mask = cv::Mat(maskHeight, maskWidth, CV_8UC1, maskBufferData.Data());
        }
    }

    cv::Mat result;
    if (mask.empty())
    {
        cv::matchTemplate(src, templ, result, method);
    }
    else
    {
        cv::matchTemplate(src, templ, result, method, mask);
    }

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    Napi::Object matchResult = Napi::Object::New(env);
    matchResult.Set("minValue", minVal);
    matchResult.Set("maxValue", maxVal);
    Napi::Object minLocation = Napi::Object::New(env);
    minLocation.Set("x", Napi::Number::New(env, minLoc.x));
    minLocation.Set("y", Napi::Number::New(env, minLoc.y));
    matchResult.Set("minLocation", minLocation);

    Napi::Object maxLocation = Napi::Object::New(env);
    maxLocation.Set("x", Napi::Number::New(env, maxLoc.x));
    maxLocation.Set("y", Napi::Number::New(env, maxLoc.y));
    matchResult.Set("maxLocation", maxLocation);

    return matchResult;
}

Napi::Value BgrToGray(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object)").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object srcObj = info[0].As<Napi::Object>();

    if (!srcObj.Has("data") || !srcObj.Has("width") || !srcObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'data', 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!srcObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = srcObj.Get("width").ToNumber().Int32Value();
    int height = srcObj.Get("height").ToNumber().Int32Value();

    cv::Mat src(height, width, CV_8UC3, srcObj.Get("data").As<Napi::TypedArray>().ArrayBuffer().Data());
    cv::Mat gray;

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    Napi::Object result = Napi::Object::New(env);
    // Create a new Uint8Array with the correct size
    // std::cout << gray.elemSize() << std::endl;
    size_t totalBytes = width * height;
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    // Copy the image data to the Uint8Array
    memcpy(uint8Array.Data(), gray.data, totalBytes);

    result.Set("width", width);
    result.Set("height", height);
    result.Set("data", uint8Array);

    return result;
}




Napi::Value EqualizeHist(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object)").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object srcObj = info[0].As<Napi::Object>();

    if (!srcObj.Has("data") || !srcObj.Has("width") || !srcObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'data', 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!srcObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = srcObj.Get("width").ToNumber().Int32Value();
    int height = srcObj.Get("height").ToNumber().Int32Value();

    cv::Mat src(height, width, CV_8UC1, srcObj.Get("data").As<Napi::TypedArray>().ArrayBuffer().Data());
    cv::Mat dst;

    cv::equalizeHist(src, dst);

    Napi::Object result = Napi::Object::New(env);
    size_t totalBytes = dst.total() * dst.elemSize();
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    memcpy(uint8Array.Data(), dst.data, totalBytes);
    result.Set("width", width);
    result.Set("height", height);
    result.Set("data", uint8Array);
    
    return result;
}


Napi::Value Blur(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 3 || !info[0].IsObject() || !info[1].IsNumber() || !info[2].IsNumber())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object, number, number)").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object srcObj = info[0].As<Napi::Object>();
    int ksizeX = info[1].As<Napi::Number>().Int32Value();
    int ksizeY = info[2].As<Napi::Number>().Int32Value();

    if (!srcObj.Has("data") || !srcObj.Has("width") || !srcObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'data', 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!srcObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = srcObj.Get("width").ToNumber().Int32Value();
    int height = srcObj.Get("height").ToNumber().Int32Value();

    cv::Mat src(height, width, CV_8UC3, srcObj.Get("data").As<Napi::TypedArray>().ArrayBuffer().Data());
    cv::Mat blurred;

    cv::blur(src, blurred, cv::Size(ksizeX, ksizeY));

    Napi::Object result = Napi::Object::New(env);
    size_t totalBytes = blurred.total() * blurred.elemSize();
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    // Copy the image data to the Uint8Array
    memcpy(uint8Array.Data(), blurred.data, totalBytes);
    result.Set("width", width);
    result.Set("height", height);
    result.Set("data", uint8Array);
    return result;
}

Napi::Value DrawRectangle(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 5 || !info[0].IsObject() || !info[1].IsArray() || !info[2].IsArray() || !info[3].IsArray() || !info[4].IsNumber())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object, array, array, array, number)").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object imageObj = info[0].As<Napi::Object>();
    Napi::Array point1Array = info[1].As<Napi::Array>();
    Napi::Array point2Array = info[2].As<Napi::Array>();
    int thickness = info[4].As<Napi::Number>().Int32Value();
    if (point1Array.Length() < 2 || point2Array.Length() < 2)
    {
        Napi::TypeError::New(env, "Invalid point array length. Expected: [x, y]").ThrowAsJavaScriptException();
        return env.Null();
    }

    int x1 = point1Array.Get((uint32_t)0).ToNumber().Int32Value();
    int y1 = point1Array.Get((uint32_t)1).ToNumber().Int32Value();
    int x2 = point2Array.Get((uint32_t)0).ToNumber().Int32Value();
    int y2 = point2Array.Get((uint32_t)1).ToNumber().Int32Value();

    Napi::Array colorArray = info[3].As<Napi::Array>();

    if (colorArray.Length() < 3)
    {
        Napi::TypeError::New(env, "Invalid color array length. Expected: [r, g, b]").ThrowAsJavaScriptException();
        return env.Null();
    }

    int r = colorArray.Get((uint32_t)0).ToNumber().Int32Value();
    int g = colorArray.Get((uint32_t)1).ToNumber().Int32Value();
    int b = colorArray.Get((uint32_t)2).ToNumber().Int32Value();

    if (!imageObj.Has("data") || !imageObj.Has("width") || !imageObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'data', 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!imageObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = imageObj.Get("width").ToNumber().Int32Value();
    int height = imageObj.Get("height").ToNumber().Int32Value();

    cv::Mat image(height, width, CV_8UC3, imageObj.Get("data").As<Napi::TypedArray>().ArrayBuffer().Data());

    cv::rectangle(image, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(b, g, r), thickness);
    Napi::Object result = Napi::Object::New(env);

    size_t totalBytes = image.total() * image.elemSize();
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    // Copy the image data to the Uint8Array
    memcpy(uint8Array.Data(), image.data, totalBytes);
    result.Set("width", width);
    result.Set("height", height);
    result.Set("data", uint8Array);
    return result;
}

Napi::Value GetRegion(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsObject())
    {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object)").ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Object imageObj = info[0].As<Napi::Object>();
    Napi::Object options = info[1].As<Napi::Object>();

    int x = options.Get((uint32_t)0).ToNumber().Int32Value();
    int y = options.Get((uint32_t)1).ToNumber().Int32Value();
    int width = options.Get((uint32_t)2).ToNumber().Int32Value();
    int height = options.Get((uint32_t)3).ToNumber().Int32Value();

    if (!imageObj.Has("data") || !imageObj.Has("width") || !imageObj.Has("height"))
    {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'data', 'width', 'height'").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!imageObj.Get("data").IsTypedArray())
    {
        Napi::TypeError::New(env, "TypedArray expected for 'data' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    int imageWidth = imageObj.Get("width").ToNumber().Int32Value();
    int imageHeight = imageObj.Get("height").ToNumber().Int32Value();

    if (x < 0 || y < 0 || width <= 0 || height <= 0 || x + width > imageWidth || y + height > imageHeight)
    {
        Napi::TypeError::New(env, "Invalid region coordinates or size").ThrowAsJavaScriptException();
        return env.Null();
    }

    cv::Mat image(imageHeight, imageWidth, CV_8UC3, imageObj.Get("data").As<Napi::TypedArray>().ArrayBuffer().Data());

    cv::Rect region(x, y, width, height);
    cv::Mat regionImage = image(region).clone();

    size_t totalBytes = regionImage.total() * regionImage.elemSize();
    Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, totalBytes);
    Napi::Uint8Array uint8Array = Napi::Uint8Array::New(env, totalBytes, arrayBuffer, 0);

    // Copy the image data to the Uint8Array
    memcpy(uint8Array.Data(), regionImage.data, totalBytes);
    Napi::Object result = Napi::Object::New(env);
    result.Set("data", uint8Array);
    result.Set("width", Napi::Number::New(env, regionImage.cols));
    result.Set("height", Napi::Number::New(env, regionImage.rows));

    return result;
}





Napi::Value DarkenColor(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    // Validate arguments: (imageData: Object, lowerBound: Array, upperBound: Array, number darkFactor)
    if (info.Length() < 4 ||
        !info[0].IsObject() ||
        !info[1].IsArray() ||
        !info[2].IsArray() ||
        !info[3].IsNumber()) {
        Napi::TypeError::New(env, "Invalid arguments. Expected: (object, array, array, number)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object imageData = info[0].As<Napi::Object>();
    Napi::Array lowerArray = info[1].As<Napi::Array>();
    Napi::Array upperArray = info[2].As<Napi::Array>();
    double darkFactor = info[3].ToNumber().DoubleValue();

    // Validate imageData object properties
    if (!imageData.Has("width") || !imageData.Has("height") || !imageData.Has("data")) {
        Napi::TypeError::New(env, "Invalid image data object. Expected properties: 'width', 'height', 'data'")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // Validate bounds arrays length
    if (lowerArray.Length() != 3 || upperArray.Length() != 3) {
        Napi::TypeError::New(env, "Lower and upper color bounds must be arrays of length 3")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    int width = imageData.Get("width").ToNumber().Int32Value();
    int height = imageData.Get("height").ToNumber().Int32Value();

    if (!imageData.Get("data").IsTypedArray()) {
        Napi::TypeError::New(env, "'data' property must be a TypedArray")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Uint8Array uint8Array = imageData.Get("data").As<Napi::Uint8Array>();
    // Account for potential byteOffset in the typed array.
    uint8_t* dataPtr = reinterpret_cast<uint8_t*>(uint8Array.ArrayBuffer().Data()) + uint8Array.ByteOffset();

    // Create a cv::Mat using the underlying data of the Uint8Array.
    cv::Mat inputImage(height, width, CV_8UC3, dataPtr);
    cv::Mat image;
    cv::cvtColor(inputImage, image, cv::COLOR_BGR2RGB);
    // Create lower and upper bounds from the provided arrays.
    cv::Scalar lower_bound(
        lowerArray.Get((uint32_t)0).ToNumber().DoubleValue(),
        lowerArray.Get((uint32_t)1).ToNumber().DoubleValue(),
        lowerArray.Get((uint32_t)2).ToNumber().DoubleValue()
    );
    cv::Scalar upper_bound(
        upperArray.Get((uint32_t)0).ToNumber().DoubleValue(),
        upperArray.Get((uint32_t)1).ToNumber().DoubleValue(),
        upperArray.Get((uint32_t)2).ToNumber().DoubleValue()
    );

    // Loop through the image pixels and darken the ones within the color range
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            cv::Vec3b& color = image.at<cv::Vec3b>(y, x);

            // Check if the pixel is within the color range
            if (color[0] >= lower_bound[0] && color[0] <= upper_bound[0] &&
                color[1] >= lower_bound[1] && color[1] <= upper_bound[1] &&
                color[2] >= lower_bound[2] && color[2] <= upper_bound[2]) {

                // Darken the pixel by scaling down its color values
                color[0] = std::max(0, int(color[0] * darkFactor));
                color[1] = std::max(0, int(color[1] * darkFactor));
                color[2] = std::max(0, int(color[2] * darkFactor));
            }
        }
    }
    cv::cvtColor(image, inputImage, cv::COLOR_RGB2BGR);
    // Return the modified imageData (which shares the same underlying data).
    return imageData;
}