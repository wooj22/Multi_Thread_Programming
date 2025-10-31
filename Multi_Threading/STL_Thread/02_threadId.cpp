
/*
  [스레드 식별자]
    std::thread::id
    std::thread::id get_id() noexcept

    std::thread::id는 비교 연산과 스트림 출력, std::hash가 제공되어 맵/셋의 키로 사용할 수 있다. 
    같은 스레드에 대해 항상 같은 값을 반환하며 서로 다른 스레드는 서로 다른 값을 갖는다. 
    기본생성된 std::thread::id{}는 '어떤 스레드에도 속하지 않음'을 뜻하며 std::thread{}`의 get_id()와 동일하다.
*/


#include <thread>
#include <iostream>

int main() {
    std::cout << "main id = " << std::this_thread::get_id() << '\n';

    std::thread t([] {std::cout << "worker id = " << std::this_thread::get_id() << '\n';});
    t.join();
}
