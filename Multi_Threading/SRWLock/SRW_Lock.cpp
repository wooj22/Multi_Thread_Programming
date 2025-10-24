
/*
	[ SRWLOCK (Slim Reader/Writer Lock) ]
	
	���� Reader/Writer ����ȭ ����
	�б�� ���� �۾��� �����Ͽ� ���� �����尡 ���ÿ� �б� ������ �� �ֵ��� �ϴ� ����ȭ ��Ŀ�������� 
	���� �����尡 ���ÿ� ���� �� �ֵ��� ����ϸ鼭�� ���� �۾��� ��Ÿ������ ����ǵ��� �����ϴ� ��ü

	- SRWLOCK�� �ſ� ������ Ŀ�� ������Ʈ�� �������� �ʱ� ������ ������ �߿��� ������ ���� ����
	- �� ���� ���� Reader�� ���� ����, Writer�� �ܵ� ���ٸ� ����
	- �����尡 ���� ȹ������ ���ϸ� Ŀ�η� ������ ����� �� ����
	- SRWLOCK�� ���� �����尡 ���϶��� ��ø ȹ��(AcquireSRWLockExclusiv ȣ��)�ϸ� �������°� �߻���

	- �б� �۾�
		AcquireSRWLockShared(&lock);
		// �б� �۾� ����, ���� ������ ���� ����
		ReleaseSRWLockShared(&lock);

	- ���� �۾�
		AcquireSRWLockExclusive(&lock);
		// ���� �۾� ����, �� �����常 ���� ����
		ReleaseSRWLockExclusive(&lock);

	- ������ �õ� : ���� ȹ���� �� �ִ����� �̸� Ȯ���� �� �ִ�.
	  if(TryAcquireSRWLockShared(&lock))	// �б� ����
	  if(TryAcquireSRWLockExclusive(&lock)) // ���� ����
*/

#include <windows.h>
#include <stdio.h>

// SRWLOCK
SRWLOCK lock;

// Reader
DWORD WINAPI ReaderThread(LPVOID param) 
{
	AcquireSRWLockShared(&lock);
	printf("Reader acquired lock\n");
	Sleep(1000);
	ReleaseSRWLockShared(&lock);
	return 0;
}

// Wirter
DWORD WINAPI WriterThread(LPVOID param) 
{
	AcquireSRWLockExclusive(&lock);
	printf("Writer acquired lock\n");
	Sleep(1000);
	ReleaseSRWLockExclusive(&lock);
	return 0;
}

int main() 
{
	InitializeSRWLock(&lock);

	HANDLE h1 = CreateThread(NULL, 0, ReaderThread, NULL, 0, NULL);
	HANDLE h2 = CreateThread(NULL, 0, WriterThread, NULL, 0, NULL);

	WaitForSingleObject(h1, INFINITE);
	WaitForSingleObject(h2, INFINITE);

	return 0;
}