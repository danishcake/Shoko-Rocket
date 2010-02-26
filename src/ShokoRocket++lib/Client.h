#pragma once
#include <string>
#include <boost/asio.hpp>

using std::string;
using boost::asio::ip::tcp;

namespace ClientState
{
	enum Enum
	{
		NotConnected, Connecting, Connected
	};
}

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
	boost::array<char, 512> read_buffer;
	boost::system::error_code error_;

	void ConnectHandler(const boost::system::error_code& error);
	void ReadHeaderFinished(boost::system::error_code error);
	void ReadBodyFinished(boost::system::error_code error);
public:
	Client(void);
	~Client(void);

	void SetName(string _name){name_ = _name;}
	string GetName(){return name_;}

	ClientState::Enum GetState(){return state_;}

	void Connect(std::string _host, unsigned short _port);
	void Tick();
};
