#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//
enum Operation
{
	_malloc,
	something
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


void checkStatus(void * data , enum Operation op , const char * obj , int return_type)
{
	char * op_name;

	switch(op)
	{
		case _malloc :op_name = "malloc()";
		break;

		case something :  op_name = "something()";
		break;
	}

	if(!data)
	{
		printf("\n-Error : %s::Failed to %s" , obj , op_name);
		if(return_type == 0)
		{
			printf("\n--Terminating thread..");
			pthread_exit(NULL);
		}
		else
		{
			printf("\n--Exiting program..");
			exit(-1);
		}
	}
}

//-------------------------------------
//
//          Variables
//
//-------------------------------------

#define n_seat        = 250;
#define n_tel         = 8;
#define n_seatMin     = 1;
#define n_seatMax     = 5;
#define t_seatMin     = 1;
#define t_seatMax     = 10;
#define p_cardSuccess = 90;
#define c_seat        = 20;


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

int customer_count = 0;
Customer * customers = 0;

int random_seed = 0;


//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

void * handleCustomer(void * customerId)
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
	// --------------------------------------------------
    if( argc != 3 )
    {
        printf("\n-Error : Program needs 2 arguments : { customers_count , seed_randomizer }");
        printf("\n--Exiting program..");
        exit(-1);
    }

	customer_count = atoi(argv[1]);
	random_seed    = atoi(argv[2]);

	customers = malloc(customer_count * sizeof(Customer));
	checkStatus(customers , _malloc , "customerThreads" , 1);



	// clean up
	free(customers);

    printf("\n");
    return 0;
}
