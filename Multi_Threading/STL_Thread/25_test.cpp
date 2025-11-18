/*
  [ std::atomic_flag.test() ] - C++20

  test()는 플래그를 수정하지 않고 상태만 확인할 수 있는 메서드로, C++20 부터 지원한다.
*/

#include <atomic>
#include <iostream>
#include <thread>

std::atomic_flag ready = ATOMIC_FLAG_INIT;

void producer() {
    std::cout << "데이터 준비 중...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ready.test_and_set(std::memory_order_release);      // flag = true
    std::cout << "데이터 준비 완료!\n";
}

void consumer() {
    // C++20: test() - 상태만 읽기 (수정 안 함)
    // ready가 false인동안 spin
    while (!ready.test(std::memory_order_acquire)) {    
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "데이터 소비 시작\n";
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();

    return 0;
}