/*
*	[문제 1: 스레드 생성과 매개변수 전달 (람다)]
	문제: 메인 스레드에서 std::vector<int>를 하나 생성합니다. 
	람다 표현식을 사용하여 새 스레드를 생성하고, 이 벡터의 참조를 람다에 캡처하여 전달하세요. 
	새 스레드는 1초 대기 후, 캡처한 벡터에 정수 3개를 (예: 10, 20, 30) push_back 해야 합니다. 
	메인 스레드는 스레드가 종료될 때까지 join() 하고, 
	벡터의 최종 내용을 출력하여 3개의 정수가 올바르게 추가되었는지 확인하세요.
	
	힌트: 람다 표현식은 지역 변수를 값([=]) 또는 참조([&])로 캡처할 수 있습니다. 
	std::thread에 람다를 직접 전달하여 스레드를 생성할 수 있습니다.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

int main()
{
	std::vector<int> vec;

	auto func = [&vec]() {
		std::cout <<  "1초 대기합니다." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "데이터를 추가합니다." << std::endl;
		vec.push_back(10); vec.push_back(20); vec.push_back(30);
		std::cout << "데이터 추가 완료!" << std::endl;
		};

	std::thread t1(func);
	t1.join();

	std::cout << "결과를 확인합니다." << std::endl;
	for (auto& v1 : vec)
	{
		std::cout << v1 << " ";
	}
}