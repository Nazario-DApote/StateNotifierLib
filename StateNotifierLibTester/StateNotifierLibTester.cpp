// StateNotifierLibTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StateNotifierLib.h"
#include <conio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <Windows.h>

std::string procName = "thisProc";
int instance = 0;

void OnConnected()
{
	printf("\nConnected to the server via TCP\\IP: localhost:1466");
}

void OnDisconnected()
{
	printf("\nServer disconnected!");
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::map<std::string, std::string> mp;
	mp.insert(std::pair<std::string, std::string>("param1", "value1"));
	mp.insert(std::pair<std::string, std::string>("param2", "value2"));

	auto stdnotif = new CStateNotifierLib();
	stdnotif->setCallbackOnConnect(std::bind(OnConnected));
	stdnotif->setCallbackOnDisconnect(std::bind(OnDisconnected));

	if (stdnotif->Init(procName, instance, "localhost", 1466))
	{
		while (true)
		{
			printf("\n Press 'S' to send state");
			printf("\n Press 'Q' to exit");

			int c = _getch();

			if (c == 's')
			{
				stdnotif->EnterStatus("Seq1", "begin send", mp);
				printf("\n");
			}
			else if (c == 'q')
			{
				exit(0);
			}
		}

	}
	else
	{
		printf("\nCannot connect to the server\n");
		system("pause");
	}

	return 0;
}

