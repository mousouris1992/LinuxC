

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS  3
#define TCOUNT 24
#define COUNT_LIMIT 12

int count = 0;
int signalSent = 0;
int isDoubleCounterThreadFinished = 0;
int isDoubleCounterThreadStarted = 0;
pthread_mutex_t countMutex;
pthread_cond_t countThresholdCondition;

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


void checkOperationStatus(enum Operation op , int rc , int return_type)
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


void * increaseCount(void *t)
{

	int *threadId = (int *)t;
	int rc;

	rc = pthread_mutex_lock(&countMutex);
    checkOperationStatus( mutex_lock , rc , 0);


	//an to thread pou tha diplasiazei ton counter den exei ksekinisei perimene mexris
	//otou se eidopoiisei ((xrisi while gia apofygi Spurious Wakeup http://en.wikipedia.org/wiki/Spurious_wakeup))
	while (isDoubleCounterThreadStarted == 0)
    {
		printf("increaseCount(): thread %d, the thread that will double the counter has not started about to wait...\n", *threadId);

        rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
        checkOperationStatus( thread_cond_wait , rc , 0);

		printf("increaseCount(): thread %d, the thread that will double the counter has started.\n",*threadId);
	}

	rc = pthread_mutex_unlock(&countMutex);
    checkOperationStatus( mutex_lock ,rc , 0);

	for (int i=0; i < TCOUNT; i++)
    {
		//lock to mutex gia na mporesei na allaksei xwris provlima twn count.
 		rc = pthread_mutex_lock(&countMutex);
        checkOperationStatus( mutex_lock ,rc , 0);


		//molis ftaseis to katallilo limit steile sima gia na ksypnisei i methodos
		//pou tha diplasiasei ton counter.
		if (count == COUNT_LIMIT)
        {
            printf("increaseCount(): thread %d, count = %d  Threshold reached.\n", *threadId, count);

			//ena mono thread prpepei na eidopoiisei to thread tou
			//diplasiasmou.
			if (signalSent == 0)
            {
      			rc = pthread_cond_signal(&countThresholdCondition);
				checkOperationStatus(thread_cond_signal ,rc , 0);

      			printf("increaseCount(): thread %d just sent signal to the doubleCounter thread.\n", *threadId);
				signalSent = 1;
			}

			//perimene mexri na ginei o diplasiasmos (xrisi while gia
			//apofygi Spurious Wakeup
			while (isDoubleCounterThreadFinished == 0)
            {
				printf("increaseCount(): thread %d, waiting for signal.\n", *threadId);
				rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
				checkOperationStatus(thread_cond_wait ,rc , 0);
			}
      	}

		count++;

		printf("increaseCount(): thread %d, count = %d, unlocking mutex\n",*threadId, count);
    	rc = pthread_mutex_unlock(&countMutex);
		checkOperationStatus(mutex_unlock , rc , 0);
    }

	pthread_exit(t);
}

/**
 * H synartisi diplasiasmou tou counter.
*/
void * doubleCountVariable(void *t)
{

	int *threadId = (int *) t;
	printf("doubleCountVariable(): thread %d started.\n", *threadId);
	int rc;
	rc = pthread_mutex_lock(&countMutex);
    checkOperationStatus(mutex_lock , rc , 0);

	isDoubleCounterThreadStarted = 1;
	printf("doubleCountVariable(): thread %d about to inform all other threads.\n", *threadId);

	//eidopoiei ta alla threads oti exei ksekinisei.
	rc = pthread_cond_broadcast(&countThresholdCondition);
	checkOperationStatus( thread_cond_broadcast , rc , 0);

	printf("doubleCountVariable(): thread %d informed the other threads\n", *threadId);

	//oso den ikanopoieitai i synthiki perimene (xrisi while gia apofygi Spurious Wakeup http://en.wikipedia.org/wiki/Spurious_wakeup)
	while (count < COUNT_LIMIT)
    {
		printf("doubleCountVariable(): thread %d going into wait...\n", *threadId);
    	rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
		checkOperationStatus(thread_cond_wait , rc , 0);

		printf("doubleCountVariable(): thread %d Condition signal received.\n", *threadId);
	}

	count *= 2;
    printf("doubleCountVariable(): thread %d count now = %d.\n", *threadId, count);

	isDoubleCounterThreadFinished = 1;
	rc = pthread_cond_broadcast(&countThresholdCondition);
    checkOperationStatus( thread_cond_broadcast , rc , 0);


	printf("doubleCountVariable(): thread %d sent signal to increaseCount threads.\n", *threadId);

	rc = pthread_mutex_unlock(&countMutex);
	checkOperationStatus( mutex_unlock , rc , 0);

	pthread_exit(t);
}


int main(int argc, char *argv[])
{
	int rc;
	int t1 = 1, t2 = 2, t3 = 3;
	pthread_t threads[3];

  	/*arxikopoiisi tou mutex kai tou condition*/
  	rc = pthread_mutex_init(&countMutex, NULL);
    checkOperationStatus(mutex_init , rc , 1);

  	rc = pthread_cond_init(&countThresholdCondition, NULL);
    checkOperationStatus(thread_cond_init , rc , 1);


	//arxikopoiisi olwn twn threads
	rc = pthread_create(&threads[0], NULL, doubleCountVariable, &t1);
	checkOperationStatus(thread_create , rc , 1);

  	rc = pthread_create(&threads[1], NULL, increaseCount, &t2);
    checkOperationStatus(thread_create , rc , 1);


  	rc = pthread_create(&threads[2], NULL, increaseCount, &t3);
    checkOperationStatus(thread_create , rc , 1);


	void *status;
	/*join gia ola ta threads.*/
  	for (int i = 0; i < NUM_THREADS; i++)
    {
    	rc = pthread_join(threads[i], &status);
        checkOperationStatus(thread_join , rc , 1);

		printf("Main(): Thread %d terminated successfully.\n", *(int *) status);
  	}

	//ektypwsi tis telikis timis tou count
  	printf ("Main(): Waited for %d threads to finish. Final value of count = %d. Done.\n", NUM_THREADS, count);

  	/*"katastrofi" mutex kai condition*/
  	rc = pthread_mutex_destroy(&countMutex);
    checkOperationStatus(mutex_destroy , rc , 1);


 	rc = pthread_cond_destroy(&countThresholdCondition);
    checkOperationStatus(thread_cond_destroy , rc , 1);



	return 1;
}
