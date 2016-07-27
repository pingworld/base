#include "b1_redis_client.h"
#include "gtest/gtest.h"

class b1_redis_client_test : public testing::Test {
protected:
	virtual void SetUp()
	{
		redis_client_ = b1_redis_client::init("192.168.221.128", 6379);
		ASSERT_TRUE(redis_client_.get() != nullptr);
	}

	virtual void TearDown()
	{
		redis_client_.reset();
	}

	b1_redis_client::b1_redis_ptr_t redis_client_;
	
	/*{
		std::string name = "test-b1-rc-error";
		std::string key1 = "test-b1-rc-key1";

		int32_t ret = redis_client_->qpop_front(name, key1);
		ASSERT_EQ(ret, 0);
	}*/
};

TEST_F(b1_redis_client_test, hset_hget_string)
{
	if (!redis_client_) return;

	std::string name = "test-b1-rc";
	std::string key = "test-b1-rc-key";

	std::string hset_value1 = "test-b1-rc-value";
	std::string hget_value1;

	int32_t ret = redis_client_->hset(name, key, hset_value1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hget(name, key, hget_value1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(hget_value1, hset_value1);
}

TEST_F(b1_redis_client_test, hset_hget_stream)
{
	if (!redis_client_) return;

	std::string name = "test-b1-rc";
	std::string key = "test-b1-rc-key";

	char hset_value2_cstr[] = { '0', '1', '\0', '3', '4', '\0', '6' };
	std::string hset_value2(hset_value2_cstr, 7);
	int32_t ret = redis_client_->hset(name, key, hset_value2);
	ASSERT_EQ(ret, 0);
	std::string hget_value2;
	ret = redis_client_->hget(name, key, hget_value2);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(hget_value2, hset_value2);
}

TEST_F(b1_redis_client_test, hdel_string)
{
	std::string name = "test-b1-rc";
	std::string key = "test-b1-rc-key";

	std::string hset_value1 = "test-b1-rc-value";
	int32_t ret = redis_client_->hset(name, key, hset_value1);
	ASSERT_EQ(ret, 0);

	int64_t ll_ret = 0;
	ret = redis_client_->hdel(name, key, &ll_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(ll_ret, 1);

	std::string hget_value1;
	ret = redis_client_->hget(name, key, hget_value1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(hget_value1, "");
}

TEST_F(b1_redis_client_test, hscan_string)
{
	std::string name = "test-b1-rc-hscan";

	std::string hset_key1 = "test-b1-rc-key1";
	std::string hset_value1 = "test-b1-rc-value1";
	std::string hset_key2 = "test-b1-rc-key2";
	std::string hset_value2 = "test-b1-rc-value2";
	std::string hset_key3 = "test-b1-rc-key3";
	std::string hset_value3 = "test-b1-rc-value3";
	b1_redis_client::b1_redis_map_t hset_maps;
	hset_maps.insert(std::make_pair(hset_key1, hset_value1));
	hset_maps.insert(std::make_pair(hset_key2, hset_value2));
	hset_maps.insert(std::make_pair(hset_key3, hset_value3));

	int32_t ret = redis_client_->hset(name, hset_key1, hset_value1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, hset_key2, hset_value2);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, hset_key3, hset_value3);
	ASSERT_EQ(ret, 0);

	b1_redis_client::b1_redis_map_t hscan_vals;
	ret = redis_client_->hscan(name, hscan_vals);
	ASSERT_EQ(ret, 3);
	ASSERT_EQ(hscan_vals.size(), 3);

	b1_redis_client::b1_redis_map_t::const_iterator cit =
		hset_maps.cbegin();
	while (cit != hset_maps.cend()) {
		b1_redis_client::b1_redis_map_t::const_iterator fit =
			hscan_vals.find(cit->first);
		ASSERT_NE(fit, hscan_vals.cend());
		ASSERT_EQ(fit->second, cit->second);
		++cit;
	}
}

TEST_F(b1_redis_client_test, hscan_stream)
{
	std::string name = "test-b1-rc-hscan-stream";

	std::string hset_key1 = "test-b1-rc-key1";
	char hset_value1_cstr[] = { '1', '2', '\0', '4', '5' };
	std::string hset_value1(hset_value1_cstr, 5);

	std::string hset_key2 = "test-b1-rc-key2";
	std::string hset_value2 = "test-b1-rc-value2";

	std::string hset_key3 = "test-b1-rc-key3";
	char hset_value3_cstr[] = { '1', '2', '\0', '4', '5' };
	std::string hset_value3(hset_value3_cstr, 5);

	b1_redis_client::b1_redis_map_t hset_maps;
	hset_maps.insert(std::make_pair(hset_key1, hset_value1));
	hset_maps.insert(std::make_pair(hset_key2, hset_value2));
	hset_maps.insert(std::make_pair(hset_key3, hset_value3));

	int32_t ret = redis_client_->hset(name, hset_key1, hset_value1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, hset_key2, hset_value2);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, hset_key3, hset_value3);
	ASSERT_EQ(ret, 0);

	b1_redis_client::b1_redis_map_t hscan_vals;
	ret = redis_client_->hscan(name, hscan_vals);
	ASSERT_EQ(ret, 3);
	ASSERT_EQ(hscan_vals.size(), 3);

	b1_redis_client::b1_redis_map_t::const_iterator cit =
		hset_maps.cbegin();
	while (cit != hset_maps.cend()) {
		b1_redis_client::b1_redis_map_t::const_iterator fit =
			hscan_vals.find(cit->first);
		ASSERT_NE(fit, hscan_vals.cend());
		ASSERT_EQ(fit->second, cit->second);
		++cit;
	}
}

TEST_F(b1_redis_client_test, hincrby)
{
	std::string name = "test-b1-rc-incrby";
	std::string key = "test-b1-rc-key-inc";
	int64_t by = 1, by_ret = 0;

	int32_t ret = redis_client_->hset(name, key, std::to_string(1));
	ASSERT_EQ(ret, 0);

	std::string hget_ret;
	ret = redis_client_->hget(name, key, hget_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(hget_ret, "1");

	ret = redis_client_->hincr(name, key, by, &by_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(by_ret, 2);
	ret = redis_client_->hincr(name, key, by, &by_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(by_ret, 3);
	by = -1;
	ret = redis_client_->hincr(name, key, by, &by_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(by_ret, 2);
	by = -1;
	ret = redis_client_->hincr(name, key, by, &by_ret);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(by_ret, 1);
}

TEST_F(b1_redis_client_test, hsize)
{
	std::string name = "test-b1-rc-hsize";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";

	int32_t ret = redis_client_->hset(name, key1, val1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, key2, val2);
	ASSERT_EQ(ret, 0);

	int64_t sz = 0;
	ret = redis_client_->hsize(name, &sz);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(sz, 2);
}

TEST_F(b1_redis_client_test, hclear)
{
	std::string name = "test-b1-rc-hclear";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";

	int32_t ret = redis_client_->hset(name, key1, val1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, key2, val2);
	ASSERT_EQ(ret, 0);

	int64_t sz = 0;
	ret = redis_client_->hsize(name, &sz);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(sz, 2);

	int64_t cz = 0;
	/*b1_redis_client::b1_redis_map_t cvs;
	cvs.insert(std::make_pair(key1, val1));
	cvs.insert(std::make_pair(key2, val2));
	ret = redis_client_->hclear(name, cvs, &cz);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cvs.size(), cz);*/
	ret = redis_client_->hclear_all(name, &cz);
	ASSERT_EQ(ret, 0);
}

TEST_F(b1_redis_client_test, hmget)
{
	std::string name = "test-b1-rc-hmget";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";

	int32_t ret = redis_client_->hset(name, key1, val1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->hset(name, key2, val2);
	ASSERT_EQ(ret, 0);

	int64_t sz = 0;
	ret = redis_client_->hsize(name, &sz);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(sz, 2);

	std::vector<std::string> key_vecs;
	key_vecs.push_back(key1);
	key_vecs.push_back(key2);
	b1_redis_client::b1_redis_map_t val_maps;
	ret = redis_client_->mul_hget(name, key_vecs, val_maps);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(val_maps.size(), 2);
	ASSERT_EQ(val_maps[key1], val1);
	ASSERT_EQ(val_maps[key2], val2);
}

TEST_F(b1_redis_client_test, hmset)
{
	std::string name = "test-b1-rc-hmset";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";

	b1_redis_client::b1_redis_map_t kvs;
	kvs.insert(std::make_pair(key1, val1));
	kvs.insert(std::make_pair(key2, val2));
	int32_t ret = redis_client_->mul_hset(name, kvs);
	ASSERT_EQ(ret, 0);
	
	std::string cmp_val1, cmp_val2;
	ret = redis_client_->hget(name, key1, cmp_val1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val1, val1);
	ret = redis_client_->hget(name, key2, cmp_val2);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val2, val2);
}

TEST_F(b1_redis_client_test, hmset_bin)
{
	std::string name = "test-b1-rc-hmset-bin";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	b1_string_t b1_val1 = cppstring_to_b1_string(val1);
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";
	b1_string_t b1_val2 = cppstring_to_b1_string(val2);

	b1_redis_client::b1_redis_bin_map_t kvs;
	kvs.insert(std::make_pair(key1, b1_val1));
	kvs.insert(std::make_pair(key2, b1_val2));
	int32_t ret = redis_client_->mul_hset(name, kvs);
	ASSERT_EQ(ret, 0);

	std::string cmp_val1, cmp_val2;
	ret = redis_client_->hget(name, key1, cmp_val1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val1, val1);
	ret = redis_client_->hget(name, key2, cmp_val2);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val2, val2);
}

TEST_F(b1_redis_client_test, hmdel)
{
	std::string name = "test-b1-rc-hmdel";
	std::string key1 = "test-b1-rc-key1";
	std::string val1 = "test-b1-rc-val1";
	std::string key2 = "test-b1-rc-key2";
	std::string val2 = "test-b1-rc-val2";

	b1_redis_client::b1_redis_map_t kvs;
	kvs.insert(std::make_pair(key1, val1));
	kvs.insert(std::make_pair(key2, val2));
	int32_t ret = redis_client_->mul_hset(name, kvs);
	ASSERT_EQ(ret, 0);

	std::string cmp_val1, cmp_val2;
	ret = redis_client_->hget(name, key1, cmp_val1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val1, val1);
	ret = redis_client_->hget(name, key2, cmp_val2);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val2, val2);

	std::vector<std::string> keys;
	keys.push_back(key1);
	keys.push_back(key2);
	int64_t ds = 0;
	ret = redis_client_->mul_hdel(name, keys, &ds);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(ds, 2);

	ret = redis_client_->hget(name, key1, cmp_val1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val1, "");
	ret = redis_client_->hget(name, key2, cmp_val2);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cmp_val2, "");
}

TEST_F(b1_redis_client_test, qpush_qsize)
{
	std::string name = "test-b1-rc-qpush_qsize4";
	std::string key1 = "test-b1-rc-key1";

	int64_t qs = 0;
	int32_t ret = redis_client_->qclear(name, key1, &qs);
	ASSERT_EQ(ret, 0);

	ret = redis_client_->qpush(name, key1);
	ASSERT_EQ(ret, 0);
	
	ret = redis_client_->qsize(name, &qs);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(qs, 1);
}

TEST_F(b1_redis_client_test, qpop_front)
{
	std::string name1 = "test-b1-rc-qpop1";
	std::string name2 = "test-b1-rc-qpop2";
	std::string key1 = "test-b1-rc-key1";
	std::string key2 = "test-b1-rc-key2";

	int64_t cs = 0;

	int32_t ret = redis_client_->qclear(name1, key1, &cs);
	ASSERT_EQ(ret, 0);

	ret = redis_client_->qclear(name2, key2, &cs);
	ASSERT_EQ(ret, 0);

	ret = redis_client_->qpush(name1, key1);
	ASSERT_EQ(ret, 0);

	ret = redis_client_->qpush(name2, key2);
	ASSERT_EQ(ret, 0);

	std::string pop_key1, pop_key2;
	ret = redis_client_->qpop_front(name1, pop_key1);
	ASSERT_GT(ret, 0);
	ASSERT_EQ(pop_key1, key1);

	ret = redis_client_->qpop_front(name2, pop_key2);
	ASSERT_GT(ret, 0);
	ASSERT_EQ(pop_key2, key2);

	// will block
	pop_key1 = "";
	ret = redis_client_->qpop_front(name1, pop_key1);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(pop_key1, "");

	/*如果不先push进去，下面会阻塞*/
	ret = redis_client_->qpush(name1, key1);
	ASSERT_EQ(ret, 0);
	ret = redis_client_->qpush(name2, key2);
	ASSERT_EQ(ret, 0);

	ret = redis_client_->qsize(name1, &cs);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cs, 1);

	ret = redis_client_->qsize(name2, &cs);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(cs, 1);

	std::vector<std::string> pop_names;
	pop_names.push_back(name1);
	pop_names.push_back(name2);
	std::string pop_names_key1;
	ret = redis_client_->qpop_front(pop_names, pop_names_key1);
	ASSERT_GT(ret, 0);
	ASSERT_EQ(pop_names_key1, key1);
}

TEST_F(b1_redis_client_test, sadd_smemb)
{
	std::string name = "test-b1-rc-sadd";
	std::string key1 = "test-b1-rc-key1";
	std::string key2 = "test-b1-rc-key2";

	std::vector<std::string> keys;
	keys.push_back(key1);
	keys.push_back(key2);
	int32_t ret = redis_client_->sadd(name, keys);
	ASSERT_EQ(ret, 0);

	std::vector<std::string> get_keys;
	ret = redis_client_->smembers(name, get_keys);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(get_keys.size(), 2);	
}

TEST_F(b1_redis_client_test, rc_error)
{
	/**
	 * 用于验证redis服务器挂起再重启之后的处理，只要能重新连接上并能获得数据即可
	 */
	std::string name = "test-b1-rc-error";
	std::string get_key1;

	int32_t ret = redis_client_->qpop_front(name, get_key1);
	ASSERT_EQ(ret, 0);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
