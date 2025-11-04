/*
	[ 스핀락 (Spin Lock) ]
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
		// 다른 스레드가 locked를 0으로 바꿀 때까지 계속 시도(spin)
		while (InterlockedCompareExchange(&locked, 1, 0) != 0)
		{
			YieldProcessor();	// 다른 스레드에게 CPU를 양보하여 무한 루프 방지 성능 향상
		}
	}

	void Unlock()
	{
		// locked 값을 0으로 설정하여 다른 스레드가 진입할 수 있도록 함
		InterlockedExchange(&locked, 0);
	}
};


// 전역 변수 및 락 객체
SpinLock		spinLock;							// 공유 데이터를 보호할 스핀락 객체			
long			counter = 0;						// 스레드들이 증가시킬 공유 변수		
const int		THREAD_COUNT = 10;
const int		INCREMENT_PER_THREAD = 100000;


// 스레드 함수
unsigned int __stdcall ThreadFunction(void* arg)
{
	int threadId = *(static_cast<int*>(arg));

	for (int i = 0; i < INCREMENT_PER_THREAD; i++)
	{
		spinLock.Lock();	// ----임계 구역 진입
		counter++;
		spinLock.Unlock();  // ----임계 구역 해제
	}

	delete static_cast<int*>(arg);
	return 0;
}



int main()
{
	vector<HANDLE> threadHandles;
	cout << THREAD_COUNT << "개의 스레드가 각각 " << INCREMENT_PER_THREAD << "번씩 counter를 증가시킵니다." << endl;

	// 스레드 생성
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		int* threadId = new int(i);

		HANDLE hThread = (HANDLE)_beginthreadex(
			NULL,                   // 보안 속성 (기본값)
			0,                      // 스택 크기 (기본값)
			ThreadFunction,         // 스레드 함수 포인터
			threadId,               // 스레드 함수에 전달할 인자
			0,                      // 생성 플래그 (즉시 실행)
			NULL                    // 스레드 ID를 받을 변수 주소 (필요 없음)
		);

		if (hThread)
		{
			threadHandles.push_back(hThread);
		}
		else
		{
			cerr << "스레드 생성 실패! 오류 코드: " << GetLastError() << endl;
			delete threadId;
		}
	}

	// 모든 스레드가 종료될 때 까지 대기
	WaitForMultipleObjects((DWORD)threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

	// 스레드 핸들 정리
	for (auto h : threadHandles)
	{
		CloseHandle(h);
	}

	long long expected_result = (long long)THREAD_COUNT * INCREMENT_PER_THREAD;
	cout << "예상 counter 값: " << expected_result << endl;
	cout << "실제 counter 값: " << counter << endl;

	if (counter == expected_result) cout << "성공: counter 값이 예상과 일치합니다!" << endl;
	else cout << "실패: counter 값이 예상과 일치하지 않습니다!" << endl;

	return 0;
}