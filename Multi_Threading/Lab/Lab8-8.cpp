/*
	[문제 8: "Check-then-Act" 문제 해결 (std::upgrade_lock)]

	문제: std::shared_mutex로 보호되는 int g_value = 0;가 있습니다. 
	스레드가 g_value가 0일 때만("Check") 값을 1로 변경("Act")하는 작업을 수행해야 합니다. 
	하지만 "Check"를 위해 shared_lock을 사용하고 "Act"를 위해 unique_lock을 사용하면, "Check"와 "Act" 사이에 다른 스레드가 끼어들 수 있습니다 ("Check-then-Act" 오류). 
	std::upgrade_lock을 사용하여 이 문제를 해결하세요. 스레드는 std::upgrade_lock을 사용하여 g_value를 확인하고, 
	조건이 맞으면 락을 std::upgrade_to_unique_lock으로 "승격"시켜 값을 안전하게 변경해야 합니다.

	힌트: std::upgrade_lock은 다른 shared_lock은 허용하지만, 다른 unique_lock이나 upgrade_lock은 차단합니다. 
	락을 해제하지 않고 std::upgrade_to_unique_lock으로 "승격"시켜 unique_lock으로 변환할 수 있으며, 이 과정은 "틈"이 없이(atomically) 안전하게 수행됩니다.
*/