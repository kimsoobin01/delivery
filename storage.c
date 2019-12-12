#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

/*
  definition of storage cell structure ----
  members :
  int building : building number of the destination
  int room : room number of the destination
  int cnt : number of packages in the cell
  char passwd[] : password setting (4 characters)
  char *contents : package context (message string)
*/
typedef struct {
   int building;
   int room;
   int cnt;
   char passwd[PASSWD_LEN + 1];

   char *context;
} storage_t;


static storage_t** deliverySystem;          //deliverySystem
static int storedCnt = 0;               //number of cells occupied
static int systemSize[2] = { 0, 0 };        //row/column of the delivery system
static char masterPassword[PASSWD_LEN + 1];   //master password

static FILE* fileSystem = NULL; // file 관련 구조체 동적 선언

// ------- inner functions ---------------

//print the inside context of a specific cell
//int x, int y : cell to print the context
static void printStorageInside(int x, int y) {  
   printf("\n------------------------------------------------------------------------\n");
   printf("------------------------------------------------------------------------\n");
   if (deliverySystem[x][y].cnt > 0)
      printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
   else
      printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

   printf("------------------------------------------------------------------------\n");
   printf("------------------------------------------------------------------------\n\n");
}

//initialize the storage
//set all the member variable as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {

   //초기값 생성  
   deliverySystem[x][y].building = 0;
   deliverySystem[x][y].room = 0;
   deliverySystem[x][y].context = (char*)malloc(sizeof(char) * 100);
   deliverySystem[x][y].cnt = 0;
   *deliverySystem[x][y].passwd = (char*)malloc(sizeof(char) * (PASSWD_LEN + 1));
   
}

//get password input and check if it is correct for the cell (x,y)
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {
   int i;
   char passwd[PASSWD_LEN + 1];

   printf(" - password : ");
   scanf("%4s", passwd);
   fflush(stdin);

   for (i = 0; i < PASSWD_LEN + 1; ++i)
   {
      if (!strcmp(passwd, masterPassword)) { break; } // 마스터키일시 반복문 빠져나오기

      if (deliverySystem[y][x].passwd[i] != passwd[i]) return -1; // 비밀번호가 일치하지 않다면 -1 반환
   }

   return 0;
}



// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {

   int x, y;
   // 파일 열려있으면 닫기  
   if (fileSystem != NULL) fclose(fileSystem);

   // 파일 쓰기모드로 열기  
   fileSystem = fopen(filepath, "w"); 

   if (fileSystem == NULL) return -1; //백업 파일이 없다면 -1을 반환

   fprintf(fileSystem, "%d %d\n", systemSize[0], systemSize[1]); // 사이즈를 넣어줌
   fprintf(fileSystem, "%s\n", masterPassword); // 마스터키를 넣어줌
   
   systemSize[0] = x;
   systemSize[1] = y;
   
   //storage 초기값 생성
   for (x = 0; x < systemSize[0]; x++)
   {
		for (y = 0; y < systemSize[1]; y++)
		{
			initStorage(x, y);
		}
   }
   
   //입력한 building, room, passwd, context backup
   
   for (x = 0; x < systemSize[0]; x++)
   {
		for (y = 0; y < systemSize[1]; y++)
		{
			if (deliverySystem[x][y].cnt > 0)
			{
				fprintf(fileSystem, "%d %d %d %d %s %s\n", x, y, deliverySystem[x][y].building, deliverySystem[x][y].room, deliverySystem[x][y].passwd, deliverySystem[x][y].context);
			}
		}
   }
   return 0;
}



//create delivery system on the double pointer deliverySystem
//char* filepath : filepath and name to read config parameters (row, column, master password, past contexts of the delivery system
//return : 0 - successfully created, -1 - failed to create the system
int str_createSystem(char* filepath) {
   int i;
   char *context;

   //파일을 읽기모드로 열기  
   fileSystem = fopen(filepath, "r"); 

   if (fileSystem == NULL) return -1; // 백업 파일이 없다면 -1 할당  

   //deliverySystem 동적 할당  
   deliverySystem = (storage_t**)malloc(systemSize[0] * sizeof(storage_t*));
   for (i = 0; i < systemSize[0]; i++)
   {
		deliverySystem[i] = (storage_t*)malloc(systemSize[1] * sizeof(storage_t));
   }
   return 0;
   
   context = (char*)malloc(sizeof(storage_t));

   return 0;
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {
   int i, j;
   if (fileSystem != NULL) // 파일 해제
   {
      fclose(fileSystem); // 현재 파일 닫아줌
      fileSystem = NULL; // 파일 시스템 해제
   }

   if (deliverySystem != NULL) // deliverySystem이 있다면
   {
      for (i = 0; i < systemSize[0]; ++i) //보관함 해제
      {
         free(deliverySystem[i]);
      }
      free(deliverySystem); // 최종적으로 해제
   }
}



//check if the input cell (x,y) is valid and whether it is occupied or not
int str_checkStorage(int x, int y) {
   if (x < 0 || x >= systemSize[0]) return -1;

   if (y < 0 || y >= systemSize[1]) return -1;

   return deliverySystem[x][y].cnt;
}


//put a package (msg) to the cell
//input parameters
//int x, int y : coordinate of the cell to put the package
//int nBuilding, int nRoom : building and room numbers of the destination
//char msg[] : package context (message string)
//char passwd[] : password string (4 characters)
//return : 0 - successfully put the package, -1 - failed to put
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE + 1], char passwd[PASSWD_LEN + 1]) {

   if (x < 0 || x >= systemSize[0]) return -1; // 범위 초과시 -1반환

   if (y < 0 || y >= systemSize[1]) return -1; // 범위 초과시 -1반환

   /*아래 delivery 저장 부분*/
   deliverySystem[x][y].building = nBuilding;
   deliverySystem[x][y].room = nRoom;
   strcpy(deliverySystem[x][y].context, msg);

   *deliverySystem[x][y].passwd = passwd;

   deliverySystem[x][y].cnt = 1;

   return 0;
}



//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {
   char passwd[PASSWD_LEN + 1];
   char *context;
   
   for (x = 0; x < systemSize[0]; x++)
   {
		for (y = 0; y < systemSize[1]; y++)
		{
			if (inputPasswd(x, y) == deliverySystem[x][y].passwd) //passwd와 저장된 passwd가 같으면
			{
				return 0;

				free(context); //동적 메모리 해제  

				printStorageInside(x, y);

				initStorage(x, y);
			}
		}
   }
   return -1;
}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) { 

   int x, y;
   int cnt = 0;

   for (x = 0; x < systemSize[0]; x++)
   {
      for (y = 0; y < systemSize[1]; y++)
      {
        /*저장소 안에 정보가 일치 하는 것이 있다면*/
         if (deliverySystem[x][y].building == nBuilding && deliverySystem[x][y].room == nRoom)
         {
            printf(" -----------> Found a package in (%d, %d)\n", x, y);
            cnt++;
         }
      }
   }
   return cnt;
}
