/*

	 [ _beginthreadex ] 

	 _beginthreadex()는 C 런타임 라이브러리에서 제공하는 스레드 생성 함수로, 
	 C++ 프로그램에서는 CreateThread() 보다 _beginthreadex() 를 권장

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
		// 스레드 작업이 진행중이라면
		if (m_hThread != NULL){ return false; }

		// 스레드 생성 및 작업 시작
		m_shouldStop = false;
		m_hThread = reinterpret_cast<HANDLE>(
			_beginthreadex(
				NULL,
				0,
				ThreadProc,			// 스레드 함수
				this,				// 매개 변수
				0,
				&m_threadId
			)
		);

		// 스레드 생성 결과 반환
		return m_hThread != NULL;
	}

	void Stop()
	{
		// 중단시킬 스레드가 없을 경우
		if (m_hThread == NULL) return;

		// 스레드 작업이 끝날때까지 대기 후 Close Handle
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
		std::cout << "워커 " << m_workerId << " 시작 (스레드 ID: " << m_threadId << ")" << std::endl;

		int iteration = 0;
		while (!m_shouldStop && iteration < 10)
		{
			std::cout << "> 워커 " << m_workerId << " 작업 중... " << ++iteration << std::endl;
			Sleep(500);
		}

		std::cout << "워커 " << m_workerId << " 종료" << std::endl;
		return 0;
	}
};

int main()
{
	ThreadSafeWorker worker(100);

	if (worker.Start())
	{
		std::cout << "워커 시작됨" << std::endl;
		Sleep(3000);
		worker.Stop();
		std::cout << "워커 정리 완료\n" << std::endl;
	}

	return 0;
}