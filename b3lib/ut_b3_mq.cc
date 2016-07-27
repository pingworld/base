#include "b3_mq.h"
#include "b3_msg.h"
#include "b3_log.h"
#include <thread>
#include <cstdio>
#include "gtest/gtest.h"

using namespace B3LIB;

class CB3MsgTest {
public:
	explicit CB3MsgTest(const std::string& d) : data_(d) {}
	~CB3MsgTest() {
		std::cout << "~CB3MsgTest(" << data_ << ")" << std::endl;
	}

	std::string data() const { return data_; }

private:
	std::string	data_;
};

struct CB3SimpleMsg {
	int i_;
	std::string s_;
	double d_;
	std::shared_ptr<CB3MsgTest> sp_;
};

TEST(test_mq, common)
{
	log_open("b3_mq.log", kLogLevelTrace, true, 1024 * 1024);

	CB3DataMsg<CB3MsgTest> msg1(1, "test");
	uint64_t msg1_id = msg1.id();

	CB3DataMsg<std::string> msg2(2, "hello");
	uint64_t msg2_id = msg2.id();

	CB3DataMsg<CB3SimpleMsg> msg3(3);
	msg3.data().i_ = 10;
	msg3.data().s_ = "10";
	msg3.data().d_ = 1.0;
	msg3.data().sp_  = std::shared_ptr<CB3MsgTest>(new CB3MsgTest("10"));

	uint64_t msg3_id = msg3.id();

	CB3MsgQueue mq;
	mq.push(std::move(msg1));
	mq.push(std::move(msg2));
	mq.push(std::move(msg3));

	auto msg = mq.pop();
	// 这地方不太好用 -- C++如果支持反射就完美了。。只能用一个消息名字来标识了
	auto& msg1_1 = dynamic_cast<CB3DataMsg<CB3MsgTest>&>(*msg);
	ASSERT_STREQ("test", msg1_1.data().data().c_str());

	msg = mq.pop();
	auto& msg2_1 = dynamic_cast<CB3DataMsg<std::string>&>(*msg);
	ASSERT_STREQ("hello", msg2_1.data().c_str());

	std::unique_ptr<CB3Msg> smsg = mq.pop();
	auto& msg3_1 = dynamic_cast<CB3DataMsg<CB3SimpleMsg>&>(*smsg);
	ASSERT_EQ(10, msg3_1.data().i_);
	ASSERT_STREQ("10", msg3_1.data().s_.c_str());
	ASSERT_EQ(1.0, msg3_1.data().d_);
}

void fpush(CB3MsgQueue& mq)
{
	for (int i = 0; i < 10000; ++i) {
		CB3DataMsg<std::string> msg(i, std::to_string(i));
		mq.push(std::move(msg));
		log_trace("-- push %d --", i);
	}
	log_trace("fpush finished!");
}

void fpop(CB3MsgQueue& mq)
{
	int i = 0;
	while (i < 20000) {
		auto msg = mq.pop();
		if (msg) { 
			++i;
			auto& dmsg = dynamic_cast<CB3DataMsg<std::string>&>(*msg);
			log_trace("++ pop %s ++", dmsg.data().c_str());
		}
	}
	log_trace("fpop finished!");
}

TEST(test_mq, mthreads)
{
	return;
	CB3MsgQueue mq;
	std::thread t1(fpush, std::ref(mq));
	std::thread t2(fpush, std::ref(mq));
	std::thread t3(fpop, std::ref(mq));
	t1.join();
	t2.join();
	t3.join();
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
