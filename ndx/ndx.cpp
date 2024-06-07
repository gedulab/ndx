#include "ndx.h"
#include <string.h>
#include "dmesg.h"
#include <time.h>
EXT_API_VERSION	ApiVersion =
{
	5,
	0,
	EXT_API_VERSION_NUMBER64,
	0
};
WINDBG_EXTENSION_APIS	ExtensionApis;
ndx_ctx ndx; // global context with generic system attributes

ULONG64	SavedMajorVersion;
ULONG64	SavedMinorVersion;
/*结束全局变量的定义及声明*/

/*
 *编写WDBGExts类型的扩展时，函数WinDbgExtensionDllInit必须被导出；
 *此函数地址表的地址通过参数lpExtensionApis传递进来；
 *此函数通常会将地址保存到全局变量中。
 */
VOID WinDbgExtensionDllInit(PWINDBG_EXTENSION_APIS lpExtensionApis, USHORT MajorVersion, USHORT MinorVersion)
{
	ExtensionApis = *lpExtensionApis;

	SavedMajorVersion = MajorVersion;
	SavedMinorVersion = MinorVersion;
	NdxInitContext(&ndx);
	return;
}
/*WinDbgExtensionDllInit函数结束*/

/*
 *编写WDBGExts类型的扩展时，函数ExtensionApiVersion必须被导出；
 *此函数应返回EXT_API_VERSION_NUMBER64，供API识别64位地址；
 *需要定义KDEXT_64BIT（见头文件中的LXP.h）；
 *需要定义ExtensionApis（见上方全局变量）。
 */
LPEXT_API_VERSION ExtensionApiVersion(VOID)
{
	return &ApiVersion;
}
/*ExtensionApiVersion函数结束*/

VOID CheckVersion(VOID)
{
	return;
}
/*CheckVersion函数结束*/

DECLARE_API(ndxhelp)
{
	dprintf
	(
		"Help for Nano Debugger eXtension (NDX) rev. 2.6:\n"
		" lxp: List linux processes.\n"
		" ps [num]: walk task list from init_task.\n"
		" address <addr>: show memory attributes of specified address.\n"
		" idt [addr]: show interrupt descriptor table on x86 system or ARM.\n"
		" dmesg: print kernel message.\n"
		" arena: watch arenas of the glibc heap.\n"
		" ready: display ready queue of processors.\n"
	);
}

DECLARE_API(help)
{
	ndxhelp(hCurrentProcess, hCurrentThread, dwCurrentPc, dwProcessor, args);
}
/*ndxhelp扩展命令结束*/

DECLARE_API(ps)
{
	ULONG64  Address;
	ULONG  Offset;
	int num = 0, max = 5;

	GetExpressionEx("lk!init_task", &Address, &args);
	GetFieldOffset("lk!task_struct", "tasks", &Offset);

	if (args != NULL && args[0] != 0) {
		max = atoi(args);
	}
	dprintf("Walking task list from %p max %d\n", Address, max);
	while (num < max)
	{
		ShowTaskStructAddress(Address);

		GetFieldValue(Address, "task_struct", "tasks.next", Address);
		Address = Address - Offset;//tasks&&list

		num = num + 1;
	}

	return;
}
void ShowTaskStructAddress(ULONG64 Address)
{
	ULONG64  pid;
	char comm[300];

	GetFieldValue(Address, "task_struct", "pid", pid);
	GetFieldValue(Address, "task_struct", "comm", comm);

	dprintf("pid %d, comm %s task_struct address = %p\n", pid, &comm, Address);

	return;
}

/*address扩展命令结束*/

DECLARE_API(slab)
{

	ULONG64 Address, FirstAddress;
	ULONG FieldOffset;
	const char* option, * filename;

	dprintf("Reading slabinfo of Linux kernel\n");

	if (args[0] == '?') {
		dprintf("-o Sort by single object size\n");
		dprintf("-t Sort by total size of all objects\n");
		dprintf("-n Sort by number of objects\n");
		dprintf("-? Display option information\n");
		return;
	}

	GetExpressionEx("lk!slab_caches", &FirstAddress, NULL);
	GetFieldValue(FirstAddress, "list_head", "next", Address);
	GetFieldOffset("kmem_cache", "list", &FieldOffset);
	dprintf("kmem_cache=%11x\n", FieldOffset);
	if (FieldOffset == 0)
	{
		dprintf("failed to get FieldOffset\n");
		return;
	}
	dprintf("slab_caches = %llx\n", FirstAddress);
	if (FirstAddress == 0) {
		dprintf("failed read FirstAddress\n");
		// failed
		return;
	}
	dprintf(" name                                                       total_obj       num_obj   objperslab    object_size   size  align useroffset usersize refcount      \n");
	ShowSlabWithSortBySize(FirstAddress, Address, FieldOffset);
}
/*slab扩展命令结束*/

DECLARE_API(arena)
{
	dprintf("%s\n", args); // 测试args的数值
	ULONG64 arena_address, arena_firstaddress, top, main_arena_address;
	char AllParameter[MAX_PATH], Parameter1[26], Parameter2[26];
	int arena_num, len, i, j, DelimNum, DelimAddr[10];
	ULONG64 *chunk_address = 0;

	DelimNum = 0;
	j = 0;
	len = strlen(args);
	if (len != 0) {
		for (i = 0; i < len; i++) {
			if (len == 0) {
				break;
			}
			AllParameter[i] = args[i];
			if (AllParameter[i] == '-' || (AllParameter[i] == ' ' && AllParameter[i + 1] != '-')) {
				DelimNum = DelimNum + 1;
				DelimAddr[j] = i + 1;
				j = j + 1;
			}
		}
		AllParameter[i] = '\0';
	}
	/*for (i = 0; i <= j; i++) {
		dprintf("%d %d\n", DelimNum, DelimAddr[i]);
	}*/
	if (DelimNum != 0) {
		Parameter2[0] = AllParameter[DelimAddr[j - 1]];
	}
	int addrcheck = 0;
	if (len != 0 && len > 10 && Parameter2[0] != 'x' && Parameter2[0] != '0') {
		i = 0;
		while (i < 16) {
			Parameter1[i] = AllParameter[DelimAddr[0] + i];
			i = i + 1;
		}
		Parameter1[i] = '\0';
		addrcheck = CheckArenaAddress(Parameter1);
	}
	else if (len != 0 && len > 10 && Parameter2[0] == 'x') {
		i = 0;
		while (i < 16) {
			Parameter1[i] = AllParameter[DelimAddr[0] + i];
			i = i + 1;
		}
		Parameter1[i] = '\0';
		addrcheck = 1;
	}
	else if (len != 0 && len > 10 && Parameter2[0] == '0') {
		Parameter2[0] = 'x';
		i = 0;
		while (i < 16) {
			Parameter1[i] = AllParameter[DelimAddr[j - 1] + i];
			i = i + 1;
		}
		addrcheck = 1;
	}
	else if (len != 0 && len < 10) {
		addrcheck = 0;
	}
	arena_num = 0;
	GetExpressionEx("main_arena", &arena_address, &args);
	//dprintf("%s\n", args);测试args的数值在GetEcpressionEx后有没有改变--》没有改变
	arena_firstaddress = arena_address;

	if (DelimNum == 0) {
		do {
			ShowMallocStateSimple(arena_address, arena_num);

			dprintf("--------------------------------------------------------------\n");
			GetFieldValue(arena_address, "malloc_state", "next", arena_address);
			arena_num = arena_num + 1;
		} while (arena_address != arena_firstaddress);
	}
	else if (Parameter2[0] == '?') {
		dprintf("Have Arena Address : Display detailed information.\n"); 
		dprintf("No Arena Address   : Display basic information.\n");
		dprintf("Flags Bit0         : Whether the allocation area contains fast_bin_chunks   F : Yes | - : No\n");
		dprintf("Flags Bit1         : Whether it is a continuous virtual address space       C : Yes | N : No\n");
	}
	else if (Parameter2[0] == 'd') {
		GetExpressionEx("main_arena", &main_arena_address, &args);
		if (addrcheck == 0) {
			dprintf("Please enter the correct address!\n");
			return;
		}
		else {
			sscanf_s(Parameter1, "%llx", &arena_address);
		}
		if (arena_address == main_arena_address) {
			ShowMainArena(arena_address);
		}
		else {
			ShowThreadArena(arena_address);
		}
	}
	else if (Parameter2[0] == 'h') {
		if (addrcheck == 0) {
			dprintf("Please enter the correct address!\n");
			return;
		}
		else {
			sscanf_s(Parameter1, "%llx", &arena_address);
		}
		GetExpressionEx("main_arena", &main_arena_address, &args);
		if (arena_address == main_arena_address) {
			dprintf("Please enter the non_main_arena address instead of the main_arena address\n");
			return;
		}
		top = ReadDigitField(arena_address, "malloc_state", "top");
		if (arena_address < top) {
			dprintf("There is no sub-heap!\n");
			return;
		}
		ShowSubHeap(top);
	}
	else if (Parameter2[0] == 's') {
		GetExpressionEx("main_arena", &main_arena_address, &args);
		if (addrcheck == 0) {
			dprintf("Please enter the correct address!\n");
			return;
		}
		else {
			sscanf_s(Parameter1, "%llx", &arena_address);
		}

		if (arena_address == main_arena_address) {
			ShowMainArenaChunkType(arena_address);
		}
		else {
			ShowNonMainArenaChunkType(arena_address);
		}

	}
	else if (Parameter2[0] == 'x') {
		if (addrcheck == 0) {
			dprintf("Please enter the correct address!\n");
			return;
		}

		chunk_address = CheckChunkAddress(Parameter1);

		if (chunk_address[0] == 0) {
			dprintf("Please enter the correct address!\n");
			return;
		}
		else {
			ShowMallocChunkSimple(chunk_address[0], chunk_address[1]);
			return;
		}
	}
	else {
		dprintf("Please enter the correct parameters!\n");
	}
}

DECLARE_API(test)
{
	ULONG64 Address = 0;

	GetFieldValue(Address, "kmem_cache_node", "next", Address);
}
extern "C" int dmesg_decode_file(const char* filename);
#define MAX_BYTES_PER_MSG 2000
int dmesg_decode_memory()
{
	ULONGLONG prinkBase, cursor, prinkLength;
	int FieldOffset = 0;
	printk_log_t hdr;
	ULONG Status, nTotal, nRemaining, count;
	char szText[MAX_BYTES_PER_MSG+1];
	char tsbuf[64];
    double delta;
	double msec;
	ReadMsr(NDB_MSR_PRINTK_BUF_BASE, &prinkBase);
	ReadMsr(NDB_MSR_PRINTK_BUF_LENGTH, &prinkLength);
	// print for debug
	dprintf("Prink buffer base %I64x, length 0x%I64x\n", prinkBase, prinkLength);

	char msrvalue;
	cursor = prinkBase;
	do {
		Status = ReadMemory(cursor, &hdr, sizeof(hdr), &count);
		if (CheckControlC())
		{
			break;
		}
		if (Status && count == sizeof(hdr)) {
			nTotal = hdr.len;
			delta =(double) hdr.ts_nsec/1000000000;//nanoseconds转换一下秒
			msec = (double)hdr.ts_nsec / 1000;
			if (delta<10)
			{
				snprintf(tsbuf, sizeof(tsbuf),
					"%13.7f", delta);
			}
			else
			{
				snprintf(tsbuf, sizeof(tsbuf),
					"%14.7f", delta);
			}
			if (hdr.len == 0 || hdr.text_len == 0) //end
				break;
			// read text part
			nRemaining = nTotal - sizeof(hdr); // to be aligned 
			if (nRemaining > MAX_BYTES_PER_MSG)
				nRemaining = MAX_BYTES_PER_MSG;
			Status = ReadMemory(cursor+sizeof(hdr),szText, nRemaining, &count);

			if (Status && count == nRemaining) {
				// print text
				dprintf("[%s]%s\n",tsbuf, szText);			
				cursor += nTotal;
			}
		}	
		
	} while (1);

	return 0;
}
#if 0
DECLARE_API(dmesg)
{
	ULONG64 Address = 0;
	const char* option, *filename;
	option = strstr(args, "-f");
	if(option != NULL) {
		filename = strchr(option, ' ');
		if(filename == NULL){
			dprintf("Missing file to decode (%s)\n", args);
			return;
		}
		filename++;
		//GetFieldValue(Address, "kmem_cache_node", "next", Address);
		dmesg_decode_file(filename);
	}
	else {

		dmesg_decode_memory();
	}
}
#endif
#define PCR_FLD_VIRTUAL_BASE 0xFFFFFFFFFFFFDD00
#define PCR_FLD_VIRTUAL_MIDR        (PCR_FLD_VIRTUAL_BASE + 1)
#define PCR_FLD_VIRTUAL_PERCPU_BASE (PCR_FLD_VIRTUAL_BASE + 2) // base of the percpu region, tpidr_el1 for armv8

int NdxPickNextField(const char* cursor, char* buffer, size_t buf_len, size_t* eaten_length)
{
	int len;
	const char* start = cursor;
	while (*start == ' ') start++;
	const char* sep = strchr(start, ' ');
	len = (sep != NULL) ? (sep - start) : strlen(start);
	if (len < buf_len) {
		strncpy(buffer, start, len);
		buffer[len] = 0;
		*eaten_length = start - cursor + len;
		return len;
	}
	return 0;
}

int NdxInitContext(ndx_ctx* ctx)
{
	PROCESSORINFO pi;
	ULONG hr = GetKdContext(&pi);
	ctx->total_cpu = pi.NumberProcessors;
	ctx->cpu_no = pi.Processor;
	ctx->percpu_base = (uint64_t*)malloc(sizeof(uint64_t) * ctx->total_cpu);
	memset(ctx->percpu_base, 0, sizeof(uint64_t) * ctx->total_cpu);

	return hr;
}

// simple wrapper of the GetFieldOffset which return status
ULONG NdxGetOffset(IN LPCSTR Type, IN LPCSTR Field) 
{
	ULONG FieldOffset = 0;
	HRESULT hr = GetFieldOffset(Type, Field, &FieldOffset);
	if (hr != S_OK) {
		dprintf("ndx failed to get %s address for 0x%x",Field, hr);
	}
	return FieldOffset;
}
uint64_t NdxGetCurrent(ndx_ctx* ndx, int cpu_no)
{
	PCSTR re;
	uint64_t rq_addr, curr_ts_addr;
	if(ndx->rq_offset == 0) {
		GetExpressionEx("lk!runqueues", &(ndx->rq_offset), &re);
	}
	if (ndx->offset_rq_curr <= 0) {
		ndx->offset_rq_curr = NdxGetOffset("rq", "curr");
	}
	if(ndx->percpu_base[cpu_no] == 0) {
		ReadControlSpace64(cpu_no, PCR_FLD_VIRTUAL_PERCPU_BASE, &ndx->percpu_base[cpu_no], sizeof(uint64_t));
	}
	rq_addr = ndx->percpu_base[cpu_no] + ndx->rq_offset;
	
	ReadPointer(rq_addr + ndx->offset_rq_curr, &curr_ts_addr);
	return curr_ts_addr;
}
int NdxGetOffsets(ndx_ctx* ndx)
{
	if (ndx->offset_cfs_avg <= 0) {
		ndx->offset_cfs_avg = NdxGetOffset("cfs_rq", "avg");
		ndx->offset_cfs_removed = NdxGetOffset("cfs_rq", "removed");
		ndx->offset_cfs_load = NdxGetOffset("cfs_rq", "load");
		ndx->offset_cfs_load_weight = NdxGetOffset("load_weight", "weight");
		ndx->offset_cfs_nr_spread_over = NdxGetOffset("cfs_rq", "nr_spread_over");
		ndx->offset_cfs_runnable_weight = NdxGetOffset("cfs_rq", "runnable_weight");
		ndx->offset_cfs_avg_load_avg = ndx->offset_cfs_avg + NdxGetOffset("sched_avg", "load_avg");
		ndx->offset_cfs_avg_runnable_load_avg = ndx->offset_cfs_avg + NdxGetOffset("sched_avg", "runnable_load_avg");
		ndx->offset_cfs_avg_util_avg = ndx->offset_cfs_avg + NdxGetOffset("sched_avg", "util_avg");
		ndx->offset_cfs_avg_util_est = ndx->offset_cfs_avg + NdxGetOffset("sched_avg", "util_est");
		ndx->offset_cfs_avg_util_est_enqueued = ndx->offset_cfs_avg_util_est + NdxGetOffset("util_est", "enqueued");
		ndx->offset_cfs_removed_load_avg = NdxGetOffset("cfs_rq", "removed.load_avg");
		ndx->offset_cfs_removed_util_avg = NdxGetOffset("cfs_rq", "removed.util_avg");
		ndx->offset_cfs_removed_runnable_sum = NdxGetOffset("cfs_rq", "removed.runnable_sum");
	}
	if (ndx->offset_task_se <= 0) {
		ndx->offset_task_se = NdxGetOffset("task_struct", "se");
		ndx->offset_task_on_cpu = NdxGetOffset("task_struct", "on_cpu");
		ndx->offset_task_cpu = NdxGetOffset("task_struct", "cpu");
		ndx->offset_task_comm = NdxGetOffset("task_struct", "comm");
		ndx->offset_task_state = NdxGetOffset("task_struct", "state");
		ndx->offset_task_pid = NdxGetOffset("task_struct", "pid");
		ndx->offset_task_tgid = NdxGetOffset("task_struct", "tgid");
		ndx->offset_task_prio = NdxGetOffset("task_struct", "prio");
		ndx->offset_task_statistics = NdxGetOffset("sched_entity", "statistics");
		ndx->offset_task_wait_sum = NdxGetOffset("sched_statistics", "wait_sum");
		ndx->offset_task_sum_exec_runtime = NdxGetOffset("sched_entity", "sum_exec_runtime");
		ndx->offset_task_sum_sleep_runtime = NdxGetOffset("sched_statistics", "sum_sleep_runtime");
		ndx->offset_thread_head = NdxGetOffset("signal_struct", "thread_head");
		ndx->offset_thread_node = NdxGetOffset("task_struct", "thread_node");
		ndx->offset_signal = NdxGetOffset("task_struct", "signal");
	}
	if (ndx->size_task_struct <= 0) {
		ndx->size_task_struct = GetTypeSize("task_struct");
	}
	return 0;
}
int NdxReadTask(ndx_ctx* ndx, uint64_t task_addr, ndx_task* task)
{
	char* task_cache = NULL;
	ULONG read = 0;
	int64_t state = 0;
	int64_t on_cpu = 0;
	int pid = 0;
	int prio = 0;
	uint64_t wait_sum;
	uint64_t sum_exec_runtime;
	int64_t sum_sleep_runtime;

	if(ndx->size_task_struct <=0 )
		ndx->size_task_struct = GetTypeSize("task_struct");
	task_cache = (char*)malloc(ndx->size_task_struct);
	ReadMemory(task_addr, task_cache, ndx->size_task_struct, &read);
	if (read < ndx->size_task_struct) {
		dprintf("read task_struct failed %d < %d \n", read, ndx->size_task_struct);
		return E_FAIL;
	}

	on_cpu = *(int*)(task_cache + ndx->offset_task_on_cpu);
	pid = *(int*)(task_cache + ndx->offset_task_pid);
	strcpy(task->comm, (char*)(task_cache + ndx->offset_task_comm));
	prio = *(int*)(task_cache + ndx->offset_task_prio);
	wait_sum = *(int*)(task_cache + ndx->offset_wait_sum);
	sum_exec_runtime = *(int*)(task_cache + ndx->offset_sum_exec_runtime);
	sum_sleep_runtime = *(int*)(task_cache + ndx->offset_sum_sleep_runtime);

	task->on_cpu = on_cpu;
	task->pid = pid;
	task->prio = prio;
	task->state = state;
	task->sum_exec_runtime = sum_exec_runtime;
	task->sum_sleep_runtime = sum_sleep_runtime;
	task->wait_sum = wait_sum;
	free(task_cache);

	return S_OK;
}