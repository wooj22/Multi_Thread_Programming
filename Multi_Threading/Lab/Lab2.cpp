/*

	���� 2: �߰� - ���� NPC �۾� ���� �ý���
	���� �����带 ���ÿ� �����ϰ� `WaitForMultipleObjects`�� `bWaitAll = TRUE` �ɼ��� ����Ͽ� ��� �۾� �ϷḦ ����ϴ� ����� �н��Ѵ�.

	����: ���ӿ��� ���� NPC�� ���ÿ� ���� �ٸ� �۾�(���� �, ���, ����)�� �����ϴ� �ý����� �����ϼ���.

	�䱸����:
	- 3���� NPC �����带 `_beginthreadex`�� ����
	- `WaitForMultipleObjects`�� ��� NPC �۾� �Ϸ� ���
	- �� NPC�� ���� �ٸ� �ð��� �۾� �Ϸ�

*/

#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>

struct NPCData 
{
    std::string name;
    std::string job;
    int workTime;
};

unsigned __stdcall NPCWorkThread(void* param)
{
    // Q. 
    NPCData* npc = static_cast<NPCData*>(param);
    std::cout << npc->name << "��(��) " << npc->job << " �۾��� �����մϴ�." << std::endl;

    for (int i = 1; i <= npc->workTime; ++i) {
        Sleep(1000);
        std::cout << npc->name << ": " << npc->job << " ���� ��... ("
            << i << "/" << npc->workTime << ")" << std::endl;
    }

    std::cout << npc->name << "��(��) " << npc->job << " �۾��� �Ϸ��߽��ϴ�!" << std::endl;
    return 0;
}

int main() {
    // NPC ������ ����
    NPCData npcs[3] = {
        {"���� ��", "���� �", 3},
        {"��� ��", "���� ���", 5},
        {"�������� ����", "���� ����", 4}
    };

    HANDLE hThreads[3];

    // Q. ��� NPC ������ ����
    for (int i = 0; i < 3; ++i) {
        hThreads[i] = (HANDLE)_beginthreadex(
            NULL,
            0,
            NPCWorkThread,
            &npcs[i],
            0,
            NULL
        );

        if (hThreads[i] == NULL) {
            std::cout << "������ " << i << " ���� ����!" << std::endl;
            return -1;
        }
    }

    std::cout << "��� NPC�� �۾��� �����߽��ϴ�. �ϷḦ ��ٸ��� ��..." << std::endl;

    // Q. ��� NPC �۾� �Ϸ���� ���
    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);
    std::cout << "\n��� NPC �۾��� �Ϸ�Ǿ����ϴ�! ������ ���� ��˴ϴ�." << std::endl;

    // �ڵ� ����
    for (int i = 0; i < 3; ++i) {
        CloseHandle(hThreads[i]);
    }

    return 0;
}