
/*
  [������ �����ϱ�]
    WinAPI�� ������ ���� : createThread, beginThreadex
    C++�� ������ ���� : std::thread
*/

#include <iostream>
#include <thread>
#include <chrono>

// ������ �Լ�
void workerFunction(int id, int iterations) 
{
    for (int i = 0; i < iterations; ++i) 
    {
        std::cout << "Thread " << id << " is working: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        // Sleep
    }
    std::cout << "Thread " << id << " completed!" << std::endl;
}

// ������ �Լ� ��ü
class WorkerTask {
private:
    int id_;
    int iterations_;

public:
    WorkerTask(int id, int iterations) : id_(id), iterations_(iterations) {}

    // ������ �Լ�
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

    // ������ ���� std::thread(�������Լ�, �Ű�����..)
    std::thread t1(workerFunction, 1, 3);
    std::thread t2(workerFunction, 2, 4);
    std::thread t3(workerFunction, 3, 2);

    // �Լ� ��ü�� ����� ������ ����
    WorkerTask task1(1, 3);
    WorkerTask task2(2, 2);

    std::thread t4(task1);
    std::thread t5(task2);
    std::thread t6(WorkerTask(3, 4));

    // ��� �����尡 �Ϸ�� ������ ��� join()
    // WinAPI�� WaitForSingleObject
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    std::cout << "All threads completed. Main thread exiting." << std::endl;
    return 0;
}