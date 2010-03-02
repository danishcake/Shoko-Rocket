#include "stdafx.h"
#include "Server.h"
#include "Client.h"

TEST(ServerBasics)
{
	//Server has name, list of players, list of maps
	Server* server= new Server();

	server->SetName("ChuChuTron2000");
	CHECK_EQUAL("ChuChuTron2000", server->GetName());
	CHECK_EQUAL(0, server->GetPlayers().size());

	
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
	for(int i = 0; i < 1000; i++)
		client->Tick();
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