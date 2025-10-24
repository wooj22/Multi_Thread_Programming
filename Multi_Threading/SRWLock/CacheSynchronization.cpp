
/*
SRW Lock ���� ����: Cache ������ ����ȭ �ϱ�
�Ʒ� �ڵ�� Windows�� SRWLOCK�� ����Ͽ� ������ ������ ĳ�ø� �����Ѵ�.

InitializeSRWLock: SRWLOCK ������ �ʱ�ȭ�Ѵ�.
AcquireSRWLockShared / ReleaseSRWLockShared: �б� ���� ȹ���ϰ� �����Ѵ�. �� ���� ���� �����尡 ���ÿ� ȹ���� �� �ִ�.
AcquireSRWLockExclusive / ReleaseSRWLockExclusive: ���� ���� ȹ���ϰ� �����Ѵ�. �� ���� �������̴�.
SRWLOCK�� Ŀ�� ���� ��ȯ���� �ʴ� �ſ� ������ ���� ����ȭ ��ü��, ������ ���� ��Ȳ���� �ְ��� ������ ���δ�.
*/

#include <windows.h>
#include <process.h> 
#include <iostream>
#include <vector>
#include <random>

// Windows�� Slim Reader/Writer(SRW) Lock�� ����ϴ� ������ ���� ĳ�� Ŭ����
class ThreadSafeCache
{
private:
    SRWLOCK srwLock;
    std::vector<int> data;
    int version;

public:
    ThreadSafeCache() : version(0) 
    {
        InitializeSRWLock(&srwLock);
        data.reserve(100);
    }

    // �б�
    // ���� �����尡 ���ÿ� ���� ���� (Shared Lock)
    int Read(size_t index) {
        // AcquireSRWLockShared() : �б� �� ȹ��
        AcquireSRWLockShared(&srwLock);

        int result = -1;
        if (index < data.size()) {
            result = data[index];
            // _beginthreadex�� ��ȯ�ϴ� ������ ID x,�ܼ� �ý��� ������ ID o
            std::cout << "[Reader " << GetCurrentThreadId()
                << "] Read value " << result
                << " at index " << index
                << " (version: " << version << ")" << std::endl;
        }
        Sleep(100);

        // ReleaseSRWLockShared() : �б� �� ����
        ReleaseSRWLockShared(&srwLock);
        return result;
    }

    // ����
    // �ϳ��� �����常 ���������� ���� (Exclusive Lock)
    void Write(int value) {
        // AcquireSRWLockExclusive() : ���� �� ȹ��
        AcquireSRWLockExclusive(&srwLock);

        data.push_back(value);
        version++;
        std::cout << "[Writer " << GetCurrentThreadId()
            << "] Wrote value " << value
            << " (new version: " << version << ")" << std::endl;
        Sleep(200);

        // ReleaseSRWLockExclusive() : ���� �� ����
        ReleaseSRWLockExclusive(&srwLock);
    }

    // �б�
    // ĳ���� ���� ũ�⸦ ������ �����ϰ� ��ȯ
    size_t Size() {
        AcquireSRWLockShared(&srwLock);
        size_t size = data.size();
        ReleaseSRWLockShared(&srwLock);
        return size;
    }
};

// Writer ������ �Լ�
unsigned int __stdcall WriterThreadFunc(void* pContext) {
    ThreadSafeCache* cache = static_cast<ThreadSafeCache*>(pContext);
    int threadId = GetCurrentThreadId(); // ���ø� ���� ID

    for (int j = 0; j < 3; j++) {
        cache->Write(threadId * 10 + j);
        Sleep(300);
    }
    _endthreadex(0);
    return 0;
}

// Reader ������ �Լ�
unsigned int __stdcall ReaderThreadFunc(void* pContext) {
    ThreadSafeCache* cache = static_cast<ThreadSafeCache*>(pContext);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int j = 0; j < 5; j++) {
        size_t size = cache->Size();
        if (size > 0) {
            std::uniform_int_distribution<> dis(0, size - 1);
            cache->Read(dis(gen));
        }
        Sleep(150);
    }
    _endthreadex(0);
    return 0;
}

int main() {
    std::cout << "=== Reader-Writer Lock Test with SRWLOCK and _beginthreadex ===\n" << std::endl;

    ThreadSafeCache cache;
    std::vector<HANDLE> hThreads;

    // Writer ������ ����
    for (int i = 0; i < 2; i++) {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &WriterThreadFunc, &cache, 0, NULL);
        if (hThread) {
            hThreads.push_back(hThread);
        }
    }

    // Reader ������ ����
    for (int i = 0; i < 4; i++) {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ReaderThreadFunc, &cache, 0, NULL);
        if (hThread) {
            hThreads.push_back(hThread);
        }
    }

    // ��� �����尡 ����� ������ ���
    WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);

    // ������ �ڵ� ����
    for (HANDLE hThread : hThreads) {
        CloseHandle(hThread);
    }

    std::cout << "\nAll threads finished." << std::endl;

    return 0;
}