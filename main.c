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

int getRandomFrom(int min , int max)
{
    return (rand()%(max - min) + min);
}


void checkOperationStatus(enum Operation op ,const char * obj,  int rc , int return_type)
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
		printf("\n-Error : %s::Failed to %s | Return code : %i" , obj, op_name , rc);
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
int customer_handler = 8;
int availabe_customer_handlers = 8;


// customers variables
typedef struct Customer
{
	pthread_t thread;
	int Id;

}Customer;

int customers_count = 0;
Customer * customers = 0;

int random_seed = 0;


//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

void * handleCustomer(void * customer)
{

	return 0;
}

//-------------------------------------
//
//           Main
//
//-------------------------------------

int main(int argc , char * argv[])
{
     printf("\n\n ----------- Main -----------\n");
     srand(time(NULL)); // randomize seed


    /* ------------------- Initialize ------------------- */
    if( argc != 3 )
    {
        printf("\n-Error : Program needs 2 arguments : { customers_count , seed_randomizer }");
        printf("\n--Exiting program..");
        exit(-1);
    }

	customers_count = atoi(argv[1]);
	random_seed    = atoi(argv[2]);

	// Init customers
	customers = malloc(customers_count * sizeof(Customer));
	if(!customers)
	{
		printf("\n-Error : customers::Failed to malloc()");
		printf("\n--Exiting program..");
		exit(-1);
	}

	int rc;
	for(int i = 0; i<customers_count; i++)
	{
		customers[i].Id = i;
		rc = pthread_create(&customers[i].thread , NULL , handleCustomer , &customers[i] );
		checkOperationStatus(thread_create , "customers[]" , rc , 1);
	}

	/* -------------------------------------------------- */


	/* ----------------- join threads ------------------- */
	void * status;
	for(int i = 0; i<customers_count; i++)
	{
		rc = pthread_join(customers[i].thread , &status);
		checkOperationStatus(thread_join  , "customers[]" , rc , 1);
	}
	/* -------------------------------------------------- */


	// clean up
	free(customers);

    printf("\n");
    return 0;
}
