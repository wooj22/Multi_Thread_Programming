
/*
  [스레드 매개변수 전달 방식]
    스레드에 매개변수를 전달할 때는 값(value), 참조(reference), 포인터(pointer) 방식을 사용할 수 있다. 
    각 방식은 장단점이 있으며, 특히 참조와 포인터를 사용할 때는 주의가 필요하다. 

    - std::thread는 기본적으로 모든 인자를 값으로 복사한다
    - 참조를 전달하려면 std::ref() 또는 std::cref()를 사용해야 한다
    - 포인터를 전달할 때는 포인터가 가리키는 객체의 생명주기를 주의해야 한다
    - 지역 변수의 주소를 스레드에 전달할 때는 스레드가 종료되기 전에 변수가 소멸되지 않도록 주의해야 한다
*/


#include <iostream>
#include <thread>
#include <string>
#include <functional>


// 값으로 받는 함수
void byValue(int x, std::string str) {
    x += 10;
    str += " modified";
    std::cout << "By value - x: " << x << ", str: " << str << std::endl;
}

// 참조로 받는 함수
void byReference(int& x, std::string& str) {
    x += 20;
    str += " changed";
    std::cout << "By reference - x: " << x << ", str: " << str << std::endl;
}

// 포인터로 받는 함수
void byPointer(int* x, std::string* str) {
    if (x && str) {
        *x += 30;
        *str += " updated";
        std::cout << "By pointer - x: " << *x << ", str: " << *str << std::endl;
    }
}

// const 참조로 받는 함수 (읽기 전용)
void byConstReference(const int& x, const std::string& str) {
    std::cout << "By const reference - x: " << x << ", str: " << str << std::endl;
}

int main() {
    int num = 100;
    std::string text = "Hello";

    std::cout << "Initial values - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 1. 값으로 전달 (원본 변경 없음)
    std::thread t1(byValue, num, text);
    t1.join();
    std::cout << "After byValue - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 2. 참조로 전달 (std::ref 필요)
    std::thread t2(byReference, std::ref(num), std::ref(text));
    t2.join();
    std::cout << "After byReference - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 3. 포인터로 전달
    std::thread t3(byPointer, &num, &text);
    t3.join();
    std::cout << "After byPointer - num: " << num << ", text: " << text << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 4. const 참조로 전달 (std::cref 사용)
    std::thread t4(byConstReference, std::cref(num), std::cref(text));
    t4.join();
    std::cout << "After byConstReference - num: " << num << ", text: " << text << std::endl;

    return 0;
}