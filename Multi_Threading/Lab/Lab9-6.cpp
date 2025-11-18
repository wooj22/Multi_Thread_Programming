/*
	문제 6: std::atomic_flag로 스핀락 구현
	문제: 문서에 나온 Spinlock 클래스는 std::atomic<bool>과 exchange를 사용했습니다. 
	C++에서 스핀락 구현을 위해 특별히 제공되는 std::atomic_flag를 사용하여 Spinlock 클래스를 다시 구현해 보세요. 
	atomic_flag는 test_and_set()과 clear() 멤버 함수를 가집니다.

	힌트
	 std::atomic_flag는 ATOMIC_FLAG_INIT로 초기화해야 합니다 (기본값은 'clear' 상태).
	 lock(): test_and_set()이 true (이미 set 되어 있었음)를 반환하는 동안 while 루프를 돕니다.
	 unlock(): clear()를 호출하여 플래그를 0(clear) 상태로 되돌립니다.
*/

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

class Spinlock {
private:
    // ATOMIC_FLAG_INIT는 atomic_flag를 
    // 'clear' (false, 0) 상태로 초기화합니다.
    std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
    void lock() {
        // test_and_set():
        // 1. 플래그를 'set' (true, 1) 상태로 바꿉니다.
        // 2. 'set'으로 바뀌기 *이전*의 값을 반환합니다.

        // 이전 값이 'set'(true)이었다면 (다른 스레드가 락을 잡고 있음)
        // 루프를 계속 돕니다.
        while (locked.test_and_set()) {
            std::this_thread::yield(); // CPU 양보
        }
        // 이전 값이 'clear'(false)였다면 (내가 락을 획득)
        // 루프를 탈출합니다.
    }

    void unlock() {
        // 플래그를 'clear' (false, 0) 상태로 되돌립니다.
        locked.clear();
    }
};

Spinlock spinlock;
int shared_counter = 0;

void increment_with_spinlock() {
    for (int i = 0; i < 10000; ++i) {
        spinlock.lock();
        ++shared_counter;  // 보호된 영역
        spinlock.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment_with_spinlock);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "결과: " << shared_counter << "\n";  // 100000

    return 0;
}