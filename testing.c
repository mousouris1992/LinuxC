
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int counter = 0;
pthread_mutex_t mutexCount;

void * incCounter(void * threadId)
{
    int *tid = (int*)threadId;
    int prevValue , rc;

    rc = pthread_mutex_lock(&mutexCount);
    if(rc != 0)
    {
        printf("\n-Error : failed to mutex_lock | Return code : %i",rc);
        pthread_exit(&rc);
    }

    printf("\n      ~Thread#%i : About to increase counter!" , *tid);
    prevValue = counter;
    counter++;
    sleep(1);
    printf("\n      ~Thread#%i : prev_counter_value  = %i , after_counter_value = %i ", *tid , prevValue , counter);

    rc = pthread_mutex_unlock(&mutexCount);
    if(rc != 0)
    {
        printf("\n-Error : Failed to mutex_lock() | Return code : %i",rc);
        pthread_exit(&rc);
    }

    pthread_exit(threadId);

}


int main(int argc, char * argv[])
{
    printf("\n\n ----------- Testing -----------\n");

    int size = 5;
    pthread_t threads[size];
    int threadIds[size];
    int rc;

    rc = pthread_mutex_init(&mutexCount , NULL);
    if(rc!=0)
    {
        printf("\n-Error : Failed to mutex_init() | Return code : %i", rc);
        exit(-1);
    }

    //
    for(int i = 0; i<size; i++)
    {
        threadIds[i] = i ;
        printf("\n-Main : creating thread#%i" , i);
        rc = pthread_create(&threads[i] , NULL , incCounter , &threadIds[i]);
        if(rc!=0)
        {
            printf("\n-Error : failed to created_thread() | Return code : %i",rc);
            exit(-1);
        }
    }


    void * status;
    for(int i = 0; i<size; i++)
    {
        rc = pthread_join(threads[i] , &status);
        if(rc!=0)
        {
            printf("\n-Error : failed to join_thread() | Return code : %i",rc);
            exit(-1);
        }

    }

    printf("\n\n-- Counter value after threads finished : %i", counter);


    printf("\n");
    return 0;
}
