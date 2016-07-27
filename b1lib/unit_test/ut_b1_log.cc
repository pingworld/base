#include "b1_log.h"
#include "gtest/gtest.h"

TEST(test_log_suite, common)
{
    log_open("b1.log", Logger::LEVEL_TRACE, true, 1024*1024);
    log_trace("hello b1log");
    log_debug("hello b1log");
    log_error("hello b1log");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
