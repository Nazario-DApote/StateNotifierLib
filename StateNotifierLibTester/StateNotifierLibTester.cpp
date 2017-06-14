// StateNotifierLibTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StateNotifierLib.h"
#include <conio.h>
#include <string.h>
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	auto stdnotif = new CStateNotifierLib();
	stdnotif->Init("thisProc", 0, "localhost", 5000);
	stdnotif->SendJson("ciao");

	std::cout << "Press any key to exit ..." << std::endl;
	_kbhit();
	return 0;
}

