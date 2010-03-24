#include "stdafx.h"
#include <Server.h>
#include <Client.h>
#include <Logger.h>
#include <ServerWorld.h>
#include <Walker.h>

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
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	CHECK_EQUAL(ClientState::Connected, c->GetState());
	delete s;

	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
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
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
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

TEST(ServerWorld)
{
	ServerWorld* sworld = new ServerWorld(0);

	delete sworld;
}

TEST(ServerWorldCollidesMiceAndCatsAndHolesAndRockets)
{
	ServerWorld* sworld = new ServerWorld("ServerWorldCollisions.Level", 0);
	CHECK_EQUAL(false, sworld->GetError());
	sworld->AddMouse(Vector2i(0, 1), Direction::North); //Hits hole 
	sworld->AddMouse(Vector2i(4, 2), Direction::South); //Hits rocket
	sworld->AddCat(Vector2i(3, 0), Direction::West); //Hits hole
	sworld->AddCat(Vector2i(0, 4), Direction::East); //Hits rocket
	
	

	CHECK_EQUAL(2, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());
	//Colide mouse with hole
	sworld->Tick(1.01f);
	CHECK_EQUAL(1, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());
	//Colide mouse with rocket
	sworld->Tick(1.0f);
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());
	//Colide cat with hole
	sworld->Tick(2.6f);
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(1, sworld->GetCats().size());
	//Colide cat with rocket
	sworld->Tick(1.5f);
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(0, sworld->GetCats().size());

	delete sworld;
}

TEST(ServerWorldCollidesMiceAndCatsWithArrows)
{
	ServerWorld* sworld = new ServerWorld("ServerWorldCollisions.Level", 0);
	CHECK_EQUAL(false, sworld->GetError());

	sworld->AddMouse(Vector2i(1,0), Direction::East);
	sworld->AddCat(Vector2i(3,4), Direction::West);
	sworld->SetPlayerArrow(Vector2i(2,0), Direction::South, 0, PlayerArrowLevel::FullArrow);
	sworld->SetPlayerArrow(Vector2i(2,4), Direction::North, 0, PlayerArrowLevel::FullArrow);

	sworld->Tick(2);
	CHECK_EQUAL(Direction::South, sworld->GetMice().at(0)->GetDirection());
	CHECK_EQUAL(Direction::North, sworld->GetCats().at(0)->GetDirection());

	delete sworld;
}

TEST(ServerHandlesOpcodeInput)
{
	ServerWorld* sworld = new ServerWorld("ServerWorldCollisions.Level", 0);
	vector<vector<Opcodes::ClientOpcode*> > opcodes;
	//Place an arrow
	opcodes.push_back(vector<Opcodes::ClientOpcode*>());
	opcodes[0].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_NORTH));
	sworld->HandleOpcodes(opcodes); 
	//Opcodes are invalid after this so clear
	opcodes[0].clear();

	CHECK_EQUAL(1, sworld->GetPlayerArrows().size());
	if(sworld->GetPlayerArrows().size() == 1)
	{
		CHECK_EQUAL(0, sworld->GetPlayerArrows()[0].player_id);
		CHECK_EQUAL(Direction::North, sworld->GetPlayerArrows()[0].direction);
		CHECK_EQUAL(false, sworld->GetPlayerArrows()[0].halved);
	}
	opcodes.push_back(vector<Opcodes::ClientOpcode*>());
	opcodes[1].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_NORTH));
	sworld->HandleOpcodes(opcodes); 
	opcodes[0].clear();
	opcodes[1].clear();
	//Should change anything as slot taken
	CHECK_EQUAL(1, sworld->GetPlayerArrows().size());
	if(sworld->GetPlayerArrows().size() == 1)
	{
		CHECK_EQUAL(0, sworld->GetPlayerArrows()[0].player_id);
		CHECK_EQUAL(Direction::North, sworld->GetPlayerArrows()[0].direction);
		CHECK_EQUAL(false, sworld->GetPlayerArrows()[0].halved);
	}

	opcodes[0].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_NORTH));
	sworld->HandleOpcodes(opcodes); 
	opcodes[0].clear();
	opcodes[1].clear();
	//Should have cleared
	CHECK_EQUAL(0, sworld->GetPlayerArrows().size());

	opcodes[1].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_SOUTH));
	sworld->HandleOpcodes(opcodes); 
	opcodes[0].clear();
	opcodes[1].clear();

	//Should now be player 1
	CHECK_EQUAL(1, sworld->GetPlayerArrows().size());
	if(sworld->GetPlayerArrows().size() == 1)
	{
		CHECK_EQUAL(1, sworld->GetPlayerArrows()[0].player_id);
		CHECK_EQUAL(Direction::South, sworld->GetPlayerArrows()[0].direction);
		CHECK_EQUAL(false, sworld->GetPlayerArrows()[0].halved);
	}

	//Now check can't be cleared by other players, but can by self
	opcodes[0].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_CLEAR));
	sworld->HandleOpcodes(opcodes); 
	opcodes[0].clear();
	opcodes[1].clear();
	CHECK_EQUAL(1, sworld->GetPlayerArrows().size());
	opcodes[1].push_back(new Opcodes::SendInput(Vector2<unsigned char>(0,0), Opcodes::SendInput::ACT_CLEAR));
	sworld->HandleOpcodes(opcodes); 
	opcodes[0].clear();
	opcodes[1].clear();
	CHECK_EQUAL(0, sworld->GetPlayerArrows().size());

	delete sworld;
}

TEST(ServerGeneratesResponseOpcodesToArrowInput)
{
	ServerWorld* sworld = new ServerWorld("ServerWorldCollisions.Level", 0);
	vector<vector<Opcodes::ClientOpcode*> > opcodes;
	//Place an arrow
	opcodes.push_back(vector<Opcodes::ClientOpcode*>());
	opcodes[0].push_back(new Opcodes::SendInput(Vector2<unsigned char>(1,3), Opcodes::SendInput::ACT_EAST));
	sworld->HandleOpcodes(opcodes); 
	//Opcodes are invalid after this so clear
	opcodes[0].clear();

	CHECK_EQUAL(1, sworld->GetPlayerArrows().size());

	vector<Opcodes::ServerOpcode*> opcodes_out = sworld->GetOpcodes();
	CHECK_EQUAL(1, opcodes_out.size()); //Should be one message to client instructing of a new arrow
	if(opcodes_out.size() == 1)
	{
		CHECK_EQUAL(1, ((Opcodes::ArrowSpawn*)opcodes_out[0])->position_.x);
		CHECK_EQUAL(3, ((Opcodes::ArrowSpawn*)opcodes_out[0])->position_.y);
		CHECK_EQUAL(Opcodes::ArrowSpawn::DIRECTION_EAST, ((Opcodes::ArrowSpawn*)opcodes_out[0])->direction_);
		CHECK_EQUAL(Opcodes::ArrowSpawn::ARROW_FULL, ((Opcodes::ArrowSpawn*)opcodes_out[0])->arrow_state_);
		CHECK_EQUAL(0, ((Opcodes::ArrowSpawn*)opcodes_out[0])->player_);
	}

	delete sworld;
}

TEST(ServerGeneratesResponseOpcodesToCollisions)
{
	ServerWorld* sworld = new ServerWorld("ServerWorldCollisions.Level", 0);

	sworld->AddMouse(Vector2i(0, 1), Direction::North); //Hits hole 
	sworld->AddMouse(Vector2i(4, 2), Direction::South); //Hits rocket
	sworld->AddCat(Vector2i(3, 0), Direction::West); //Hits hole
	sworld->AddCat(Vector2i(0, 4), Direction::East); //Hits rocket

	CHECK_EQUAL(2, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());
	//Colide mouse with hole
	sworld->Tick(1.01f);
	vector<Opcodes::ServerOpcode*> opcodes_out = sworld->GetOpcodes();
	CHECK_EQUAL(1, opcodes_out.size()); //Should be one message to client about death of mouse
	if(opcodes_out.size() == 1)
	{
		CHECK_CLOSE(0, ((Opcodes::KillWalker*)opcodes_out[0])->position_.x, 0.05f);
		CHECK_CLOSE(0, ((Opcodes::KillWalker*)opcodes_out[0])->position_.y, 0.05f);
		CHECK(((Opcodes::KillWalker*)opcodes_out[0])->death_);
	}
	opcodes_out.clear();


	CHECK_EQUAL(1, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());

	//Colide mouse with rocket
	sworld->Tick(1.0f);
	opcodes_out = sworld->GetOpcodes();
	CHECK_EQUAL(1, opcodes_out.size()); //Should be one message to client about death of mouse
	if(opcodes_out.size() == 1)
	{
		CHECK_CLOSE(4, ((Opcodes::KillWalker*)opcodes_out[0])->position_.x, 0.05f);
		CHECK_CLOSE(4, ((Opcodes::KillWalker*)opcodes_out[0])->position_.y, 0.05f);
		CHECK(!((Opcodes::KillWalker*)opcodes_out[0])->death_);
	}
	opcodes_out.clear();
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(2, sworld->GetCats().size());


	//Colide cat with hole
	sworld->Tick(2.6f);
	opcodes_out = sworld->GetOpcodes();
	CHECK_EQUAL(1, opcodes_out.size()); //Should be one message to client about death of mouse
	if(opcodes_out.size() == 1)
	{
		CHECK_CLOSE(0, ((Opcodes::KillWalker*)opcodes_out[0])->position_.x, 0.05f);
		CHECK_CLOSE(0, ((Opcodes::KillWalker*)opcodes_out[0])->position_.y, 0.05f);
		CHECK(((Opcodes::KillWalker*)opcodes_out[0])->death_);
	}
	opcodes_out.clear();
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(1, sworld->GetCats().size());
	//Colide cat with rocket
	sworld->Tick(1.5f);
	opcodes_out = sworld->GetOpcodes();
	CHECK_EQUAL(1, opcodes_out.size()); //Should be one message to client about death of mouse
	if(opcodes_out.size() == 1)
	{
		CHECK_CLOSE(4, ((Opcodes::KillWalker*)opcodes_out[0])->position_.x, 0.05f);
		CHECK_CLOSE(4, ((Opcodes::KillWalker*)opcodes_out[0])->position_.y, 0.05f);
		CHECK(((Opcodes::KillWalker*)opcodes_out[0])->death_);
	}
	opcodes_out.clear();
	CHECK_EQUAL(0, sworld->GetMice().size());
	CHECK_EQUAL(0, sworld->GetCats().size());


	delete sworld;
}

TEST(RepeatMessagesOK)
{
	Server* s = new Server();
	Client* c = new Client();

	c->Connect("localhost", 9020);
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	CHECK_EQUAL(ClientState::Connected, c->GetState());

	c->GetOpcodes(); //Throw away new opcodes

	s->SendOpcodeToAll(new Opcodes::ChatMessage("ElectricBoogaloo", Opcodes::ChatMessage::SENDER_SERVER));
	s->SendOpcodeToAll(new Opcodes::ChatMessage("ElectricBoogaloo2", Opcodes::ChatMessage::SENDER_SERVER));
	s->SendOpcodeToAll(new Opcodes::ChatMessage("ElectricBoogaloo3", Opcodes::ChatMessage::SENDER_SERVER));
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	vector<Opcodes::ServerOpcode*> opcodes = c->GetOpcodes();
	CHECK_EQUAL(3, opcodes.size());

	s->GetOpcodes();
	c->SendOpcode(new Opcodes::SetName("Edward"));
	c->SendOpcode(new Opcodes::SetName("Edward"));
	c->SendOpcode(new Opcodes::SetName("Edward"));
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	CHECK_EQUAL(3, s->GetOpcodes()[0].size());
	c->SendOpcode(new Opcodes::SendChatMessage("Hey dude"));
	c->SendOpcode(new Opcodes::SetName("Edward"));
	c->SendOpcode(new Opcodes::UpdateCursor(Vector2<unsigned short>()));
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	CHECK_EQUAL(3, s->GetOpcodes()[0].size());

	s->SendOpcodeToAll(new Opcodes::PlayerName("Edward", 0));
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	opcodes = c->GetOpcodes();
	
	CHECK_EQUAL(1, opcodes.size());

	delete c;
	delete s;
}

TEST(MPWorldCanHandleArrowInputOpcodes)
{
}

TEST(MPWorldCanHandleCollisionChangeOpcodes)
{

}

TEST(ClientsSetLevel)
{
}