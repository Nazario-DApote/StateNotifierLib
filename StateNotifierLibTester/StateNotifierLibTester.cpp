// StateNotifierLibTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StateNotifierLib.h"
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
#include "StateMachine.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::AutoPtr;
using Poco::Logger;
using Poco::FormattingChannel;
using Poco::PatternFormatter;
using Poco::WindowsColorConsoleChannel;
using Poco::NumberParser;

class StateNotifierTester : public Application
{
public:
	StateNotifierTester():
		_helpRequested(false),
		_procName("thisProc"),
		_instance(0),
		_logger (Logger::get("TestLogger")),
		_srvPort(1466),
		_srvHost("localhost") {

		this->setUnixOptions(true);

		AutoPtr<WindowsColorConsoleChannel> consoleChannel(new WindowsColorConsoleChannel);
		AutoPtr<PatternFormatter> pf(new PatternFormatter);
		pf->setProperty("pattern", "[%Y-%m-%d %H:%M:%S] %t");
		AutoPtr<FormattingChannel> pfChannel(new FormattingChannel(pf, consoleChannel));
		_logger.setChannel(pfChannel);
	}

private:
	std::string _procName;
	int _instance;
	bool _helpRequested;
	Logger& _logger;
	int _srvPort;
	std::string _srvHost;
	std::string _to;

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

		options.addOption(
			Option("host", "H", "set server address")
			.required(false)
			.repeatable(false)
			.argument("<hostname/ip>", true)
			.callback(OptionCallback<StateNotifierTester>(this, &StateNotifierTester::handleHost)));

		options.addOption(
			Option("port", "p", "set server port")
			.required(false)
			.repeatable(false)
			.argument("<portNbr>", true)
			.callback(OptionCallback<StateNotifierTester>(this, &StateNotifierTester::handlePort)));

		options.addOption(
			Option("to", "to", "event destination process")
			.required(false)
			.repeatable(false)
			.argument("<to>", true)
			.callback(OptionCallback<StateNotifierTester>(this, &StateNotifierTester::handleTo)));
	}

	void handleTo(const std::string& name, const std::string& value)
	{
		_to = value;
	}

	void handlePort(const std::string& name, const std::string& value)
	{
		int tmpValue;
		if (NumberParser::tryParse(value, tmpValue))
		{
			poco_assert(tmpValue > 0);
			StateNotifierTester::_srvPort = tmpValue;
		}
	}

	void handleHost(const std::string& name, const std::string& value)
	{
		_srvHost = value;
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setUnixStyle(true);
		helpFormatter.setCommand(commandName());
		helpFormatter.setHeader("A tester for StateNotifierLib library");
		helpFormatter.setAutoIndent();
		helpFormatter.setUsage("<OPTIONS> <ProcessName>");
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
			auto stdnotif = new CStateNotifierLib();
			//stdnotif->setCallbackOnConnect(std::bind(&StateNotifierTester::OnConnected, this));
			//stdnotif->setCallbackOnDisconnect(std::bind(&StateNotifierTester::OnDisconnected, this));
			//stdnotif->setCallbackOnError(std::bind(&StateNotifierTester::OnError, this, std::placeholders::_1));
			//stdnotif->setCallbackOnInfo(std::bind(&StateNotifierTester::OnInfo, this, std::placeholders::_1));
			stdnotif->setCallbackOnConnect([&]() { poco_information(_logger, Poco::format("Connected to the server via TCP\\IP: %s:%d", _srvHost, _srvPort)); });
			stdnotif->setCallbackOnDisconnect([&]() { poco_information(_logger, "Server disconnected!"); });
			stdnotif->setCallbackOnError([&](std::string errMsg) { poco_critical(_logger, errMsg); });
			stdnotif->setCallbackOnInfo([&](std::string infoMsg) { poco_information(_logger, infoMsg); });

			if (stdnotif->Init(_procName, _instance, _srvHost, _srvPort))
			{
				std::map<std::string, std::string> mp;
				mp.insert(std::pair<std::string, std::string>("param1", "value1"));
				mp.insert(std::pair<std::string, std::string>("param2", "value2"));

				SM::Top::Box box;
				box._stnotif = stdnotif;
				Macho::Machine<SM::Top> stateMachine;

				while (true)
				{
					poco_information(_logger, "Press 'S' to send state");
					poco_information(_logger, "Press 'Q' to exit");

					int c = _getch();

					if (c == 's' || c == 'S')
					{
						// notify new event received
						stdnotif->EventRecv("SM", "KeyPress", _procName, std::map<string, string>());
						// change state
						stateMachine->event();
						// notify new state
						stdnotif->EnterStatus("SM", string(stateMachine.currentState().name()), mp);
						if(!_to.empty())
							stdnotif->EventEmit("SM", "AUTHORIZED", _to, std::map<string, string>());
					}
					else if (c == 'q' || c == 'Q')
					{
						exit(0);
					}
				}
			}
			else
			{
				_logger.error("Cannot connect to the server");
				system("pause");
			}
		}

		return EXIT_OK;
	}

	//void OnConnected()
	//{
	//	 poco_information(_logger, Poco::format("Connected to the server via TCP\\IP: %s:%d", _srvHost, _srvPort));
	//}

	//void OnDisconnected()
	//{
	//	poco_information(_logger, "Server disconnected!");
	//}

	//void OnError(const std::string& errMsg)
	//{
	//	poco_critical(_logger, errMsg);
	//}

	//void OnInfo(const std::string& infoMsg)
	//{
	//	poco_information(_logger, infoMsg);
	//}

};

// entry point
POCO_APP_MAIN(StateNotifierTester);

