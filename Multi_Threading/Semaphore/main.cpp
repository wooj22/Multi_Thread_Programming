/*
	[�������� (Semaphore)]
	���� �ڿ��� ������ �� �ִ� �������� ������ �����ϴ� ����ȭ ��ü.
	�ѹ��� N���� �����尡 �ڿ��� �����ϵ��� ����ϰ� ���� �� ����ϸ�,
	�������� ī��Ʈ ���� ��� ������ �������� �ִ� ������ ��Ÿ����.

	1. CreateSemaphore(���ȼӼ�, �ʱ�ī��Ʈ, �ִ�ī��Ʈ, �̸�)
		: �������� ��ü ����
	   - �ʱ�ī��Ʈ: ������� ������ ���� �ʱ� ī��Ʈ �� (��, ���ÿ� ���� ������ ������ ��)
	   - �ִ�ī��Ʈ: ������� ���� �� �ִ� �ִ� ī��Ʈ ��
	
	2. WaitForSingleObject(���������ڵ�, ���ð�)
		: �������� �ڸ��� �� ������ ���
	   - �������� ī��Ʈ�� 0���� ũ�� ī��Ʈ�� 1 ���ҽ�Ű�� ��� ����.
	   - �������� ī��Ʈ�� 0�̸� �ٸ� �����尡 ReleaseSemaphore�� ȣ���� ������ ���.

	3. ReleaseSemaphore(���������ڵ�, ������ī��Ʈ, ����ī��Ʈ������ġ)
		: ��	������ ����(����)
	   - �������� ī��Ʈ�� ������ ����ŭ ������Ų��.
	   - ��� ���� �����尡 �ִٸ�, �� �� �ϳ��� ����� ������� ȹ���� �� �ִ�.
*/


// ��������� �۾� �ӵ� �����ϱ� (Rate Limiting)
#include <windows.h>
#include <iostream>
#include <queue>
#include <string>
#include <process.h> // _beginthreadex, _endthreadex

using namespace std;

queue<string>		taskQueue;		// �۾��� ������ ���� ť
CRITICAL_SECTION	cs;			    // �Ӱ豸�� (ũ��Ƽ�� ����) : ť�� ���� ����ȭ ��ü
HANDLE			    semaphore;		// ó�� ������ �۾��� �ִ� ������ �����ϴ� ��������

volatile bool isRunning = true;		// �۾� ����, ó�� ���� �÷���
const int MAX_BUFFER_SIZE = 5;		// ����(ť)�� �ִ� ũ��. ���������� �ִ� ī��Ʈ


/* �۾� ������(Producer) ������ �Լ� */
// ������ �۾��� �����ؼ� ť�� �߰�
unsigned int __stdcall ProducerThread(void* pParam)
{
	int taskCount = 0;


	while (taskCount < 20)
	{
		// 1. �������� ���
		// ���� �������� ī��Ʈ�� 0�̸� (��, ī��Ʈ�� �� á�ٸ�)
		// Consumer�� �۾��� ó���ϰ� ������� ������ �� ���� ���⼭ ���
		// count > 0 �̸� : ��������� ��� ī��Ʈ�� 1 ���ҽ�Ű�� �Լ��� ����. ������� ��� ���� ���� �ڵ� ����.
		// count == 0 �̸� : ������� ī��Ʈ�� 0���� Ŀ���� ����(��, �ٸ� �����尡 ReleaseSemaphore ȣ���� �� ����) ������ ���(block)
		WaitForSingleObject(semaphore, INFINITE);

		// 2. ũ��Ƽ�� ������ ���� ť�� ���� ���� ��ȣ
		EnterCriticalSection(&cs);

		// �۾� ����
		taskCount++;
		string task = "�۾� " + to_string(taskCount);
		taskQueue.push(task);
		cout << "[����] " << task << "(���� ť ũ�� : " << taskQueue.size() << ")\n";

		// 3. ũ��Ƽ�� ���� ����
		LeaveCriticalSection(&cs);

		// ���� �۾� ������ �ùķ��̼��ϱ� ���� ª�� �ð� ���
		Sleep(100);
	}

	// �۾� ������ �Ϸ�Ǹ� Consumer ������ ���Ḧ ���� �÷��� ����
	isRunning = false;
	cout << "\n--- ��� �۾� ���� �Ϸ� ---\n" << endl;
	return 0;
}

/* �۾� �Һ���(Consumer) ������ �Լ� */
// ť���� �۾��� ������ ��ġ�� ó��
unsigned int __stdcall ConsumerThread(void* pParam)
{
	while (isRunning || !taskQueue.empty())
	{
		string task;
		bool hasTask = false;

		// 1. ũ��Ƽ�� ������ ���� ť�� ���� ���� ��ȣ
		EnterCriticalSection(&cs);

		// ť���� �۾� ��������
		if (!taskQueue.empty())
		{
			
			task = taskQueue.front();
			taskQueue.pop();
			hasTask = true;
		}

		// 2. ũ��Ƽ�� ���� ����
		LeaveCriticalSection(&cs);

		if (hasTask)
		{
			std::cout << "    [ó��] " << task << " ����..." << std::endl;

			// ���� �۾� ó���� �ùķ��̼��ϱ� ���� �� �ð� ���
			Sleep(500);

			std::cout << "    [ó��] " << task << " �Ϸ�!" << std::endl;

			// 3. �۾� ó���� �������� �˸���, ����ִ� ������ �������� ������� �˸��ϴ�.
			//    �� ȣ��� ���� �������� ī��Ʈ�� 1 �����ϰ�,
			//    Producer �����尡 ��� ���̾��ٸ� �ٽ� �۾��� ������ �� �ְ� �˴ϴ�.
			ReleaseSemaphore(semaphore, 1, NULL);
		}
	}
}


int main()
{
	HANDLE hThreads[2];
	unsigned int threadID;

	// 1. ����ȭ ��ü �ʱ�ȭ
	InitializeCriticalSection(&cs);

	// �������� ����
	// �ʱ� ī��Ʈ: MAX_BUFFER_SIZE (ó������ ���۰� ��������Ƿ� �ִ�ġ��ŭ �۾� ���� ����)
	// �ִ� ī��Ʈ: MAX_BUFFER_SIZE (������ �ִ� ũ��)
	semaphore = CreateSemaphore(
		NULL,            // �⺻ ���� �Ӽ�
		MAX_BUFFER_SIZE, // �ʱ� ī��Ʈ - ���� ��� ������ �ڿ� ����
		MAX_BUFFER_SIZE, // �ִ� ī��Ʈ - �ڿ��� �ִ� ����
		NULL             // �̸� ���� ��������
	);

	if (semaphore == NULL)
	{
		std::cerr << "�������� ���� ����: " << GetLastError() << std::endl;
		return 1;
	}

	std::cout << "--- ������� �̿��� �ӵ� ����(Rate Limiting) �ùķ��̼� ���� ---" << std::endl;
	std::cout << "�ִ� ���� ũ��: " << MAX_BUFFER_SIZE << std::endl << std::endl;

	// 2. ������ ����
	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, &ProducerThread, NULL, 0, &threadID);
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, &ConsumerThread, NULL, 0, &threadID);

	// 3. ��� �����尡 ����� ������ ���
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	std::cout << "\n--- ��� �۾� ó�� �Ϸ�. ���α׷� ����. ---" << std::endl;

	// 4. ���ҽ� ����
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
	CloseHandle(semaphore);
	DeleteCriticalSection(&cs);

	return 0;
}