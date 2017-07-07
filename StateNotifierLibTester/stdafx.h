// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define POCO_STATIC

// TODO: reference additional headers your program requires here
#include <conio.h>
#include <iostream>
#include <string.h>
#include <map>

#include "Poco\Util\Application.h"
#include "Poco\Util\Option.h"
#include "Poco\Util\HelpFormatter.h"
#include "Poco\Logger.h"
#include "Poco\WindowsConsoleChannel.h"
#include "Poco\FormattingChannel.h"
#include "Poco\PatternFormatter.h"
#include "Poco\AutoPtr.h"
#include "Poco\String.h"
#include "Poco\NumberParser.h"