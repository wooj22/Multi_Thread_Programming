/*
	[ Interlocked ]
	여러 스레드가 동시에 하나의 변수를 읽고 수정할 때 발생할 수 있는 경쟁 조건(race condition)을 방지.
	단순한 변수 연산(증가, 감소, 교체 등)에는 락을 걸지 않고 Interlocked를 통해
	하드웨어 수준에서 원자적(atomic) 연산으로 독점적으로 처리할 수 있도록 지원.
*/

// Lock - Free 참조 카운팅 실습
// Interlocked 함수를 사용하여 락 없이 원자적으로 참조 카운팅을 구현
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
using namespace std;

class RefCountedObject
{
private:
	volatile LONG refCount;		// 참조 카운트
	string		  data;

public:
	RefCountedObject(const string& str) : refCount(1), data(str)
	{
		cout << "Ojbect create : " << data << endl;
	}

private:
	~RefCountedObject()
	{
		cout << "Object destroyed: " << data << endl;
	}

public:
	void AddRef()
	{
		// InterlockedIncrement : 원자적으로 참조카운트 1 증가
		LONG newCount = InterlockedIncrement(&refCount);

		string msg = "RefCount increased to : " + to_string(newCount) +
			" [Thread : " + to_string(GetCurrentThreadId()) + "]\n";

		cout << msg;
	}

	void Release()
	{
		// InterlockedIncrement : 원자적으로 참조카운트 1 감소
		LONG newCount = InterlockedDecrement(&refCount);

		string msg = "RefCount decreased to : " + to_string(newCount) + 
			" [Thread:" + to_string(GetCurrentThreadId()) + "]\n";
		cout << msg;
		
		// 참조 카운팅이 0이라면  객체를 자동으로 삭제
		if (newCount == 0) delete this;
	}

	LONG GetRefCount() const
	{
		return InterlockedCompareExchange(const_cast<volatile LONG*>(&refCount), 0, 0);
	}
};

int main()
{
	RefCountedObject* obj = new RefCountedObject("Test");

	thread t1([&]() { obj->AddRef(); obj->Release(); });
	thread t2([&]() { obj->AddRef(); obj->Release(); });
	thread t3([&]() { obj->Release(); });

	t1.join();
	t2.join();
	t3.join();
}