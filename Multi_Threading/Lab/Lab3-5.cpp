## ���� 5: ��Ʈ �÷��׸� �̿��� ������ ���� ����

* *�䱸���� : **
�ϳ��� ���� ���� `g_flags`�� ����Ͽ� ���� ���¸� ��Ʈ����ũ�� �����Ѵ�. `FLAG_A` (0x01), `FLAG_B` (0x02), `FLAG_C` (0x04) �� ���� �÷��װ� �ִ�. `Interlocked` ��Ʈ ���� �Լ��� ����Ͽ� �����忡 �����ϰ� �÷��׸� �߰��ϰ� �����ϴ� �ڵ带 �ۼ��Ѵ�.

** �ǽ� �ڵ� : **

```cpp
#include <windows.h>
#include <iostream>

LONG g_flags = 0; // �ʱ� ���´� 0

#define FLAG_A 0x01 // 0001
#define FLAG_B 0x02 // 0010
#define FLAG_C 0x04 // 0100

// �÷��׸� ���������� �߰��ϴ� �Լ�
void AddFlag(LONG flag) {
    // TODO 1: g_flags�� flag�� ���������� �߰�(OR ����)�ϼ���.
}

// �÷��׸� ���������� �����ϴ� �Լ�
void RemoveFlag(LONG flag) {
    // TODO 2: g_flags���� flag�� ���������� ����(AND ����)�ϼ���.
    // ��Ʈ: �����Ϸ��� ��Ʈ�� 0���� ����� �������� 1�� �����ϴ� ����ũ(~flag)�� ����ؾ� �մϴ�.
}

void PrintFlags() {
    printf("���� Flags: 0x%02lX\n", g_flags);
}

int main() {
    PrintFlags(); // �ʱ� ����: 0x00

    // ������ 1�� FLAG_A�� FLAG_C�� �߰��Ѵٰ� ����
    AddFlag(FLAG_A | FLAG_C);
    PrintFlags(); // ����: 0x05

    // ������ 2�� FLAG_B�� �߰��Ѵٰ� ����
    AddFlag(FLAG_B);
    PrintFlags(); // ����: 0x07

    // ������ 3�� FLAG_A�� �����Ѵٰ� ����
    RemoveFlag(FLAG_A);
    PrintFlags(); // ����: 0x06

    return 0;
}
```

** ���� �� �ؼ� : **

```cpp
// ... (����) ...

// �÷��׸� ���������� �߰��ϴ� �Լ�
void AddFlag(LONG flag) {
    // TODO 1: g_flags�� flag�� ���������� �߰�(OR ����)�ϼ���.
    InterlockedOr(&g_flags, flag);
}

// �÷��׸� ���������� �����ϴ� �Լ�
void RemoveFlag(LONG flag) {
    // TODO 2: g_flags���� flag�� ���������� ����(AND ����)�ϼ���.
    // ��Ʈ: �����Ϸ��� ��Ʈ�� 0���� ����� �������� 1�� �����ϴ� ����ũ(~flag)�� ����ؾ� �մϴ�.
    InterlockedAnd(&g_flags, ~flag);
}

// ... (����) ...
```

*** �ؼ�** :
***�÷��� �߰�** : `InterlockedOr(& g_flags, flag)`�� `g_flags |= flag` ������ ���������� �����մϴ�.Ư�� ��Ʈ�� `1`�� ����(�߰�)�ϴ� �� ���˴ϴ�.
* **�÷��� ���� * *: `InterlockedAnd( & g_flags, ~flag)`�� `g_flags &= ~flag` ������ ���������� �����մϴ�. `~flag`�� �����Ϸ��� ��Ʈ�� `0`�̰� �������� `1`�� ��Ʈ����ũ�� ����ϴ�.�� ����ũ�� `AND` ������ �ϸ� ���ϴ� ��Ʈ�� `0`���� ����(����)�� �� �ֽ��ϴ�.