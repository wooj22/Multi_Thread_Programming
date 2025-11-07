/*
	[문제 5: 읽기/쓰기 락 (std::shared_mutex)]
	문제: std::shared_mutex를 사용하여 간단한 Configuration 클래스를 구현하세요. 
	이 클래스는 std::map<std::string, int>을 내부에 가집니다.

	int getSetting(const std::string& key): 읽기 함수.
	std::shared_lock을 사용하여 여러 스레드가 동시에 설정을 읽을 수 있도록 하세요.
	void setSetting(const std::string& key, int value): 쓰기 함수.
	std::unique_lock<std::shared_mutex>을 사용하여 오직 하나의 스레드만 설정을 변경할 수 있도록 하세요 (이때 모든 읽기 스레드도 대기해야 함).
	힌트: std::shared_mutex는 읽기/쓰기 락입니다. std::shared_lock(읽기 락)은 여러 개가 동시에 존재할 수 있습니다. 
	std::unique_lock(쓰기 락)은 배타적이어서, 다른 모든 shared_lock과 unique_lock을 차단합니다.
*/

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <shared_mutex>
#include <chrono>

class Configuration
{
private:
	std::map<std::string, int> map;
	std::shared_mutex sm;

public:
	int getSetting(const std::string& key)
	{
		std::shared_lock<std::shared_mutex> lock(sm);
		auto it = map.find(key);

		if (it != map.end())
		{
			int result = it->second;
			lock.unlock();
			printf("읽기 : %s, %d\n", key.c_str(), result);
			return result;
		}
	}

	void setSetting(const std::string& key, int value)
	{
		std::unique_lock<std::shared_mutex> lock(sm);
		map[key] = value;

		printf("쓰기 : %s, %d\n", key.c_str(), value);
	}
};

int main()
{
	Configuration configuration;
	
	std::thread t1([&configuration]()
		{
			for (int i = 0; i < 10; i++)
			{
				configuration.setSetting(std::to_string(i), i);
			}
		});

	std::thread t2([&configuration]()
		{
			for (int i = 0; i < 10; i++)
			{
				configuration.getSetting(std::to_string(i));
			}
		});

	std::thread t3([&configuration]()
		{
			for (int i = 0; i < 10; i++)
			{
				configuration.getSetting(std::to_string(i));
			}
		});


	t1.join();
	t2.join();
	t3.join();

	return 0;
}
