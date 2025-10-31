
/*
  [������ �Ű����� ���� ���]
    �����忡 �Ű������� ������ ���� ��(value), ����(reference), ������(pointer) ����� ����� �� �ִ�. 
    �� ����� ������� ������, Ư�� ������ �����͸� ����� ���� ���ǰ� �ʿ��ϴ�. 

    - std::thread�� �⺻������ ��� ���ڸ� ������ �����Ѵ�
    - ������ �����Ϸ��� std::ref() �Ǵ� std::cref()�� ����ؾ� �Ѵ�
    - �����͸� ������ ���� �����Ͱ� ����Ű�� ��ü�� �����ֱ⸦ �����ؾ� �Ѵ�
    - ���� ������ �ּҸ� �����忡 ������ ���� �����尡 ����Ǳ� ���� ������ �Ҹ���� �ʵ��� �����ؾ� �Ѵ�
*/


#include <iostream>
#include <thread>
#include <string>
#include <functional>


// ������ �޴� �Լ�
void byValue(int x, std::string str) {
    x += 10;
    str += " modified";
    std::cout << "By value - x: " << x << ", str: " << str << std::endl;
}

// ������ �޴� �Լ�
void byReference(int& x, std::string& str) {
    x += 20;
    str += " changed";
    std::cout << "By reference - x: " << x << ", str: " << str << std::endl;
}

// �����ͷ� �޴� �Լ�
void byPointer(int* x, std::string* str) {
    if (x && str) {
        *x += 30;
        *str += " updated";
        std::cout << "By pointer - x: " << *x << ", str: " << *str << std::endl;
    }
}

// const ������ �޴� �Լ� (�б� ����)
void byConstReference(const int& x, const std::string& str) {
    std::cout << "By const reference - x: " << x << ", str: " << str << std::endl;
}

int main() {
    int num = 100;
    std::string text = "Hello";

    std::cout << "Initial values - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 1. ������ ���� (���� ���� ����)
    std::thread t1(byValue, num, text);
    t1.join();
    std::cout << "After byValue - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 2. ������ ���� (std::ref �ʿ�)
    std::thread t2(byReference, std::ref(num), std::ref(text));
    t2.join();
    std::cout << "After byReference - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 3. �����ͷ� ����
    std::thread t3(byPointer, &num, &text);
    t3.join();
    std::cout << "After byPointer - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 4. const ������ ���� (std::cref ���)
    std::thread t4(byConstReference, std::cref(num), std::cref(text));
    t4.join();
    std::cout << "After byConstReference - num: " << num << ", text: " << text << std::endl;

    return 0;
}