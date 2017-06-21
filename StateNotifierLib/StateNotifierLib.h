#pragma once

#include <string>
#include <map>
#include <functional>

typedef std::function<void()> connectionCallback;

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

	void setCallbackOnConnect(connectionCallback fnct);
	void setCallbackOnDisconnect(connectionCallback fnct);
	bool getConnected();

	bool Init(const std::string& processName, int instance, const std::string& host, int port);
	void EnterStatus(const std::string& sequence, const std::string& stateName, const std::map<std::string, std::string>& params);
	void ExitStatus(const std::string& sequence, const std::string& stateName, const std::map<std::string, std::string>& params);
	void SendEvent(const std::string& sequence, const std::string& eventName, const std::map<std::string, std::string>& params);

private:
	std::unique_ptr<CStateNotifierLibPimpl> _pimpl;
};
