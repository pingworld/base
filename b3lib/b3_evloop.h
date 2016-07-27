#ifndef __B3_EVLOOP_H__
#define __B3_EVLOOP_H__

#include <memory>
#include <thread>
#include "event2/event.h"
#include "event2/event_struct.h"
#include "b3_mq.h"
#include "b3_msg.h"

#ifdef WIN32
#else
#include <unistd.h>
#endif

namespace B3LIB
{

using b3_thread_id_t = std::thread::id;

using b3_evbase_t = std::unique_ptr<struct event_base, decltype(&event_base_free)>;
using b3_event_t  = std::unique_ptr<struct event, decltype(&event_free)>;

using b3_mq_t = std::unique_ptr<CB3MsgQueue>;

// 当某事件发生时的回调函数
using b3_event_cb_t = void(*)(evutil_socket_t, short, void*);

class CB3EventLoop {
public:
	CB3EventLoop(b3_event_cb_t cb) 
		: notify_evt_{nullptr, event_free}, evbase_{nullptr, event_base_free}
		, mq_{new CB3MsgQueue()}, cb_(cb) {
	}
	~CB3EventLoop() = default;
	CB3EventLoop(const CB3EventLoop&) = delete;
	CB3EventLoop& operator=(const CB3EventLoop&) = delete;

	int32_t start() {
#ifdef WIN32
		struct sockaddr_in serv_addr;
		int sockfd;
		if ((sockfd = createLocalListSock(&serv_addr)) < 0) return -1;
		if (createLocalSocketPair(sockfd, notify_fds_, &serv_addr) == -1) return -2;
#else
		if (pipe(notify_fds_)) return -1;
#endif
		evbase_.reset(event_base_new());
		notify_evt_.reset(event_new(evbase_.get(), notify_fds_[NTY_FD_RECV], 
									EV_READ | EV_PERSIST, cb_,
									static_cast<void*>(this)));
		if (event_add(notify_evt_.get(), nullptr) != 0) return -3;
		looper_.reset(new std::thread([=] { run(); }));
	}

	// 通知当前事件循环有特定事件发生
	void notify(CB3Msg&& msg) {		
		mq_->push(std::move(msg));
		char c = '1';
#ifdef WIN32
		if (mem_write(notify_fds_[NTY_FD_SEND], &c, 1) != 1) {
#else
		if (write(notify_fds_[NTY_FD_SEND], &c, 1) != 1) {
#endif
			// TODO
			// throw ex??
			// pop the msg with uid??
		}
	}

	// 阻塞等待当前事件循环中的特定事件
	std::unique_ptr<CB3Msg> wait() {
		char c;
#ifdef WIN32
		if (mem_read(notify_fds_[NTY_FD_RECV], &c, 1) != 1) {
#else
		if (read(notify_fds_[NTY_FD_RECV], &c, 1) != 1) {
#endif
			printf("[fatal] can't read char from pipe");
			// how todo??
		}
		return mq_->pop();
	}

private:
	void run() {
		event_base_dispatch(evbase_.get());
	}
	
	// 确保某些接口是运行在当前事件循环中的
	void assert_loop_thread() {
	}
		
private:
	enum {NTY_FD_RECV = 0, NTY_FD_SEND = 1};
	std::unique_ptr<std::thread>	looper_;
	int								notify_fds_[2];
	b3_event_t						notify_evt_;
	b3_evbase_t						evbase_;
	b3_mq_t							mq_;
	b3_event_cb_t					cb_;
};

}

#endif
