#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//


enum Operation
{

    mutex_lock,
    mutex_unlock,
    thread_cond_wait,
    thread_cond_broadcast,
    thread_cond_signal,
    mutex_init,
    thread_cond_init,
    thread_create,
    thread_join,
    mutex_destroy,
    thread_cond_destroy

};

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
        case mutex_lock : op_name = "mutex_lock()";
        break;

        case mutex_unlock : op_name = "mutex_unlock()";
        break;

        case thread_cond_wait : op_name = "thread_cond_wait()";
        break;

        case thread_cond_broadcast :  op_name = "thread_cond_broadcast()";
        break;

        case thread_cond_signal : op_name = "thread_cond_signal()";
        break;

        case thread_create : op_name = "thread_create()";
        break;

        case thread_join :  op_name = "thread_join()";
        break;

        case mutex_init : op_name = "mutex_init()";
        break;

        case mutex_destroy : op_name = "mutex_destroy()";
        break;

        case thread_cond_init : op_name = "thread_cond_init()";
        break;

        case thread_cond_destroy : op_name = "thread_cond_destroy()";
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


void mutex_Lock(pthread_mutex_t *mutex)
{
	int rc = pthread_mutex_lock(mutex);
	checkOperationStatus(mutex_lock , rc , 0);
}

void mutex_Unlock(pthread_mutex_t *mutex)
{
	int rc = pthread_mutex_unlock(mutex);
	checkOperationStatus(mutex_unlock , rc , 0);
}

void cond_Wait(pthread_cond_t *cond , pthread_cond_t *mutex)
{
	int rc = pthread_cond_wait(cond , mutex);
	checkOperationStatus(thread_cond_wait , rc , 0);
}

void cond_Broadcast(pthread_cond_t *cond)
{
	int rc = pthread_cond_broadcast(cond);
	checkOperationStatus(thread_cond_broadcast , rc , 0);
}


//-------------------------------------
//
//          Variables
//
//-------------------------------------

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
pthread_mutex_t av_handler_mutex;

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
	int rc;

	Customer* cust = (Customer *)customer;
	int tid = cust->Id;
	int sec = getRandom(t_seatMin , t_seatMax);

	printf("\nCustomer#%i : enters the queue!" , tid);
	printf("\nCustomer#%i : av_customer_handlers = %i" , tid, av_customer_handlers);

	mutex_Lock(&av_handler_mutex);
	while(av_customer_handlers == 0)
	{
		printf("\nCustomer#%i : waits until there is an availabe handler..", tid);

		cond_Wait(&av_handler_cond , &av_handler_mutex);
		//checkOperationStatus(thread_cond_wait , "customer#i", rc , 0);
	}

	av_customer_handlers--;
	printf("\nCustomer#%i : is being handled..." , tid);
	printf("\nCustomer#%i : av_customer_handlers after = %i" , tid, av_customer_handlers);
	mutex_Unlock(&av_handler_mutex);

	sleep(2);

	mutex_Lock(&av_handler_mutex);
	printf("\nCustomer#%i : Finished , freeing customerHandler..");
	av_customer_handlers++;
	// broadcasting signal for all the customers in 'queue' so they can get handled!
	cond_Broadcast(&av_handler_cond);

	mutex_Unlock(&av_handler_mutex);


	return 0;
}


//-------------------------------------
//
//           Init Function
//
//-------------------------------------

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
	checkOperationStatus(mutex_init , rc , 1);

	rc = pthread_mutex_init(&av_handler_mutex , NULL);
	checkOperationStatus(mutex_init  , rc , 1);

	rc = pthread_cond_init(&av_handler_cond , NULL);
	checkOperationStatus(thread_cond_init , rc , 1);

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
		checkOperationStatus(thread_create , rc , 1);
	}

	/* -------------------------------------------------- */


	/* ----------------- join threads ------------------- */
	void * status;
	for(int i = 0; i<customers_count; i++)
	{
		rc = pthread_join(customers[i].thread , &status);
		checkOperationStatus(thread_join  , rc , 1);
	}
	/* -------------------------------------------------- */


	// clean up
	free(customers);

    printf("\n");
    return 0;
}
