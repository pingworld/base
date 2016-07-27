#ifndef __B3_LOG_H__
#define __B3_LOG_H__

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <string>

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include "win32/win32.h"
#endif

namespace B3LIB {

static const int kLogLevelNone = -1;
static const int kLogLevelMin = 0;
static const int kLogLevelFatal = 0;
static const int kLogLevelError = 1;
static const int kLogLevelWarn = 2;
static const int kLogLevelInfo = 3;
static const int kLogLevelDebug = 4;
static const int kLogLevelTrace = 5;
static const int kLogLevelMax = 5;

class Logger {
public:
	static int get_level(const char* name);
	static Logger* shared();

	std::string level_name();
	inline std::string output_name() const { return filename_; }
	inline uint64_t rotate_size() const { return rotate_size_; }

private:
	FILE* fp_;
	char filename_[PATH_MAX] = { 0 };
	int level_;
	pthread_mutex_t* lock_;
	uint64_t rotate_size_;
	struct {
		uint64_t w_curr;
		uint64_t w_total;
	} stats = { 0, 0 };

	void rotate();
	void threadsafe();

public:
	Logger();
	~Logger();

	inline int level() const
	{
		return level_;
	}

	inline void set_level(int level)
	{
		this->level_ = level;
	}

	int open(FILE* fp, int level = kLogLevelDebug, bool is_threadsafe = false);
	int open(const char* filename, int level = kLogLevelDebug,
			 bool is_threadsafe = false, uint64_t rotate_size = 0);
	void close(void);

	int logv(int level, const char* fmt, va_list ap);

	int trace(const char* fmt, ...);
	int debug(const char* fmt, ...);
	int info(const char* fmt, ...);
	int warn(const char* fmt, ...);
	int error(const char* fmt, ...);
	int fatal(const char* fmt, ...);
};

// interface for c
int log_open(FILE* fp, int level = kLogLevelDebug, bool is_threadsafe = false);
int log_open(const char* filename, int level = kLogLevelDebug,
			 bool is_threadsafe = false, uint64_t rotate_size = 0);
int log_level();
void set_log_level(int level);
void set_log_level(const char* s);
int log_write(int level, const char* fmt, ...);

#ifdef NDEBUG
#define log_trace(fmt, args...) do{}while(0)
#else
#define log_trace(fmt, ...)	\
	log_write(kLogLevelTrace, "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#define log_debug(fmt, ...)	\
	log_write(kLogLevelDebug, "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#define log_info(fmt, ...)	\
	log_write(kLogLevelInfo,  "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#define log_warn(fmt, ...)	\
	log_write(kLogLevelWarn,  "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#define log_error(fmt, ...)	\
	log_write(kLogLevelError, "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#define log_fatal(fmt, ...)	\
	log_write(kLogLevelFatal, "%s(%d): " fmt, __MYFILE__, __LINE__, ##__VA_ARGS__)
#endif

}


#endif
