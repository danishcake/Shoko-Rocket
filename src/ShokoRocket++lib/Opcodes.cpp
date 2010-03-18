#include "Opcodes.h"
#include "Logger.h"

namespace Opcodes
{
	unsigned int GetBodySize(ServerOpcode* _header)
	{
		switch(_header->opcode_)
		{
		case ChatMessage::OPCODE:
			return sizeof(ChatMessage) - ServerOpcode::HEADERSIZE;
			break;
		case WalkerSpawn::OPCODE:
			return sizeof(WalkerSpawn) - ServerOpcode::HEADERSIZE;
			break;
		case ArrowSpawn::OPCODE:
			return sizeof(ArrowSpawn) - ServerOpcode::HEADERSIZE;
			break;
		case KillWalker::OPCODE:
			return sizeof(KillWalker) - ServerOpcode::HEADERSIZE;
			break;
		case DriveCursor::OPCODE:
			return sizeof(DriveCursor) - ServerOpcode::HEADERSIZE;
			break;
		case PlayerName::OPCODE:
			return sizeof(PlayerName) - ServerOpcode::HEADERSIZE;
			break;
		case StateTransition::OPCODE:
			return sizeof(StateTransition) - ServerOpcode::HEADERSIZE;
			break;
		case ClientDisconnection::OPCODE:
			return sizeof(ClientDisconnection) - ServerOpcode::HEADERSIZE;
			break;
		case KickClient::OPCODE:
			return sizeof(KickClient) - ServerOpcode::HEADERSIZE;
			break;
		case ReadyState::OPCODE:
			return sizeof(ReadyState) - ServerOpcode::HEADERSIZE;
			break;
		default:
			//TODO warnings here
			Logger::ErrorOut() << "Unable to find opcode, things about to break\n";
			return 0;
			break;
		}
	}

	unsigned int GetBodySize(ClientOpcode* _header)
	{
		switch(_header->opcode_)
		{
		case UpdateCursor::OPCODE:
			return sizeof(UpdateCursor) - ClientOpcode::HEADERSIZE;
			break;
		case SendInput::OPCODE:
			return sizeof(SendInput) - ClientOpcode::HEADERSIZE;
			break;
		case SetName::OPCODE:
			return sizeof(SetName) - ClientOpcode::HEADERSIZE;
			break;
		case SendChatMessage::OPCODE:
			return sizeof(SendChatMessage) - ClientOpcode::HEADERSIZE;
			break;
		case SetReady::OPCODE:
			return sizeof(SetReady) - ClientOpcode::HEADERSIZE;
			break;
		default:
			//TODO warnings here
			Logger::ErrorOut() << "Unable to find opcode, things about to break\n";
			return 0;
			break;
		}
	}

	ClientOpcode* GetClientOpcode(ClientOpcode* _header)
	{
		ClientOpcode* opcode = NULL;
		switch(_header->opcode_)
		{
		case UpdateCursor::OPCODE:
			opcode = new UpdateCursor(Vector2<unsigned short>());
			break;
		case SendInput::OPCODE:
			opcode = new SendInput(Vector2<unsigned short>(), SendInput::ACT_CLEAR);
			break;
		case SetName::OPCODE:
			opcode = new SetName("");
			break;
		case SendChatMessage::OPCODE:
			opcode = new SendChatMessage("");
			break;
		case SetReady::OPCODE:
			opcode = new SetReady(false);
			break;
		default:
			//TODO warnings here
			Logger::ErrorOut() << "Unable to find opcode, things about to break\n";
			break;
		}
		return opcode;
	}

	ServerOpcode* GetServerOpcode(ServerOpcode* _header)
	{
		ServerOpcode* opcode = NULL;
		switch(_header->opcode_)
		{
		case ChatMessage::OPCODE:
			opcode = new ChatMessage("", 0);
			break;
		case DriveCursor::OPCODE:
			opcode = new DriveCursor(0, Vector2<short>());
			break;
		case WalkerSpawn::OPCODE:
			opcode = new WalkerSpawn(WalkerSpawn::WALKER_CAT,Vector2<short>(), WalkerSpawn::DIRECTION_EAST, 0);
			break;
		case ArrowSpawn::OPCODE:
			opcode = new ArrowSpawn(ArrowSpawn::ARROW_CLEAR, Vector2<unsigned char>(), ArrowSpawn::DIRECTION_EAST, 0);
			break;
		case KillWalker::OPCODE:
			opcode = new KillWalker(Vector2f(), 0, false);
			break;
		case PlayerName::OPCODE:
			opcode = new PlayerName("", 0);
			break;
		case StateTransition::OPCODE:
			opcode = new StateTransition(StateTransition::STATE_GAME, "");
			break;
		case ClientDisconnection::OPCODE:
			opcode = new ClientDisconnection(0);
			break;
		case KickClient::OPCODE:
			opcode = new KickClient("");
			break;
		case ReadyState::OPCODE:
			opcode = new ReadyState(false, 0);
			break;
		default:
			//TODO warnings here
			Logger::ErrorOut() << "Unable to find opcode, things about to break\n";
			break;
		}
		return opcode;
	}
}

