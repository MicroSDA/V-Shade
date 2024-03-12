#pragma once
#ifdef SHADE_BUILD_DLL
	#define SHADE_API __declspec(dllexport)
#else
	#define SHADE_API __declspec(dllimport)
#endif

#ifndef SHADE_INLINE
	#define SHADE_INLINE __forceinline
#endif // SHADE_INLINE

#ifndef SNEW
	#define SNEW new 
#endif // SNEW

#ifndef SDELETE
	#define SDELETE delete 
#endif // SDELETE
