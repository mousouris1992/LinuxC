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

#define BIL            1000000000L

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
	srand(time(NULL)); // randomize seed


	//printf("\n random[%i , %i] : %i" ,n_seatMin , n_seatMax , getRandom(n_seatMin , n_seatMax));
	//printf("\n random[%i , %i] : %i" ,t_seatMin , t_seatMax , getRandom(t_seatMin , t_seatMax));

	struct timespec
	t_start,
	t_end;

	clock_gettime(CLOCK_REALTIME , &t_start);
	sleep(2);
	clock_gettime(CLOCK_REALTIME , &t_end);

	double total_time = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec) / BIL;

	printf("time : %d",total_time);

	printf("\n");
	return 0;
}
