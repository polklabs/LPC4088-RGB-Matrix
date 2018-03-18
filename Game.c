#include "Game.h"

int matrix[32][16] = {{0}};

int dotPosX = 0;
int dotPosY = 0;
int fillPercent = 60;
bool pause = false;
int numGroup = 4;

list_t* push(list_t *head, int value){
	if(head == NULL){
		list_t* temp;
		temp = malloc(sizeof(list_t));
		temp->next = NULL;
		temp->data = value;
		return temp;
	}
	else{
		head->next = push(head->next, value);
		return head;
	}
}

void freeList(list_t* head){
	list_t* tmp;
	while(head != NULL){
		tmp = head;
		head = head->next;
		free(tmp);
	}
}

//functions

void deleteGroup(list_t *deleteYX){
	list_t *temp = deleteYX;
	while(temp != NULL && temp->next != NULL){
		int y = temp->data;
		temp = temp->next;
		int x = temp->data;
		matrix[y][x] = 0;
		temp = temp->next;
	}
	freeList(deleteYX);
}

//check to see if there are any colors in a row of n or more
void checkGroup(){
	bool clean = true;
	list_t *deleteYX = NULL;
	int tries[] = {0, 1, -1, 0, -1, 1, -1, -1};
	int y,x,i,k;
	for(y = 0; y < 32; y++){
		for(x = 0; x < 16; x++){
			if (matrix[y][x] !=0){
				clean = false;
				int dot = matrix[y][x];
				for(i = 0; i < sizeof(tries)/sizeof(tries[0]); i = i +2){
					list_t *temp = NULL;
					int size = 0;
					for(k = 0; k < 32; k++){
						if(y+(k*tries[i]) >= 0){
							if(x + (k*tries[i+1]) >= 0 && x + (k*tries[i+1]) < 16){
								if(matrix[y+(k*tries[i])][x+(k*tries[i+1])] == dot) {
									temp = push(temp, y+(k*tries[i]));
									temp = push(temp, x+(k*tries[i+1]));
									size++;
								}else{
									break;
								}
							}
						}
					}
					if (size >=numGroup){
						while(temp != NULL){
							deleteYX = push(deleteYX, temp->data);
							temp = temp->next;
						}
					}
					freeList(temp);

				}
			}
			if(clean==true)
				break;
		}
	}
	deleteGroup(deleteYX);
}

void newGame(int seed){
	srand(seed);
	int y,x;
	for(y = 0; y < (int)(31*((float)fillPercent/(float)100)); y++)
		for(x = 0; x < 16; x++)
			matrix[y][x] = (rand() % 5) + 1;
}

void update(){
	if(pause == true)
		return;
	bool done = true;
	dotPosY -=1;
	int y, x;
	for(y = 1; y<= 31; y++){
		for(x = 0; x <16; x++){
			if(matrix[y][x] !=0){
				if(matrix[y-1][x] == 0){
					done = false;
					matrix[y-1][x] = matrix[y][x];
					matrix[y][x] = 0;
				}
			}
		}
	}
	if (done == true){
		checkGroup();
		int color = (rand() % 5) + 1;
		dotPosX = (rand() % 16);
		dotPosY = 31;
		matrix[31][dotPosX] = color;
	}
}

void moveDotLeft(){
	if(pause == false){
		if (dotPosX >= 1 && matrix[dotPosY][dotPosX-1] == 0){
			matrix[dotPosY][dotPosX-1] = matrix[dotPosY][dotPosX];
			matrix[dotPosY][dotPosX] = 0;
			dotPosX -=1;
		}
	}
}

void moveDotRight(){
	if(pause == false){
		if (dotPosX <= 14 && matrix[dotPosY][dotPosX+1] == 0){
			matrix[dotPosY][dotPosX+1] = matrix[dotPosY][dotPosX];
			matrix[dotPosY][dotPosX] = 0;
			dotPosX += 1;
		}
	}
}

void Pause(){
	if (pause == true)
		pause = false;
	else
		pause = true;
}
