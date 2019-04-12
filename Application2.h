#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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



#define BILLION        1000000000L;

#define n_seat         10
#define n_zoneA        5
#define n_zoneB        10
#define n_zoneC        10

#define p_zoneA        20
#define p_zoneB        40
#define p_zoneC        40

#define c_zoneA        30
#define c_zoneB        25
#define c_zoneC        20

#define n_tel          8
#define n_cash         4

#define n_seatMin      1
#define n_seatMax      5

#define t_seatMin      5
#define t_seatMax      10

#define t_cashMin      2
#define t_cashMax      4

#define p_cardSuccess  90



// Business variables
int balance;

int *  zones[3];          // 3 seat zones { zoneA , zoneB , zoneC }
int    zoneSize[3];       // size of each zone { sizeA , sizeB , sizeC }
int    free_seats[3];     // free seats of each zone
int    zone_costs[3];     // seat cost of each zone
char * zoneNames[3];      // name of each zone { "zoneA" , "zoneB" , "zoneC" }


// customer handlers && cashers
int av_customer_handlers;
int av_customer_cashers;


// struct defining a customer
typedef struct Customer
{
	pthread_t thread;              // customer's thread

	int     Id;                    // customer's Id ( = thread's Id )
	int     zoneId;                // customer's corresponding zone Id
	int     seats_count;           // number of seats requested
	int *   seats_index;           // binded seats index
	int     payment_success;       // 1 for card_payment success , 0 else.
	int     payment_value;         // payment value according to the binded seats
	char *  msg;                   //

}
Customer;

// customers data structure
Customer * customers;
int customers_count;


int seed;
double m_wait_time;
double m_total_time;

struct timespec t_execution_start , t_execution_end;
double total_execution_time;

// Mutexes && cond_variables
pthread_mutex_t av_handler_mutex ,
pthread_mutex_t seats_access_mutex;
pthread_mutex_t balance_access_mutex;
pthread_mutex_t report_state_mutex;
pthread_cond_t av_handler_cond;


// Mutexes && cond_variables
pthread_mutex_t av_handler_mutex;        // mutex         for -> shared variable   : available_handlers
pthread_mutex_t av_cashers_mutex;        // mutex         for -> shared variable   : available_cashers
pthread_mutex_t seats_access_mutex;      // mutex         for -> shared variable   : seatsPlan
pthread_mutex_t balance_access_mutex;    // mutex         for -> shared variable   : business_balance
pthread_mutex_t report_state_mutex;      // mutex         for -> shared state      : reporting customer's state
pthread_cond_t av_handler_cond;          // cond_variable for -> waiting condition : on available handlers
pthread_cond_t av_cashers_cond;          // cond_variable for -> waiting condition : on available cashers

//-------------------------------------
//
//      Customer Service Functions
//
//-------------------------------------

int approveSeatsRequest(Customer * cust);
void bindRequestedSeats(Customer * cust);
void unBindRequestedSeats(Customer * cust);
int approvePaymentRequest();
void transferMoneyToAccount(int money , int customerId);
void * handleCustomer(void * customer);

//-------------------------------------
//
//        Helper Functions
//
//-------------------------------------

int getRandom(int min , int max);
void checkOperationStatus(enum Operation op ,  int rc , int return_type);
void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void cond_wait(pthread_cond_t *cond , pthread_cond_t *mutex);
void cond_broadcast(pthread_cond_t *cond);
void Init(char * argv[]);
void cleanUp();
