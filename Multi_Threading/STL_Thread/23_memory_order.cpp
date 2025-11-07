/*
  [ memory_order (메모리 순서) ]

   memory_order는 std::atomic 연산의 메모리 가기성 제어용 옵션으로, CPU의 명령어 재정렬을 제어할 수 있다.
   그렇다면 memory_order는 왜 필요할까?
   {
       data = 100;
       ready = true;
   }
   CPU는 성능 최적화를 위해 명령어를 마음대로 재정렬한다.
   위 코드를 보면 data가 100으로 바뀐 뒤, ready가 true로 바뀐다. 단일 스레드에서는 순서가 중요하지 않지만
   멀티스레드에서는 다른 스레드가 ready를 먼저 보고 data를 아직 안본 상황이 생길수도있다.
   즉, data를 먼저 써진다는 것을 보장할 수 없다.


   (약함)
   - memory_order_relaxed : 순서 제약 없음. 원자성만 보장 -> “결과만 중요, 순서는 상관없음”
   - memory_order_consume : 잘 안쓰임. 이 원자 변수의 값을 이용해서 다른 메모리를 참조할 때만, 그 참조가 release 이전의 쓰기를 볼 수 있음           
   - memory_order_acquire : 이후의 읽기/쓰기 명령들이 앞당겨지지 않도록 막음 -> “잠금 획득 후: 이후 작업은 이 잠금 뒤에만 실행돼야 함”
   - memory_order_release : 이전의 읽기/쓰기 명령들이 뒤로 밀리지 않도록 막음 -> “잠금 해제 전: 지금까지 한 작업이 다 끝나야 함”        
   - memory_order_acq_rel : acquire + release 둘 다 적용 -> “획득과 해제를 동시에 함”              
   - memory_order_seq_cst : 가장 강한 순서 보장 (기본값)
   (강함)
*/

// 1. memory_order_seq_cst
// 가장 강력하지만 가장 느리다. 모든 스레드가 동일한 순서로 연산을 관찰한다.
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> x{ 0 };
std::atomic<int> y{ 0 };

void thread1() {
    x.store(1, std::memory_order_seq_cst);  // 기본값
    int r1 = y.load(std::memory_order_seq_cst);
}

void thread2() {
    y.store(1, std::memory_order_seq_cst);
    int r2 = x.load(std::memory_order_seq_cst);
}
// r1 == 0 && r2 == 0 은 불가능 (순차 일관성 보장)



// 2. memory_order_relaxed
// 순서 보장 없이 원자성만 보장한다. 가장 빠르지만 신중하게 사용해야 한다.
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

std::atomic<int> counter{ 0 };

void increment_relaxed() {
    for (int i = 0; i < 100000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment_relaxed);
    }

    for (auto& t : threads) {
        t.join();
    }

    // 최종 값은 정확 (원자성 보장)
    // 하지만 중간 값의 관찰 순서는 보장되지 않음
    std::cout << "결과: " << counter << "\n";  // 1000000

    return 0;
}


// 3. memory_order_acquire / release
// 생산자, 소비자 패턴에 이상적이다.
#include <atomic>
#include <thread>
#include <iostream>
#include <cassert>

std::atomic<bool> ready{ false };
int data = 0;

void producer() {
    data = 100;  // 데이터 준비
    ready.store(true, std::memory_order_release);    // 해제: 이전 쓰기들이 가시화
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // 획득: 이후 읽기들이 보호됨
        // 대기
    }
    assert(data == 100);  // 항상 참! (획득-해제 의미론)
    std::cout << "데이터: " << data << "\n";
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();

    return 0;
}

/*
    Producer 스레드:
      data = 100;        ───┐
      release (ready)       │ 해제: 이 선 위의 모든 쓰기가
      ─────────────────────┴─ 아래로 재배치 안 됨
    
    Consumer 스레드:
      ─────────────────────┬─ 획득: 이 선 아래의 모든 읽기가
      acquire (ready)       │ 위로 재배치 안 됨
      assert(data==100)  ───┘
*/