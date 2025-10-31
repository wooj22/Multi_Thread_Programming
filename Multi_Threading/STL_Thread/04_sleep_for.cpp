
/*
  [스레드 일시 중지, 잠재우기]
    std::this_thread::sleep_for() -> 현재 시점으로부터 특정 시간(duration) 동안 스레드를 잠재운다.
    std::this_thread::sleep_until() -> 특정 시각(time point)까지 스레드를 잠재운다.

    아주 짧은 지연을 `sleep_for(1us)`처럼 요청해도 실제 대기시간은 운영체제 타이머 해상도에 의해 훨씬 길어질 수 있다. 
    고해상도 타이밍이 필요하면 바쁜 대기와 백오프, 또는 플랫폼의 고해상도 타이머를 검토해야 한다.
*/

#include <iostream>
#include <thread>
#include <chrono>

void sleepForExample() {
    std::cout << "[sleep_for] 시작\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[sleep_for] 2초 후 깨어남\n";
}

void sleepUntilExample() {
    using clock = std::chrono::steady_clock;
    auto wakeup = clock::now() + std::chrono::seconds(2);

    std::cout << "[sleep_until] 시작\n";
    std::this_thread::sleep_until(wakeup);
    std::cout << "[sleep_until] 정확히 2초 후 시각 도달\n";
}

int main() {
    sleepForExample();
    sleepUntilExample();
}