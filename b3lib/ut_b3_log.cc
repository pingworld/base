#include "b3_log.h"
#include "gtest/gtest.h"

using namespace B3LIB;

TEST(test_log_suite, common)
{
	log_open("b3.log", kLogLevelTrace, true, 1024 * 1024);
	log_trace("hello log trace");
	log_debug("hello log debug");
	log_info("hello log info");
	log_error("hello log error");
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}