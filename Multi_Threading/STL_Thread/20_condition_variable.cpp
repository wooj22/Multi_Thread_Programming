/*
  [ std::condition_variable ]

	조건 변수(conditoin variable)는 스레드간의 통신을 위한 동기화 도구로,
	스레드간에 어떤 조건이 만족될때까지 기다리거나, 조건이 만족 되었음을 다른 스레드에게 알리는 역할을 한다.
    조건 변수 자체는 데이터나 상태를 저장하지 않고 신호의 역할만 하기 때문에 상태나 조건은 별도의 변수로 관리해야한다.
	뮤텍스(mutex)와 함께 사용된다.

	주요 메서드:
	* wait() 함수는 뮤텍스 락을 해제하고 스레드를 대기 상태로 전환시킨다. 조건이 만족되어 깨어나면 다시 락을 획득한다.
	 - wait(unique_lock<mutex>& lock): 스레드를 대기
	 - wait(unique_lock<mutex>& lock, Predicate pred): pred가 true가 될 때 까지 스레드를 대기. 대기 후 깨어났을 때도 조건을 확인한다.
	 - wait_for(): 지정된 시간 동안 대기
	 - wait_until(): 특정 시점까지 대기
     
     - notify_one(): 한 스레드에게 조건이 만족되었음을 알린다.
     - notify_all(): 모든 스레드에게 조건이 만족되었음을 알린다.

	std::condition_variable은 락을 먼저 해제해준 다음 notify_()를 호출하여 락때문에 스레드가 다시 대기하는 상황을 방지한다.
    - WinAPI의 CONDITION_VARIABLE : WakeAllConditionVariable() -> 락 해제
	- std::condition_variable : 락 해제 -> notify_all()
*/

#include <iostream>
#include <string>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>


class TaskQueue {
private:
    std::queue<std::string> tasks;
    std::mutex mtx;                     // 뮤텍스
	std::condition_variable cv;         // 조건 변수
    bool stop_requested = false;        // 조건 상태 변수

public:
    void add_task(const std::string& task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(task);

			// notify 전에 락 해제
        }

        // 하나의 대기 스레드만 깨움
        cv.notify_one(); 
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop_requested = true;

            // notify 전에 락 해제
        }

        // 모든 대기 스레드를 깨움
        cv.notify_all(); 
    }

    bool process_task(int worker_id) {
        std::unique_lock<std::mutex> lock(mtx);

        // 작업이 있거나 종료 요청이 있을 때까지 대기
        cv.wait(lock, [this] { return !tasks.empty() || stop_requested; });

        if (stop_requested && tasks.empty()) {
            return false;
        }

        std::string task = tasks.front();
        tasks.pop();
        lock.unlock();

        // 작업 처리
        std::cout << "Worker " << worker_id << " processing: "
            << task << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return true;
    }
};

int main() {
    TaskQueue queue;
    std::vector<std::thread> workers;

    // 3개의 worker threads
    for (int i = 1; i <= 3; ++i) {
        workers.emplace_back([&queue, i]() {
            while (queue.process_task(i)) {
                // 계속 작업 처리
            }
            std::cout << "Worker " << i << " stopped." << std::endl;
            });
    }

    // add tasks
    for (int i = 1; i <= 10; ++i) {
        queue.add_task("Task " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 잠시 대기 후 종료 (조건 만족)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    queue.stop();

    // worker 작업이 종료될때까지 대기
    for (auto& w : workers) {
        w.join();
    }

    return 0;
}