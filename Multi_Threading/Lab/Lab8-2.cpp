/*
*   [문제 2: std::ref를 이용한 참조 매개변수 전달]
	문제: void modifyValue(int& value, int add)라는 함수를 작성하세요. 
	이 함수는 전달된 참조 value에 add만큼의 값을 더해야 합니다.
	main 함수에서 int myValue = 100;을 선언하고, modifyValue 함수를 실행하는 새 스레드를 생성하세요. 
	이 스레드에 myValue의 참조와 50을 인자로 전달하세요. 스레드 종료 후 myValue의 값이 150이 되었는지 확인하세요.
	
	힌트: std::thread 생성자는 기본적으로 인자를 복사합니다. 함
	수가 참조(&)를 인자로 받더라도, 스레드에 그냥 전달하면 값이 복사되어 원본이 수정되지 않습니다. 
	원본 변수의 참조를 스레드 함수에 전달하려면 std::ref()로 감싸야 합니다.
*/

#include <iostream>
#include <thread>
#include <atomic>

void modifyValue(int& value, int add)
{
	value += add;
}

int main()
{
	int myValue = 100;
	std::cout << "myValue의 변경 전 값 : " << myValue << std::endl;

	std::thread t1(modifyValue, std::ref(myValue), 50);
	t1.join();

	std::cout << "myValue의 변경 후 값 : " << myValue;
}