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
					    timer_(io_, boost::posix_time::milliseconds(100)), 
						start_timer_(io_, boost::posix_time::milliseconds(1000)), 
						closing_(false)
{
	players_count_ = 0;
	required_players_ = 2;
	current_time_ = 0;
	max_players_ = 8;
	start_counter_ = 0;
	thread_ = new boost::thread(ServerThread(this));
	timer_.async_wait(boost::bind(&Server::PeriodicTidyup, this, boost::asio::placeholders::error));
	work_ = new boost::asio::io_service::work(io_);
	StartConnection();
	ingame_ = false;
	level_ = "Multiplayer/Multiplayer 1.Level";
}

Server::~Server(void)
{
	Logger::DiagnosticOut() << "Server: Destructor started\n";
	closing_ = true;
	mutex_.lock();
	delete work_;
	//Call close, giving handlers opportunity to complete on a valid object
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		(*it)->Close();
	}

	connections_.clear();
	acceptor_.close();
	timer_.cancel();
	start_timer_.cancel();

	//Should rejoin as all connections will throw errors
	thread_->join();

	delete thread_;
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		delete *it;
	}
	mutex_.unlock();
	Logger::DiagnosticOut() << "Server: Destructor finished\n";
}

void Server::StartConnection()
{
	//Starts listening for a new connection
	//Find first empty slot
	int last_id = -1;
	int empty_id = 0;

	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		empty_id = last_id + 1;
		if((*it)->GetPlayerID() != last_id + 1)
		{
			break;
		} else
		{
			empty_id++;
		}
		last_id++;
	}
	

	ServerConnection* connection = new ServerConnection(io_, this, empty_id);
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
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(!ec)
		{
			Logger::DiagnosticOut() << "Server: Connection accepted\n";
			if(players_count_ < max_players_)
			{
				_connection->Start();
				//Send names
				for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
				{
					if((*it)->GetPlayerNameSet())
					{
						Opcodes::ReadyState* rs = new Opcodes::ReadyState((*it)->GetReady(), (*it)->GetPlayerID());
						_connection->SendOpcode(rs);
						Opcodes::PlayerName* pn = new Opcodes::PlayerName((*it)->GetPlayerName(), (*it)->GetPlayerID());
						pn->time_ = current_time_;
						_connection->SendOpcode(pn);
					}
				}
				players_count_++;
				StartConnection();
			} else
			{ //Too many players, immediately send a kick message
				_connection->SendOpcode(new Opcodes::KickClient("Sorry, server full"));
			}

		}
		else
		{
			Logger::DiagnosticOut() << "Error during connection, " << ec.message() << "\n";
		}
		mutex_.unlock();
	} else Logger::DiagnosticOut() << "Server: Unable to acquire mutex, must be shutting down\n";
}

void Server::PeriodicTidyup(boost::system::error_code _error)
{
	if(mutex_.timed_lock(boost::posix_time::microseconds(100)))
	{
		//TODO use a predicate here
		vector<ServerConnection*> dead_connections;
		vector<int> disconnected_client_ids;
		for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
		{
			if((*it)->GetError())
				dead_connections.push_back(*it);
		}
		for(vector<ServerConnection*>::iterator it = dead_connections.begin(); it != dead_connections.end(); ++it)
		{
			players_count_--;
			disconnected_client_ids.push_back((*it)->GetPlayerID());
			delete *it;
			connections_.erase(std::remove(connections_.begin(), connections_.end(), *it), connections_.end());
		}
		for(vector<int>::iterator it = disconnected_client_ids.begin(); it != disconnected_client_ids.end(); ++it)
		{
			unsigned char pid = *it;
			Logger::DiagnosticOut() << "Sending disconnection message for client " << pid << "\n";
			SendOpcodeToAll(new Opcodes::ClientDisconnection(pid));
		}

		if(players_count_ >= required_players_)
		{
			int ready_count = 0;
			for(std::vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
			{
				if((*it)->GetReady()) ready_count++;
			}
			if(ready_count == players_count_ && !ingame_ && start_counter_ == 0)
			{
				start_timer_.async_wait(boost::bind(&Server::StartGameCallback, this, boost::asio::placeholders::error));
				start_counter_ = 5;
				SendOpcodeToAll(new Opcodes::ChatMessage("Starting in 5", Opcodes::ChatMessage::SENDER_SERVER));
			}
		}

		//Schedule another cleanup in 100ms
		if(!closing_)
		{
			timer_.expires_from_now(boost::posix_time::milliseconds(100));	
			timer_.async_wait(boost::bind(&Server::PeriodicTidyup, this, boost::asio::placeholders::error));
		}

		mutex_.unlock();
	} else Logger::DiagnosticOut() << "Server: Unable to acquire mutex, must be shutting down\n";
}

bool Server::Tick()
{
	io_.run();

	return closing_;
}

void Server::HandleOpcode(ServerConnection* _connection, int _player_id, Opcodes::ClientOpcode* _opcode)
{
	//No need to lock mutex as will already be locked by calling method
	while(opcodes_.size() <= _player_id)
	{
		opcodes_.push_back(vector<Opcodes::ClientOpcode*>());
	}
	opcodes_[_player_id].push_back(_opcode);

	//Handle some basic opcodes common to Lobby and game, eg chat, name changes
	switch(_opcode->opcode_)
	{
	case Opcodes::SendChatMessage::OPCODE:
		{
			Opcodes::SendChatMessage* client_msg = static_cast<Opcodes::SendChatMessage*>(_opcode);
			Opcodes::ChatMessage* msg = new Opcodes::ChatMessage(client_msg->message_, _player_id);
			SendOpcodeToAll(msg);
		}
		break;
	case Opcodes::SetName::OPCODE:
		{
			Opcodes::SetName* client_setname = static_cast<Opcodes::SetName*>(_opcode);
			std::string newname = client_setname->name_;

			for(int i = 0; i < 99; i++)
			{
				bool version_ok = true;
				std::string name = newname;
				if(i > 0) name = name + boost::lexical_cast<std::string, int>(i);
				//Find a unique name - 
				for(std::vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
				{
					if(*it == _connection || !(*it)->GetConnected())
						continue;
					if((*it)->GetPlayerName() == name)
					{
						version_ok = false;
					}
				}
				if(version_ok)
				{
					newname = name;
					break;
				}
			}
			Opcodes::PlayerName* msg = new Opcodes::PlayerName(newname, _player_id);
			SendOpcodeToAll(msg);
			_connection->SetPlayerName(newname);
		}
		break;
	case Opcodes::SetReady::OPCODE:
		{
			Opcodes::SetReady* client_ready = static_cast<Opcodes::SetReady*>(_opcode);
			if(client_ready->ready_)
			{
				_connection->SetReady(true);
				SendOpcodeToAll(new Opcodes::ReadyState(true, _player_id));
				SendOpcodeToAll(new Opcodes::ChatMessage("Ready!", _player_id));
			} else
			{
				_connection->SetReady(false);
				SendOpcodeToAll(new Opcodes::ReadyState(false, _player_id));
				SendOpcodeToAll(new Opcodes::ChatMessage("Not ready!", _player_id));
			}
		}
		break;
	case Opcodes::RequestDownload::OPCODE:
		{
			Opcodes::RequestDownload* download_req = static_cast<Opcodes::RequestDownload*>(_opcode);
			_connection->SendLevelToClient(download_req->level_);
		}
		break;
	}
}

vector<vector<Opcodes::ClientOpcode*> > Server::GetOpcodes()
{
	//Prevent vector being changed by background thread while I work with it
	mutex_.lock();
	vector<vector<Opcodes::ClientOpcode*> > opcodes_copy = opcodes_;
	opcodes_.clear();
	mutex_.unlock();
	return opcodes_copy;
}

void Server::SendOpcodeToAll(Opcodes::ServerOpcode* _opcode)
{
	if(_opcode->time_ == 0)
	{
		_opcode->time_ = current_time_;
	}
	for(vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
	{
		if((*it)->GetConnected())
		{
			(*it)->SendOpcode(_opcode);
		}
	}
	delete _opcode;
}


void Server::StartGameCallback(boost::system::error_code _error_code)
{
	start_counter_--;
	if(start_counter_ == 0)
	{
		if(players_count_ >= required_players_)
		{
			ingame_ = true;
			SendOpcodeToAll(new Opcodes::ChatMessage("Starting game!", Opcodes::ChatMessage::SENDER_SERVER));
			SendOpcodeToAll(new Opcodes::StateTransition(Opcodes::StateTransition::STATE_GAME, level_));
		} else
		{
			SendOpcodeToAll(new Opcodes::ChatMessage("Start aborted due to players leaving", Opcodes::ChatMessage::SENDER_SERVER));
		}
		for(std::vector<ServerConnection*>::iterator it = connections_.begin(); it != connections_.end(); ++it)
		{
			(*it)->SetReady(false);
		}
	} else
	{
		start_timer_.expires_from_now(boost::posix_time::milliseconds(1000));
		start_timer_.async_wait(boost::bind(&Server::StartGameCallback, this, boost::asio::placeholders::error));
		SendOpcodeToAll(new Opcodes::ChatMessage("Starting in " + boost::lexical_cast<string, int>(start_counter_), Opcodes::ChatMessage::SENDER_SERVER));
	}
}
