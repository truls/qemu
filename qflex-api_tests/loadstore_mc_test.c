#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)

pthread_t traversing_thread ;
struct arg_struct {

uint64_t* arr ;
uint64_t size ;
int thread_number ;

} ;

void* access( void *arguments )
{
       struct arg_struct *args = (struct arg_struct *)arguments;
       uint64_t i , upper_limit , lower_limit , tmp , size ;
       size = args->size ;
       uint64_t* arr = args->arr ;
       int c = args->thread_number ;
      if ( c == 1 ){  lower_limit = 0; upper_limit = (uint64_t) ( size*0.75) ; }
      else {  upper_limit = size ; lower_limit = (uint64_t) ( size*0.25) ; }

      printf ( " Hello I am Pthread #%ld. Ready to traverse now \n" , gettid())  
;
      printf ( " My thread number is %d\n " , c ) ;
      printf ( " My lower limit is %llu \n " , lower_limit ) ;
      printf ( " My upper limit is %llu \n " , upper_limit ) ;

     while(1)
     {
        // for( i = lower_limit ; i<=upper_limit ; i = i+64)
        // {
        //           tmp = arr[i] ;
        // }

          if(c==1)
         {
            __asm__ __volatile__(

                               "OUTER_LOOP:\n\t"
                               "MOV %x[ind], %x[lower_limit]\n\t"
                               "INNER_LOOP:\n\t"
                               "ADD x0, %x[arr], %x[ind]\n\t"
                               "LDR %x[tmp], [x0]\n\t"
                               "ADD %x[ind], %x[ind], #64\n\t"
                               "CMP %x[ind], %x[upper_limit]\n\t"
                               "B.NE INNER_LOOP\n\t"
                               "FINISH:\n\t"
                               : [ind] "+r" (i), [tmp] "=r" (tmp)
                               : [arr] "r" (arr), [lower_limit] "r" (lower_limit), [upper_limit] "r" (upper_limit)
                               : "x0"
                               );

        } // end of if
     
       else 
      {
            __asm__ __volatile__(

                               "OUTER_LOOP:\n\t"
                               "MOV %x[ind], %x[lower_limit]\n\t"
                               "INNER_LOOP:\n\t"
                               "ADD x0, %x[arr], %x[ind]\n\t"
                               "LDR %x[tmp], [x0]\n\t"
                               "STR %x[tmp], [x0]\n\t"
                               "ADD %x[ind], %x[ind], #64\n\t"
                               "CMP %x[ind], %x[upper_limit]\n\t"
                               "B.NE INNER_LOOP\n\t"
                               "FINISH:\n\t"
                               : [ind] "+r" (i), [tmp] "=r" (tmp)
                               : [arr] "r" (arr), [lower_limit] "r" (lower_limit), [upper_limit] "r" (upper_limit)
                               : "x0"
                               );

        } // end of else 
     
     }//end of while



}

int main(int argc, char **argv)
{
   if ( argc != 3) {
printf("Usage: %s <array_size_in_KB> <use_random_accesses: 1 or 0>\n", argv[0]);
        return 1;
    }

    printf ( "Hi\n") ;
    uint64_t size = atoi(argv[1]) * 1024; // Size is size in Bytes
    int is_rand = atoi(argv[2]);

    uint64_t* arr = malloc(size * sizeof(uint64_t));

    uint64_t i = 0, tmp = 5;

    struct arg_struct args1 ;
    struct arg_struct args2 ;
     args1.arr = arr ;
    args1.size = size ;
     args2.arr = arr ;
    args2.size = size ;
   
     for (i = 0; i < size; i++) {
        arr[i] = i;
    }

    printf("Array size is %llu bytes.\nInit done!\n", size);
    int rc ;
    args1.thread_number = 1 ;
    rc = pthread_create(&traversing_thread , NULL , access , (void*)(&args1) ) ;
    args2.thread_number = 2 ;
    rc = pthread_create(&traversing_thread , NULL , access ,(void* )(&args2) ) ;
    while(1) { }
}

