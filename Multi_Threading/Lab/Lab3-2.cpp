## 문제 2: 단 한번만 실행되는 초기화 구현하기

* *요구사항 : **
여러 스레드가 `InitializeResource()` 함수를 호출하지만, 실제 리소스 초기화 작업(`printf`로 대체)은 단 한 번만 수행되어야 한다. `Interlocked` 함수를 사용하여 첫 번째로 이 함수를 호출한 스레드만 초기화 코드를 실행하도록 만들자.

** 실습 코드 : **

```cpp
#include <windows.h>
#include <iostream>
#include <vector>

#define NUM_THREADS 5

LONG g_initialized = 0; // 0: 초기화 안됨, 1: 초기화 완료

void InitializeResource() {
    // TODO: g_initialized 값을 1로 바꾸되,
    // 오직 바꾸기 전의 값이 0이었던 스레드만 아래 "리소스 초기화" 코드를 실행하도록 만드세요.
    // 힌트: 값을 교환(Exchange)하고 이전 값을 반환하는 함수를 사용하세요.


    // if (이전 값이 0이었다면?) {
    //     printf("[Thread %lu] 리소스 초기화 수행!\n", GetCurrentThreadId());
    // } else {
    //     printf("[Thread %lu] 이미 다른 스레드가 초기화를 완료했습니다.\n", GetCurrentThreadId());
    // }
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    InitializeResource();
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
    return 0;
}
```

** 정답 및 해설 : **

```cpp
// ... (생략) ...

void InitializeResource() {
    // TODO: g_initialized 값을 1로 바꾸되,
    // 오직 바꾸기 전의 값이 0이었던 스레드만 아래 "리소스 초기화" 코드를 실행하도록 만드세요.
    if (InterlockedExchange(&g_initialized, 1) == 0) {
        printf("[Thread %lu] 리소스 초기화 수행!\n", GetCurrentThreadId());
    }
    else {
        printf("[Thread %lu] 이미 다른 스레드가 초기화를 완료했습니다.\n", GetCurrentThreadId());
    }
}

// ... (생략) ...
```

*** 해설** :
*`InterlockedExchange(& g_initialized, 1)`는 `g_initialized`의 값을 원자적으로 `1`로 설정하고, ** 설정하기 전의 원래 값** 을 반환합니다.
* 가장 먼저 도착한 스레드는 `g_initialized`가 `0`이었으므로 `0`을 반환받아 `if`문 안의 초기화 코드를 실행합니다.
* 그 뒤에 도착하는 스레드들은 `g_initialized`가 이미 `1`로 바뀐 상태이므로 `1`을 반환받아 `else` 문을 실행하게 됩니다.

---- -