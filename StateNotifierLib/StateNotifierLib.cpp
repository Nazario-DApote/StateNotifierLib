// StateNotifierLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <queue>
#include <excpt.h>
#include <algorithm>
#include <memory>
#include "StateNotifierLib.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/NetException.h"
#include "Poco/Format.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/Exception.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"

using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::Timestamp;
using Poco::Thread;
using Poco::ThreadPool;
using Poco::Runnable;
using Poco::Mutex;
using Poco::ScopedLock;
using Poco::JSON::Object;
using Poco::Net::SocketAddress;
using Poco::Net::StreamSocket;
using Poco::Net::SocketStream;
using Poco::Net::NetException;

using namespace std;

class CStateNotifierLibPimpl : public Runnable
{
private:
	int _id;
	unique_ptr<StreamSocket> _psock;
	Mutex _mutexQueue;
	queue<string> _workQueue;
	bool _stop;
	int _instance;
	string _processName;
	string _host;
	int _port;
	bool _connected;

public:
	string getProcess() { return _processName; }
	int getInstance() { return _instance; }

	CStateNotifierLibPimpl() :_id(0), _connected(false) {}
	CStateNotifierLibPimpl(int n) :_id(n), _connected(false) {}
	~CStateNotifierLibPimpl()
	{
		stop();
		_psock.release();
	};

	bool getConnected()
	{
		return _connected;
	}

	void addQueue(string msg)
	{
		if (!_stop)
		{
			ScopedLock<Mutex> lock(_mutexQueue);
			_workQueue.push(msg);
		}
	}

	/// clear queue
	void clearQueue()
	{
		ScopedLock<Mutex> lock(_mutexQueue);
		queue<string> empty;
		swap(_workQueue, empty);
	}

	bool init(const string& processName, int instance, const string& host, int port)
	{
		if(!_connected)
		{
			_processName = processName;
			_instance = instance;
			_host = host;
			_port = port;

			try
			{
				_psock.release();
				SocketAddress endpoint(_host, _port);
				unique_ptr<StreamSocket> socket_ptr(new StreamSocket(endpoint));
				_psock.swap(socket_ptr);
				_connected = true;
			}
			catch(NetException& e)
			{
				cerr << e.displayText() << endl;
				_connected = false;
			}
		}

		return _connected;
	}

	void stop()
	{
		_stop = true;
		clearQueue();
		// close socket
		_psock.get()->shutdown();
		_connected = false;
	}

	bool sendMsg(const string msg)
	{
		int len = msg.length();
		int bytesToWrite = len + sizeof(int);
		vector<char> buf(bytesToWrite);
		fill(buf.begin(), buf.end(), 0);
		auto buf_ptr = buf.data();
		memcpy(buf_ptr, static_cast<void*>(&len), sizeof(int)); // set message size in head
		memcpy(buf_ptr + sizeof(int), msg.c_str(), len);		// set message content
		int tmp = 0;
		try {
			while (bytesToWrite > tmp) {
				tmp += _psock.get()->sendBytes(buf_ptr + tmp, bytesToWrite - tmp);
			}
			return true;
		}
		catch (NetException error) {
#if _DEBUG
			cout << "recv failed (Error: " << error.displayText() << ')' << endl;
#endif
			return false;
		}
	}

	virtual void run()
	{
		_stop = false;

		while (!_stop)
		{
			while (!_workQueue.empty())
			{
				ScopedLock<Mutex> lock(_mutexQueue);
				try
				{
					auto msg = _workQueue.front(); // pick
					sendMsg(msg);
					_workQueue.pop();
				}
				catch (NetException& e)
				{
					cout << "NetException: " << e.what();
				}
				catch (exception &e)
				{
					cout << "Exception: " << e.what();
				}
			}

			Thread::sleep(100);
		}
	}
};

// This is the constructor of a class that has been exported.
// see StateNotifierLib.h for the class definition
CStateNotifierLib::CStateNotifierLib()
{
	_pimpl = unique_ptr<CStateNotifierLibPimpl>(new CStateNotifierLibPimpl());
	return;
}

bool CStateNotifierLib::Init(const string& processName, int instance, const string& host, int port)
{
	auto res = _pimpl->init(processName, instance, host, port);
	if(res)
		ThreadPool::defaultPool().start(*_pimpl.get()); // start working thread
	return res;
}

void build_state(Object * const result,
	const string& process,
	int instance,
	const string& sequence,
	const string& name,
	const string& type,
	const map<string, string>& params)
{
	Timestamp now;

	// smart pointer, so don't worry about cleaning up
	result->set("process", process);
	result->set("instance", instance);
	result->set("name", name);
	if(!sequence.empty())
		result->set("sequence", sequence);
	result->set("type", type);
	result->set("startTime", DateTimeFormatter::format(now, DateTimeFormat::ISO8601_FORMAT));

	Object inner;
	for (map<string, string>::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		inner.set(it->first, it->second);
	}

	result->set("parameters", inner);
}

void CStateNotifierLib::SendJson(string message)
{
	_pimpl->addQueue(message);
}

void CStateNotifierLib::EnterStatus(const string& sequence, const string& stateName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_state(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), sequence, stateName, "ENTERSTATE", params);
	ostringstream  os;
	json->stringify(os, 1);
	string s = os.str();
#ifdef _DEBUG
	cout << s << endl;
#endif
	_pimpl->addQueue(s);
}

void CStateNotifierLib::ExitStatus(const string& sequence, const string& stateName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_state(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), sequence, stateName, "EXITSTATE", params);
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
#ifdef _DEBUG
	cout << s << endl;
#endif
	_pimpl->addQueue(s);
}

void CStateNotifierLib::SendEvent(const string& sequence, const string& eventName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_state(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), NULL, eventName, "EVENT", params);
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
#ifdef _DEBUG
	cout << s << endl;
#endif
	_pimpl->addQueue(s);
}

CStateNotifierLib::~CStateNotifierLib()
{
	_pimpl.release();

	// wait threads finish
	ThreadPool::defaultPool().joinAll();
}