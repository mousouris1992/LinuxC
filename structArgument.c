#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct PRODUCT_ARGS
{
    int a;
    int b;
    int Id;

}PRODUCT_ARGS;

void * mult(void * f_args)
{
    PRODUCT_ARGS * args = (PRODUCT_ARGS *)f_args;
    int result = args->a * args->b;

    printf("\n     ~Thread#%i : (%i) * (%i) = %i\n\n" , args->Id, args->a , args->b , result);
    pthread_exit(NULL);

}

int main(int argc , char * argv[])
{

    printf("\n\n ----------- Main -----------\n");


    int count;
    if(argc < 2)
    {
        count = 10;
    }
    else
    {
        count = atoi(argv[1]);
    }
    //pthread_t * threads = malloc(count * sizeof(pthread_t));
    pthread_t threads[count];

    srand(time(NULL));
    PRODUCT_ARGS product_args[count];

    printf("\n--Creating threads ...");
    for(int i = 0; i<count; i++)
    {
        product_args[i].a  = rand()%20 + 1;
        product_args[i].b  = rand()%20 + 1;
        product_args[i].Id = i;

        if ( pthread_create(&threads[i] , NULL , mult , &product_args[i]) != 0)
        {
            printf("\n-Error : Failed to create thread#%i!" , i);
            exit(-1);
        }

        printf("\n     --Creating thread--");
        printf("\n       -Thread id : %i",i);
        printf("\n       -Arguments { %i , %i }\n" , product_args[i].a , product_args[i].b);
    }

    for(int i = 0; i<count; i++)
    {
        if ( pthread_join(threads[i] , NULL) != 0)
        {
            printf("\n-Error : Failed to join() thread!");
            exit(-1);
        }
        printf("\n--Joining() thread#%i" , i);
    }


    //free(threads);

    printf("\n");
    return 0;
}
