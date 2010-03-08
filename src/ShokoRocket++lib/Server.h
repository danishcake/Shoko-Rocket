#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "ServerConnection.h"
#include "Opcodes.h"

using std::string;
using std::vector;
using std::map;

class Server;

struct ServerThread
{
public:
	ServerThread(Server* _server){server_ = _server;}

	Server* server_;
	void operator()();
};

/* Server class. Handles multiple clients
 *
 * Designed to run on a single thread that it manages itself. Mutex prevents concurrent 
 * access to resources and ensure safe deletion
 * main() thread should always lock() before acting and should complete fast (<100ms)
 * callbacks should always be on a background thread, and should attempt to lock. Being unable to lock indicates
 * shutdown in process
 */
class Server
{
protected:
	string name_;
	boost::asio::io_service io_;
	boost::asio::io_service::work* work_;
	boost::thread* thread_;
	boost::mutex mutex_;
	boost::asio::deadline_timer timer_;

	tcp::acceptor acceptor_;
	void StartConnection();
	void ConnectionAccepted(ServerConnection* _connection, boost::system::error_code ec);

	vector<ServerConnection*> connections_;
	vector<vector<Opcodes::ClientOpcode*> > opcodes_;
	bool closing_;
	int players_count_;
public:
	Server(void);
	~Server(void);

	void SetName(string _name){name_ = _name;}
	string GetName(){return name_;}

	bool Tick();
	void PeriodicTidyup(boost::system::error_code _error);
	void HandleOpcode(int _player_id, Opcodes::ClientOpcode* _opcode);

	void SendOpcodeToAll(Opcodes::ServerOpcode* _opcode);

	boost::mutex& GetMutex(){return mutex_;}

	vector<vector<Opcodes::ClientOpcode*> > GetOpcodes();
};
