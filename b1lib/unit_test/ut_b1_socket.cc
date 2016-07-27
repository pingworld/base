#include "b1_socket.h"
#include "gtest/gtest.h"

TEST(b1_socket_suite, ip_addr_helper)
{
    int rc = 0;

//     std::vector<std::string> sip_vec;
//     sip_vec.push_back("127.0.0.1");
//     sip_vec.push_back("0.0.0.0");

    // 1. ip in string, convert into struct in_addr;
    //std::string sip = "127.0.0.1";
    std::string sip = "0.0.0.0";
    uint64_t u64_ip = 0;
    rc = b1_sock_addr_str_to_ulong(sip, u64_ip);
    ASSERT_EQ(rc, 0);
 
    // 2. ip in int, convert into string format;
    std::string sip_cmp;
    rc = b1_sock_addr_ulong_to_str(u64_ip, sip_cmp);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(sip_cmp, sip);

    printf("u64_ip: %lu\n", u64_ip);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
