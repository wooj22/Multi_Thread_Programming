
/*

    과제 1: 기본 - 플레이어 체력 회복 시스템
    `_beginthreadex`와 `WaitForSingleObject`의 기본 사용법을 익히고, 단일 스레드의 생명주기를 이해한다.

    문제: 게임에서 플레이어가 데미지를 받은 후 시간이 지나면서 체력이 자동으로 회복되는 시스템을 구현하세요.

    요구사항
    - `_beginthreadex`로 체력 회복 스레드 생성
    - 메인 스레드에서 `WaitForSingleObject`로 회복 완료 대기
    - 체력이 100%가 되면 스레드 종료

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

    std::cout << "체력 회복 시작! 현재 체력: " << player->health << "/" << player->maxHealth << std::endl;

    while (player->health < player->maxHealth) {
        Sleep(1000); // 1초 대기
        player->health += 10; // 10씩 회복
        if (player->health > player->maxHealth) {
            player->health = player->maxHealth;
        }

        std::cout << "체력 회복 중... " << player->health << "/" << player->maxHealth << std::endl;
    }

    player->isHealing = false;
    std::cout << "체력이 완전히 회복되었습니다!" << std::endl;
    return 0;
}

int main()
{
    PlayerData player = { 30, 100, true };

    // 스레드 생성
    HANDLE hThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        HealthRecoveryThread,
        &player,
        0,
        NULL
    );

    if (hThread == NULL) {
        std::cout << "스레드 생성 실패!" << std::endl;
        return -1;
    }

    std::cout << "메인 게임 루프 실행 중..." << std::endl;

    // Q. 체력 회복 완료까지 대기
    WaitForSingleObject(hThread, INFINITE);

    std::cout << "체력 회복이 완료되어 게임을 계속할 수 있습니다!" << std::endl;

    CloseHandle(hThread);
    return 0;
}