## ���� 2: �� �ѹ��� ����Ǵ� �ʱ�ȭ �����ϱ�

* *�䱸���� : **
���� �����尡 `InitializeResource()` �Լ��� ȣ��������, ���� ���ҽ� �ʱ�ȭ �۾�(`printf`�� ��ü)�� �� �� ���� ����Ǿ�� �Ѵ�. `Interlocked` �Լ��� ����Ͽ� ù ��°�� �� �Լ��� ȣ���� �����常 �ʱ�ȭ �ڵ带 �����ϵ��� ������.

** �ǽ� �ڵ� : **

```cpp
#include <windows.h>
#include <iostream>
#include <vector>

#define NUM_THREADS 5

LONG g_initialized = 0; // 0: �ʱ�ȭ �ȵ�, 1: �ʱ�ȭ �Ϸ�

void InitializeResource() {
    // TODO: g_initialized ���� 1�� �ٲٵ�,
    // ���� �ٲٱ� ���� ���� 0�̾��� �����常 �Ʒ� "���ҽ� �ʱ�ȭ" �ڵ带 �����ϵ��� ���弼��.
    // ��Ʈ: ���� ��ȯ(Exchange)�ϰ� ���� ���� ��ȯ�ϴ� �Լ��� ����ϼ���.


    // if (���� ���� 0�̾��ٸ�?) {
    //     printf("[Thread %lu] ���ҽ� �ʱ�ȭ ����!\n", GetCurrentThreadId());
    // } else {
    //     printf("[Thread %lu] �̹� �ٸ� �����尡 �ʱ�ȭ�� �Ϸ��߽��ϴ�.\n", GetCurrentThreadId());
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

** ���� �� �ؼ� : **

```cpp
// ... (����) ...

void InitializeResource() {
    // TODO: g_initialized ���� 1�� �ٲٵ�,
    // ���� �ٲٱ� ���� ���� 0�̾��� �����常 �Ʒ� "���ҽ� �ʱ�ȭ" �ڵ带 �����ϵ��� ���弼��.
    if (InterlockedExchange(&g_initialized, 1) == 0) {
        printf("[Thread %lu] ���ҽ� �ʱ�ȭ ����!\n", GetCurrentThreadId());
    }
    else {
        printf("[Thread %lu] �̹� �ٸ� �����尡 �ʱ�ȭ�� �Ϸ��߽��ϴ�.\n", GetCurrentThreadId());
    }
}

// ... (����) ...
```

*** �ؼ�** :
*`InterlockedExchange(& g_initialized, 1)`�� `g_initialized`�� ���� ���������� `1`�� �����ϰ�, ** �����ϱ� ���� ���� ��** �� ��ȯ�մϴ�.
* ���� ���� ������ ������� `g_initialized`�� `0`�̾����Ƿ� `0`�� ��ȯ�޾� `if`�� ���� �ʱ�ȭ �ڵ带 �����մϴ�.
* �� �ڿ� �����ϴ� ��������� `g_initialized`�� �̹� `1`�� �ٲ� �����̹Ƿ� `1`�� ��ȯ�޾� `else` ���� �����ϰ� �˴ϴ�.

---- -