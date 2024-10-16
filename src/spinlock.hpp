#pragma once
#include <common.h>
#include <mutex>

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

using lock_guard = std::lock_guard<SpinLock>;