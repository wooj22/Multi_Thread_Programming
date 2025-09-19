/*

	 [ _beginthreadex ] 

	 _beginthreadex()�� C ��Ÿ�� ���̺귯������ �����ϴ� ������ ���� �Լ���, 
	 C++ ���α׷������� CreateThread() ���� _beginthreadex() �� ����

*/

#include <windows.h>
#include <iostream>
#include <process.h>

class ThreadSafeWorker
{
private:
	int m_workerId;
	bool m_shouldStop;
	HANDLE m_hThread;
	unsigned m_threadId;

public:
	ThreadSafeWorker(int workerId)
		: m_workerId(workerId), m_shouldStop(false), m_hThread(NULL) {}
	~ThreadSafeWorker() { Stop(); }

	bool Start()
	{
		// ������ �۾��� �������̶��
		if (m_hThread != NULL){ return false; }

		// ������ ���� �� �۾� ����
		m_shouldStop = false;
		m_hThread = reinterpret_cast<HANDLE>(
			_beginthreadex(
				NULL,
				0,
				ThreadProc,			// ������ �Լ�
				this,				// �Ű� ����
				0,
				&m_threadId
			)
		);

		// ������ ���� ��� ��ȯ
		return m_hThread != NULL;
	}

	void Stop()
	{
		// �ߴܽ�ų �����尡 ���� ���
		if (m_hThread == NULL) return;

		// ������ �۾��� ���������� ��� �� Close Handle
		m_shouldStop = true;
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

private:
	static unsigned __stdcall ThreadProc(void* pParam)
	{
		ThreadSafeWorker* worker = static_cast<ThreadSafeWorker*>(pParam);
		return worker->WorkerFunctioin();
	}

	unsigned WorkerFunctioin()
	{
		std::cout << "��Ŀ " << m_workerId << " ���� (������ ID: " << m_threadId << ")" << std::endl;

		int iteration = 0;
		while (!m_shouldStop && iteration < 10)
		{
			std::cout << "> ��Ŀ " << m_workerId << " �۾� ��... " << ++iteration << std::endl;
			Sleep(500);
		}

		std::cout << "��Ŀ " << m_workerId << " ����" << std::endl;
		return 0;
	}
};

int main()
{
	ThreadSafeWorker worker(100);

	if (worker.Start())
	{
		std::cout << "��Ŀ ���۵�" << std::endl;
		Sleep(3000);
		worker.Stop();
		std::cout << "��Ŀ ���� �Ϸ�\n" << std::endl;
	}

	return 0;
}