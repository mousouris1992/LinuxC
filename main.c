#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//-------------------------------------
//
//        Utility Functions
//
//-------------------------------------
int getRandomFrom(int min , int max)
{
    return (rand()%(max - min) + min);
}



//-------------------------------------
//
//          Variables
//
//-------------------------------------
int balance = 0;
int Transactions_counter = 0;

int customer_handler = 8;
int availabe_customer_handlers = 8;



//-------------------------------------
//
//           Main
//
//-------------------------------------
int main(int argc , char * argv[])
{

    printf("\n\n ----------- Main -----------\n");
    srand(time(NULL)); // randomize seed





    printf("\n");
    return 0;
}
