#include "b1_log.h"
#include "gtest/gtest.h"

TEST(test_log_suite, common)
{
	log_open("b1.log", Logger::LEVEL_TRACE, true, 1024 * 1024);
	log_trace("hello libb1.log1");
	log_debug("hello libb1.log2");
	log_error("hello libb1.log3");
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
