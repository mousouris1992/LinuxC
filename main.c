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
#define t_seatMin      1 // 5
#define t_seatMax      5 // 10
#define p_cardSuccess  90
#define c_seat         20

int seed;

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
	int seats_count;
	int * seats_index;
	int payment_success;
	int payment_value;
	char * msg;
	char * error_msg;

}
Customer;

int customers_count = 0;
Customer * customers = 0;

double m_wait_time;
double m_total_time;

struct timespec t_execution_start , t_execution_end;
double total_execution_time;

// Mutexes && cond_variables
pthread_mutex_t mutex0;
pthread_mutex_t av_handler_mutex , av_handler_mutex_2;
pthread_mutex_t service_mutex;
pthread_mutex_t seats_access_mutex;
pthread_mutex_t balance_access_mutex;
pthread_mutex_t report_state_mutex;

pthread_cond_t av_handler_cond;

// testing variables
//static int shared_var = 0;

//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

int approveSeatsRequest(Customer * cust , int process_time)
{

	int approve;
	sleep(process_time);

	mutex_lock(&seats_access_mutex);
	if(free_seats < cust->seats_count)
	{
		if(free_seats == 0)
		{
			cust->msg = "Seats reservation rejected | Theatre is full!";
		}
		else
		{
			cust->msg = "Seats reservation rejected | Not enough free seats!";
		}
		approve = 0;
	}
	else
	{
		/*
		//seats_index = malloc( seats_requested * sizeof(int));
		int count = 0;
		for(int i = 0; i<n_seat; i++)
		{
			if(seatsPlan[i] == 0)
			{
				//printf("\n -- seats_index[%i] = %i" , count , i );
				seats_index[count] = i;
				count++;
			}

			if(count == seats_requested)
			{
				break;
			}
		}
		*/
		approve = 1;
	}
	mutex_unlock(&seats_access_mutex);

	return approve;
}

void bindRequestedSeats(int * seats_index , int seats_requested , int customerId)
{

	mutex_lock(&seats_access_mutex);

	int count = 0;
	for(int i = 0; i<n_seat; i++)
	{
		if(seatsPlan[i] == 0)
		{
			seats_index[count] = i;
			seatsPlan[i] = customerId;
			count++;
		}

		if(count == seats_requested)
		{
			break;
		}
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
	//printf("\n\nCustomer#%i : enters the queue!" , tid);
	//printf("\nCustomer#%i : av_customer_handlers = %i" , tid, av_customer_handlers);


	/*
	  * mutex_lock()
	  * check for available customer handlers
	  * if (av_customer_handlers > 0 ) -> customer is being handled
	  * else -> customer waits in the queue until a customer handler is free
	*/
	mutex_lock(&av_handler_mutex);

	//printf("\n-Report : av_customer_handlers = %i" , av_customer_handlers);
	while(av_customer_handlers == 0)
	{
		// customer waits on "av_handler_cond" condition till it gets signaled from another customer whose service handling has finished
		//printf("\nCustomer#%i : waits in queue until there is an availabe handler..", tid);
		cond_wait(&av_handler_cond , &av_handler_mutex);
		//printf("\nCustomer#%i : Finally is his time to get handled..", tid);

	}

	/* customer gets handled by a customerHandler */
	av_customer_handlers--;
	// mutex_unlock() - share of shared variable no more needed
	mutex_unlock(&av_handler_mutex);
	clock_gettime(CLOCK_REALTIME , &t_wait_end);



	// customer is handled here..
	//printf("\n\nCustomer#%i : is being handled by a customerHandler..." , tid);


	// generate random number from [n_seatMin , n_seatMax]
    // seats to be taken by current customer
    cust->seats_count = getRandom(n_seatMin , n_seatMax);

    // time to fetch the request by the customerHandler
    // if (requested seats get approved) -> bind seats && payment process
    // else -> error message && current customer's handling completes
    int t_random = getRandom(t_seatMin , t_seatMax); //sleep(t_random);
    cust->seats_index = 0;

    //printf("\n-Customer#%i : requesting [%i] seats to server!",tid , cust->seats_count);

	if( approveSeatsRequest(cust , t_random))
	{
		cust->seats_index = malloc(cust->seats_count * sizeof(int));
		/*
		if(seats_index_buffer == 0)
		{
			printf("\n bad memory on seats_index_buffer");
		}
		else
		{
			for(int i = 0; i < seats_count; i++)
			{
				printf("\n seats_index[%i] = %i" , i , seats_index_buffer[i]);
			}
		}
		*/

		// bind requested seats
		bindRequestedSeats(cust->seats_index , cust->seats_count , tid);

		// proceed to payment
		//printf("\n-Server : Seats request of Customer#%i -> approved!" , tid);
		//printf("\n-Server : [%i] Requested seats got binded to customer#%i ... " , seats_count , tid);
		//printf("\n-Server : Proceeding with card payment with custoer#%i ..." , tid);


		// p_cardSuccess for payment to get approved
		// else seats gets replaced to seatsPlan
		if( approvePaymentRequest(tid) )
		{
			int money_to_pay = cust->seats_count * c_seat;
			transferMoneyToAccount( money_to_pay , tid);
			cust->payment_value = money_to_pay;
			cust->payment_success = 1;
			//cust->msg = "Seats reservation succesfull! | TransferId : %i"
			// print transfer info
		}
		else // customer's seats request gets rejected and binded seats return to the seatsPlan
		{
			unBindRequestedSeats(cust->seats_index , cust->seats_count , tid);
			cust->payment_success = 0;
			free(cust->seats_index);
			cust->seats_index = 0;
			cust->msg = "-Seats reservation rejected | Card Payment failure!";
			// print transfer info
		}

	}
	else
	{

	}
	//free(seats_index_buffer);


	// again , we have to mutex_lock() in order to access shared variable
	mutex_lock(&av_handler_mutex);

	//printf("\n\nCustomer#%i : Service Finished!" , tid);
	av_customer_handlers++;

	// broadcasting signal for all the customers in 'queue' so they can get handled by the free customerHandler!
    //printf("\n-Report : A CustomerHandler is free , next customer in queue is being signaled!");
	pthread_cond_signal(&av_handler_cond);	//cond_broadcast(&av_handler_cond);
	mutex_unlock(&av_handler_mutex);

	//
	clock_gettime(CLOCK_REALTIME , &t_global_end);
	double wait_time  = (t_wait_end.tv_sec - t_start.tv_sec) + (t_wait_end.tv_nsec - t_start.tv_nsec) / BILLION;
	double total_time = (t_global_end.tv_sec - t_start.tv_sec) + (t_global_end.tv_nsec - t_start.tv_nsec) / BILLION;

	m_wait_time += wait_time;
	m_total_time += total_time;

	/* customer Report */
	mutex_lock(&report_state_mutex);

	printf("\n\n   ____Customer Report____");
	printf("\n-customerId : %i" , tid);
	if(cust->payment_success)
	{
		printf("\n-Seats reservation -> succesfull ! ");
		printf("\n      - transferId : %i" , tid);
		printf("\n      - Seats Reserved : ");
		for(int i = 0; i<cust->seats_count; i++)
		{
			printf(" [%i]" , cust->seats_index[i]);
		}
		printf("\n      - Payment Value : %i ", cust->payment_value);
	}
	else
	{
		printf("\n%s" ,cust->msg);
	}

	printf("\n--Wait time  : %f" , wait_time);
	printf("\n--Total time : %f" , total_time);
	printf("\n\n");

	mutex_unlock(&report_state_mutex);


	if(cust->seats_index != 0)
	{
		free(cust->seats_index);
	}

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


	// create Customers && their Threads
	int rc;
	for(int i = 0; i<customers_count; i++)
	{
		customers[i].Id = i+1;
		customers[i].seats_count = 0;
		customers[i].seats_index = 0;
		customers[i].payment_success = 0;
		customers[i].payment_value = 0;
		customers[i].msg = "";
		customers[i].error_msg = "";

		rc = pthread_create(&customers[i].thread , NULL , handleCustomer , &customers[i] );
		checkOperationStatus(_thread_create , rc , 1);
	}


	/* ----------------- join threads ------------------- */
	void * status;
	for(int i = 0; i<customers_count; i++)
	{
		rc = pthread_join(customers[i].thread , &status);
		checkOperationStatus(_thread_join  , rc , 1);
	}



	/* ------------------ Final Report ------------------------ */
	clock_gettime(CLOCK_REALTIME , &t_execution_end);

	total_execution_time = (t_execution_end.tv_sec - t_execution_start.tv_sec) + (t_execution_end.tv_nsec - t_execution_start.tv_nsec) / BILLION;
	m_wait_time  /= (double)customers_count;
	m_total_time /= (double)customers_count;

	printf("\n\n\n -------- Theatre Report --------");

	// print final seats Plan
	printf("\n\n      -Total Customers handled : %i" , customers_count);
	printf("\n      -Total Balance : %i" , balance);
	printf("\n\n -------- Theatre SeatsPlan --------");
	for(int i = 0; i<n_seat; i++)
	{
		printf("\n      seat[%i] -> ",i);
		if(seatsPlan[i] != 0)
		{
			printf("customer_%i" , seatsPlan[i]);
		}
		else
		{
			printf("free");
		}
	}
	printf("\n");
	printf("\n      -average wait_time    : %f" , m_wait_time);
	printf("\n      -average total_time   : %f" , m_total_time);
	printf("\n      -Total execution time : %f" , total_execution_time);

	// clean up
	cleanUp();

    printf("\n\n\n");
    return 0;
}



//-------------------------------------
//
//        Helper Functions
//
//-------------------------------------

int getRandom(int min , int max)
{
    return (rand_r(&seed)%(max - min) + min);
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

	clock_gettime(CLOCK_REALTIME , &t_execution_start);

	customers_count = atoi(argv[1]);
	seed            = atoi(argv[2]);

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

	rc = pthread_mutex_init(&report_state_mutex , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_cond_init(&av_handler_cond , NULL);
	checkOperationStatus(_thread_cond_init , rc , 1);


	for(int i = 0; i<n_seat; i++)
	{
		seatsPlan[i] = 0;
	}

}


void cleanUp()
{
	int rc;

	free(customers);

	rc = pthread_mutex_destroy(&mutex0);
    checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&av_handler_mutex);
	checkOperationStatus(_mutex_destroy , rc , 1);

    rc = pthread_mutex_destroy(&service_mutex);
    checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&seats_access_mutex);
    checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&balance_access_mutex);
	checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&report_state_mutex);
	checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_cond_destroy(&av_handler_cond);
	checkOperationStatus(_thread_cond_destroy , rc , 1);


}
