#include "Client.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "Logger.h"

Client::Client(void) : name_("CHU CHU"), io_(boost::asio::io_service()), socket_(io_)
{
	state_ = ClientState::NotConnected;
}

Client::~Client(void)
{
}

void Client::Connect(std::string _host, unsigned short _port)
{
	state_ = ClientState::Connecting;
	tcp::resolver resolver(io_);
	tcp::resolver_query query("localhost", "9020");
	endpoints_ = resolver.resolve(query);
	tcp::endpoint ep = *endpoints_++;
	socket_.async_connect(ep, boost::bind(&Client::ConnectHandler, this, boost::asio::placeholders::error));
}

void Client::Tick()
{
	io_.run();
}

void Client::ConnectHandler(const boost::system::error_code& error)
{
	tcp::resolver::iterator end; // End marker.
	if(error)
	{
		if(endpoints_ != end)
		{
			tcp::endpoint ep = *endpoints_++;
			boost::system::error_code error;
			socket_.close();
			socket_.async_connect(ep, boost::bind(&Client::ConnectHandler, this, boost::asio::placeholders::error));			
		} else
		{
			Logger::DiagnosticOut() << "Unable to connect to any of the endpoints\n";
			state_ = ClientState::NotConnected;
		}
	} else
	{
		Logger::DiagnosticOut() << "Connected successfully\n";
		state_ = ClientState::Connected;
		boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 1), boost::bind(&Client::ReadHeaderFinished, this, boost::asio::placeholders::error));
	}
}
void Client::ReadHeaderFinished(boost::system::error_code error)
{
	if(error == boost::asio::error::eof)
		std::cout << "Client: Header read OK, but server DC'd\n";
	else if(error)
		std::cout << "Client: Error during header read: " << error.message() << "\n";
	else
	{
		std::cout << "Client: Read header finished\n";
		boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 32), boost::bind(&Client::ReadBodyFinished, this, boost::asio::placeholders::error));
	}
	if(error) error_ = error;
}

void Client::ReadBodyFinished(boost::system::error_code error)
{
	if(error == boost::asio::error::eof)
		std::cout << "Client: Body read OK, but client DC'd\n";
	else if(error)
		std::cout << "Client: Error during body read: " << error.message() << "\n";
	else
	{
		std::cout << "Client: Read body finished, looking for header again\n";
		boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 1), boost::bind(&Client::ReadHeaderFinished, this, boost::asio::placeholders::error));
	}
	if(error) error_ = error;	
}