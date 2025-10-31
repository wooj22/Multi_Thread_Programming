
/*
  [ std::mutex ]

    뮤텍스(Mutual Exclusion)는 여러 스레드가 공유 자원에 동시에 접근하는 것을 방지하는 동기화 기법으로
    단일 프로세스에 임계구역을 지정해 하나의 스레드만 접근하도록 보호한다.
    (WinAPI의 CRITICAL_SECTION. WinAPI의 mutex가 아님에 주의)

    - mutex.lock() : 임계구역 시작, 잠금에 성공할 때까지 대기
    - mutex.unlock() : 임계구역 해제
    - mutex.try_lock() : 잠그는 시도만 해보고, 실패하면 다음으로 넘어가기(lock을 기다리지 않는)
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

// 전역 뮤텍스와 공유 데이터
std::mutex mtx;
int shared_counter = 0;

// 수동 뮤텍스 (비권장)
void unsafeIncrement(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        mtx.lock();  // 뮤텍스 잠금

        // 임계 영역 (Critical Section)
        int temp = shared_counter;
        temp++;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        shared_counter = temp;

        mtx.unlock();  // 뮤텍스 해제
    }
}

// try_lock을 사용한 비차단 잠금 시도
void tryLockExample(int id) {
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (mtx.try_lock()) {
            std::cout << "Thread " << id << " acquired lock on attempt " << attempt << std::endl;

            // 임계 영역
            shared_counter += 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            mtx.unlock();
            break;
        }
        else {
            std::cout << "Thread " << id << " failed to acquire lock on attempt " << attempt << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

int main() {
    std::cout << "=== Manual Mutex Usage Example ===" << std::endl;

    // 1. lock/unlock
    shared_counter = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) 
        threads.emplace_back(unsafeIncrement, i, 100);

    for (auto& t : threads) 
        t.join();

    std::cout << "Counter after manual locking: " << shared_counter << " (Expected: 300)" << std::endl;


    // 2. try_lock
    std::cout << "\n=== Try Lock Example ===" << std::endl;
    shared_counter = 0;
    threads.clear();

    for (int i = 0; i < 3; ++i) 
        threads.emplace_back(tryLockExample, i);
    

    for (auto& t : threads) 
        t.join();
    

    std::cout << "Final counter: " << shared_counter << std::endl;

    return 0;
}