#ifdef __cplusplus
extern "C" {
#endif
#ifdef CONFIG_FLEXUS

#ifdef DEBUG // temporary, just to make requirements of dummies clear
#include <assert.h>
#define ASSERT(COND) assert(COND)
#define REQUIRES(COND) assert(COND)
#define ENSURES(COND) assert(COND)
#define dbg_printf(COND printf(COND)
#else
#define ASSERT(...)
#define REQUIRES(...)
#define ENSURES(...)
#define dbg_printf(...)
#endif
 
#include "qemu/osdep.h"            // Added for flexus because in the new version of QEMU qmp-commands.h uses type error that it doesnt call itself . osdep calls it for qmp-commands.h
#include "cpu.h"
#include "qom/cpu.h"
#include "qemu/config-file.h"
#include "sysemu/cpus.h"
#include "qmp-commands.h"

#define QEMUFLEX_PROTOTYPES
#define QEMUFLEX_QEMU_INTERNAL
#include "api.h"

#if !defined(TARGET_I386) && !defined(TARGET_SPARC64) && !defined(TARGET_ARM)
#error "Architecture not supported by QEMUFLEX API!"	
#endif

extern int smp_cpus;

//Functions I am not sure on(wasn't the last person to work on them)
//The callback functions


//Functions that I think do what they should, but are hard to test
//QEMU_read_register
//--SPARC64 -still have no idea what the DUDL register is
//Specific registers that are hard to test.
//--all floating point


//Functions that the formatting might be wrong on (and should be tested more)
//QEMU_read_phys_memory --not sure what all should be taken account of in terms of
//                 --endianess and byte ordering.
//mem_op_is functions --not 100% sure they are right, but I think they are easy


//functions that should be tested more, but I think are correct.
//QEMU_get_cpu_by_index
//QEMU_get_all_processors
//QEMU_get_processor_number
//QEMU_logical_to_physical--might not work with sparc Look at target-sparc/mmu_helper.c:848

//QEMU_get_program_counter

static QEMU_callback_table_t * QEMU_all_callbacks_tables = NULL;

int QEMU_clear_exception(void)
{
    //[???]Not sure what this function can access. 
    //functions in qemu-2.0.0/include/qom/cpu.h might be useful if they are useable
    //not sure if this is how to use FOREACH
//    CPUState *cpu;
//    CPU_FOREACH(cpu){
//        cpu_reset_interrupt(cpu, -1);//but this needs a CPUState and a mask//really have no idea what this does
        //mask = -1 since it should be 0xFFFF...FF for any sized int.
        //and we want to clear all exceptions(?)
        
        //cpu_reset(cpu); // this might be what we want instead--probably not
        //since it completely resets the cpu, so basically rebooting. 
    //}
    //There might not be anything like SIMICS exceptions, so for now leaving this function blank
    return 42;
}

void QEMU_read_register(conf_object_t *cpu, int reg_index, unsigned *reg_size, void *data_out)
{
  REQUIRES(cpu->type == QEMU_CPUState);
  CPUState * qemucpu = cpu->object;
  return cpu_read_register(qemucpu, reg_index, reg_size, data_out);
}

uint64_t QEMU_read_register_by_type(conf_object_t *cpu, int reg_index, int reg_type) {
  REQUIRES(cpu->type == QEMU_CPUState);
  CPUState * qemucpu = cpu->object;
  return readReg(qemucpu, reg_index, reg_type);
}

conf_object_t *QEMU_get_ethernet(void) {
	return NULL;
}

//get the physical memory for a given cpu TODO: WHAT DOES THIS RETURN
//Assuming it returns the AddressSpace of the cpu.
conf_object_t *QEMU_get_phys_memory(conf_object_t *cpu){
    //As far as I can tell it works.
    conf_object_t *as = malloc(sizeof(conf_object_t));
    as->type = QEMU_AddressSpace;
    as->object = (AddressSpace *)cpu_get_address_space_flexus(cpu->object);                      // Changed name here
    return as;
}


uint64_t QEMU_read_phys_memory(conf_object_t *cpu,
		physical_address_t pa, int bytes)
{
  REQUIRES(0 <= bytes && bytes <= 8);
  uint64_t buf;
  cpu_physical_memory_read(pa, &buf, bytes);
  return buf;
}

conf_object_t *QEMU_get_cpu_by_index(int index)
{
	//need to allocate memory 
	conf_object_t *cpu = malloc(sizeof(conf_object_t));
	//where will it get deleted?
        cpu->name = "<placeholder_cpu_name>";  //FIXME: This should be retrieved using a qemu_get_cpu_name(i) function. CPUs should have names, to distinguish CPUs of different machines 
	cpu->object = qemu_get_cpu(index);
	if(cpu->object){
		cpu->type = QEMU_CPUState;
	}
	ENSURES(cpu->type == QEMU_CPUState);
	return cpu;
}

int QEMU_get_processor_number(conf_object_t *cpu){
	CPUState * env = cpu->object;
	return cpu_proc_num(env);
}

conf_object_t *QEMU_get_phys_mem(conf_object_t *cpu) {
  return NULL;
}

uint64_t QEMU_step_count(conf_object_t *cpu){
  return QEMU_get_instruction_count(QEMU_get_processor_number(cpu));
}

void QEMU_flush_all_caches(void){
  return;
}

int QEMU_get_num_cpus(void) {
  unsigned ncpus    = 1;
  unsigned nsockets = 1;
  unsigned ncores   = 1;
  unsigned nthreads = 1;

  QemuOpts * opts = (QemuOpts*)qemu_opts_find( qemu_find_opts("smp-opts"), NULL );
  if( opts != NULL ) {
    // copy and paste from smp_parse
    ncpus    = qemu_opt_get_number(opts, "cpus", 0);
    nsockets = qemu_opt_get_number(opts, "sockets", 0);
    ncores   = qemu_opt_get_number(opts, "cores", 0);
    nthreads = qemu_opt_get_number(opts, "threads", 0);

    /* compute missing values, prefer sockets over cores over threads */
    if (ncpus == 0 || nsockets == 0) {
      nsockets = nsockets > 0 ? nsockets : 1;
      ncores = ncores > 0 ? ncores : 1;
      nthreads = nthreads > 0 ? nthreads : 1;
      if (ncpus == 0) {
	ncpus = ncores * nthreads * nsockets;
      }
    }
  }
  return ncpus;
}

int QEMU_get_num_sockets(void) {
  unsigned nsockets = 1;
  QemuOpts * opts = (QemuOpts*)qemu_opts_find( qemu_find_opts("smp-opts"), NULL );
  if( opts != NULL )
    nsockets = qemu_opt_get_number(opts, "sockets", 0);
  return  nsockets > 0 ? nsockets : 1;
}

int QEMU_get_num_cores(void) {
  unsigned ncpus    = 1;
  unsigned nsockets = 1;
  unsigned ncores   = 1;
  unsigned nthreads = 1;

  QemuOpts * opts = (QemuOpts*)qemu_opts_find( qemu_find_opts("smp-opts"), NULL );
  if( opts != NULL ) {
    // copy and paste from smp_parse
    ncpus    = qemu_opt_get_number(opts, "cpus", 0);
    nsockets = qemu_opt_get_number(opts, "sockets", 0);
    ncores   = qemu_opt_get_number(opts, "cores", 0);
    nthreads = qemu_opt_get_number(opts, "threads", 0);

    /* compute missing values, prefer sockets over cores over threads */
    if (ncpus == 0 || nsockets == 0) {
      ncores = ncores > 0 ? ncores : 1;
    } else {
      if (ncores == 0) {
	nthreads = nthreads > 0 ? nthreads : 1;
	ncores = ncpus / (nsockets * nthreads);
      }
    }
  }
  return ncores;
}

int QEMU_get_num_threads_per_core(void) {
  unsigned ncpus    = 1;
  unsigned nsockets = 1;
  unsigned ncores   = 1;
  unsigned nthreads = 1;

  QemuOpts * opts = (QemuOpts*)qemu_opts_find( qemu_find_opts("smp-opts"), NULL );
  if( opts != NULL ) {
    // copy and paste from smp_parse
    ncpus    = qemu_opt_get_number(opts, "cpus", 0);
    nsockets = qemu_opt_get_number(opts, "sockets", 0);
    ncores   = qemu_opt_get_number(opts, "cores", 0);
    nthreads = qemu_opt_get_number(opts, "threads", 0);

    /* compute missing values, prefer sockets over cores over threads */
    nthreads = nthreads > 0 ? nthreads : 1;
  
    if (ncpus != 0 && nsockets != 0 && ncores != 0) {
      nthreads = ncpus / (ncores * nsockets);
    }
  }
  return nthreads;
}

// return the id of the socket of the processor
int QEMU_cpu_get_socket_id(conf_object_t *cpu) {
  int threads_per_core = QEMU_get_num_threads_per_core();
  int cores_per_socket = QEMU_get_num_cores();
  int cpu_id = QEMU_get_processor_number(cpu);
  return cpu_id / (cores_per_socket * threads_per_core);
}

// return the core id of the processor
int QEMU_cpu_get_core_id(conf_object_t *cpu) {
  int threads_per_core = QEMU_get_num_threads_per_core();
  int cores_per_socket = QEMU_get_num_cores();
  int cpu_id = QEMU_get_processor_number(cpu);
  return (cpu_id / threads_per_core) % cores_per_socket; 
}

// return the thread id of the processor
int QEMU_cpu_get_thread_id(conf_object_t *cpu) {
  int threads_per_core = QEMU_get_num_threads_per_core();
  int cpu_id = QEMU_get_processor_number(cpu);
  return cpu_id % threads_per_core;
}

conf_object_t *QEMU_get_all_processors(int *numCPUs) {
  // copy and paste from smp_parse
  unsigned ncpus = QEMU_get_num_cpus();

  (*numCPUs) = ncpus;
  int *indexes = (int*)malloc( sizeof(int)*ncpus);
  cpu_pop_indexes(indexes);
  conf_object_t * cpus = malloc(sizeof(conf_object_t)*ncpus);

  int i = 0;
  for( i = 0; i < ncpus; i++ ) {
    cpus[i].name = "<placeholder_cpu_name>";  //FIXME: This should be retrieved using a qemu_get_cpu_name(i) function. CPUs should have names, to distinguish CPUs of different machines
    cpus[i].object = qemu_get_cpu(indexes[i]);
    if(cpus[i].object){//probably not needed error checking
      cpus[i].type = QEMU_CPUState;
    }
    i++;
  }
  free(indexes);
  return cpus;
}

int QEMU_set_tick_frequency(conf_object_t *cpu, double tick_freq) 
{
	return 42;
}

double QEMU_get_tick_frequency(conf_object_t *cpu){
	return 3.14;
}


uint64_t QEMU_get_program_counter(conf_object_t *cpu) 
{
	REQUIRES(cpu->type == QEMU_CPUState);
	//[???]In QEMU the PC is updated only after translation blocks not sure where it is stored
	//1194     *cs_base = env->segs[R_CS].base;
	//1195     *pc = *cs_base + env->eip;
	//from cpu.h.
	//This looks like how they get the cpu there so I will just do that here(env_ptr is a CPUX86State pointer
	//which is what I think cpu->object is.
	//requires x86

	CPUState * qemucpu = cpu->object;
	return cpu_get_program_counter(qemucpu);
}

physical_address_t QEMU_logical_to_physical(conf_object_t *cpu, 
		data_or_instr_t fetch, logical_address_t va) 
{
  REQUIRES(cpu->type == QEMU_CPUState);
  CPUState * qemucpu = cpu->object;

  return mmu_logical_to_physical(qemucpu, va);
}

//------------------------Should be fairly simple here-------------------------

//[???] I assume these are treated as booleans were 1 = true, 0 = false
int QEMU_mem_op_is_data(generic_transaction_t *mop)
{
	//[???]not sure on this one
	//possibly
	//if it is read or write. Might also need cache?
	if((mop->type == QEMU_Trans_Store) || (mop->type == QEMU_Trans_Load)){
		return 1;
	}else{
		return 0;
	}
}

int QEMU_mem_op_is_write(generic_transaction_t *mop)
{
	//[???]Assuming Store == write.
	if(mop->type == QEMU_Trans_Store){
		return 1;
	}else{
		return 0;
	}
}

int QEMU_mem_op_is_read(generic_transaction_t *mop)
{
	//[???]Assuming load == read.
	if(mop->type == QEMU_Trans_Load){
		return 1;
	}else{
		return 0;
	}
}

//ALEX - Added for timing, dummy for now

instruction_error_t QEMU_instruction_handle_interrupt(conf_object_t *cpu, pseudo_exceptions_t pendingInterrupt) {
	return QEMU_IE_OK;
}
 
int QEMU_get_pending_exception(void) {
  return 0;
} 

int QEMU_advance(void) {
  printf("QEMU_advance called!\n");
  //CPUState * cpu_state = cpu->object;
  //CPUArchState *env = cpu_state->env_ptr;
  //printf("before calling cpu_exec\n");
  //return get_info(env);

  printf("Need to advance an instruction properly and return the exception id, if one was raised!\n"); 
  //ALEX - May need to change function's signature; how do we specify which core to proceed by 1 instr?
  assert(false);
  return 0;
} 

conf_object_t *QEMU_get_object(const char *name) {
	//TODO: lookup request object by name and return reference to it
        printf("QEMU_get_object called!\n"); 
	assert(false);
	return NULL; //dummy
}
//ALEX - end

//NOOSHIN: begin --> dummy
int QEMU_cpu_exec_proc (conf_object_t *cpu) {
  
  int ret;
  //TODO: call the cpu_exec function and synchronize the exception IDs
  printf("QEMU_CPU_EXEC_PROC called\n");
  CPUState * cpu_state = cpu->object;
  //CPUArchState *env = cpu_state->env_ptr;
  printf("before calling cpu_exec\n");
  ret =  get_info(cpu_state);
  //assert(false);//Need to handle exceptions
  return ret;
 //return cpu_exec(env); 
}
//NOOSHIN: end

int flexus_is_simulating = 0;

int QEMU_is_in_simulation(void) {
  return flexus_is_simulating;
}

void QEMU_toggle_simulation(int enable) {
  if( enable != QEMU_is_in_simulation() ) {
    flexus_is_simulating = enable;
    QEMU_flush_tb_cache();
  }
}

int64_t flexus_simulation_length = -1;

int64_t QEMU_get_simulation_length(void) {
  return flexus_simulation_length;
}

void QEMU_flush_tb_cache(void) {
  CPUState* cpu = first_cpu;
  CPU_FOREACH(cpu) {
    tb_flush(cpu);
  }
}

//[???]Not sure what this does if there is a simulation_break, shouldn't there be a simulation_resume? 
void QEMU_break_simulation(const char * msg)
{
    //[???]it could be pause_all_vcpus(void)
    //Causes the simulation to pause, can be restarted in qemu monitor by calling stop then cont
    //or can be restarted by calling resume_all_vcpus();
    //looking at it some functtions in vl.c might be useful
    printf("Exiting because of break_simulation\n"); 
    printf("With exit message: %s\n", msg);

    Error* error_msg = NULL;
    error_setg(&error_msg, "%s", msg);
    qmp_quit(&error_msg);//seems to work, found from hmp.c:718 
    error_free(error_msg);

    //qemu_system_suspend();//from vl.c:1940//doesn't work at all
    //calls pause_all_vcpus(), and some other stuff.
    //For QEMU to know that they are paused I think.
    //qemu_system_suspend_request();//might be better from vl.c:1948
    //sort of works, but then resets cpus I think
    

    //I have not found anything that lets you send a message when you pause the simulation, but there can be a wakeup messsage.
    //in vl.c


    //possibly in cpus.c vm_stop, which takes in a state variable might not be resumeable 
    return;
}

void QEMU_initialize(void) {
  QEMU_initialize_counts();
  QEMU_setup_callback_tables();
}

void QEMU_shutdown(void) {
  QEMU_free_callback_tables();
  QEMU_deinitialize_counts();
}

uint64_t QEMU_total_instruction_count = 0;
uint64_t *QEMU_instruction_counts = NULL;

void QEMU_initialize_counts(void) {
  int num_cpus = QEMU_get_num_cpus();
  QEMU_instruction_counts = (uint64_t*)malloc(num_cpus*sizeof(uint64_t));
  
  QEMU_total_instruction_count = 0;
  int i = 0;
  for( ; i < num_cpus; i++ )
    QEMU_instruction_counts[i] = 0;
}

void QEMU_deinitialize_counts(void) {
  free(QEMU_instruction_counts);
}

uint64_t QEMU_get_instruction_count(int cpu_number) {
  return QEMU_instruction_counts[cpu_number];
}

void QEMU_increment_instruction_count(int cpu_number) {
  QEMU_total_instruction_count++;
  QEMU_instruction_counts[cpu_number]++;
}

uint64_t QEMU_get_total_instruction_count(void) {
  return QEMU_total_instruction_count;
}

// setup the callbacks for every cpu
void QEMU_setup_callback_tables(void) {
  // one table for every cpu + one for generic callbacks
  int numTables = QEMU_get_num_cpus() + 1;
  QEMU_all_callbacks_tables = (QEMU_callback_table_t*)malloc(sizeof(QEMU_callback_table_t)*numTables);

  int i = 0;
  for( ; i < numTables; i++ ) {
    QEMU_callback_table_t * table = QEMU_all_callbacks_tables + i;
    table->next_callback_id = 0;
    int j = 0;
    for( ; j < QEMU_callback_event_count; j++ ) {
      table->callbacks[j] = NULL;
    }
  }
}

// delete the callback tables for every cpu and all the callbacks
void QEMU_free_callback_tables(void) {
  int numTables = QEMU_get_num_cpus() + 1;
  // free every table
  int i = 0;
  for( ; i < numTables; i++ ) {
    QEMU_callback_table_t * table = QEMU_all_callbacks_tables + i;
    // free every callback in this event pool
    int j = 0;
    for( ; j < QEMU_callback_event_count; j++ ) {
      while( table->callbacks[j] != NULL ) {
        QEMU_callback_container_t *next = table->callbacks[j]->next;
        free(table->callbacks[j]);
        table->callbacks[j] = next;
      }
    }
  }
  free(QEMU_all_callbacks_tables);
}
// note: see QEMU_callback_table in api.h
// return a unique identifier to the callback struct or -1
// if an error occured
int QEMU_insert_callback(
			 int cpu_id,
			 QEMU_callback_event_t event,
			 void* obj, void* fn) {
  //[???]use next_callback_id then update it
  //If there are multiple callback functions, we must chain them together.
  //error checking-
  if(event>=QEMU_callback_event_count){
    //call some sort of errorthing possibly
    return -1;
  }
  QEMU_callback_table_t * table = &QEMU_all_callbacks_tables[cpu_id+1];
  QEMU_callback_container_t *container = table->callbacks[event];
  QEMU_callback_container_t *containerNew = malloc(sizeof(QEMU_callback_container_t));

  // out of memory, return -1
  if( containerNew == NULL )
    return -1;

  containerNew->id = table->next_callback_id;
  containerNew->callback = fn;
  containerNew->obj = obj;
  containerNew->next = NULL;

  if(container == NULL){
    //Simple case there is not a callback function for event 
    table->callbacks[event] = containerNew;
  }else{
    //we need to add another callback to the chain
    //Now find the current last function in the callbacks list
    while(container->next!=NULL){
      container = container->next;
    }
    container->next = containerNew;
  }
  table->next_callback_id++;
  return containerNew->id;
}

// delete a callback specific to the given cpu
void QEMU_delete_callback(
			  int cpu_id,
			  QEMU_callback_event_t event,
			  uint64_t callback_id) {
  //need to point prev->next to current->next
  //start with the first in the list and check its ID
  QEMU_callback_table_t * table = &QEMU_all_callbacks_tables[cpu_id+1];
  QEMU_callback_container_t *container = table->callbacks[event];
  QEMU_callback_container_t *prev = NULL;

  while( container != NULL ) {
    if( container->id == callback_id ) {
      // we have found a callback with the right id and cpu_id

      if( prev != NULL ) {
        // this is not the first element of he list
        // remove the element from the list
        prev->next = container->next;
      }else {
        // this is the first element of the list
        table->callbacks[event] = container->next;
      }
      free(container);
      break;
    }
    prev = container;
    container = container->next;
  }
}

static void do_execute_callback(
			 QEMU_callback_container_t *curr,
			 QEMU_callback_event_t event,
			 QEMU_callback_args_t *event_data);

static void do_execute_callback(
			 QEMU_callback_container_t *curr,
			 QEMU_callback_event_t event,
			 QEMU_callback_args_t *event_data) {
  dbg_printf("Executing callback id %"PRId64"\n");
  void *callback = curr->callback;
  switch (event) {
    // noc : class_data, conf_object_t
  case QEMU_config_ready:
    (*(cb_func_void)callback)(
							);
               
    break;
  case QEMU_magic_instruction:
    if (!curr->obj)
      (*(cb_func_nocI_t)callback)(
				  event_data->nocI->class_data
				  , event_data->nocI->obj
				  , event_data->nocI->bigint
				  );
    else
      (*(cb_func_nocI_t2)callback)(
				   (void*)curr->obj,
				   event_data->nocI->class_data
				   , event_data->nocI->obj
				   , event_data->nocI->bigint
				   );   
    break;
  case QEMU_continuation:
  case QEMU_asynchronous_trap:
  case QEMU_exception_return:
  case QEMU_ethernet_network_frame:
  case QEMU_ethernet_frame:
  case QEMU_periodic_event:
    if (!curr->obj){
      (*(cb_func_noc_t)callback)(
				 event_data->noc->class_data
				 , event_data->noc->obj
				 );
    }else{

      (*(cb_func_noc_t2)callback)(
				  (void *) curr->obj
				  , event_data->noc->class_data
				  , event_data->noc->obj
				  );
    }
    break;
    // nocIs : class_data, conf_object_t, int64_t, char*
  case QEMU_simulation_stopped:
    if (!curr->obj)
      (*(cb_func_nocIs_t)callback)(
				   event_data->nocIs->class_data
				   , event_data->nocIs->obj
				   , event_data->nocIs->bigint
				   , event_data->nocIs->string
				   );
    else
      (*(cb_func_nocIs_t2)callback)(
				    (void *) curr->obj
				    , event_data->nocIs->class_data
				    , event_data->nocIs->obj
				    , event_data->nocIs->bigint
				    , event_data->nocIs->string
				    );

    break;
    // nocs : class_data, conf_object_t, char*
  case QEMU_xterm_break_string:
  case QEMU_gfx_break_string:
    if (!curr->obj)
      (*(cb_func_nocs_t)callback)(
				  event_data->nocs->class_data
				  , event_data->nocs->obj
				  , event_data->nocs->string
				  );
    else
      (*(cb_func_nocs_t2)callback)(
				   (void *) curr->obj
				   , event_data->nocs->class_data
				   , event_data->nocs->obj
				   , event_data->nocs->string
				   );

    break;
    // ncm : conf_object_t, memory_transaction_t
  case QEMU_cpu_mem_trans:
    if (!curr->obj)
      (*(cb_func_ncm_t)callback)(
				 event_data->ncm->space
				 , event_data->ncm->trans
				 );
    else
      (*(cb_func_ncm_t2)callback)(
				  (void *) curr->obj
				  , event_data->ncm->space
				  , event_data->ncm->trans
				  );

    break;

  case QEMU_dma_mem_trans:
    if (!curr->obj){
      (*(cb_func_ncm_t)callback)(
				 event_data->ncm->space
				 , event_data->ncm->trans
				 );
    }
    else
      {
	//  printf("is this run(ifobj)\n");
	(*(cb_func_ncm_t2)callback)(
				    (void *) curr->obj
				    , event_data->ncm->space
				    , event_data->ncm->trans
				    );
      }
    break;
  default:
    dbg_printf("Event not found...\n");
    break;
  }
}

void QEMU_execute_callbacks(
			       int cpu_id,
			       QEMU_callback_event_t event,
			       QEMU_callback_args_t *event_data) {

  dbg_printf("Executing cpu specific callbacks for event %d\n", event);
  QEMU_callback_table_t * generic_table = &QEMU_all_callbacks_tables[0];
  QEMU_callback_table_t * table = &QEMU_all_callbacks_tables[cpu_id+1];
  QEMU_callback_container_t *curr = table->callbacks[event];

  // execute specified callbacks
  for (; curr != NULL; curr = curr->next)
      do_execute_callback(curr, event, event_data);

  if( cpu_id + 1 != 0 ) {
    // only execute the generic callbacks once
    curr = generic_table->callbacks[event];
    for (; curr != NULL; curr = curr->next)
      do_execute_callback(curr, event, event_data);
  }
}

#endif /* CONFIG_FLEXUS */

#ifdef __cplusplus
}
#endif
