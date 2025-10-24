# C++의 `volatile` (바럴타일 / 발러타일)
아래 내용은 C++17~23 기준에서의 설명한 것이다.     
핵심은 “컴파일러 최적화에 의해 사라지면 안 되는 메모리 접근을 **강제로 남겨두는 타입 한정자**” 라는 점이다.

## 1) `volatile`의 정의와 의미
* `volatile`은 **타입 한정자(cv-qualifier)** 로서, “이 객체에 대한 읽기/쓰기 접근 자체가 외부 세계에 의미가 있으니 컴파일러가 제거하거나 합치거나 캐시하지 말라”는 신호를 준다.
* 결과적으로, **각각의 읽기(load)** 와 **쓰기(store)** 가 코드에 보이는 대로 실제로 수행된다고 가정하도록 만든다.
* 반대로 말하면, `volatile`은 **동기화, 원자성, 락, 메모리 장벽** 을 제공하지 않는다. CPU나 컴파일러의 재배치가 완전히 금지되는 것도 아니다.  보장 범위는 “접근 그 자체를 생략/병합하지 말라”에 가깝다.  
  

## 2) 무엇을 보장하는가 / 보장하지 않는가

### 보장하는 것
* **접근의 존재성**: `volatile` 객체에 대한 각 읽기/쓰기 연산은 최적화로 제거되지 않는다.
* **접근의 순서 보존(해당 표현식 내)**: 같은 `volatile` 객체에 대한 접근은 표현식 순서대로 발생해야 한다는 제약이 생긴다(컴파일러 수준에서 불필요한 재배치/병합 억제).
  
### 보장하지 않는 것
* **스레드 간 동기화**를 절대 보장하지 않는다. 데이터 경쟁(data race)을 막지 못한다.
* **원자성(atomicity)** 을 제공하지 않는다(32비트보다 큰 타입이 두 번에 나뉘어 쓰일 수도 있음).
* **메모리 장벽(fence)** 이 아니다. 다른 일반 메모리 접근의 재배치를 막지 못한다.
* **캐시 무효화/하드웨어 버스 동기화** 같은 하드웨어 레벨의 효과를 약속하지 않는다.
  
스레드 간 통신/동기화가 목적이라면 **`std::atomic`**과 적절한 **메모리 순서(memory_order)**를 사용해야 한다.  
    

## 3) 언제 사용해야 하는가 (정당한 사용처)
1. **메모리 맵트 I/O(MMIO) 레지스터 접근**
   하드웨어 레지스터를 메모리에 매핑해 두고 폴링하거나 값을 쓰는 경우, 접근 자체가 효과이므로 최적화로 지워지면 안 된다.
2. **임베디드 환경의 폴링 루프**
   외부 장치 상태를 계속 읽어야 하는 루프에서, 매 반복마다 실제 읽기가 일어나야 한다.
3. **신호 처리기와 상호작용(전통적 C 세계)**
   `volatile sig_atomic_t` 같은 패턴이 C에서 흔하다. 다만 현대 C++에서는 가능하면 `std::atomic`을 권장한다(신호 처리기에서의 사용 가능성은 구현/플랫폼 제약을 따르지만, 최소한 멀티스레드 동기화 목적으로 `volatile`을 쓰는 건 잘못이다).
  

## 4) 사용하면 안 되는 경우
* **멀티스레드 동기화/가시성 보장** 목적으로는 절대 사용하지 않는다. `std::atomic`을 사용해야 한다.
* **성능 최적화**를 위해 무턱대고 붙이지 않는다. 오히려 최적화가 막혀 성능이 떨어진다.
* **일반 변수에 ‘변경될 수 있음’을 표현**하려는 의도로 쓰지 않는다. 그런 의미는 타입 시스템/최적화와 맞지 않는다.
  

## 5) 예제 코드

### (A) 메모리 맵트 I/O 레지스터 폴링

```cpp
#include <cstdint>

constexpr std::uintptr_t UART_STATUS_ADDR = 0x4000'1000;
constexpr std::uintptr_t UART_DATA_ADDR   = 0x4000'1004;

inline volatile std::uint32_t* const UART_STATUS =
    reinterpret_cast<volatile std::uint32_t*>(UART_STATUS_ADDR);
inline volatile std::uint32_t* const UART_DATA =
    reinterpret_cast<volatile std::uint32_t*>(UART_DATA_ADDR);

// 비트 0: RX 데이터 준비됨
constexpr std::uint32_t RX_READY = 1u << 0;

char uart_read_char() {
    // 매 반복마다 실제 하드웨어 레지스터를 읽어야 하므로 volatile이 필요함
    while ( (*UART_STATUS & RX_READY) == 0 ) {
        // busy-wait
    }
    return static_cast<char>(*UART_DATA & 0xFFu);
}
```

설명: `volatile`이 없으면 컴파일러가 `*UART_STATUS` 읽기를 레지스터에 캐시하거나 루프 밖으로 hoist하는 등의 최적화를 시도할 수 있는데, 이는 외부 장치 상태 변화를 관측하지 못하게 만든다. `volatile`은 각 접근을 실제로 수행하도록 보장한다.
  

### (B) 잘못된 예: 멀티스레드 통신에 `volatile` 사용

```cpp
#include <thread>
#include <chrono>

volatile bool ready = false; // ❌ 동기화 용도로 잘못 사용

int main() {
    std::thread t1([] {
        // 다른 쓰기들과의 순서/가시성이 보장되지 않음
        ready = true;
    });

    std::thread t2([] {
        while (!ready) { /* spin */ } // 데이터 레이스 가능
        // 여기 도달했어도 t1의 다른 쓰기를 반드시 관측한다는 보장이 없음
    });

    t1.join();
    t2.join();
}
```

위 코드는 **데이터 레이스**이며 동작이 미정이다. `volatile`은 스레드 간 동기화를 제공하지 않는다.

#### 올바른 대안: `std::atomic` 사용

```cpp
#include <atomic>
#include <thread>

std::atomic<bool> ready{false}; // ✅

int main() {
    std::thread t1([] {
        // release: 이 이전의 쓰기가 이후 acquire에 의해 관측되도록 보장
        ready.store(true, std::memory_order_release);
    });

    std::thread t2([] {
        // acquire: ready의 true 관측 시, t1의 이전 쓰기들이 보이도록 보장
        while (!ready.load(std::memory_order_acquire)) { /* spin */ }
        // 안전하게 진행 가능
    });

    t1.join();
    t2.join();
}
```

### (C) 최적화 제거 방지 예시

```cpp
// 외부에서 바뀔 수 있다고 가정하는 상태 레지스터
extern volatile unsigned status_reg;

void wait_flag() {
    // 아래 루프는 매번 status_reg를 실제로 읽는다
    while ((status_reg & 0x1u) == 0) {
        // do nothing
    }
}
```

`volatile`이 없으면 컴파일러가 `status_reg`를 한 번만 읽고 루프를 상수 판단해 제거하는 등의 최적화를 할 수 있다. `volatile`은 이를 막는다.
  

## 6) 추가 심화 포인트
* **타입 전파**: `volatile`은 포인터/참조/클래스 멤버 함수(cv-qualifier)에도 전파될 수 있다. 예를 들어 `void f() volatile`은 `volatile` 객체에서만 호출 가능한 멤버 함수를 뜻한다.
* **캐스팅 주의**: 실제로 `volatile` 객체를 `const_cast` 등으로 벗겨내고 접근하면 **정의되지 않은 동작(UB)**이 될 수 있다.
* **`std::atomic`과의 관계**: `std::atomic<T>`는 동기화/원자성/메모리 순서를 제공한다. `atomic`에 굳이 `volatile`을 덧붙이는 것은 거의 의미가 없고 혼란만 준다.

## 7) 요약
* `volatile`은 **컴파일러 최적화를 억제하여 “접근 그 자체가 효과”인 경우를 지키기 위한 도구**다.
* **하드웨어 레지스터, 폴링 루프, 특정 저수준 상호작용**에만 사용해야 한다.
* **스레드 동기화에는 절대 사용하지 말고** `std::atomic`을 사용해야 한다.

</br>  
</br>  
     

# Win32 API에서의 `volatile`
C++ 언어 자체의 `volatile` 키워드를 그대로 사용하는 경우도 많지만, **Microsoft의 구현 세부사항(특히 Windows 커널 및 시스템 헤더)** 에서는 의미가 약간 확장되거나 특수화되어 있다.
아래에서 차이점과 사용 시점을 정리하겠다.


## 1. C++ `volatile`과 Win32 `volatile`의 차이점 요약

| 구분     | C++ 표준 `volatile`                | Win32 / Microsoft 확장 `volatile`                            |
| ------ | -------------------------------- | ---------------------------------------------------------- |
| 정의 위치  | 언어 차원의 키워드 (`volatile`)          | 매크로나 typedef로 재정의된 `volatile` (예: `typedef volatile LONG`) |
| 목적     | **컴파일러 최적화 억제** (메모리 접근 생략 금지)   | **커널, 드라이버, 원자적 메모리 접근 및 동기화 보조**를 위해 사용                   |
| 동기화 효과 | 없음 (atomic, memory ordering 미보장) | 일부 Win32 함수(`InterlockedXxx`)와 결합될 때만 **원자적 접근 보장**        |
| 주 사용처  | 하드웨어 레지스터, 폴링 루프                 | 커널 드라이버, 공유 메모리, Interlocked 연산, volatile 공유 변수            |

즉, Win32에서의 `volatile`은 언어적 의미는 같지만, **Windows 커널의 메모리 모델 하에서 “원자적 접근이 필요한 변수”를 표현하는 컨벤션**으로 사용되는 경우가 많다.

   
## 2. Win32 API에서의 실제 사용 예

### (1) Interlocked API와 함께 사용
Windows에서 `volatile`은 **Interlocked 함수군** (`InterlockedIncrement`, `InterlockedExchange`, 등)과 함께 사용되는 패턴이 흔하다.

```cpp
#include <Windows.h>

volatile LONG g_flag = 0;  // 여러 스레드가 접근하는 공유 변수

DWORD WINAPI WorkerThread(LPVOID) {
    while (InterlockedCompareExchange(&g_flag, 1, 0) != 0) {
        // 다른 스레드가 플래그를 잡고 있으면 대기
        Sleep(1);
    }

    // 임계 구역 진입
    // ...
    InterlockedExchange(&g_flag, 0); // 해제
    return 0;
}
```

* `volatile LONG` 선언은 컴파일러 최적화를 막아 실제 메모리 접근을 보장한다.
* 그러나 **진짜 동기화는 `InterlockedCompareExchange`가 수행**한다.
* 따라서 `volatile`만으로는 안전하지 않지만, Win32 컨벤션상 “이 변수는 다른 스레드와 공유되며, Interlocked API로 보호된다”는 표시 역할을 한다.


### (2) 커널 모드 드라이버의 메모리 매핑 I/O
커널 드라이버 코드에서는 하드웨어 레지스터 접근 시 다음과 같이 `volatile`이 반드시 필요하다.

```cpp
typedef struct _DEVICE_REGISTER {
    volatile ULONG Status;
    volatile ULONG Control;
    volatile ULONG Data;
} DEVICE_REGISTER, *PDEVICE_REGISTER;

PDEVICE_REGISTER g_DeviceRegs = (PDEVICE_REGISTER)MmMapIoSpace(...);

void WriteToDevice() {
    g_DeviceRegs->Control = 0x1; // 실제 메모리(레지스터)에 즉시 반영
}
```

이 경우는 C++ `volatile`의 원래 의미 그대로, “접근을 생략하지 말라”는 목적이다.


### (3) Windows 헤더의 정의
Win32 헤더를 보면 다음과 같이 정의되어 있다.

```cpp
typedef volatile LONG LONG_PTR;
typedef volatile LONG *PLONG;
typedef volatile ULONG ULONG_PTR;
```

즉, Win32의 많은 타입들이 `volatile`을 내부적으로 포함하고 있다.
이건 멀티스레드/인터럽트 환경에서 최적화로 인한 비일관성(access caching)을 막기 위한 Microsoft의 방어적 조치다.


## 3. 잘못된 사용 예
`volatile`만으로 멀티스레드에서의 안전성을 보장한다고 생각하면 안 된다.

```cpp
volatile LONG counter = 0;

void IncrementWrong() {
    counter++;  // ❌ 비원자적 연산
}
```

이 경우 두 스레드가 동시에 `counter++`를 하면 **race condition**이 발생한다.
`volatile`은 단지 접근을 생략하지 않게 할 뿐, 원자성(atomicity)을 보장하지 않는다.


## 4. 올바른 사용 예

```cpp
volatile LONG counter = 0;

void IncrementCorrect() {
    InterlockedIncrement(&counter);  // ✅ 원자적 증가
}
```

이 경우에는 Win32의 Interlocked 함수가 **원자적 연산 + 메모리 순서 보장**을 제공하므로 안전하다.

  
## 5. 요약 정리

| 항목     | C++ `volatile` | Win32 API `volatile`                             |
| ------ | -------------- | ------------------------------------------------ |
| 근본적 의미 | 접근 최적화 억제      | 동일하지만, 커널/스레드 컨텍스트에서 관용적으로 사용                    |
| 동기화 기능 | 없음             | `Interlocked*`와 결합 시 원자성 보장                      |
| 사용 목적  | 하드웨어 접근, 폴링    | 커널 객체, Interlocked 변수, 드라이버, 공유 메모리              |
| 대안     | `std::atomic`  | 커널: Interlocked / 유저: SRWLock, CriticalSection 등 |

  

## 6. 결론
* Win32의 `volatile`은 언어적으로 C++의 `volatile`과 같지만,
  **Windows의 메모리 모델 및 API 컨벤션 안에서 “공유 메모리 접근용”으로 확장된 의미로 사용**된다.
* 실제 동기화나 원자성은 `Interlocked` 함수 또는 명시적 락이 담당한다.
* 즉, **Win32의 `volatile`은 “최적화를 막는 보조 표시” + “공유 변수임을 나타내는 코드 컨벤션”**이다.

