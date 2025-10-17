## 문제 4: 스레드별 데이터 합산하기

* *요구사항 : **
여러 스레드가 각자 계산한 값(`localValue`)을 전역 합계 변수 `g_totalSum`에 누적한다. `InterlockedAdd` 또는 `InterlockedExchangeAdd`를 사용하여 이 과정을 스레드 세이프하게 만든다.

    * *실습 코드 : **

    ```cpp
#include <windows.h>
#include <iostream>
#include <vector>

#define NUM_THREADS 10

    LONGLONG g_totalSum = 0;

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    LONGLONG localValue = 0;
    for (int i = 0; i < 10000; ++i) {
        localValue += (i + 1);
    }

    // TODO: 이 스레드에서 계산한 localValue를 전역 변수 g_totalSum에 원자적으로 더하세요.
    // 64비트 버전을 사용해야 합니다.

    printf("[Thread %lu] 로컬 합계 %lld를 더했습니다.\n", GetCurrentThreadId(), localValue);
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

    // 1부터 10000까지의 합 = 50005000
    // 스레드 10개이므로 예상 결과 = 50005000 * 10 = 500050000
    printf("\n최종 합계: %lld (예상: 500050000)\n", g_totalSum);
    return 0;
}
```

** 정답 및 해설 : **

```cpp
// ... (생략) ...

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    LONGLONG localValue = 0;
    for (int i = 0; i < 10000; ++i) {
        localValue += (i + 1);
    }

    // TODO: 이 스레드에서 계산한 localValue를 전역 변수 g_totalSum에 원자적으로 더하세요.
    // 64비트 버전을 사용해야 합니다.
    InterlockedAdd64(&g_totalSum, localValue);
    // 또는 InterlockedExchangeAdd64(&g_totalSum, localValue); 도 가능합니다.
    // 이 문제에서는 반환값이 필요 없으므로 둘 다 정답입니다.

    printf("[Thread %lu] 로컬 합계 %lld를 더했습니다.\n", GetCurrentThreadId(), localValue);
    return 0;
}

// ... (생략) ...
```

*** 해설** :
*`g_totalSum`은 여러 스레드가 동시에 접근하여 수정하는 공유 자원입니다. `g_totalSum += localValue; ` 와 같은 일반 연산은 원자적이지 않아 데이터 경쟁(Race Condition)을 일으키고 최종 합계가 틀리게 됩니다.
* `InterlockedAdd64(& g_totalSum, localValue)`는 `g_totalSum`에 `localValue`를 더하는 과정을 원자적으로 처리하여 데이터 손실 없이 정확한 합계를 보장합니다. `LONGLONG` 타입이므로 64비트 버전인 `InterlockedAdd64`를 사용해야 합니다.

---- -