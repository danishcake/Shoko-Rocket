#include "stdafx.h"
#include "Opcodes.h"

using Opcodes::ServerOpcode;
using Opcodes::ChatMessage;
using Opcodes::PlayerName;
using Opcodes::SetName;

TEST(ChatMessageCasts)
{
	ServerOpcode* chat = new ChatMessage("Is it better to have loved and lost?", 0);

	CHECK_EQUAL(chat->opcode_, ChatMessage::OPCODE);
	CHECK_EQUAL("Is it better to have loved and lost?", ((ChatMessage*)chat)->message_);
	delete chat;
}

TEST(ChatMessageLengthLimited)
{
	ChatMessage* chat = new ChatMessage("123456789012345678901234567890" //30 char
										"123456789012345678901234567890"
										"123456789012345678901234567890"
										"123456789012345678901234567890"
										"123456789012345678901234567890"
										"123456789012345678901234567890"
										"123456789012345678901234567890"
										"123456789012345678901234567890" //240
										"1234567890123456" //256
										"and then some more", 0);

	CHECK_EQUAL('\0', chat->message_[255]);
	delete chat;
}

TEST(NameLengthLimited)
{
	PlayerName name("12345678901234567890123456789012" //32 characters
					"and then some more", 1);
	CHECK_EQUAL('\0', name.name_[31]);
}

TEST(SetNameLengthLimited)
{
	SetName name("12345678901234567890123456789012" //32 characters
				 "and then some more");
	CHECK_EQUAL('\0', name.name_[29]);
}