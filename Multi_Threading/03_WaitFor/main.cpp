/*

	 [ WaitForSingleObject(�ڵ�, ��� �ð�) ]
	 [ WaitForMultipleObjects(�ڵ� ����, �ڵ� �迭, True(��簴ü)/False(�ϳ���), ��� �ð�) ]

	 ��ȯ ���� ���� �۾��� �� ��ġ�� �����尡 ����Ȱ���, ���߿� ����Ȱ��� �� �� ����
	 - WAIT_OBJECT_0 : ��ü�� ��ȣ ���°� ��
	 - WAIT_TIMEOU T: �ð� �ʰ�
	 - WAIT_FAILED : ���� �߻�

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <process.h>
#include <random>

// �۾� ������ ������ �Լ��� 
// �Ű������� �ѱ� ������ �Լ��� ��ȯ���� ������ unsigned�̴�.
unsigned __stdcall FastWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[�����۾� " << id << "] ����" << std::endl;
    Sleep(1000);
    std::cout << "[�����۾� " << id << "] �Ϸ�" << std::endl;
    return 100 + id;
}

unsigned __stdcall SlowWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[�����۾� " << id << "] ����" << std::endl;
    Sleep(3000);
    std::cout << "[�����۾� " << id << "] �Ϸ�" << std::endl;
    return 200 + id;
}

unsigned __stdcall UnpredictableWorker(void* param)
{
    int id = *static_cast<int*>(param);
    std::cout << "[�ұ�Ģ�۾� " << id << "] ����" << std::endl;

    // C++11 random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5);
    int sleepTime = dis(gen) * 1000;

    std::cout << "[�ұ�Ģ�۾� " << id << "] ���� �ð�: " << sleepTime / 1000 << "��" << std::endl;

    Sleep(sleepTime);
    std::cout << "[�ұ�Ģ�۾� " << id << "] �Ϸ�" << std::endl;
    return 300 + id;
}

// WaitForSingleObject - �ֱ��� ���� Ȯ��
void DemonstrateSingleObjectWait()
{
    std::cout << "=== WaitForSingleObject ===" << std::endl;

    int workerId = 1;
    unsigned threadId;

    // ������ ���� �� �۾� ����
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL,           
        0,              
        SlowWorker,     // ������ �Լ�
        &workerId,      // �Ű�����
        0,              
        &threadId       
    ));

    // �ڵ� ��ȿ�� �˻�
    if (hThread == NULL)
    {
        std::cout << "������ ���� ����! ���� �ڵ�: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "������ �۾� ��... 2�ʸ��� ���� Ȯ��" << std::endl;

    while (true)
    {
        // 2�� ��� �� hThread ���� ��ȯ
        DWORD waitResult = WaitForSingleObject(hThread, 2000);  

        switch (waitResult)
        {
        // WAIT_OBJECT_0 : ��ȣ ����
        case WAIT_OBJECT_0:
            std::cout << "������ �Ϸ�!" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "���� �ڵ�: " << exitCode << std::endl;
            }

            CloseHandle(hThread);
            return;
        
        // WAIT_OBJECT_0 : �ð� �ʰ�
        case WAIT_TIMEOUT:
            std::cout << "���� ���� ��... ��� ���" << std::endl;
            break;

        // WAIT_FAILED : ���� �߻�
        case WAIT_FAILED:
            std::cout << "��� �� ���� �߻�! ���� �ڵ�: " << GetLastError() << std::endl;
            CloseHandle(hThread);
            return;
        }
    }
}

// WaitForMultipleObjects - �Ϸ�Ǵ� ������ ���� Ȯ��
void DemonstrateMultipleObjectsWait()
{
    std::cout << "\n=== WaitForMultipleObjects ===" << std::endl;

    // ������ ���� �� �۾� ����
    int ids[] = { 1, 2, 3 };
    HANDLE threads[3];
    unsigned threadIds[3];

    threads[0] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, FastWorker, &ids[0], 0, &threadIds[0]));
    threads[1] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, SlowWorker, &ids[1], 0, &threadIds[1]));
    threads[2] = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, UnpredictableWorker, &ids[2], 0, &threadIds[2]));

    // �ڵ� ��ȿ�� �˻�
    bool allValid = true;
    for (int i = 0; i < 3; i++)
    {
        if (threads[i] == NULL)
        {
            std::cout << "������ " << i + 1 << " ���� ����! ���� �ڵ�: " << GetLastError() << std::endl;
            allValid = false;

            // �̹� ������ ������� ����
            for (int j = 0; j < i; j++)
            {
                CloseHandle(threads[j]);
            }
        }
    }
    if (!allValid) return;

    // �ϳ��� ����ɶ����� ���
    std::cout << "\n--- �ó����� 1 : ù ��° �Ϸ�Ǵ� ������ ��� ---" << std::endl;
    DWORD firstCompleted = WaitForMultipleObjects(3, threads, FALSE, INFINITE); 

    // ���� ù ��° ������(threads[0])�� ����Ǿ��ٸ�, WAIT_OBJECT_0 ���� ����
    // ���� �� ��° ������(threads[1])�� ����Ǿ��ٸ�, WAIT_OBJECT_0 + 1 ���� ���� 
    // ���� �� ��° ������(threads[2])�� ����Ǿ��ٸ�, WAIT_OBJECT_0 + 2 ���� ����
    if (firstCompleted >= WAIT_OBJECT_0 && firstCompleted < WAIT_OBJECT_0 + 3)
    {
        int completedIndex = firstCompleted - WAIT_OBJECT_0;
        std::cout << "ù ��° �Ϸ� : ������ " << completedIndex + 1 << std::endl;

        // �Ϸ�� �������� ���� �ڵ� Ȯ��
        DWORD exitCode;
        if (GetExitCodeThread(threads[completedIndex], &exitCode))
        {
            std::cout << "�Ϸ�� �������� ���� �ڵ�: " << exitCode << std::endl;
        }
    }

    // ��� �����尡 ����ɶ����� ���
    std::cout << "\n--- �ó����� 2 : ��� ������ �Ϸ� ��� ---" << std::endl;
    DWORD allCompleted = WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    if (allCompleted == WAIT_OBJECT_0)
    {
        std::cout << "��� ������ �Ϸ�!" << std::endl;

        // �� �������� ���� �ڵ� Ȯ��
        for (int i = 0; i < 3; i++)
        {
            DWORD exitCode;
            if (GetExitCodeThread(threads[i], &exitCode))
            {
                std::cout << "������ " << i + 1 << " ���� �ڵ�: " << exitCode << std::endl;
            }
        }
    }
    else if (allCompleted == WAIT_FAILED)
    {
        std::cout << "��� ������ ��� �� ���� �߻�! ���� �ڵ�: " << GetLastError() << std::endl;
    }

    // �ڵ� ����
    for (int i = 0; i < 3; i++)
    {
        CloseHandle(threads[i]);
    }
}

// WaitForSingleObject - Ÿ�� �ƿ�
void DemonstrateTimeoutWait()
{
    std::cout << "\n=== Ÿ�Ӿƿ� ó�� ���� ===" << std::endl;

    int workerId = 10;
    unsigned threadId;

    // ������ ���� �� �۾� ����
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL, 0, SlowWorker, &workerId, 0, &threadId
    ));

    // �ڵ� ��ȿ�� �˻�
    if (hThread == NULL)
    {
        std::cout << "������ ���� ����! ���� �ڵ�: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "2�� Ÿ�Ӿƿ����� ��� (3�� �۾��̹Ƿ� Ÿ�Ӿƿ� ����)" << std::endl;

    // 2�� ����� ������ ���� ��ȯ
    DWORD waitResult = WaitForSingleObject(hThread, 2000);

    switch (waitResult)
    {
    case WAIT_OBJECT_0:
        std::cout << "����� �޸� ���� �Ϸ��!" << std::endl;

        DWORD exitCode;
        if (GetExitCodeThread(hThread, &exitCode))
        {
            std::cout << "���� �ڵ�: " << exitCode << std::endl;
        }
        break;

    case WAIT_TIMEOUT:
        std::cout << "������ Ÿ�Ӿƿ� �߻�" << std::endl;
        std::cout << "����ڿ��� ���� ��Ȳ ���� �� ��� ���..." << std::endl;

        // ���� ���� ����
        if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
        {
            std::cout << "���������� �Ϸ��" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "���� �ڵ�: " << exitCode << std::endl;
            }
        }
        break;

    case WAIT_FAILED:
        std::cout << "��� ����! ���� �ڵ�: " << GetLastError() << std::endl;
        break;
    }

    CloseHandle(hThread);
}

// WaitForSingleObject - �ֱ��� ���� Ȯ��
void DemonstratePeriodicStatusCheck()
{
    std::cout << "\n=== �ֱ��� ���� Ȯ�� ���� ===" << std::endl;

    int workerId = 20;
    unsigned threadId;

    // ������ ���� �� �۾� ����
    HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL, 0, UnpredictableWorker, &workerId, 0, &threadId
    ));

    // ��� ��ȿ�� üũ
    if (hThread == NULL)
    {
        std::cout << "������ ���� ����!" << std::endl;
        return;
    }

    // 1�ʸ��� ������ ���� Ȯ��
    int checkCount = 0;
    while (true)
    {
        DWORD waitResult = WaitForSingleObject(hThread, 1000);  // 1�ʸ��� Ȯ��

        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            std::cout << "\n�۾� �Ϸ�! �� " << checkCount << "�� Ȯ����" << std::endl;

            DWORD exitCode;
            if (GetExitCodeThread(hThread, &exitCode))
            {
                std::cout << "���� �ڵ�: " << exitCode << std::endl;
            }

            CloseHandle(hThread);
            return;

        case WAIT_TIMEOUT:
            checkCount++;
            std::cout << "." << std::flush;  // ���� ��Ȳ ǥ��
            if (checkCount % 10 == 0)
            {
                std::cout << " (" << checkCount << "�� ���)" << std::endl;
            }
            break;

        case WAIT_FAILED:
            std::cout << "\n���� Ȯ�� ����!" << std::endl;
            CloseHandle(hThread);
            return;
        }
    }
}

int main()
{
    std::cout << "Windows ������ ��� �Լ� ���� (_beginthreadex ���)\n" << std::endl;

    // �� ���� �Լ� ����
    DemonstrateSingleObjectWait();
    DemonstrateMultipleObjectsWait();
    DemonstrateTimeoutWait();
    DemonstratePeriodicStatusCheck();

    std::cout << "\n��� ���� �Ϸ�!" << std::endl;
    return 0;
}