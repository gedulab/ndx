#include "ndx.h"
#include<iostream>
#include<vector>
#include<chrono>
#include"ready.h"

using namespace std;

void print_rq_tasks(NDX_VECTOR_TASK cpu_task) {
	dprintf("on_cpu  state       PID     prio  wait-time"
		"    sum-exec    summ-sleep    comm\n");
	dprintf("---------------------------------------------"
		"----------------------------------------------------\n");

	for (vector<ndx_task>::iterator it = cpu_task.begin(); it != cpu_task.end(); it++)
	{
		dprintf("%-8d%08X    %-8u%-6u%-13u%-12u%-14u%-20s\n",
			it->on_cpu, it->state, it->pid, it->prio, it->wait_sum, 
			it->sum_exec_runtime, it->sum_sleep_runtime, it->comm);

	}
	dprintf("---------------------------------------------"
		"----------------------------------------------------\n");
}

void ReadReadyQueue(ndx_ctx* ndx, uint64_t init_task_addr, int cpu_no, NDX_VECTOR_TASK* vector_array, ULONG task_Offset) 
{
	int cpu = -1;
	uint64_t ts_addr = init_task_addr;
	uint64_t signal_addr = 0;
	uint64_t next_addr = 0, thread_task;
	ndx_task task;
	uint64_t state;
	BOOL bRet;
	HRESULT hr;

	ndx->offset_wait_sum = ndx->offset_task_se + ndx->offset_task_statistics + ndx->offset_task_wait_sum;
	ndx->offset_sum_exec_runtime = ndx->offset_task_se + ndx->offset_task_sum_exec_runtime;
	ndx->offset_sum_sleep_runtime = ndx->offset_task_se + ndx->offset_task_statistics + ndx->offset_task_sum_sleep_runtime;

	do { // iterate on process 
		bRet = ReadPointer(ts_addr + ndx->offset_signal, &signal_addr);
		if (!bRet || signal_addr == 0) {
			goto TAG_NEXT_PROCESS;
		}
		bRet = ReadPointer(signal_addr + ndx->offset_thread_head, &next_addr);
		if (!bRet || next_addr == 0) {
			goto TAG_NEXT_PROCESS;
		}
		do { // iterate on thread of the process
			thread_task = next_addr - ndx->offset_thread_node;
			hr = GetFieldValue(thread_task, "task_struct", "state", state);
			if (hr == S_OK && state == 0) {
				hr = GetFieldValue(thread_task, "task_struct", "cpu", cpu);
				if ((cpu_no > -1 && (cpu != cpu_no)) || cpu < 0 || cpu >= ndx->total_cpu){
					goto TAG_NEXT_THREAD;
				}
				hr = NdxReadTask(ndx, thread_task, &task);
				if (hr != S_OK) {
					goto TAG_NEXT_THREAD;
				}

				//strcpy(task.comm, comm);
				vector_array[cpu].push_back(task);
			}
TAG_NEXT_THREAD:
			bRet = ReadPointer(next_addr, &next_addr);
		} while (bRet && next_addr != signal_addr + ndx->offset_thread_head);
TAG_NEXT_PROCESS:
		hr = GetFieldValue(ts_addr, "task_struct", "tasks.next", ts_addr);
		ts_addr = ts_addr - task_Offset;
	} while (hr == S_OK && ts_addr != init_task_addr);
}

void print_task_comm_pid(uint64_t rq_addr, uint64_t ts_addr, int cpu) {
	char comm[16];
	int pid = 0;

	GetFieldValue(ts_addr, "task_struct", "comm", comm);
	GetFieldValue(ts_addr, "task_struct", "pid", pid);

	dprintf("\n\tCPU subordinate processes: \n");
	dprintf("\t%-30s%-30p", "task_struct:", ts_addr);
	dprintf("\t%-30s%-30s", "comm =", comm);
	dprintf("%-30s%-30d\n", "pid =", pid);

}

void print_task(uint64_t rq_addr, uint64_t ts_addr, int cpu) {

	int64_t state = 0;
	int64_t on_cpu = 0;
	static int offset_task_se = -1;
	char comm[16];
	int pid = 0;
	//uint64_t vruntime = 0;
	//uint64_t nvcsw = 0;
	//uint64_t nivcsw = 0;
	int prio = 0;

	uint64_t wait_sum = 0;
	uint64_t sum_exec_runtime = 0;
	int64_t sum_sleep_runtime = 0;

	uint64_t se = 0;
	uint64_t statistics = 0;
	if (offset_task_se < 0) {
		offset_task_se = NdxGetOffset("task_struct", "se");
	}
	se = ts_addr + offset_task_se;
	statistics = se + NdxGetOffset("se", "statistics");

	GetFieldValue(ts_addr, "task_struct", "state", state);
	GetFieldValue(ts_addr, "task_struct", "on_cpu", on_cpu);
	GetFieldValue(ts_addr, "task_struct", "comm", comm);
	GetFieldValue(ts_addr, "task_struct", "pid", pid);
	//GetFieldValue(se,"se","vruntime",vruntime );
	//GetFieldValue(ts_addr,"task_struct","nvcsw", nvcsw);
	//GetFieldValue(ts_addr,"task_struct","nivcsw", nivcsw);
	GetFieldValue(ts_addr, "task_struct", "prio", prio);
	GetFieldValue(statistics, "statistics", "wait_sum", wait_sum);
	GetFieldValue(se, "se", "sum_exec_runtime", sum_exec_runtime);
	GetFieldValue(statistics, "statistics", "sum_sleep_runtime", sum_sleep_runtime);

	//dprintf("\t%-30s%-30p","task_struct:",ts_addr);
	//dprintf("\t%-30s%-30d\n","CPU :",cpu);
	//dprintf("\t%-30s%-30s", "comm =", comm);
	//dprintf("%-30s%-30d\n", "pid =", pid);
	//dprintf("\t%-30s%-30d", "vruntime =", vruntime);
	//dprintf("%-30s%-30d\n", "nvcsw =", nvcsw);
	//dprintf("\t%-30s%-30d", "nivcsw =", nivcsw);
	//dprintf("%-30s%-30d\n", "prio =", prio);
	//dprintf("\t%-30s%-30d", "wait_sum =", wait_sum);
	//dprintf("%-30s%-30d\n", "sum_exec_runtime =", sum_exec_runtime);
	//dprintf("\t%-30s%-30d", "sum_sleep_runtime =", sum_sleep_runtime);

	dprintf("%-8u%08X    %-8u%-16u%-13u%-12u%-14u%-20s\n",
		on_cpu, state, pid, prio, wait_sum, sum_exec_runtime, sum_sleep_runtime, comm);



}

void  print_rq(int cpu_no, uint64_t rq_addr, ULONG task_Offset, uint64_t init_ts_addr) {

	//int on_cpu = 0;
	uint64_t state = 4;
	uint64_t curr_addr = 0;
	uint64_t curr_ts_addr = 0;
	uint64_t curr_offset = 0;
	uint64_t ts_addr = 0;
	uint32_t ti_cpu = 0;
	int cpu = 100;
	ULONG cpu_offset = 1;

	curr_offset = NdxGetOffset("rq", "curr");
	curr_addr = rq_addr + curr_offset;
	ReadPointer(curr_addr, &curr_ts_addr);
	ts_addr = init_ts_addr;
	cpu_offset = NdxGetOffset("task_struct", "cpu");

	int i = 0;
	dprintf("on_cpu  state       PID     prio            wait-time"
		"    sum-exec    summ-sleep    comm\n");
	dprintf("-------------------------------------------------------"
		"----------------------------------------------------\n");
	while (1) {
		GetFieldValue(ts_addr, "task_struct", "state", state);
		GetFieldValue(ts_addr, "task_struct", "cpu", cpu);
		//GetFieldValue(ts_addr, "task_struct", "on_cpu", on_cpu);
		//GetFieldValue(ts_addr+NdxGetOffset("task_struct","thread_info"), "thread_info", "cpu", ti_cpu);
		//dprintf("ti_cpu offset%d\n",NdxGetOffset("thread_info","cpu"));

		if (cpu == cpu_no && state == 0) {
			//print_task_comm_pid(rq_addr,ts_addr,cpu);
			print_task(rq_addr, ts_addr, cpu);
		}

		GetFieldValue(ts_addr, "task_struct", "tasks.next", ts_addr);
		ts_addr = ts_addr - task_Offset;

		if (ts_addr == init_ts_addr) {
			break;
		}
	}
	dprintf("-------------------------------------------------------"
		"----------------------------------------------------\n");
	dprintf("\n\n\n");
}

void print_cfs_group_stats(uint64_t se) {

	ULONG64 Address = 0;
	ULONG FieldOffset = 0;
	ULONG64 statistics = se + NdxGetOffset("sched_entity", "statistics");
	ULONG64 avg = se + NdxGetOffset("sched_entity", "avg");

	uint64_t				exec_start = 0;
	uint64_t				sum_exec_runtime = 0;
	uint64_t				vruntime = 0;
	uint64_t				wait_start = 0;
	uint64_t				wait_max = 0;
	uint64_t				wait_count = 0;
	uint64_t				wait_sum = 0;
	uint64_t				sleep_start = 0;
	uint64_t				sleep_max = 0;
	uint64_t				block_start = 0;
	uint64_t				block_max = 0;
	uint64_t				slice_max = 0;
	uint8_t                 se_load_weight = 0;
	uint64_t                se_runnable_weight = 0;
	uint64_t                se_avg_load_avg = 0;
	uint64_t                se_avg_util_avg = 0;
	uint64_t                exec_max = 0;
	uint64_t				se_avg_runnable_load_avg = 0;

	GetFieldValue(se, "se", "exec_start", exec_start);
	GetFieldValue(se, "se", "vruntime", vruntime);
	GetFieldValue(se, "se", "sum_exec_runtime", sum_exec_runtime);
	GetFieldValue(statistics, "statistics", "wait_start", wait_start);
	GetFieldValue(statistics, "statistics", "sleep_start", sleep_start);
	GetFieldValue(statistics, "statistics", "", block_start);
	GetFieldValue(statistics, "statistics", "", sleep_max);
	GetFieldValue(statistics, "statistics", "", block_max);
	GetFieldValue(statistics, "statistics", "", exec_max);
	GetFieldValue(statistics, "statistics", "", slice_max);
	GetFieldValue(statistics, "statistics", "", wait_max);
	GetFieldValue(statistics, "statistics", "", wait_sum);
	GetFieldValue(statistics, "statistics", "", wait_count);
	GetFieldValue(se + NdxGetOffset("sched_entity", "load"), "load", "weight", se_load_weight);
	GetFieldValue(se, "se", "runable_weight", se_runnable_weight);
	GetFieldValue(avg, "avg", "load_avg", se_avg_load_avg);
	GetFieldValue(avg, "avg", "util_avg", se_avg_util_avg);
	GetFieldValue(avg, "avg", "runnable_load_avg", se_avg_runnable_load_avg);

	dprintf("\tcfs_rq_tg_se_stats:\n");
	dprintf("\t%-30s%-30u", "exec_start =", exec_start);
	dprintf("%-30s%-30d\n", "vruntime =", vruntime);
	dprintf("\t%-30s%-30u", "sum_exec_runtime =", sum_exec_runtime);
	dprintf("%-30s%-30d\n", "wait_start =", wait_start);
	dprintf("\t%-30s%-30d", "sleep_start =", sleep_start);
	dprintf("%-30s%-30d\n", "block_start =", block_start);
	dprintf("\t%-30s%-30d", "sleep_max =", sleep_max);
	dprintf("%-30s%-30d\n", "block_max =", block_max);
	dprintf("\t%-30s%-30d", "exec_max =", exec_max);
	dprintf("%-30s%-30d\n", "slice_max =", slice_max);
	dprintf("\t%-30s%-30d", "wait_max =", wait_max);
	dprintf("%-30s%-30d\n", "wait_sum =", wait_sum);
	dprintf("\t%-30s%-30d", "wait_count =", wait_count);
	dprintf("%-30s%-30d\n", "se_load_weight =", se_load_weight);
	dprintf("\t%-30s%-30d", "se_runnable_weight =", se_runnable_weight);
	dprintf("%-30s%-30d", "se_avg_load_avg =", se_avg_load_avg);
	dprintf("\t%-30s%-30d", "se_avg_util_avg =", se_avg_util_avg);
	dprintf("%-30s%-30d\n", "se_avg_runnable_load_avg =", se_avg_runnable_load_avg);
}

void print_cfs_stats(ndx_ctx* ndx, int cpu_no, uint64_t cfs_rq_addr)
{
	char* cfs_rq_cache = NULL;
	int cfs_rq_size = -1;
	ULONG read = 0;

	unsigned int cfs_nr_spread_over = 0;
	uint64_t cfs_load_weight = 0;
	uint64_t cfs_runnable_weight = 0;
	uint64_t cfs_avg_load_avg = 0;
	uint64_t cfs_avg_runnable_load_avg = 0;
	uint64_t cfs_avg_util_avg = 0;
	unsigned int cfs_avg_util_est_enqueued = 0;
	uint64_t cfs_removed_load_avg = 0;
	uint64_t cfs_removed_util_avg = 0;
	uint64_t cfs_removed_runnable_sum = 0;
	uint64_t cfs_tg = 0;
	uint64_t cfs_tg_addr = 0;
	uint64_t cfs_tg_load_avg_contrib = 0;
	uint64_t cfs_tg_load_avg = 0;
	int cfs_throttled = 0;
	int cfs_throttle_count = 0;

	cfs_rq_size = GetTypeSize("task_struct");
	cfs_rq_cache = (char*)malloc(cfs_rq_size);


	ReadMemory(cfs_rq_addr, cfs_rq_cache, cfs_rq_size, &read);
	if (read < cfs_rq_size) {
		dprintf("read task_struct failed %d < %d \n", read, cfs_rq_size);
		return;
	}

	cfs_nr_spread_over = *(int*)(cfs_rq_cache + ndx->offset_cfs_nr_spread_over);
	cfs_load_weight = *(int*)(cfs_rq_cache + ndx->offset_cfs_load_weight);
	cfs_runnable_weight = *(int*)(cfs_rq_cache + ndx->offset_cfs_runnable_weight);
	cfs_avg_load_avg = *(int*)(cfs_rq_cache + ndx->offset_cfs_avg_load_avg);
	cfs_avg_runnable_load_avg = *(int*)(cfs_rq_cache + ndx->offset_cfs_avg_runnable_load_avg);
	cfs_avg_util_avg = *(int*)(cfs_rq_cache + ndx->offset_cfs_avg_util_avg);
	cfs_avg_util_est_enqueued = *(int*)(cfs_rq_cache + ndx->offset_cfs_avg_util_est_enqueued);
	cfs_removed_load_avg = *(int*)(cfs_rq_cache + ndx->offset_cfs_removed_load_avg);
	cfs_removed_util_avg = *(int*)(cfs_rq_cache + ndx->offset_cfs_removed_util_avg);
	cfs_removed_runnable_sum = *(int*)(cfs_rq_cache + ndx->offset_cfs_removed_runnable_sum);

	dprintf("\tcfs_rq stats:\n");
	dprintf("\t%-30s%-30u", "cfs_nr_spread_over =", cfs_nr_spread_over);
	dprintf("%-30s%-30u\n", "cfs_load_weight =", cfs_load_weight);
	dprintf("\t%-30s%-30u", "cfs_runnable_weight =", cfs_runnable_weight);
	dprintf("%-30s%-30u\n", "cfs_avg_load_avg =", cfs_avg_load_avg);
	dprintf("\t%-30s%-30u", "cfs_avg_runnable_load_avg =", cfs_avg_runnable_load_avg);
	dprintf("%-30s%-30u\n", "cfs_avg_util_avg =", cfs_avg_util_avg);
	dprintf("\t%-30s%-30u", "cfs_avg_util_est_enqueued =", cfs_avg_util_est_enqueued);
	dprintf("%-30s%-30u\n", "cfs_removed_load_avg =", cfs_removed_load_avg);
	dprintf("\t%-30s%-30u", "cfs_remove_util_avg =", cfs_removed_util_avg);
	dprintf("%-30s%-30u\n", "cfs_remove_runnable_sum =", cfs_removed_runnable_sum);
}

void print_rt_stats(int cpu_no, uint64_t rt_rq_addr)
{
	unsigned int rt_nr_running = 0;
	int	rt_throttled = 0;
	uint64_t rt_time = 0;
	uint64_t rt_runtime = 0;
	uint64_t rt_nr_migratory = 0;

	GetFieldValue(rt_rq_addr, "rt_rq", "rt_nr_running", rt_nr_running);
	GetFieldValue(rt_rq_addr, "rt_rq", "rt_throttled", rt_throttled);
	GetFieldValue(rt_rq_addr, "rt_rq", "rt_time", rt_time);
	GetFieldValue(rt_rq_addr, "rt_rq", "rt_runtime", rt_runtime);
	GetFieldValue(rt_rq_addr, "rt_rq", "rt_nr_migratory", rt_nr_migratory);

	dprintf("\n\trt_rq stats:\n");
	dprintf("\t%-30s%-30u", "rt_nr_running =", rt_nr_running);
	dprintf("%-30s%-30d\n", "rt_throttled =", rt_throttled);
	dprintf("\t%-30s%-30u", "rt_time =", rt_time);
	dprintf("%-30s%-30u\n", "rt_runtime =", rt_runtime);
	dprintf("\t%-30s%-30u\n", "rt_nr_migratory =", rt_nr_migratory);

}

void print_dl_stats(int cpu_no, uint64_t dl_rq_addr) {
	uint64_t dl_nr_running = 0;
	uint64_t dl_nr_migratory = 0;


	GetFieldValue(dl_rq_addr, "dl_rq", "dl_nr_running", dl_nr_running);
	GetFieldValue(dl_rq_addr, "dl_rq", "dl_nr_migratory", dl_nr_migratory);

	dprintf("\n\tdl_ rq stats:\n");
	dprintf("\t%-30s%-30u", "dl_nr_migratory:", dl_nr_migratory);
	dprintf("%-30s%-30u\n", "dl_nr_running =", dl_nr_running);
}

int ndx_show_rq(int cpu_no, uint64_t rq_addr)
{
	unsigned int nr_running = 0;
	uint64_t nr_switches = 0;
	uint64_t nr_load_updates = 0;
	uint64_t nr_uninterruptible = 0;
	uint64_t next_balance = 0;

	uint64_t clock = 0;
	uint64_t clock_task = 0;
	uint64_t avg_idle = 0;
	uint64_t max_idle_balance_const = 0;

	unsigned int yld_count = 0;
	unsigned int sched_count = 0;
	unsigned int sched_goidle = 0;
	unsigned int ttwu_count = 0;
	unsigned int ttwu_local = 0;


	// P(nr_running); rq->nr_running
	GetFieldValue(rq_addr, "rq", "nr_running", nr_running);
	GetFieldValue(rq_addr, "rq", "nr_switches", nr_switches);
	GetFieldValue(rq_addr, "rq", "nr_load_updates", nr_load_updates);
	GetFieldValue(rq_addr, "rq", "nr_uninterruptible", nr_uninterruptible);
	GetFieldValue(rq_addr, "rq", "next_balance", next_balance);

	GetFieldValue(rq_addr, "rq", "clock", clock);
	GetFieldValue(rq_addr, "rq", "clock_task", clock_task);
	GetFieldValue(rq_addr, "rq", "avg_idle", avg_idle);
	GetFieldValue(rq_addr, "rq", "max_idle_balance_const", max_idle_balance_const);

	GetFieldValue(rq_addr, "rq", "yld_count", yld_count);
	GetFieldValue(rq_addr, "rq", "sched_count", sched_count);
	GetFieldValue(rq_addr, "rq", "sched_goidle", sched_goidle);
	GetFieldValue(rq_addr, "rq", "ttwu_count", ttwu_count);
	GetFieldValue(rq_addr, "rq", "ttwu_local", ttwu_local);

	dprintf("\t%-30s%-30u", "nr_running =", nr_running);
	dprintf("%-30s%-30u\n", "nr_switches =", nr_switches);
	dprintf("\t%-30s%-30u", "nr_load_updates =", nr_load_updates);
	dprintf("%-30s%-30u\n", "nr_uninterruptible =", nr_uninterruptible);
	dprintf("\t%-30s%-30u", "next_balance =", next_balance);
	dprintf("%-30s%-30u\n", "yld_count =", yld_count);

	dprintf("\t%-30s%-30u", "clock =", clock);
	dprintf("%-30s%-30d\n", "clock_task =", clock_task);
	dprintf("\t%-30s%-30d", "avg_idle =", avg_idle);
	dprintf("%-30s%-30u\n", "max_idle_balance_const =", max_idle_balance_const);

	dprintf("\t%-30s%-30u", "sched_count =", sched_count);
	dprintf("%-30s%-30u\n", "sched_goidle =", sched_goidle);
	dprintf("\t%-30s%-30u", "ttwu_count =", ttwu_count);
	dprintf("%-30s%-30u\n", "ttwu_local =", ttwu_local);

	return 0;
}

//-a num(打印所有的数据)，-t(只打印进程的命令)

void NdxReadyUsage()
{
	dprintf("!ready to show linux ready queue or a CPU\n."
		" Syntax: !ready [-c cpu-no] [-f display-flags]\n"
		" display flags:\n"
		"  Bit0: task list on ready queue.\n"
		"  Bit1: queue attributes.\n"
	);
}

void PrintReadyTaskList(ndx_ctx* ndx, int cpu_no,int total_cpu, NDX_VECTOR_TASK* vector_array) 
{
	uint64_t thread_task;
	ndx_task task;
	int start = (cpu_no >= 0) ? cpu_no : 0;
	for (int i = start; i < start + total_cpu; i++) {
		dprintf("Ready tasks of CPU %d:\n", i);
		if (vector_array[i].size() == 0) {
			thread_task = NdxGetCurrent(ndx, i);
			NdxReadTask(ndx, thread_task, &task);
			vector_array[i].push_back(task);
		}
		print_rq_tasks(vector_array[i]);
	}
}
// cpu_no = -1 to print all cpus
void PrintReadyQueueAttributes(ndx_ctx* ndx, int cpu_no, NDX_VECTOR_TASK* vector_array) {
	
	uint64_t cfs_rq = 0;
	uint64_t rt_rq = 0;
	uint64_t dl_rq = 0;
	uint64_t percpu = 0;
	uint64_t rq_addr = 0;
	int total_cpu = (cpu_no < 0) ? ndx->total_cpu : 1;
	int start = (cpu_no < 0) ? 0 : cpu_no;
	
	for (int i = start; i < start + total_cpu; i++) {
		ReadControlSpace64(i, PCR_FLD_VIRTUAL_PERCPU_BASE, &percpu, sizeof(percpu));
		rq_addr = percpu + ndx->rq_offset;
		dprintf("Ready queue of CPU %d:\n", i);
		dprintf("\n\t%-30s%-30p\n", "percpu: ", percpu);
		dprintf("\t%-30s%-30p\n", "rq_addr: ", rq_addr);
		ndx_show_rq(i, rq_addr);

		cfs_rq = rq_addr + NdxGetOffset("rq", "cfs");
		rt_rq = rq_addr + NdxGetOffset("rq", "rt");
		dl_rq = rq_addr + NdxGetOffset("rq", "dl");

		dprintf("\n\t%-30s%-30p\n", "cfs_rq: ", cfs_rq);
		dprintf("\t%-30s%-30p\n", "rt_rq: ", rt_rq);
		dprintf("\t%-30s%-30p\n", "dl_rq: ", dl_rq);

		print_cfs_stats(ndx, i, cfs_rq);
		print_rt_stats(i, rt_rq);
		print_dl_stats(i, dl_rq);
	}
}

DECLARE_API(ready)
{
	const char* cursor = args;
	int cpu_no = -1, total_cpu = ndx.total_cpu;
	int num = 0, argc = 1, len = 0;
	size_t eaten_length = 0;
	uint32_t flags = NDX_READY_FLAG_TASK_LIST;
	char option[100];
	NDX_VECTOR_TASK* vector_array = new NDX_VECTOR_TASK[total_cpu];

	PCSTR re;
	uint64_t ts_addr = 0;
	ULONG task_Offset = 0;
	int cpu_mask = -1;

	// load field offsets 
	NdxGetOffsets(&ndx);

	while (*cursor != 0) {
		switch (cursor[1]) {
		case 'f':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				// read ready queue
				flags = strtoul(option, NULL, 0);
			}
			else {
				dprintf("missing display flags, using default\n");
			}
			break;
		case 'c':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				cpu_no = strtoul(option, NULL, 0);
				total_cpu = 1;
			}
			else {
				dprintf("missing cpu no to show, using default\n");
			}
			break;
		case 'h':
		case 'H':
		case '?':
			NdxReadyUsage();
			return;
		default:
			dprintf("unknown command option %s\n", cursor);
			break;
		}

		if (len > 0) {
			cursor += eaten_length;
			while (*cursor == ' ') cursor++;
		}
		else
			break;
	};

	GetExpressionEx("lk!runqueues", &(ndx.rq_offset), &re);
	GetExpressionEx("lk!init_task", &ts_addr, &re);
	GetFieldOffset("task_struct", "tasks", &task_Offset);

	if (ndx.rq_offset == 0) {
		dprintf("failed to get lk!runqueues. please load symbols for kernel.");
		return;
	}
	if (flags & NDX_READY_FLAG_TASK_LIST){
		ReadReadyQueue(&ndx, ts_addr, cpu_no, vector_array, task_Offset);
		PrintReadyTaskList(&ndx, cpu_no, total_cpu, vector_array);
	}
	if (flags & NDX_READY_FLAG_QUEUE_ATTRIBUTES) {
		PrintReadyQueueAttributes(&ndx, cpu_no, vector_array);
	}

	delete[] vector_array;
}