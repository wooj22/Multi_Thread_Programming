/*
  [ std::atomic ] - C++20

  C++20 부터는 std::atomic에 대기/ 통지 매커니즘이 추가된다.

  - atomic.wait(old_value) : atomic의 value가 old_value와 다를때까지 대기
  - atomic.notify_one() : 대기중인 스레드를 하나 깨움
  - atomic.notify_all() : 대기중인 스레드를 모두 깨움
*/

#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> value{ 0 };

void waiter() {
    std::cout << "현재 값: " << value << ", 대기 시작...\n";

    // wait(old_value): value가 old_value와 다를 때까지 대기
    value.wait(0, std::memory_order_acquire);

    std::cout << "값이 변경됨! 새 값: " << value << "\n";
}

void notifier() {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    value.store(42, std::memory_order_release);
    std::cout << "값 변경: 42\n";

    // 대기 중인 스레드 하나를 깨움
    value.notify_one();
}

int main() {
    std::thread t1(waiter);
    std::thread t2(notifier);

    t1.join();
    t2.join();

    return 0;
}

/*

wait() vs std::mutex
std::atomic::wait()는 내부적으로 리눅스에서는 futex(fast userspace mutex)를 사용해 구현되어 있어, 커널 진입이 적고 매우 가볍게 동작한다. 
하지만 그렇다고 해서 std::mutex를 모두 atomic::wait() 기반으로 바꾸는 것이 좋은 선택은 아니다.

1. 기본 동작 방식의 차이
 std::mutex
 - 임계 구역(critical section)을 보호하기 위한 상호 배제(mutual exclusion) 기능을 제공한다.
 - 한 스레드만 락을 획득할 수 있으며, 다른 스레드는 자동으로 블록된다.
 - 내부적으로도 futex를 사용할 수 있지만, 락의 상태 관리·소유권·재진입 가능성 등을 함께 처리한다.
 
 std::atomic::wait() / notify_one()
 - 단순히 값 변화에 대해 대기(wait) 하는 기능만 제공한다.
 - “조건이 만족될 때까지 잠들고, 값이 바뀌면 깨어난다”는 형태다.
 - 락 소유권 개념이 없고, 데이터 경쟁을 방지하지 않는다.
 - 즉, atomic::wait()은 동기화 원자적 조건 대기를 위한 것이고, 상호 배제를 위한 것이 아니다.

2. 성능 차이
 - 단순 대기/알림 시나리오(예: 생산자–소비자 큐, 스핀락 대체)에서는 atomic::wait()이 mutex + condition_variable 조합보다 빠르다.
   커널 호출 수가 줄고, 사용자 공간에서 대부분 처리된다.
 - 하지만 락을 통한 자원 보호가 필요한 경우에는 std::mutex가 더 안전하고, 유지보수하기 쉽다.
 - atomic::wait()는 락이 아니라서, 메모리 일관성 확보나 예외 처리 중 락 해제 같은 안전 장치가 없다.

*/