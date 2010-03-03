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
	Logger::DiagnosticOut() << "ClientCantConnectToNothing Tests\n";
	Client* client = new Client();
	client->Connect("localhost", 9020);
	CHECK_EQUAL(ClientState::Connecting, client->GetState());
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500)); //Requires a hefty period for connection attempt to time out
	CHECK_EQUAL(ClientState::NotConnected, client->GetState());

	delete client;
	Logger::DiagnosticOut() << "ClientCantConnectToNothing Tests end\n";
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

	// Opcodes are cleared once returned
	CHECK_EQUAL(0, s->GetOpcodes().size());
	delete s;
	delete c;
}

TEST(ClientGetsServerName)
{
}

TEST(ClientsChat)
{
}

TEST(ClientsSetLevel)
{
}