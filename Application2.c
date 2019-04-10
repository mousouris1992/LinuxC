
#include "Application2.h"
//



//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

int approveSeatsRequest(Customer * cust)
{

	int approve;
	sleep(getRandom(t_seatMin , t_seatMax));

	int seats_count = cust->seats_count;
	int zoneId = cust->zoneId;

	mutex_lock(&seats_access_mutex);
	if(free_seats[zoneId] < seats_count)
	{
		cust->msg = "-Seats request rejected | Reason :  not enough available seats!";
		approve = 0;
	}
	else
	{
		cust->seats_index = malloc( seats_count * sizeof(int));
		if(!cust->seats_index)
		{
			printf("\n-Error : cust->seats_index::malloc() failed!");
			printf("\n--Exiting program...");
			exit(-1);
		}

		int seats_found = 0;
		int av_seats_count;
		for(int i = 0; i<zoneSize[zoneId]; i++)
		{
			av_seats_count = 0;
			for(int j = 0; j<n_seat; j++)
			{

				if(zones[zoneId][i * n_seat + j] == 0)
				{
					cust->seats_index[av_seats_count] = i * n_seat + j;
					//printf("\nseats count : %i" , av_seats_count);
					av_seats_count++;
				}

				if(av_seats_count == seats_count)
				{
					seats_found = 1;
					break;
				}
			}

			if(seats_found)
			{
				approve = 1;
				break;
			}
		}
	}
	mutex_unlock(&seats_access_mutex);
	return approve;
}

void bindRequestedSeats(int * seats_index , int seats_requested , int customerId)
{

	mutex_lock(&seats_access_mutex);

	mutex_unlock(&seats_access_mutex);

}

void unBindRequestedSeats(int * seats_index , int seats_requested , int customerId)
{
	mutex_lock(&seats_access_mutex);

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
		cond_wait(&av_handler_cond , &av_handler_mutex);
	}

	/* customer gets handled by a customerHandler */
	av_customer_handlers--;
	// mutex_unlock() - share of shared variable no more needed
	mutex_unlock(&av_handler_mutex);
	clock_gettime(CLOCK_REALTIME , &t_wait_end);


	// customer is handled here..

	// customer selects zone { A,B,C }
    int p_zone = getRandom(1,100);
	if(p_zone <= p_zoneB)
	{
		cust->zoneId = 1; // 1 = zoneB
	}
	else if(p_zone <= p_zoneB + p_zoneC)
	{
		cust->zoneId = 2; // 2 = zoneC
	}
	else
	{
		cust->zoneId = 0; // 0 = zoneA
	}

	// customer selects how many seats[n_seatMin , n_seatMax]
	cust->seats_count = getRandom(n_seatMin , n_seatMax);

#define PHASE_2
#ifdef PHASE_2
	if( approveSeatsRequest(cust))
	{
		printf("\n -Seats approved!");
		/*
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
		*/

	}
	else
	{
		printf("\n -Seats rejected!");

	}
#endif


	// again , we have to mutex_lock() in order to access shared variable
	mutex_lock(&av_handler_mutex);
	av_customer_handlers++;
	// broadcasting signal for all the customers in 'queue' so they can get handled by the free customerHandler!
	pthread_cond_signal(&av_handler_cond);	//cond_broadcast(&av_handler_cond);
	mutex_unlock(&av_handler_mutex);


	/* ------ customer report ------- */
	clock_gettime(CLOCK_REALTIME , &t_global_end);
	double wait_time  = (t_wait_end.tv_sec - t_start.tv_sec) + (t_wait_end.tv_nsec - t_start.tv_nsec) / BILLION;
	double total_time = (t_global_end.tv_sec - t_start.tv_sec) + (t_global_end.tv_nsec - t_start.tv_nsec) / BILLION;

	m_wait_time += wait_time;
	m_total_time += total_time;

	mutex_lock(&report_state_mutex);

	/*
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
	*/

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
		customers[i].zoneId = -1;
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


void Init(char * argv[])
{



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

	// Initialize availbe handlers && cashers
	av_customer_handlers = n_tel;
	av_customer_cashers  = n_cash;

	// Initialize seats
	zoneSize[0] = n_zoneA;
	zoneSize[1] = n_zoneB;
	zoneSize[2] = n_zoneC;

	free_seats[0] = zoneSize[0] * n_seat;
	free_seats[1] = zoneSize[1] * n_seat;
	free_seats[2] = zoneSize[2] * n_seat;

	for(int i = 0; i<3; i++)
	{
		zones[i] = malloc( free_seats[i] * sizeof(int));
		if(!zones[i])
		{
			printf("\n-Error : zones[i]::malloc() failed!");
			printf("\n--Exiting program...");
			exit(-1);
		}

		for(int j = 0; j<free_seats[i]; j++)
		{
			zones[i][j] = 0;
		}
	}

	// Init Mutexes && cond_variables
	int rc;
	rc = pthread_mutex_init(&mutex0 , NULL);
	checkOperationStatus(_mutex_init , rc , 1);

	rc = pthread_mutex_init(&av_handler_mutex , NULL);
	checkOperationStatus(_mutex_init  , rc , 1);

	rc = pthread_mutex_init(&av_cashers_mutex , NULL);
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

	rc = pthread_cond_init(&av_cashers_cond , NULL);
	checkOperationStatus(_thread_cond_init , rc , 1);



}


void cleanUp()
{
	int rc;

	free(customers);

	rc = pthread_mutex_destroy(&mutex0);
    checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&av_handler_mutex);
	checkOperationStatus(_mutex_destroy , rc , 1);

	rc = pthread_mutex_destroy(&av_cashers_mutex);
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

	rc = pthread_cond_destroy(&av_cashers_cond);
	checkOperationStatus(_thread_cond_destroy , rc , 1);


}


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
