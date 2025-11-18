/*
	문제 1: 스레드 안전한 카운터 만들기
	문제: 10개의 스레드를 생성하여 각 스레드가 100,000번씩 카운터를 증가시키는 프로그램을 작성하세요. 
	std::atomic<int>를 사용하여 데이터 레이스(data race) 없이 최종 결과가 정확히 1,000,000이 나오도록 보장하세요.
	
	힌트: 일반 int 대신 std::atomic<int> 타입의 변수를 선언하고, 각 스레드에서 이 변수의 ++ 연산자를 호출하면 됩니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

atomic<int> counter = 0;

void worker()
{
	for (int i = 0; i < 100000; i++)
	{
		counter++;
	}
}

int main()
{
	vector<thread> threads;

	for (int i = 0; i < 10; i++)
	{
		threads.push_back(thread(worker));
	}

	for (auto& t : threads)
	{
		t.join();
	}

	cout << "counter 예상 결과 : 1000000\ncounter 실제 결과 : " << counter;
}