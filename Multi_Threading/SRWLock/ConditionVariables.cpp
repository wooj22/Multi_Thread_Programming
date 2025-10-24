/*
	[ 조건 변수(Condition Variables)]

    공유 상태가 특정 조건을 만족할 때까지 스레드를 잠시 재움(sleep)으로써 바쁜 대기(spin) 없이 효율적으로 동기화하는 원시 동기화 객체
    조건 변수 자체는 락이 아니며, 반드시 락(CRITICAL_SECTION 또는 SRWLOCK)과 함께 사용해야한다.
    
    // 초기화
    VOID InitializeConditionVariable(PCONDITION_VARIABLE cv);
    
    // 대기
    BOOL SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD dwMilliseconds);
    BOOL SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD dwMilliseconds, ULONG Flags);
    
    // 신호
    VOID WakeConditionVariable(PCONDITION_VARIABLE cv);    // 1개 스레드 깨움
    VOID WakeAllConditionVariable(PCONDITION_VARIABLE cv); // 모든 대기 스레드 깨움
    
    조건 변수는 허위(wakeup without reason)또는 타임아웃으로 깨어날 수 있기 때문에 조건은 무조건 while 루프로 검사한다.
    사용 종료시 별도 정리가 필요 없으나 조건 변수가 놓인 메모리를 해제하거나 객체를 파괴하기 전에 대기중인 스레드가 없도록 만들어야한다.
    '종료 플래그 설정 -> WakeAllConditionVariable -> thread.Join() -> 락 정리'

    - 생성 파괴 비용이 매우 낮음
    - 공정성 보장 x. 깨어난 스레드의 순서는 비결정적
    - WaitForMultipleObjects 불가. 조건 변수 전용 대기 API 사용

    - 패턴
    1) 락을 잡는다.
    2) 조건을 검사한다.
    3) 조건이 거짓이면 조건 변수에서 잠든다(이때 락을 원자적으로 풀고 기다린다).
    4) 깨어나면 락을 다시 잡은 상태로 깨어난다.
    5) 조건이 참이 되면 작업을 진행한다.

    lock();
    while (조건이 아직 만족되지 않으면)
    {
        wait(); // 조건이 만족될 때까지 락을 원자적으로 풀고 대기
    }

    unlock();
*/


// SRWLOCK과 함께 쓰는 예제
// 공유 모드로 기다릴 때는 CONDITION_VARIABLE_LOCKMODE_SHARED 플래그를 사용한다.
#include <windows.h>

SRWLOCK lock = SRWLOCK_INIT;                        // 락
CONDITION_VARIABLE cv = CONDITION_VARIABLE_INIT;    // 조건변수

volatile LONG readyCount = 0;
const LONG target = 10;

void wait_until_ready_shared(void) {
    AcquireSRWLockShared(&lock);    // 읽기 락 획득

    // 조건은 무조건 while 루프로 검사
    while (readyCount < target) 
    {
        // cv 조건변수가 CONDITION_VARIABLE_LOCKMODE_SHARED를 만족할때까지 Sleep
        // lock은 원자적 해제, 깨어날때 다시 받음
        // CONDITION_VARIABLE_LOCKMODE_SHARED : 현재 이 스레드는 공유 락 모드(읽기 모드)로 대기한다
        SleepConditionVariableSRW(&cv, &lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
    }

    ReleaseSRWLockShared(&lock);    // 읽기 락 해제
}

void make_progress(void) {
    AcquireSRWLockExclusive(&lock); // 쓰기 락 획득

    readyCount++;                   // 상태 변경
    WakeAllConditionVariable(&cv);  // 모든 대기 스레드 깨움

    ReleaseSRWLockExclusive(&lock); // 쓰기 락 해제
}