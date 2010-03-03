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
}

class ServerConnection
{
protected:
	tcp::socket socket_;
	io_service& io_service_;
	boost::system::error_code error_;
	Server* server_;
	int player_id_;
	Opcodes::ClientOpcode* client_opcode_;

public:
	ServerConnection(io_service& _io_service, Server* _server, int _player_id);
	~ServerConnection(void);

	tcp::socket& GetSocket(){return socket_;}
	boost::system::error_code& GetError(){return error_;}

	//Called when first connected
	void Start();
	void WriteFinished(boost::system::error_code error, SBuffer _buffer);

	void ReadHeaderFinished(boost::system::error_code error, SBuffer _buffer);
	void ReadBodyFinished(boost::system::error_code error, SBuffer _buffer);
};
