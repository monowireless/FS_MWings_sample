#include "common.h" // common include
#include <MWutls.h>

template <typename T=float, uint32_t S=128> 
class pkt_seqence_data {
public:
	struct data {
		T v;
		uint32_t t;
	};
	using FQUE = TWEUTILS::FixedQueue<data, false>; // Fixed Queue type

private:
	FQUE fque_;                                     // Fixed Queue

	uint32_t sid_;                                  // SID
	uint32_t seq_;                                  // SEQ NUMBER
	uint32_t seq_mask_;								// SEQ MASK (MAX VALUE)
	uint32_t timeout_ms_;                           // Timeout

public:
	pkt_seqence_data() 
			: fque_(S)
			, sid_(0xffffffff)
			, seq_(0xffffffff)
			, seq_mask_(0xffff)
			, timeout_ms_(3000)
			{}

	// push new data (in case of invalid data, just return false)
	bool push_humble(uint32_t pkt_seq, uint32_t sid, uint32_t tick, T value) {
		bool b_accept = false;

		// set first sample (store SID and SEQ)
		if (fque_.empty()) {
			b_accept = true;
			seq_ = pkt_seq;
			sid_ = sid;
		} else {
			if (   is_seq_valid(pkt_seq) // packet seqeunc, valid if same or +1.
					&& sid_ == sid           // same SID
					&& ((tick - fque_[0].t) < timeout_ms_) // within timeout from first packet.
				) {
				b_accept = true;
			} else {
				b_accept = false;
			}
		}

		// if valid sample comes, push data in there.
		if (b_accept) {
			fque_.push_force({value, tick});
			seq_ = pkt_seq; // update seq
		}

		return b_accept;
	}

	// push new data (in case of invalid data, clear queue and push it)
	uint32_t push(uint32_t pkt_seq, uint32_t sid, uint32_t tick, T value) {
		if (!push_humble(pkt_seq, sid, tick, value)) {
			clear();
			push_humble(pkt_seq, sid, tick, value);
		}
		return size();
	}

	// clear the que.
	void clear() {
		fque_.clear();
		sid_ = 0xffffffff;
		seq_ = 0xffffffff;
	}

	// returns data count stored in the que.
	size_t size() { return fque_.size(); }

	// access by the given index, [0] is the oldest one.
	inline data& operator[] (int i) { return fque_[i]; }
	// get value by the given index.
	inline T& get_value(int i) { return fque_[i].v; }
	// get tick by the given index.
	inline uint32_t get_tick(int i) { return fque_[i].t; }

public: // getter/setter section
	void set_timeout(uint32_t t) { timeout_ms_ = t; }
	void set_seqmask(uint32_t m) { seq_mask_ = m; }

private: // internal use.
	bool is_seq_valid(uint32_t seq) {
		return (seq == seq_) || (seq == ((seq_ + 1) & seq_mask_));
	}
};