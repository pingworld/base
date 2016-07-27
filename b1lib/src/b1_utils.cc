#include "b1_utils.h"

namespace b1lib
{
namespace b1_utils
{

int32_t
B1Utility::parse_fields_from_line(const std::string& from_line, const char split_char, 
								  std::vector<std::string>& fields)
{
	if (from_line.length() == 0) { return 0; }

	int index = 0, sym_pos = 0;
	std::string tmp_line = from_line;
	while (sym_pos != -1) {
		sym_pos = from_line.find_first_of(split_char, index);
		std::string field = from_line.substr(index, sym_pos - index);

		if (field.length() != 0) {
			fields.push_back(field);
		}

		index = sym_pos + 1;
	}

	return fields.size();
}

}
}
