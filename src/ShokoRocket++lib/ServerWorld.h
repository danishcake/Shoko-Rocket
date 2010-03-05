#pragma once
#include "MPWorld.h"
#include "Opcodes.h"

class ServerWorld : public MPWorld
{
protected:
	bool GetPlayerArrow(Vector2i _position, PlayerArrow& _arrow);
	void HandleInputOpcode(int _player_id, Opcodes::SendInput* _input);
	void GenerateArrowOpcode(int _player_id, Vector2i _position, Direction::Enum _direction, PlayerArrowLevel::Enum _arrow_state);
	void GenerateWalkerDeath(int _uid, Vector2f _position, bool _death);
	vector<Opcodes::ServerOpcode*> opcodes_to_clients_;
public:
	//Constructors
	ServerWorld();
	ServerWorld(string _filename);

	//Update cat & mouse positions
	virtual WorldState::Enum Tick(float _dt);
	//Called when walkers reach a new gridsquare
	virtual void WalkerReachNewSquare(Walker* _walker);
	//Called when new opcodes received by server object
	void HandleOpcodes(vector<vector<Opcodes::ClientOpcode*> > _opcodes);
	//Returns opcodes for each client. Caller is responsible for freeing them. 
	vector<Opcodes::ServerOpcode*> GetOpcodes();
};