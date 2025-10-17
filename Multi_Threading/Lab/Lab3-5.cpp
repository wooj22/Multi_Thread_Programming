## 문제 5: 비트 플래그를 이용한 스레드 상태 관리

* *요구사항 : **
하나의 정수 변수 `g_flags`를 사용하여 여러 상태를 비트마스크로 관리한다. `FLAG_A` (0x01), `FLAG_B` (0x02), `FLAG_C` (0x04) 세 가지 플래그가 있다. `Interlocked` 비트 연산 함수를 사용하여 스레드에 안전하게 플래그를 추가하고 제거하는 코드를 작성한다.

** 실습 코드 : **

```cpp
#include <windows.h>
#include <iostream>

LONG g_flags = 0; // 초기 상태는 0

#define FLAG_A 0x01 // 0001
#define FLAG_B 0x02 // 0010
#define FLAG_C 0x04 // 0100

// 플래그를 원자적으로 추가하는 함수
void AddFlag(LONG flag) {
    // TODO 1: g_flags에 flag를 원자적으로 추가(OR 연산)하세요.
}

// 플래그를 원자적으로 제거하는 함수
void RemoveFlag(LONG flag) {
    // TODO 2: g_flags에서 flag를 원자적으로 제거(AND 연산)하세요.
    // 힌트: 제거하려는 비트만 0으로 만들고 나머지는 1로 유지하는 마스크(~flag)를 사용해야 합니다.
}

void PrintFlags() {
    printf("현재 Flags: 0x%02lX\n", g_flags);
}

int main() {
    PrintFlags(); // 초기 상태: 0x00

    // 스레드 1이 FLAG_A와 FLAG_C를 추가한다고 가정
    AddFlag(FLAG_A | FLAG_C);
    PrintFlags(); // 예상: 0x05

    // 스레드 2가 FLAG_B를 추가한다고 가정
    AddFlag(FLAG_B);
    PrintFlags(); // 예상: 0x07

    // 스레드 3이 FLAG_A를 제거한다고 가정
    RemoveFlag(FLAG_A);
    PrintFlags(); // 예상: 0x06

    return 0;
}
```

** 정답 및 해설 : **

```cpp
// ... (생략) ...

// 플래그를 원자적으로 추가하는 함수
void AddFlag(LONG flag) {
    // TODO 1: g_flags에 flag를 원자적으로 추가(OR 연산)하세요.
    InterlockedOr(&g_flags, flag);
}

// 플래그를 원자적으로 제거하는 함수
void RemoveFlag(LONG flag) {
    // TODO 2: g_flags에서 flag를 원자적으로 제거(AND 연산)하세요.
    // 힌트: 제거하려는 비트만 0으로 만들고 나머지는 1로 유지하는 마스크(~flag)를 사용해야 합니다.
    InterlockedAnd(&g_flags, ~flag);
}

// ... (생략) ...
```

*** 해설** :
***플래그 추가** : `InterlockedOr(& g_flags, flag)`는 `g_flags |= flag` 연산을 원자적으로 수행합니다.특정 비트를 `1`로 설정(추가)하는 데 사용됩니다.
* **플래그 제거 * *: `InterlockedAnd( & g_flags, ~flag)`는 `g_flags &= ~flag` 연산을 원자적으로 수행합니다. `~flag`는 제거하려는 비트만 `0`이고 나머지는 `1`인 비트마스크를 만듭니다.이 마스크와 `AND` 연산을 하면 원하는 비트만 `0`으로 설정(제거)할 수 있습니다.