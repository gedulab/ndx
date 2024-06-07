#pragma once
#include <stdint.h>
#include <windows.h>
#include "dbgexts.h"
#include <wdbgexts.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct ndx_task_ {
		uint64_t state;
		int on_cpu;
		int pid;
		int prio;
		uint64_t wait_sum;
		uint64_t sum_exec_runtime;
		int64_t sum_sleep_runtime;
		char comm[16];
	} ndx_task;

#define KDEXT_64BIT
	/* 页可读，可写，可执行，可以多个进程共享 */
#define VM_READ		0x00000001	/* currently active flags */
#define VM_WRITE	0x00000002
#define VM_EXEC		0x00000004
#define VM_SHARED	0x00000008
/* mprotect() hardcodes VM_MAYREAD >> 4 == VM_READ, and so for r/w/x bits. */
#define VM_MAYREAD	0x00000010	/* limits for mprotect() etc */
#define VM_MAYWRITE	0x00000020
#define VM_MAYEXEC	0x00000040
#define VM_MAYSHARE	0x00000080
#define VM_GROWSDOWN	0x00000100	/* general info on the segment */
#define VM_UFFD_MISSING	0x00000200	/* missing pages tracking */
#define VM_PFNMAP	0x00000400	/* Page-ranges managed without "struct page", just pure PFN */
#define VM_DENYWRITE	0x00000800	/* ETXTBSY on write attempts.. */
#define VM_UFFD_WP	0x00001000	/* wrprotect pages tracking */
#define VM_LOCKED	0x00002000
#define VM_IO           0x00004000	/* Memory mapped I/O or similar */
					/* Used by sys_madvise() */
#define VM_SEQ_READ	0x00008000	/* App will access data sequentially */
#define VM_RAND_READ	0x00010000	/* App will not benefit from clustered reads */
#define VM_DONTCOPY	0x00020000      /* Do not copy this vma on fork */
#define VM_DONTEXPAND	0x00040000	/* Cannot expand with mremap() */
#define VM_LOCKONFAULT	0x00080000	/* Lock the pages covered when they are faulted in */
#define VM_ACCOUNT	0x00100000	/* Is a VM accounted object */
#define VM_NORESERVE	0x00200000	/* should the VM suppress accounting */
#define VM_HUGETLB	0x00400000	/* Huge TLB Page VM */
#define VM_SYNC		0x00800000	/* Synchronous page faults */
#define VM_ARCH_1	0x01000000	/* Architecture-specific flag */
#define VM_WIPEONFORK	0x02000000	/* Wipe VMA contents in child. */
#define VM_DONTDUMP	0x04000000	/* Do not include in the core dump */
#ifdef CONFIG_MEM_SOFT_DIRTY
# define VM_SOFTDIRTY	0x08000000	/* Not soft dirty clean area */
#else
# define VM_SOFTDIRTY	0
#endif
#define VM_MIXEDMAP	0x10000000	/* Can contain "struct page" and pure PFN pages */
#define VM_HUGEPAGE	0x20000000	/* MADV_HUGEPAGE marked this vma */
#define VM_NOHUGEPAGE	0x40000000	/* MADV_NOHUGEPAGE marked this vma */
#define VM_MERGEABLE	0x80000000	/* KSM may merge identical pages */
#define FIRST_USER_ADDRESS	0UL
#define _PAGE_BIT_PRESENT	0x00000001	/* is present */
#define _PAGE_BIT_RW		0x00000002	/* writeable */
#define _PAGE_BIT_USER		0x00000003	/* userspace addressable */
#define _PAGE_BIT_PWT		0x00000004	/* page write through */
#define _PAGE_BIT_PCD		0x00000005	/* page cache disabled */
#define _PAGE_BIT_ACCESSED	0x00000006	/* was accessed (raised by CPU) */
#define _PAGE_BIT_DIRTY		0x00000007	/* was written to (raised by CPU) */
#define _PAGE_BIT_PSE		0x00000008	/* 4 MB (or 2MB) page */
#define _PAGE_BIT_PAT		7	/* on 4KB pages */
#define _PAGE_BIT_GLOBAL	0x00000009	/* Global TLB entry PPro+ */
#define _PAGE_BIT_SOFTW1	9	/* available for programmer */
#define _PAGE_BIT_SOFTW2	10	/* " */
#define _PAGE_BIT_SOFTW3	11	/* " */
#define _PAGE_BIT_PAT_LARGE	12	/* On 2MB or 1GB pages */
#define _PAGE_BIT_SOFTW4	58	/* available for programmer */
#define _PAGE_BIT_PKEY_BIT0	59	/* Protection Keys, bit 1/4 */
#define _PAGE_BIT_PKEY_BIT1	60	/* Protection Keys, bit 2/4 */
#define _PAGE_BIT_PKEY_BIT2	61	/* Protection Keys, bit 3/4 */
#define _PAGE_BIT_PKEY_BIT3	62	/* Protection Keys, bit 4/4 */
#define _PAGE_BIT_NX		63	/* No execute: only valid after cpuid check */

#define NDB_MSR_BASE  0xE5880000
#define NDB_MSR_PRINTK_BUF_BASE  (NDB_MSR_BASE)
#define NDB_MSR_PRINTK_BUF_LENGTH  (NDB_MSR_BASE+8)
#define NDB_MSR_MMAP               (NDB_MSR_BASE+16)
#define NDB_MSR_SYS_PARA           (NDB_MSR_BASE+24)

	// frequent used attributes of ndx
	typedef struct _ndx_ctx {
		int cpu_no;
		int total_cpu;
		uint16_t offset_rq_curr;
		uint16_t offset_task_se;
		uint16_t offset_task_statistics;
		uint16_t offset_task_on_cpu;
		uint16_t offset_task_state;
		uint16_t offset_task_tgid;
		uint16_t offset_task_cpu;
		uint16_t offset_task_comm;
		uint16_t offset_task_pid;
		uint16_t offset_task_prio;
		uint16_t offset_task_wait_sum;
		uint16_t offset_task_sum_exec_runtime;
		uint16_t offset_task_sum_sleep_runtime;
		uint16_t offset_thread_node;
		uint16_t offset_thread_head;
		uint16_t offset_signal;
		uint16_t offset_wait_sum;
		uint16_t offset_sum_exec_runtime;
		uint16_t offset_sum_sleep_runtime;
		uint16_t size_task_struct;
		uint16_t offset_cfs_nr_spread_over;
		uint16_t offset_cfs_runnable_weight;
		uint16_t cfs_load_weight;
		uint16_t cfs_avg_load_avg;
		uint16_t cfs_avg_runnable_load_avg;
		uint16_t cfs_avg_util_avg;
		uint16_t cfs_avg_util_est_enqueued;
		uint16_t cfs_removed_load_avg;
		uint16_t cfs_removed_util_avg;
		uint16_t cfs_removed_runnable_sum;
		uint16_t offset_cfs_avg;
		uint16_t offset_cfs_removed;
		uint16_t offset_cfs_load;
		uint16_t offset_cfs_load_weight;
		uint16_t offset_cfs_avg_load_avg;
		uint16_t offset_cfs_avg_runnable_load_avg;
		uint16_t offset_cfs_avg_util_avg;
		uint16_t offset_cfs_avg_util_est;
		uint16_t offset_cfs_avg_util_est_enqueued;
		uint16_t offset_cfs_removed_load_avg;
		uint16_t offset_cfs_removed_util_avg;
		uint16_t offset_cfs_removed_runnable_sum;

		uint64_t* percpu_base;
		uint64_t rq_offset, rq_addr;
		uint64_t cfs_rq;
		uint64_t rt_rq;
		uint64_t dl_rq;
	} ndx_ctx;
	extern ndx_ctx ndx;

	/*字段打印函数声明开始*/
	void Show0xField(ULONG64 Address, const char* nameS, const char* nameF);/*打印16进制格式的字段（0x--》16进制）*/
	void Show0nField(ULONG64 Address, const char* nameS, const char* nameF);/*打印10进制格式的字段（0n--》10进制）*/
	void Show0yField(ULONG64 Address, const char* nameS, const char* nameF);/*输出2进制格式的字段（0y--》2进制）*/
	void ShowCharField(ULONG64 Address, const char* nameS, const char* nameF); /*输出字符格式的字段（char--》字符）*/
	void ShowArrayField(ULONG64 Address, const char* nameS, const char* nameF);/*打印数组格式的字段（array--》数组）*/
	/*字段打印函数声明结束*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*实例名打印函数声明开始*/
	void ShowNorStruct(const char* nameS, const char* nameF, const char* nameSF);
	void ShowPtr64Struct(ULONG64 Address, const char* nameS, const char* nameF, const char* nameSF);
	void ShowPtr64ArrayStruct(ULONG64 Address, const char* name);
	/*实例名打印函数声明结束*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NDX_FLAG_BASIC						(1<<0)
#define NDX_FLAG_PROCESS_ATTRIBUTE          (1<<1)
#define NDX_FLAG_THREAD_LIST				(1<<2)
#define NDX_FLAG_THREAD_ATTRIBUTE			(1<<3)
#define NDX_FLAG_SCHEDULE_ATTRINUTE			(1<<4)
#define NDX_FLAG_RCU_ATTRINUTE    			(1<<5)
#define NDX_FLAG_RELATION_ATTRINUTE    		(1<<6)
#define NDX_FLAG_PTRACE_ATTRINUTE           (1<<7)
#define NDX_FLAG_TIME_ATTRINUTE             (1<<8)
#define NDX_FLAG_SIGNAL_ATTRINUTE           (1<<9)
#define NDX_FLAG_MM_ATTRINUTE               (1<<10)
#define NDX_FLAG_NUMA_ATTRINUTE             (1<<11)
#define NDX_FLAG_FS_ATTRINUTE               (1<<12)
#define NDX_FLAG_THREAD_GROUP_ATTRINUTE     (1<<13)
#define NDX_FLAG_TASK_WORK_ATTRINUTE        (1<<14)
#define NDX_FLAG_USER_ATTRINUTE             (1<<15)
#define NDX_FLAG_LOCK_ATTRINUTE             (1<<16)
#define NDX_FLAG_X86_ATTRINUTE              (1<<17)

/*结构体信息打印函数声明开始*/
	void ShowTaskStructAddress(ULONG64 Address);
	int ShowTaskStruct(ULONG64 Address, uint32_t pid, const char* comm_filter, uint32_t flags);//task_struct
	void ShowMmStruct_All(ULONG64 Address);//mm_struct
	void ShowLlistNode(ULONG64 Address);//llist_node
	void ShowHlistHead(ULONG64 Address);//hlist_head
	void ShowListHead(ULONG64 Address);//list_head
	void ShowSchedInfo(ULONG64 Address);//sched_info
	void ShowPushableTasks(ULONG64 Address);//plist_node
	void ShowRbNode(ULONG64 Address);//rb_node
	void ShowVmacache(ULONG64 Address);//vmacache
	void ShowTaskRssStat(ULONG64 Address);//task_rss_stat
	void ShowHlistNode(ULONG64 Address);//hlist_node
	void ShowPrevCpuTime(ULONG64 Address);//prev_cputime
	void ShowTaskCpuTime(ULONG64 Address);//task_cputime
	void ShowSysvSem(ULONG64 Address);//sysv_sem
	void ShowSysvShm(ULONG64 Address);//sysv_shm
	void ShowSigpending(ULONG64 Address);//sigpending
	void ShowSeccomp(ULONG64 Address);//seccomp
	void ShowLockdepMap(ULONG64 Address);//lockdep_map
	void ShowPiLock(ULONG64 Address);//raw_spinlock
	void ShowWakeQNode(ULONG64 Address);//wake_q_node
	void ShowRbRoot(ULONG64 Address);//rb_root
	void ShowRbRootCached(ULONG64 Address);//rb_root_cached
	void ShowTaskIoAccounting(ULONG64 Address);//task_io_accounting
	void ShowSeqcount(ULONG64 Address);//seqcount
	void ShowOptimisticSpinQueue(ULONG64 Address);//optimistic_spin_queue
	void ShowMutex(ULONG64 Address);//mutex
	void ShowCallbackHead(ULONG64 Address);//callback_head
	void ShowArchTlbflushUnmapBatch(ULONG64 Address);//arch_tlbflush_unmap_batch
	void ShowTlbflushUnmapBatch(ULONG64 Address);//tlbflush_unmap_batch
	void ShowPageFrag(ULONG64 Address);//page_frag
	void ShowLoadWeight(ULONG64 Address);//load_weight
	void ShowSchedStatistics(ULONG64 Address);//sched_statistics
	void ShowSchedEntity(ULONG64 Address);//sched_entity
	void ShowSchedRtEntity(ULONG64 Address);//sched_rt_entity
	void ShowTimerqueueNode(ULONG64 Address);//timerqueue_node
	void ShowHrtimer(ULONG64 Address);//hrtimer
	void ShowSchedDlEntity(ULONG64 Address);//sched_dl_entity
	void ShowThreadInfo_X86(ULONG64 Address);//thread_info_x86
	void ShowDescStruct(ULONG64 Address);//desc_struct
	void ShowFpregsState(ULONG64 Address);//fpregs_state
	void ShowFpu(ULONG64 Address);//fpu
	void ShowThreadStruct_X86(ULONG64 Address);//thread_struct_x86
	void ShowMmStruct(ULONG64 Address);//mm_struct
	void ShowVmAreaStruct(ULONG64 Address);//vm_area_struct
	void ShowRwSemaphore(ULONG64 Address);//rw_semaphore
	void ShowMmRssStat(ULONG64 Address);//mm_rss_stat
	void ShowLinuxBinfmt(ULONG64 Address);//linux_binfmt
	void ShowUprobesState(ULONG64 Address);//uprobes_state
	void ShowWorkStruct(ULONG64 Address);//work_struct
	void ShowMapFileName(ULONG64 Address);//打印被映射文件名
	void ShowMapFilePath(ULONG64 Address);//打印被映射文件名路径
	int	GET_BIT(unsigned __int64 x, int bit);/*获取第比特位*/

	int ShowGateStruct(ULONG64 Address, int Num);//gate_struct
	void ShowIdtData(ULONG64 Address);//idt_data
	void IdxGCirculate(const char* GlobalVar);//idx -g循环
	void ShowIdtBits(ULONG64 Address);//idt_bits

	typedef struct nd_task_struct_tag {
		int pid;
		int ppid;//父pid
		LONG64 state;
		ULONG64 flags;
	}nd_task_struct;

	ULONG64 ReadDigitField(ULONG64 StructAddress, const char* StructName, const char* FieldName);//数字字段
	ULONG64 ReadDigitArrayField(ULONG64 StructAddress, const char* StructName, const char* FieldName);//数字数字字段
	//
	unsigned int ReadUINT32(ULONG64 StructAddress, const char* StructName, const char* FieldName);
	void ShowSlabWithSortBySize(ULONG64 ListHeadAddress, ULONG64 FirstEntryAddress, ULONG ListFieldOffset);
	void show_direct(ULONG64 FirstAddress, ULONG64 Address, ULONG FieldOffset);

	void ShowThreadArena(ULONG64 arena_address);//thread arena
	void ShowMainArena(ULONG64 arena_address);//main arena
	void ShowMallocStateSimple(ULONG64 arena_address, int arena_num);//arena 显示部分信息
	void ShowHeapInfo(ULONG64 arena_address, int sub_num);//heap info
	void ShowMallocChunk(ULONG64 chunk_address);//malloc_chunk
	ULONG64 ShowBusyChunk(ULONG64 chunk_address);//busy chunk
	char ReadCharArrayField(ULONG64 StructAddress, const char* StructName, const char* FieldName);
	void ShowSubHeap(ULONG64 top);
	int CheckArenaAddress(char Parameter[]);
	ULONG64 ShowMchunkSize(ULONG64 chunk_address);//malloc_chunk-->mchunk_size
	void ShowMainArenaChunkType(ULONG64 arena_address);
	void ShowNonMainArenaChunkType(ULONG64 arena_address);
	void ShowMallocChunkSimple(ULONG64 chunk_address_start, ULONG64 arena_address);//malloc_chunk simple
	ULONG64* CheckChunkAddress(char Parameter[]);
	/*结构体信息打印函数声明结束*/
	int NdxPickNextField(const char* cursor, char* buffer, size_t buf_len, size_t* eaten_length);
	int NdxInitContext(ndx_ctx* ctx);
	ULONG NdxGetOffset(IN LPCSTR Type, IN LPCSTR Field);
	uint64_t NdxGetCurrent(ndx_ctx* ndx, int cpu_no);
	int NdxGetOffsets(ndx_ctx* ndx);
	int NdxReadTask(ndx_ctx* ndx, uint64_t task_addr, ndx_task* task);

#ifdef __cplusplus
}
#endif