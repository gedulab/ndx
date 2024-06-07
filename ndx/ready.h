#pragma once

#include <vector>
#include "ndx.h"

ULONG GetOffset(IN LPCSTR Type, IN LPCSTR Field);

typedef std::vector<ndx_task> NDX_VECTOR_TASK;
typedef std::vector<ndx_task> NDX_VECTOR_TASK;


void print_rq_tasks(NDX_VECTOR_TASK cpu_task);

void ReadReadyQueue(uint64_t init_task_addr, int cpu_mask, int cpu_no, NDX_VECTOR_TASK* vector_array, ULONG task_Offset);

void print_task_comm_pid(uint64_t rq_addr, uint64_t ts_addr, int cpu);

void print_task(uint64_t rq_addr, uint64_t ts_addr, int cpu);

void  print_rq(int cpu_no, uint64_t rq_addr, ULONG task_Offset, uint64_t init_ts_addr);

void print_cfs_group_stats(uint64_t se);

void print_cfs_stats(int cpu_no, uint64_t cfs_rq_addr);

void print_rt_stats(int cpu_no, uint64_t rt_rq_addr);

void print_dl_stats(int cpu_no, uint64_t dl_rq_addr);

int ndx_show_rq(int cpu_no, uint64_t rq_addr);

void NdxReadyUsage();

void PrintReadyFlagQueuAttributes(ndx_ctx* cpu, NDX_VECTOR_TASK* vector_array);

void PrintReadyFlagTaskList(int cpu_no, int total_cpu, NDX_VECTOR_TASK* vector_array);

#define NDX_READY_FLAG_TASK_LIST						(1<<0)
#define NDX_READY_FLAG_QUEUE_ATTRIBUTES					(1<<1)
#define PCR_FLD_VIRTUAL_BASE 0xFFFFFFFFFFFFDD00
#define PCR_FLD_VIRTUAL_MIDR        (PCR_FLD_VIRTUAL_BASE + 1)
#define PCR_FLD_VIRTUAL_PERCPU_BASE (PCR_FLD_VIRTUAL_BASE + 2) // base of the percpu region, tpidr_el1 for armv8

DECLARE_API(ready);