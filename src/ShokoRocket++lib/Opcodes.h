#pragma once
#include <string>
#include "vmath.h"
#include <boost/shared_ptr.hpp>
#include <vector>
using std::string;

namespace Opcodes
{
	/*
	 * Messages sent from Server to Client
	 */
	struct ServerOpcode
	{
		static const unsigned int HEADERSIZE = 5;
	public:
		ServerOpcode()
		{
			time_ = 0;
		}
		unsigned char opcode_;
		unsigned int time_;
	};

	/*
	 * Represents a message sent either by the server itself or a client
	 */
	struct ChatMessage : public ServerOpcode
	{
		static const unsigned char OPCODE = 1;
		static const unsigned char SENDER_SERVER = 255;
	public:
		ChatMessage(string _message, unsigned char _sender)
		{
			opcode_ = OPCODE;
			sender_ = _sender;
			//Construct message carefully
			std::size_t len = _message.size();
			len = len > 255 ? 255 : len;
			memset(&message_, 0, 256);
			memcpy(&message_, _message.c_str(), len);
			
		}
		unsigned char sender_;
		char message_[256];
	};

	/* 
	 * Instructs a client to spawn a walker at the given location 
	 */
	struct WalkerSpawn : public ServerOpcode
	{
		static const unsigned char OPCODE = 2;
		
		typedef unsigned char WalkerType;
		typedef unsigned char Direction;
		static const WalkerType WALKER_MOUSE = 0;
		static const WalkerType WALKER_CAT  = 1;
		static const Direction DIRECTION_NORTH  = 0;
		static const Direction DIRECTION_SOUTH  = 1;
		static const Direction DIRECTION_EAST = 2;
		static const Direction DIRECTION_WEST = 3;
	public:
		WalkerSpawn(WalkerType _walker_type, Vector2<unsigned char> _position, Direction _direction, unsigned int _uid)
		{
			opcode_ = OPCODE;
			walker_type_ = _walker_type;
			position_ = _position;
			direction_ = _direction;
			uid_ = _uid;
		}
		WalkerType walker_type_;
		Vector2<unsigned char> position_;
		Direction direction_;
		unsigned int uid_;
	};

	/*
	 * Instructs the client to spawn an arrow at the given location
	 */
	struct ArrowSpawn : public ServerOpcode
	{
		static const unsigned char OPCODE = 3;
		
		typedef unsigned char Direction;
		typedef unsigned char ArrowState;
		static const Direction DIRECTION_NORTH  = 0;
		static const Direction DIRECTION_SOUTH  = 1;
		static const Direction DIRECTION_EAST = 2;
		static const Direction DIRECTION_WEST = 3;

		static const ArrowState ARROW_CLEAR = 0;
		static const ArrowState ARROW_HALF = 1;
		static const ArrowState ARROW_FULL = 2;
	public:
		ArrowSpawn(ArrowState _arrow_state, Vector2<unsigned char> _position, Direction _direction, unsigned char _player)
		{
			opcode_ = OPCODE;
			arrow_state_ = _arrow_state;
			position_ = _position;
			direction_ = _direction;
			player_ = _player;
		}
		Vector2<unsigned char> position_;
		Direction direction_;
		ArrowState arrow_state_;
		unsigned char player_;
	};

	/*
	 * Instructs the client to kill the walker with the given unique ID
	 */
	struct KillWalker : public ServerOpcode
	{
		static const unsigned char OPCODE = 4;
	public:
		KillWalker(Vector2f _position, unsigned int _uid, bool _death)
		{
			opcode_ = OPCODE;
			position_ = _position;
			uid_ = _uid;
			if(_death)
				death_ = 1;
			else
				death_ = 0;
		}
		Vector2f position_;
		unsigned char death_;
		unsigned int uid_;
	};

	/* 
	 * Instructs the client to update the cursor position for a given other player
	 */
	struct DriveCursor : public ServerOpcode
	{
		static const unsigned char OPCODE = 5;
	public:
		DriveCursor(unsigned char _player, Vector2<short> _position)
		{
			opcode_ = OPCODE;
			player_ = _player;
			position_ = _position;
		}
		Vector2<short> position_;
		unsigned char player_;
	};

	/*
	 * Informs the client of a player name
	 */
	struct PlayerName : public ServerOpcode
	{
		static const unsigned OPCODE = 6;
	public:
		PlayerName(std::string _name, unsigned char _player)
		{
			opcode_ = OPCODE;
			player_ = _player;
			//Construct message carefully
			std::size_t len = _name.size();
			len = len > 31 ? 31 : len;
			memset(&name_, 0, 32);
			memcpy(&name_, _name.c_str(), len);
		}
		char name_[32];
		unsigned char player_;
	};



	/*
	 * Transitions the client state
	 */
	struct StateTransition : public ServerOpcode
	{
		static const unsigned OPCODE = 7;

		typedef unsigned char NewState;
		static const NewState STATE_LOBBY = 1;
		static const NewState STATE_GAME = 2;
	public:
		StateTransition(NewState _state, std::string _level)
		{
			opcode_ = OPCODE;
			state_ = _state;
			std::size_t len = _level.size();
			len = len > 255 ? 255 : len;
			memset(&level_, 0, 256);
			memcpy(&level_, _level.c_str(), len);
		}
		NewState state_;
		char level_[256];
	};




	/*
	 * ClientDisconnect - informs other clients of a dropout
	 */
	struct ClientDisconnection : public ServerOpcode
	{
		static const unsigned OPCODE = 8;
	public:
		ClientDisconnection(unsigned char _client_id)
		{
			opcode_ = OPCODE;
			client_id_ = _client_id;
		}
		unsigned char client_id_;
	};
	/*
	 * ClientDisconnect - politely tells a client why they're being disconnected
	 */
	struct KickClient : public ServerOpcode
	{
		static const unsigned OPCODE = 9;
	public:
		KickClient(std::string _message)
		{
			opcode_ = OPCODE;
			std::size_t len = _message.size();
			len = len > 255 ? 255 : len;
			memset(&msg_, 0, 256);
			memcpy(&msg_, _message.c_str(), len);
		}
		char msg_[256];
	};

	/*
	 * Client ready status
	 */
	struct ReadyState : public ServerOpcode
	{
		static const unsigned OPCODE = 10;
	public:
		ReadyState(bool _ready, unsigned char _sender)
		{
			opcode_ = OPCODE;
			ready_ = _ready;
			sender_ = _sender;
		}
		bool ready_;
		unsigned char sender_;
	};


	/*
	 * Level Download
	 */
	struct LevelDownload : public ServerOpcode
	{
		static const unsigned OPCODE = 11;
	public:
		LevelDownload(std::string _level_name, unsigned int _level_size)
		{
			opcode_ = OPCODE;
			std::size_t len = _level_name.size();
			len = len > 255 ? 255 : len;
			memset(&level_name_, 0, 256);
			memcpy(&level_name_, _level_name.c_str(), len);		
			level_size_ = _level_size;
		}
		unsigned int level_size_;
		char level_name_[256];
		//At the end of sending this send the level data too
	};
	/*
	 * Level Download(extended)
	 * Included received level data
	 */
	struct LevelDownloadData : public ServerOpcode
	{
		static const unsigned OPCODE = 11;
	public:
		LevelDownloadData(std::string _level_name, boost::shared_ptr<std::vector<char> > _data)
		{
			opcode_ = OPCODE;
			std::size_t len = _level_name.size();
			len = len > 255 ? 255 : len;
			memset(&level_name_, 0, 256);
			memcpy(&level_name_, _level_name.c_str(), len);		
			data_ = _data;
		}
		boost::shared_ptr<std::vector<char> > data_;
		char level_name_[256];
		//At the end of sending this send the level data too
	};


	/* 
	 * Instructs a client to update a walker position/orientation
	 */
	struct WalkerUpdate : public ServerOpcode
	{
		static const unsigned char OPCODE = 12;

		typedef unsigned char Direction;
		static const Direction DIRECTION_NORTH  = 0;
		static const Direction DIRECTION_SOUTH  = 1;
		static const Direction DIRECTION_EAST = 2;
		static const Direction DIRECTION_WEST = 3;
	public:
		WalkerUpdate(Vector2<unsigned char> _position, Direction _direction, unsigned int _uid)
		{
			opcode_ = OPCODE;
			position_ = _position;
			direction_ = _direction;
			uid_ = _uid;
		}
		Vector2<unsigned char> position_;
		Direction direction_;
		unsigned int uid_;
	};

	/*
	 * Messages sent from client to server
	 */
	struct ClientOpcode
	{
		static const unsigned int HEADERSIZE = 1;
	public:
		char opcode_;
	};

	/*
	 * Informs the server of an updated cursor position
	 */ 
	struct UpdateCursor : public ClientOpcode
	{
		static const unsigned char OPCODE = 1;
	public:
		UpdateCursor(Vector2<unsigned short> _position)
		{
			opcode_ = OPCODE;
			position_ = _position;
		}
		Vector2<unsigned short> position_;
	};

	/*
	 * Informs the server of other input, typically attempted arrow placement
	 */ 
	struct SendInput : public ClientOpcode
	{
		static const unsigned char OPCODE = 2;
		typedef unsigned char Action;
		static const Action ACT_NORTH = 1;
		static const Action ACT_SOUTH = 2;
		static const Action ACT_WEST = 3;
		static const Action ACT_EAST = 4;
		static const Action ACT_CLEAR = 5;
		static const Action ACT_NONE = 5;
	public:
		SendInput(Vector2<unsigned char> _position, Action _action)
		{
			opcode_ = OPCODE;
			position_ = _position;
			action_ = _action;
		}
		Vector2<unsigned char> position_;
		Action action_;
	};
	/*
	 * Sets the clients name
	 */
	struct SetName : public ClientOpcode
	{
		static const unsigned char OPCODE = 3;
	public:
		SetName(std::string _name)
		{
			opcode_ = OPCODE;
			//Construct message carefully
			std::size_t len = _name.size();
			len = len > 29 ? 29 : len;
			memset(&name_, 0, 30);
			memcpy(&name_, _name.c_str(), len);
		}
		char name_[30];
	};

	/*
	 * Represents a message sent either by the client to the server
	 */
	struct SendChatMessage : public ClientOpcode
	{
		static const unsigned char OPCODE = 4;
		static const unsigned char SENDER_SERVER = 255;
	public:
		SendChatMessage(string _message)
		{
			opcode_ = OPCODE;
			//Construct message carefully
			std::size_t len = _message.size();
			len = len > 255 ? 255 : len;
			memset(&message_, 0, 256);
			memcpy(&message_, _message.c_str(), len);
			
		}
		char message_[256];
	};

	/* 
	 * Represents a message sent to toggle ready state. Game states when all ready
	 */
	struct SetReady : public ClientOpcode
	{
		static const unsigned char OPCODE = 5;
	public:
		SetReady(bool _ready)
		{
			opcode_ = OPCODE;
			ready_ = _ready;
		}
		bool ready_;
	};

	/* 
	 * Asks the server to send the named level
	 */
	struct RequestDownload : public ClientOpcode
	{
		static const unsigned char OPCODE = 6;
	public:
		RequestDownload(std::string _level)
		{
			opcode_ = OPCODE;
			std::size_t len = _level.size();
			len = len > 255 ? 255 : len;
			memset(&level_, 0, 256);
			memcpy(&level_, _level.c_str(), len);		
		}
		char level_[256];
	};

	/* 
	 * Informs server that loading is complete
	 */
	struct LoadComplete : public ClientOpcode
	{
		static const unsigned char OPCODE = 7;
	public:
		LoadComplete()
		{
			opcode_ = OPCODE;
		}
	};

	unsigned int GetBodySize(ServerOpcode* _header);
	unsigned int GetBodySize(ClientOpcode* _header);
	ServerOpcode* GetServerOpcode(ServerOpcode* _header);
	ClientOpcode* GetClientOpcode(ClientOpcode* _header);
}
