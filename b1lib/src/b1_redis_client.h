#ifndef _B1_REDIS_CLIENT_H_
#define _B1_REDIS_CLIENT_H_

/**
 * The client of redis based on hiredis
 */

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "b1_utils.h"
#include "b1_string.h"
#include "hiredis/hiredis.h"

using namespace b1lib;

#define ASSERT_CMDS_PARAM_VALID1(n)											\
	do { if (n.length() == 0) return 0; } while(0)
#define ASSERT_CMDS_PARAM_VALID2(n, k)										\
	do { if (n.length() == 0 || k.length() == 0) return 0; } while(0)

/**
 * 有些错误是没法处理的，比如：
 * 1. 未知命令；
 * 2. IO读写错误；
 * 3. 服务器内存错误；
 */
#define MAX_REDIS_RETRY_TIMES	10
#define CHECK_REDIS_REPLY(r, cmd)											\
	do {																	\
		int retry_times = 0;												\
		while (retry_times++ < MAX_REDIS_RETRY_TIMES) {						\
			if (r.type() == reply::type_t::ERROR) {							\
				printf("redis reply error:%s\n", r.str().c_str());			\
				if (r.err_type() == REDIS_ERR_EOF) {						\
					if (redisReconnect(c_) == REDIS_OK) {					\
						printf("reconnect redis succ.\n");					\
						retry_times = 0;									\
						r = run(cmd);										\
						if (r.type() != reply::type_t::ERROR) {				\
							break;											\
						}													\
					}														\
				} else {													\
					printf("can't process such kind of error.\n");			\
					return -1;												\
				}															\
			} else {														\
				break;														\
			}																\
		}																	\
		if (retry_times >= MAX_REDIS_RETRY_TIMES) return -1;					\
	} while (0)

class reply {
public:
	/**
	* @brief Define reply type
	*/
	enum class type_t {
		STRING = 1,
		ARRAY = 2,
		INTEGER = 3,
		NIL = 4,
		STATUS = 5,
		ERROR = 6
	};

	/**
	* @brief Type of reply, other field values are dependent of this
	* @return
	*/
	inline type_t type() const { return _type; }
	/**
	* @brief Returns string value if present, otherwise an empty string
	* @return
	*/
	inline const std::string& str() const { return _str; }
	/**
	* @brief Returns integer value if present, otherwise 0
	* @return
	*/
	inline long long integer() const { return _integer; }
	/**
	* @brief Returns a vector of sub-replies if present, otherwise an empty one
	* @return
	*/
	inline const std::vector<reply>& elements() const { return _elements; }

	inline operator const std::string&() const { return _str; }

	inline operator long long() const { return _integer; }

	inline void set_err_type(int err) { _err_type = err; }
	inline const int err_type() const { return _err_type; }

	inline bool operator==(const std::string& rvalue) const
	{
		if (_type == type_t::STRING || _type == type_t::ERROR || 
			_type == type_t::STATUS) {
			return _str == rvalue;
		} else {
			return false;
		}
	}

	inline bool operator==(const long long rvalue) const
	{
		if (_type == type_t::INTEGER) {
			return _integer == rvalue;
		} else {
			return false;
		}
	}

private:
	reply(redisReply *c_reply) 
		: _type(type_t::ERROR), _integer(0), _err_type(0) {
		if (c_reply) {
			_type = static_cast<type_t>(c_reply->type);
			switch (_type) {
			case type_t::ERROR:
			case type_t::STRING:
			case type_t::STATUS:
				_str = std::string(c_reply->str, c_reply->len);
				break;
			case type_t::INTEGER:
				_integer = c_reply->integer;
				break;
			case type_t::ARRAY:
				for (size_t i = 0; i < c_reply->elements; ++i) {
					_elements.push_back(reply(c_reply->element[i]));
				}
				break;
			default:
				break;
			}
		}		
	}

private:
	type_t _type;
	std::string _str;
	long long _integer;
	std::vector<reply> _elements;
	int _err_type;

	friend class b1_redis_client;
};

class redis_command {
public:
	inline redis_command() {}

	inline redis_command(std::string arg)
	{
		_args.push_back(std::move(arg));
	}

	template<typename Type>
	inline redis_command& operator<<(const Type arg)
	{
		_args.push_back(std::to_string(arg));
		return *this;
	}

	template<typename Type>
	inline redis_command& operator()(const Type arg)
	{
		_args.push_back(std::to_string(arg));
		return *this;
	}

	inline operator const std::vector<std::string>& ()
	{
		return _args;
	}

private:
	std::vector<std::string> _args;
};

template<>
inline redis_command& redis_command::operator<<(const char* arg)
{
	_args.push_back(arg);
	return *this;
}

template<>
inline redis_command& redis_command::operator()(const char* arg)
{
	_args.push_back(arg);
	return *this;
}

template<>
inline redis_command& redis_command::operator<<(std::string arg)
{
	_args.push_back(std::move(arg));
	return *this;
}

template<>
inline redis_command& redis_command::operator()(std::string arg)
{
	_args.push_back(std::move(arg));
	return *this;
}

template<>
inline redis_command& redis_command::operator<<(b1_string_t arg)
{
	_args.push_back(b1_string_to_cppstring(arg));
	return *this;
}

template<>
inline redis_command& redis_command::operator()(b1_string_t arg)
{
	_args.push_back(b1_string_to_cppstring(arg));
	return *this;
}

/**
 * TODO：
 * 1. 错误处理
 *	- 获取到错误的reply？
 *	- redis挂掉之后该如何处理？
 */
class b1_redis_client : b1_utils::noncopyable {
public:
	typedef std::shared_ptr<b1_redis_client> b1_redis_ptr_t;
	typedef std::map<std::string, std::string> b1_redis_map_t;
	typedef b1_redis_map_t::const_iterator b1_redis_map_cit_t;

	typedef std::map<std::string, b1_string_t> b1_redis_bin_map_t;
	typedef b1_redis_bin_map_t::const_iterator b1_redis_bin_map_cit_t;

	~b1_redis_client() {
		printf("~b1_redis_client\n");
		if (c_) redisFree(c_);
		is_redis_connected_ = false;
	}

	static b1_redis_ptr_t 
	init(const std::string &host = "localhost", const uint32_t port = 6379) {
		b1_redis_ptr_t  ret = b1_redis_ptr_t(new b1_redis_client(host, port));
		if (!ret->is_redis_connected()) {
			printf("can't connected to remote redis server\n");
			ret.reset();
		}
		return ret;
	}

	int32_t hset(const std::string &name, const std::string &key, 
				 const std::string &value);
	int32_t hget(const std::string &name, const std::string &key, 
				 std::string &value);
	/* 返回真正删除的个数 */
	int32_t hdel(const std::string &name, const std::string &key, int64_t *ds);
	int32_t hscan(const std::string &name, b1_redis_map_t &val_map);
	int32_t hincr(const std::string &name, const std::string &key, 
				  int64_t incrby, int64_t *ret);
	int32_t hsize(const std::string &name, int64_t *ret);
	
	// std::map<std::string, std::string> 
	// std::map<std::string, std::vector<char> >
	template<typename Type>
	int32_t hclear(const std::string &name, const Type &val_map, int64_t *ret);	

	int32_t hclear_all(const std::string &name, int64_t *ret);

	int32_t mul_hget(const std::string &name, 
					 const std::vector<std::string> &key_vecs,
					 b1_redis_map_t &val_maps);

	// std::map<std::string, std::string> 
	// std::map<std::string, std::vector<char> >
	template<typename Type>
	int32_t mul_hset(const std::string &name, const Type &kv_rec);	

	int32_t mul_hdel(const std::string &name, 
					 const std::vector<std::string> &key_rec, 
					 int64_t *ds);

	int32_t qpush(const std::string &name, const std::string &val);
	int32_t qsize(const std::string &name, int64_t *ret);
	int32_t qclear(const std::string &name, const std::string &val, 
				   int64_t *ret);
	int32_t qpop_front(const std::string &name, std::string &val);
	int32_t qpop_front(const std::vector<std::string> &name, std::string &val);

	int32_t sadd(const std::string &name, const std::vector<std::string> &keys);
	int32_t smembers(const std::string &name, std::vector<std::string> &keys);

private:
	b1_redis_client(const std::string &host = "localhost",
		const uint32_t port = 6379);
	inline bool is_redis_connected() {
		/*return (c_ != nullptr);*/
		return is_redis_connected_;
	}

	void append(const std::vector<std::string>& args);
	reply get_reply();

	inline reply run(const std::vector<std::string>& args) {
		append(args);
		return get_reply();
	}

private:
	bool is_redis_connected_;
	redisContext *c_;
};

template<typename Type>
int32_t b1_redis_client::hclear(const std::string &name, const Type &val_map, 
								int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_hclear("HDEL");
	typename Type::const_iterator cit = val_map.cbegin();
	cmd_hclear << name;
	while (cit != val_map.cend()) {
		cmd_hclear << cit->first;
		++cit;
	}

	reply r = run(cmd_hclear);
	CHECK_REDIS_REPLY(r, cmd_hclear);

	int64_t r_int = r.integer();

	printf("hclear size: %lld\n", r_int);
	*ret = r_int;

	return 0;
}

template<typename Type>
int32_t b1_redis_client::mul_hset(const std::string &name, const Type &kv_rec)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	if (kv_rec.size() == 0) return 0;

	redis_command cmd_hmset("HMSET");
	cmd_hmset << name;
	typename Type::const_iterator cit = kv_rec.cbegin();
	while (cit != kv_rec.cend()) {
		cmd_hmset << cit->first << cit->second;
		++cit;
	}

	reply r = run(cmd_hmset);
	CHECK_REDIS_REPLY(r, cmd_hmset);

	printf("mul_hset ret: %s\n", r.str().c_str());

	return 0;
}

#endif
