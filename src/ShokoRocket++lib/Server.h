#pragma once
#include <string>
#include <vector>
#include "ServerConnection.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using std::string;
using std::vector;

class Server;

struct ServerThread
{
public:
	ServerThread(Server* _server){server_ = _server;}

	Server* server_;
	void operator()();
};

class Server
{
protected:
	string name_;
	boost::asio::io_service io_;
	boost::asio::io_service::work* work_;
	boost::thread* thread_;
	boost::asio::deadline_timer timer_;

	tcp::acceptor acceptor_;
	void StartConnection();
	void ConnectionAccepted(ServerConnection* _connection, boost::system::error_code ec);

	vector<ServerConnection*> connections_;
	bool closing_;
public:
	Server(void);
	~Server(void);

	void SetName(string _name){name_ = _name;}
	string GetName(){return name_;}

	vector<string> GetPlayers(){return vector<string>();}

	bool Tick();
	void PeriodicTidyup(boost::system::error_code _error);
};
