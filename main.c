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
int free_seats = n_seat;

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
pthread_mutex_t seats_access_mutex;
pthread_mutex_t balance_access_mutex;

pthread_cond_t av_handler_cond;

// testing variables
//static int shared_var = 0;

//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

int approveSeatsRequest(int * seats_index , int seats_requested , int process_time)
{


	int approve;

	sleep(process_time);

	mutex_lock(&seats_access_mutex);
	if(free_seats < seats_requested)
	{
		seats_index = 0;
		approve = 0;
	}
	else
	{
		seats_index = malloc( seats_requested * sizeof(int));
		if(!seats_index)
		{
			printf("\n -- approveSeatsRequest()::seats_index::malloc() failed!");
			exit(-1);
		}
		
		int count = 0;

		for(int i = 0; i<n_seat; i++)
		{
			if(seatsPlan[i] == 0)
			{
				seats_index[count] = i;
				count++;
			}

			if(count == seats_requested -1)
			{
				break;
			}
		}

		approve = 1;
	}

	mutex_unlock(&seats_access_mutex);

	return approve;
}

void bindRequestedSeats(int * seats_index , int seats_requested , int customerId)
{

	mutex_lock(&seats_access_mutex);

	for(int i = 0; i<seats_requested; i++)
	{
		seatsPlan[seats_index[i]] = customerId;
	}

	mutex_unlock(&seats_access_mutex);

}

void unBindRequestedSeats(int * seats_index , int seats_requested , int customerId)
{
	mutex_lock(&seats_access_mutex);

	for(int i = 0; i<seats_requested; i++)
	{
		seatsPlan[seats_index[i]] = 0;
	}

	mutex_unlock(&seats_access_mutex);
}

int approvePaymentRequest(int customerId)
{
	int p = getRandom(1 , 100);
	if ( p <= p_cardSuccess)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void transferMoneyToAccount(int money , int customerId)
{
	mutex_lock(&balance_access_mutex);
	balance += money;
	mutex_unlock(&balance_access_mutex);
}


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

	/* customer gets handled by a customerHandler */
	av_customer_handlers--;
	// mutex_unlock() - share of shared variable no more needed
	mutex_unlock(&av_handler_mutex);
	clock_gettime(CLOCK_REALTIME , &t_wait_end);



	// customer is handled here..
	printf("\n\nCustomer#%i : is being handled by a customerHandler..." , tid);

	// generate random number from [n_seatMin , n_seatMax]
	// seats to be taken by current customer
	int seats_count = getRandom(n_seatMin , n_seatMax);

	// time to fetch the request by the customerHandler
	// if (requested seats get approved) -> bind seats && payment process
	// else -> error message && current customer's handling completes
	int t_random = getRandom(t_seatMin , t_seatMax); //sleep(t_random);
	int * seats_index_buffer;
	printf("\n-Server : About to approve Customer's#%i seats Request..",tid);

#define PHASE_2
#ifdef PHASE_2
	if( approveSeatsRequest(seats_index_buffer , seats_count , t_random))
	{
		printf("\n-Server : Seats request approved for Customer#%i .. ", tid);
		/*
		// bind requested seats
		bindRequestedSeats(seats_index_buffer , seats_count , tid);

		// proceed to payment
		printf("\n-Server : Seats request -> approved!");
		printf("\n-Server : [%i] Requested seats got binded to customer#%i ... " , seats_count , tid);
		printf("\n-Server : Proceeding with card payment with custoer#%i ..." , tid);

		// p_cardSuccess for payment to get approved
		// else seats gets replaced to seatsPlan
		if( approvePaymentRequest(tid) )
		{

			transferMoneyToAccount( seats_count * c_seat , tid);
			// print transfer info

		}
		else // customer's seats request gets rejected and binded seats return to the seatsPlan
		{
			unBindRequestedSeats(seats_index_buffer , seats_count , tid);
			// print transfer info
		}

		*/
		free(seats_index_buffer);
	}
	else
	{
		// print error message and exit
	}
#endif

	// again , we have to mutex_lock() in order to access shared variable
	mutex_lock(&av_handler_mutex);

	printf("\n\nCustomer#%i : Service Finished!" , tid);
	av_customer_handlers++;

	// broadcasting signal for all the customers in 'queue' so they can get handled by the free customerHandler!
    printf("\n-Report : A CustomerHandler is free , next customer in queue is being signaled!");
	pthread_cond_signal(&av_handler_cond);	//cond_broadcast(&av_handler_cond);
	mutex_unlock(&av_handler_mutex);

	//
	clock_gettime(CLOCK_REALTIME , &t_global_end);
	double wait_time  = (t_wait_end.tv_sec - t_start.tv_sec) + (t_wait_end.tv_nsec - t_start.tv_nsec) / BILLION;
	double total_time = (t_global_end.tv_sec - t_start.tv_sec) + (t_global_end.tv_nsec - t_start.tv_nsec) / BILLION;


	//printf("\nwait time : %d ", total_time);
	//mutex_lock(mutex0);
	printf("\n\n-Customer#%i [Report] : Total time  = %f", tid ,total_time);
	printf("\n           [Report] : Wait time = %f", wait_time);
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
		customers[i].Id = i+1;
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

	rc = pthread_mutex_init(&seats_access_mutex , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_mutex_init(&balance_access_mutex , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_cond_init(&av_handler_cond , NULL);
	checkOperationStatus(_thread_cond_init , rc , 1);


	for(int i = 0; i<n_seat; i++)
	{
		seatsPlan[i] = 0;
	}

}
