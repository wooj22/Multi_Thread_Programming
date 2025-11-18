/*
	문제 3: 작업 교환(exchange)
	문제: 단 하나의 작업(task)을 나타내는 std::atomic<int> task_id{100} 변수가 있습니다. 
	여러 개의 워커(worker) 스레드가 이 작업을 가져가려고 시도합니다. 
	exchange()를 사용하여, 단 하나의 스레드만 원래 값(100)을 가져가고, task_id를 0 (작업 완료)으로 설정하도록 구현하세요. 
	작업을 가져간 스레드는 "스레드 [ID]가 작업 [작업 ID]를 가져감"을 출력하고, 나머지는 "스레드 [ID]가 작업에 실패함"을 출력하세요.
	
	힌트: task_id.exchange(0)을 호출하면 현재 값을 0으로 바꾸고 이전 값을 반환합니다. 
	반환된 값이 100인 스레드가 작업을 획득한 스레드입니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

std::atomic<int> task_id{ 100 };

void work(int id)
{
	int old = task_id.exchange(0);

	if (old == 100) {
		printf("스레드 %d가 작업 %d를 가져감\n", id, old);
	}
	else {
		printf("실패. 실제 값 : %d\n", old);
	}
}


int main()
{
	vector<thread> workers;
	workers.push_back(thread(work, 1));
	workers.push_back(thread(work, 2));
	workers.push_back(thread(work, 3));

	for (auto& w : workers)
	{
		w.join();
	}
}