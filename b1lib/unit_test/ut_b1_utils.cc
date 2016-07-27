#include "b1_log.h"
#include "b1_utils.h"
#include "gtest/gtest.h"

TEST(test_utils_suite, common)
{
	std::string lines = "123#456#789#089";
	std::vector<std::string> fields;
	
	b1lib::b1_utils::B1Utility::parse_fields_from_line(lines, '#', fields);
	ASSERT_EQ(fields.size(), 4);
	ASSERT_EQ(fields[0], "123");
	ASSERT_EQ(fields[1], "456");
	ASSERT_EQ(fields[2], "789");
	ASSERT_EQ(fields[3], "089");
	//printf("0:%s, 1:%s, 2:%s\n", fields[0].c_str(), fields[1].c_str(), fields[2].c_str());

	lines = "##";
	fields.clear();
	b1lib::b1_utils::B1Utility::parse_fields_from_line(lines, '#', fields);
	ASSERT_EQ(fields.size(), 0);

	lines = "123##";
	fields.clear();
	b1lib::b1_utils::B1Utility::parse_fields_from_line(lines, '#', fields);
	ASSERT_EQ(fields.size(), 1);
	ASSERT_EQ(fields[0], "123");

	lines = "123##345#678";
	fields.clear();
	b1lib::b1_utils::B1Utility::parse_fields_from_line(lines, '#', fields);
	ASSERT_EQ(fields.size(), 3);
	ASSERT_EQ(fields[0], "123");
	ASSERT_EQ(fields[1], "345");
	ASSERT_EQ(fields[2], "678");
	//printf("0:%s, 1:%s, 2:%s\n", fields[0].c_str(), fields[1].c_str(), fields[2].c_str());

	lines = "#123###345#678#";
	fields.clear();
	b1lib::b1_utils::B1Utility::parse_fields_from_line(lines, '#', fields);
	ASSERT_EQ(fields.size(), 3);
	ASSERT_EQ(fields[0], "123");
	ASSERT_EQ(fields[1], "345");
	ASSERT_EQ(fields[2], "678");
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
