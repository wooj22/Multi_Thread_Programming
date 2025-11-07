/*
	[문제 9: 백그라운드 리소스 로딩 (Condition Variable)]
	
	문제: 게임 플레이 중 메인 스레드(게임 루프)가 멈추지 않도록, 용량이 큰 텍스처나 맵 데이터 같은 리소스를 별도의 스레드에서 로딩해야 합니다.
	ResourceManager 클래스를 작성하세요.
	
	1. std::string m_resourceData 멤버와 로딩 완료 여부를 알릴 bool m_isLoaded 플래그, 그리고 이를 보호할 std::mutex와 std::condition_variable을 가집니다.
	2. void startLoading(): "Loading..." 문자열을 m_resourceData에 할당하는 작업(실제로는 시간이 오래 걸리는 파일 I/O)을 시뮬레이션하는 새 스레드를 시작합니다. 
	   이 스레드는 작업을 완료하면 m_isLoaded를 true로 설정하고, 대기 중인 스레드에 notify_one()을 보냅니다. (작업 시뮬레이션은 std::this_thread::sleep_for(std::chrono::seconds(2))를 사용합니다.)
	3. std::string getResource(): 메인 스레드가 호출할 함수입니다. 
	   m_isLoaded가 true가 될 때까지 std::condition_variable의 wait()를 사용하여 대기해야 합니다. 
	   리소스가 준비되면 m_resourceData를 반환합니다.
	   main 함수에서 startLoading()을 호출한 직후, getResource()를 호출하여 메인 스레드가 리소스 로딩이 완료될 때까지 올바르게 대기하는지 확인하세요.
	
	힌트: std::condition_variable::wait()는 std::unique_lock과 함께 사용해야 합니다. 
	wait는 람다 조건식이 true를 반환할 때까지 (또는 notify를 받을 때까지) 뮤텍스를 해제하고 대기 상태에 들어갑니다.
*/