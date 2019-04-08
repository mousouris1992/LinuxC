#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 3
#define TCOUNT 24
#define COUNT_LIMIT 12

static int count = 0;
static int signalSent = 0;
static int isDoubleCounterThreadFinished = 0;
static int isDoubleCounterThreadStarted = 0;
