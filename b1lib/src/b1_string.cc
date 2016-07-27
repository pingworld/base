#include "b1_string.h"
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>

const char *b1_string_to_cstring(const b1_string_t &bs) 
{
	if (bs.size() == 0) return nullptr;

	std::string std_s(bs.begin(), bs.end());
	return std_s.c_str();
}

b1_string_t cstring_to_b1_string(const char *cs, size_t cs_len)
{
	b1_string_t ccl_s(cs, cs + cs_len);
	return ccl_s;
}

std::string b1_string_to_cppstring(const b1_string_t &from)
{
	std::string std_s(from.begin(), from.end());
	return std_s;
}

b1_string_t cppstring_to_b1_string(const std::string &from)
{
	b1_string_t ccl_s(from.c_str(), from.c_str() + from.length());
	return ccl_s;
}

const char *b1_string_to_bytes(const b1_string_t &bs)
{
	if (bs.size() == 0) return nullptr;

	char *ret = new char[bs.size()];
	if (!ret) return nullptr;
	memset(ret, 0, bs.size());
	memcpy(ret, &bs[0], bs.size());

	return ret;
}

b1_string_t bytes_to_b1_string(const char *bytes, size_t bytes_len)
{
	return cstring_to_b1_string(bytes, bytes_len);
}

b1_string_t b1_string_read_uint16(const b1_string_t &bs, uint16_t &ou)
{
	try	{
		ou = std::stoi(b1_string_to_cppstring(bs), nullptr, 10);
	} catch (std::exception &e)	{
		ou = 0;
	}	
	return bs;
}

b1_string_t b1_string_read_uint32(const b1_string_t &bs, uint32_t &ou)
{
	try {
		ou = std::stoi(b1_string_to_cppstring(bs), nullptr, 10);
	} catch (std::exception &e) {
		ou = 0;
	}
	return bs;
}

b1_string_t b1_string_read_std_string(const b1_string_t &bs, char *os, size_t os_len)
{
	if ((bs.size() < os_len) || !os) return bs;
	memcpy(os, &bs[0], os_len);
	b1_string_t bs_ret(bs.begin() + os_len, bs.end());
	return bs;
}

b1_string_t b1_string_write_uint16(uint16_t iu)
{
	return cppstring_to_b1_string(std::to_string(iu));
}

b1_string_t b1_string_write_uint32(uint32_t iu)
{
	return cppstring_to_b1_string(std::to_string(iu));
}

b1_string_t b1_string_write_std_cstring(const char *ins, size_t ins_len)
{
	b1_string_t bs;
	if (!ins || (ins_len <= 0)) return bs;

	for (size_t i = 0; i < ins_len; ++i) {
		bs.push_back(ins[i]);
	}
	return bs;
}

b1_string_t b1_string_write_std_cppstring(const std::string &ins)
{
	return b1_string_write_std_cstring(ins.c_str(), ins.length());
}
