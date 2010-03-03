#include "Server.h"
#include <boost/bind.hpp>
#include "Logger.h"

void ServerThread::operator()()
{
	bool finished = false;
	while(!finished)
	{
		finished = server_->Tick();
	}
}

Server::Server(void) : io_(), acceptor_(io_, tcp::endpoint(tcp::v4(), 9020)),
					    timer_(io_, boost::posix_time::milliseconds(100)), closing_(false)
{
	players_count_ = 0;
	thread_ = new boost::thread(ServerThread(this));
	timer_.async_wait(boost::bind(&Server::PeriodicTidyup, this, boost::asio::placeholders::error));
	work_ = new boost::asio::io_service::work(io_);
	StartConnection();
}

Server::~Server(void)
{
	closing_ = true;
	delete work_;
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		delete *it;
	}
	acceptor_.close();
	//Should rejoin as all connections will throw errors
	thread_->join();
	delete thread_;
}

void Server::StartConnection()
{
	//Starts listening for a new connection
	ServerConnection* connection = new ServerConnection(io_, this, players_count_);
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
		Logger::DiagnosticOut() << "Server: Connection accepted\n";
		_connection->Start();
		StartConnection();
		players_count_++;
	}
	else
	{
		Logger::DiagnosticOut() << "Error during connection, " << ec.message() << "\n";
	}
}

void Server::PeriodicTidyup(boost::system::error_code _error)
{
	Logger::DiagnosticOut() << "Periodic tidyup at server\n";
	//TODO use a predicate here
	vector<ServerConnection*> dead_connections;
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		//if((*it)->GetError())
		//	dead_connections.push_back(*it);
	}
	for(vector<ServerConnection*>::iterator it = dead_connections.begin(); it != dead_connections.end(); ++it)
	{
		delete *it;
		connections_.erase(std::remove(connections_.begin(), connections_.end(), *it), connections_.end());
	}
	//Schedule another cleanup in 100ms
	if(!closing_)
	{
		timer_.expires_from_now(boost::posix_time::milliseconds(100));	
		timer_.async_wait(boost::bind(&Server::PeriodicTidyup, this, boost::asio::placeholders::error));
	}
}

bool Server::Tick()
{
	io_.run();

	return closing_;
}

void Server::HandleOpcode(int _player_id, Opcodes::ClientOpcode* _opcode)
{
	if(opcodes_.size() <= _player_id)
	{
		opcodes_.push_back(vector<Opcodes::ClientOpcode*>());
	}
	opcodes_[_player_id].push_back(_opcode);
}

vector<vector<Opcodes::ClientOpcode*> > Server::GetOpcodes()
{
	vector<vector<Opcodes::ClientOpcode*> > opcodes_copy = opcodes_;
	opcodes_.clear();
	return opcodes_copy;
}
