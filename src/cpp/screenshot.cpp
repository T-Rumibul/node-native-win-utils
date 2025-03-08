#include <napi.h>
#include <iostream>
#include <Windows.h>
#include <dxgi.h>
#include <inspectable.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <roerrorapi.h>
#include <shlobj_core.h>
#include <dwmapi.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <vector>
#include <stdexcept>
#include <gdiplus.h>
#include <cstdlib>

#pragma comment (lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace Napi;
// Global GDI+ initialization token
ULONG_PTR gdiplusToken = 0;

// RAII wrapper for GDI+ initialization
class GdiPlusInitializer {
public:
    GdiPlusInitializer() {
        GdiplusStartupInput startupInput;
        GdiplusStartup(&gdiplusToken, &startupInput, NULL);
    }

    ~GdiPlusInitializer() {
        GdiplusShutdown(gdiplusToken);
    }
};





Napi::Value CaptureWindow(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "Window name must be provided as a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string windowName = info[0].As<Napi::String>().Utf8Value();
    HWND hwndTarget = FindWindowA(NULL, windowName.c_str());
    if (!hwndTarget)
    {
        Napi::TypeError::New(env, "Window not found").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Init COM
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // Create Direct 3D Device
    winrt::com_ptr<ID3D11Device> d3dDevice;
    winrt::check_hresult(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        d3dDevice.put(),
        nullptr,
        nullptr));

    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device;
    const auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    {
        winrt::com_ptr<::IInspectable> inspectable;
        winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put()));
        device = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
    }

    auto idxgiDevice2 = dxgiDevice.as<IDXGIDevice2>();
    winrt::com_ptr<IDXGIAdapter> adapter;
    winrt::check_hresult(idxgiDevice2->GetParent(winrt::guid_of<IDXGIAdapter>(), adapter.put_void()));
    winrt::com_ptr<IDXGIFactory2> factory;
    winrt::check_hresult(adapter->GetParent(winrt::guid_of<IDXGIFactory2>(), factory.put_void()));

    ID3D11DeviceContext *d3dContext = nullptr;
    d3dDevice->GetImmediateContext(&d3dContext);

    RECT rect{};
    DwmGetWindowAttribute(hwndTarget, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    const auto size = winrt::Windows::Graphics::SizeInt32{rect.right - rect.left, rect.bottom - rect.top};

    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool =
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
            device,
            winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            size);

    const auto activationFactory = winrt::get_activation_factory<
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interopFactory = activationFactory.as<IGraphicsCaptureItemInterop>();
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem captureItem = {nullptr};
    interopFactory->CreateForWindow(hwndTarget, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
                                    reinterpret_cast<void **>(winrt::put_abi(captureItem)));

    auto isFrameArrived = false;
    winrt::com_ptr<ID3D11Texture2D> texture;
    const auto session = m_framePool.CreateCaptureSession(captureItem);
    m_framePool.FrameArrived([&](auto &framePool, auto &)
                             {
        if (isFrameArrived) return;
        auto frame = framePool.TryGetNextFrame();

        struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
            IDirect3DDxgiInterfaceAccess : ::IUnknown
        {
            virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
        };

        auto access = frame.Surface().as<IDirect3DDxgiInterfaceAccess>();
        access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), texture.put_void());
        isFrameArrived = true; });

    session.IsCursorCaptureEnabled(false);
    session.StartCapture();

    // Message pump
    MSG msg;
    clock_t timer = clock();
    while (!isFrameArrived)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
            DispatchMessage(&msg);

        if (clock() - timer > 20000)
        {
            // TODO: try to make here a better error handling
            return env.Null();
        }
    }

    session.Close();

    D3D11_TEXTURE2D_DESC capturedTextureDesc;
    texture->GetDesc(&capturedTextureDesc);

    capturedTextureDesc.Usage = D3D11_USAGE_STAGING;
    capturedTextureDesc.BindFlags = 0;
    capturedTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    capturedTextureDesc.MiscFlags = 0;

    winrt::com_ptr<ID3D11Texture2D> userTexture = nullptr;
    winrt::check_hresult(d3dDevice->CreateTexture2D(&capturedTextureDesc, NULL, userTexture.put()));

    d3dContext->CopyResource(userTexture.get(), texture.get());

    D3D11_MAPPED_SUBRESOURCE resource;
    winrt::check_hresult(d3dContext->Map(userTexture.get(), NULL, D3D11_MAP_READ, 0, &resource));

    BITMAPINFO lBmpInfo;

    // BMP 32 bpp
    ZeroMemory(&lBmpInfo, sizeof(BITMAPINFO));
    lBmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lBmpInfo.bmiHeader.biBitCount = 32;
    lBmpInfo.bmiHeader.biCompression = BI_RGB;
    lBmpInfo.bmiHeader.biWidth = capturedTextureDesc.Width;
    lBmpInfo.bmiHeader.biHeight = capturedTextureDesc.Height;
    lBmpInfo.bmiHeader.biPlanes = 1;
    lBmpInfo.bmiHeader.biSizeImage = capturedTextureDesc.Width * capturedTextureDesc.Height * 4;

    auto imageBuffer = cv::Mat(capturedTextureDesc.Height, capturedTextureDesc.Width, CV_8UC4, resource.pData, resource.RowPitch);
    std::vector<uchar> encodedImage;
    cv::imencode(".png", imageBuffer, encodedImage);

    if (encodedImage.empty())
    {
        return env.Null(); // Error handling for encoding failure
    }

    auto data = encodedImage.data();

    d3dContext->Unmap(userTexture.get(), 0);
    Napi::Buffer<uchar> resultBuffer = Napi::Buffer<uchar>::Copy(env, data, encodedImage.size());

    return resultBuffer;
}



class ScreenCaptureWorker : public AsyncWorker {
  public:
      ScreenCaptureWorker(Napi::Env env, Promise::Deferred deferred)
          : AsyncWorker(env), deferred(deferred), buffer(nullptr), bufferSize(0) {}
  
      ~ScreenCaptureWorker() {
          if (buffer) delete[] buffer;
      }
  
      void Execute() override {
        HDC hdcScreen = NULL;
        HDC hdcMem = NULL;
        HBITMAP hBitmap = NULL;
        IStream* stream = NULL;

        try {
            // Step 1: Capture screen using GDI
            hdcScreen = GetDC(NULL);
            if (!hdcScreen) throw "Failed to get screen DC";

            int width = GetSystemMetrics(SM_CXSCREEN);
            int height = GetSystemMetrics(SM_CYSCREEN);

            hdcMem = CreateCompatibleDC(hdcScreen);
            if (!hdcMem) throw "Failed to create compatible DC";

            hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
            if (!hBitmap) throw "Failed to create bitmap";

            SelectObject(hdcMem, hBitmap);
            if (!BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY)) {
                throw "Failed to copy screen content";
            }

            // Step 2: Convert bitmap to PNG using GDI+
            Bitmap bitmap(hBitmap, NULL);
            if (bitmap.GetLastStatus() != Ok) {
                throw "Failed to create GDI+ bitmap";
            }

            // Create in-memory stream to store PNG
            if (CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK) {
                throw "Failed to create stream";
            }

            CLSID pngClsid;
            if (GetEncoderClsid(L"image/png", &pngClsid) == -1) {
                throw "Failed to get PNG encoder";
            }

            if (bitmap.Save(stream, &pngClsid, NULL) != Ok) {
                throw "Failed to save PNG to stream";
            }

            // Get PNG data from stream
            STATSTG stat;
            stream->Stat(&stat, STATFLAG_NONAME);
            bufferSize = static_cast<DWORD>(stat.cbSize.QuadPart);
            
            HGLOBAL hGlobal = NULL;
            GetHGlobalFromStream(stream, &hGlobal);
            buffer = static_cast<BYTE*>(GlobalLock(hGlobal));
            if (!buffer) throw "Failed to lock stream memory";

            // Copy to local buffer as stream will be released
            BYTE* tempBuffer = new BYTE[bufferSize];
            memcpy(tempBuffer, buffer, bufferSize);
            buffer = tempBuffer;

            // Cleanup
            GlobalUnlock(hGlobal);
            stream->Release();
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdcScreen);
        }
        catch (const char* error) {
            SetError(error);
        }
        catch (...) {
            SetError("Unknown error occurred during screen capture");
        }

        // Cleanup in case of exceptions
        if (stream) stream->Release();
        if (hBitmap) DeleteObject(hBitmap);
        if (hdcMem) DeleteDC(hdcMem);
        if (hdcScreen) ReleaseDC(NULL, hdcScreen);
      }
  
      void OnOK() override {
          Buffer<BYTE> result = Buffer<BYTE>::Copy(Env(), buffer, bufferSize);
          deferred.Resolve(result);
      }
  
      void OnError(const Napi::Error& e) override {
          deferred.Reject(e.Value());
      }
  
  private:
      Promise::Deferred deferred;
      BYTE* buffer;
      DWORD bufferSize;
  
      int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
          UINT numEncoders = 0;
          UINT size = 0;
  
          if (GetImageEncodersSize(&numEncoders, &size) != Ok || size == 0)
              return -1;
  
          ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
          if (!pImageCodecInfo) return -1;
  
          if (GetImageEncoders(numEncoders, size, pImageCodecInfo) != Ok) {
              free(pImageCodecInfo);
              return -1;
          }
  
          for (UINT i = 0; i < numEncoders; i++) {
              if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
                  *pClsid = pImageCodecInfo[i].Clsid;
                  free(pImageCodecInfo);
                  return i;
              }
          }
  
          free(pImageCodecInfo);
          return -1;
      }
  };
  
  Value CaptureScreenAsync(const CallbackInfo& info) {
    Env env = info.Env();
    Promise::Deferred deferred = Promise::Deferred::New(env);
    ScreenCaptureWorker* worker = new ScreenCaptureWorker(env, deferred);
    worker->Queue();
    return deferred.Promise();
}





