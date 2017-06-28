#pragma once

#include <string>
#include <map>
#include <functional>

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the STATENOTIFIERLIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// STATENOTIFIERLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef STATIC_LIB
#ifdef STATENOTIFIERLIB_EXPORTS
#define STATENOTIFIERLIB_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define STATENOTIFIERLIB_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif
#else
#define STATENOTIFIERLIB_API
#define EXPIMP_TEMPLATE
#endif

typedef std::function<void()> connectionCallback;
typedef std::function<void(const std::string& errMsg)> logCallback;

// This class is exported from the StateNotifierLib.dll
class CStateNotifierLib {
public:
	STATENOTIFIERLIB_API CStateNotifierLib(void);
	STATENOTIFIERLIB_API ~CStateNotifierLib(void);

	STATENOTIFIERLIB_API void setCallbackOnConnect(connectionCallback fnct);
	STATENOTIFIERLIB_API void setCallbackOnDisconnect(connectionCallback fnct);
	STATENOTIFIERLIB_API void setCallbackOnInfo(logCallback fnct);
	STATENOTIFIERLIB_API void setCallbackOnError(logCallback fnct);
	STATENOTIFIERLIB_API bool getConnected();

	STATENOTIFIERLIB_API bool Init(const std::string& processName, int instance, const std::string& host, int port);
	STATENOTIFIERLIB_API void EnterStatus(const std::string& sequence, const std::string& stateName, const std::map<std::string, std::string>& params);
	STATENOTIFIERLIB_API void ExitStatus(const std::string& sequence, const std::string& stateName, const std::map<std::string, std::string>& params);
	STATENOTIFIERLIB_API void EventEmit(const std::string& sequence, const std::string& eventName, const std::string& to, const std::map<std::string, std::string>& params);
	STATENOTIFIERLIB_API void EventRecv(const std::string& sequence, const std::string& eventName, const std::string& from, const std::map<std::string, std::string>& params);
	STATENOTIFIERLIB_API void Event(const std::string& sequence, const std::string& eventName, const std::map<std::string, std::string>& params);

private:
	class CStateNotifierLibPimpl;
	std::unique_ptr<CStateNotifierLibPimpl> _pimpl;
};
