#include "qflex/qflex.h"
#include "../libqflex/api.h"

bool qflex_inst_done = false;
bool qflex_prologue_done = false;
uint64_t qflex_prologue_pc = 0xDEADBEEF;

void qflex_api_values_init(CPUState *cpu) {
    qflex_inst_done = false;
    qflex_prologue_done = false;
    qflex_prologue_pc = cpu_get_program_counter(cpu);
}
