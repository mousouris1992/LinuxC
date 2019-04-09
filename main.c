#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main(int argc , char * argv[])
{

    printf("\n\n ----------- Main -----------\n");

    srand(time(NULL)); // randomize seed

    int random = rand()%100;

    printf("random : %i",random);



    printf("\n");
    return 0;
}
