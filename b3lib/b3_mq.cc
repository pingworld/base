#include "b3_mq.h"
#include "b3_msg.h"

namespace B3LIB
{

void CB3MsgQueue::push(CB3Msg&& msg)
{
	std::lock_guard<std::mutex> scp_lock(lock_);
	queue_.push(msg.move());
}

std::unique_ptr<CB3Msg> CB3MsgQueue::pop()
{
	std::lock_guard<std::mutex> scp_lock(lock_);
	if (queue_.empty()) return nullptr;
	auto msg = queue_.front()->move();
	queue_.pop();
	return msg;
}

}
