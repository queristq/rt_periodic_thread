#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define TIMESPEC_ADD(A,B) /* A += B */ \
    do {                                   \
    (A).tv_sec += (B).tv_sec;          \
    (A).tv_nsec += (B).tv_nsec;        \
    if ( (A).tv_nsec >= 1000000000 ) { \
    (A).tv_sec++;                  \
    (A).tv_nsec -= 1000000000;     \
    }                                  \
    } while (0)

#define TIMESPEC_SUB(A,B) /* A -= B */ \
    do {                                   \
    (A).tv_sec -= (B).tv_sec;          \
    (A).tv_nsec -= (B).tv_nsec;        \
    if ( (A).tv_nsec < 0 ) {           \
    (A).tv_sec--;                  \
    (A).tv_nsec += 1000000000;     \
    }                                  \
    } while (0)


void *thread_func ( void *param )
{
    FILE *fp;
    char filename_fp[50];

    long thread_id = (long) param;

    sprintf (filename_fp, "%ld%s", thread_id, ".txt");
    fp = fopen(filename_fp, "w");//opening file

    struct timespec t_next, period, t_now, t_prev, t_diff;

    /* period = 1 ms * thread_id */
    period.tv_sec = 0;
    period.tv_nsec = ( 1 ) * 500000; // a x ms

    clock_gettime ( CLOCK_MONOTONIC, &t_now );
    t_next = t_now;
    t_prev = t_now;

    int t_jitter[100];
    for (int i=0;i<100;i++) t_jitter[i] = 0;
    int t_j = 0;

    for ( int i = 0; i < 100000; i++ )
    {
        clock_gettime ( CLOCK_MONOTONIC, &t_now );
        t_diff = t_now;
        TIMESPEC_SUB ( t_diff, t_prev );
        t_prev = t_now;
        t_j = t_diff.tv_nsec / 10000;
        if (t_j<0) t_jitter[0]++;
        else if (t_j>99) t_jitter[99]++;
        else t_jitter[t_j]++;
        if(i%10000==1) printf("%d\n",i);
        TIMESPEC_ADD ( t_next, period );
        clock_nanosleep ( CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next, NULL );
    }

    for (int i=0;i<100;i++) {
        printf("%d ",t_jitter[i]);
        fprintf(fp, "%d\n",t_jitter[i]);
    }
    fclose(fp);
    return NULL;
}

int main ()
{
    int policy;
    struct sched_param prio;
    pthread_attr_t attr;

    pthread_t tid1;
    pthread_t tid2;

    policy = SCHED_OTHER;
    if (pthread_setschedparam( pthread_self(),policy, &prio )){
        perror ("Error: pthread_setschedparam (root permission?)");
        exit(1);
    }

    pthread_attr_init( &attr);
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED);
    policy = SCHED_FIFO;
    pthread_attr_setschedpolicy( &attr, policy);
    prio.sched_priority = 99; // priority range should be btw -20 to +19
    pthread_attr_setschedparam(&attr,&prio);

    if ( pthread_create(&tid1, &attr, thread_func, (void *)(1)) ){
        perror ( "Error: pthread1_create" );
        return 1;
    }

    pthread_attr_init( &attr);
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED);
    policy = SCHED_FIFO;
    pthread_attr_setschedpolicy( &attr, policy);
    prio.sched_priority = 1; // priority range should be btw -20 to +19
    pthread_attr_setschedparam(&attr,&prio);

    if ( pthread_create(&tid2, &attr, thread_func, (void *)(2)) ){
        perror ( "Error: pthread2_create" );
        return 1;
    }

    /* wait for threads to finish */
    pthread_join ( tid1, NULL );
    pthread_join ( tid2, NULL );

    return 0;
}
