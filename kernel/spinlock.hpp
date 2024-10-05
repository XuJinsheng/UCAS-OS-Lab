#include <atomic>

// Used in multi-core environment
class SpinLock
{
public:
	SpinLock()
		: flag(false) {}
	void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire))
			;
	}
	void unlock()
	{
		flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};