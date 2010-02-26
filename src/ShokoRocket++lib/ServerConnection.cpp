#include "ServerConnection.h"
#include <boost/bind.hpp>

using std::string;

ServerConnection::ServerConnection(io_service& _io_service)
: io_service_(_io_service), socket_(_io_service)
{
	boost::asio::socket_base::non_blocking_io command(true);
	socket_.io_control(command);
}

ServerConnection::~ServerConnection(void)
{
	socket_.close();
}

void ServerConnection::Start()
{
	//Start an async write
	SBuffer send_buffer = SBuffer(new boost::array<char, 512>());
	memcpy(send_buffer->c_array(), "Hello", 5);

	socket_.async_send(boost::asio::buffer(*send_buffer, 5), boost::bind(&ServerConnection::WriteFinished, this, boost::asio::placeholders::error, send_buffer));
	//At same time expect data
	boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 1), boost::bind(&ServerConnection::ReadHeaderFinished, this, boost::asio::placeholders::error));
}

void ServerConnection::WriteFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(error == boost::asio::error::eof)
		std::cout << "Write finished, connection closed cleanly\n";
	else if(error)
		std::cout << "Error encountered writing: " << error.message() << "\n";
	else
		std::cout << "Write finished\n";
	if(error) error_ = error;
	
}

void ServerConnection::ReadHeaderFinished(boost::system::error_code error)
{
	if(error == boost::asio::error::eof)
		std::cout << "Header read OK, but client DC'd\n";
	else if(error)
		std::cout << "Error during header read: " << error.message() << "\n";
	else
	{
		std::cout << "Read header finished\n";
		boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 32), boost::bind(&ServerConnection::ReadBodyFinished, this, boost::asio::placeholders::error));
	}
	if(error) error_ = error;
}

void ServerConnection::ReadBodyFinished(boost::system::error_code error)
{
	if(error == boost::asio::error::eof)
		std::cout << "Body read OK, but client DC'd\n";
	else if(error)
		std::cout << "Error during body read: " << error.message() << "\n";
	else
	{
		std::cout << "Read body finished, looking for header again\n";
		boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 1), boost::bind(&ServerConnection::ReadHeaderFinished, this, boost::asio::placeholders::error));
	}
	if(error) error_ = error;	
}