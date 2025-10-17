## 문제 1: 스레드 안전한 작업 카운터 만들기
* *요구사항 : **
여러 개의 스레드가 동시에 `Work()` 함수를 호출한다. `Work()` 함수는 시간이 걸리는 작업을 흉내 내기 위해 `Sleep()`을 사용한다.전역 변수 `g_workCount`를 사용하여 현재 몇 개의 스레드가 `Work()` 함수를 실행 중인지 추적해야 한다. `Interlocked` 함수를 사용하여 `g_workCount`를 스레드에 안전하게 증가시키고 감소시킨다.

** 실습 코드 : **

```cpp
#include <windows.h>
#include <iostream>
#include <vector>

#define NUM_THREADS 10
#define MAX_CONCURRENT_WORKS 3 // 동시에 최대 3개까지만 작업 허용

LONG g_workCount = 0; // 현재 작업 중인 스레드 수

void Work() {
    // TODO 1: 현재 작업 중인 스레드 수를 원자적으로 1 증가시키고,
    // 그 결과(증가 후의 값)를 newCount 변수에 저장하세요.
    LONG newCount = 0; // 이 부분을 수정하세요.

    if (newCount > MAX_CONCURRENT_WORKS) {
        printf("[Thread %lu] 작업량이 많아 실행하지 못했습니다. (현재 %ld개 실행 중)\n", GetCurrentThreadId(), newCount);
    }
    else {
        printf("[Thread %lu] 작업 시작! (현재 %ld개 실행 중)\n", GetCurrentThreadId(), newCount);
        Sleep(100); // 작업을 흉내 냅니다.
        printf("[Thread %lu] 작업 완료!\n", GetCurrentThreadId());
    }

    // TODO 2: 작업이 끝났으므로, 현재 작업 중인 스레드 수를 원자적으로 1 감소시키세요.

}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    for (int i = 0; i < 5; ++i) {
        Work();
        Sleep(50);
    }
    return 0;
}

int main() {
    std::vector<HANDLE> hThreads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
        if (hThread) {
            hThreads.push_back(hThread);
        }
    }

    WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);

    for (HANDLE hThread : hThreads) {
        CloseHandle(hThread);
    }

    printf("모든 스레드 실행 완료. 최종 작업 카운트: %ld\n", g_workCount); // 최종적으로 0이 되어야 함
    return 0;
}
```

** 정답 및 해설 : **

```cpp
// ... (생략) ...

void Work() {
    // TODO 1: 현재 작업 중인 스레드 수를 원자적으로 1 증가시키고,
    // 그 결과(증가 후의 값)를 newCount 변수에 저장하세요.
    LONG newCount = InterlockedIncrement(&g_workCount);

    if (newCount > MAX_CONCURRENT_WORKS) {
        printf("[Thread %lu] 작업량이 많아 실행하지 못했습니다. (현재 %ld개 실행 중)\n", GetCurrentThreadId(), newCount);
    }
    else {
        printf("[Thread %lu] 작업 시작! (현재 %ld개 실행 중)\n", GetCurrentThreadId(), newCount);
        Sleep(100); // 작업을 흉내 냅니다.
        printf("[Thread %lu] 작업 완료!\n", GetCurrentThreadId());
    }

    // TODO 2: 작업이 끝났으므로, 현재 작업 중인 스레드 수를 원자적으로 1 감소시키세요.
    InterlockedDecrement(&g_workCount);
}

// ... (생략) ...
```

*** 해설** :
*`InterlockedIncrement(& g_workCount)`: 여러 스레드가 동시에 접근해도 `g_workCount`를 안전하게 1 증가시킵니다. * *연산 후의 값 * *을 반환하므로, 현재 몇 개의 작업이 실행 중인지 즉시 알 수 있습니다.
* `InterlockedDecrement(& g_workCount)`: 작업이 끝난 후 카운트를 안전하게 1 감소시켜 다른 스레드가 진입할 수 있도록 합니다.이 두 함수를 사용함으로써 `g_workCount`는 항상 정확한 값을 유지하게 됩니다.

---- -