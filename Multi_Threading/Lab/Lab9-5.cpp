/*
	문제 5: 은행 계좌 입출금 시뮬레이션
	문제: std::atomic<long> balance{1000}로 은행 계좌 잔액을 나타냅니다. 5개의 "입금" 스레드와 5개의 "출금" 스레드를 만드세요.
	
	각 입금 스레드는 100씩 10번 입금합니다 (fetch_add).
	각 출금 스레드는 50씩 10번 출금합니다 (fetch_sub). 모든 스레드 종료 후 최종 잔액을 출력하세요. 
	(초기 1000 + (5 * 100 * 10) - (5 * 50 * 10) = 3500)
	힌트: 입금 스레드는 balance.fetch_add(100)을, 출금 스레드는 balance.fetch_sub(50)을 루프 안에서 호출하면 됩니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

std::atomic<long> balance{ 1000 };

// 입금
void Deposit()
{
	for (int i = 0; i < 10; i++)
	{
		balance.fetch_add(100);
	}
}

// 출금
void Withdrawal()
{
	for (int i = 0; i < 10; i++)
	{
		balance.fetch_sub(50);
	}
}

int main()
{
	vector<thread> senders;
	vector<thread> receivers;

	for (int i = 0; i < 5; i++)
	{
		senders.emplace_back(Deposit);
	}

	for (int i = 0; i < 5; i++)
	{
		receivers.emplace_back(Withdrawal);
	}

	for (int i = 0; i < 5; i++)
	{
		senders[i].join();
		receivers[i].join();
	}

	cout << "기대 계좌 잔액 : " << 3500 << endl;
	cout << "실제 계좌 잔액 : " << balance.load();
}