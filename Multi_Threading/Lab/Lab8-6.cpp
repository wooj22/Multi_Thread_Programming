/*
	[문제 6: 스레드 생명주기와 RAII (ThreadGuard)]
	문제: 문서의 ThreadGuard 클래스를 참고하여, 스레드 객체의 join()을 보장하는 RAII 클래스를 작성하세요. 
	이 클래스는 생성자에서 std::thread 객체의 참조를 받고, 소멸자에서 해당 스레드가 joinable()한지 확인한 후 join()을 호출해야 합니다. 
	main 함수 내의 try 블록에서 스레드를 생성하고 ThreadGuard로 감싼 뒤, 고의로 예외를 발생시키세요. 
	catch 블록 이후에 "프로그램 종료" 메시지를 출력하게 하여, 
	예외가 발생했음에도 불구하고 ThreadGuard의 소멸자가 호출되어 스레드가 안전하게 join되었음을 증명하세요.

	힌트: RAII(Resource Acquisition Is Initialization) 패턴을 사용하면 객체의 생명주기를 통해 자원(여기서는 스레드)을 관리할 수 있습니다. 
	예외가 발생하여 스택이 풀릴 때(stack unwinding) 객체의 소멸자가 호출되는 것을 이용합니다.
*/