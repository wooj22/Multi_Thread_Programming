/*
실습 1: CreateThread

WINAPI 호출 규약: 스레드 함수는 반드시 DWORD WINAPI 형태로 선언해야 한다.
매개변수 전달: ThreadData 구조체를 통해 여러 데이터를 스레드에 전달한다.
병렬 실행: 두 개의 워커 스레드와 메인 스레드가 동시에 작업을 수행한다.
스레드 대기: WaitForMultipleObjects를 사용해 모든 스레드의 완료를 기다린다.
리소스 정리: CloseHandle로 스레드 핸들을 반드시 해제한다.

=== CreateThread 예제 ===

스레드 생성 완료:
- 스레드 1 ID: 1234
- 스레드 2 ID: 5678

스레드 1 시작: 첫 번째 워커 스레드
스레드 2 시작: 두 번째 워커 스레드
메인 스레드: 다른 작업 수행 중...
스레드 1: 작업 1/3 수행 중...
스레드 2: 작업 1/5 수행 중...
메인 스레드: 작업 1/4
...
*/

#include <windows.h>
#include <iostream>

// 스레드에 전달할 데이터 구조체
struct ThreadData
{
	int threadId;
	const char* message;
	int count;
};

// 스레드 함수
// 반드시 WINAPI 호출 규약을 사용 (DWORD WINAPI)
DWORD WINAPI WorkerThread(LPVOID lpParam)
{
	ThreadData* data = static_cast<ThreadData*>(lpParam);
	printf("스레드 %d 시작 : %s\n", data->threadId, data->message);

	// 작업 수행
	for (int i = 1; i <= data->count; i++)
	{
		printf("> 스레드 %d: 작업 %d/%d 수행 중...\n", data->threadId, i, data->count);
		Sleep(1000);
	}

	// 작업 완료, 스레드 종료 코드 반환
	printf("스레드 %d 완료!\n", data->threadId);
	return data->threadId * 100;
}

int main()
{
	printf("==== Create Thread 예제 ====\n\n");

	// Thread Struct Data
	ThreadData threadData1 = { 1, "첫 번째 워커 스레드", 3 };
	ThreadData threadData2 = { 2, "두 번째 워커 스레드", 5 };
	DWORD threadId1, threadId2;

	// Create Thread
	HANDLE hThread1 = CreateThread(
		NULL,                   // 기본 보안 설정
		0,                      // 기본 스택 크기 (1MB)
		WorkerThread,           // 스레드 함수 ⭐
		&threadData1,           // 매개변수 ⭐
		0,                      // 즉시 실행
		&threadId1              // 스레드 ID 받을 변수
	);


	HANDLE hThread2 = CreateThread(
		NULL,                   
		0,                      
		WorkerThread,           
		&threadData2,           
		0,                      
		&threadId2              
	);

	// 스레드 생성 성공 여부 확인
	if (hThread1 == NULL) {
		printf("첫 번째 스레드 생성 실패! 오류 코드: %lu\n", GetLastError());
		return 1;
	}

	if (hThread2 == NULL) {
		printf("두 번째 스레드 생성 실패! 오류 코드: %lu\n", GetLastError());
		CloseHandle(hThread1);
		return 1;
	}

	printf("--- 스레드 생성 완료 ---\n");
	printf("스레드 1 ID : %lu\n", threadId1);
	printf("스레드 2 ID : %lu\n\n", threadId2);

	// 메인 스레드 작업 수행
	printf("메인 스레드 : 다른 작업 수행 중...\n");
	for (int i = 1; i <= 4; i++)
	{
		printf("> 메인 스레드 : 작업 %d/4\n", i);
		Sleep(800);
	}
	printf("메인 스레드 작업 완료\n\n");

	// 모든 스레드가 완료될때 까지 대기
	printf("모든 스레드 완료 대기 중...\n");
	HANDLE threads[] = { hThread1 , hThread2 };
	DWORD waitResult = WaitForMultipleObjects(
		2,          // 대기할 객체 수
		threads,    // 핸들 배열
		TRUE,       // 모든 객체가 신호 상태가 될 때까지 대기
		INFINITE    // 무한 대기
	);

	if (waitResult == WAIT_OBJECT_0)
	{
		printf("모든 스레드가 완료되었습니다.\n");

		// 스레드 종료 코드 확인
		DWORD exitCode1, exitCode2;
		GetExitCodeThread(hThread1, &exitCode1);
		GetExitCodeThread(hThread2, &exitCode2);

		printf("스레드 1 종료 코드 : %lu\n", exitCode1);
		printf("스레드 2 종료 코드 : %lu\n", exitCode2);
	}
	else
	{
		printf("스레드 대기 중 오류 발생: %lu\n", GetLastError());
	}

	// 핸들 정리 ⭐
	CloseHandle(hThread1);
	CloseHandle(hThread2);

	printf("\n프로그램 종료\n");
	return 0;
}