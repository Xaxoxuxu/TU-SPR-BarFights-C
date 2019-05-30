#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "pthread.h"

#define DRUNK_NAME_LEN 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int RoundsCnt = 0;

typedef struct Drunk
{
	char name[DRUNK_NAME_LEN];
	int health;
}Drunk;

Drunk* createDrunk(char* name)
{
	Drunk* drunk = malloc(sizeof(Drunk));

	strcpy_s(drunk->name, _countof(drunk->name), name);
	drunk->health = 100;

	return drunk;
}

typedef struct argStruct
{
	Drunk* arg1;
	Drunk* arg2;
}argStruct;

void IncrementRounds()
{
	pthread_mutex_lock(&mutex);

	RoundsCnt++;

	pthread_mutex_unlock(&mutex);
}

void Attack(Drunk* attacker, Drunk* attacked)
{
	int damage = (rand() % 20) + 1;
	int newHealth = attacked->health - damage;
	if (newHealth < 0)
	{
		newHealth = 0;
	}
	attacked->health = newHealth;

	printf("%s hit %s for %d damage (%d hp left)!\n", attacker->name, attacked->name, damage, attacked->health);

	if (attacked->health == 0)
	{
		printf("\n%s killed %s\n\n", attacker->name, attacked->name);
	}
}

void createArray(Drunk*** arr, size_t size)
{
	*arr = malloc(size * sizeof(Drunk));

	for (size_t i = 0; i < size; i++)
	{
		char name[DRUNK_NAME_LEN];
		snprintf(name, _countof(name), "Drunk-%d", i + 1);
		(*arr)[i] = createDrunk(name);
	}
}

void recreateArray(Drunk*** arr, size_t* currentArrSize)
{
	size_t alive = 0;

	// loop through original arr and count alive
	for (size_t i = 0; i < (*currentArrSize); i++)
	{
		if ((*arr)[i]->health > 0)
		{
			alive++;
		}
	}

	Drunk** tempArr = NULL;
	tempArr = malloc(alive * sizeof(Drunk));

	size_t newArrIndex = 0;
	for (size_t i = 0; i < (*currentArrSize); i++)
	{
		if ((*arr)[i]->health > 0)
		{
			tempArr[newArrIndex++] = createDrunk((*arr)[i]->name);
		}
	}

	*arr = tempArr;
	(*currentArrSize) = alive;

	//for (size_t i = 0; i < alive; i++)
	//{
	//	tempArr[i] = NULL;
	//}
	//tempArr = NULL;
}

void* fight(void* drunks)
{
	srand((unsigned)time(NULL) + GetCurrentThreadId());

	argStruct* args = drunks;
	Drunk* drunk = args->arg1;
	Drunk* anotherDrunk = args->arg2;

	printf("%s CHALLENGED %s's mother!\n", drunk->name, anotherDrunk->name);
	Sleep(2 * 1000);

	int turn = 1;
	while (drunk->health > 0 && anotherDrunk->health > 0)
	{
		if (turn % 2 == 0)
		{
			Attack(drunk, anotherDrunk);
		}
		else
		{
			Attack(anotherDrunk, drunk);
		}

		Sleep(1 * 100);
		turn++;
	}

	if (drunk->health <= 0)
	{
		anotherDrunk->health = 100;
	}
	else
	{
		drunk->health = 100;
	}

	IncrementRounds();

	return 0;
}

int main()
{
	// this size is dynamically changed multiple times
	size_t currentArrSize = -1;

	printf("Enter number of drunks: ");
	if (scanf_s("%d", &currentArrSize) == 0)
	{
		return;
	}

	Drunk** drunks = NULL;
	createArray(&drunks, currentArrSize);

	size_t argsArrSize = currentArrSize / 2;
	argStruct* args = malloc(argsArrSize * sizeof(argStruct));

	while (1)
	{
		pthread_t* threadIds = malloc((currentArrSize / 2) * sizeof(pthread_t));
		size_t threadIndex = 0;
		size_t drunksIndex = 0;

		for (size_t i = 0; i < currentArrSize / 2; i++)
		{
			args[i].arg1 = drunks[drunksIndex++];
			args[i].arg2 = drunks[drunksIndex++];
			pthread_create(&threadIds[threadIndex++], NULL, fight, &args[i]);
		}

		for (size_t i = 0; i < currentArrSize / 2; i++)
		{
			pthread_join(threadIds[i], NULL);
		}

		recreateArray(&drunks, &currentArrSize);

		if (currentArrSize < 2)
		{
			break;
		}

		free(threadIds);
		Sleep(1 * 1000);
	}

	size_t winner = drunks[0]->health > 0 ? 0 : 1;

	printf("The winner is: %s\n", drunks[winner]->name);
	printf("Total rounds: %ld\n", RoundsCnt);

	for (size_t i = 0; i < currentArrSize; i++)
	{
		free(drunks[i]);
	}
	free(drunks);
	free(args);

	return 0;
}