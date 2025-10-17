## ���� 3: CAS�� �̿��� Lock - Free ������ ������Ʈ

* *�䱸���� : **
���� ���� `g_value`�� �ִ�.������� `g_value`�� Ư�� ��(`expectedValue`)�� ���� ���ο� ��(`newValue`)���� ������Ʈ�ؾ� �Ѵ�.�̷��� '�� �� ��ü' ������ `InterlockedCompareExchange` (CAS)�� ����Ͽ� �����Ѵ�.

    * *�ǽ� �ڵ� : **

    ```cpp
#include <windows.h>
#include <iostream>

    LONG g_value = 100;

// g_value�� expectedValue�� ���� ���� newValue�� ������Ʈ�ϴ� �Լ�
void UpdateValue(LONG newValue, LONG expectedValue) {
    LONG originalValue;

    // TODO: g_value�� ���� ���� expectedValue�� ������ newValue�� ��ü�ϼ���.
    // InterlockedCompareExchange �Լ��� ����ϰ�, ��ȯ��(��ü ���� ��)�� originalValue�� �����ϼ���.
    // originalValue = ...

    if (originalValue == expectedValue) {
        printf("[����] ���� %ld���� %ld�� ����Ǿ����ϴ�.\n", originalValue, g_value);
    }
    else {
        printf("[����] �� ���� �õ� ����. (��밪: %ld, ���簪: %ld)\n", expectedValue, originalValue);
    }
}

int main() {
    printf("�ʱⰪ: %ld\n", g_value);

    // ���� ���̽�: g_value�� 100�� �� 200���� ���� �õ�
    UpdateValue(200, 100);

    // ���� ���̽�: g_value�� 100�� �� 300���� ���� �õ� (��밪�� Ʋ��)
    UpdateValue(300, 100); // �� �������� g_value�� �̹� 200�̹Ƿ� �����ؾ� ��

    printf("������: %ld\n", g_value);
    return 0;
}
```

** ���� �� �ؼ� : **

```cpp
// ... (����) ...

void UpdateValue(LONG newValue, LONG expectedValue) {
    LONG originalValue;

    // TODO: g_value�� ���� ���� expectedValue�� ������ newValue�� ��ü�ϼ���.
    originalValue = InterlockedCompareExchange(&g_value, newValue, expectedValue);

    if (originalValue == expectedValue) {
        printf("[����] ���� %ld���� %ld�� ����Ǿ����ϴ�.\n", originalValue, g_value);
    }
    else {
        printf("[����] �� ���� �õ� ����. (��밪: %ld, ���簪: %ld)\n", expectedValue, originalValue);
    }
}

// ... (����) ...
```

*** �ؼ�** :
*`InterlockedCompareExchange(& g_value, newValue, expectedValue)`�� ���������� ������ ���� �����մϴ�.
1.  `g_value`�� ���� ���� �д´�.
2.  �� ���� `expectedValue`(�� ��° ����)�� ������ ���Ѵ�.
3.  ������ `g_value`�� `newValue`(�� ��° ����)�� ������Ʈ�Ѵ�.
4. * *�� ����� ������� ���� ���� `g_value` �� * *�� ��ȯ�Ѵ�.
* ���� ��ȯ�� `originalValue`�� ���� �����ߴ� `expectedValue`�� ���ϸ� CAS ������ ���� ���θ� ��Ȯ�� �� �� �ֽ��ϴ�.

---- -