/*
	[세마포어 (Semaphore)]
	공유 자원에 접근할 수 있는 스레드의 개수를 제어하는 동기화 객체.
	한번에 N개의 스레드가 자원에 접근하도록 허용하고 싶을 때 사용하며,
	세마포어 카운트 값이 허용 가능한 스레드의 최대 개수를 나타낸다.

	1. CreateSemaphore(보안속성, 초기카운트, 최대카운트, 이름)
		: 세마포어 객체 생성
	   - 초기카운트: 세마포어가 생성될 때의 초기 카운트 값 (즉, 동시에 접근 가능한 스레드 수)
	   - 최대카운트: 세마포어가 가질 수 있는 최대 카운트 값
	
	2. WaitForSingleObject(세마포어핸들, 대기시간)
		: 세마포어 자리가 날 때까지 대기
	   - 세마포어 카운트가 0보다 크면 카운트를 1 감소시키고 즉시 리턴.
	   - 세마포어 카운트가 0이면 다른 스레드가 ReleaseSemaphore를 호출할 때까지 대기.

	3. ReleaseSemaphore(세마포어핸들, 증가할카운트, 이전카운트저장위치)
		: 세	마포어 해제(증가)
	   - 세마포어 카운트를 지정한 값만큼 증가시킨다.
	   - 대기 중인 스레드가 있다면, 그 중 하나가 깨어나서 세마포어를 획득할 수 있다.
*/


// 세마포어로 작업 속도 제어하기 (Rate Limiting)
#include <windows.h>
#include <iostream>
#include <queue>
#include <string>
#include <process.h> // _beginthreadex, _endthreadex

using namespace std;

queue<string>		taskQueue;		// 작업을 저장할 공유 큐
CRITICAL_SECTION	cs;			    // 임계구역 (크리티컬 섹션) : 큐에 대한 동기화 객체
HANDLE			    semaphore;		// 처리 가능한 작업의 최대 개수를 제한하는 세마포어

volatile bool isRunning = true;		// 작업 생성, 처리 상태 플래그
const int MAX_BUFFER_SIZE = 5;		// 버퍼(큐)의 최대 크기. 세마포어의 최대 카운트


/* 작업 생성자(Producer) 스레드 함수 */
// 빠르게 작업을 생성해서 큐에 추가
unsigned int __stdcall ProducerThread(void* pParam)
{
	int taskCount = 0;


	while (taskCount < 20)
	{
		// 1. 세마포어 대기
		// 만약 세마포어 카운트가 0이면 (즉, 카운트가 꽉 찼다면)
		// Consumer가 작업을 처리하고 세마포어를 해제할 때 까지 여기서 대기
		// count > 0 이면 : 세마포어는 즉시 카운트를 1 감소시키고 함수를 리턴. 스레드는 대기 없이 다음 코드 실행.
		// count == 0 이면 : 스레드는 카운트가 0보다 커질때 까지(즉, 다른 스레드가 ReleaseSemaphore 호출할 때 까지) 무한정 대기(block)
		WaitForSingleObject(semaphore, INFINITE);

		// 2. 크리티컬 섹션을 통해 큐애 대한 접근 보호
		EnterCriticalSection(&cs);

		// 작업 생성
		taskCount++;
		string task = "작업 " + to_string(taskCount);
		taskQueue.push(task);
		cout << "[생성] " << task << "(현재 큐 크기 : " << taskQueue.size() << ")\n";

		// 3. 크리티컬 섹션 해제
		LeaveCriticalSection(&cs);

		// 빠른 작업 생성을 시뮬레이션하기 위해 짧은 시간 대기
		Sleep(100);
	}

	// 작업 생성이 완료되면 Consumer 스레드 종료를 위해 플래그 설정
	isRunning = false;
	cout << "\n--- 모든 작업 생성 완료 ---\n" << endl;
	return 0;
}

/* 작업 소비자(Consumer) 스레드 함수 */
// 큐에서 작업을 가져와 느치게 처리
unsigned int __stdcall ConsumerThread(void* pParam)
{
	while (isRunning || !taskQueue.empty())
	{
		string task;
		bool hasTask = false;

		// 1. 크리티컬 섹션을 통해 큐애 대한 접근 보호
		EnterCriticalSection(&cs);

		// 큐에서 작업 가져오기
		if (!taskQueue.empty())
		{
			
			task = taskQueue.front();
			taskQueue.pop();
			hasTask = true;
		}

		// 2. 크리티컬 섹션 해제
		LeaveCriticalSection(&cs);

		if (hasTask)
		{
			std::cout << "    [처리] " << task << " 시작..." << std::endl;

			// 느린 작업 처리를 시뮬레이션하기 위해 긴 시간 대기
			Sleep(500);

			std::cout << "    [처리] " << task << " 완료!" << std::endl;

			// 3. 작업 처리가 끝났음을 알리고, 비어있는 슬롯이 생겼음을 세마포어에 알립니다.
			//    이 호출로 인해 세마포어 카운트가 1 증가하고,
			//    Producer 스레드가 대기 중이었다면 다시 작업을 생성할 수 있게 됩니다.
			ReleaseSemaphore(semaphore, 1, NULL);
		}
	}
}


int main()
{
	HANDLE hThreads[2];
	unsigned int threadID;

	// 1. 동기화 객체 초기화
	InitializeCriticalSection(&cs);

	// 세마포어 생성
	// 초기 카운트: MAX_BUFFER_SIZE (처음에는 버퍼가 비어있으므로 최대치만큼 작업 생성 가능)
	// 최대 카운트: MAX_BUFFER_SIZE (버퍼의 최대 크기)
	semaphore = CreateSemaphore(
		NULL,            // 기본 보안 속성
		MAX_BUFFER_SIZE, // 초기 카운트 - 현재 사용 가능한 자원 개수
		MAX_BUFFER_SIZE, // 최대 카운트 - 자원의 최대 개수
		NULL             // 이름 없는 세마포어
	);

	if (semaphore == NULL)
	{
		std::cerr << "세마포어 생성 실패: " << GetLastError() << std::endl;
		return 1;
	}

	std::cout << "--- 세마포어를 이용한 속도 제어(Rate Limiting) 시뮬레이션 시작 ---" << std::endl;
	std::cout << "최대 버퍼 크기: " << MAX_BUFFER_SIZE << std::endl << std::endl;

	// 2. 스레드 생성
	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, &ProducerThread, NULL, 0, &threadID);
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, &ConsumerThread, NULL, 0, &threadID);

	// 3. 모든 스레드가 종료될 때까지 대기
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

	std::cout << "\n--- 모든 작업 처리 완료. 프로그램 종료. ---" << std::endl;

	// 4. 리소스 해제
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
	CloseHandle(semaphore);
	DeleteCriticalSection(&cs);

	return 0;
}