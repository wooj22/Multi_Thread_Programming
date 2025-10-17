/*
	[ CAS(Compare-And-Swap) ���� ]
	
	CAS�� ���������� �޸� ��ġ�� ���� ���ϰ� ��ü�ϴ� ������ �����Ѵ�.
	�� ������ lock-free �ڷᱸ���� �˰��򿡼� ���� ���ȴ�.

	InterlockedCompareExchange(Destination, Exchange, Comperand)
	 : �� ���� �μ��� �޾Ƽ� ������ �޸� ��ġ�� ���� ���ϰ� �´ٸ� ��ü�Ѵ�.
	- volatile LONG* Destination : ���� ��ü�� �޸� ��ġ�� ������
	- LONG Exchange : ��ü�� ���ο� ��
	- LONG Comperand : ���� ��
*/

// ������ lock - free ť(���� ������ / ���� �Һ���(MPMC)) �ǽ�
// SPSC ��ó�� �ܼ����� �ʰ�, ���� �����尡 ���ÿ� `head`�� `tail`�� �����ϹǷ� CAS(Compare-And-Swap) �� ����ؾ� �Ѵ�.
// ������ �� ������ ���� ũ�� ���� ���� ������� ����

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


// ���� : ���� �����ڰ� ���ÿ� ȣ�� ����
bool Enqueue(LockFreeQueue* q, int value)
{
	while (true)
	{
		LONG tail = q->tail;
		LONG head = q->head;
		LONG nextTail = (tail + 1) % QUEUE_SIZE;

		// full condition
		if (nextTail == head) return false;

		// tail CAS((InterlockedCompareExchange) �õ�
		// &q->tail == tail��� nextTail�� ��ü �� &q->tail ��ȯ
		if (InterlockedCompareExchange(&q->tail, nextTail, tail) == tail)
		{
			// ���������� tail�� �����ߴٸ� �� ����
			q->buffer[tail] = value;
			return true;
		}

		// �����ߴٸ� �ٸ� �����尡 tail�� ������ ���̹Ƿ� �ٽ� �õ�
	}
}


// ���� : ���� �Һ��ڰ� ���ÿ� ȣ�� ����
bool Dequeue(LockFreeQueue* q, int* outValue) {
	while (true) {
		LONG head = q->head;
		LONG tail = q->tail;

		if (head == tail) {
			// ť�� ��� ����
			return false;
		}

		LONG nextHead = (head + 1) % QUEUE_SIZE;

		// head CAS �õ�
		if (InterlockedCompareExchange(&q->head, nextHead, head) == head) {
			*outValue = q->buffer[head];
			return true;
		}
		// ���� �� �ٸ� �����尡 head ������Ʈ �� ��õ�
	}
}


LockFreeQueue g_queue;

DWORD WINAPI Producer(LPVOID param) {
	int id = (int)(INT_PTR)param;
	for (int i = 0; i < 10; i++) {
		while (!Enqueue(&g_queue, id * 100 + i)) {
			Sleep(1); // ť�� ���� ���� ��� ���
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
			Sleep(1); // ť�� ��� ��� ���
		}
		std::cout << "Consumer " << id << " <- " << value << std::endl;
	}
	return 0;
}

int main() {
	InitQueue(&g_queue);

	// ������ 2��, �Һ��� 2��
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
