// ChatClient.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "ChatClient.h"

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <process.h>

// 제공된 헤더
#include "PacketID.h"
#include "ErrorCode.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32.lib")

using namespace NCommon;

// --- 상수 정의 ---
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 968
#define WM_APP_RECV_PACKET (WM_APP + 1) // 스레드로부터 패킷 수신 알림

// --- UI 컨트롤 ID ---
#define IDC_EDIT_IP 1001
#define IDC_EDIT_ID 1002
#define IDC_EDIT_PW 1003
#define IDC_BTN_CONNECT 1004
#define IDC_BTN_DISCONNECT 1005 // 새로 추가

#define IDC_EDIT_LOBBY_ID 1010
#define IDC_BTN_LOBBY_ENTER 1011
#define IDC_BTN_LOBBY_LEAVE 1012
#define IDC_LIST_LOBBY_CHAT 1013
#define IDC_EDIT_LOBBY_CHAT 1014
#define IDC_BTN_LOBBY_SEND 1015

#define IDC_EDIT_ROOM_ID 1020
#define IDC_BTN_ROOM_ENTER 1021
#define IDC_BTN_ROOM_LEAVE 1022
#define IDC_LIST_ROOM_CHAT 1023
#define IDC_EDIT_ROOM_CHAT 1024
#define IDC_BTN_ROOM_SEND 1025

#define IDC_LIST_ROOM_USERS 1030
#define IDC_LIST_LOG 1031 // 시스템 로그


const UINT16 SERVER_PORT = 32452;

// --- 전역 변수 ---
HINSTANCE g_hInst;
HWND g_hMainWnd;
bool g_bRunning = false; // 프로그램 실행 상태

// UI 컨트롤 핸들
HWND g_hEditIP, g_hEditID, g_hEditPW, g_hBtnConnect, g_hBtnDisconnect; // Disconnect 추가
HWND g_hEditLobbyID, g_hBtnLobbyEnter, g_hBtnLobbyLeave, g_hListLobbyChat, g_hEditLobbyChat, g_hBtnLobbySend;
HWND g_hEditRoomID, g_hBtnRoomEnter, g_hBtnRoomLeave, g_hListRoomChat, g_hEditRoomChat, g_hBtnRoomSend;
HWND g_hListRoomUsers, g_hListLog;

// 네트워크 및 스레드
SOCKET g_Socket = INVALID_SOCKET; // INVALID_SOCKET으로 초기화
HANDLE g_hSendThread = NULL;
HANDLE g_hRecvThread = NULL;
DWORD g_dwSendThreadID;
DWORD g_dwRecvThreadID;

// 스레드 안전 큐
std::queue<std::vector<char>> g_SendQueue;
CRITICAL_SECTION g_csSendQueue;

std::queue<std::vector<char>> g_RecvQueue;
CRITICAL_SECTION g_csRecvQueue;

// --- 함수 프로토타입 ---
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateUI(HWND hWnd);
void AddLog(const wchar_t* pMsg);
void ProcessPacket(char* pPacket);
bool ConnectToServer(const char* pszIP, unsigned short port);
void DisconnectFromServer();

unsigned WINAPI SendThread(void* lpParam);
unsigned WINAPI RecvThread(void* lpParam);

// --- 헬퍼 함수 ---

// 로그 리스트박스에 메시지 추가
void AddLog(const wchar_t* pMsg)
{
	SendMessage(g_hListLog, LB_ADDSTRING, 0, (LPARAM)pMsg);
	// 자동 스크롤
	int nCount = (int)SendMessage(g_hListLog, LB_GETCOUNT, 0, 0);
	SendMessage(g_hListLog, LB_SETTOPINDEX, nCount - 1, 0);
}

// 메인 스레드에서 패킷을 송신 큐에 추가
template<typename T>
void PostSendPacket(PACKET_ID id, const T& pktBody)
{
	if (!g_bRunning) return; // 연결 중이 아닐 때 전송 방지

	short totalSize = sizeof(PktHeader) + sizeof(T);
	std::vector<char> sendBuffer(totalSize);

	PktHeader header;
	header.TotalSize = totalSize;
	header.Id = (short)id;
	header.Reserve = 0;

	memcpy(sendBuffer.data(), &header, sizeof(PktHeader));
	memcpy(sendBuffer.data() + sizeof(PktHeader), &pktBody, sizeof(T));

	EnterCriticalSection(&g_csSendQueue);
	g_SendQueue.push(sendBuffer);
	LeaveCriticalSection(&g_csSendQueue);
}

// 바디가 없는 패킷용
void PostSendPacket(PACKET_ID id)
{
	if (!g_bRunning) return; // 연결 중이 아닐 때 전송 방지

	short totalSize = sizeof(PktHeader);
	std::vector<char> sendBuffer(totalSize);

	PktHeader header;
	header.TotalSize = totalSize;
	header.Id = (short)id;
	header.Reserve = 0;

	memcpy(sendBuffer.data(), &header, sizeof(PktHeader));

	EnterCriticalSection(&g_csSendQueue);
	g_SendQueue.push(sendBuffer);
	LeaveCriticalSection(&g_csSendQueue);
}

// --- WinMain ---
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	g_hInst = hInstance;
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"ChatClient";
	RegisterClassExW(&wcex);

	g_hMainWnd = CreateWindowW(L"ChatClient", L"Win32 Chat Client", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

	if (!g_hMainWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hMainWnd, nCmdShow);
	UpdateWindow(g_hMainWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// --- 윈도우 프로시저 ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		// Winsock 초기화
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			MessageBox(hWnd, L"WSAStartup 실패", L"오류", MB_OK);
			return -1;
		}

		// 큐 동기화 객체 초기화
		InitializeCriticalSection(&g_csSendQueue);
		InitializeCriticalSection(&g_csRecvQueue);

		// UI 생성
		CreateUI(hWnd);
		AddLog(L"클라이언트 초기화 완료. 서버에 접속하세요.");
	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
			// --- 로그인/연결 ---
		case IDC_BTN_CONNECT:
		{
			if (g_bRunning)
			{
				AddLog(L"이미 연결되어 있습니다.");
				break;
			}

			char szIP[32];
			char szID[MAX_USER_ID_SIZE + 1];
			char szPW[MAX_USER_PASSWORD_SIZE + 1];

			GetWindowTextA(g_hEditIP, szIP, 32);
			GetWindowTextA(g_hEditID, szID, MAX_USER_ID_SIZE);
			GetWindowTextA(g_hEditPW, szPW, MAX_USER_PASSWORD_SIZE);

			if (ConnectToServer(szIP, SERVER_PORT)) // 포트는 임의로 11021로 지정
			{
				AddLog(L"서버 접속 성공. 스레드 시작.");
				g_bRunning = true;
				g_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, NULL, 0, (unsigned*)&g_dwSendThreadID);
				g_hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, NULL, 0, (unsigned*)&g_dwRecvThreadID);
				
				// 로그인 패킷 전송
				PktLogInReq req;
				strcpy_s(req.szID, szID);
				strcpy_s(req.szPW, szPW);
				PostSendPacket(PACKET_ID::LOGIN_IN_REQ, req);
				AddLog(L"로그인 요청 전송.");
			}
			else
			{
				AddLog(L"서버 접속 실패.");
			}
		}
		break;

		// --- 연결 끊기 ---
		case IDC_BTN_DISCONNECT:
		{
			if (g_bRunning)
			{
				AddLog(L"서버와 연결을 종료합니다.");
				DisconnectFromServer();
				AddLog(L"연결 종료 완료.");
			}
			else
			{
				AddLog(L"연결되어 있지 않습니다.");
			}
		}
		break;

		// --- 로비 ---
		case IDC_BTN_LOBBY_ENTER:
		{
			PktLobbyEnterReq req;
			req.LobbyId = GetDlgItemInt(hWnd, IDC_EDIT_LOBBY_ID, NULL, FALSE);
			PostSendPacket(PACKET_ID::LOBBY_ENTER_REQ, req);
			AddLog(L"로비 입장 요청.");
		}
		break;
		case IDC_BTN_LOBBY_LEAVE:
		{
			PostSendPacket(PACKET_ID::LOBBY_LEAVE_REQ);
			AddLog(L"로비 나가기 요청.");
		}
		break;
		case IDC_BTN_LOBBY_SEND:
		{
			PktLobbyChatReq req;
			GetWindowTextW(g_hEditLobbyChat, req.Msg, MAX_LOBBY_CHAT_MSG_SIZE);
			PostSendPacket(PACKET_ID::LOBBY_CHAT_REQ, req);
			SetWindowTextW(g_hEditLobbyChat, L""); // 입력창 비우기
		}
		break;

		// --- 방 ---
		case IDC_BTN_ROOM_ENTER:
		{
			PktRoomEnterReq req;
			req.IsCreate = true; // UI 단순화를 위해 '만들기'로 고정
			req.RoomIndex = GetDlgItemInt(hWnd, IDC_EDIT_ROOM_ID, NULL, FALSE);
			PostSendPacket(PACKET_ID::ROOM_ENTER_REQ, req);
			AddLog(L"방 입장 요청.");
		}
		break;
		case IDC_BTN_ROOM_LEAVE:
		{
			PostSendPacket(PACKET_ID::ROOM_LEAVE_REQ);
			AddLog(L"방 나가기 요청.");
		}
		break;
		case IDC_BTN_ROOM_SEND:
		{
			PktRoomChatReq req;
			GetWindowTextW(g_hEditRoomChat, req.Msg, MAX_ROOM_CHAT_MSG_SIZE);
			PostSendPacket(PACKET_ID::ROOM_CHAT_REQ, req);
			SetWindowTextW(g_hEditRoomChat, L""); // 입력창 비우기
		}
		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	// Recv 스레드로부터 패킷 도착 알림
	case WM_APP_RECV_PACKET:
	{
		std::queue<std::vector<char>> tempQueue;
		EnterCriticalSection(&g_csRecvQueue);
		g_RecvQueue.swap(tempQueue);
		LeaveCriticalSection(&g_csRecvQueue);

		while (!tempQueue.empty())
		{
			std::vector<char>& pktData = tempQueue.front();
			if (pktData.empty()) // 접속 끊김 시그널
			{
				if (g_bRunning) // g_bRunning은 스레드에서도 false로 바뀔 수 있음
				{
					AddLog(L"서버와 접속이 끊겼습니다.");
					DisconnectFromServer(); // 접속 끊김 처리
				}
			}
			else
			{
				ProcessPacket(pktData.data());
			}
			tempQueue.pop();
		}
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		DisconnectFromServer(); // 프로그램 종료 시 정리

		DeleteCriticalSection(&g_csSendQueue);
		DeleteCriticalSection(&g_csRecvQueue);
		WSACleanup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// --- UI 생성 ---
void CreateUI(HWND hWnd)
{
	int x = 10, y = 10;

	// --- 로그인 그룹 --- (x: 10, y: 10)
	CreateWindowW(L"STATIC", L"서버 IP:", WS_CHILD | WS_VISIBLE, x, y, 60, 20, hWnd, NULL, g_hInst, NULL);
	g_hEditIP = CreateWindowW(L"EDIT", L"127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x + 65, y, 120, 20, hWnd, (HMENU)IDC_EDIT_IP, g_hInst, NULL);

	CreateWindowW(L"STATIC", L"ID:", WS_CHILD | WS_VISIBLE, x + 200, y, 30, 20, hWnd, NULL, g_hInst, NULL);
	g_hEditID = CreateWindowW(L"EDIT", L"test1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x + 235, y, 80, 20, hWnd, (HMENU)IDC_EDIT_ID, g_hInst, NULL);

	CreateWindowW(L"STATIC", L"PW:", WS_CHILD | WS_VISIBLE, x + 330, y, 30, 20, hWnd, NULL, g_hInst, NULL);
	g_hEditPW = CreateWindowW(L"EDIT", L"1234", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD, x + 365, y, 80, 20, hWnd, (HMENU)IDC_EDIT_PW, g_hInst, NULL);

	// 버튼 이름 변경 및 '연결 끊기' 버튼 추가
	g_hBtnConnect = CreateWindowW(L"BUTTON", L"연결/로그인", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 460, y, 90, 20, hWnd, (HMENU)IDC_BTN_CONNECT, g_hInst, NULL);
	g_hBtnDisconnect = CreateWindowW(L"BUTTON", L"연결 끊기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 555, y, 80, 20, hWnd, (HMENU)IDC_BTN_DISCONNECT, g_hInst, NULL);


	y += 40;

	// --- 로비 그룹 --- (x: 10, y: 50, w: 400)
	CreateWindowW(L"STATIC", L"로비 ID:", WS_CHILD | WS_VISIBLE, x, y, 60, 20, hWnd, NULL, g_hInst, NULL);
	g_hEditLobbyID = CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER, x + 65, y, 50, 20, hWnd, (HMENU)IDC_EDIT_LOBBY_ID, g_hInst, NULL);
	g_hBtnLobbyEnter = CreateWindowW(L"BUTTON", L"입장", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y, 60, 20, hWnd, (HMENU)IDC_BTN_LOBBY_ENTER, g_hInst, NULL);
	g_hBtnLobbyLeave = CreateWindowW(L"BUTTON", L"나가기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 185, y, 60, 20, hWnd, (HMENU)IDC_BTN_LOBBY_LEAVE, g_hInst, NULL);

	y += 30;
	g_hListLobbyChat = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, x, y, 400, 300, hWnd, (HMENU)IDC_LIST_LOBBY_CHAT, g_hInst, NULL);
	y += 310;
	g_hEditLobbyChat = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x, y, 335, 20, hWnd, (HMENU)IDC_EDIT_LOBBY_CHAT, g_hInst, NULL);
	g_hBtnLobbySend = CreateWindowW(L"BUTTON", L"전송", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 340, y, 60, 20, hWnd, (HMENU)IDC_BTN_LOBBY_SEND, g_hInst, NULL);

	// --- 방 그룹 --- (x: 420, y: 50, w: 400)
	x = 420; y = 50;
	CreateWindowW(L"STATIC", L"방 ID:", WS_CHILD | WS_VISIBLE, x, y, 60, 20, hWnd, NULL, g_hInst, NULL);
	g_hEditRoomID = CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER, x + 65, y, 50, 20, hWnd, (HMENU)IDC_EDIT_ROOM_ID, g_hInst, NULL);
	g_hBtnRoomEnter = CreateWindowW(L"BUTTON", L"입장", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y, 60, 20, hWnd, (HMENU)IDC_BTN_ROOM_ENTER, g_hInst, NULL);
	g_hBtnRoomLeave = CreateWindowW(L"BUTTON", L"나가기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 185, y, 60, 20, hWnd, (HMENU)IDC_BTN_ROOM_LEAVE, g_hInst, NULL);

	y += 30;
	g_hListRoomChat = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, x, y, 400, 300, hWnd, (HMENU)IDC_LIST_ROOM_CHAT, g_hInst, NULL);
	y += 310;
	g_hEditRoomChat = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x, y, 335, 20, hWnd, (HMENU)IDC_EDIT_ROOM_CHAT, g_hInst, NULL);
	g_hBtnRoomSend = CreateWindowW(L"BUTTON", L"전송", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 340, y, 60, 20, hWnd, (HMENU)IDC_BTN_ROOM_SEND, g_hInst, NULL);

	// --- 방 유저 리스트 --- (x: 830, y: 50)
	x = 830; y = 50;
	CreateWindowW(L"STATIC", L"방 유저", WS_CHILD | WS_VISIBLE, x, y, 150, 20, hWnd, NULL, g_hInst, NULL);
	y += 30;
	g_hListRoomUsers = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, x, y, 150, 330, hWnd, (HMENU)IDC_LIST_ROOM_USERS, g_hInst, NULL);

	// --- 시스템 로그 --- (y: 450)
	x = 10; y = 450;
	CreateWindowW(L"STATIC", L"시스템 로그", WS_CHILD | WS_VISIBLE, x, y, 150, 20, hWnd, NULL, g_hInst, NULL);
	y += 30;
	g_hListLog = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, x, y, WINDOW_WIDTH - 40, 400, hWnd, (HMENU)IDC_LIST_LOG, g_hInst, NULL);
}

// --- 네트워크 함수 ---
bool ConnectToServer(const char* pszIP, unsigned short port)
{
	g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_Socket == INVALID_SOCKET) {
		AddLog(L"socket() 실패");
		return false;
	}

	SOCKADDR_IN serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, pszIP, &serverAddr.sin_addr);

	if (connect(g_Socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		AddLog(L"connect() 실패");
		closesocket(g_Socket);
		g_Socket = INVALID_SOCKET;
		return false;
	}

	return true;
}

// 접속 종료 처리 함수 (메인 스레드에서만 호출)
void DisconnectFromServer()
{
	if (g_bRunning == false && g_Socket == INVALID_SOCKET)
	{
		return; // 이미 정리됨
	}

	g_bRunning = false; // 스레드 루프 중지 플래그

	if (g_Socket != INVALID_SOCKET)
	{
		shutdown(g_Socket, SD_BOTH); // 스레드 블로킹(recv) 해제
		closesocket(g_Socket);
		g_Socket = INVALID_SOCKET;
	}

	// 스레드가 스스로 종료될 때까지 대기
	if (g_hRecvThread != NULL)
	{
		WaitForSingleObject(g_hRecvThread, 1000);
		CloseHandle(g_hRecvThread);
		g_hRecvThread = NULL;
	}
	if (g_hSendThread != NULL)
	{
		WaitForSingleObject(g_hSendThread, 1000);
		CloseHandle(g_hSendThread);
		g_hSendThread = NULL;
	}

	// 큐 비우기
	EnterCriticalSection(&g_csSendQueue);
	std::queue<std::vector<char>> emptySend;
	std::swap(g_SendQueue, emptySend);
	LeaveCriticalSection(&g_csSendQueue);

	EnterCriticalSection(&g_csRecvQueue);
	std::queue<std::vector<char>> emptyRecv;
	std::swap(g_RecvQueue, emptyRecv);
	LeaveCriticalSection(&g_csRecvQueue);
}


// --- 스레드 함수 ---

// 송신 스레드
unsigned WINAPI SendThread(void* lpParam)
{
	std::queue<std::vector<char>> tempQueue;

	while (g_bRunning)
	{
		Sleep(64); // 64ms 마다 실행

		EnterCriticalSection(&g_csSendQueue);
		if (g_SendQueue.empty())
		{
			LeaveCriticalSection(&g_csSendQueue);
			continue;
		}
		g_SendQueue.swap(tempQueue);
		LeaveCriticalSection(&g_csSendQueue);

		while (!tempQueue.empty())
		{
			std::vector<char>& pktData = tempQueue.front();
			int nSent = send(g_Socket, pktData.data(), (int)pktData.size(), 0);

			if (nSent == SOCKET_ERROR)
			{
				// 메인 스레드에 접속 끊김 알림 (빈 벡터 전송)
				EnterCriticalSection(&g_csRecvQueue);
				g_RecvQueue.push(std::vector<char>());
				LeaveCriticalSection(&g_csRecvQueue);
				PostMessage(g_hMainWnd, WM_APP_RECV_PACKET, 0, 0);

				g_bRunning = false; // 스레드 종료 플래그
				break;
			}
			tempQueue.pop();
		}
	}
	return 0;
}

// 수신 스레드
unsigned WINAPI RecvThread(void* lpParam)
{
	char recvBuffer[8192];
	int nBufferPos = 0; // 버퍼에 남아있는 데이터 크기

	while (g_bRunning)
	{
		int nRecv = recv(g_Socket, recvBuffer + nBufferPos, 8192 - nBufferPos, 0);

		if (nRecv == SOCKET_ERROR || nRecv == 0)
		{
			// 접속 끊김
			EnterCriticalSection(&g_csRecvQueue);
			g_RecvQueue.push(std::vector<char>()); // 빈 벡터로 접속 끊김 시그널 전송
			LeaveCriticalSection(&g_csRecvQueue);
			PostMessage(g_hMainWnd, WM_APP_RECV_PACKET, 0, 0);

			g_bRunning = false; // 스레드 종료 플래그
			break;
		}

		nBufferPos += nRecv;
		int nProcessPos = 0;

		while (nBufferPos >= sizeof(PktHeader))
		{
			PktHeader* pHeader = (PktHeader*)(recvBuffer + nProcessPos);
			short nTotalSize = pHeader->TotalSize;

			if (nTotalSize <= 0 || nTotalSize > 8192) // 비정상 패킷 크기 방어
			{
				g_bRunning = false; // 연결 끊음 처리
				EnterCriticalSection(&g_csRecvQueue);
				g_RecvQueue.push(std::vector<char>());
				LeaveCriticalSection(&g_csRecvQueue);
				PostMessage(g_hMainWnd, WM_APP_RECV_PACKET, 0, 0);
				break;
			}

			if (nBufferPos >= nTotalSize) // 완전한 패킷이 도착
			{
				std::vector<char> pktData(nTotalSize);
				memcpy(pktData.data(), recvBuffer + nProcessPos, nTotalSize);

				EnterCriticalSection(&g_csRecvQueue);
				g_RecvQueue.push(pktData);
				LeaveCriticalSection(&g_csRecvQueue);

				PostMessage(g_hMainWnd, WM_APP_RECV_PACKET, 0, 0);

				nProcessPos += nTotalSize;
				nBufferPos -= nTotalSize;
			}
			else
			{
				break;
			}
		}

		if (!g_bRunning) break; // 비정상 패킷으로 루프 종료 시

		if (nBufferPos > 0 && nProcessPos > 0)
		{
			memmove(recvBuffer, recvBuffer + nProcessPos, nBufferPos);
		}
		else if (nBufferPos <= 0) // 버퍼가 깨끗하게 비워진 경우
		{
			nBufferPos = 0;
		}
	}
	return 0;
}


// --- 패킷 처리 (메인 스레드에서 실행) ---
void ProcessPacket(char* pPacket)
{
	PktHeader* pHeader = (PktHeader*)pPacket;
	void* pBody = pPacket + sizeof(PktHeader);
	PACKET_ID id = (PACKET_ID)pHeader->Id;

	wchar_t szLog[512]; // 로그용 버퍼
	wchar_t szMsg[512]; // UI용 버퍼

	switch (id)
	{
	case PACKET_ID::LOGIN_IN_RES:
	{
		PktLogInRes* pRes = (PktLogInRes*)pBody;
		if (pRes->ErrorCode == (short)ERROR_CODE::NONE) {
			AddLog(L"로그인 성공");
		}
		else {
			swprintf_s(szLog, L"로그인 실패 (에러: %d)", pRes->ErrorCode);
			AddLog(szLog);
		}
	}
	break;

	case PACKET_ID::LOBBY_ENTER_RES:
	{
		PktLobbyEnterRes* pRes = (PktLobbyEnterRes*)pBody;
		if (pRes->ErrorCode == (short)ERROR_CODE::NONE) {
			AddLog(L"로비 입장 성공");
		}
		else {
			swprintf_s(szLog, L"로비 입장 실패 (에러: %d)", pRes->ErrorCode);
			AddLog(szLog);
		}
	}
	break;

	case PACKET_ID::LOBBY_CHAT_NTF:
	{
		PktLobbyChatNtf* pNtf = (PktLobbyChatNtf*)pBody;
		swprintf_s(szMsg, L"[%S]: %s", pNtf->UserID, pNtf->Msg); // %S : char* -> wchar_t
		SendMessage(g_hListLobbyChat, LB_ADDSTRING, 0, (LPARAM)szMsg);
	}
	break;

	case PACKET_ID::ROOM_ENTER_RES:
	{
		PktRoomEnterRes* pRes = (PktRoomEnterRes*)pBody;
		if (pRes->ErrorCode == (short)ERROR_CODE::NONE) {
			AddLog(L"방 입장 성공");
			SendMessage(g_hListRoomUsers, LB_RESETCONTENT, 0, 0); // 유저 리스트 초기화
		}
		else {
			swprintf_s(szLog, L"방 입장 실패 (에러: %d)", pRes->ErrorCode);
			AddLog(szLog);
		}
	}
	break;

	case PACKET_ID::ROOM_ENTER_NEW_USER_NTF:
	{
		PktRoomEnterUserInfoNtf* pNtf = (PktRoomEnterUserInfoNtf*)pBody;
		swprintf_s(szMsg, L"%S", pNtf->UserID); // %S : char* -> wchar_t
		SendMessage(g_hListRoomUsers, LB_ADDSTRING, 0, (LPARAM)szMsg);
	}
	break;

	case PACKET_ID::ROOM_LEAVE_USER_NTF:
	{
		PktRoomLeaveUserInfoNtf* pNtf = (PktRoomLeaveUserInfoNtf*)pBody;
		swprintf_s(szMsg, L"%S", pNtf->UserID);

		LRESULT nIndex = SendMessage(g_hListRoomUsers, LB_FINDSTRINGEXACT, -1, (LPARAM)szMsg);
		if (nIndex != LB_ERR) {
			SendMessage(g_hListRoomUsers, LB_DELETESTRING, nIndex, 0);
		}
	}
	break;

	case PACKET_ID::ROOM_CHAT_NTF:
	{
		PktRoomChatNtf* pNtf = (PktRoomChatNtf*)pBody;
		swprintf_s(szMsg, L"[%S]: %s", pNtf->UserID, pNtf->Msg);
		SendMessage(g_hListRoomChat, LB_ADDSTRING, 0, (LPARAM)szMsg);
	}
	break;

	// --- 기타 패킷 응답 (로그만 출력) ---
	case PACKET_ID::LOBBY_LEAVE_RES:
		AddLog(L"로비 나가기 완료");
		break;
	case PACKET_ID::ROOM_LEAVE_RES:
		AddLog(L"방 나가기 완료");
		SendMessage(g_hListRoomUsers, LB_RESETCONTENT, 0, 0);
		break;
	case PACKET_ID::LOBBY_CHAT_RES:
		AddLog(L"로비 채팅 전송 응답");
		break;
	case PACKET_ID::ROOM_CHAT_RES:
		AddLog(L"방 채팅 전송 응답");
		break;

	default:
		swprintf_s(szLog, L"알 수 없는 패킷 ID 수신: %d", (short)id);
		AddLog(szLog);
		break;
	}
}