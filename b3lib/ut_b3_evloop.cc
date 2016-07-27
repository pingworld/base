#include "b3_evloop.h"
#include "b3_log.h"

using namespace B3LIB;

struct test_data {
	std::string  name_;
};
using test_data_t = struct test_data;

static void test_proc(evutil_socket_t fd, short what, void* d)
{
	log_trace("fd:%d, what:%u", fd, what);
	CB3EventLoop* loop = static_cast<CB3EventLoop*>(d);
	std::unique_ptr<CB3Msg> msg = loop->wait();
	log_trace("msg, nameid:%u", msg->nameid());
	if (msg->nameid() == 1) {
		auto& msg1 = dynamic_cast<CB3DataMsg<std::string>&>(*msg);
		log_trace("msg, string:%s", msg1.data().c_str());
	}
}

int main()
{
	log_open("b3_loop.log", kLogLevelTrace, true, 1024 * 1024);

	CB3EventLoop loop(test_proc);
	loop.start();

	CB3DataMsg<std::string> msg1(1, "hello-evloop");
	loop.notify(std::move(msg1));

	sleep(1000);

	return 0;
}
