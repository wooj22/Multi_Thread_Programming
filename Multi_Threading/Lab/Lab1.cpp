
/*

    ���� 1: �⺻ - �÷��̾� ü�� ȸ�� �ý���
    `_beginthreadex`�� `WaitForSingleObject`�� �⺻ ������ ������, ���� �������� �����ֱ⸦ �����Ѵ�.

    ����: ���ӿ��� �÷��̾ �������� ���� �� �ð��� �����鼭 ü���� �ڵ����� ȸ���Ǵ� �ý����� �����ϼ���.

    �䱸����
    - `_beginthreadex`�� ü�� ȸ�� ������ ����
    - ���� �����忡�� `WaitForSingleObject`�� ȸ�� �Ϸ� ���
    - ü���� 100%�� �Ǹ� ������ ����

*/


#include <iostream>
#include <windows.h>
#include <process.h>

struct PlayerData
{
    int health;
    int maxHealth;
    bool isHealing;
};

unsigned __stdcall HealthRecoveryThread(void* param)
{
    PlayerData* player = (PlayerData*)param;

    std::cout << "ü�� ȸ�� ����! ���� ü��: " << player->health << "/" << player->maxHealth << std::endl;

    while (player->health < player->maxHealth) {
        Sleep(1000); // 1�� ���
        player->health += 10; // 10�� ȸ��
        if (player->health > player->maxHealth) {
            player->health = player->maxHealth;
        }

        std::cout << "ü�� ȸ�� ��... " << player->health << "/" << player->maxHealth << std::endl;
    }

    player->isHealing = false;
    std::cout << "ü���� ������ ȸ���Ǿ����ϴ�!" << std::endl;
    return 0;
}

int main()
{
    PlayerData player = { 30, 100, true };

    // ������ ����
    HANDLE hThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        HealthRecoveryThread,
        &player,
        0,
        NULL
    );

    if (hThread == NULL) {
        std::cout << "������ ���� ����!" << std::endl;
        return -1;
    }

    std::cout << "���� ���� ���� ���� ��..." << std::endl;

    // Q. ü�� ȸ�� �Ϸ���� ���
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "ü�� ȸ���� �Ϸ�Ǿ� ������ ����� �� �ֽ��ϴ�!" << std::endl;

    CloseHandle(hThread);
    return 0;
}