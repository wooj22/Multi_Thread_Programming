
/*
SRW Lock 구현 예제: Cache 데이터 동기화 하기
아래 코드는 Windows의 SRWLOCK을 사용하여 스레드 안전한 캐시를 구현한다.

InitializeSRWLock: SRWLOCK 변수를 초기화한다.
AcquireSRWLockShared / ReleaseSRWLockShared: 읽기 락을 획득하고 해제한다. 이 락은 여러 스레드가 동시에 획득할 수 있다.
AcquireSRWLockExclusive / ReleaseSRWLockExclusive: 쓰기 락을 획득하고 해제한다. 이 락은 독점적이다.
SRWLOCK은 커널 모드로 전환되지 않는 매우 가볍고 빠른 동기화 객체로, 경쟁이 적은 상황에서 최고의 성능을 보인다.
*/

#include <windows.h>
#include <process.h> 
#include <iostream>
#include <vector>
#include <random>

// Windows의 Slim Reader/Writer(SRW) Lock을 사용하는 스레드 안전 캐시 클래스
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

    // 읽기
    // 여러 스레드가 동시에 접근 가능 (Shared Lock)
    int Read(size_t index) {
        // AcquireSRWLockShared() : 읽기 락 획득
        AcquireSRWLockShared(&srwLock);

        int result = -1;
        if (index < data.size()) {
            result = data[index];
            // _beginthreadex가 반환하는 스레드 ID x,단순 시스템 스레드 ID o
            std::cout << "[Reader " << GetCurrentThreadId()
                << "] Read value " << result
                << " at index " << index
                << " (version: " << version << ")" << std::endl;
        }
        Sleep(100);

        // ReleaseSRWLockShared() : 읽기 락 해제
        ReleaseSRWLockShared(&srwLock);
        return result;
    }

    // 쓰기
    // 하나의 스레드만 독점적으로 접근 (Exclusive Lock)
    void Write(int value) {
        // AcquireSRWLockExclusive() : 쓰기 락 획득
        AcquireSRWLockExclusive(&srwLock);

        data.push_back(value);
        version++;
        std::cout << "[Writer " << GetCurrentThreadId()
            << "] Wrote value " << value
            << " (new version: " << version << ")" << std::endl;
        Sleep(200);

        // ReleaseSRWLockExclusive() : 쓰기 락 해제
        ReleaseSRWLockExclusive(&srwLock);
    }

    // 읽기
    // 캐시의 현재 크기를 스레드 안전하게 반환
    size_t Size() {
        AcquireSRWLockShared(&srwLock);
        size_t size = data.size();
        ReleaseSRWLockShared(&srwLock);
        return size;
    }
};

// Writer 스레드 함수
unsigned int __stdcall WriterThreadFunc(void* pContext) {
    ThreadSafeCache* cache = static_cast<ThreadSafeCache*>(pContext);
    int threadId = GetCurrentThreadId(); // 예시를 위한 ID

    for (int j = 0; j < 3; j++) {
        cache->Write(threadId * 10 + j);
        Sleep(300);
    }
    _endthreadex(0);
    return 0;
}

// Reader 스레드 함수
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

    // Writer 스레드 생성
    for (int i = 0; i < 2; i++) {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &WriterThreadFunc, &cache, 0, NULL);
        if (hThread) {
            hThreads.push_back(hThread);
        }
    }

    // Reader 스레드 생성
    for (int i = 0; i < 4; i++) {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ReaderThreadFunc, &cache, 0, NULL);
        if (hThread) {
            hThreads.push_back(hThread);
        }
    }

    // 모든 스레드가 종료될 때까지 대기
    WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);

    // 스레드 핸들 정리
    for (HANDLE hThread : hThreads) {
        CloseHandle(hThread);
    }

    std::cout << "\nAll threads finished." << std::endl;

    return 0;
}