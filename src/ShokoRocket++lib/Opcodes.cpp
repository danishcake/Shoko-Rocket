#include "Opcodes.h"

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
		default:
			//TODO warnings here
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
		default:
			//TODO warnings here
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
		default:
			//TODO warnings here
			break;
		}
		return opcode;
	}
}

