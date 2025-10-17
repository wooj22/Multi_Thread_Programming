## ���� 1: ������ ������ �۾� ī���� �����
* *�䱸���� : **
���� ���� �����尡 ���ÿ� `Work()` �Լ��� ȣ���Ѵ�. `Work()` �Լ��� �ð��� �ɸ��� �۾��� �䳻 ���� ���� `Sleep()`�� ����Ѵ�.���� ���� `g_workCount`�� ����Ͽ� ���� �� ���� �����尡 `Work()` �Լ��� ���� ������ �����ؾ� �Ѵ�. `Interlocked` �Լ��� ����Ͽ� `g_workCount`�� �����忡 �����ϰ� ������Ű�� ���ҽ�Ų��.

** �ǽ� �ڵ� : **

```cpp
#include <windows.h>
#include <iostream>
#include <vector>

#define NUM_THREADS 10
#define MAX_CONCURRENT_WORKS 3 // ���ÿ� �ִ� 3�������� �۾� ���

LONG g_workCount = 0; // ���� �۾� ���� ������ ��

void Work() {
    // TODO 1: ���� �۾� ���� ������ ���� ���������� 1 ������Ű��,
    // �� ���(���� ���� ��)�� newCount ������ �����ϼ���.
    LONG newCount = 0; // �� �κ��� �����ϼ���.

    if (newCount > MAX_CONCURRENT_WORKS) {
        printf("[Thread %lu] �۾����� ���� �������� ���߽��ϴ�. (���� %ld�� ���� ��)\n", GetCurrentThreadId(), newCount);
    }
    else {
        printf("[Thread %lu] �۾� ����! (���� %ld�� ���� ��)\n", GetCurrentThreadId(), newCount);
        Sleep(100); // �۾��� �䳻 ���ϴ�.
        printf("[Thread %lu] �۾� �Ϸ�!\n", GetCurrentThreadId());
    }

    // TODO 2: �۾��� �������Ƿ�, ���� �۾� ���� ������ ���� ���������� 1 ���ҽ�Ű����.

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

    printf("��� ������ ���� �Ϸ�. ���� �۾� ī��Ʈ: %ld\n", g_workCount); // ���������� 0�� �Ǿ�� ��
    return 0;
}
```

** ���� �� �ؼ� : **

```cpp
// ... (����) ...

void Work() {
    // TODO 1: ���� �۾� ���� ������ ���� ���������� 1 ������Ű��,
    // �� ���(���� ���� ��)�� newCount ������ �����ϼ���.
    LONG newCount = InterlockedIncrement(&g_workCount);

    if (newCount > MAX_CONCURRENT_WORKS) {
        printf("[Thread %lu] �۾����� ���� �������� ���߽��ϴ�. (���� %ld�� ���� ��)\n", GetCurrentThreadId(), newCount);
    }
    else {
        printf("[Thread %lu] �۾� ����! (���� %ld�� ���� ��)\n", GetCurrentThreadId(), newCount);
        Sleep(100); // �۾��� �䳻 ���ϴ�.
        printf("[Thread %lu] �۾� �Ϸ�!\n", GetCurrentThreadId());
    }

    // TODO 2: �۾��� �������Ƿ�, ���� �۾� ���� ������ ���� ���������� 1 ���ҽ�Ű����.
    InterlockedDecrement(&g_workCount);
}

// ... (����) ...
```

*** �ؼ�** :
*`InterlockedIncrement(& g_workCount)`: ���� �����尡 ���ÿ� �����ص� `g_workCount`�� �����ϰ� 1 ������ŵ�ϴ�. * *���� ���� �� * *�� ��ȯ�ϹǷ�, ���� �� ���� �۾��� ���� ������ ��� �� �� �ֽ��ϴ�.
* `InterlockedDecrement(& g_workCount)`: �۾��� ���� �� ī��Ʈ�� �����ϰ� 1 ���ҽ��� �ٸ� �����尡 ������ �� �ֵ��� �մϴ�.�� �� �Լ��� ��������ν� `g_workCount`�� �׻� ��Ȯ�� ���� �����ϰ� �˴ϴ�.

---- -