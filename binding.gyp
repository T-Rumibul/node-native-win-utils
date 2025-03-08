{
    "targets": [
        {
            "target_name": "node-native-win-utils",
            "sources": ["src/cpp/main.cpp"],
            "openssl_flips": "",
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "src/cpp",
                "libs",
                "include",
                "dll"
            ],
            "dependencies": [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "libraries": [
                "dwmapi.lib",
                "windowsapp.lib",
                "-lgdi32", 
                "-lgdiplus",
                "<!(node -p \"require('path').resolve('libs/libjpeg-turbo.lib')\")",
                "<!(node -p \"require('path').resolve('libs/libpng.lib')\")",
                "<!(node -p \"require('path').resolve('libs/zlib.lib')\")",
                "<!(node -p \"require('path').resolve('libs/opencv_core470.lib')\")",
                "<!(node -p \"require('path').resolve('libs/opencv_imgcodecs470.lib')\")",
                "<!(node -p \"require('path').resolve('libs/opencv_imgproc470.lib')\")",
                "<!(node -p \"require('path').resolve('libs/tesseract.lib')\")",

            ],
            "conditions": [
                ["OS=='win'", {
                    "defines": [
                        "_CRT_SECURE_NO_WARNINGS"
                    ],
                    "msvs_settings": {
                        "VCCLCompilerTool": {
                            "AdditionalOptions": ["/EHsc"]
                        }
                    }
                }]
            ],
            "defines": ["NAPI_CPP_EXCEPTIONS"],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-rtti"],
            "cflags": ["/std:c++21"]
        }
    ]
}
