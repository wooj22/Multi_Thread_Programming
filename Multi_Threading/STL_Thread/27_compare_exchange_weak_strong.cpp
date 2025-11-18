/*
  [compare_exchange_strong]
   - 예상 값과 실제 값이 같으면 반드시 교환 성공
   - Spurious failure (가짜 실패) 없음
   -> 단일 시도에 적합, 웬만해선 strong을 사용한다.

  [compare_exchange_weak]
   - 예상 값과 실제 값이 같아도 실패할 수 있음 (spurious failure)
   - 하드웨어 최적화 가능 (일부 아키텍처에서 더 빠름)
   -> 재시도 루프 조건문에서의 사용만 적합하다.
*/

#include <atomic>
#include <iostream>

std::atomic<int> value{ 100 };

void demonstrate_strong() {
    std::cout << "=== compare_exchange_strong ===\n";

    int expected = 100;
    bool success = value.compare_exchange_strong(expected, 200);

    if (success) {
        std::cout << "성공: 100 -> 200\n";
    }
    else {
        std::cout << "실패. 실제 값: " << expected << "\n";
    }

    // 단일 시도로도 신뢰 가능
}

void demonstrate_weak() {
    std::cout << "\n=== compare_exchange_weak (루프 필요) ===\n";

    value.store(100);
    int expected = 100;

    // weak는 루프에서 사용해야 함
    while (!value.compare_exchange_weak(expected, 300,
        std::memory_order_release,
        std::memory_order_relaxed)) {
        std::cout << "재시도... (expected는 자동 갱신됨: " << expected << ")\n";
        expected = 100;  // 다시 설정
    }

    std::cout << "최종 성공: " << value << "\n";
}

int main() {
    demonstrate_strong();
    demonstrate_weak();

    return 0;
}