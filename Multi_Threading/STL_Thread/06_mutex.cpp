
/*
  [ std::mutex ]

    ���ؽ�(Mutual Exclusion)�� ���� �����尡 ���� �ڿ��� ���ÿ� �����ϴ� ���� �����ϴ� ����ȭ �������
    ���� ���μ����� �Ӱ豸���� ������ �ϳ��� �����常 �����ϵ��� ��ȣ�Ѵ�.
    (WinAPI�� CRITICAL_SECTION. WinAPI�� mutex�� �ƴԿ� ����)

    - mutex.lock() : �Ӱ豸�� ����, ��ݿ� ������ ������ ���
    - mutex.unlock() : �Ӱ豸�� ����
    - mutex.try_lock() : ��״� �õ��� �غ���, �����ϸ� �������� �Ѿ��(lock�� ��ٸ��� �ʴ�)
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

// ���� ���ؽ��� ���� ������
std::mutex mtx;
int shared_counter = 0;

// ���� ���ؽ� (�����)
void unsafeIncrement(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        mtx.lock();  // ���ؽ� ���

        // �Ӱ� ���� (Critical Section)
        int temp = shared_counter;
        temp++;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        shared_counter = temp;

        mtx.unlock();  // ���ؽ� ����
    }
}

// try_lock�� ����� ������ ��� �õ�
void tryLockExample(int id) {
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (mtx.try_lock()) {
            std::cout << "Thread " << id << " acquired lock on attempt " << attempt << std::endl;

            // �Ӱ� ����
            shared_counter += 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            mtx.unlock();
            break;
        }
        else {
            std::cout << "Thread " << id << " failed to acquire lock on attempt " << attempt << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

int main() {
    std::cout << "=== Manual Mutex Usage Example ===" << std::endl;

    // 1. lock/unlock
    shared_counter = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) 
        threads.emplace_back(unsafeIncrement, i, 100);

    for (auto& t : threads) 
        t.join();

    std::cout << "Counter after manual locking: " << shared_counter << " (Expected: 300)" << std::endl;


    // 2. try_lock
    std::cout << "\n=== Try Lock Example ===" << std::endl;
    shared_counter = 0;
    threads.clear();

    for (int i = 0; i < 3; ++i) 
        threads.emplace_back(tryLockExample, i);
    

    for (auto& t : threads) 
        t.join();
    

    std::cout << "Final counter: " << shared_counter << std::endl;

    return 0;
}