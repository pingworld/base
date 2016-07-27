#ifndef __B3_MQ_H__
#define __B3_MQ_H__

#include <queue>
#include <mutex>
#include <thread>
#include <memory>

namespace B3LIB
{

class CB3Msg;
class CB3MsgQueue {
public:
	CB3MsgQueue() = default;
	// The destructors of the elements are called and the used storage is deallocated.
	~CB3MsgQueue() = default;

	void push(CB3Msg&&);
	std::unique_ptr<CB3Msg> pop();

	CB3MsgQueue(const CB3MsgQueue&) = delete;
	CB3MsgQueue& operator=(const CB3MsgQueue&) = delete;

private:
	std::queue<std::unique_ptr<CB3Msg>> queue_;
	std::mutex lock_;
};

}

#endif
