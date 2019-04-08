#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct PRODUCT_ARGS
{
    int a;
    int b;

}PRODUCT_ARGS;

void * mult(void * f_args)
{
    PRODUCT_ARGS * args = (PRODUCT_ARGS *)f_args;
    int result = args->a * args->b;

    printf("\n     ~Thread : (%i) * (%i) = %i\n\n" , args->a , args->b , result);
    pthread_exit(NULL);

}

int main(int argc , char * argv[])
{

    printf("\n\n ----------- Main -----------\n");

    PRODUCT_ARGS product_args;
    product_args.a = 4;
    product_args.b = 32;

    pthread_t thread1 , thread2;
    pthread_t threads * = malloc(2 * sizeof(pthread_t));

    if ( pthread_create(&threads[0] , NULL , mult , &product_args) != 0)
    {
        printf("\n-Error : Failed to create thread1!");
        exit(-1);
    }

    PRODUCT_ARGS product_args2;
    product_args2.a = 10;
    product_args2.b = 5;

    if ( pthread_create(&threads[1] , NULL , mult , &product_args2) !=0)
    {
        printf("\n-Error : Failed to create thread2!");
        exit(-1);
    }

    if ( pthread_join(threads[0] , NULL) != 0)
    {
        printf("\n-Error : Failed to join() thread1!");
        exit(-1);
    }

    if ( pthread_join(threads[1] , NULL) != 0)
    {
        printf("\n-Error : Failed to join() thread2!");
        exit(-1);
    }

    printf("\n");
    return 0;
}
