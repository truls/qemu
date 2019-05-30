#ifndef QFLEX_H
#define QFLEX_H

#include <stdbool.h>
#include "qflex/qflex-log.h"

#define QFLEX_EXEC_IN  (0)
#define QFLEX_EXEC_OUT (1)

/** Note for cpu_exec.
  * Depending on the desired type of execution,
  * cpu_exec should break from the double while loop
  * in the correct manner.
  */
typedef enum {
    PROLOGUE,   // Breaks when the Arch State is back to the initial user program
    SINGLESTEP, // Breaks when a single TB (instruction) is executed
    EXCP        // Breaks when the exeception routine is done
} QFlexExecType_t;

extern bool qflex_inst_done;
extern bool qflex_prologue_done;
extern uint64_t qflex_prologue_pc;

void qflex_api_values_init(CPUState *cpu);

/** qflex_cpu_exec
 * mirror cpu_exec, with qflex execution flow control
 * for TCG execution
 */
int qflex_cpu_exec(CPUState *cpu, QFlexExecType_t type);

static inline bool qflex_is_inst_done(void)     { return qflex_inst_done; }
static inline bool qflex_is_prologue_done(void) { return qflex_prologue_done; }
static inline bool qflex_update_prologue_done(uint64_t cur_pc) {
    qflex_prologue_done = (cur_pc == qflex_prologue_pc);
    return qflex_prologue_done;
}
static inline void qflex_update_inst_done(bool done) { qflex_inst_done = done; }

#endif /* QFLEX_H */
