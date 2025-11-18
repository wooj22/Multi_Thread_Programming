/*
	문제 2: 간단한 생산자-소비자 플래그
	문제: std::atomic<bool>을 사용하여 간단한 생산자-소비자 시나리오를 구현하세요.
	
	생산자(Producer) 스레드는 1초간 대기 후, data (일반 int) 변수에 100을 쓰고 ready 플래그를 true로 설정합니다.
	소비자(Consumer) 스레드는 ready 플래그가 true가 될 때까지 load()를 호출하며 대기(spinning)합니다.
	ready가 true가 되면, data 값을 읽어 "데이터: [값]"을 출력합니다.
	힌트: 소비자 스레드에서 while 루프를 사용하여 ready.load()가 true를 반환할 때까지 기다리세요. 
	생산자 스레드에서는 데이터를 쓴 후 ready.store(true)를 호출하세요. 
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
using namespace std;

int value = 0;
atomic<bool> ready;

void Producer()
{
	this_thread::sleep_for(chrono::seconds(1));
	value = 100;
	ready.store(true);
}

void Consumer()
{
	while (!ready.load())
	{
		// spin
		this_thread::yield();
	}

	cout << "데이터 : " << value;
}

int main()
{
	
	thread projucer(Producer);
	thread consumer(Consumer);

	projucer.join();
	consumer.join();
}