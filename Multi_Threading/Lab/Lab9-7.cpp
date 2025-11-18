/*
	문제 7: 최신 값 발행(Publishing)
	문제: 하나의 "발행(Publisher)" 스레드와 여러 개의 "구독(Subscriber)" 스레드가 있습니다.
	
	std::atomic<int> latest_value{0} 변수를 사용합니다.
	발행 스레드는 1초마다 latest_value를 store()를 통해 1씩 증가시킵니다 (총 5번).
	구독 스레드(3개)들은 500ms마다 latest_value를 load()하여 현재 값을 출력합니다.
	힌트: 발행 스레드와 구독 스레드에 각각 다른 sleep_for 주기를 주어 비동기적으로 값이 갱신되고 읽히는 것을 관찰하세요.
*/

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

std::atomic<int> latest_value{ 0 };
std::atomic<bool> running{ true }; // 프로그램 종료 플래그

void publisher() {
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        latest_value.store(i);
        std::cout << "[발행] 새 값: " << i << "\n";
    }
    running.store(false); // 실행 종료
}

void subscriber(int id) {
    while (running.load()) {
        int current_value = latest_value.load();
        std::cout << "[구독 " << id << "] 현재 값: "
            << current_value << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    std::vector<std::thread> threads;

    // 발행 스레드 1개
    threads.emplace_back(publisher);

    // 구독 스레드 3개
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(subscriber, i + 1);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}