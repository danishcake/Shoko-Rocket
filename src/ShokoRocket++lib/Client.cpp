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
	server_opcode_ = NULL;
	logging_ = false;
}

Client::~Client(void)
{
	if(logging_) Logger::DiagnosticOut() << "Client: Destructor started\n";
	closing_ = true;
	mutex_.lock();
	socket_.close();
	thread_->join();
	delete thread_;
	mutex_.unlock(); //Probably not necessary as being destroyed
	if(logging_) Logger::DiagnosticOut() << "Client: Destructor finished\n";
}

void Client::Connect(std::string _host, unsigned short _port)
{
	state_ = ClientState::Connecting;
	tcp::resolver resolver(io_);
	tcp::resolver_query query("localhost", "9020");
	endpoints_ = resolver.resolve(query);
	tcp::endpoint ep = *endpoints_++;
	socket_.async_connect(ep, boost::bind(&Client::ConnectHandler, this, boost::asio::placeholders::error));
	if(logging_) Logger::DiagnosticOut() << "Client: Attempting to connect to " << boost::lexical_cast<std::string, tcp::endpoint>(ep) << "\n";
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
				if(logging_) Logger::DiagnosticOut() << "Client: Attempting to connect to " << boost::lexical_cast<std::string, tcp::endpoint>(ep) << "\n";
			} else
			{
				if(logging_) Logger::DiagnosticOut() << "Client: Unable to connect to any of the endpoints\n";
				state_ = ClientState::NotConnected;
			}
		} else
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Connected successfully\n";
			state_ = ClientState::Connected;
			boost::shared_ptr<Opcodes::ClientOpcode> opcode = boost::shared_ptr<Opcodes::ClientOpcode>(new Opcodes::SetName(name_));
			socket_.async_send(boost::asio::buffer((char*)opcode.get(), sizeof(Opcodes::SetName)), boost::bind(&Client::WriteFinished, this, boost::asio::placeholders::error, opcode));

			CBuffer_ptr read_buffer = CBuffer_ptr(new CBuffer());

			boost::asio::async_read(socket_, boost::asio::buffer(*read_buffer, Opcodes::ServerOpcode::HEADERSIZE), 
									boost::bind(&Client::ReadHeaderFinished, 
									this, 
									boost::asio::placeholders::error,
									read_buffer));
			
		}
		mutex_.unlock();
	} else if(logging_) Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}

void Client::ReadHeaderFinished(boost::system::error_code error, CBuffer_ptr _read_buffer)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Header read OK, but server DC'd\n";
		}
		else if(error)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Error during header read: " << error.message() << "\n";
		}
		else
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Read header finished\n";
			int body_size = Opcodes::GetBodySize((Opcodes::ServerOpcode*)_read_buffer->c_array());
			if(logging_) Logger::DiagnosticOut() << "Client: Trying to read body of size " << body_size << "\n";
			server_opcode_ = Opcodes::GetServerOpcode((Opcodes::ServerOpcode*)_read_buffer->c_array());
		
			CBuffer_ptr read_buffer = CBuffer_ptr(new CBuffer());
			boost::asio::async_read(socket_, 
									boost::asio::buffer(*read_buffer, body_size),
									boost::bind(&Client::ReadBodyFinished,
									this,
									boost::asio::placeholders::error,
									read_buffer));
		}
		if(error)
		{
			error_ = error;
			closing_ = true;
			state_ = ClientState::NotConnected;
		}
		mutex_.unlock();
	} else if(logging_) Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}

void Client::ReadBodyFinished(boost::system::error_code error, CBuffer_ptr _read_buffer)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Body read OK, but client DC'd\n";
		}
		else if(error)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Error during body read: " << error.message() << "\n";
		}
		else
		{
			memcpy(((char*)server_opcode_) + Opcodes::ServerOpcode::HEADERSIZE, _read_buffer->c_array(),  Opcodes::GetBodySize(server_opcode_));
			if(server_opcode_->opcode_ == Opcodes::LevelDownload::OPCODE)
			{
				//Start special read of level
				Opcodes::LevelDownload* level_opcode = (Opcodes::LevelDownload*)server_opcode_;
				if(logging_) Logger::DiagnosticOut() << "Client: Read body finished, looking for level of size " << level_opcode->level_size_ << " called " << level_opcode->level_name_ << "\n";

				
				boost::shared_ptr<std::vector<char> > level_data = boost::shared_ptr<std::vector<char> >(new std::vector<char>(level_opcode->level_size_));
				boost::asio::async_read(socket_, 
						boost::asio::buffer(*level_data),
						boost::bind(&Client::ReadLevelFinished,
						this,
						boost::asio::placeholders::error,
						level_data,
						std::string(level_opcode->level_name_)));

				delete server_opcode_;
				server_opcode_ = NULL;
			} else
			{
				opcodes_.push_back(server_opcode_);
				server_opcode_ = NULL;
				CBuffer_ptr read_buffer = CBuffer_ptr(new CBuffer());
				if(logging_) Logger::DiagnosticOut() << "Client: Read body finished, looking for header again\n";
				boost::asio::async_read(socket_, 
										boost::asio::buffer(*read_buffer, Opcodes::ServerOpcode::HEADERSIZE),
										boost::bind(&Client::ReadHeaderFinished,
										this,
										boost::asio::placeholders::error,
										read_buffer));
			}
		}
		if(error)
		{
			error_ = error;	
			closing_ = true;
			state_ = ClientState::NotConnected;
		}
		mutex_.unlock();
	} else if(logging_) Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}

void Client::ReadLevelFinished(boost::system::error_code error, boost::shared_ptr<std::vector<char> > _level_data, std::string _filename)
{
	if(mutex_.timed_lock(boost::posix_time::milliseconds(100)))
	{
		if(error == boost::asio::error::eof)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Level read OK, but client DC'd\n";
		}
		else if(error)
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Error during level read: " << error.message() << "\n";
		}
		else
		{
			if(logging_) Logger::DiagnosticOut() << "Client: Finished receiving level, looking for new opcode header\n";

			Opcodes::LevelDownloadData* level_download = new Opcodes::LevelDownloadData(_filename, _level_data);
			opcodes_.push_back(level_download);

			CBuffer_ptr read_buffer = CBuffer_ptr(new CBuffer());
			boost::asio::async_read(socket_, 
									boost::asio::buffer(*read_buffer, Opcodes::ServerOpcode::HEADERSIZE),
									boost::bind(&Client::ReadHeaderFinished,
									this,
									boost::asio::placeholders::error,
									read_buffer));
		}
		mutex_.unlock();
	} else if(logging_) Logger::DiagnosticOut() << "Client: Unable to get lock, so must be closing\n";
}

void Client::WriteFinished(boost::system::error_code error, boost::shared_ptr<Opcodes::ClientOpcode> _data)
{
	mutex_.lock();
	if(logging_) Logger::DiagnosticOut() << "Client: Write complete\n";
	//Do nothing, but memory should now be freed
	mutex_.unlock();
}

vector<Opcodes::ServerOpcode*> Client::GetOpcodes()
{
	mutex_.lock();
	vector<Opcodes::ServerOpcode*> opcodes_copy = opcodes_;
	opcodes_.clear();
	mutex_.unlock();
	return opcodes_copy;
}

void Client::SendOpcode(Opcodes::ClientOpcode* _opcode)
{
	boost::shared_ptr<Opcodes::ClientOpcode> opcode = boost::shared_ptr<Opcodes::ClientOpcode>(_opcode);
	socket_.async_send(boost::asio::buffer((char*)opcode.get(), Opcodes::GetBodySize(_opcode) + Opcodes::ClientOpcode::HEADERSIZE), boost::bind(&Client::WriteFinished, this, boost::asio::placeholders::error, opcode));
}