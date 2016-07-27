#include "b3_log.h"
#include <map>
#include <algorithm>

namespace B3LIB
{

static Logger logger;

int log_open(FILE* fp, int level, bool is_threadsafe)
{
	return logger.open(fp, level, is_threadsafe);
}

int log_open(const char* filename, int level, bool is_threadsafe, uint64_t rotate_size)
{
	return logger.open(filename, level, is_threadsafe, rotate_size);
}

int log_level()
{
	return logger.level();
}

void set_log_level(int level)
{
	logger.set_level(level);
}

static std::map<std::string, int> log_level_name_num_map = {
	{ "fatal", kLogLevelFatal },
	{ "error", kLogLevelError },
	{ "warn",  kLogLevelWarn  },
	{ "info",  kLogLevelInfo  },
	{ "debug", kLogLevelDebug },
	{ "trace", kLogLevelTrace }
};
static std::map<int, std::string> log_level_num_name_map = {
	{ kLogLevelFatal, "fatal" },
	{ kLogLevelError, "error" },
	{ kLogLevelWarn,  "warn"  },
	{ kLogLevelInfo,  "info"  },
	{ kLogLevelDebug, "debug" },
	{ kLogLevelTrace, "trace" }
};

void set_log_level(const char* s)
{
	std::string ss(s);
	std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);	
	logger.set_level(log_level_name_num_map[ss]);
}

int log_write(int level, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = logger.logv(level, fmt, ap);
	va_end(ap);
	return ret;
}

/********************************************************************/

Logger* Logger::shared()
{
	return &logger;
}

Logger::Logger()
	: fp_(stdout), level_(kLogLevelDebug), lock_(nullptr), rotate_size_(0)
{}

Logger::~Logger()
{
	if (lock_) {
		pthread_mutex_destroy(lock_);
		free(lock_);
	}
	this->close();
}

std::string Logger::level_name()
{
	return log_level_num_name_map[level_];
}

void Logger::threadsafe()
{
	if (lock_) {
		pthread_mutex_destroy(lock_);
		free(lock_);
		lock_ = nullptr;
	}
	lock_ = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lock_, nullptr);
}

int Logger::open(FILE* fp, int level, bool is_threadsafe)
{
	this->fp_ = fp;
	this->level_ = level;
	if (is_threadsafe) {
		this->threadsafe();
	}
	return 0;
}

int Logger::open(const char* filename, int level, bool is_threadsafe, uint64_t rotate_size)
{
	if (strlen(filename) > PATH_MAX - 20) {
		fprintf(stderr, "log filename too long!");
		return -1;
	}
	this->level_ = level;
	this->rotate_size_ = rotate_size;
	strcpy(this->filename_, filename);

	FILE* fp = nullptr;
	if (strcmp(filename, "stdout") == 0) {
		fp = stdout;
	} else if (strcmp(filename, "stderr") == 0) {
		fp = stderr;
	} else {
		fp = fopen(filename, "a");
		if (fp == NULL) {
			return -1;
		}

		struct stat st;
#ifndef WIN32
		int ret = fstat(fileno(fp), &st);
#else
		int ret = fstat(_fileno(fp), &st);
#endif		
		if (ret == -1) {
			fprintf(stderr, "fstat log file %s error!", filename);
			return -1;
		} else {
			stats.w_curr = st.st_size;
		}
	}
	return this->open(fp, level, is_threadsafe);
}

void Logger::close(void)
{
	if (fp_ != stdin && fp_ != stdout) {
		fclose(fp_);
	}
}

void Logger::rotate()
{
	fclose(fp_);
	char newpath[PATH_MAX];
	time_t time;
	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	tm = localtime(&time);
	sprintf(newpath, "%s.%04d%02d%02d-%02d%02d%02d",
			this->filename_,
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	int ret = rename(this->filename_, newpath);
	if (ret == -1) {
		return;
	}
	fp_ = fopen(this->filename_, "a");
	if (fp_ == NULL) {
		return;
	}
	stats.w_curr = 0;
}

int Logger::get_level(const char* levelname)
{
	return log_level_name_num_map[levelname];
}

#define LEVEL_NAME_LEN	8
#define LOG_BUF_LEN		4096

inline static const char* get_level_name(int level)
{
	std::string name = log_level_num_name_map[level];
	name.resize(LEVEL_NAME_LEN - 3, ' ');
	return ("[" + name + "] ").c_str();
}

int Logger::logv(int level, const char* fmt, va_list ap)
{
	if (logger.level_ < level) {
		return 0;
	}

	char buf[LOG_BUF_LEN] = {0};
	int len = 0;
	char* ptr = buf;

	time_t time;
	struct timeval tv;
	struct tm* tms;
	gettimeofday(&tv, nullptr);
	time = tv.tv_sec;
	tms = localtime(&time);
	/* %3ld 在数值位数超过3位的时候不起作用, 所以这里转成int */
	len = sprintf(ptr, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
				  tms->tm_year + 1900, tms->tm_mon + 1, tms->tm_mday,
				  tms->tm_hour, tms->tm_min, tms->tm_sec, (int)(tv.tv_usec / 1000));
	if (len < 0) {
		return -1;
	}
	ptr += len;

	memcpy(ptr, get_level_name(level), LEVEL_NAME_LEN);
	ptr += LEVEL_NAME_LEN;

	int space = sizeof(buf) - (ptr - buf) - 10;
	len = vsnprintf(ptr, space, fmt, ap);
	if (len < 0) {
		return -1;
	}
	ptr += len > space ? space : len;
	*ptr++ = '\n';
	*ptr = '\0';

	len = ptr - buf;
	// change to write(), without locking?
	if (this->lock_) {
		pthread_mutex_lock(this->lock_);
	}
	fwrite(buf, len, 1, this->fp_);
	fflush(this->fp_);

	stats.w_curr += len;
	stats.w_total += len;
	if (rotate_size_ > 0 && stats.w_curr > rotate_size_) {
		this->rotate();
	}
	if (this->lock_) {
		pthread_mutex_unlock(this->lock_);
	}

	return len;
}

#define LOG_TYPE_MACRO(l)					\
	do {									\
		va_list ap;							\
		va_start(ap, fmt);					\
		int ret = logger.logv(l, fmt, ap);	\
		va_end(ap);							\
		return ret;							\
	} while(0)	

int Logger::trace(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelTrace);
}

int Logger::debug(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelDebug);
}

int Logger::info(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelInfo);
}

int Logger::warn(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelWarn);
}

int Logger::error(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelError);
}

int Logger::fatal(const char* fmt, ...)
{
	LOG_TYPE_MACRO(kLogLevelFatal);
}

}
