/*
	[문제 7: std::unique_lock의 유연한 사용 (수동 lock/unlock)]
	문제: std::unique_lock을 사용하여 뮤텍스를 관리하는 함수를 작성하세요. 
	이 함수는 다음과 같이 동작해야 합니다.

	std::unique_lock을 생성하되, 처음에는 락을 잠그지 않습니다 (std::defer_lock).
	"락 없이 작업 수행" 메시지를 출력합니다.
	수동으로 lock()을 호출하여 락을 획득합니다.
	"임계 영역 진입" 메시지를 출력합니다.
	수동으로 unlock()을 호출하여 락을 해제합니다.
	"락 외부에서 다시 작업 수행" 메시지를 출력합니다.
	힌트: std::unique_lock은 std::lock_guard보다 유연합니다. 
	std::unique_lock<std::mutex> lock(mtx, std::defer_lock);처럼 생성하면 락을 바로 잠그지 않습니다. 
	이후 lock.lock()과 lock.unlock()을 수동으로 호출하여 락의 소유권을 제어할 수 있습니다.
*/