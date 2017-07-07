// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "tchar.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#define POCO_STATIC
#include <queue>
#include <algorithm>
#include <memory>
#include <math.h>

#include "finally.h"

#include "Poco/JSON/Object.h"
#include "Poco/Net/NetException.h"
#include "Poco/Format.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/Exception.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Buffer.h"

using namespace std;