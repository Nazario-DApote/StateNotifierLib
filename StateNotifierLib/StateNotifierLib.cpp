// StateNotifierLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <queue>
#include <excpt.h>
#include <algorithm>
#include <utility>
#include <memory>
#include "StateNotifierLib.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/Exception.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Timestamp;
using Poco::Thread;
using Poco::Util::HelpFormatter;
using Poco::Mutex;
using Poco::ScopedLock;
using Poco::JSON::Object;
using Poco::Net::ConnectionRefusedException;

class CStateNotifierLibPimpl : public Poco::Runnable
{
private:
	int _id;
	WebSocket* _psock;
	Mutex _mutexQueue;
	std::queue<std::string> _workQueue;
	bool _stop;
	int _instance;
	std::string _processName;
	std::string _host;
	int _port;
	bool _connected;

public:
	CStateNotifierLibPimpl() :_id(0), _psock(NULL), _connected(false) {}
	CStateNotifierLibPimpl(int n) :_id(n), _psock(NULL), _connected(false) {}
	~CStateNotifierLibPimpl()
	{
		// close ws
		stop();
	};

	bool getConnected()
	{
		return _connected;
	}

	void addQueue(std::string msg)
	{
		if (!_stop)
		{
			Poco::ScopedLock<Mutex> lock(_mutexQueue);
			_workQueue.push(msg);
		}
	}

	void clearQueue()
	{
		// clear queue
		Poco::ScopedLock<Mutex> lock(_mutexQueue);
		std::queue<std::string> empty;
		std::swap(_workQueue, empty);
	}

	bool init(std::string processName, int instance, std::string host, int port)
	{
		if(!_connected)
		{
			_processName = processName;
			_instance = instance;
			_host = host;
			_port = port;

			HTTPClientSession cs(_host, _port);
			HTTPRequest request(HTTPRequest::HTTP_GET, "/ws", HTTPMessage::HTTP_1_1);
			//request.set("origin", "http://www.websocket.org");
			HTTPResponse response;
			try
			{
				if(_psock != NULL)
				{
					_psock->shutdown();
					_connected = false;
					delete _psock;
				}

				_psock = new WebSocket(cs, request, response);
				_connected = true;
			}
			catch(WebSocketException& e)
			{
				std::cerr << e.displayText() << std::endl;
				_connected = false;
			}
		}

		return _connected;
	}

	void stop()
	{
		_stop = true;
		clearQueue();
		_psock->shutdown();
		_connected = false;
		delete _psock;
	}

	virtual void run()
	{
		_stop = false;

		while (!_stop)
		{
			while (!_workQueue.empty())
			{
				Poco::ScopedLock<Mutex> lock(_mutexQueue);
				try
				{
					auto msg = _workQueue.front(); // pick
					const char *testStr = msg.c_str();
					int len = _psock->sendFrame(testStr, strlen(testStr), WebSocket::FRAME_TEXT);
					std::cout << "Sent bytes " << len << std::endl;
					int flags = 0;
					_workQueue.pop();
				}
				catch (WebSocketException& e)
				{
					std::cout << "WebSocketException: " << e.what();
				}
				catch (std::exception &e)
				{
					std::cout << "Exception: " << e.what();
				}
			}

			Thread::sleep(100);
		}
	}
};

// This is an example of an exported variable
//STATENOTIFIERLIB_API int nStateNotifierLib=0;

// This is an example of an exported function.
//STATENOTIFIERLIB_API int fnStateNotifierLib(void)
//{
//	return 42;
//}

// This is the constructor of a class that has been exported.
// see StateNotifierLib.h for the class definition
CStateNotifierLib::CStateNotifierLib()
{
	_pimpl = std::unique_ptr<CStateNotifierLibPimpl>(new CStateNotifierLibPimpl());
	return;
}

bool CStateNotifierLib::Init(std::string processName, int instance, std::string host, int port)
{
	auto res = _pimpl->init(processName, instance, host, port);
	if(res)
		Poco::ThreadPool::defaultPool().start(*_pimpl.get()); // start working thread
	return res;
}

void build_object(Poco::JSON::Object * const result, std::string processName, int instance, std::string sequence, std::string stateName, std::map<std::string, std::string> params)
{
	// smart pointer, so don't worry about cleaning up
	result->set("process", processName);
	result->set("instance", instance);

	auto inner = new Poco::JSON::Object;
	for (std::map<std::string, std::string>::iterator it = params.begin(); it != params.end(); ++it)
	{
		inner->set(it->first, it->second);
	}

	std::string key = "parameters";
	result->set(key, inner);
	printf("isObject: %i\n", result->isObject(key)); // true
}

void CStateNotifierLib::SendJson(std::string message)
{
	_pimpl->addQueue(message);
}

void CStateNotifierLib::SendStatus(std::string sequence, std::string stateName, std::map<std::string, std::string> params)
{
	auto json = new Poco::JSON::Object;
	// todo
	build_object(json, "", 0, sequence, stateName, params);
	std::ostringstream os;
	std::cout << "before stringlify.." << std::endl;
	json->stringify(os, 1);
	std::cout << os.str() << std::endl;
	std::string s = os.str();
	_pimpl->addQueue(s);
	delete json;
}

void CStateNotifierLib::SendEvent(std::string sequence, std::string eventName, std::map<std::string, std::string> params)
{
}

CStateNotifierLib::~CStateNotifierLib()
{
	_pimpl.release();

	// wait threads finish
	Poco::ThreadPool::defaultPool().joinAll();
}