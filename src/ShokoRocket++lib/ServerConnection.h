#pragma once
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::shared_ptr;

typedef boost::shared_ptr<boost::array<char, 512> > SBuffer;

class ServerConnection
{
protected:
	tcp::socket socket_;
	io_service& io_service_;
	boost::system::error_code error_;
	boost::array<char, 512> read_buffer;
public:
	ServerConnection(io_service& _io_service);
	~ServerConnection(void);

	tcp::socket& GetSocket(){return socket_;}
	boost::system::error_code& GetError(){return error_;}

	//Called when first connected
	void Start();
	void WriteFinished(boost::system::error_code error, SBuffer _buffer);

	void ReadHeaderFinished(boost::system::error_code error);
	void ReadBodyFinished(boost::system::error_code error);
};
