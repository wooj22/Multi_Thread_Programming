/*
	[ ���ɶ� (Spin Lock) ]
*/

#include <iostream>
#include <vector>
#include <windows.h>
#include <process.h> // _beginthreadex
using namespace std;

// SpinLock Class
class SpinLock
{
private:
	volatile LONG locked;

public:
	SpinLock() : locked(0) {}

	void Lock()
	{
		// �ٸ� �����尡 locked�� 0���� �ٲ� ������ ��� �õ�(spin)
		while (InterlockedCompareExchange(&locked, 1, 0) != 0)
		{
			YieldProcessor();	// �ٸ� �����忡�� CPU�� �纸�Ͽ� ���� ���� ���� ���� ���
		}
	}

	void Unlock()
	{
		// locked ���� 0���� �����Ͽ� �ٸ� �����尡 ������ �� �ֵ��� ��
		InterlockedExchange(&locked, 0);
	}
};


// ���� ���� �� �� ��ü
SpinLock		spinLock;							// ���� �����͸� ��ȣ�� ���ɶ� ��ü			
long			counter = 0;						// ��������� ������ų ���� ����		
const int		THREAD_COUNT = 10;
const int		INCREMENT_PER_THREAD = 100000;


// ������ �Լ�
unsigned int __stdcall ThreadFunction(void* arg)
{
	int threadId = *(static_cast<int*>(arg));

	for (int i = 0; i < INCREMENT_PER_THREAD; i++)
	{
		spinLock.Lock();	// ----�Ӱ� ���� ����
		counter++;
		spinLock.Unlock();  // ----�Ӱ� ���� ����
	}

	delete static_cast<int*>(arg);
	return 0;
}



int main()
{
	vector<HANDLE> threadHandles;
	cout << THREAD_COUNT << "���� �����尡 ���� " << INCREMENT_PER_THREAD << "���� counter�� ������ŵ�ϴ�." << endl;

	// ������ ����
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		int* threadId = new int(i);

		HANDLE hThread = (HANDLE)_beginthreadex(
			NULL,                   // ���� �Ӽ� (�⺻��)
			0,                      // ���� ũ�� (�⺻��)
			ThreadFunction,         // ������ �Լ� ������
			threadId,               // ������ �Լ��� ������ ����
			0,                      // ���� �÷��� (��� ����)
			NULL                    // ������ ID�� ���� ���� �ּ� (�ʿ� ����)
		);

		if (hThread)
		{
			threadHandles.push_back(hThread);
		}
		else
		{
			cerr << "������ ���� ����! ���� �ڵ�: " << GetLastError() << endl;
			delete threadId;
		}
	}

	// ��� �����尡 ����� �� ���� ���
	WaitForMultipleObjects((DWORD)threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

	// ������ �ڵ� ����
	for (auto h : threadHandles)
	{
		CloseHandle(h);
	}

	long long expected_result = (long long)THREAD_COUNT * INCREMENT_PER_THREAD;
	cout << "���� counter ��: " << expected_result << endl;
	cout << "���� counter ��: " << counter << endl;

	if (counter == expected_result) cout << "����: counter ���� ����� ��ġ�մϴ�!" << endl;
	else cout << "����: counter ���� ����� ��ġ���� �ʽ��ϴ�!" << endl;

	return 0;
}