
/*
  [스레드 생성하기]
    WinAPI의 스레드 생성 : createThread, beginThreadex
    C++의 스레드 생성 : std::thread
*/

#include <iostream>
#include <thread>
#include <chrono>

// 스레드 함수
void workerFunction(int id, int iterations) 
{
    for (int i = 0; i < iterations; ++i) 
    {
        std::cout << "Thread " << id << " is working: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        // Sleep
    }
    std::cout << "Thread " << id << " completed!" << std::endl;
}

// 스레드 함수 객체
class WorkerTask {
private:
    int id_;
    int iterations_;

public:
    WorkerTask(int id, int iterations) : id_(id), iterations_(iterations) {}

    // 스레드 함수
    void operator()() {
        for (int i = 0; i < iterations_; ++i) {
            std::cout << "Worker " << id_ << " processing: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        std::cout << "Worker " << id_ << " finished!" << std::endl;
    }
};

int main() {
    std::cout << "Main thread starting..." << std::endl;

    // 스레드 생성 std::thread(스레드함수, 매개변수..)
    std::thread t1(workerFunction, 1, 3);
    std::thread t2(workerFunction, 2, 4);
    std::thread t3(workerFunction, 3, 2);

    // 함수 객체를 사용한 스레드 생성
    WorkerTask task1(1, 3);
    WorkerTask task2(2, 2);

    std::thread t4(task1);
    std::thread t5(task2);
    std::thread t6(WorkerTask(3, 4));

    // 모든 스레드가 완료될 때까지 대기 join()
    // WinAPI의 WaitForSingleObject
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    std::cout << "All threads completed. Main thread exiting." << std::endl;
    return 0;
}