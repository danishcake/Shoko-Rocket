#include "ServerConnection.h"
#include <boost/bind.hpp>
#include "Opcodes.h"
#include "Logger.h"
#include "ServerConnection.h"
#include "Server.h"
#include <boost/filesystem.hpp>

using std::string;

ServerConnection::ServerConnection(io_service& _io_service, Server* _server, int _player_id)
: io_service_(_io_service), socket_(_io_service), player_id_(_player_id)
{
	server_ = _server;
	player_name_ = "Unknown player";
	client_opcode_ = NULL;
	connected_ = false;
	player_name_set_ = false;
	disconnecting_ = false;
	ready_ = false;
	logging_ = false;
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
		if(logging_)
		{
			if(error == boost::asio::error::eof)
				Logger::DiagnosticOut() << "Server: Write finished, connection closed cleanly\n";
			else if(error)
				Logger::DiagnosticOut() << "Server: Error encountered writing: " << error.message() << "\n";
			else
				Logger::DiagnosticOut() << "Server: Write finished\n";
		}
		if(error) error_ = error;
		server_->GetMutex().unlock();
	} else if(logging_) Logger::DiagnosticOut() << "ServerConnection::WriteFinished: Unable to lock, probably being shutdown\n";
	if(disconnecting_)
	{
		socket_.close();
		connected_ = false;
		error_ = boost::asio::error::eof;
	}
}

void ServerConnection::ReadHeaderFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(server_->GetMutex().timed_lock(boost::posix_time::milliseconds(100)))
	{
		
		if(error == boost::asio::error::eof)
		{
			if(logging_) Logger::DiagnosticOut() << "Server: Header read OK, but client DC'd\n";
		}
		else if(error)
		{
			if(logging_) Logger::DiagnosticOut() << "Server: Error during header read: " << error.message() << "\n";
		}
		else
		{
			int body_size = Opcodes::GetBodySize((Opcodes::ClientOpcode*)_buffer->c_array());
			if(logging_) Logger::DiagnosticOut() << "Server: Read header finished, looking for body size " << body_size << "\n";
			client_opcode_ = Opcodes::GetClientOpcode((Opcodes::ClientOpcode*)_buffer->c_array());
			SBuffer read_buffer = SBuffer(new boost::array<char, 512>());
			boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, body_size), boost::bind(&ServerConnection::ReadBodyFinished, this, boost::asio::placeholders::error, read_buffer));
		}
		if(error) error_ = error;
		server_->GetMutex().unlock();
	} else if(logging_) Logger::DiagnosticOut() << "ServerConnection::ReadHeaderFinished: Unable to lock, probably being shutdown\n";
}

void ServerConnection::ReadBodyFinished(boost::system::error_code error, SBuffer _buffer)
{
	if(server_->GetMutex().timed_lock(boost::posix_time::milliseconds(100)))
	{
		
		if(error == boost::asio::error::eof)
		{
			if(logging_) Logger::DiagnosticOut() << "Server: Body read OK, but client DC'd\n";
		}
		else if(error)
		{
			if(logging_) Logger::DiagnosticOut() << "Server: Error during body read: " << error.message() << "\n";
		}
		else
		{
			//Handle opcode creation

			memcpy(((char*)client_opcode_) + Opcodes::ClientOpcode::HEADERSIZE, _buffer->c_array(),  Opcodes::GetBodySize(client_opcode_));
			//Pass newly created opcode to server, which is then responsible for freeing it
			server_->HandleOpcode(this, player_id_, client_opcode_);

			if(logging_) Logger::DiagnosticOut() << "Server: Read body finished, looking for header again\n";
			SBuffer read_buffer = SBuffer(new boost::array<char, 512>());
			boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, Opcodes::ClientOpcode::HEADERSIZE), boost::bind(&ServerConnection::ReadHeaderFinished, this, boost::asio::placeholders::error, read_buffer));
		}
		if(error) error_ = error;
		client_opcode_ = NULL;
		server_->GetMutex().unlock();
	} else if(logging_) Logger::DiagnosticOut() << "ServerConnection::ReadBodyFinished: Unable to lock, probably being shutdown\n";
}

void ServerConnection::SendOpcode(Opcodes::ServerOpcode* _opcode)
{
	SBuffer send_buffer = SBuffer(new boost::array<char, 512>());
	
	int bytes_to_send = Opcodes::GetBodySize(_opcode) + Opcodes::ServerOpcode::HEADERSIZE;
	memcpy(send_buffer->c_array(), _opcode, bytes_to_send);

	socket_.async_send(boost::asio::buffer(*send_buffer, bytes_to_send), boost::bind(&ServerConnection::WriteFinished, this, boost::asio::placeholders::error, send_buffer));
}

void ServerConnection::SendLevelToClient(std::string _level_name)
{
	//TODO lots of error checking here
	//unsigned int filesize = boost::filesystem::file_size("Levels/" + _level_name);

	//Read data into a vector
	std::ifstream file(("Levels/" + _level_name).c_str());
	if(file.is_open())
	{
 		boost::shared_ptr<std::vector<char> > level_data = boost::shared_ptr<std::vector<char> >(new std::vector<char>());

		file >> std::noskipws; //Turns off whitespace skipping
		std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(),
				  std::back_inserter(*level_data));

		Opcodes::LevelDownload* level_download = new Opcodes::LevelDownload(_level_name, level_data->size());
		SendOpcode(level_download);

		//Send data to client
		socket_.async_send(boost::asio::buffer(*level_data), boost::bind(&ServerConnection::WriteLevelFinished, this, boost::asio::placeholders::error, level_data));
	} else
	{
		SendOpcode(new Opcodes::KickClient("Sorry, kicking you as server cannot send you the level for some reason"));
	}
}

void ServerConnection::WriteLevelFinished(boost::system::error_code error, boost::shared_ptr<std::vector<char> > _data)
{
	if(logging_) Logger::DiagnosticOut() << "ServerConnection: Finished writing level to server, disposing of in memory copy (wrote " << _data->size() << " bytes) \n";
}