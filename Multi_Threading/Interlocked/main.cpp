/*
	[ Interlocked ]
	���� �����尡 ���ÿ� �ϳ��� ������ �а� ������ �� �߻��� �� �ִ� ���� ����(race condition)�� ����.
	�ܼ��� ���� ����(����, ����, ��ü ��)���� ���� ���� �ʰ� Interlocked�� ����
	�ϵ���� ���ؿ��� ������(atomic) �������� ���������� ó���� �� �ֵ��� ����.
*/

// Lock - Free ���� ī���� �ǽ�
// Interlocked �Լ��� ����Ͽ� �� ���� ���������� ���� ī������ ����
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
using namespace std;

class RefCountedObject
{
private:
	volatile LONG refCount;		// ���� ī��Ʈ
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
		// InterlockedIncrement : ���������� ����ī��Ʈ 1 ����
		LONG newCount = InterlockedIncrement(&refCount);

		string msg = "RefCount increased to : " + to_string(newCount) +
			" [Thread : " + to_string(GetCurrentThreadId()) + "]\n";

		cout << msg;
	}

	void Release()
	{
		// InterlockedIncrement : ���������� ����ī��Ʈ 1 ����
		LONG newCount = InterlockedDecrement(&refCount);

		string msg = "RefCount decreased to : " + to_string(newCount) + 
			" [Thread:" + to_string(GetCurrentThreadId()) + "]\n";
		cout << msg;
		
		// ���� ī������ 0�̶��  ��ü�� �ڵ����� ����
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