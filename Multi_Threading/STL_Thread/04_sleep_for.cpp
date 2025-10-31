
/*
  [������ �Ͻ� ����, ������]
    std::this_thread::sleep_for() -> ���� �������κ��� Ư�� �ð�(duration) ���� �����带 ������.
    std::this_thread::sleep_until() -> Ư�� �ð�(time point)���� �����带 ������.

    ���� ª�� ������ `sleep_for(1us)`ó�� ��û�ص� ���� ���ð��� �ü�� Ÿ�̸� �ػ󵵿� ���� �ξ� ����� �� �ִ�. 
    ���ػ� Ÿ�̹��� �ʿ��ϸ� �ٻ� ���� �����, �Ǵ� �÷����� ���ػ� Ÿ�̸Ӹ� �����ؾ� �Ѵ�.
*/

#include <iostream>
#include <thread>
#include <chrono>

void sleepForExample() {
    std::cout << "[sleep_for] ����\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[sleep_for] 2�� �� ���\n";
}

void sleepUntilExample() {
    using clock = std::chrono::steady_clock;
    auto wakeup = clock::now() + std::chrono::seconds(2);

    std::cout << "[sleep_until] ����\n";
    std::this_thread::sleep_until(wakeup);
    std::cout << "[sleep_until] ��Ȯ�� 2�� �� �ð� ����\n";
}

int main() {
    sleepForExample();
    sleepUntilExample();
}