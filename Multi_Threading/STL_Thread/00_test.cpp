#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> counter = 0;

void worker(int id)
{
	for (int i = 0; i < 1000; ++i)
	{
		int expected = 10000;
		counter.compare_exchange_strong(expected, 0);
	}
}