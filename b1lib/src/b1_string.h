#ifndef _B1_STRING_H_
#define _B1_STRING_H_

#include <vector>
#include <string>
#include <cstdint>

typedef std::vector<char> b1_string_t;

const char *b1_string_to_cstring(const b1_string_t &bs);
b1_string_t cstring_to_b1_string(const char *cs, size_t cs_len);

b1_string_t cppstring_to_b1_string(const std::string &from);
std::string b1_string_to_cppstring(const b1_string_t &from);

const char *b1_string_to_bytes(const b1_string_t &bs);
b1_string_t bytes_to_b1_string(const char *bytes, size_t bytes_len);

b1_string_t b1_string_read_int8(const b1_string_t &bs, int8_t &ou);
b1_string_t b1_string_read_uint8(const b1_string_t &bs, uint8_t &ou);
b1_string_t b1_string_read_int16(const b1_string_t &bs, int16_t &ou);
b1_string_t b1_string_read_uint16(const b1_string_t &bs, uint16_t &ou);
b1_string_t b1_string_read_int32(const b1_string_t &bs, int32_t &ou);
b1_string_t b1_string_read_uint32(const b1_string_t &bs, uint32_t &ou);
b1_string_t b1_string_read_std_string(const b1_string_t &bs, char *os, size_t os_len);

b1_string_t b1_string_write_int8(int8_t iu);
b1_string_t b1_string_write_uint8(uint8_t iu);
b1_string_t b1_string_write_int16(int16_t iu);
b1_string_t b1_string_write_uint16(uint16_t iu);
b1_string_t b1_string_write_int32(int32_t iu);
b1_string_t b1_string_write_uint32(uint32_t iu);
b1_string_t b1_string_write_std_cstring(const char *ins, size_t ins_len);
b1_string_t b1_string_write_std_cppstring(const std::string &ins);

#endif
