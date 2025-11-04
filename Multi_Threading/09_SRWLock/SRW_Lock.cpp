
/*
	[ SRWLOCK (Slim Reader/Writer Lock) ]
	
	고성능 Reader/Writer 동기화 도구
	읽기와 쓰기 작업을 구분하여 여러 스레드가 동시에 읽기 수행할 수 있도록 하는 동기화 매커니즘으로 
	여러 스레드가 동시에 읽을 수 있도록 허용하면서도 쓰지 작업은 배타적으로 수행되도록 보장하는 객체

	- SRWLOCK은 매우 빠르며 커널 오브젝트를 생성하지 않기 때문에 성능이 중요한 곳에서 자주 사용됨
	- 한 번에 여러 Reader가 접근 가능, Writer는 단독 접근만 가능
	- 스레드가 락을 획득하지 못하면 커널로 진입해 대기할 수 있음
	- SRWLOCK은 같은 스레드가 동일락을 중첩 획득(AcquireSRWLockExclusiv 호출)하면 교착상태가 발생함

	- 읽기 작업
		AcquireSRWLockShared(&lock);
		// 읽기 작업 수행, 여러 스레드 접근 가능
		ReleaseSRWLockShared(&lock);

	- 쓰기 작업
		AcquireSRWLockExclusive(&lock);
		// 쓰기 작업 수행, 한 스레드만 접근 가능
		ReleaseSRWLockExclusive(&lock);

	- 비차단 시도 : 락을 획득할 수 있는지를 미리 확인할 수 있다.
	  if(TryAcquireSRWLockShared(&lock))	// 읽기 가능
	  if(TryAcquireSRWLockExclusive(&lock)) // 쓰기 가능
*/

#include <windows.h>
#include <stdio.h>

// SRWLOCK
SRWLOCK lock;

// Reader
DWORD WINAPI ReaderThread(LPVOID param) 
{
	AcquireSRWLockShared(&lock);
	printf("Reader acquired lock\n");
	Sleep(1000);
	ReleaseSRWLockShared(&lock);
	return 0;
}

// Wirter
DWORD WINAPI WriterThread(LPVOID param) 
{
	AcquireSRWLockExclusive(&lock);
	printf("Writer acquired lock\n");
	Sleep(1000);
	ReleaseSRWLockExclusive(&lock);
	return 0;
}

int main() 
{
	InitializeSRWLock(&lock);

	HANDLE h1 = CreateThread(NULL, 0, ReaderThread, NULL, 0, NULL);
	HANDLE h2 = CreateThread(NULL, 0, WriterThread, NULL, 0, NULL);

	WaitForSingleObject(h1, INFINITE);
	WaitForSingleObject(h2, INFINITE);

	return 0;
}