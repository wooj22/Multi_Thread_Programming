/*
	[ CAS(Compare-And-Swap) 패턴 ]
	
	CAS는 원자적으로 메모리 위치의 값을 비교하고 교체하는 동작을 수행한다.
	이 패턴은 lock-free 자료구조와 알고리즘에서 자주 사용된다.

	InterlockedCompareExchange(Destination, Exchange, Comperand)
	 : 세 개의 인수를 받아서 지정된 메모리 위치의 값을 비교하고 맞다면 교체한다.
	- volatile LONG* Destination : 값을 교체할 메모리 위치의 포인터
	- LONG Exchange : 교체할 새로운 값
	- LONG Comperand : 비교할 값
*/

// 간단한 lock - free 큐(다중 생산자 / 다중 소비자(MPMC)) 실습
// SPSC 때처럼 단순하지 않고, 여러 스레드가 동시에 `head`와 `tail`을 갱신하므로 CAS(Compare-And-Swap) 를 사용해야 한다.
// 가능한 한 간단한 고정 크기 원형 버퍼 기반으로 구현

#include <windows.h>
#include <iostream>
using namespace std;

const int QUEUE_SIZE = 8;

/* Lock Free Queue */
struct LockFreeQueue
{
	volatile LONG head;
	volatile LONG tail;
	int buffer[QUEUE_SIZE];
};


void InitQueue(LockFreeQueue* q)
{
	q->head = 0;
	q->tail = 0;
}


// 삽입 : 여러 생산자가 동시에 호출 가능
bool Enqueue(LockFreeQueue* q, int value)
{
	while (true)
	{
		LONG tail = q->tail;
		LONG head = q->head;
		LONG nextTail = (tail + 1) % QUEUE_SIZE;

		// full condition
		if (nextTail == head) return false;

		// tail CAS((InterlockedCompareExchange) 시도
		// &q->tail == tail라면 nextTail로 교체 후 &q->tail 반환
		if (InterlockedCompareExchange(&q->tail, nextTail, tail) == tail)
		{
			// 성공적으로 tail을 갱신했다면 값 저장
			q->buffer[tail] = value;
			return true;
		}

		// 실패했다면 다른 스레드가 tail을 갱신한 것이므로 다시 시도
	}
}


// 삭제 : 여러 소비자가 동시에 호출 가능
bool Dequeue(LockFreeQueue* q, int* outValue) {
	while (true) {
		LONG head = q->head;
		LONG tail = q->tail;

		if (head == tail) {
			// 큐가 비어 있음
			return false;
		}

		LONG nextHead = (head + 1) % QUEUE_SIZE;

		// head CAS 시도
		if (InterlockedCompareExchange(&q->head, nextHead, head) == head) {
			*outValue = q->buffer[head];
			return true;
		}
		// 실패 시 다른 스레드가 head 업데이트 → 재시도
	}
}


LockFreeQueue g_queue;

DWORD WINAPI Producer(LPVOID param) {
	int id = (int)(INT_PTR)param;
	for (int i = 0; i < 10; i++) {
		while (!Enqueue(&g_queue, id * 100 + i)) {
			Sleep(1); // 큐가 가득 차면 잠시 대기
		}
		std::cout << "Producer " << id << " -> " << (id * 100 + i) << std::endl;
	}
	return 0;
}

DWORD WINAPI Consumer(LPVOID param) {
	int id = (int)(INT_PTR)param;
	int value;
	for (int i = 0; i < 10; i++) {
		while (!Dequeue(&g_queue, &value)) {
			Sleep(1); // 큐가 비면 잠시 대기
		}
		std::cout << "Consumer " << id << " <- " << value << std::endl;
	}
	return 0;
}

int main() {
	InitQueue(&g_queue);

	// 생산자 2개, 소비자 2개
	HANDLE producers[2];
	HANDLE consumers[2];

	producers[0] = CreateThread(NULL, 0, Producer, (LPVOID)1, 0, NULL);
	producers[1] = CreateThread(NULL, 0, Producer, (LPVOID)2, 0, NULL);

	consumers[0] = CreateThread(NULL, 0, Consumer, (LPVOID)1, 0, NULL);
	consumers[1] = CreateThread(NULL, 0, Consumer, (LPVOID)2, 0, NULL);

	WaitForMultipleObjects(2, producers, TRUE, INFINITE);
	WaitForMultipleObjects(2, consumers, TRUE, INFINITE);

	return 0;
}
