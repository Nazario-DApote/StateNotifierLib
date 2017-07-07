// StateNotifierLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "StateNotifierLib.h"

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
using Poco::Exception;
using Poco::Net::NetException;
using Poco::Buffer;
using Poco::format;

class CStateNotifierLib::CStateNotifierLibPimpl : public Runnable
{
private:
	unique_ptr<StreamSocket> _psock;
	Mutex _mutexQueue;
	queue<string> _workQueue;
	bool _stop;
	int _instance;
	string _processName;
	string _host;
	int _port;
	connectionCallback _onConnect, _onDisconnect;
	logCallback _onError, _onInfo;

public:
	string getProcess() { return _processName; }
	int getInstance() { return _instance; }
	void setCallbackOnConnect(connectionCallback fnct) { _onConnect = fnct; }
	void setCallbackOnDisconnect(connectionCallback fnct) { _onDisconnect = fnct; }
	void setCallbackOnError(logCallback fnct) { _onError = fnct; }
	void setCallbackOnInfo(logCallback fnct) { _onInfo = fnct; }

	CStateNotifierLibPimpl() {}
	~CStateNotifierLibPimpl()
	{
		stop();
	};

	bool getConnected()
	{
		if (_psock.get() != nullptr)
		{
			try
			{
				bool part1 = _psock->poll(1000, StreamSocket::SELECT_READ);
				bool part2 = (_psock->available() == 0);
				if (part1 && part2)
					return false;
				else
					return true;
			}
			catch (NetException& e) {
				notifyError(e.message());
			}
		}

		return false;
	}

	void addQueue(string msg)
	{
		if (!_stop)
		{
			ScopedLock<Mutex> lock(_mutexQueue);
			_workQueue.push(msg);
		}
	}

	void limitWorkingQueue()
	{
		ScopedLock<Mutex> lock(_mutexQueue);
		// keep only last QUEUE_MAX_SIZE values
		auto QUEUE_MAX_SIZE = 100;
		if (_workQueue.size() > QUEUE_MAX_SIZE)
		{
			vector<string> vcopy(QUEUE_MAX_SIZE);
			for (auto i = QUEUE_MAX_SIZE - 1; i >= 0; --i)
			{
				vcopy[i] = std::move(_workQueue.front());
				_workQueue.pop();
			}
			clearQueue();
			for (auto it = vcopy.begin(); it != vcopy.end(); ++it)
			{
				_workQueue.push(std::move(*it));
			}
		}
	}

	void notifyConnected()
	{
		if (_onConnect != nullptr)
			_onConnect();
	}

	void notifyDisconnected()
	{
		if (_onDisconnect != nullptr)
			_onDisconnect();
	}

	void notifyError(const std::string& errMsg)
	{
		if (_onError != nullptr)
			_onError(errMsg);
	}

	void notifyInfo(const std::string& errMsg)
	{
		if (_onInfo != nullptr)
			_onInfo(errMsg);
	}

	void clearQueue()
	{
		ScopedLock<Mutex> lock(_mutexQueue);
		queue<string> empty;
		swap(_workQueue, empty);
	}

	bool connect()
	{
		try
		{
			SocketAddress endpoint(_host, _port);
			unique_ptr<StreamSocket> socket_ptr(new StreamSocket(endpoint));
			_psock.swap(socket_ptr);
			socket_ptr.release();
			notifyConnected();
			return true;
		}
		catch (NetException& e)
		{
			notifyError(e.displayText());
		}

		return true;
	}

	bool init(const string& processName, int instance, const string& host, int port)
	{
		poco_assert(processName.length() > 0);
		poco_assert(instance >= 0);
		poco_assert(host.length() > 0);
		poco_assert(65536 > port && port >= 1024);

		_processName = processName;
		_instance = instance;
		_host = host;
		_port = port;

		if (!getConnected())
			return connect();

		return true;
	}

	void stop()
	{
		_stop = true;
		clearQueue();
		notifyDisconnected();
	}

	bool sendMsg(const string& msg)
	{
		if (getConnected())
		{
			int len = msg.length();
			int bytesToWrite = len + sizeof(int);
			Buffer<char> buf(bytesToWrite);
			fill(buf.begin(), buf.end(), 0);
			auto buf_ptr = buf.begin();
			memcpy(buf_ptr, static_cast<void*>(&len), sizeof(int)); // set message size in head
			memcpy(buf_ptr + sizeof(int), msg.c_str(), len);		// set message content
			int tmp = 0;
			try {
				while (bytesToWrite > tmp) {
					tmp += _psock->sendBytes(buf_ptr + tmp, bytesToWrite - tmp);
				}
				return true;
			}
			catch (NetException e) {
				notifyError(e.message());
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool exponentialBackoffConnectionRetry(int& nRetry)
	{
		// update retry number
		nRetry++;

		// calculate delay through the Exponential Back-off Algorithm
		// https://devcentral.f5.com/articles/implementing-the-exponential-backoff-algorithm-to-thwart-dictionary-attacks
		auto e = ((1 << nRetry) - 1) / 2;
		int maxDelay = 300; // 5min

		// limit max delay
		auto delay = e;
		if (e > maxDelay)
			delay = maxDelay;

		// Notify retry and wait
		notifyInfo(format("Server disconnected. Retrying in %d sec", delay));
		Thread::sleep(delay * 1000);

		// try to connect
		return connect();
	}

	virtual void run()
	{
		_stop = false;
		int nRetry = 0;
		bool disconnectionNotified = false;

		while (!_stop)
		{
			if (!getConnected())
			{
				limitWorkingQueue();

				if (!disconnectionNotified)
					notifyDisconnected();

				if( !exponentialBackoffConnectionRetry(nRetry) )
					continue;
			}

			nRetry = 0;
			disconnectionNotified = false;

			while (!_workQueue.empty())
			{
				try
				{
					ScopedLock<Mutex> lock(_mutexQueue);
					auto msg = _workQueue.front(); // pick
					sendMsg(msg);
					//finally([&]() { _workQueue.pop(); });
					_workQueue.pop(); // dequeue
				}
				catch (NetException& e)
				{
					notifyError(e.message());
					break;
				}
			}

			Thread::sleep(200);
		}
	}
};

// This is the constructor of a class that has been exported.
// see StateNotifierLib.h for the class definition
CStateNotifierLib::CStateNotifierLib()
{
	_pimpl = std::unique_ptr<CStateNotifierLibPimpl>(new CStateNotifierLib::CStateNotifierLibPimpl());
}

void build_json(Object * const result,
	const string& process,
	int instance,
	const string& sequence,
	const string& name,
	const string& type,
	const map<string, string>& params)
{
	poco_assert(process.length() > 0);
	poco_assert(instance >= 0);
	poco_assert(type.length() > 0);

	Timestamp now;

	// smart pointer, so don't worry about cleaning up
	result->set("process", process);
	result->set("instance", instance);
	result->set("name", name);
	if (!sequence.empty())
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

bool CStateNotifierLib::Init(const string& processName, int instance, const string& host, int port)
{
	auto res = _pimpl->init(processName, instance, host, port);
	if (res)
		ThreadPool::defaultPool().start(*_pimpl); // start working thread
	return res;
}

void CStateNotifierLib::EnterStatus(const string& sequence, const string& stateName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_json(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), sequence, stateName, "ENTERSTATE", params);
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
	_pimpl->notifyInfo(s);
	_pimpl->addQueue(s);
}

void CStateNotifierLib::ExitStatus(const string& sequence, const string& stateName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_json(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), sequence, stateName, "EXITSTATE", params);
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
	_pimpl->notifyInfo(s);
	_pimpl->addQueue(s);
}

void CStateNotifierLib::EventEmit(const string& sequence, const string& eventName, const std::string& to, const map<string, string>& params)
{
	poco_assert(to.length() > 0);
	auto json = auto_ptr<Object>(new Object);
	build_json(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), std::string(), eventName, "EVENT_EMIT", params);
	json->set("to", to);
	json->set("from", _pimpl->getProcess());
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
	_pimpl->notifyInfo(s);
	_pimpl->addQueue(s);
}

void CStateNotifierLib::EventRecv(const string& sequence, const string& eventName, const std::string& from, const map<string, string>& params)
{
	poco_assert(from.length() > 0);
	auto json = auto_ptr<Object>(new Object);
	build_json(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), std::string(), eventName, "EVENT_RECV", params);
	json->set("from", from);
	json->set("to", _pimpl->getProcess());
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
	_pimpl->notifyInfo(s);
	_pimpl->addQueue(s);
}

void CStateNotifierLib::Event(const string& sequence, const string& eventName, const map<string, string>& params)
{
	auto json = auto_ptr<Object>(new Object);
	build_json(json.get(), _pimpl->getProcess(), _pimpl->getInstance(), std::string(), eventName, "EVENT_EMIT", params);
	json->set("from", _pimpl->getProcess());
	json->set("to", _pimpl->getProcess());
	ostringstream os;
	json->stringify(os, 1);
	string s = os.str();
	_pimpl->notifyInfo(s);
	_pimpl->addQueue(s);
}

void CStateNotifierLib::setCallbackOnConnect(connectionCallback fnct)
{
	_pimpl->setCallbackOnConnect(fnct);
};

void CStateNotifierLib::setCallbackOnDisconnect(connectionCallback fnct)
{
	_pimpl->setCallbackOnDisconnect(fnct);
};

void CStateNotifierLib::setCallbackOnError(logCallback fnct)
{
	_pimpl->setCallbackOnError(fnct);
};

void CStateNotifierLib::setCallbackOnInfo(logCallback fnct)
{
	_pimpl->setCallbackOnInfo(fnct);
};

bool CStateNotifierLib::getConnected()
{
	return _pimpl->getConnected();
}

CStateNotifierLib::~CStateNotifierLib()
{
	_pimpl.release();

	// wait threads finish
	ThreadPool::defaultPool().joinAll();
}