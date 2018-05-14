#include "Game.h"
#include "board.h"

int matrix[32][16] = {{0}};

//Where are the dots currently
int dotPosX = 0;
int dotPosY = 0;
int dotPosX2 = 0;
int dotPosX3 = 0;
int dotPosY2 = 0;
int dotPosY3 = 0;

//Are the dots still falling
bool fall1 = false;
bool fall2 = false;
bool fall3 = false;

int fillPercent = 60;
bool pause = false;
int numGroup = 4;

//If the player has lost
bool lose = false;
bool drained = false;

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
	//Loop through all pixels
	for(y = 0; y < 32; y++){
		clean = true;
		for(x = 0; x < 16; x++){
			//Is a pixel filled in
			if (matrix[y][x] != 0){
				clean = false;
				int dot = matrix[y][x];
				for(i = 0; i < sizeof(tries)/sizeof(tries[0]); i = i +2){
					//Skip if close to edge
					if(x > 16 - numGroup)
						i+=2;
					if(y < numGroup-1)
						i+=2;
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
		}
		if(clean==true){
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

void checkLoss(){
	int x;
	for(x = 0; x < 16; x++)
		if(matrix[31][x] != 0){
			lose = true;
			return;
		}
}

void update(){
	if(pause == true)
		return;
	bool done = true;
	bool clean = true;
	int y, x;

	for(x = 0; x < 16; x++)
		if(matrix[0][x]!=0){
			clean = false;
			if(lose == true)
				matrix[0][x] = 0;
		}

	if(clean == true){
		drained = true;
		return;
	}

	for(y = 1; y<= 31; y++){
		for(x = 0; x <16; x++){
			if(matrix[y][x] !=0){
				if(matrix[y-1][x] == 0){
					done = false;
					matrix[y-1][x] = matrix[y][x];
					matrix[y][x] = 0;
				}
				else if(y == dotPosY && x == dotPosX){
					fall1 = false;
				}
				else if(y == dotPosY2 && x == dotPosX2){
					fall2 = false;
				}
				else if(y == dotPosY3 && x == dotPosX3){
					fall3 = false;
				}
			}
		}
	}
	checkLoss();
	dotPosY--;
	dotPosY2--;
	dotPosY3--;
	if (done == true && lose == false){
		Chip_GPIO_SetPinState(LPC_GPIO, 2, 9, true);
		checkGroup();
		Chip_GPIO_SetPinState(LPC_GPIO, 2, 9, false);
		//New Dot 1
		int color = (rand() % 5) + 1;
		dotPosX = (rand() % 16);
		dotPosY = 31;
		matrix[31][dotPosX] = color;
		fall1 = true;

		//New Dot 2
		color = (rand() % 5)+1;
		dotPosX2 = (rand() % 16);
		while(dotPosX2 == dotPosX)
			dotPosX2 = (rand() %16);
		dotPosY2 = 31;
		matrix[31][dotPosX2] = color;
		fall2 = true;

		//New Dot 3
		color = (rand() % 5)+1;
		dotPosX3 = (rand() % 16);
		while(dotPosX3 == dotPosX || dotPosX3 == dotPosX2)
			dotPosX3 = (rand() % 16);
		dotPosY3 = 31;
		matrix[31][dotPosX3] = color;
		fall3 = true;
	}
}

void moveDotLeft(){
	if(pause == false){
		if (fall1 == true && dotPosX >= 1 && matrix[dotPosY][dotPosX-1] == 0){
			matrix[dotPosY][dotPosX-1] = matrix[dotPosY][dotPosX];
			matrix[dotPosY][dotPosX] = 0;
			dotPosX -=1;
		}
		if (fall2 == true && dotPosX2 >= 1 && matrix[dotPosY2][dotPosX2-1] == 0){
			matrix[dotPosY2][dotPosX2-1] = matrix[dotPosY2][dotPosX2];
			matrix[dotPosY2][dotPosX2] = 0;
			dotPosX2 -=1;
		}
		if (fall3 == true && dotPosX3 >= 1 && matrix[dotPosY3][dotPosX3-1] == 0){
			matrix[dotPosY3][dotPosX3-1] = matrix[dotPosY3][dotPosX3];
			matrix[dotPosY3][dotPosX3] = 0;
			dotPosX3 -=1;
		}
	}
}

void moveDotRight(){
	if(pause == false){
		if (fall1 == true && dotPosX <= 14 && matrix[dotPosY][dotPosX+1] == 0){
			matrix[dotPosY][dotPosX+1] = matrix[dotPosY][dotPosX];
			matrix[dotPosY][dotPosX] = 0;
			dotPosX += 1;
		}
		if (fall2 == true && dotPosX2 <= 14 && matrix[dotPosY2][dotPosX2+1] == 0){
			matrix[dotPosY2][dotPosX2+1] = matrix[dotPosY2][dotPosX2];
			matrix[dotPosY2][dotPosX2] = 0;
			dotPosX2 += 1;
		}
		if (fall3 == true && dotPosX3 <= 14 && matrix[dotPosY3][dotPosX3+1] == 0){
			matrix[dotPosY3][dotPosX3+1] = matrix[dotPosY3][dotPosX3];
			matrix[dotPosY3][dotPosX3] = 0;
			dotPosX3 += 1;
		}
	}
}

void Pause(){
	if (pause == true)
		pause = false;
	else
		pause = true;
}
