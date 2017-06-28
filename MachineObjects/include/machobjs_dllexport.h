#ifndef MACHOBJS_DLLEXPORT_H
#define MACHOBJS_DLLEXPORT_H

// LOGLIB_DLL is defined only in "LogLib.dll" project
#ifdef _WIN32
#	ifdef MACHOBJS_DLL
#		define MACHOBJS_DllExport	__declspec( dllexport )
#		define MACHOBJS_DllImport	__declspec( dllimport )
#	elif STATIC_LIB
#		define MACHOBJS_DllExport
#		define MACHOBJS_DllImport
#	else
#		define MACHOBJS_DllExport	__declspec( dllimport )
#		define MACHOBJS_DllImport	__declspec( dllimport )
#	endif
#else // _WIN32
#	define MACHOBJS_DllExport
#	define MACHOBJS_DllImport
#endif // _WIN32


// Additional STATIC_LIB dependencies
#if defined(STATIC_LIB)
#	if defined(_DEBUG)
#		pragma comment(lib,"SmallObjectsD_static")
// #		pragma comment(lib,"SysLogLibD_static")
#	elif defined(NDEBUG)
#		pragma comment(lib,"SmallObjects_static")
// #		pragma comment(lib,"SysLogLib_static")
#	endif
#endif // STATIC_LIB


#endif // MACHOBJS_DLLEXPORT_H