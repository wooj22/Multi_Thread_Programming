## ���� 4: �����庰 ������ �ջ��ϱ�

* *�䱸���� : **
���� �����尡 ���� ����� ��(`localValue`)�� ���� �հ� ���� `g_totalSum`�� �����Ѵ�. `InterlockedAdd` �Ǵ� `InterlockedExchangeAdd`�� ����Ͽ� �� ������ ������ �������ϰ� �����.

    * *�ǽ� �ڵ� : **

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

    // TODO: �� �����忡�� ����� localValue�� ���� ���� g_totalSum�� ���������� ���ϼ���.
    // 64��Ʈ ������ ����ؾ� �մϴ�.

    printf("[Thread %lu] ���� �հ� %lld�� ���߽��ϴ�.\n", GetCurrentThreadId(), localValue);
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

    // 1���� 10000������ �� = 50005000
    // ������ 10���̹Ƿ� ���� ��� = 50005000 * 10 = 500050000
    printf("\n���� �հ�: %lld (����: 500050000)\n", g_totalSum);
    return 0;
}
```

** ���� �� �ؼ� : **

```cpp
// ... (����) ...

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    LONGLONG localValue = 0;
    for (int i = 0; i < 10000; ++i) {
        localValue += (i + 1);
    }

    // TODO: �� �����忡�� ����� localValue�� ���� ���� g_totalSum�� ���������� ���ϼ���.
    // 64��Ʈ ������ ����ؾ� �մϴ�.
    InterlockedAdd64(&g_totalSum, localValue);
    // �Ǵ� InterlockedExchangeAdd64(&g_totalSum, localValue); �� �����մϴ�.
    // �� ���������� ��ȯ���� �ʿ� �����Ƿ� �� �� �����Դϴ�.

    printf("[Thread %lu] ���� �հ� %lld�� ���߽��ϴ�.\n", GetCurrentThreadId(), localValue);
    return 0;
}

// ... (����) ...
```

*** �ؼ�** :
*`g_totalSum`�� ���� �����尡 ���ÿ� �����Ͽ� �����ϴ� ���� �ڿ��Դϴ�. `g_totalSum += localValue; ` �� ���� �Ϲ� ������ ���������� �ʾ� ������ ����(Race Condition)�� ����Ű�� ���� �հ谡 Ʋ���� �˴ϴ�.
* `InterlockedAdd64(& g_totalSum, localValue)`�� `g_totalSum`�� `localValue`�� ���ϴ� ������ ���������� ó���Ͽ� ������ �ս� ���� ��Ȯ�� �հ踦 �����մϴ�. `LONGLONG` Ÿ���̹Ƿ� 64��Ʈ ������ `InterlockedAdd64`�� ����ؾ� �մϴ�.

---- -