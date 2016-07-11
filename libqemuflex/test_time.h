#ifndef TEST_TIME_H
#define TEST_TIME_H

#include <inttypes.h>

extern int64_t test_start_time;
extern int64_t test_end_time;
extern int64_t test_start_ins;
extern int64_t test_num_ins;
extern int64_t test_ins_limit;

int64_t clock_get_current_time_us(void);
void test_time_check_end(void);

extern int64_t test_ls_ins;
extern int64_t test_cumulative_ls_time;
extern int64_t test_brif_ins;
extern int64_t test_cumulative_brif_time;

#endif /* TEST_TIME_H */
