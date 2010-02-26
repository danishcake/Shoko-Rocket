#pragma once
#include <string>
#include <vector>
#include "ServerConnection.h"
#include <boost/asio.hpp>

using std::string;
using std::vector;

class Server
{
protected:
	string name_;
	boost::asio::io_service io_;
	boost::asio::io_service::work work_;

	tcp::acceptor acceptor_;
	void StartConnection();
	void ConnectionAccepted(ServerConnection* _connection, boost::system::error_code ec);

	vector<ServerConnection*> connections_;
public:
	Server(void);
	~Server(void);

	void SetName(string _name){name_ = _name;}
	string GetName(){return name_;}

	vector<string> GetPlayers(){return vector<string>();}

	void Tick();
};
