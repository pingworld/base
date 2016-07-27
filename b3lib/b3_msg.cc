#include "b3_msg.h"

namespace B3LIB
{

static uint64_t gen_msg_id() 
{
	static std::atomic<uint64_t> uni_msg_id(0);
	return ++uni_msg_id;
}

CB3Msg::CB3Msg(uint32_t nameid) : id_(gen_msg_id()), nameid_(nameid)
{}

std::unique_ptr<CB3Msg> CB3Msg::move()
{
	return std::unique_ptr<CB3Msg>(new CB3Msg(std::move(*this)));
}

}
