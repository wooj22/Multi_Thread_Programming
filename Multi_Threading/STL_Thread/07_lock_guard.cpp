/*
  [ std::lock_guard ]

    �������� lock() unlock()�� ȣ���ϸ�, 
    ���ܰ� �߻��ϰų� �Լ��� ���⿡ ��ȯ�Ǿ��� ��� unlock()�� ȣ����� �ʾ� ������� �߻��� �� �ִ�.
    lock�� RAII�������� ������ lock_guard�� ����Ͽ� �����ϰ� lock/unlock�� �̷�������� �Ѵ�.
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

    // �Ա� - lock_guard ���
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx_);  // �ڵ� ���

        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive");
        }

        double old_balance = balance_;
        balance_ += amount;

        std::cout << account_name_ << " - Deposit: $" << amount
            << " (Balance: $" << old_balance << " -> $" << balance_ << ")"
            << std::endl;
    }  // lock_guard �Ҹ��ڿ��� �ڵ����� unlock

    // ��� - lock_guard ���
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

    // �ܾ� ��ȸ - const �޼��忡���� lock_guard ���
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return balance_;
    }

    // ���� ��ü - �� ���� ���ؽ��� �����ϰ� ����
    static void transfer(BankAccount& from, BankAccount& to, double amount) {
        // std::lock�� ����Ͽ� ����� ����
        std::lock(from.mtx_, to.mtx_);

        // adopt_lock: �̹� ��� ���ؽ��� ����
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

// ���� Ʈ������� �����ϴ� ������ �Լ�
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

    // ���� �����忡�� ���ÿ� ���� �۾� ����
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(performTransactions, std::ref(account1), i);
    }

    // ���� ��ü ������
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