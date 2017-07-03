========================================================================
	DYNAMIC LINK LIBRARY : StateNotifierLib Project Overview
========================================================================

AppWizard has created this StateNotifierLib DLL for you.

This file contains a summary of what you will find in each of the files that
make up your StateNotifierLib application.


StateNotifierLib.vcxproj
	This is the main project file for VC++ projects generated using an Application Wizard.
	It contains information about the version of Visual C++ that generated the file, and
	information about the platforms, configurations, and project features selected with the
	Application Wizard.

StateNotifierLib.vcxproj.filters
	This is the filters file for VC++ projects generated using an Application Wizard.
	It contains information about the association between the files in your project
	and the filters. This association is used in the IDE to show grouping of files with
	similar extensions under a specific node (for e.g. ".cpp" files are associated with the
	"Source Files" filter).

StateNotifierLib.cpp
	This is the main DLL source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
	These files are used to build a precompiled header (PCH) file
	named StateNotifierLib.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////


Esempio:

{
	"process": "PumpManager",
	"instance": 0,
	"type": "ENTERSTATE"
	"startTime" : "2017-06-20T07:21:59Z",
	"sequence": "pump1",
	"name": "idle",
	"paramters" : {
		"param1": "value1",
		"param2": "value2",
	}
}


{
	"process": "PumpManager",
	"instance": 0,
	"type": "EXITSTATE",
	"startTime" : "2017-06-20T07:21:59Z",
	"sequence": "pump1",
	"name": "idle",
	"paramters" : {
		"param1": "value1",
		"param2": "value2",
	}
}

{
	"process": "PumpManager",
	"instance": 0,
	"type": "EVENT_EMIT",
	"to": "",
	"name":
	"startTime" : "2017-06-20T07:21:59Z",
	"paramters" : {
		"param1": "value1",
		"param2": "value2",
	}
}

{
	"process": "PumpManager",
	"instance": 0,
	"type": "EVENT_RECV",
	"from": "",
	"name":
	"startTime" : "2017-06-20T07:21:59Z",
	"paramters" : {
		"param1": "value1",
		"param2": "value2",
	}
}

{
	"process": "PumpManager",
	"instance": 0,
	"type": "EVENT",
	"from": "",
	"name":
	"startTime" : "2017-06-20T07:21:59Z",
	"paramters" : {
		"param1": "value1",
		"param2": "value2",
	}
}
