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

    pthread_t thread;
    if ( pthread_create(&thread , NULL , mult , &product_args) != 0)
    {
        printf("\n-Error : Failed to create thread!");
        exit(-1);
    }

    if ( pthread_join(thread , NULL) != 0)
    {
        printf("\n-Error : Failed to join() thread!");
        exit(-1);
    }

    printf("\n");
    return 0;
}
