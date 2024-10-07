#define _NEW
#undef _GLIBCXX_HOSTED
#include <atomic>

// Used in multi-core environment
class SpinLock
{
public:
	SpinLock() : flag(false)
	{
	}
	void lock()
	{
		while (flag.test_and_set())
			;
	}
	void unlock()
	{
		flag.clear();
	}

private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};