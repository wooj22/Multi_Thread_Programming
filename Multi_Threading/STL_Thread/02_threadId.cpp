
/*
  [������ �ĺ���]
    std::thread::id
    std::thread::id get_id() noexcept

    std::thread::id�� �� ����� ��Ʈ�� ���, std::hash�� �����Ǿ� ��/���� Ű�� ����� �� �ִ�. 
    ���� �����忡 ���� �׻� ���� ���� ��ȯ�ϸ� ���� �ٸ� ������� ���� �ٸ� ���� ���´�. 
    �⺻������ std::thread::id{}�� '� �����忡�� ������ ����'�� ���ϸ� std::thread{}`�� get_id()�� �����ϴ�.
*/


#include <thread>
#include <iostream>

int main() {
    std::cout << "main id = " << std::this_thread::get_id() << '\n';

    std::thread t([] {std::cout << "worker id = " << std::this_thread::get_id() << '\n';});
    t.join();
}
