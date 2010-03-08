#include "ServerConnection.h"
#include <boost/bind.hpp>
#include "Opcodes.h"
#include "Logger.h"
#include "ServerConnection.h"
#include "Server.h"

using std::string;

ServerConnection::ServerConnection(io_service& _io_service, Server* _server, int _player_id)
: io_service_(_io_service), socket_(_io_service), player_id_(_player_id)
{
	server_ = _server;
	//boost::asio::socket_base::non_blocking_io command(true);
	//socket_.io_control(command);
	//Does this actually do anything?
	client_opcode_ = NULL;
	connected_ = false;
}

ServerConnection::~ServerConnection(void)
{
	connected_ = false;
	socket_.close();
}

void ServerConnection::Start()
{
	connected_ = true;
	//Start an async write
	SBuffer send_buffer = SBuffer(new boost::array<char, 512>());
	Opcodes::ChatMessage cm("Welcome to ShokoRocket", Opcodes::ChatMessage::SENDER_SERVER);
	memcpy(send_buffer->c_array(), &cm, sizeof(cm));

	
	socket_.async_send(boost::asio::buffer(*send_buffer, sizeof(Opcodes::ChatMessage)), boost::bind(&ServerConnection::WriteFinished, this, boost::asio::placeholders::error, send_buffer));
	//At same time expect data
	SBuffer read_buffer = SBuffer(new boost::array<char, 512>());
	boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, Opcodes::ClientOpcode::HEADERSIZE), boost::bind(&ServerConnection::ReadHeaderFinished, this, boost::asio::placeholders::error, read_buffer));
}

void ServerConnection::WriteFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(server_->GetMutex().timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
			Logger::DiagnosticOut() << "Server: Write finished, connection closed cleanly\n";
		else if(error)
			Logger::DiagnosticOut() << "Server: Error encountered writing: " << error.message() << "\n";
		else
			Logger::DiagnosticOut() << "Server: Write finished\n";
		if(error) error_ = error;
		server_->GetMutex().unlock();
	} else Logger::DiagnosticOut() << "ServerConnection::WriteFinished: Unable to lock, probably being shutdown\n";
}

void ServerConnection::ReadHeaderFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(server_->GetMutex().timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
			Logger::DiagnosticOut() << "Server: Header read OK, but client DC'd\n";
		else if(error)
			Logger::DiagnosticOut() << "Server: Error during header read: " << error.message() << "\n";
		else
		{
			
			int body_size = Opcodes::GetBodySize((Opcodes::ClientOpcode*)_buffer->c_array());
			Logger::DiagnosticOut() << "Server: Read header finished, looking for body size " << body_size << "\n";
			client_opcode_ = Opcodes::GetClientOpcode((Opcodes::ClientOpcode*)_buffer->c_array());
			SBuffer read_buffer = SBuffer(new boost::array<char, 512>());
			boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, body_size), boost::bind(&ServerConnection::ReadBodyFinished, this, boost::asio::placeholders::error, read_buffer));
		}
		if(error) error_ = error;
		server_->GetMutex().unlock();
	} else Logger::DiagnosticOut() << "ServerConnection::ReadHeaderFinished: Unable to lock, probably being shutdown\n";
}

void ServerConnection::ReadBodyFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(server_->GetMutex().timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
			Logger::DiagnosticOut() << "Server: Body read OK, but client DC'd\n";
		else if(error)
			Logger::DiagnosticOut() << "Server: Error during body read: " << error.message() << "\n";
		else
		{
			//Handle opcode creation

			memcpy(((char*)client_opcode_) + Opcodes::ClientOpcode::HEADERSIZE, _buffer->c_array(),  Opcodes::GetBodySize(client_opcode_));
			//Pass newly created opcode to server, which is then responsible for freeing it
			server_->HandleOpcode(player_id_, client_opcode_);

			Logger::DiagnosticOut() << "Server: Read body finished, looking for header again\n";
			SBuffer read_buffer = SBuffer(new boost::array<char, 512>());
			boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, Opcodes::ClientOpcode::HEADERSIZE), boost::bind(&ServerConnection::ReadHeaderFinished, this, boost::asio::placeholders::error, read_buffer));
		}
		if(error) error_ = error;
		client_opcode_ = NULL;
		server_->GetMutex().unlock();
	} else Logger::DiagnosticOut() << "ServerConnection::ReadBodyFinished: Unable to lock, probably being shutdown\n";
}

void ServerConnection::SendOpcode(Opcodes::ServerOpcode* _opcode)
{
	SBuffer send_buffer = SBuffer(new boost::array<char, 512>());
	
	int bytes_to_send = Opcodes::GetBodySize(_opcode) + Opcodes::ServerOpcode::HEADERSIZE;
	memcpy(send_buffer->c_array(), _opcode, bytes_to_send);

	socket_.async_send(boost::asio::buffer(*send_buffer, bytes_to_send), boost::bind(&ServerConnection::WriteFinished, this, boost::asio::placeholders::error, send_buffer));
}