/*

	���� 3: ��� - ���� ���ҽ� �ε� �ý���  
	`WaitForMultipleObjects`�� `bWaitAll = FALSE` �ɼ��� Ȱ���Ͽ� �켱������ �ִ� �۾����� ȿ�������� �����ϴ� ����� ������.  
  
	����: ���� ���� �� ���� ���ҽ�(�ؽ�ó, ����, �� ������)�� ���ÿ� �ε��ϵ�, �ʼ� ���ҽ��� ���� �ε��Ǹ� ������ ������ �� �ִ� �ý����� �����ϼ���.

	�䱸����:
	- 4���� ���ҽ� �ε� ������ ����
	- `WaitForMultipleObjects`�� `bWaitAll = FALSE` �ɼ� Ȱ��
	- �ʼ� ���ҽ� �ε� �Ϸ� �� ���� ����, �������� ��׶��忡�� ��� �ε�

*/

#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>

struct ResourceData
{
    std::string name;
    bool isEssential; // �ʼ� ���ҽ� ����
    int loadTime;     // �ε� �ð� (��)
    bool isLoaded;    // �ε� �Ϸ� ����
};

unsigned __stdcall ResourceLoadThread(void* param)
{
    ResourceData* resource = (ResourceData*)param;

    std::cout << "[�ε� ����] " << resource->name;
    if (resource->isEssential) {
        std::cout << " (�ʼ�)";
    }
    std::cout << std::endl;

    for (int i = 1; i <= resource->loadTime; ++i) {
        Sleep(1000);
        std::cout << resource->name << " �ε� ��... ("
            << i << "/" << resource->loadTime << ")" << std::endl;
    }

    resource->isLoaded = true;
    std::cout << "[�ε� �Ϸ�] " << resource->name << std::endl;
    return 0;
}

int main()
{
    // ���ҽ� ������ ����
    ResourceData resources[4] = {
        {"�⺻ �ؽ�ó", true, 2},      // �ʼ�
        {"�÷��̾� ��", true, 3},    // �ʼ�
        {"�������", false, 5},       // ������
        {"ȿ����", false, 4}          // ������
    };

    HANDLE hThreads[4];
    HANDLE hEssentialThreads[2]; // �ʼ� ���ҽ���
    int essentialCount = 0;

    // ��� ���ҽ� �ε� ������ ����
    for (int i = 0; i < 4; ++i) {
        hThreads[i] = (HANDLE)_beginthreadex(
            NULL, 0, ResourceLoadThread, &resources[i], 0, NULL
        );

        if (hThreads[i] == NULL) {
            std::cout << "������ " << i << " ���� ����!" << std::endl;
            return -1;
        }

        // �ʼ� ���ҽ� �ڵ� ���� ����
        if (resources[i].isEssential) {
            hEssentialThreads[essentialCount++] = hThreads[i];
        }
    }

    std::cout << "\n���� ���ҽ� �ε��� �����մϴ�..." << std::endl;
    std::cout << "�ʼ� ���ҽ� �ε� �Ϸ� �� ������ ���۵˴ϴ�.\n" << std::endl;

    // Q. �ʼ� ���ҽ��� �Ϸ�Ǳ⸦ ��� (bWaitAll = TRUE)
    WaitForMultipleObjects(2, hEssentialThreads, TRUE, INFINITE);

    std::cout << "\n=== �ʼ� ���ҽ� �ε� �Ϸ�! ������ �����մϴ�! ===" << std::endl;
    std::cout << "������ ���ҽ��� ��׶��忡�� ��� �ε��˴ϴ�...\n" << std::endl;

    // ���� �ùķ��̼� (3��)
    for (int i = 1; i <= 3; ++i) {
        Sleep(1000);
        std::cout << "���� �÷��� ��... (" << i << "/3)" << std::endl;
    }

    std::cout << "\n��� ���ҽ� �ε� �ϷḦ ��ٸ��ϴ�..." << std::endl;

    // Q. ������ ��� ���ҽ� �Ϸ� ���
    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

        std::cout << "\n��� ���ҽ� �ε��� �Ϸ�Ǿ����ϴ�!" << std::endl;
    std::cout << "���� ��� ���� ����� ����� �� �ֽ��ϴ�." << std::endl;

    // �ڵ� ����
    for (int i = 0; i < 4; ++i) {
        CloseHandle(hThreads[i]);
    }

    return 0;
}