#include "Client.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "Logger.h"


void ClientThread::operator()()
{
	bool finished = false;
	while(!finished)
	{
		finished = client_->Tick();
	}
}

Client::Client(void) : name_("CHU CHU"), io_(boost::asio::io_service()), socket_(io_), closing_(false)
{
	state_ = ClientState::NotConnected;
	thread_ = new boost::thread(ClientThread(this));
}

Client::~Client(void)
{
	Logger::DiagnosticOut() << "Client: Destructor started\n";
	closing_ = true;
	mutex_.lock();
	socket_.close();
	thread_->join();
	delete thread_;
	mutex_.unlock(); //Probably not necessary as being destroyed
	Logger::DiagnosticOut() << "Client: Destructor finished\n";
}

void Client::Connect(std::string _host, unsigned short _port)
{
	state_ = ClientState::Connecting;
	tcp::resolver resolver(io_);
	tcp::resolver_query query("localhost", "9020");
	endpoints_ = resolver.resolve(query);
	tcp::endpoint ep = *endpoints_++;
	socket_.async_connect(ep, boost::bind(&Client::ConnectHandler, this, boost::asio::placeholders::error));
	Logger::DiagnosticOut() << "Client: Attempting to connect to " << boost::lexical_cast<std::string, tcp::endpoint>(ep) << "\n";
}

bool Client::Tick()
{
	io_.run();

	return closing_;
}

void Client::ConnectHandler(const boost::system::error_code& error)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
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
				Logger::DiagnosticOut() << "Client: Attempting to connect to " << boost::lexical_cast<std::string, tcp::endpoint>(ep) << "\n";
			} else
			{
				Logger::DiagnosticOut() << "Client: Unable to connect to any of the endpoints\n";
				state_ = ClientState::NotConnected;
			}
		} else
		{
			Logger::DiagnosticOut() << "Client: Connected successfully\n";
			state_ = ClientState::Connected;
			boost::shared_ptr<Opcodes::ClientOpcode> opcode = boost::shared_ptr<Opcodes::ClientOpcode>(new Opcodes::SetName(name_));
			
			socket_.async_send(boost::asio::buffer((char*)opcode.get(), sizeof(Opcodes::SetName)), boost::bind(&Client::WriteFinished, this, boost::asio::placeholders::error, opcode));

			boost::asio::async_read(socket_, boost::asio::buffer(read_buffer, 1), boost::bind(&Client::ReadHeaderFinished, this, boost::asio::placeholders::error));
			
		}
		mutex_.unlock();
	} else Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}
void Client::ReadHeaderFinished(boost::system::error_code error)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
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
		if(error)
		{
			error_ = error;
			closing_ = true;
			state_ = ClientState::NotConnected;
		}
		mutex_.unlock();
	} else Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}

void Client::ReadBodyFinished(boost::system::error_code error)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
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
		if(error)
		{
			error_ = error;	
			closing_ = true;
			state_ = ClientState::NotConnected;
		}
		mutex_.unlock();
	} else Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}
void Client::WriteFinished(boost::system::error_code error, boost::shared_ptr<Opcodes::ClientOpcode> _data)
{
	mutex_.lock();
	Logger::DiagnosticOut() << "Finished writing from client\n";
	//Do nothing, but memory should now be freed
	mutex_.unlock();
}