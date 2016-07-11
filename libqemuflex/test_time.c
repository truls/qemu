#include "test_time.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int64_t test_start_time = -1;
int64_t test_end_time   = -1;
int64_t test_start_ins  = -1;
int64_t test_num_ins    =  0;
int64_t test_ins_limit  = -1;

static int test_started = 0;

int64_t test_ls_ins = 0;
int64_t test_cumulative_ls_time = 0;
int64_t test_brif_ins = 0;
int64_t test_cumulative_brif_time = 0;

static int file_exists( const char* filename ) {
  FILE* fd = fopen(filename, "r");
  if( fd != NULL ) {
    fclose(fd);
    return 1;
  }

  return 0;
}

int64_t clock_get_current_time_us(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (int64_t)((int64_t)time.tv_sec * 1000000 + time.tv_nsec / 1000);
}

void test_time_check_end(void) {
  if( test_num_ins > test_start_ins && !test_started && test_start_ins > -1 ) {
    test_started = 1;
    test_start_ins = test_num_ins;
    test_start_time = clock_get_current_time_us();
  }

  if( (test_num_ins - test_start_ins) > test_ins_limit && test_started ) {
    test_end_time = clock_get_current_time_us();
    int64_t diff_time_us = test_end_time - test_start_time;
    float mips = (float)(test_num_ins - test_start_ins) / (float)diff_time_us;
    printf("Tested MIPS: %f\n", mips);
    printf("Diff time (us): %ld\n", diff_time_us);
    printf("Num ins: %ld\n", test_num_ins );

    FILE* mips_file = NULL;
#if defined(CONFIG_TEST_LS) && defined(CONFIG_TEST_FETCH)
    const char* mips_file_path = "mips_fetchls.m";
#elif defined(CONFIG_TEST_LS)
    const char* mips_file_path = "mips_ls.m";
#elif defined(CONFIG_TEST_FETCH)
    const char* mips_file_path = "mips_fetch.m";
#else
#error Measuring what?
#endif

    if( !file_exists( mips_file_path ) ) {
      mips_file = fopen( mips_file_path, "w" );
#if defined(CONFIG_TEST_LS) && defined(CONFIG_TEST_FETCH)
    fprintf( mips_file, "invmips_fetchls = [ " );
#elif defined(CONFIG_TEST_LS)
    fprintf( mips_file, "invmips_ls = [ " );
#elif defined(CONFIG_TEST_FETCH)
    fprintf( mips_file, "invmips_fetch = [ " );
#else
#error Measuring what?
#endif
      fclose( mips_file );
    }

    mips_file = fopen( mips_file_path, "a" );
    fprintf( mips_file, "%f; ",  1.0f / mips );

    fclose( mips_file );

#if defined(CONFIG_TEST_PROP)
    float ls_ratio = (float)test_ls_ins / test_num_ins;
    float brif_ratio =  (float)test_brif_ins / test_num_ins;

    printf("L/S frequency %f\n", ls_ratio);
    printf("BR/IF frequency %f\n", brif_ratio);

    FILE* prop_file = NULL;
#if defined(CONFIG_TEST_LS) && defined(CONFIG_TEST_FETCH)
    const char* prop_file_path = "prop_fetchls.m";
#elif defined(CONFIG_TEST_LS)
    const char* prop_file_path = "prop_ls.m";
#elif defined(CONFIG_TEST_FETCH)
    const char* prop_file_path = "prop_fetch.m";
#else
#error Measuring what?
#endif
    if( !file_exists( prop_file_path ) ) {
      prop_file = fopen( prop_file_path, "w" );
#if defined(CONFIG_TEST_LS) && defined(CONFIG_TEST_FETCH)
    fprintf( prop_file, "proportions_fetchls = [ " );
#elif defined(CONFIG_TEST_LS)
    fprintf( prop_file, "proportions_ls = [ " );
#elif defined(CONFIG_TEST_FETCH)
    fprintf( prop_file, "proportions_fetch = [ " );
#else
#error Measuring what?
#endif
      fclose( prop_file );
    }

    prop_file = fopen( prop_file_path, "a" );
    fprintf( prop_file, "%f %f; ", ls_ratio, brif_ratio );

    fclose( prop_file );
#endif
#if defined(CONFIG_TEST_LS_TIME)
    printf("L/S average time (us) %f\n", (float)test_cumulative_ls_time / (float)test_ls_ins );
#endif
#if defined(CONFIG_TEST_FETCH_TIME)
    printf("BR/IF average time (us) %f\n", (float)test_cumulative_brif_time / (float)test_ls_ins );
#endif
    exit(0);
  }
}
