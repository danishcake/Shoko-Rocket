#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "Opcodes.h"

using std::string;
using boost::asio::ip::tcp;
using std::vector;

typedef boost::array<char, 512> CBuffer;
typedef boost::shared_ptr<CBuffer> CBuffer_ptr;

namespace ClientState
{
	enum Enum
	{
		NotConnected, Connecting, Connected
	};
}

class Client;

struct ClientThread
{
public:
	Client* client_;
	void operator()();

	ClientThread(Client* _client){client_ = _client;}

};

/* Represents a client that interacts with the server
 * Collects opcodes and puts them in a vector to be processed
 */
class Client
{
protected:
	string name_;
	ClientState::Enum state_;
	std::string host;
	unsigned short port;

	boost::asio::io_service io_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	boost::system::error_code error_;

	boost::thread* thread_;
	boost::mutex mutex_;

	void ConnectHandler(const boost::system::error_code& error);
	void ReadHeaderFinished(boost::system::error_code error, CBuffer_ptr _read_buffer);
	void ReadBodyFinished(boost::system::error_code error, CBuffer_ptr _read_buffer);
	void ReadLevelFinished(boost::system::error_code error, boost::shared_ptr<std::vector<char> > _level_data, std::string _filename);
	void WriteFinished(boost::system::error_code error, boost::shared_ptr<Opcodes::ClientOpcode> _data);

	bool closing_;
	vector<Opcodes::ServerOpcode*> opcodes_;
	Opcodes::ServerOpcode* server_opcode_;

public:
	Client(void);
	~Client(void);

	void SetName(string _name){name_ = _name;}
	string GetName(){return name_;}

	ClientState::Enum GetState(){return state_;}

	void Connect(std::string _host, unsigned short _port);
	bool Tick();

	vector<Opcodes::ServerOpcode*> GetOpcodes();
	void SendOpcode(Opcodes::ClientOpcode* _opcode);

};
