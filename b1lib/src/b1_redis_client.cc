#include "b1_redis_client.h"
#include <assert.h>

b1_redis_client::b1_redis_client(const std::string &host, const uint32_t port) 
	: is_redis_connected_(true) {
	struct timeval tv = {30, 0};
	c_ = redisConnectWithTimeout(host.c_str(), port, tv);
	if (c_->err != REDIS_OK) {
		printf("c_->err:%d, c_->errstr:%s\n", c_->err, c_->errstr);
		redisFree(c_);
		c_ = nullptr;
		is_redis_connected_ = false;
	}	
}

void b1_redis_client::append(const std::vector<std::string>& args)
{
	std::vector<const char*> argv;
	argv.reserve(args.size());
	std::vector<size_t> argvlen;
	argvlen.reserve(args.size());

	for (std::vector<std::string>::const_iterator it = args.begin();
		 it != args.end(); ++it) {
		argv.push_back(it->c_str());
		argvlen.push_back(it->size());
	}

	int ret = redisAppendCommandArgv(c_, static_cast<int>(args.size()),
		argv.data(), argvlen.data());
	if (ret != REDIS_OK) {
		printf("redis return error!\n");
	}
}

/* Create a reply object */
static redisReply *createReplyObject(int type)
{
	redisReply *r = (redisReply *)calloc(1, sizeof(*r));

	if (r == NULL)
		return NULL;

	r->type = type;
	return r;
}

#include <string.h>
reply b1_redis_client::get_reply()
{
	redisReply *r = nullptr;
	int error = redisGetReply(c_, reinterpret_cast<void**>(&r));
	if (error != REDIS_OK) {
		/**
		* TODO:
		* 1. 统计所有的错误！
		*/
		r = createReplyObject(REDIS_REPLY_ERROR);
		if (r == NULL) return reply(nullptr);

		size_t len = strlen(c_->errstr);

		char *buf = (char *)malloc(len + 1);
		if (buf == NULL) {
			freeReplyObject(r);
			return reply(nullptr);
		}

		/* Copy string value */
		memcpy(buf, c_->errstr, len);
		buf[len] = '\0';
		r->str = buf;
		r->len = len;
	}

	reply ret(r);
	freeReplyObject(r);

	if (ret.type() == reply::type_t::ERROR) {
		ret.set_err_type(c_->err);
	}

	return ret;
}

int32_t b1_redis_client::hset(const std::string &name, const std::string &key, 
							  const std::string &value)
{
	ASSERT_CMDS_PARAM_VALID2(name, key);

	printf("hset of string value\n");

	redis_command cmd_hset("HSET");
	cmd_hset << name << key << value;

	reply r = run(cmd_hset);
	CHECK_REDIS_REPLY(r, cmd_hset);

	return 0;
}

int32_t b1_redis_client::hget(const std::string &name, const std::string &key, 
							  std::string &value)
{
	ASSERT_CMDS_PARAM_VALID2(name, key);

	redis_command cmd_hget("HGET");
	cmd_hget << name << key;

	reply r = run(cmd_hget);
	CHECK_REDIS_REPLY(r, cmd_hget);

	value = r.str();

	return 0;
}

int32_t b1_redis_client::hdel(const std::string &name, const std::string &key, 
							 int64_t *ds)
{
	ASSERT_CMDS_PARAM_VALID2(name, key);

	redis_command cmd_hdel("HDEL");
	cmd_hdel << name << key;

	reply r = run(cmd_hdel);
	CHECK_REDIS_REPLY(r, cmd_hdel);

	long long ret = r.integer();
	*ds = ret;
	
	printf("hdel %lld from redis for %s:%s\n", ret, name.c_str(), key.c_str());

	return 0;
}

int32_t b1_redis_client::hscan(const std::string &name, b1_redis_map_t &val_map)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_hscan("HGETALL");
	cmd_hscan << name;

	reply r = run(cmd_hscan);
	CHECK_REDIS_REPLY(r, cmd_hscan);

	const std::vector<reply> &eles = r.elements();

	// must be even
	if (eles.size() % 2 != 0) {
		assert(false);
		return -1;
	}
	printf("hscan %s get elements size:%u\n", name.c_str(), eles.size());

	std::vector<reply>::const_iterator cit = eles.cbegin();
	while ((cit != eles.cend()) && (cit+1 != eles.cend())) {
		reply r_key = *cit;
		reply r_value = *(cit + 1);

#ifdef DEBUG
		printf("r_key:%s, r_value:%s\n", r_key.str().c_str(), 
			r_value.str().c_str());
#endif

		val_map.insert(std::make_pair(r_key.str(), r_value.str()));

		cit += 2;
	}

	return val_map.size();
}

int32_t b1_redis_client::hincr(const std::string &name, const std::string &key,
							   int64_t incrby, int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID2(name, key);

	redis_command cmd_hincr("HINCRBY");
	cmd_hincr << name << key << incrby;

	reply r = run(cmd_hincr);
	CHECK_REDIS_REPLY(r, cmd_hincr);

	long long r_int = r.integer();

	printf("hincrby %lld ret: %llu\n", incrby, r_int);
	*ret = r_int;

	return 0;
}

//However, the returned integer is guaranteed to be in the range of a signed 
//64 bit integer.
int32_t b1_redis_client::hsize(const std::string &name, int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_hsize("HLEN");
	cmd_hsize << name;

	reply r = run(cmd_hsize);
	CHECK_REDIS_REPLY(r, cmd_hsize);

	int64_t r_int = r.integer();

	printf("hsize ret: %llu\n", r_int);
	*ret = r_int;

	return 0;
}

int32_t b1_redis_client::mul_hget(const std::string &name, 
								  const std::vector<std::string> &key_vecs, 
								  b1_redis_client::b1_redis_map_t &val_maps)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	if (key_vecs.size() == 0) return 0;

	redis_command cmd_hmget("HMGET");
	cmd_hmget << name;
	std::vector<std::string>::const_iterator cit = key_vecs.cbegin();	
	while (cit != key_vecs.cend()) {
		cmd_hmget << *cit;
		++cit;
	}

	reply r = run(cmd_hmget);
	CHECK_REDIS_REPLY(r, cmd_hmget);

	const std::vector<reply> &eles = r.elements();
	size_t i = 0;
	std::vector<reply>::const_iterator r_cit = eles.cbegin();
	while (r_cit != eles.cend()) {

#ifdef DEBUG
		printf("hmget key:%s, value:%s\n", key_vecs[i].c_str(), 
			r_cit->str().c_str());
#endif

		val_maps.insert(std::make_pair(key_vecs[i], r_cit->str()));
		++r_cit;
		++i;
	}

	return 0;
}

int32_t b1_redis_client::mul_hdel(const std::string &name, 
								  const std::vector<std::string> &key_rec,
								  int64_t *ds)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_hclear("HDEL");
	std::vector<std::string>::const_iterator cit = key_rec.cbegin();
	cmd_hclear << name;
	while (cit != key_rec.cend()) {
		cmd_hclear << *cit;
		++cit;
	}

	reply r = run(cmd_hclear);
	CHECK_REDIS_REPLY(r, cmd_hclear);

	int64_t r_int = r.integer();

	printf("hclear size: %lld\n", r_int);
	*ds = r_int;

	return 0;
}

int32_t b1_redis_client::qpush(const std::string &name, const std::string &val)
{
	ASSERT_CMDS_PARAM_VALID2(name, val);

	redis_command cmd_qpush("LPUSH");
	cmd_qpush << name << val;

	reply r = run(cmd_qpush);
	CHECK_REDIS_REPLY(r, cmd_qpush);

	int64_t r_int = r.integer();

	printf("qpush list size: %lld\n", r_int);
	return 0;
}

int32_t b1_redis_client::qsize(const std::string &name, int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_qsize("LLEN");
	cmd_qsize << name;

	reply r = run(cmd_qsize);
	CHECK_REDIS_REPLY(r, cmd_qsize);

	int64_t r_int = r.integer();

	printf("qsize list size: %lld\n", r_int);
	*ret = r_int;

	return 0;
}

int32_t b1_redis_client::qclear(const std::string &name, const std::string &val,
								int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_qclear("LREM");
	cmd_qclear << name << 0 << val;

	reply r = run(cmd_qclear);
	CHECK_REDIS_REPLY(r, cmd_qclear);

	*ret = r.integer();

	printf("qclear size: %lld\n", *ret);

	return 0;
}

int32_t b1_redis_client::qpop_front(const std::string &name, std::string &val)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_qf("BRPOP");
	cmd_qf << name << 3;

	reply r = run(cmd_qf); 
	CHECK_REDIS_REPLY(r, cmd_qf);

	const std::vector<reply> &eles = r.elements();
	if (eles.size() != 0) {
		printf("qpop_front1 key:%s, value:%s\n", eles[0].str().c_str(),
			eles[1].str().c_str());

		val = eles[1].str();
		return eles.size();
	} else {
		printf("qpop_front1 err_type:%d\n", r.err_type());
		return 0;
	}	
}

int32_t b1_redis_client::qpop_front(const std::vector<std::string> &name, 
									std::string &val)
{
	if (name.size() == 0) return 0;

	redis_command cmd_qf("BRPOP");
	std::vector<std::string>::const_iterator cit = name.cbegin();
	while (cit != name.cend()) {
		printf("key: %s\n", cit->c_str());
		cmd_qf << *cit;
		++cit;
	}
	cmd_qf << 0;	// timeout

	reply r = run(cmd_qf);
	CHECK_REDIS_REPLY(r, cmd_qf);

	const std::vector<reply> &eles = r.elements();
	if (eles.size() != 0) {
		printf("qpop_front1 key:%s, value:%s\n", eles[0].str().c_str(),
			eles[1].str().c_str());

		val = eles[1].str();
		return eles.size();
	} else {
		printf("qpop_front1 err_type:%d\n", r.err_type());
		return 0;
	}
}

int32_t b1_redis_client::sadd(const std::string &name, 
							  const std::vector<std::string> &keys)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	if (keys.size() == 0) return 0;

	redis_command cmd_sadd("SADD");
	cmd_sadd << name;
	std::vector<std::string>::const_iterator cit = keys.cbegin();
	while (cit != keys.cend()) {
		cmd_sadd << *cit;
		++cit;
	}

	reply r = run(cmd_sadd);
	CHECK_REDIS_REPLY(r, cmd_sadd);

	int64_t r_int = r.integer();

	printf("sadd ret:%lld\n", r_int);
	return 0;
}

int32_t b1_redis_client::smembers(const std::string &name, 
								  std::vector<std::string> &keys)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	redis_command cmd_smembers("SMEMBERS");
	cmd_smembers << name;

	reply r = run(cmd_smembers);
	CHECK_REDIS_REPLY(r, cmd_smembers);

	const std::vector<reply> &eles = r.elements();
	std::vector<reply>::const_iterator r_cit = eles.cbegin();
	while (r_cit != eles.cend()) {

		printf("smembers keys:%s\n", r_cit->str().c_str());

		keys.push_back(r_cit->str());
		++r_cit;
	}

	return 0;
}

int32_t b1_redis_client::hclear_all(const std::string &name, int64_t *ret)
{
	ASSERT_CMDS_PARAM_VALID1(name);

	// redis的DEL可以将hash看成一个完整的内容，所以可以一次性全部删除
	redis_command cmd_hclear("DEL");
	cmd_hclear << name;

	reply r = run(cmd_hclear);
	CHECK_REDIS_REPLY(r, cmd_hclear);

	int64_t r_int = r.integer();

	printf("hclear all size: %lld\n", r_int);
	*ret = r_int;

	return 0;
}


