#include "Server.h"
#include <boost/bind.hpp>
#include "Logger.h"

Server::Server(void) : io_(), acceptor_(io_, tcp::endpoint(tcp::v4(), 9020)), work_(io_)
{
}

Server::~Server(void)
{
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		delete *it;
	}
}

void Server::StartConnection()
{
	//Starts listening for a new connection
	ServerConnection* connection = new ServerConnection(io_);
	connections_.push_back(connection);

	//Start listening for connection
	acceptor_.async_accept(connection->GetSocket(),
		                   boost::bind(&Server::ConnectionAccepted, 
						   this, 
						   connection, 
						   boost::asio::placeholders::error));
}

void Server::ConnectionAccepted(ServerConnection* _connection, boost::system::error_code ec)
{
	if(!ec)
	{
		_connection->Start();
		StartConnection();
	}
	else
	{
		Logger::DiagnosticOut() << "Error during connection, " << ec.message() << "\n";
	}
}

void Server::Tick()
{
	vector<ServerConnection*> dead_connections;
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		if((*it)->GetError())
			dead_connections.push_back(*it);
	}
	for(vector<ServerConnection*>::iterator it = dead_connections.begin(); it != dead_connections.end(); ++it)
	{
		delete *it;
		connections_.erase(std::remove(connections_.begin(), connections_.end(), *it), connections_.end());
	}
}
