
/*
  [스케줄링 양보]
    std::this_thread::yield
    void yield() noexcept

    스케줄러에 “다른 준비된 스레드를 먼저 실행해도 된다”는 힌트를 준다. 
    현재 스레드의 실행을 즉시 양보하고 다시 스케줄될 수 있다.
    실제로 컨텍스트 스위치가 일어날지, 어느 정도 대기할지는 구현과 시스템 상태에 따라 다르다. 
    준비된 다른 스레드가 없으면 아무 변화가 없을 수 있다. 
    과도한 사용은 성능을 해칠 수 있으므로 스핀 대기에서의 백오프 단계 등에 제한적으로 쓰는 것이 좋다. 
    동기화에는 `condition_variable`, 세마포어, `atomic::wait/notify` 같은 프리미티브가 더 적절하다.
*/


#include <atomic>
#include <thread>

std::atomic<bool> ready = false;        // automic변수는 원자적 연산 보장

void wait_ready() {
    int spins = 0;

    // load() : 원자 변수의 값을 읽는 함수
    // memory_order_acquire : load시 읽기 장벽을 세워 이후의 모든 메모리 접근이 이 load보다 뒤로 밀림
    while (!ready.load(std::memory_order_acquire)) {
        if (spins < 100) ++spins;           // 아주 짧게 바쁜 대기 후
        else std::this_thread::yield();     // 다른 스레드에게 CPU 자원 양보
    }
}