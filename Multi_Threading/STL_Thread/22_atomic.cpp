/*
  [ std::atomic ]

   atomic은 원자적 연산을 제공하는 템플릿 클래스로, 
   winAPI에서 volatile 변수로 Interlocked 연산하는 것과 같은 개념이다.
   원자적 연산은 중단될 수 없는 단일 연산으로, 다른 스레드가 중간 상태를 관찰할 수 없어 경쟁 조건 문제를 방지한다.

   atomic 변수는 원자적 연산을 위해 복사가 불가능하며, 읽기 쓰기는 load(), store()를 통해 이루어진다.
   같은 원리로 atomic는 vector에 pushback을 하면 안되고, 무조건 emplaceback을 사용하여야 한다.

   - load() : 읽기
   - store(n) : 쓰기
   - exchange(new) : 새 값으로 변경하고, 이전 값 반환
   - compare_exchange_strong(compare, new) : Compare-And-Swap (CAS) 함수
                                                     value == compare이라면 new로 변경하고 true 반환
                                                     value != compare이라면 compare을 value로 변경하고 false 반환
   
   정수형만 가능한 연산
   - fetch_add(int n) : n만큼 증가 후, 이전 값 반환
   - fetch_sub(int n) : n만큼 감소 후, 이전 값 반환
   - ++, --, +=, -=
   - 비트연산                                               
*/

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> g_counter(0); // 전역 원자적 카운터

void Worker(int id)
{
    for (int i = 0; i < 1000; ++i)
    {
        g_counter.fetch_add(1); // 원자적으로 +1
        // g_counter++;         // 동일한 효과
    }
}

int main()
{
    std::vector<std::thread> threads;

    // 10개의 스레드 생성
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(Worker, i);
    }

    // 스레드 조인
    for (auto& t : threads)
        t.join();

    std::cout << "최종 카운터 값: " << g_counter.load() << std::endl; // 안전하게 읽기

    // compare_exchange_strong 예시
    int expected = 10000;
    if (g_counter.compare_exchange_strong(expected, 0))
    {
        std::cout << "CAS 성공: 값이 10000이어서 0으로 초기화됨" << std::endl;
    }
    else
    {
        std::cout << "CAS 실패: 현재 값은 " << g_counter.load()
            << " (expected는 " << expected << "으로 변경됨)" << std::endl;
    }

    return 0;
}