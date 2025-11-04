
/*
	[Lock-Free Object Pool using SLIST]
*/

#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>

#define CONTAINING_RECORD(address, type, field) \
    ((type *)((char*)(address) - (size_t)offsetof(type, field)))

// SLIST Entry
typedef struct Player
{
	SLIST_ENTRY ItemEntry;		// SLIST EntryÀÇ ÇÊ¼ö ¸â¹ö
	int id;
};

// SLIST Lock-Free Ojbect Pool
class ObjectPool
{
private:
	PSLIST_HEADER pListHead;

public:
	ObjectPool(int initSize)
	{
		// Slist Header Allocation & Initialization
		pListHead = (PSLIST_HEADER)_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT);
		if (pListHead == NULL)
			std::cerr << "Memory allocation failed." << std::endl;
		InitializeSListHead(pListHead);

		// pool initialization
		for (int i = 0; i < initSize; ++i)
		{
			Player* pObj = (Player*)_aligned_malloc(sizeof(Player), MEMORY_ALLOCATION_ALIGNMENT);
			pObj->id = i;
			PushObject(pObj);
		}
	}

	~ObjectPool()
	{
		_aligned_free(pListHead);
	}

	void PushObject(Player* pObj)
	{
		InterlockedPushEntrySList(pListHead, &pObj->ItemEntry);
		printf("Push : %d\n", pObj->id);
	}

	Player* PopObject()
	{
		PSLIST_ENTRY pListEntry;
		if ((pListEntry = InterlockedPopEntrySList(pListHead)) != NULL)
		{
			Player* pObj = CONTAINING_RECORD(pListEntry, Player, ItemEntry);
			printf("Pop : %d\n", pObj->id);
			return pObj;
		}
		else
		{
			printf("Pool is empty!\n");
			return nullptr;
		}
	}

	void PoPAllObject()
	{
		PSLIST_ENTRY pListEntry;
		while ((pListEntry = InterlockedPopEntrySList(pListHead)) != NULL)
		{
			Player* pObj = CONTAINING_RECORD(pListEntry, Player, ItemEntry);
			printf("PoP : %d\n", pObj->id);
		}
	}
};

int main()
{
	// object pool
	ObjectPool pool(10);

	// thread
	std::thread producer([&pool]() {
		for (int i = 0; i < 10; ++i)
		{
			Player* p = new Player(); p->id = 10 + i;
			if (p) pool.PushObject(p);
			Sleep(100);
		}
		});

	std::thread consumer([&pool]() {
		while (Player* p = pool.PopObject())
			{
				Sleep(100);
			}
		});


	producer.join();
	consumer.join();

	return 0;
}