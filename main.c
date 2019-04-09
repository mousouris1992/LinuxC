#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//


enum Operation
{

    _mutex_lock,
    _mutex_unlock,
    _thread_cond_wait,
    _thread_cond_broadcast,
    _thread_cond_signal,
    _mutex_init,
    _thread_cond_init,
    _thread_create,
    _thread_join,
    _mutex_destroy,
    _thread_cond_destroy

};


//-------------------------------------
//
//          Variables
//
//-------------------------------------

#define BILLION        1000000000L;

#define n_seat         250
#define n_tel          8
#define n_seatMin      1
#define n_seatMax      5
#define t_seatMin      1
#define t_seatMax      10
#define p_cardSuccess  90
#define c_seat         20


// business variables
int balance = 0;
int Transactions_counter = 0;
int seatsPlan[n_seat];

// customer handlers
int av_customer_handlers = n_tel;


// customers variables
typedef struct Customer
{
	pthread_t thread;

	int Id;

}
Customer;

int customers_count = 0;
Customer * customers = 0;
int random_seed = 0;

// Mutexes && cond_variables
pthread_mutex_t mutex0;
pthread_mutex_t av_handler_mutex , av_handler_mutex_2;
pthread_mutex_t service_mutex;

pthread_cond_t av_handler_cond;

// testing variables
//static int shared_var = 0;

//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

void * handleCustomer(void * customer)
{

	Customer* cust = (Customer *)customer;
	int tid = cust->Id;

	struct timespec
	t_start,
	t_global_end,
	t_wait_end;
	clock_gettime(CLOCK_REALTIME , &t_start);

	// customer enters the queue of service...
	printf("\n\nCustomer#%i : enters the queue!" , tid);
	//printf("\nCustomer#%i : av_customer_handlers = %i" , tid, av_customer_handlers);


	/*
	  * mutex_lock()
	  * check for available customer handlers
	  * if (av_customer_handlers > 0 ) -> customer is being handled
	  * else -> customer waits in the queue until a customer handler is free
	*/
	mutex_lock(&av_handler_mutex);

	printf("\n-Report : av_customer_handlers = %i" , av_customer_handlers);
	while(av_customer_handlers == 0)
	{
		// customer waits on "av_handler_cond" condition till it gets signaled from another customer whose service handling has finished
		printf("\nCustomer#%i : waits in queue until there is an availabe handler..", tid);
		cond_wait(&av_handler_cond , &av_handler_mutex);
		printf("\nCustomer#%i : Finally is his time to get handled..", tid);

	}
	clock_gettime(CLOCK_REALTIME , &t_wait_end);

	/* customer gets handled by a customerHandler */
	av_customer_handlers--;
	// mutex_unlock() - share of shared variable no more needed
	mutex_unlock(&av_handler_mutex);


	// services being handled...
	printf("\n\nCustomer#%i : is being handled by a customerHandler..." , tid);
	mutex_lock(&service_mutex);
	// ...
	// ..
	// ..
	// ...
	mutex_unlock(&service_mutex);
	sleep(2);


	// again , we have to mutex_lock() in order to access shared variable
	mutex_lock(&av_handler_mutex);

	printf("\n\nCustomer#%i : Finished!");
	av_customer_handlers++;

	// broadcasting signal for all the customers in 'queue' so they can get handled by the free customerHandler!
    printf("\n-Report : A CustomerHandler is free , next customer in queue is being signaled!");
	pthread_cond_signal(&av_handler_cond);	//cond_broadcast(&av_handler_cond);


	mutex_unlock(&av_handler_mutex);

	//
	clock_gettime(CLOCK_REALTIME , &t_global_end);
	double wait_time = (t_wait_end.tv_sec - t_start.tv_sec) + (t_wait_end.tv_nsec - t_start.tv_nsec) / BILLION;
	double total_time = (t_global_end.tv_sec - t_start.tv_sec) + (t_global_end.tv_nsec - t_start.tv_nsec) / BILLION;


	//printf("\nwait time : %d ", total_time);
	//mutex_lock(mutex0);
	printf("\n\n-Thread#%i [Report] : Wait time  = %f", tid ,wait_time);
	//printf("\n           [Report] : Total time = %f", total_time);
	//mutex_unlock(mutex0);

	pthread_exit(cust->Id);
}


//-------------------------------------
//
//           Main
//
//-------------------------------------

int main(int argc , char * argv[])
{


     printf("\n\n ----------- Main -----------\n");


    /* ------------------- Initialize ------------------- */
    if( argc != 3 )
    {
        printf("\n-Error : Program needs 2 arguments : { customers_count , seed_randomizer }");
        printf("\n--Exiting program..");
        exit(-1);
    }

	Init(argv);

	int rc;
	for(int i = 0; i<customers_count; i++)
	{
		customers[i].Id = i;
		rc = pthread_create(&customers[i].thread , NULL , handleCustomer , &customers[i] );
		checkOperationStatus(_thread_create , rc , 1);
	}

	/* -------------------------------------------------- */


	/* ----------------- join threads ------------------- */
	void * status;
	for(int i = 0; i<customers_count; i++)
	{
		rc = pthread_join(customers[i].thread , &status);
		checkOperationStatus(_thread_join  , rc , 1);
	}
	/* -------------------------------------------------- */


	// clean up
	free(customers);

    printf("\n");
    return 0;
}



//-------------------------------------
//
//        Helper Functions
//
//-------------------------------------

int getRandom(int min , int max)
{
    return (rand()%(max - min) + min);
}


void checkOperationStatus(enum Operation op ,  int rc , int return_type)
{
    char * op_name;
    switch(op)
    {
        case _mutex_lock : op_name = "mutex_lock()";
        break;

        case _mutex_unlock : op_name = "mutex_unlock()";
        break;

        case _thread_cond_wait : op_name = "thread_cond_wait()";
        break;

        case _thread_cond_broadcast :  op_name = "thread_cond_broadcast()";
        break;

        case _thread_cond_signal : op_name = "thread_cond_signal()";
        break;

        case _thread_create : op_name = "thread_create()";
        break;

        case _thread_join :  op_name = "thread_join()";
        break;

        case _mutex_init : op_name = "mutex_init()";
        break;

        case _mutex_destroy : op_name = "mutex_destroy()";
        break;

        case _thread_cond_init : op_name = "thread_cond_init()";
        break;

        case _thread_cond_destroy : op_name = "thread_cond_destroy()";
        break;

    }

    if(rc != 0)
    {
		printf("\n-Error : Failed to %s | Return code : %i", op_name , rc);
        if(return_type == 1)
        {
            exit(-1);
        }
        else
        {
            pthread_exit(&rc);
        }
    }

}


void mutex_lock(pthread_mutex_t *mutex)
{
	int rc = pthread_mutex_lock(mutex);
	checkOperationStatus(_mutex_lock , rc , 0);
}

void mutex_unlock(pthread_mutex_t *mutex)
{
	int rc = pthread_mutex_unlock(mutex);
	checkOperationStatus(_mutex_unlock , rc , 0);
}

void cond_wait(pthread_cond_t *cond , pthread_cond_t *mutex)
{
	int rc = pthread_cond_wait(cond , mutex);
	checkOperationStatus(_thread_cond_wait , rc , 0);
}

void cond_broadcast(pthread_cond_t *cond)
{
	int rc = pthread_cond_broadcast(cond);
	checkOperationStatus(_thread_cond_broadcast , rc , 0);
}


void Init(char * argv[])
{
	av_customer_handlers = n_tel;

	int rc;
	srand(time(NULL)); // randomize seed

	customers_count = atoi(argv[1]);
	random_seed     = atoi(argv[2]);

	// Init customers
	customers = malloc(customers_count * sizeof(Customer));
	if(!customers)
	{
		printf("\n-Error : customers::Failed to malloc()");
		printf("\n--Exiting program..");
		exit(-1);
	}

	// Init Mutexes && cond_variables
	rc = pthread_mutex_init(&mutex0 , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_mutex_init(&av_handler_mutex , NULL);
	checkOperationStatus(_mutex_init  , rc , 1);

	rc = pthread_mutex_init(&service_mutex , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_cond_init(&av_handler_cond , NULL);
	checkOperationStatus(_thread_cond_init , rc , 1);

}
