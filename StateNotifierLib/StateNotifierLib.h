#pragma once

#include <string>
#include <map>

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the STATENOTIFIERLIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// STATENOTIFIERLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef STATENOTIFIERLIB_EXPORTS
#define STATENOTIFIERLIB_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define STATENOTIFIERLIB_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

class CStateNotifierLibPimpl; // private implementation
template class STATENOTIFIERLIB_API std::unique_ptr<CStateNotifierLibPimpl>;

// This class is exported from the StateNotifierLib.dll
class STATENOTIFIERLIB_API CStateNotifierLib {
public:
	CStateNotifierLib(void);
	~CStateNotifierLib(void);
	// TODO: add your methods here.

	bool Init(std::string processName, int instance, std::string host, int port);
	void SendJson(std::string message);
	void SendStatus(std::string sequence, std::string stateName, std::map<std::string, std::string> params);
	void SendEvent(std::string sequence, std::string eventName, std::map<std::string, std::string> params);

private:
	std::unique_ptr<CStateNotifierLibPimpl> _pimpl;
};

//extern STATENOTIFIERLIB_API int nStateNotifierLib;
//
//STATENOTIFIERLIB_API int fnStateNotifierLib(void);
