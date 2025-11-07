/*
*	[문제 3: 레이스 컨디션 해결 (std::lock_guard)]
	문제: 전역 변수 int g_counter = 0;가 있습니다. 
	10개의 스레드를 생성하여 각 스레드가 100,000번씩 g_counter를 증가시키도록 하세요. 
	std::mutex와 std::lock_guard를 사용하여 g_counter에 대한 접근이 스레드로부터 안전하도록(thread-safe) 만드세요.
	모든 스레드 종료 후 g_counter의 최종 값이 1,000,000이 되는지 확인하세요.
	
	힌트: 전역 std::mutex를 하나 선언하세요.
	스레드가 카운터를 증가시키는 임계 영역(critical section)에 접근하기 직전에 std::lock_guard<std::mutex> lock(my_mutex);를 선언하세요. 
	lock_guard는 생성 시 자동으로 뮤텍스를 잠그고, 스코프를 벗어날 때 (소멸 시) 자동으로 잠금을 해제하여 예외가 발생해도 안전합니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

int g_counter = 0;
std::mutex mutex;

void counterplus(int id)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		for (int i = 0; i < 100000; i++)
		{
			g_counter++;
		}
	}

	std::cout << id << "번째 스레드 작업 완료" << std::endl;
}

int main()
{
	std::vector<std::thread> workers;

	for (int i = 0; i < 10; i++)
	{
		workers.push_back(std::thread(counterplus, i));
	}

	for (auto& t : workers)
	{
		t.join();
	}

	std::cout << "모든 스레드 작업 완료" << std::endl;
	std::cout << "결과 기대 값 : " << 100000 * 10 << std::endl;
	std::cout << "실제 값 : " << g_counter << std::endl;
}