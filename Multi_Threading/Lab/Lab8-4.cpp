/*
	[문제 4: std::timed_mutex와 try_lock_for]
	문제: std::timed_mutex를 하나 선언하세요. 
	첫 번째 스레드를 생성하여 이 뮤텍스를 즉시 lock()하고 2초 동안 sleep한 후 unlock()하게 하세요. 
	첫 번째 스레드 시작 직후, 두 번째 스레드를 생성하세요. 
	두 번째 스레드는 500밀리초(ms) 동안만 뮤텍스 잠금을 시도(try_lock_for)해야 합니다. 
	잠금에 성공하면 "Lock Acquired"를, 시간 내에 잠금에 실패하면(timeout) "Timed out"을 출력하게 하세요.

	힌트: std::timed_mutex는 try_lock_for(duration) 멤버 함수를 제공합니다.
	이 함수는 지정된 시간 동안 잠금을 시도하고, 성공하면 true를, 실패하면 false를 반환합니다.
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex tm;

int main()
{
	std::thread t1([] {
		tm.lock();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		tm.unlock();
		});

	std::thread t2([] {
		if (tm.try_lock_for(std::chrono::milliseconds(500)))
		{
			std::cout << "Lock Acquired\n";
		}
		else
		{
			std::cout << "Time out\n";
		}
		});

	t1.join();
	t2.join();
}
