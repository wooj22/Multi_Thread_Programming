## 문제 3: CAS를 이용한 Lock - Free 데이터 업데이트

* *요구사항 : **
전역 변수 `g_value`가 있다.스레드는 `g_value`가 특정 값(`expectedValue`)일 때만 새로운 값(`newValue`)으로 업데이트해야 한다.이러한 '비교 후 교체' 연산을 `InterlockedCompareExchange` (CAS)를 사용하여 구현한다.

    * *실습 코드 : **

    ```cpp
#include <windows.h>
#include <iostream>

    LONG g_value = 100;

// g_value가 expectedValue와 같을 때만 newValue로 업데이트하는 함수
void UpdateValue(LONG newValue, LONG expectedValue) {
    LONG originalValue;

    // TODO: g_value의 현재 값이 expectedValue와 같으면 newValue로 교체하세요.
    // InterlockedCompareExchange 함수를 사용하고, 반환값(교체 전의 값)을 originalValue에 저장하세요.
    // originalValue = ...

    if (originalValue == expectedValue) {
        printf("[성공] 값이 %ld에서 %ld로 변경되었습니다.\n", originalValue, g_value);
    }
    else {
        printf("[실패] 값 변경 시도 실패. (기대값: %ld, 현재값: %ld)\n", expectedValue, originalValue);
    }
}

int main() {
    printf("초기값: %ld\n", g_value);

    // 성공 케이스: g_value가 100일 때 200으로 변경 시도
    UpdateValue(200, 100);

    // 실패 케이스: g_value가 100일 때 300으로 변경 시도 (기대값이 틀림)
    UpdateValue(300, 100); // 이 시점에서 g_value는 이미 200이므로 실패해야 함

    printf("최종값: %ld\n", g_value);
    return 0;
}
```

** 정답 및 해설 : **

```cpp
// ... (생략) ...

void UpdateValue(LONG newValue, LONG expectedValue) {
    LONG originalValue;

    // TODO: g_value의 현재 값이 expectedValue와 같으면 newValue로 교체하세요.
    originalValue = InterlockedCompareExchange(&g_value, newValue, expectedValue);

    if (originalValue == expectedValue) {
        printf("[성공] 값이 %ld에서 %ld로 변경되었습니다.\n", originalValue, g_value);
    }
    else {
        printf("[실패] 값 변경 시도 실패. (기대값: %ld, 현재값: %ld)\n", expectedValue, originalValue);
    }
}

// ... (생략) ...
```

*** 해설** :
*`InterlockedCompareExchange(& g_value, newValue, expectedValue)`는 내부적으로 다음과 같이 동작합니다.
1.  `g_value`의 현재 값을 읽는다.
2.  이 값이 `expectedValue`(세 번째 인자)와 같은지 비교한다.
3.  같으면 `g_value`를 `newValue`(두 번째 인자)로 업데이트한다.
4. * *비교 결과와 상관없이 연산 전의 `g_value` 값 * *을 반환한다.
* 따라서 반환된 `originalValue`와 내가 예상했던 `expectedValue`를 비교하면 CAS 연산의 성공 여부를 정확히 알 수 있습니다.

---- -