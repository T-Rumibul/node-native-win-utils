// The following ifdef block is the standard way of creating macros which make STATICTESSERACT_APIing
// from a DLL simpler. All files within this DLL are compiled with the STATICTESSERACT_STATICTESSERACT_APIS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// STATICTESSERACT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being STATICTESSERACT_APIed.
#ifdef STATICTESSERACT_EXPORTS
#define STATICTESSERACT_API __declspec(dllexport)
#else
#define STATICTESSERACT_API __declspec(dllimport)
#endif
#include <tesseract/baseapi.h>
#include <tesseract/capi.h>


extern "C"
{
	STATICTESSERACT_API tesseract::TessBaseAPI* Tesseract_Create();
	STATICTESSERACT_API void Tesseract_Delete(tesseract::TessBaseAPI*& tesseract_ptr);
	STATICTESSERACT_API int Tesseract_Init(tesseract::TessBaseAPI* tesseract_ptr, const char* datapath, const char* language);
	STATICTESSERACT_API void Tesseract_End(tesseract::TessBaseAPI*& tesseract_ptr);
	STATICTESSERACT_API void Tesseract_SetPageSegMode(tesseract::TessBaseAPI* tesseract_ptr, int mode);
	STATICTESSERACT_API int Tesseract_GetPageSegMode(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API float Tesseract_MeanTextConf(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API void Tesseract_SetImage(tesseract::TessBaseAPI* tesseract_ptr, const unsigned char* imagedata, int width, int height, int bytes_per_pixel, int bytes_per_line);
	STATICTESSERACT_API const char* Tesseract_GetUTF8Text(tesseract::TessBaseAPI* tesseract_ptr, int* len);
	STATICTESSERACT_API void Tesseract_FreeUTF8Text(char*& utf8_text_ptr);
	STATICTESSERACT_API bool Tesseract_SetVariable(tesseract::TessBaseAPI* tesseract_ptr, const char* name, const char* value);
	STATICTESSERACT_API void Tesseract_Clear(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API int Tesseract_GetBlockCount(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API int Tesseract_GetLineCount(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API int Tesseract_GetWordCount(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API int Tesseract_GetCharacterCount(tesseract::TessBaseAPI* tesseract_ptr);
	STATICTESSERACT_API void Tesseract_GetBlockMatch(tesseract::TessBaseAPI* tesseract_ptr, int index, char*& text, int* len, float* confidence, int* x1, int* y1, int* x2, int* y2);
	STATICTESSERACT_API void Tesseract_GetLineMatch(tesseract::TessBaseAPI* tesseract_ptr, int index, char*& text, int* len, float* confidence, int* x1, int* y1, int* x2, int* y2);
	STATICTESSERACT_API void Tesseract_GetWordMatch(tesseract::TessBaseAPI* tesseract_ptr, int index, char*& text, int* len, float* confidence, int* x1, int* y1, int* x2, int* y2);
	STATICTESSERACT_API void Tesseract_GetCharacterMatch(tesseract::TessBaseAPI* tesseract_ptr, int index, char*& text, int* len, float* confidence, int* x1, int* y1, int* x2, int* y2);

}

