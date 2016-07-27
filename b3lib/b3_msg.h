#ifndef __B3_MSG_H__
#define __B3_MSG_H__

#include <atomic>
#include <memory>

namespace B3LIB
{

using b3_msg_id_t = std::atomic<uint64_t>;

class CB3Msg {
public:
	explicit CB3Msg(uint32_t nameid);
	virtual ~CB3Msg() = default;

	CB3Msg(const CB3Msg&) = delete;
	CB3Msg& operator=(const CB3Msg&) = delete;

	// call std::atomic<uint64_t>::opeartor T() is safe
	inline uint64_t id() const { return id_; }
	inline uint32_t nameid() const { return nameid_; }

	// virtual move constructor
	virtual std::unique_ptr<CB3Msg> move();

protected:
	CB3Msg(CB3Msg&&) = default;
	CB3Msg& operator=(CB3Msg&&) = default;

private:
	uint64_t	id_;
	uint32_t	nameid_;		// nameid_ 是在逻辑上区分该消息的用途的
};

template <typename CB3DataType>
class CB3DataMsg : public CB3Msg {
public:
	template <typename ... CB3Arg>
	CB3DataMsg(uint32_t nameid, CB3Arg&& ... args)
		: CB3Msg(nameid), data_(new CB3DataType(std::forward<CB3Arg>(args) ... )) {
	}
	virtual ~CB3DataMsg() = default;

	CB3DataMsg(const CB3DataMsg&) = delete;
	CB3DataMsg& operator=(const CB3DataMsg&) = delete;

	virtual std::unique_ptr<CB3Msg> move() override {
		return std::unique_ptr<CB3Msg>(new CB3DataMsg<CB3DataType>(std::move(*this)));
	}

	inline CB3DataType& data() const { return *data_; }

protected:
	CB3DataMsg(CB3DataMsg&&) = default;
	CB3DataMsg& operator=(CB3DataMsg&&) = default;
	
private:
	std::unique_ptr<CB3DataType> data_;
};

}

#endif
