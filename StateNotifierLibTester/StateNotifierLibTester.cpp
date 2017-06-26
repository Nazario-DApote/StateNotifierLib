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
#include "Poco\Util\Application.h"
#include "Poco\Util\Option.h"
#include "Poco\Util\HelpFormatter.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

class StateNotifierTester : public Application
{
public:
	StateNotifierTester():
		_helpRequested(false),
		_procName("thisProc"),
		_instance(0) {
	}

private:
	std::string _procName;
	int _instance;
	bool _helpRequested;

protected:

	// override Application virtual method
	void defineOptions(OptionSet& options)
	{
		Application::defineOptions(options);

		options.addOption(
			Option("help", "h", "display argument help information")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<StateNotifierTester>(this, &StateNotifierTester::handleHelp)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("StateNotifierTester <OPTIONS> <Process>Name");
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setAutoIndent();
		helpFormatter.setHeader("A tester for StateNotifierLib library");
		helpFormatter.setFooter("Nazario D'Apote");
		helpFormatter.format(std::cout);
		stopOptionsProcessing();
		_helpRequested = true;
	}

	virtual int main(const std::vector<std::string> &args)
	{
		if (!_helpRequested)
		{
			if (args.size() > 0)
			{
				_procName = args[0];
			}
			std::map<std::string, std::string> mp;
			mp.insert(std::pair<std::string, std::string>("param1", "value1"));
			mp.insert(std::pair<std::string, std::string>("param2", "value2"));

			auto stdnotif = new CStateNotifierLib();
			stdnotif->setCallbackOnConnect(std::bind(&StateNotifierTester::OnConnected, this));
			stdnotif->setCallbackOnDisconnect(std::bind(&StateNotifierTester::OnDisconnected, this));
			stdnotif->setCallbackOnError(std::bind(&StateNotifierTester::OnError, this, std::placeholders::_1));

			if (stdnotif->Init(_procName, _instance, "localhost", 1466))
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
		}

		return EXIT_OK;
	}

	void OnConnected()
	{
		printf("\nConnected to the server via TCP\\IP: localhost:1466");
	}

	void OnDisconnected()
	{
		printf("\nServer disconnected!");
	}

	void OnError(const std::string& errMsg)
	{
#ifdef _DEBUG
		std::cerr << std::endl << errMsg << std::endl;
#endif
	}

};

// entry point
POCO_APP_MAIN(StateNotifierTester);

