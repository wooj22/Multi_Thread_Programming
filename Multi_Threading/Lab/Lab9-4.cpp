/*
	문제 4: Compare-And-Swap (CAS)으로 값 갱신
	문제: std::atomic<int> value{10}가 있습니다. 스레드에서 value의 현재 값이 10일 경우에만 20으로 갱신하는 코드를 작성하세요. 
	compare_exchange_strong()을 사용하세요.
	
	값이 10이어서 갱신에 성공하면 "성공: 10 -> 20"을 출력하세요.
	다른 스레드가 값을 먼저 변경(예: 15)하여 갱신에 실패하면, "실패: 현재 값 [실제 값]"을 출력하세요.
	힌트: compare_exchange_strong(expected, desired)는 expected 변수를 인자로 받습니다. 
	expected에 예상 값(10)을, desired에 새 값(20)을 넣으세요. 함수가 false를 반환하면 expected 변수는 value의 실제 값으로 갱신됩니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

std::atomic<int> value{ 10 };

void work()
{
	int expected = 10;
	int desired = 20;
	bool success = value.compare_exchange_strong(expected, desired);

	if (success)
	{
		cout << "성공 : 10 -> 20\n";
	}
	else
	{
		printf("실패 : 현재 값 : %d\n", value.load());
	}
}

int main()
{
	vector<thread> workers;
	workers.push_back(thread(work));
	workers.push_back(thread(work));
	workers.push_back(thread(work));

	for (auto& w : workers)
	{
		w.join();
	}
}