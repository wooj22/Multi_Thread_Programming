/*

	���� 4: ��� - �ǽð� ��Ƽ�÷��̾� �� ��� ���� �ý���
	������ �н��Ѵ�:
	- `WaitForMultipleObjects`�� Ÿ�Ӿƿ� ��� Ȱ��
	- `WAIT_TIMEOUT` ó���� ���� ���Ǻ� ���� ����
	- �ǽð� ���ӿ����� ����ȭ ����
  
	����: 4���� �÷��̾ ���ÿ� �ൿ�� �����ϰ�, ��� �÷��̾ ������ �Ϸ��ϰų� ���� �ð��� ������ ������ �����ϴ� �ý����� �����ϼ���.

	�䱸����:
	- �� �÷��̾�� ���� �����忡�� �ൿ ���� (1-5�� ����)
	- 30�� ���� �ð� ���� ��� �÷��̾ �����ϸ� ��� ���� ����
	- ���� �ð� �ʰ� �� �������� ���� �÷��̾�� �⺻ �������� ó��
	- `WaitForMultipleObjects`�� Ÿ�Ӿƿ� ��� Ȱ��

*/

#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <cstdlib>
#include <ctime>

struct PlayerAction 
{
    int playerId;
    std::string playerName;
    std::string action;
    bool hasSelected;
    int thinkingTime; // ��� �ð� (��)
};

unsigned __stdcall PlayerThinkingThread(void* param)
{
    PlayerAction* player = (PlayerAction*)param;

    std::cout << player->playerName << "��(��) �ൿ�� ����ϰ� �ֽ��ϴ�..." << std::endl;

    // ������ ��� �ð� (1-5��)
    Sleep(player->thinkingTime * 1000);

    // ���� �׼� ����
    std::string actions[] = { "����", "���", "��ų", "������", "����" };
    player->action = actions[rand() % 5];
    player->hasSelected = true;

    std::cout << "[���� �Ϸ�] " << player->playerName << " -> " << player->action << std::endl;
    return 0;
}

int main()
{
    srand((unsigned int)time(NULL));

    // �÷��̾� ������ ����
    PlayerAction players[4];
    std::string names[] = { "���� �Ƽ�", "������ �ָ�", "���� �κ�", "����� �����ϵ�" };

    for (int i = 0; i < 4; ++i) {
        players[i] = { i + 1, names[i], "", false, (rand() % 5) + 1 };
    }

    HANDLE hThreads[4];

    std::cout << "=== �� ��� ���� ����! ===" << std::endl;
    std::cout << "���� �ð�: 30��" << std::endl;
    std::cout << "��� �÷��̾ �ൿ�� �������ּ���!\n" << std::endl;

    // ������ ����
    for (int i = 0; i < 4; ++i) {
        hThreads[i] = (HANDLE)_beginthreadex(
            NULL, 0, PlayerThinkingThread, &players[i], 0, NULL
        );

        if (hThreads[i] == NULL) {
            std::cout << "�÷��̾� " << i + 1 << " ������ ���� ����!" << std::endl;
            return -1;
        }
    }

    // 30�� Ÿ�Ӿƿ�
    DWORD waitResult = WaitForMultipleObjects(4, hThreads, TRUE, 30000); // 30��
    std::cout << "\n=== ���� �ð� ����! ===" << std::endl;

    if (waitResult == WAIT_OBJECT_0) {
        std::cout << "��� �÷��̾ �ð� ���� �ൿ�� �����߽��ϴ�!" << std::endl;
    }
    else if (waitResult == WAIT_TIMEOUT) {
        std::cout << "���� �ð��� �ʰ��Ǿ����ϴ�!" << std::endl;

        // �������� ���� �÷��̾�� �⺻ �ൿ ó��
        for (int i = 0; i < 4; ++i) {
            if (!players[i].hasSelected) {
                players[i].action = "�⺻ ����";
                std::cout << "[�ڵ� ����] " << players[i].playerName << " -> �⺻ ����" << std::endl;
            }
        }
    }
    else {
        std::cout << "��� �� ������ �߻��߽��ϴ�!" << std::endl;
    }

    // ���� ��� ���
    std::cout << "\n=== ���� ���� ===\n" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << players[i].playerName << "�� �ൿ: " << players[i].action << std::endl;
        Sleep(500);
    }

    std::cout << "\n������ �Ϸ�Ǿ����ϴ�!" << std::endl;

    // �ڵ� ����
    for (int i = 0; i < 4; ++i) {
        CloseHandle(hThreads[i]);
    }

    return 0;
}