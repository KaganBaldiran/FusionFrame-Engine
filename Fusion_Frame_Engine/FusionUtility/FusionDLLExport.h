#pragma once

#define BUILD_DLL

#ifdef BUILD_DLL 
#define FUSIONFRAME_EXPORT __declspec(dllexport)
#else
#define FUSIONFRAME_EXPORT __declspec(dllimport)
#endif

#define FUSIONFRAME_EXPORT_FUNCTION extern FUSIONFRAME_EXPORT
#define FUSIONFRAME_EXPORT_FUNCTION_C_LINKAGE extern "C" FUSIONFRAME_EXPORT
