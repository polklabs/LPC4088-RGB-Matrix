#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

//Linked List
typedef struct list{
	int data;
	struct list *next;
}list_t;

list_t* push(list_t *head, int value);
void freeList(list_t* head);
//functions

void deleteGroup(list_t *deleteYX);

//check to see if there are any colors in a row of n or more
void checkGroup();

void newGame();

void update();

void moveDotLeft();

void moveDotRight();

void Pause();
