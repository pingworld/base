#ifndef _B1_UTILS_H_
#define _B1_UTILS_H_

#ifdef LINUX
#include <unistd.h>
#include <sys/syscall.h>
#   define  b1_getpid()     syscall(SYS_gettid)
#else
#   define  b1_getpid()     0
#endif

#include <stdint.h>
#include <vector>
#include <string>

namespace b1lib
{
namespace b1_utils
{

class noncopyable {
protected:
	noncopyable() = default;
	~noncopyable() = default;

	noncopyable(const noncopyable&) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
};

class B1Utility {
public:
	static int32_t parse_fields_from_line(const std::string& from_line, const char split_char, std::vector<std::string>& fields);
};

}
}

#endif
