/*
	[ ���� ����(Condition Variables)]

    ���� ���°� Ư�� ������ ������ ������ �����带 ��� ���(sleep)���ν� �ٻ� ���(spin) ���� ȿ�������� ����ȭ�ϴ� ���� ����ȭ ��ü
    ���� ���� ��ü�� ���� �ƴϸ�, �ݵ�� ��(CRITICAL_SECTION �Ǵ� SRWLOCK)�� �Բ� ����ؾ��Ѵ�.
    
    // �ʱ�ȭ
    VOID InitializeConditionVariable(PCONDITION_VARIABLE cv);
    
    // ���
    BOOL SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD dwMilliseconds);
    BOOL SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD dwMilliseconds, ULONG Flags);
    
    // ��ȣ
    VOID WakeConditionVariable(PCONDITION_VARIABLE cv);    // 1�� ������ ����
    VOID WakeAllConditionVariable(PCONDITION_VARIABLE cv); // ��� ��� ������ ����
    
    ���� ������ ����(wakeup without reason)�Ǵ� Ÿ�Ӿƿ����� ��� �� �ֱ� ������ ������ ������ while ������ �˻��Ѵ�.
    ��� ����� ���� ������ �ʿ� ������ ���� ������ ���� �޸𸮸� �����ϰų� ��ü�� �ı��ϱ� ���� ������� �����尡 ������ �������Ѵ�.
    '���� �÷��� ���� -> WakeAllConditionVariable -> thread.Join() -> �� ����'

    - ���� �ı� ����� �ſ� ����
    - ������ ���� x. ��� �������� ������ �������
    - WaitForMultipleObjects �Ұ�. ���� ���� ���� ��� API ���

    - ����
    1) ���� ��´�.
    2) ������ �˻��Ѵ�.
    3) ������ �����̸� ���� �������� ����(�̶� ���� ���������� Ǯ�� ��ٸ���).
    4) ����� ���� �ٽ� ���� ���·� �����.
    5) ������ ���� �Ǹ� �۾��� �����Ѵ�.

    lock();
    while (������ ���� �������� ������)
    {
        wait(); // ������ ������ ������ ���� ���������� Ǯ�� ���
    }

    unlock();
*/


// SRWLOCK�� �Բ� ���� ����
// ���� ���� ��ٸ� ���� CONDITION_VARIABLE_LOCKMODE_SHARED �÷��׸� ����Ѵ�.
#include <windows.h>

SRWLOCK lock = SRWLOCK_INIT;                        // ��
CONDITION_VARIABLE cv = CONDITION_VARIABLE_INIT;    // ���Ǻ���

volatile LONG readyCount = 0;
const LONG target = 10;

void wait_until_ready_shared(void) {
    AcquireSRWLockShared(&lock);    // �б� �� ȹ��

    // ������ ������ while ������ �˻�
    while (readyCount < target) 
    {
        // cv ���Ǻ����� CONDITION_VARIABLE_LOCKMODE_SHARED�� �����Ҷ����� Sleep
        // lock�� ������ ����, ����� �ٽ� ����
        // CONDITION_VARIABLE_LOCKMODE_SHARED : ���� �� ������� ���� �� ���(�б� ���)�� ����Ѵ�
        SleepConditionVariableSRW(&cv, &lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
    }

    ReleaseSRWLockShared(&lock);    // �б� �� ����
}

void make_progress(void) {
    AcquireSRWLockExclusive(&lock); // ���� �� ȹ��

    readyCount++;                   // ���� ����
    WakeAllConditionVariable(&cv);  // ��� ��� ������ ����

    ReleaseSRWLockExclusive(&lock); // ���� �� ����
}