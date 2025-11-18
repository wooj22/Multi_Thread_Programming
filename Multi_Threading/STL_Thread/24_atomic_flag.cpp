/*
  [ std::atomic_flag ]

  atomic_floag는 가장 기본적인 atomic 타입으로, 유일하게 lock-free가 보장되는 atomic xkdlqdlek.
  boolean flag를 표현하며 가장 저수준의 동기화 primitive이다.

  - flag.test_and_set() : 플래그를 true로 설정하고 이전 값 반환
  - flag.clear() : 플래그를 flase로 설정
*/

#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

std::atomic_flag flag = ATOMIC_FLAG_INIT;		// 반드시 이렇게 초기화 해야한다.

void worker(int id)
{
	// test_and_set : 플래그를 true로 설정하고 이전 값 반환
	while (flag.test_and_set(std::memory_order_acquire))
	{
		// 이미 설정되어있으면 대기 (spin)
		// 첫번째로 이곳에 다다른 스레드만 통과하고, 나머지 스레드들은 이미 true인 상태이기 때문에 spin중
	}

	// 임계 영역
	std::cout << "worker " << id << "작업 중...\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// clear : 플래그를 false로 설정
	flag.clear(std::memory_order_release);
}

int main()
{
	std::vector <std::thread> threads;

	for (int i = 0; i < 5; i++)
	{
		threads.emplace_back(worker, i);
	}

	for (auto& t : threads)
	{
		t.join();
	}

	return 0;
}