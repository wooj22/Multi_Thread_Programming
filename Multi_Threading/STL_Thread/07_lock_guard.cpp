/*
  [ std::lock_guard ]

    수동으로 lock() unlock()을 호출하면, 
    예외가 발생하거나 함수가 조기에 반환되었을 경우 unlock()이 호출되지 않아 데드락이 발생할 수 있다.
    lock을 RAII패턴으로 구현한 lock_guard를 사용하여 안전하게 lock/unlock이 이루어지도록 한다.
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <exception>
#include <chrono>

class BankAccount {
private:
    mutable std::mutex mtx_;
    double balance_;
    std::string account_name_;

public:
    BankAccount(const std::string& name, double initial_balance)
        : account_name_(name), balance_(initial_balance) {
    }

    // 입금 - lock_guard 사용
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx_);  // 자동 잠금

        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive");
        }

        double old_balance = balance_;
        balance_ += amount;

        std::cout << account_name_ << " - Deposit: $" << amount
            << " (Balance: $" << old_balance << " -> $" << balance_ << ")"
            << std::endl;
    }  // lock_guard 소멸자에서 자동으로 unlock

    // 출금 - lock_guard 사용
    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx_);

        if (amount <= 0) {
            throw std::invalid_argument("Withdrawal amount must be positive");
        }

        if (balance_ >= amount) {
            double old_balance = balance_;
            balance_ -= amount;

            std::cout << account_name_ << " - Withdraw: $" << amount
                << " (Balance: $" << old_balance << " -> $" << balance_ << ")"
                << std::endl;
            return true;
        }

        std::cout << account_name_ << " - Withdrawal failed: Insufficient funds"
            << " (Requested: $" << amount << ", Available: $" << balance_ << ")"
            << std::endl;
        return false;
    }

    // 잔액 조회 - const 메서드에서도 lock_guard 사용
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return balance_;
    }

    // 계좌 이체 - 두 개의 뮤텍스를 안전하게 관리
    static void transfer(BankAccount& from, BankAccount& to, double amount) {
        // std::lock을 사용하여 데드락 방지
        std::lock(from.mtx_, to.mtx_);

        // adopt_lock: 이미 잠긴 뮤텍스를 관리
        std::lock_guard<std::mutex> lock1(from.mtx_, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(to.mtx_, std::adopt_lock);

        if (from.balance_ >= amount) {
            from.balance_ -= amount;
            to.balance_ += amount;

            std::cout << "Transfer: $" << amount
                << " from " << from.account_name_
                << " to " << to.account_name_ << " successful" << std::endl;
        }
        else {
            std::cout << "Transfer failed: Insufficient funds in "
                << from.account_name_ << std::endl;
        }
    }
};

// 여러 트랜잭션을 수행하는 스레드 함수
void performTransactions(BankAccount& account, int thread_id) {
    try {
        for (int i = 0; i < 5; ++i) {
            if (i % 2 == 0) {
                account.deposit(10.0 * (thread_id + 1));
            }
            else {
                account.withdraw(5.0 * (thread_id + 1));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (const std::exception& e) {
        std::cout << "Thread " << thread_id << " exception: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== Bank Account Thread Safety Demo ===" << std::endl;

    BankAccount account1("Checking", 1000.0);
    BankAccount account2("Savings", 500.0);

    std::cout << "\nInitial balances:" << std::endl;
    std::cout << "Checking: $" << account1.getBalance() << std::endl;
    std::cout << "Savings: $" << account2.getBalance() << std::endl;

    // 여러 스레드에서 동시에 계좌 작업 수행
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(performTransactions, std::ref(account1), i);
    }

    // 계좌 이체 스레드
    threads.emplace_back([&account1, &account2]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        BankAccount::transfer(account1, account2, 100.0);
        });

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nFinal balances:" << std::endl;
    std::cout << "Checking: $" << account1.getBalance() << std::endl;
    std::cout << "Savings: $" << account2.getBalance() << std::endl;

    return 0;
}