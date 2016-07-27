#include "b1_string.h"
#include "gtest/gtest.h"

TEST(b1_string_suite, test_cs)
{
	std::string cs("hello-b1-string");
	b1_string_t cs_b1 = cstring_to_b1_string(cs.c_str(), cs.length());
	const char *cs_c = b1_string_to_cstring(cs_b1);
	ASSERT_EQ(0, strcmp(cs.c_str(), cs_c));
}

TEST(b1_string_suite, test_csb)
{
	b1_string_t csb;
	csb.push_back((char)1);
	csb.push_back((char)2);
	csb.push_back((char)3);
	csb.push_back((char)4);
	csb.push_back((char)5);
	const char *csb_c = b1_string_to_cstring(csb);
	char *csb_c_cmp = new char[5];
	memset(csb_c_cmp, 0, 5);
	for (int i = 0; i < 5; ++i) {
		csb_c_cmp[i] = i + 1;
	}
	ASSERT_EQ(0, strcmp(csb_c, csb_c_cmp));
}

TEST(b1_string_suite, test_csb_2)
{
	b1_string_t csb;
	csb.push_back((char)1);
	csb.push_back((char)2);
	csb.push_back((char)3);
	csb.push_back((char)4);
	csb.push_back((char)5);
	const char *csb_c = b1_string_to_cstring(csb);
	char *csb_c_cmp = new char[5];
	memset(csb_c_cmp, 0, 5);
	for (int i = 0; i < 5; ++i) {
		csb_c_cmp[i] = i + 1;
	}
	b1_string_t csb_cmp = cstring_to_b1_string(csb_c_cmp, 5);
	ASSERT_EQ(csb.size(), csb_cmp.size());
	ASSERT_EQ(csb_c_cmp[0], csb_cmp[0]);
	ASSERT_EQ(csb_c_cmp[1], csb_cmp[1]);
	ASSERT_EQ(csb_c_cmp[2], csb_cmp[2]);
	ASSERT_EQ(csb_c_cmp[3], csb_cmp[3]);
	ASSERT_EQ(csb_c_cmp[4], csb_cmp[4]);
}

TEST(b1_string_suite, test_csb_3)
{
	b1_string_t csb1;
	const char *csb1_c = b1_string_to_cstring(csb1);
	ASSERT_EQ(csb1_c, nullptr);

	std::string csb2_c("");
	b1_string_t csb2 = cppstring_to_b1_string(csb2_c);
	ASSERT_EQ(csb2.size(), 0);
}

TEST(b1_string_suite, test_csb_4)
{
	char cs[6] = {1, 2, 0, 1, 2, 3};
	b1_string_t b1s = bytes_to_b1_string(&cs[0], 6);
	ASSERT_EQ(b1s.size(), 6);
	ASSERT_EQ(b1s[0], cs[0]);
	ASSERT_EQ(b1s[1], cs[1]);
	ASSERT_EQ(b1s[2], cs[2]);
	ASSERT_EQ(b1s[3], cs[3]);
	ASSERT_EQ(b1s[4], cs[4]);
	ASSERT_EQ(b1s[5], cs[5]);

	const char *cs1 = b1_string_to_bytes(b1s);
	ASSERT_EQ(cs1[0], cs[0]);
	ASSERT_EQ(cs1[1], cs[1]);
	ASSERT_EQ(cs1[2], cs[2]);
	ASSERT_EQ(cs1[3], cs[3]);
	ASSERT_EQ(cs1[4], cs[4]);
	ASSERT_EQ(cs1[5], cs[5]);
}

TEST(b1_string_suite, test_csb_5)
{
	const char *cs = nullptr;
	b1_string_t b1s = bytes_to_b1_string(cs, 0);
	ASSERT_EQ(b1s.size(), 0);

	const char *cs1 = b1_string_to_bytes(b1s);
	ASSERT_EQ(cs1, nullptr);
}

TEST(b1_string_suite, test_rw16)
{
	uint16_t iu = 0x0a0b;
	b1_string_t cbs = b1_string_write_uint16(iu);
	uint16_t cmp_iu = 0;
	b1_string_read_uint16(cbs, cmp_iu);
	ASSERT_EQ(iu, cmp_iu);

	char cs[6] = { 1, 2, 0, 1, 2, 3 };
	b1_string_t b1s = bytes_to_b1_string(&cs[0], 6);
	b1_string_read_uint16(b1s, cmp_iu);
}

TEST(b1_string_suite, test_rw32)
{
	uint32_t iu = 0x0a0b0c0d;
	b1_string_t cbs = b1_string_write_uint32(iu);
	uint32_t cmp_iu = 0;
	b1_string_read_uint32(cbs, cmp_iu);
}

TEST(b1_string_suite, test_rstr)
{
	std::string cs("hello-b1-string");
	b1_string_t cs_b1 = cstring_to_b1_string(cs.c_str(), cs.length());

	size_t cs_b1_cstr_len = cs_b1.size();
	char *cs_b1_cstr = new char[cs_b1_cstr_len];
	ASSERT_NE(cs_b1_cstr, nullptr);

	b1_string_read_std_string(cs_b1, cs_b1_cstr, cs_b1_cstr_len);
	std::string cs_b1_cppstr = std::string(cs_b1_cstr);
	ASSERT_EQ(cs_b1_cppstr.length(), cs.length());
	ASSERT_EQ(cs_b1_cppstr, cs);
}

TEST(b1_string_suite, test_wstr)
{
	std::string cs("hello-b1-string");

	b1_string_t cs_b1 =	b1_string_write_std_cstring(cs.c_str(), cs.length());
	ASSERT_EQ(cs_b1.size(), cs.length());
	ASSERT_EQ(cs_b1[0], cs[0]);
	ASSERT_EQ(cs_b1[1], cs[1]);
	ASSERT_EQ(cs_b1[2], cs[2]);
	ASSERT_EQ(cs_b1[3], cs[3]);
	ASSERT_EQ(cs_b1[4], cs[4]);
	ASSERT_EQ(cs_b1[5], cs[5]);
}

TEST(b1_string_suite, test_string_cmp)
{
	std::string cs("111222333");
	b1_string_t cs_b1 = cppstring_to_b1_string(cs);
	ASSERT_EQ(memcmp(cs.c_str(), &cs_b1[0], cs.length()), 0);

	b1_string_t cs_b2 = b1_string_write_std_cppstring(cs);
	ASSERT_EQ(memcmp(cs.c_str(), &cs_b2[0], cs.length()), 0);
}

TEST(b1_string_suite, test_string_json)
{
	std::string cs("{\"_CMD_\":1001,\"_CH_\":0,\"_AIM_\":\"__press_client_002__\"}");
	b1_string_t cs_b1 = cppstring_to_b1_string(cs);
	ASSERT_EQ(cs_b1[0], '{');
	ASSERT_EQ(cs_b1[1], '"');
	ASSERT_EQ(cs_b1[2], '_');
	ASSERT_EQ(cs.length(), cs_b1.size());
	ASSERT_EQ(cs_b1.size(), 54);
}

TEST(b1_string_suite, test_string_nonseeasc)
{
	// 110;010000001;29048;1454122225;2;0
	char cs[] = { 8, '1', '1', '0', ';', '0', '1' };
	b1_string_t cs_b1 = cstring_to_b1_string(cs, 7);
	ASSERT_EQ(cs_b1[0], 8);
	ASSERT_EQ(cs_b1[1], '1');
	ASSERT_EQ(cs_b1[2], '1');
	ASSERT_EQ(cs_b1[3], '0');
	ASSERT_EQ(cs_b1[4], ';');
	ASSERT_EQ(cs_b1[5], '0');
	ASSERT_EQ(cs_b1[6], '1');
	std::string cs_cpp = std::string(cs + 1, 6);
	printf("cs_cpp:%s\n", cs_cpp.c_str());
	const char *pcs = cs;
	std::string pcs_cpp = std::string(pcs + 1, 6);
	printf("pcs_cpp:%s\n", pcs_cpp.c_str());
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
