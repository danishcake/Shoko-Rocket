#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include <Logger.h>

TEST(ServerBasics)
{
	//Server has name, list of players, list of maps
	Server* server= new Server();

	server->SetName("ChuChuTron2000");
	CHECK_EQUAL("ChuChuTron2000", server->GetName());
	//boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	delete server;
}

TEST(ClientBasics)
{
	//Client has name
	Client* client = new Client();
	CHECK_EQUAL("CHU CHU", client->GetName());
	client->SetName("danishcake");
	CHECK_EQUAL("danishcake", client->GetName());

	delete client;
}

TEST(ClientCantConnectToNothing)
{
	Client* client = new Client();
	client->Connect("localhost", 9020);
	CHECK_EQUAL(ClientState::Connecting, client->GetState());
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500)); //Requires a hefty period for connection attempt to time out
	CHECK_EQUAL(ClientState::NotConnected, client->GetState());

	delete client;
}

TEST(ClientCanConnectToServer)
{
	Server* s = new Server();
	Client* c = new Client();
	CHECK_EQUAL(ClientState::NotConnected, c->GetState());
	c->Connect("localhost", 9020);
	CHECK_EQUAL(ClientState::Connecting, c->GetState());
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	CHECK_EQUAL(ClientState::Connected, c->GetState());
	delete s;

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	CHECK_EQUAL(ClientState::NotConnected, c->GetState());
	delete c;
}

TEST(ClientCanAuthenticate)
{
}

//Opcode testing

TEST(ServerGetsClientName)
{
	Server* s = new Server();
	Client* c = new Client();
	c->SetName("Edward");
	c->Connect("localhost", 9020);
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	CHECK_EQUAL(ClientState::Connected, c->GetState());

	vector<vector<Opcodes::ClientOpcode*> > opcodes = s->GetOpcodes(); //Returns a vector of collected opcodes
	CHECK_EQUAL(1, opcodes.size()); //One client connected
	//Assumes first message client sends is name of Client
	CHECK_EQUAL("Edward", ((Opcodes::SetName*)opcodes[0][0])->name_);
	//TODO fix this memory leak by freeing opcodes

	// Opcodes are cleared once returned
	CHECK_EQUAL(0, s->GetOpcodes().size());
	delete s;
	delete c;
}

TEST(ClientGetsServerWelcome)
{
	Logger::DiagnosticOut() << "Running test: Do clients get server welcome?\n";
	Server* s = new Server();
	Client* c = new Client();

	c->Connect("localhost", 9020);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	CHECK_EQUAL(ClientState::Connected, c->GetState());

	vector<Opcodes::ServerOpcode*> opcodes = c->GetOpcodes(); //Returns a vector of collected opcodes
	CHECK_EQUAL(1, opcodes.size()); //Only one welcome message
	if(opcodes.size() > 0)
	{
		CHECK_EQUAL(Opcodes::ChatMessage::OPCODE, opcodes[0]->opcode_);
	}

	// Opcodes are cleared once returned
	CHECK_EQUAL(0, c->GetOpcodes().size());
	delete s;
	delete c;
	Logger::DiagnosticOut() << "End test: Do clients get server welcome?\n\n";
}

TEST(ClientsChat)
{
}

TEST(ClientsSetLevel)
{
}