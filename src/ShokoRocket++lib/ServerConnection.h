#pragma once
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::shared_ptr;
class Server;

typedef boost::shared_ptr<boost::array<char, 512> > SBuffer;

namespace Opcodes
{
	struct ClientOpcode;
	struct ServerOpcode;
}

class ServerConnection
{
protected:
	tcp::socket socket_;
	io_service& io_service_;
	boost::system::error_code error_;
	Server* server_;
	int player_id_;
	std::string player_name_;
	bool connected_;
	bool player_name_set_;
	bool disconnecting_;
	bool ready_;
	Opcodes::ClientOpcode* client_opcode_;

public:
	ServerConnection(io_service& _io_service, Server* _server, int _player_id);
	~ServerConnection(void);

	tcp::socket& GetSocket(){return socket_;}
	boost::system::error_code& GetError(){return error_;}

	//Called when first connected
	void Start();
	void WriteFinished(boost::system::error_code error, SBuffer _buffer);
	void Close(){socket_.close();}
	void SendOpcode(Opcodes::ServerOpcode* _opcode);

	void ReadHeaderFinished(boost::system::error_code error, SBuffer _buffer);
	void ReadBodyFinished(boost::system::error_code error, SBuffer _buffer);

	bool GetConnected(){return connected_;}
	int GetPlayerID(){return player_id_;}

	void SetPlayerName(std::string _player_name){player_name_ = _player_name; player_name_set_ = true;}
	std::string GetPlayerName(){return player_name_;}
	bool GetPlayerNameSet(){return player_name_set_;}

	//Triggers the connection to close when write finished
	void SetDisconnecting(){disconnecting_ = true;}
	void SetReady(bool _ready){ready_ = _ready;}
	bool GetReady(){return ready_;}

	//Sends a level to a client
	void SendLevelToClient(std::string _level_name);

	void WriteLevelFinished(boost::system::error_code error, boost::shared_ptr<std::vector<char> > _data);
};
