/*
  [ std::jthread ]

  jthread(joinable thread)는 C++20에서 도입된 RAII 스레드 클래스로, std::thread의 여러 문제점을 해결한다.

  ┌─────────────────────────────────────────────────┐
  │         std::thread vs std::jthread             │
  ├─────────────────────────────────────────────────┤
  │                                                 │
  │  std::thread                                    │
  │  ✗ 소멸자에서 join() 호출 안함                   │
  │  ✗ 중단 메커니즘 없음                            │
  │  ✗ 수동으로 join/detach 필요                     │
  │                                                 │
  │  std::jthread                                   │
  │  ✓ 소멸자에서 자동 join()                        │
  │  ✓ 협력적 중단 지원                              │
  │  ✓ RAII 원칙 준수                               │
  │  ✓ stop_token 자동 전달                         │
  │                                                 │
  └─────────────────────────────────────────────────┘

  * 주요 멤버 함수 *
  - get_id() : 스레드 id 반환
  - joinable() : join 가능 여부 확인
  - join() : 스레드 종료 대기
  - detach() : 스레드 분리
  - get_stop_source() : stop_source 객체 반환
  - get_stop_token() : stop_token 객체 반환
  - request_stop() : 종료 요청


  [std::stop_source]
  stop_source는 외부에서 스레드에게 종료 신호를 전달하는 객체이다.
  스레드에서 get_stop_source()로 얻은 stop_source()에 request_stop()을 하면, 
  스레드의 stop_token이 stop_requested()==true 상태가 된다.
  (굳이 stop_source를 사용하지 않고 외부에서 스레드에 바로 request_stop()을 해도 된다)

  [std::stop_token]
  스레드 내부에서 종료 요청이 들어왔는지 확인하는 객체
  스레드 내부에서는 stop_token을 보고 종료해야할 시점인지를 확인할 수 있다.

  - stop_requested() : 종료 요청이 들어왔는지 확인
  - stop_possible() : 종료 요청이 가능한 상태인지 확인
  - swap() : 다른 stop_token과 교환

  [std::stop_callback]
  std::stop_token에 종료 요청이 들어왔을 경우, 등록된 콜백 함수를 실행하는 장치
*/

#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

void worker(std::stop_token st)	// jthread는 첫번째 매개변수로 stop_token이 자동으로 들어온다
{
	std::stop_callback cb(st, []() {
		std::cout << "Stop requested! Callback triggered.\n";
		});

	while (!st.stop_requested())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << "working...\n";
	}
}

int main()
{
	std::jthread t(worker);
	std::this_thread::sleep_for(chrono::seconds(1));
	t.get_stop_source().request_stop();
}