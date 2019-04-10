
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
		cust->msg = "-Seats request rejected | Reason : not enough available seats!";
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
					av_seats_count++;
				}
				else if(zones[zoneId][i * n_seat + j] != 0 && av_seats_count > 0)
				{
					av_seats_count = 0;
				}

				if(av_seats_count == seats_count)
				{
					seats_found = 1;
					//free_seats[zoneId] -= seats_count;
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

	if(!approve)
	{
		cust->msg = "-Seats request rejected | Reason : couldn't allocate enough seats in the same line!";
	}
	mutex_unlock(&seats_access_mutex);
	return approve;
}

void bindRequestedSeats(Customer * cust)
{

	mutex_lock(&seats_access_mutex);
	for(int i = 0; i<cust->seats_count; i++)
	{
		int seat_index = cust->seats_index[i];
		zones[cust->zoneId][ seat_index ] = cust->Id;
	}
	free_seats[cust->zoneId] -= cust->seats_count;
	mutex_unlock(&seats_access_mutex);

}

void unBindRequestedSeats(Customer * cust)
{
	mutex_lock(&seats_access_mutex);
	for(int i = 0; i<cust->seats_count; i++)
	{
		int seat_index = cust->seats_index[i];
		zones[cust->zoneId][ seat_index ] = 0;
	}
	free_seats[cust->zoneId] += cust->seats_count;
	mutex_unlock(&seats_access_mutex);
}

int approvePaymentRequest()
{
	sleep(getRandom(t_cashMin , t_cashMax));
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
	t_casher_start,
	t_global_end,
	t_wait_end,
	t_casher_end;
	clock_gettime(CLOCK_REALTIME , &t_start);


	/*
	  * mutex_lock()
	  * check for available customer handlers
	  * if (av_customer_handlers > 0 ) -> customer is being handled
	  * else -> customer waits in the queue until a customer handler is free
	*/
	mutex_lock(&av_handler_mutex);
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


    // check customer's seats request
	if( approveSeatsRequest(cust))
	{
		// bind requested seats
		bindRequestedSeats(cust);

        clock_gettime(CLOCK_REALTIME , &t_casher_start);
		mutex_lock(&av_cashers_mutex);
		while(av_customer_cashers == 0)
		{
			cond_wait(&av_cashers_cond , &av_cashers_mutex);
		}

		av_customer_cashers--;
		mutex_unlock(&av_cashers_mutex);
		clock_gettime(CLOCK_REALTIME , &t_casher_end);

		if( approvePaymentRequest() )
		{
			int money_to_pay = cust->seats_count * zone_costs[cust->zoneId];
			transferMoneyToAccount( money_to_pay , tid);
			cust->payment_value = money_to_pay;
			cust->payment_success = 1;
		}
		else // customer's seats request gets rejected and binded seats return to the seatsPlan
		{
			unBindRequestedSeats(cust);
			cust->payment_success = 0;
			cust->msg = "-Seats reservation rejected | Card Payment failure!";
		}

		mutex_lock(&av_cashers_mutex);
		av_customer_cashers++;
		pthread_cond_signal(&av_cashers_cond);
		mutex_unlock(&av_cashers_mutex);

	}
	else
	{

	}


	// again , we have to mutex_lock() in order to access shared variable
	mutex_lock(&av_handler_mutex);
	av_customer_handlers++;
	// broadcasting signal for all the customers in 'queue' so they can get handled by the free customerHandler!
	pthread_cond_signal(&av_handler_cond);	//cond_broadcast(&av_handler_cond);
	mutex_unlock(&av_handler_mutex);


	/* ------ customer report ------- */
	clock_gettime(CLOCK_REALTIME , &t_global_end);
	double wait_time_h  = (t_wait_end.tv_sec - t_start.tv_sec) + (t_wait_end.tv_nsec - t_start.tv_nsec) / BILLION;
	double wait_time_c  = (t_casher_end.tv_sec - t_casher_start.tv_sec) + (t_casher_end.tv_nsec - t_casher_start.tv_nsec) / BILLION;
	double wait_time = (wait_time_h + wait_time_c)/2.0d;

	double total_time = (t_global_end.tv_sec - t_start.tv_sec) + (t_global_end.tv_nsec - t_start.tv_nsec) / BILLION;

	m_wait_time += wait_time;
	m_total_time += total_time;

	mutex_lock(&report_state_mutex);

	printf("\n\n   ____Customer Report____");
	printf("\n-customerId : %i" , tid);
	if(cust->payment_success)
	{
		printf("\n-Seats reservation -> succesfull ! ");
		printf("\n      - transferId : %i" , tid);
		printf("\n      - Seats Reserved at %s : ", zoneNames[cust->zoneId]);
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

	printf("\n--Wait time on handler  : %f" , wait_time_h);
	printf("\n--Wait time on casher   : %f" , wait_time_c);
	printf("\n--Total wait time       : %f" , wait_time);
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
	printf("\n\n      -Total Customers handled : %i" , customers_count);
	printf("\n      -Total Balance : %i" , balance);
	printf("\n\n -------- Theatre SeatsPlan --------");

	for(int i = 0; i<3; i++)
	{
		printf("\n ==========| %s |==========\n" , zoneNames[i] );

		for(int l = 0; i<zonesSize[i]; l++)
		{
			printf("LINE%i   " , l);
		}
		printf("\n");
		for(int seat = 0; seat<n_seat; seat++)
		{

			for(int line = 0; line<zonesSize[i]; line++)
			{
				int customer_id = zones[i][n_seat * line + seat];
				if(customer_id != 0)
				{
					printf("[%i:customer%i]  " , seat , customer_id );
				}
				else
				{
					printf("[%i:free]  " , seat );
				}
			}
			printf("\n");
		}
		printf("\n\n");
	}

	/*
	printf("\n ==========| %s |==========" , zoneNames[i] );
	for(int line = 0; line<zoneSize[i]; line++)
	{
		printf("\n ----------[ Line : %i ]----------\n" , line);
		for(int seat = 0; seat<n_seat; seat++)
		{
			int customer_id = zones[i][n_seat * line + seat];
			if(customer_id != 0)
			{
				printf("[ %i : customer%i ]  " , seat , customer_id );
			}
			else
			{
				printf("[ %i : free ]  " , seat );
			}
		}
		printf("\n");
	}
	printf("\n\n");
	*/

	printf("\n -------- Statistics -------- \n");
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

	zone_costs[0] = c_zoneA;
	zone_costs[1] = c_zoneB;
	zone_costs[2] = c_zoneC;

	zoneNames[0] = "zoneA";
	zoneNames[1] = "zoneB";
	zoneNames[2] = "zoneC";

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
