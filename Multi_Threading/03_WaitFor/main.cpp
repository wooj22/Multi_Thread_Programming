/*

	 [ WaitForSingleObject(핸들, 대기 시간) ]
	 [ WaitForMultipleObjects(핸들 개수, 핸들 배열, True(모든객체)/False(하나라도), 대기 시간) ]

	 반환 값을 통해 작업을 다 마치고 스레드가 종료된건지, 도중에 종료된건지 알 수 있음
	 - WAIT_OBJECT_0 : 객체가 신호 상태가 됨
	 - WAIT_TIMEOU T: 시간 초과
	 - WAIT_FAILED : 오류 발생

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <process.h>
#include <random>

// 작업 유형별 스레드 함수들 
// 매개변수로 넘길 스레드 함수의 반환값은 무조건 unsigned이다.
unsigned __stdcall FastWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[빠른작업 " << id << "] 시작" << std::endl;
    Sleep(1000);
    std::cout << "[빠른작업 " << id << "] 완료" << std::endl;
    return 100 + id;
}

unsigned __stdcall SlowWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[느린작업 " << id << "] 시작" << std::endl;
    Sleep(3000);
    std::cout << "[느린작업 " << id << "] 완료" << std::endl;
    return 200 + id;
}

unsigned __stdcall UnpredictableWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[불규칙작업 " << id << "] 시작" << std::endl;

    // C++11 random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5);
    int sleepTime = dis(gen) * 1000;

    std::cout << "[불규칙작업 " << id << "] 예상 시간: " << sleepTime / 1000 << "초" << std::endl;

    Sleep(sleepTime);
    std::cout << "[불규칙작업 " << id << "] 완료" << std::endl;
    return 300 + id;
}

// WaitForSingleObject - 주기적 상태 확인
void DemonstrateSingleObjectWait()
{
    std::cout << "=== WaitForSingleObject ===" << std::endl;

    int workerId = 1;
    unsigned threadId;

    // 스레드 생성 및 작업 시작
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL,           
        0,              
        SlowWorker,     // 스레드 함수
        &workerId,      // 매개변수
        0,              
        &threadId       
    ));

    // 핸들 유효성 검사
    if (hThread == NULL)
    {
        std::cout << "스레드 생성 실패! 오류 코드: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "스레드 작업 중... 2초마다 상태 확인" << std::endl;

    while (true)
    {
        // 2초 대기 후 hThread 상태 반환
        DWORD waitResult = WaitForSingleObject(hThread, 2000);  

        switch (waitResult)
        {
        // WAIT_OBJECT_0 : 신호 상태
        case WAIT_OBJECT_0:
            std::cout << "스레드 완료!" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "종료 코드: " << exitCode << std::endl;
            }

            CloseHandle(hThread);
            return;
        
        // WAIT_OBJECT_0 : 시간 초과
        case WAIT_TIMEOUT:
            std::cout << "아직 실행 중... 계속 대기" << std::endl;
            break;

        // WAIT_FAILED : 오류 발생
        case WAIT_FAILED:
            std::cout << "대기 중 오류 발생! 오류 코드: " << GetLastError() << std::endl;
            CloseHandle(hThread);
            return;
        }
    }
}

// WaitForMultipleObjects - 완료되는 스레드 순서 확인
void DemonstrateMultipleObjectsWait()
{
    std::cout << "\n=== WaitForMultipleObjects ===" << std::endl;

    // 스레드 생성 및 작업 시작
    int ids[] = { 1, 2, 3 };
    HANDLE threads[3];
    unsigned threadIds[3];

    threads[0] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, FastWorker, &ids[0], 0, &threadIds[0]));
    threads[1] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, SlowWorker, &ids[1], 0, &threadIds[1]));
    threads[2] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, UnpredictableWorker, &ids[2], 0, &threadIds[2]));

    // 핸들 유효성 검사
    bool allValid = true;
    for (int i = 0; i < 3; i++)
    {
        if (threads[i] == NULL)
        {
            std::cout << "스레드 " << i + 1 << " 생성 실패! 오류 코드: " << GetLastError() << std::endl;
            allValid = false;

            // 이미 생성된 스레드들 정리
            for (int j = 0; j < i; j++)
            {
                CloseHandle(threads[j]);
            }
        }
    }
    if (!allValid) return;

    // 하나라도 종료될때까지 대기
    std::cout << "\n--- 시나리오 1 : 첫 번째 완료되는 스레드 대기 ---" << std::endl;
    DWORD firstCompleted = WaitForMultipleObjects(3, threads, FALSE, INFINITE); 

    // 만약 첫 번째 스레드(threads[0])가 종료되었다면, WAIT_OBJECT_0 값을 리턴
    // 만약 두 번째 스레드(threads[1])가 종료되었다면, WAIT_OBJECT_0 + 1 값을 리턴 
    // 만약 세 번째 스레드(threads[2])가 종료되었다면, WAIT_OBJECT_0 + 2 값을 리턴
    if (firstCompleted >= WAIT_OBJECT_0 && firstCompleted < WAIT_OBJECT_0 + 3)
    {
        int completedIndex = firstCompleted - WAIT_OBJECT_0;
        std::cout << "첫 번째 완료 : 스레드 " << completedIndex + 1 << std::endl;

        // 완료된 스레드의 종료 코드 확인
        DWORD exitCode;
        if (GetExitCodeThread(threads[completedIndex], &exitCode))
        {
            std::cout << "완료된 스레드의 종료 코드: " << exitCode << std::endl;
        }
    }

    // 모든 스레드가 종료될때까지 대기
    std::cout << "\n--- 시나리오 2 : 모든 스레드 완료 대기 ---" << std::endl;
    DWORD allCompleted = WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    if (allCompleted == WAIT_OBJECT_0)
    {
        std::cout << "모든 스레드 완료!" << std::endl;

        // 각 스레드의 종료 코드 확인
        for (int i = 0; i < 3; i++)
        {
            DWORD exitCode;
            if (GetExitCodeThread(threads[i], &exitCode))
            {
                std::cout << "스레드 " << i + 1 << " 종료 코드: " << exitCode << std::endl;
            }
        }
    }
    else if (allCompleted == WAIT_FAILED)
    {
        std::cout << "모든 스레드 대기 중 오류 발생! 오류 코드: " << GetLastError() << std::endl;
    }

    // 핸들 정리
    for (int i = 0; i < 3; i++)
    {
        CloseHandle(threads[i]);
    }
}

// WaitForSingleObject - 타임 아웃
void DemonstrateTimeoutWait()
{
    std::cout << "\n=== 타임아웃 처리 데모 ===" << std::endl;

    int workerId = 10;
    unsigned threadId;

    // 스레드 생성 및 작업 시작
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL, 0, SlowWorker, &workerId, 0, &threadId
    ));

    // 핸들 유효성 검사
    if (hThread == NULL)
    {
        std::cout << "스레드 생성 실패! 오류 코드: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "2초 타임아웃으로 대기 (3초 작업이므로 타임아웃 예상)" << std::endl;

    // 2초 대기후 스레드 상태 반환
    DWORD waitResult = WaitForSingleObject(hThread, 2000);

    switch (waitResult)
    {
    case WAIT_OBJECT_0:
        std::cout << "예상과 달리 빨리 완료됨!" << std::endl;

        DWORD exitCode;
        if (GetExitCodeThread(hThread, &exitCode))
        {
            std::cout << "종료 코드: " << exitCode << std::endl;
        }
        break;

    case WAIT_TIMEOUT:
        std::cout << "예상대로 타임아웃 발생" << std::endl;
        std::cout << "사용자에게 진행 상황 보고 후 계속 대기..." << std::endl;

        // 무한 대기로 변경
        if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
        {
            std::cout << "최종적으로 완료됨" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "종료 코드: " << exitCode << std::endl;
            }
        }
        break;

    case WAIT_FAILED:
        std::cout << "대기 실패! 오류 코드: " << GetLastError() << std::endl;
        break;
    }

    CloseHandle(hThread);
}

// WaitForSingleObject - 주기적 상태 확인
void DemonstratePeriodicStatusCheck()
{
    std::cout << "\n=== 주기적 상태 확인 데모 ===" << std::endl;

    int workerId = 20;
    unsigned threadId;

    // 스레드 생성 및 작업 시작
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL, 0, UnpredictableWorker, &workerId, 0, &threadId
    ));

    // 헨들 유효성 체크
    if (hThread == NULL)
    {
        std::cout << "스레드 생성 실패!" << std::endl;
        return;
    }

    // 1초마다 스레드 상태 확인
    int checkCount = 0;
    while (true)
    {
        DWORD waitResult = WaitForSingleObject(hThread, 1000);  // 1초마다 확인

        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            std::cout << "\n작업 완료! 총 " << checkCount << "번 확인함" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "종료 코드: " << exitCode << std::endl;
            }

            CloseHandle(hThread);
            return;

        case WAIT_TIMEOUT:
            checkCount++;
            std::cout << "." << std::flush;  // 진행 상황 표시
            if (checkCount % 10 == 0)
            {
                std::cout << " (" << checkCount << "초 경과)" << std::endl;
            }
            break;

        case WAIT_FAILED:
            std::cout << "\n상태 확인 실패!" << std::endl;
            CloseHandle(hThread);
            return;
        }
    }
}

int main()
{
    std::cout << "Windows 스레드 대기 함수 데모 (_beginthreadex 사용)\n" << std::endl;

    // 각 데모 함수 실행
    DemonstrateSingleObjectWait();
    DemonstrateMultipleObjectsWait();
    DemonstrateTimeoutWait();
    DemonstratePeriodicStatusCheck();

    std::cout << "\n모든 데모 완료!" << std::endl;
    return 0;
}