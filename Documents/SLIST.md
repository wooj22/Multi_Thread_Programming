# SLIST (Singly Linked List)란?
[Interlocked Singly linked list 이해와 사용](https://docs.google.com/presentation/d/1TDyr82kOlH-1drSqJdk_I1caKk6FYuwZY-HeROVVxEQ/edit?usp=sharing )  
  
`SLIST`는 **원자적(atomic)으로 항목을 추가하고 제거할 수 있는 단일 연결 리스트**이다. 일반적인 연결 리스트와 달리, `SLIST`는 여러 스레드가 동시에 접근해도 데이터의 일관성이 깨지지 않도록 설계되었다. 이는 내부적으로 락(Lock)을 사용하지 않고 CPU의 원자적 연산(Interlocked-functions)을 이용하기 때문에 **락프리(Lock-Free)** 자료구조로 분류된다.  
  
주로 스레드 풀(Thread Pool)의 작업 큐, 메모리 풀(Memory Pool) 관리 등 여러 스레드가 공유 자원에 빠르게 접근하고 반납해야 하는 시나리오에서 매우 효율적이다.
  

## 주요 구성 요소 및 함수
  * **`SLIST_HEADER`**: 리스트의 시작점을 관리하는 구조체입니다. 리스트를 사용하기 전에 반드시 초기화해야 합니다.
  * **`SLIST_ENTRY`**: 리스트에 저장될 각 항목(노드)이 포함해야 하는 구조체입니다. 이 구조체를 데이터 구조체의 첫 번째 멤버로 포함시켜야 합니다.
  * **`InitializeSListHead()`**: `SLIST_HEADER`를 초기화합니다.
  * **`InterlockedPushEntrySList()`**: 리스트의 맨 앞에 원자적으로 새 항목을 추가합니다. (LIFO - Last-In, First-Out)
  * **`InterlockedPopEntrySList()`**: 리스트의 맨 앞에서 원자적으로 항목을 제거합니다.
  * **`QueryDepthSList()`**: 리스트에 현재 몇 개의 항목이 있는지 확인합니다. (이 함수는 락프리가 아니므로 동기화가 필요할 수 있다.)
  * **`InterlockedFlushSList()`**: 리스트의 모든 항목을 원자적으로 제거하여 반환합니다.
  

## ✅ 간단한 사용 예제 (싱글 스레드)
먼저, 하나의 스레드에서 `SLIST`를 어떻게 초기화하고, 데이터를 넣고 빼는지 기본적인 사용법을 보여주는 예제이다.

```cpp
#include <iostream>
#include <windows.h>

#define CONTAINING_RECORD(address, type, field) \
    ((type *)((char*)(address) - (size_t)offsetof(type, field)))

// 리스트에 저장할 데이터 구조체 정의
// SLIST_ENTRY가 반드시 첫 번째 멤버여야 합니다.
typedef struct _MY_DATA {
    SLIST_ENTRY ItemEntry; // SLIST 노드 정보
    int id;
} MY_DATA, *PMY_DATA;

int main() {
    // 1. SList 헤더를 선언하고 초기화합니다.
    // _aligned_malloc을 사용하여 메모리 정렬을 맞춰주는 것이 중요합니다.
    PSLIST_HEADER pListHead = (PSLIST_HEADER)_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT);
    if (pListHead == NULL) {
        std::cerr << "Memory allocation failed." << std::endl;
        return 1;
    }
    InitializeSListHead(pListHead);

    // 2. 데이터 생성 및 리스트에 추가 (Push)
    for (int i = 0; i < 5; ++i) {
        PMY_DATA pData = (PMY_DATA)_aligned_malloc(sizeof(MY_DATA), MEMORY_ALLOCATION_ALIGNMENT);
        pData->id = i;
        std::cout << "Push: " << pData->id << std::endl;
        InterlockedPushEntrySList(pListHead, &(pData->ItemEntry));
    }

    std::cout << "\n-------------------\n" << std::endl;

    // 3. 리스트에서 데이터 꺼내기 (Pop)
    PSLIST_ENTRY pListEntry;
    while ((pListEntry = InterlockedPopEntrySList(pListHead)) != NULL) {
        // SLIST_ENTRY 포인터로부터 원래 데이터 구조체의 포인터를 계산합니다.
        PMY_DATA pData = CONTAINING_RECORD(pListEntry, MY_DATA, ItemEntry);
        //SLIST_ENTRY가 MY_DATA에서 꼭 첫번째 멤버라면 그냥 캐스팅으로도 충분한다.  
        //PMY_DATA pData = reinterpret_cast<MY_DATA*>(pListEntry);  

        std::cout << "Pop: " << pData->id << std::endl;

        // 사용이 끝난 메모리는 해제합니다.
        _aligned_free(pData);
    }

    // 4. SList 헤더 메모리 해제
    _aligned_free(pListHead);

    return 0;
}
```

**실행 결과:**

```
Push: 0
Push: 1
Push: 2
Push: 3
Push: 4

-------------------

Pop: 4
Pop: 3
Pop: 2
Pop: 1
Pop: 0
```

`SLIST`는 스택(Stack)처럼 동작하기 때문에 가장 마지막에 추가된 `4`가 가장 먼저 나오는 것을 볼 수 있다.


### CONTAINING_RECORD 가 아닌 캐스팅을 사용한다면
**“SLIST_ENTRY를 구조체의 첫 번째 멤버로 둔다”를 팀 규칙으로 엄격히 고정**한다면, 그 구조체의 내부 레이아웃이 바뀌어도 `SLIST_ENTRY`의 위치는 항상 제일 앞에 있게 된다. 이 경우에는 **`reinterpret_cast`를 써도 기술적으로 안전**하다.

즉, 아래 전제들이 모두 지켜진다면 문제될 게 없다.

#### 안전한 전제 조건
1. **SLIST_ENTRY는 구조체의 첫 번째 멤버**여야 한다.
   → `offsetof(T, ItemEntry) == 0`이 항상 성립함.

2. **표준 레이아웃(standard-layout)** 구조체여야 한다.
   → 즉, 상속이나 비표준 배치가 없어야 하며 C 스타일 구조여야 한다.
   → 컴파일러가 `offsetof()` 계산을 신뢰할 수 있는 타입이어야 함.

3. **플랫폼이 동일한 ABI를 유지**해야 한다.
   → Windows MSVC/clang-cl/gcc for Windows 환경에서는 이 규칙이 일관되게 유지된다.

4. **팀 룰이 문서화되어 있고**, 구조체 템플릿이 이 룰을 위반하지 않도록 관리된다.
   → 예: “모든 SLIST 노드 구조체는 반드시 첫 번째 멤버에 SLIST_ENTRY를 둘 것”이라는 코드 컨벤션.

#### ✅ 따라서 아래는 완전히 안전

```cpp
struct MY_DATA {
    SLIST_ENTRY ItemEntry;  // 반드시 첫 번째
    int data;
    DWORD threadId;
};

PSLIST_ENTRY e = InterlockedPopEntrySList(pHead);
if (e) {
    // reinterpret_cast 안전
    MY_DATA* pData = reinterpret_cast<MY_DATA*>(e);
    printf("%d\n", pData->data);
}
```

* 여기서는 `CONTAINING_RECORD(e, MY_DATA, ItemEntry)`와 결과가 완전히 동일하다.
* 실제로 Windows 커널 코드 일부나 드라이버 샘플에서도 이런 패턴을 많이 쓴다.

#### 그럼에도 `CONTAINING_RECORD`가 여전히 쓰이는 이유
1. **의도를 명확히 표현**한다.
   → 코드 리뷰 시 “이 포인터는 SLIST_ENTRY에서 역복원된 것이다”를 한눈에 알 수 있음.

2. **첫 멤버가 아닐 수도 있는 구조체**에서도 쓸 수 있다.
   → 예: 같은 `SLIST_ENTRY` 노드를 여러 구조체에서 공유하는 경우.

3. **윈도우 표준 패턴**으로 이미 정착되어 있다.
   → MS 내부 코드나 WDK 예제에서도 `reinterpret_cast` 대신 `CONTAINING_RECORD`를 사용함.
   → 일종의 문서화된 안전 패턴이다.

#### 결론
* **규칙적으로 첫 멤버로 고정한다면** `reinterpret_cast`를 써도 완전히 문제없다.
* 다만 **코드 명확성과 일관성**을 위해 `CONTAINING_RECORD`를 유지하는 게 더 일반적이다.
* 즉, **기술적으로는 안전하지만, 유지보수 측면에서는 매크로가 낫다**는 것이 정답이다.
    
</br>   

## 🚀 락프리 컨테이너 사용 예제 (멀티 스레드)
이제 `SLIST`의 진정한 강점인 락프리 특성을 활용한 예제이다. 여러 개의 **생산자(Producer) 스레드**가 동시에 데이터를 `SLIST`에 추가하고, 여러 개의 **소비자(Consumer) 스레드**가 동시에 데이터를 가져가는 시나리오이다.

이 과정에서 별도의 `Mutex`나 `Critical Section` 같은 락을 전혀 사용하지 않는다.

```cpp
#include <iostream>
#include <windows.h>
#include <vector>
#include <process.h> // for _beginthreadex

// 리스트에 저장할 데이터 구조체
typedef struct _MY_DATA {
    SLIST_ENTRY ItemEntry;
    int data;
    DWORD threadId;
} MY_DATA, *PMY_DATA;

// 스레드에 전달할 정보
typedef struct _THREAD_PARAMS {
    PSLIST_HEADER pListHead;
    HANDLE hStartEvent;
} THREAD_PARAMS, *PTHREAD_PARAMS;

// 생산자 스레드 함수
unsigned int __stdcall ProducerThread(void* pParam) {
    PTHREAD_PARAMS params = (PTHREAD_PARAMS)pParam;
    DWORD threadId = GetCurrentThreadId();

    // 모든 스레드가 동시에 시작하도록 이벤트 대기
    WaitForSingleObject(params->hStartEvent, INFINITE);

    for (int i = 0; i < 10; ++i) {
        PMY_DATA pData = (PMY_DATA)_aligned_malloc(sizeof(MY_DATA), MEMORY_ALLOCATION_ALIGNMENT);
        pData->data = i;
        pData->threadId = threadId;

        // 락 없이 데이터를 리스트에 추가
        InterlockedPushEntrySList(params->pListHead, &(pData->ItemEntry));
        // std::cout은 스레드 안전하지 않으므로 실제 환경에서는 다른 로깅 방식을 사용해야 합니다.
        // 여기서는 예시를 위해 간단히 사용합니다.
        printf("Producer [%lu] pushed data %d\n", threadId, i);
        Sleep(50); // 다른 스레드에게 실행 기회 부여
    }
    return 0;
}

// 소비자 스레드 함수
unsigned int __stdcall ConsumerThread(void* pParam) {
    PTHREAD_PARAMS params = (PTHREAD_PARAMS)pParam;
    DWORD threadId = GetCurrentThreadId();

    WaitForSingleObject(params->hStartEvent, INFINITE);

    for (int i = 0; i < 10; ++i) {
        // 락 없이 데이터를 리스트에서 꺼냄
        PSLIST_ENTRY pListEntry = InterlockedPopEntrySList(params->pListHead);

        if (pListEntry != NULL) {
            PMY_DATA pData = CONTAINING_RECORD(pListEntry, MY_DATA, ItemEntry);
            printf("Consumer [%lu] popped data %d (from thread %lu)\n", threadId, pData->data, pData->threadId);
            _aligned_free(pData);
        } else {
            printf("Consumer [%lu] found list empty.\n", threadId);
        }
        Sleep(100);
    }
    return 0;
}

int main() {
    // SList 헤더 초기화
    PSLIST_HEADER pListHead = (PSLIST_HEADER)_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT);
    InitializeSListHead(pListHead);

    // 스레드 동시 시작을 위한 이벤트 생성
    HANDLE hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    THREAD_PARAMS params = { pListHead, hStartEvent };

    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 3;
    std::vector<HANDLE> hThreads;

    // 생산자 스레드 생성
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        hThreads.push_back((HANDLE)_beginthreadex(NULL, 0, ProducerThread, &params, 0, NULL));
    }

    // 소비자 스레드 생성
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        hThreads.push_back((HANDLE)_beginthreadex(NULL, 0, ConsumerThread, &params, 0, NULL));
    }

    std::cout << "Starting all threads...\n";
    SetEvent(hStartEvent); // 모든 스레드 시작 신호!

    // 모든 스레드가 종료될 때까지 대기
    WaitForMultipleObjects((DWORD)hThreads.size(), hThreads.data(), TRUE, INFINITE);

    std::cout << "All threads finished.\n";

    // 스레드 핸들 및 이벤트 핸들 정리
    for (HANDLE h : hThreads) {
        CloseHandle(h);
    }
    CloseHandle(hStartEvent);

    // 남은 데이터 정리 (실제로는 남지 않아야 함)
    PSLIST_ENTRY pListEntry;
    while ((pListEntry = InterlockedPopEntrySList(pListHead)) != NULL) {
        PMY_DATA pData = CONTAINING_RECORD(pListEntry, MY_DATA, ItemEntry);
        _aligned_free(pData);
    }

    _aligned_free(pListHead);

    return 0;
}
```

이 예제를 실행하면, 여러 생산자 스레드와 소비자 스레드가 뒤섞여 출력되면서도 충돌 없이 `SLIST`에 데이터를 넣고 빼는 것을 확인할 수 있다. 이는 `InterlockedPushEntrySList`와 `InterlockedPopEntrySList`가 내부적으로 원자성을 보장해주기 때문이다. 이처럼 `SLIST`는 락으로 인한 성능 저하나 데드락(Deadlock)의 위험 없이 고성능의 동기화 구조를 만들 때 매우 유용하다.


## SLIST를 Queue 처럼 사용하기 
큐처럼 사용하고 싶다면 “두 개의 스택(SLIST)”을 조합하면 된다.

* **PushList**: 생산자가 데이터를 넣는 쪽 (기존처럼 `InterlockedPushEntrySList`)
* **PopList**: 소비자가 꺼내는 쪽

원리:

1. 소비자가 데이터를 꺼내려 할 때 `PopList`가 비어 있으면
   → `InterlockedFlushSList(PushList)`를 호출해서 모든 항목을 한 번에 가져온 후
   → 역순으로 `PopList`에 다시 push
2. 이제 `PopList`에서 pop하면 **입력 순서대로(FIFO)** 데이터가 나온다.


### 예시 코드 (핵심 부분만)

```cpp
// 두 개의 SLIST 사용
SLIST_HEADER PushList;
SLIST_HEADER PopList;

InitializeSListHead(&PushList);
InitializeSListHead(&PopList);

// Producer
void PushData(int value)
{
    PMY_DATA pData = (PMY_DATA)_aligned_malloc(sizeof(MY_DATA), MEMORY_ALLOCATION_ALIGNMENT);
    pData->data = value;
    InterlockedPushEntrySList(&PushList, &pData->ItemEntry);
}

// Consumer
PMY_DATA PopData()
{
    PSLIST_ENTRY entry = InterlockedPopEntrySList(&PopList);
    if (!entry)
    {
        // PopList가 비었으면 PushList의 모든 데이터를 가져온 뒤 역순으로 넣기
        PSLIST_ENTRY flushed = InterlockedFlushSList(&PushList);
        while (flushed)
        {
            PSLIST_ENTRY next = flushed->Next;
            flushed->Next = NULL;
            InterlockedPushEntrySList(&PopList, flushed);
            flushed = next;
        }
        entry = InterlockedPopEntrySList(&PopList);
    }

    if (entry)
        return CONTAINING_RECORD(entry, MY_DATA, ItemEntry);

    return NULL;
}
```

### 특징 및 주의점

| 항목         | 설명                                          |
| ---------- | ------------------------------------------- |
| Lock-free  | SLIST의 atomic 연산만 사용하므로 락이 없음               |
| FIFO 보장    | Flush + Reverse 재배열로 FIFO 순서 유지             |
| 다중 스레드 안전성 | Producer 다중, Consumer 단일일 때 안전함             |
| 단점         | 다중 Consumer 환경에서는 동기화가 필요함 (Flush 시점 충돌 가능) |


### 정리

* SLIST는 본래 **Stack** 전용이므로 Queue를 직접 지원하지 않는다.
* Queue처럼 사용하려면 **Flush + Reverse** 트릭을 써야 한다.
* 멀티스레드 다중 소비자가 동시에 접근하는 경우, Lock-free FIFO 큐를 원한다면
  → `Windows API의 SRWLock + std::queue` 또는
  → **MSDN의 SListQueue 예제**(LockFreeQueue) 같은 커스텀 구현을 참고하는 것이 좋다.


### 멀티프로듀서/멀티컨슈머(MPMC) 대응 버전  
핵심은 `PushHead`(enqueue 용)와 `PopHead`(dequeue 용)를 분리하고, 소비자가 `PopHead`가 비었을 때 **한 명만** `PushHead`를 `Flush`해서 `PopHead`로 역순 재적재하는 “리필(refill)”을 수행하도록 **원자적 플래그**로 보호하는 것이다. 평상시에는 완전 lock-free로 동작하고, 리필 구간만 짧게 CAS로 상호배제를 걸어준다.

```cpp
// SList FIFO Queue (MPMC) using two SLISTs + CAS-based refill section
// - Windows Vista+ (Interlocked SList APIs)
// - Build: /MT or /MD, C++17 이상 권장
// - 주의: SLIST 노드는 MEMORY_ALLOCATION_ALIGNMENT 정렬이 필요함

#include <windows.h>
#include <process.h>
#include <cstdio>
#include <vector>

#define CONTAINING_RECORD(address, type, field) \
    ((type *)((char*)(address) - (size_t)offsetof(type, field)))


// ====== 데이터 노드 ======
typedef struct _NODE {
    SLIST_ENTRY Entry;         // 반드시 첫 필드 아니어도 되지만 포함되어야 함
    int value;
    DWORD producerTid;
} NODE, *PNODE;

// ====== 큐 구조 ======
typedef struct _SLIST_QUEUE {
    SLIST_HEADER PushHead;     // 생산자가 넣는 쪽 (LIFO)
    SLIST_HEADER PopHead;      // 소비자가 꺼내는 쪽 (LIFO)
    volatile LONG RefillLock;  // 0: unlock, 1: lock (CAS로 리필 구간 보호)
} SLIST_QUEUE, *PSLIST_QUEUE;

void SListQueueInit(PSLIST_QUEUE q) {
    InitializeSListHead(&q->PushHead);
    InitializeSListHead(&q->PopHead);
    q->RefillLock = 0;
}

void SListQueueDestroy(PSLIST_QUEUE q) {
    // 남은 노드 정리
    PSLIST_ENTRY e;
    while ((e = InterlockedPopEntrySList(&q->PopHead)) != NULL) {
        PNODE n = CONTAINING_RECORD(e, NODE, Entry);
        _aligned_free(n);
    }
    while ((e = InterlockedPopEntrySList(&q->PushHead)) != NULL) {
        PNODE n = CONTAINING_RECORD(e, NODE, Entry);
        _aligned_free(n);
    }
}

// ====== Enqueue (MP-safe) ======
void SListQueueEnqueue(PSLIST_QUEUE q, PNODE node) {
    // PushHead에 LIFO로 넣는다
    InterlockedPushEntrySList(&q->PushHead, &node->Entry);
}

// ====== 내부: PushHead -> PopHead 리필 ======
void SListQueueRefill(PSLIST_QUEUE q) {
    // 누군가 이미 리필 중이면 넘어간다
    if (InterlockedCompareExchange(&q->RefillLock, 1, 0) != 0) {
        return; // 다른 스레드가 리필 중
    }

    // 우리가 리필 담당자가 되었음
    PSLIST_ENTRY flushed = InterlockedFlushSList(&q->PushHead);
    // flushed는 PushHead의 head부터 최신 push 순서대로 연결된 단일 리스트(LIFO)다
    // PopHead에 다시 push하면 역순이 되므로 전체적으로 FIFO가 성립한다
    while (flushed) {
        PSLIST_ENTRY next = flushed->Next;
        flushed->Next = NULL;
        InterlockedPushEntrySList(&q->PopHead, flushed);
        flushed = next;
    }

    // 잠금 해제
    InterlockedExchange(&q->RefillLock, 0);
}

// ====== Dequeue (MC-safe) ======
PNODE SListQueueDequeue(PSLIST_QUEUE q) {
    // 1) PopHead에서 먼저 꺼내본다
    PSLIST_ENTRY e = InterlockedPopEntrySList(&q->PopHead);
    if (e) {
        return CONTAINING_RECORD(e, NODE, Entry);
    }

    // 2) 비었다면 리필 시도
    SListQueueRefill(q);

    // 3) 다시 Pop 시도
    e = InterlockedPopEntrySList(&q->PopHead);
    if (e) {
        return CONTAINING_RECORD(e, NODE, Entry);
    }

    // 4) 그래도 없으면 진짜 비어있다
    return NULL;
}

// ====== 테스트용 생산자/소비자 ======
typedef struct _THREAD_PARAMS {
    PSLIST_QUEUE pQueue;
    HANDLE hStartEvent;
    int itemsPerProducer;
    LONG* pDoneProducers;       // 종료 카운터
    int totalProducers;
    LONG* pConsumedCount;       // 소비 카운터
    int totalExpected;          // 전체 기대 아이템 수
} THREAD_PARAMS, *PTHREAD_PARAMS;

unsigned __stdcall ProducerThread(void* p) {
    PTHREAD_PARAMS prm = (PTHREAD_PARAMS)p;
    WaitForSingleObject(prm->hStartEvent, INFINITE);

    DWORD tid = GetCurrentThreadId();
    for (int i = 0; i < prm->itemsPerProducer; ++i) {
        PNODE n = (PNODE)_aligned_malloc(sizeof(NODE), MEMORY_ALLOCATION_ALIGNMENT);
        n->value = i;
        n->producerTid = tid;
        SListQueueEnqueue(prm->pQueue, n);
        // 데모 출력을 너무 많이 하면 느려질 수 있으니 필요 시 주석
        // printf("P[%lu] enqueue %d\n", tid, i);
        // Sleep(1);
    }

    InterlockedIncrement(prm->pDoneProducers);
    return 0;
}

unsigned __stdcall ConsumerThread(void* p) {
    PTHREAD_PARAMS prm = (PTHREAD_PARAMS)p;
    WaitForSingleObject(prm->hStartEvent, INFINITE);

    for (;;) {
        PNODE n = SListQueueDequeue(prm->pQueue);
        if (n) {
            LONG c = InterlockedIncrement(prm->pConsumedCount);
            // 데모용 출력
            printf("C[%lu] dequeue val=%d from P[%lu] (consumed=%ld)\n",
                   GetCurrentThreadId(), n->value, n->producerTid, c);
            _aligned_free(n);
            continue;
        }

        // 큐가 비었을 때, 모든 생산자가 끝났고 더 이상 들어올 게 없으면 종료
        LONG done = *prm->pDoneProducers;
        if (done >= prm->totalProducers) {
            // 혹시 막판에 뒤늦게 들어온 아이템이 있는지 한 번만 더 시도
            PNODE last = SListQueueDequeue(prm->pQueue);
            if (last) {
                LONG c = InterlockedIncrement(prm->pConsumedCount);
                printf("C[%lu] dequeue val=%d from P[%lu] (consumed=%ld)\n",
                       GetCurrentThreadId(), last->value, last->producerTid, c);
                _aligned_free(last);
                continue;
            }
            break;
        }

        // 바쁜 대기 과도 방지
        Sleep(0);
    }
    return 0;
}

// ====== 메인 ======
int main() {
    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 3;
    const int ITEMS_PER_PRODUCER = 20;

    SLIST_QUEUE q;
    SListQueueInit(&q);

    HANDLE hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    LONG doneProducers = 0;
    LONG consumedCount = 0;
    THREAD_PARAMS prm;
    prm.pQueue = &q;
    prm.hStartEvent = hStartEvent;
    prm.itemsPerProducer = ITEMS_PER_PRODUCER;
    prm.pDoneProducers = &doneProducers;
    prm.totalProducers = NUM_PRODUCERS;
    prm.pConsumedCount = &consumedCount;
    prm.totalExpected = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::vector<HANDLE> threads;

    // 생산자
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        threads.push_back((HANDLE)_beginthreadex(NULL, 0, ProducerThread, &prm, 0, NULL));
    }
    // 소비자
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        threads.push_back((HANDLE)_beginthreadex(NULL, 0, ConsumerThread, &prm, 0, NULL));
    }

    printf("Starting all threads...\n");
    SetEvent(hStartEvent);

    WaitForMultipleObjects((DWORD)threads.size(), threads.data(), TRUE, INFINITE);
    printf("All threads joined. expected=%d, consumed=%ld\n",
           prm.totalExpected, consumedCount);

    for (HANDLE h : threads) CloseHandle(h);
    CloseHandle(hStartEvent);

    // 혹시 남은 노드 정리
    SListQueueDestroy(&q);
    return 0;
}
```

### 동작 요약

* `SListQueueEnqueue`는 항상 `PushHead`에 넣으므로 **멀티 프로듀서**가 동시에 호출해도 안전하다.
* `SListQueueDequeue`는 먼저 `PopHead`에서 꺼내고, 비어 있으면 **리필 시도**를 한다.

  * 리필은 `RefillLock`을 `InterlockedCompareExchange`로 획득한 **한 스레드만** 수행하므로 **멀티 컨슈머**에서도 안전하다.
  * 리필 과정은 `InterlockedFlushSList(PushHead)`로 한 번에 가져온 뒤 `PopHead`에 **역순 푸시**해서 전체적으로 FIFO가 되게 한다.
* 종료 조건은 생산자들이 모두 끝났고 큐가 비었을 때 소비자가 루프를 탈출하게 했다.

### 주의사항

* SLIST 노드 메모리는 `_aligned_malloc(..., MEMORY_ALLOCATION_ALIGNMENT)`로 할당하고 `_aligned_free`로 해제해야 한다.
* 이 구조는 일반 경로(리필이 필요 없는 경우)에서 lock-free로 동작하고, **리필 구간만** 짧게 CAS로 보호하므로 높은 스루풋을 기대할 수 있다.
* 진짜 완전한 lock-free MPMC FIFO(ABA 문제·메모리 재활용 포함)를 원한다면 Michael–Scott queue 같은 전용 알고리즘을 고려하는 것이 좋다. 다만 여기서는 **SLIST를 반드시 써야 한다**는 제약을 만족시키는 실용적 타협안이다.
