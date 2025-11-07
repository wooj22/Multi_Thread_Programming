/*
	[문제 10: 스레드 안전한 게임 엔티티 (Read-Write Lock)]

	문제: 게임 월드에 Player 객체가 하나 있습니다. 
	이 Player 객체의 데이터(위치, 체력)는 여러 스레드에서 동시에 접근됩니다.

	- 물리 스레드 (Writer): 0.1초마다 플레이어의 position을 갱신합니다.
	- AI 스레드 (Writer): 0.5초마다 플레이어의 health를 갱신합니다 (예: 몬스터에게 피격).
	- 렌더링 스레드 (Reader): 0.016초마다(약 60FPS) 플레이어의 position과 health를 읽어서 화면에 출력합니다.

	요구사항: std::shared_mutex를 사용하여 Player 클래스를 스레드로부터 안전하게 만드세요.
	1. updatePosition (물리)과 takeDamage (AI) 함수는 쓰기 락 (std::unique_lock) 을 사용하여 데이터를 배타적으로 수정해야 합니다.
	2. render (렌더링) 함수는 읽기 락 (std::shared_lock) 을 사용하여 여러 렌더링 스레드(가 있다고 가정)가 동시에, 그리고 쓰기 작업이 없을 때, 데이터를 빠르게 읽어갈 수 있도록 해야 합니다.
	
	힌트: 읽기 작업(Render)은 자주 발생하고, 쓰기 작업(Physics, AI)은 드물게 발생합니다. 
	이럴 때 std::shared_mutex가 가장 효율적입니다. std::shared_lock은 여러 리더가 동시에 접근하는 것을 허용하고, std::unique_lock은 모든 리더와 라이터를 차단합니다. 
	const 멤버 함수인 render에서 뮤텍스를 잠그려면 뮤텍스를 mutable로 선언해야 합니다.
*/