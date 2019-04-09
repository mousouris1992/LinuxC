#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//


//-------------------------------------
//
//        Helper Functions
//
//-------------------------------------

int getRandom(int min , int max)
{
    return (rand()%(max - min) + min);
}

#define n_seat         250
#define n_tel          8
#define n_seatMin      1
#define n_seatMax      5
#define t_seatMin      1
#define t_seatMax      10
#define p_cardSuccess  90
#define c_seat         20


int main()
{
	printf("\n random[%i , %i] : %i" ,n_seatMin , n_seatMax , getRandom(n_seatMin , n_seatMax));
	printf("\n random[%i , %i] : %i" ,t_seatMin , t_seatMax , getRandom(t_seatMin , t_seatMax));

	printf("\n");
	return 0;
}
