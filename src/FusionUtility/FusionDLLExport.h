#pragma once

#ifdef _WIN32 

#define BUILD_DLL

#ifdef BUILD_DLL 
#define FUSIONFRAME_EXPORT __declspec(dllexport)
#else
#define FUSIONFRAME_EXPORT __declspec(dllimport)
#endif

#define FUSIONFRAME_EXPORT_FUNCTION extern FUSIONFRAME_EXPORT
#define FUSIONFRAME_EXPORT_FUNCTION_C_LINKAGE extern "C" FUSIONFRAME_EXPORT

#elif defined(__linux__)

#define FUSIONFRAME_EXPORTs
#define FUSIONFRAME_EXPORT_FUNCTION
#define FUSIONFRAME_EXPORT_FUNCTION_C_LINKAGE 

#endif // 