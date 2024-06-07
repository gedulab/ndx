#include "ndx.h"
#include <chrono>
ULONG64 CodeCount = 0;
ULONG64 WorkCount = 0;
ULONG64 WriteCount = 0;
ULONG64 ShareCount = 0;

/*结束全局变量的定义及声明*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*打印字段开始*/
void Show0xField(ULONG64 Address, const char *nameS, const char *nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	GetFieldValue(Address, nameS, nameF, value);

	dprintf("+0x%04lx %s = 0x%llx \n", Offset, nameF, value);
}/*打印16进制格式的字段（0x--》16进制）*/


void Show0xTaskField(char* task_cache, const char* nameS, const char* nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	//GetFieldValue(Address, nameS, nameF, value);
	value = *(ULONG64*)(task_cache + Offset);

	dprintf("+0x%04lx %s = 0x%llx \n", Offset, nameF, value);
}/*打印16进制格式的字段（0x--》16进制）*/


void Show0nField(ULONG64 Address, const char *nameS, const char *nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	GetFieldValue(Address, nameS, nameF, value);

	dprintf("+0x%04lx %s = %d\n", Offset, nameF, value);
}/*打印10进制格式的字段（--》10进制）*/


void Show0nTaskField(char* task_cache, const char* nameS, const char* nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	//GetFieldValue(Address, nameS, nameF, value);
	value = *(ULONG64*)(task_cache + Offset);

	dprintf("+0x%04lx %s = %d\n", Offset, nameF, value);
}/*打印10进制格式的字段（--》10进制）*/

//改造条件，将ULONG64 Address替换为一个已经读好的task_struct cache
//这样，在读取的时候就可以直接取，不必再用GetFieldValue
//在原来的基础上，将所有调用此函数的地方全部进行改进
void Show0yField(ULONG64 Address, const char* nameS, const char* nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	GetFieldValue(Address, nameS, nameF, value);

	char Base_value[64];
	_itoa_s(value, Base_value, 2);
	dprintf("+0x%04lx %s = 0y%s \n", Offset, nameF, Base_value);
}/*打印2进制格式的字段（0y--》2进制）*/

void Show0yTaskField(char* task_cache, const char* nameS, const char* nameF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	//GetFieldValue(Address, nameS, nameF, value);

	value = *(ULONG64*)(task_cache + Offset);

	char Base_value[64];
	_itoa_s(value, Base_value, 2);
	dprintf("+0x%04lx %s = 0y%s \n", Offset, nameF, Base_value);
}/*打印2进制格式的字段（0y--》2进制）*/



void ShowCharField(ULONG64 Address, const char *nameS, const char *nameF)
{
	char value[MAX_PATH];
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	ReadPointer(Address+Offset,&Address);
	GetFieldValue(Address, nameS, nameF, value);

	dprintf("+0x%04lx %s = %s\n", Offset, nameF, value);
}/*打印字符格式的字段（char--》字符）*/
void ShowArrayField(ULONG64 Address, const char *nameS, const char *nameF)
{
	ULONG64 value[15];
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	GetFieldValue(Address, nameS, nameF, value);

	dprintf("+0x%04lx %s = %d\n", Offset, nameF, value[0]);
}/*打印数组格式的字段（array--》数组）*/
/*打印字段结束*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*打印实例名开始*/
void ShowNorStruct(const char *nameS, const char *nameF, const char *nameSF)
{
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);

	dprintf("+0x%04lx %s = %s\n", Offset, nameF, nameSF);
}/*打印实例名*/
void ShowPtr64Struct(ULONG64 Address,const char *nameS, const char *nameF, const char *nameSF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	GetFieldValue(Address, nameS, nameF, value);

	dprintf("+0x%04lx %s = 0x%p %s\n", Offset, nameF, value, nameSF);
}/*打印地址及实例名*/

void ShowPtr64TaskStruct(char* task_cache, const char* nameS, const char* nameF, const char* nameSF)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset(nameS, nameF, &Offset);
	//GetFieldValue(Address, nameS, nameF, value);
	value = *(ULONG64*)(task_cache + Offset);

	dprintf("+0x%04lx %s = 0x%p %s\n", Offset, nameF, value, nameSF);
}/*打印地址及实例名*/

void ShowPtr64ArrayStruct(ULONG64 Address, const char *name)
{
	ULONG64 value[3];
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%04lx %s = 0x%p\n", Offset, name, value);
}
/*打印实例名结束*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*打印结构体信息开始*/
void ShowLlistNode(ULONG64 Address)//llist_node
{
	ULONG64 next;

	GetFieldValue(Address, "llist_node", "next", next);

	dprintf("\tnext = 0x%p\n", next);
}
void ShowHlistHead(ULONG64 Address)//hlist_head
{
	ULONG64 first;

	GetFieldValue(Address, "hlist_head", "first", first);

	dprintf("\tfirst = 0x%p\n", first);
}
void ShowListHead(ULONG64 Address)//list_head
{
	ULONG64 next, prev;

	GetFieldValue(Address, "list_head", "next", next);
	GetFieldValue(Address, "list_head", "prev", prev);

	dprintf("\t{next = 0x%p prev = 0x%p}\n", next, prev);
}
void ShowSchedInfo(ULONG64 Address)//sched_info
{
	ULONG64 pcount, run_delay, last_arrival, last_queued;

	GetFieldValue(Address, "sched_info", "pcount", pcount);
	GetFieldValue(Address, "sched_info", "run_delay", run_delay);
	GetFieldValue(Address, "sched_info", "last_arrival", last_arrival);
	GetFieldValue(Address, "sched_info", "last_queued", last_queued);

	dprintf("\tpcount\t= %d run_delay   = %d\n", pcount, run_delay);
	dprintf("\tlast_arrival = %d last_queued = %d\n", last_arrival, run_delay);
}
void ShowPushableTasks(ULONG64 Address)//plist_node
{
	ULONG64 prio;
	ULONG Offset;

	GetFieldValue(Address, "sched_info", "prio", prio);

	dprintf("\tprio = %d\n", prio);

	dprintf("\tprio_list = list_head\n");
	GetFieldOffset("plist_node", "prio_list", &Offset);
	ShowListHead(Address + Offset);
	dprintf("\tnode_list = list_head\n");
	GetFieldOffset("plist_node", "node_list", &Offset);
	ShowListHead(Address + Offset);
}
void ShowRbNode(ULONG64 Address)//rb_node
{
	ULONG64 __rb_parent_color, rb_right, rb_left;

	GetFieldValue(Address, "rb_node", "__rb_parent_color", __rb_parent_color);
	GetFieldValue(Address, "rb_node", "rb_right", rb_right);
	GetFieldValue(Address, "rb_node", "rb_left", rb_left);

	dprintf("\t__rb_parent_color = %d\n", __rb_parent_color);
	dprintf("\trb_right = 0x%p rb_left = 0x%p\n", rb_right, rb_left);
}
void ShowVmacache(ULONG64 Address)//vmacache
{
	ULONG64 seqnum, vmas[10];

	GetFieldValue(Address, "vmacache", "seqnum", seqnum);
	GetFieldValue(Address, "vmacache", "vmas", vmas);

	dprintf("\tseqnum   = %d\n", seqnum);
	dprintf("\trb_right = 0x%p\n", vmas);
}
void ShowTaskRssStat(ULONG64 Address)//task_rss_stat
{
	ULONG64 event, count[10];

	GetFieldValue(Address, "task_rss_stat", "event", event);
	GetFieldValue(Address, "task_rss_stat", "count", count);

	dprintf("\tevent = %d count = %d\n", event, count);
}
void ShowHlistNode(ULONG64 Address)//hlist_node
{
	ULONG64 next, pprev;

	GetFieldValue(Address, "hlist_node", "next", next);
	GetFieldValue(Address, "hlist_node", "pprev", pprev);

	dprintf("\tnext = 0x%p pprev = 0x%p\n", next, pprev);
}
void ShowPrevCpuTime(ULONG64 Address)//prev_cputime
{
	ULONG64 utime, stime;

	GetFieldValue(Address, "prev_cputime", "utime", utime);
	GetFieldValue(Address, "prev_cputime", "stime", stime);

	dprintf("\tutime = %d stime = %d\n", utime, stime);
	dprintf("\tlock  = raw_spinlock\n");
}
void ShowTaskCpuTime(ULONG64 Address)//task_cputime
{
	ULONG64 utime, stime, sum_exec_runtime;

	GetFieldValue(Address, "task_cputime", "utime", utime);
	GetFieldValue(Address, "task_cputime", "stime", stime);
	GetFieldValue(Address, "task_cputime", "sum_exec_runtime", sum_exec_runtime);

	dprintf("\tutime = %d stime = %d\n", utime, stime);
	dprintf("\tsum_exec_runtime = %d\n", sum_exec_runtime);
}
void ShowSysvSem(ULONG64 Address)//sysv_sem
{
	ULONG64 undo_list;

	GetFieldValue(Address, "sysv_sem", "undo_list", undo_list);

	dprintf("\tundo_list = 0x%p\n", undo_list);
}
void ShowSysvShm(ULONG64 Address)//sysv_shm
{
	ULONG Offset;

	dprintf("\tshm_clist = list_head\n");
	GetFieldOffset("sysv_shm", "shm_clist", &Offset);
	ShowListHead(Address + Offset);
}
void ShowSigpending(ULONG64 Address)//sigpending
{
	ULONG Offset;

	dprintf("\tlist = list_head\n");
	GetFieldOffset("sigpending", "list", &Offset);
	ShowListHead(Address + Offset);

	dprintf("\tsignal  = sigset\n");
}
void ShowSeccomp(ULONG64 Address)//seccomp
{
	ULONG64 mode, filter;

	GetFieldValue(Address, "seccomp", "mode", mode);
	GetFieldValue(Address, "seccomp", "filter", filter);

	dprintf("\tmode = %d stime = %d\n", mode, filter);
}
void ShowLockdepMap(ULONG64 Address)//lockdep_map
{
	char name[50];
	ULONG64 key, cpu, ip, class_cache[10];

	GetFieldValue(Address, "lockdep_map", "key", key);
	GetFieldValue(Address, "lockdep_map", "class_cache", class_cache);
	GetFieldValue(Address, "lockdep_map", "name", name);
	GetFieldValue(Address, "lockdep_map", "cpu", cpu);
	GetFieldValue(Address, "lockdep_map", "ip", ip);

	dprintf("\tkey = 0x%p\n", key);
	dprintf("\tclass_cache  = 0x%p\n", class_cache);
	dprintf("\tname = %s\n", name);
	dprintf("\tcpu  = %d ip = %d\n", cpu, ip);
}
void ShowPiLock(ULONG64 Address)//raw_spinlock
{
	ULONG64 magic, owner_cpu, owner;
	ULONG Offset;

	GetFieldValue(Address, "raw_spinlock", "magic", magic);
	GetFieldValue(Address, "raw_spinlock", "owner_cpu", owner_cpu);
	GetFieldValue(Address, "raw_spinlock", "owner", owner);

	dprintf("\traw_lock = arch_spinlock\n");
	dprintf("\tmagic = %d owner_cpu = %d\n", magic, owner_cpu);
	dprintf("\towner = 0x%p\n", owner);

	dprintf("\tdep_map = lockdep_map\n");
	GetFieldOffset("raw_spinlock", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
void ShowWakeQNode(ULONG64 Address)//wake_q_node
{
	ULONG64 next;

	GetFieldValue(Address, "wake_q_node", "next", next);

	dprintf("\tnext = 0x%p\n", next);
}
void ShowRbRoot(ULONG64 Address)//rb_root
{
	ULONG64 rb_node;

	GetFieldValue(Address, "rb_root", "rb_node", rb_node);

	dprintf("\trb_node = 0x%p\n", rb_node);
}
void ShowRbRootCached(ULONG64 Address)//rb_root_cached
{
	ULONG64 rb_leftmost;
	ULONG Offset;

	GetFieldValue(Address, "wake_q_node", "rb_leftmost", rb_leftmost);

	dprintf("\trb_root = rb_root\n");
	GetFieldOffset("rb_root_cached", "rb_root", &Offset);
	ShowRbRoot(Address + Offset);

	dprintf("\trb_leftmost = 0x%p\n", rb_leftmost);
}
void ShowTaskIoAccounting(ULONG64 Address)//task_io_accounting
{
	ULONG64 rchar, wchar, syscr, syscw, read_bytes, write_bytes, cancelled_write_bytes;

	GetFieldValue(Address, "task_io_accounting", "rchar", rchar);
	GetFieldValue(Address, "task_io_accounting", "wchar", wchar);
	GetFieldValue(Address, "task_io_accounting", "syscr", syscr);
	GetFieldValue(Address, "task_io_accounting", "syscw", syscw);
	GetFieldValue(Address, "task_io_accounting", "read_bytes", read_bytes);
	GetFieldValue(Address, "task_io_accounting", "write_bytes", write_bytes);
	GetFieldValue(Address, "task_io_accounting", "cancelled_write_bytes", cancelled_write_bytes);

	dprintf("\trchar = %d wchar = %d\n", rchar, wchar);
	dprintf("\tsyscr = %d syscw = %d\n", syscr, syscw);
	dprintf("\tread_bytes = %d write_bytes = %d\n", read_bytes, write_bytes);
	dprintf("\tcancelled_write_bytes = %d\n", cancelled_write_bytes);
}
void ShowSeqcount(ULONG64 Address)//seqcount
{
	ULONG64 sequence;
	ULONG Offset;

	GetFieldValue(Address, "seqcount", "sequence", sequence);

	dprintf("\tsequence = %d\n", sequence);

	dprintf("\tdep_map = lockdep_map\n");
	GetFieldOffset("seqcount", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
void ShowOptimisticSpinQueue(ULONG64 Address)//optimistic_spin_queue
{
	dprintf("\ttail = atomic\n");
}
void ShowMutex(ULONG64 Address)//mutex
{
	ULONG64 magic;
	ULONG Offset;

	GetFieldValue(Address, "mutex", "magic", magic);

	dprintf("\towner = atomic_long\n");
	dprintf("\twait_lock = spinlock\n");
	dprintf("\tmagic = %d\n", magic);

	dprintf("\tosq = optimistic_spin_queue\n");
	GetFieldOffset("mutex", "osq", &Offset);
	ShowOptimisticSpinQueue(Address + Offset);
	dprintf("\twait_list = list_head\n");
	GetFieldOffset("mutex", "wait_list", &Offset);
	ShowListHead(Address + Offset);
	dprintf("\tdep_map = lockdep_map\n");
	GetFieldOffset("mutex", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
void ShowCallbackHead(ULONG64 Address)//callback_head
{
	ULONG64 next, func;

	GetFieldValue(Address, "callback_head", "next", next);
	GetFieldValue(Address, "callback_head", "func", func);

	dprintf("\tnext = 0x%p func = 0x%p\n", next, func);
}
void ShowArchTlbflushUnmapBatch(ULONG64 Address)//arch_tlbflush_unmap_batch
{
	dprintf("\tcpumask = cpumask\n");
}
void ShowTlbflushUnmapBatch(ULONG64 Address)//tlbflush_unmap_batch
{
	bool flush_required, writable;
	ULONG Offset;

	GetFieldValue(Address, "tlbflush_unmap_batch", "flush_required", flush_required);
	GetFieldValue(Address, "tlbflush_unmap_batch", "writable", writable);

	dprintf("\tflush_required = %s writable0 = %s\n", flush_required, writable);

	dprintf("\tarch = arch_tlbflush_unmap_batch\n");
	GetFieldOffset("tlbflush_unmap_batch", "arch", &Offset);
	ShowArchTlbflushUnmapBatch(Address + Offset);
}
void ShowPageFrag(ULONG64 Address)//page_frag
{
	ULONG64 page, offset, size;

	GetFieldValue(Address, "page_frag", "page", page);
	GetFieldValue(Address, "page_frag", "offset", offset);
	GetFieldValue(Address, "page_frag", "size", size);

	dprintf("\tpage = 0x%p\n", page);
	dprintf("\toffset = %d size = %d\n", offset, size);
}
void ShowLoadWeight(ULONG64 Address)//load_weight
{
	ULONG64 weight, inv_weight;

	GetFieldValue(Address, "load_weight", "weight", weight);
	GetFieldValue(Address, "load_weight", "inv_weight", inv_weight);

	dprintf("\tweight = %d inv_weight = %d\n", weight, inv_weight);
}
void ShowSchedStatistics(ULONG64 Address)//sched_statistics
{
	ULONG64 wait_start, wait_max, wait_count, wait_sum,
		iowait_count, iowait_sum, sleep_start, sleep_max,
		sum_sleep_runtime, block_start, block_max, exec_max,
		slice_max, nr_migrations_cold, nr_failed_migrations_affine, nr_failed_migrations_running,
		nr_failed_migrations_hot, nr_forced_migrations, nr_wakeups, nr_wakeups_sync,
		nr_wakeups_migrate, nr_wakeups_local, nr_wakeups_remote, nr_wakeups_affine,
		nr_wakeups_affine_attempts, nr_wakeups_passive, nr_wakeups_idle;

	GetFieldValue(Address, "sched_statistics", "wait_start", wait_start);
	GetFieldValue(Address, "sched_statistics", "wait_max", wait_max);
	GetFieldValue(Address, "sched_statistics", "wait_count", wait_count);
	GetFieldValue(Address, "sched_statistics", "wait_sum", wait_sum);
	GetFieldValue(Address, "sched_statistics", "iowait_count", iowait_count);
	GetFieldValue(Address, "sched_statistics", "iowait_sum", iowait_sum);
	GetFieldValue(Address, "sched_statistics", "sleep_start", sleep_start);
	GetFieldValue(Address, "sched_statistics", "sleep_max", sleep_max);
	GetFieldValue(Address, "sched_statistics", "sum_sleep_runtime", sum_sleep_runtime);
	GetFieldValue(Address, "sched_statistics", "block_start", block_start);
	GetFieldValue(Address, "sched_statistics", "block_max", block_max);
	GetFieldValue(Address, "sched_statistics", "exec_max", exec_max);
	GetFieldValue(Address, "sched_statistics", "slice_max", slice_max);
	GetFieldValue(Address, "sched_statistics", "nr_migrations_cold", nr_migrations_cold);
	GetFieldValue(Address, "sched_statistics", "nr_failed_migrations_affine", nr_failed_migrations_affine);
	GetFieldValue(Address, "sched_statistics", "nr_failed_migrations_running", nr_failed_migrations_running);
	GetFieldValue(Address, "sched_statistics", "nr_failed_migrations_hot", nr_failed_migrations_hot);
	GetFieldValue(Address, "sched_statistics", "nr_forced_migrations", nr_forced_migrations);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups", nr_wakeups);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_sync", nr_wakeups_sync);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_migrate", nr_wakeups_migrate);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_local", nr_wakeups_local);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_remote", nr_wakeups_remote);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_affine", nr_wakeups_affine);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_affine_attempts", nr_wakeups_affine_attempts);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_passive", nr_wakeups_passive);
	GetFieldValue(Address, "sched_statistics", "nr_wakeups_idle", nr_wakeups_idle);

	dprintf("\twait_start = %d wait_max = %d\n", wait_start, wait_max);
	dprintf("\twait_count = %d wait_sum = %d\n", wait_count, wait_sum);
	dprintf("\tiowait_count = %d iowait_sum = %d\n", iowait_count, iowait_sum);
	dprintf("\tsleep_start = %d sleep_max = %d\n", sleep_start, sleep_max);
	dprintf("\tsum_sleep_runtime = %d block_start = %d\n", sum_sleep_runtime, block_start);
	dprintf("\tblock_max = %d exec_max = %d\n", block_max, exec_max);
	dprintf("\tslice_max = %d nr_migrations_cold = %d\n", slice_max, nr_migrations_cold);
	dprintf("\tnr_failed_migrations_affine = %d nr_failed_migrations_running = %d\n", nr_failed_migrations_affine, nr_failed_migrations_running);
	dprintf("\tnr_failed_migrations_hot = %d nr_forced_migrations = %d\n", nr_failed_migrations_hot, nr_forced_migrations);
	dprintf("\tnr_wakeups = %d inv_weight = %d\n", nr_wakeups, nr_wakeups_sync);
	dprintf("\tnr_wakeups_migrate = %d nr_wakeups_local = %d\n", nr_wakeups_migrate, nr_wakeups_local);
	dprintf("\tnr_wakeups_remote = %d nr_wakeups_affine = %d\n", nr_wakeups_remote, nr_wakeups_affine);
	dprintf("\tnr_wakeups_affine_attempts = %d nr_wakeups_passive = %d\n", nr_wakeups_affine_attempts, nr_wakeups_passive);
	dprintf("\tnr_wakeups_idle = %dn", nr_wakeups_idle);
}
void ShowSchedEntity(ULONG64 Address)//sched_entity
{
	ULONG64 runnable_weight, on_rq, exec_start, sum_exec_runtime,
		vruntime, prev_sum_exec_runtime, nr_migrations, depth,
		parent, cfs_rq, my_q;
	ULONG Offset;

	GetFieldValue(Address, "sched_entity", "runnable_weight", runnable_weight);
	GetFieldValue(Address, "sched_entity", "on_rq", on_rq);
	GetFieldValue(Address, "sched_entity", "exec_start", exec_start);
	GetFieldValue(Address, "sched_entity", "sum_exec_runtime", sum_exec_runtime);
	GetFieldValue(Address, "sched_entity", "vruntime", vruntime);
	GetFieldValue(Address, "sched_entity", "prev_sum_exec_runtime", prev_sum_exec_runtime);
	GetFieldValue(Address, "sched_entity", "nr_migrations", nr_migrations);
	GetFieldValue(Address, "sched_entity", "depth", depth);
	GetFieldValue(Address, "sched_entity", "parent", parent);
	GetFieldValue(Address, "sched_entity", "cfs_rq", cfs_rq);
	GetFieldValue(Address, "sched_entity", "my_q", my_q);

	dprintf("\trunnable_weight  = %d on_rq = %d\n", runnable_weight, on_rq);
	dprintf("\texec_start  = %d sum_exec_runtime = %d\n", exec_start, sum_exec_runtime);
	dprintf("\tvruntime  = %d prev_sum_exec_runtime = %d\n", vruntime, prev_sum_exec_runtime);
	dprintf("\tnr_migrations  = %d depth = %d\n", nr_migrations, depth);
	dprintf("\tparent = 0x%p\n", parent);
	dprintf("\tcfs_rq = 0x%p\n", cfs_rq);
	dprintf("\tmy_q = 0x%p\n", my_q);

	dprintf("\trun_node = rb_node\n");
	GetFieldOffset("sched_entity", "run_node", &Offset);
	ShowRbNode(Address + Offset);
	dprintf("\tgroup_node = list_head\n");
	GetFieldOffset("sched_entity", "group_node", &Offset);
	ShowListHead(Address + Offset);
	dprintf("\tload = load_weight\n", my_q);
	GetFieldOffset("sched_entity", "load", &Offset);
	ShowLoadWeight(Address + Offset);
	dprintf("\tstatistics = sched_statistics\n");
	GetFieldOffset("sched_entity", "statistics", &Offset);
	ShowSchedStatistics(Address + Offset);
}
void ShowSchedRtEntity(ULONG64 Address)//sched_rt_entity
{
	ULONG64 timeout, watchdog_stamp, time_slice, on_rq,
		on_list, back, parent, rt_rq,
		my_q;
	ULONG Offset;

	GetFieldValue(Address, "sched_rt_entity", "timeout", timeout);
	GetFieldValue(Address, "sched_rt_entity", "watchdog_stamp", watchdog_stamp);
	GetFieldValue(Address, "sched_rt_entity", "time_slice", time_slice);
	GetFieldValue(Address, "sched_rt_entity", "on_rq", on_rq);
	GetFieldValue(Address, "sched_rt_entity", "on_list", on_list);
	GetFieldValue(Address, "sched_rt_entity", "back", back);
	GetFieldValue(Address, "sched_rt_entity", "parent", parent);
	GetFieldValue(Address, "sched_rt_entity", "rt_rq", rt_rq);
	GetFieldValue(Address, "sched_rt_entity", "my_q", my_q);

	dprintf("\ttimeout  = %d watchdog_stamp = %d\n", timeout, watchdog_stamp);
	dprintf("\ttime_slice  = %d on_rq = %d\n", time_slice, on_rq);
	dprintf("\ton_list  = %d\n", on_list);
	dprintf("\tback = 0x%p\n", back);
	dprintf("\tparent = 0x%p\n", parent);
	dprintf("\trt_rq = 0x%p\n", rt_rq);
	dprintf("\tmy_q = 0x%p\n", my_q);

	dprintf("\trun_list = list_head\n");
	GetFieldOffset("sched_rt_entity", "run_list", &Offset);
	ShowListHead(Address + Offset);
}
void ShowTimerqueueNode(ULONG64 Address)//timerqueue_node
{
	ULONG64 next;
	ULONG Offset;

	dprintf("\tnode = rb_node\n");
	GetFieldOffset("timerqueue_node", "node", &Offset);
	ShowRbNode(Address + Offset);

	GetFieldValue(Address, "load_weight", "timerqueue_node", next);

	dprintf("\tnext = %p\n", next);
}
void ShowHrtimer(ULONG64 Address)//hrtimer
{
	ULONG64 state, is_rel, is_soft, function, base;
	ULONG Offset;

	dprintf("\tnode = timerqueue_node\n");
	GetFieldOffset("hrtimer", "node", &Offset);
	ShowTimerqueueNode(Address + Offset);

	GetFieldValue(Address, "load_weight", "state", state);
	GetFieldValue(Address, "load_weight", "is_rel", is_rel);
	GetFieldValue(Address, "load_weight", "is_soft", is_soft);
	GetFieldValue(Address, "load_weight", "function", function);
	GetFieldValue(Address, "load_weight", "base", base);

	dprintf("\tstate = %d is_rel = %d\n", state, is_rel);
	dprintf("\tis_soft = %d\n", is_soft);
	dprintf("\tfunction = 0x%p\n", function);
	dprintf("\tbase = 0x%p\n", base);
}
void ShowSchedDlEntity(ULONG64 Address)//sched_dl_entity
{
	ULONG64 dl_runtime, dl_deadline, dl_period, dl_bw,
		dl_density, runtime, deadline, flags,
		dl_throttled, dl_boosted, dl_yielded, dl_non_contending,
		dl_overrun;
	ULONG Offset;

	GetFieldValue(Address, "sched_rt_entity", "dl_runtime", dl_runtime);
	GetFieldValue(Address, "sched_rt_entity", "dl_deadline", dl_deadline);
	GetFieldValue(Address, "sched_rt_entity", "dl_period", dl_period);
	GetFieldValue(Address, "sched_rt_entity", "dl_bw", dl_bw);
	GetFieldValue(Address, "sched_rt_entity", "dl_density", dl_density);
	GetFieldValue(Address, "sched_rt_entity", "runtime", runtime);
	GetFieldValue(Address, "sched_rt_entity", "deadline", deadline);
	GetFieldValue(Address, "sched_rt_entity", "flags", flags);
	GetFieldValue(Address, "sched_rt_entity", "dl_throttled", dl_throttled);
	GetFieldValue(Address, "sched_rt_entity", "dl_boosted", dl_boosted);
	GetFieldValue(Address, "sched_rt_entity", "dl_yielded", dl_yielded);
	GetFieldValue(Address, "sched_rt_entity", "dl_non_contending", dl_non_contending);
	GetFieldValue(Address, "sched_rt_entity", "dl_overrun", dl_overrun);

	char Base_dl_throttled[64], Base_dl_boosted[64], Base_dl_yielded[64],
		Base_dl_non_contending[64], Base_dl_overrun[64];
	_itoa_s(dl_throttled, Base_dl_throttled, 2);
	_itoa_s(dl_boosted, Base_dl_boosted, 2);
	_itoa_s(dl_yielded, Base_dl_yielded, 2);
	_itoa_s(dl_non_contending, Base_dl_non_contending, 2);
	_itoa_s(dl_overrun, Base_dl_overrun, 2);

	dprintf("\tdl_runtime  = %d dl_deadline = %d\n", dl_runtime, dl_deadline);
	dprintf("\tdl_period  = %d dl_bw = %d\n", dl_period, dl_bw);
	dprintf("\tdl_density  = %d runtime = %d\n", dl_density, runtime);
	dprintf("\tdeadline  = %d flags = 0x%p\n", deadline, flags);
	dprintf("\tdl_throttled  = 0y%s dl_boosted = 0y%s\n", Base_dl_throttled, Base_dl_boosted);
	dprintf("\tdl_yielded  = 0y%s dl_non_contending = 0y%s\n", Base_dl_yielded, Base_dl_non_contending);
	dprintf("\tdl_overrun  = 0y%s\n", Base_dl_overrun);

	dprintf("\trb_node = rb_node\n");
	GetFieldOffset("sched_dl_entity", "rb_node", &Offset);
	ShowRbNode(Address + Offset);
	dprintf("\tdl_timer = hrtimer\n");
	GetFieldOffset("sched_dl_entity", "dl_timer", &Offset);
	ShowHrtimer(Address + Offset);
	dprintf("\tinactive_timer = hrtimer\n");
	GetFieldOffset("sched_dl_entity", "inactive_timer", &Offset);
	ShowHrtimer(Address + Offset);
}
void ShowThreadInfo_X86(ULONG64 Address)//thread_info
{
	ULONG64 flags, status;

	GetFieldValue(Address, "thread_info", "flags", flags);
	GetFieldValue(Address, "thread_info", "status", status);

	dprintf("\tflags = 0x%p status = %d\n", flags, status);
}
void ShowDescStruct(ULONG64 Address)//desc_struct
{
	ULONG64 limit0, base0, base1, limit1;

	GetFieldValue(Address, "desc_struct", "limit0", limit0);
	GetFieldValue(Address, "desc_struct", "base0", base0);
	GetFieldValue(Address, "desc_struct", "base1", base1);
	GetFieldValue(Address, "desc_struct", "limit1", limit1);

	dprintf("\tlimit0 = %d base0 = %d\n", limit0, base0);
	dprintf("\tbase1 = %d limit1 = %d\n", base1, limit1);
}
void ShowFpregsState(ULONG64 Address)//fpregs_state
{
	dprintf("\tfsave = fregs_state\n");
	dprintf("\tfxsave = fxregs_state\n");
	dprintf("\tsoft = swregs_state\n");
	dprintf("\txsave = xregs_state\n");
}
void ShowFpu(ULONG64 Address)//fpu
{
	ULONG64 last_cpu;
	UCHAR initialized;
	ULONG Offset;

	GetFieldValue(Address, "desc_struct", "last_cpu", last_cpu);
	GetFieldValue(Address, "desc_struct", "initialized", initialized);

	dprintf("\tlast_cpu = %d initialized = %s\n", last_cpu, initialized);

	dprintf("\tstate = fpregs_state\n");
	GetFieldOffset("thread_struct", "state", &Offset);
	ShowFpregsState(Address + Offset);
}
void ShowThreadStruct_X86(ULONG64 Address)//thread_struct
{
	ULONG64 sp, es, ds, fsindex,
		gsindex, fsbase, gsbase, ptrace_bps[5],
		ptrace_dr7, cr2, trap_nr, error_code,
		io_bitmap_ptr, iopl, io_bitmap_max, addr_limit,
		sig_on_uaccess_err, uaccess_err;
	ULONG Offset;
	char Base_sig_on_uaccess_err[64], Base_uaccess_err[64];

	dprintf("\ttls_array = desc_struct\n");
	GetFieldOffset("thread_struct", "tls_array", &Offset);
	ShowDescStruct(Address + Offset);

	GetFieldValue(Address, "thread_struct", "sp", sp);
	GetFieldValue(Address, "thread_struct", "es", es);
	GetFieldValue(Address, "thread_struct", "ds", ds);
	GetFieldValue(Address, "thread_struct", "fsindex", fsindex);
	GetFieldValue(Address, "thread_struct", "gsindex", gsindex);
	GetFieldValue(Address, "thread_struct", "fsbase", fsbase);
	GetFieldValue(Address, "thread_struct", "gsbase", gsbase);
	GetFieldValue(Address, "thread_struct", "ptrace_bps", ptrace_bps);
	GetFieldValue(Address, "thread_struct", "ptrace_dr7", ptrace_dr7);
	GetFieldValue(Address, "thread_struct", "cr2", cr2);
	GetFieldValue(Address, "thread_struct", "trap_nr", trap_nr);
	GetFieldValue(Address, "thread_struct", "error_code", error_code);
	GetFieldValue(Address, "thread_struct", "io_bitmap_ptr", io_bitmap_ptr);
	GetFieldValue(Address, "thread_struct", "iopl", iopl);
	GetFieldValue(Address, "thread_struct", "io_bitmap_max", io_bitmap_max);
	GetFieldValue(Address, "thread_struct", "addr_limit", addr_limit);
	GetFieldValue(Address, "thread_struct", "sig_on_uaccess_err", sig_on_uaccess_err);
	GetFieldValue(Address, "thread_struct", "uaccess_err", uaccess_err);

	_itoa_s(sig_on_uaccess_err, Base_sig_on_uaccess_err, 2);
	_itoa_s(uaccess_err, Base_uaccess_err, 2);

	dprintf("\tsp = %d es = %d\n", sp, es);
	dprintf("\tds = %d fsindex = %d\n", ds, fsindex);
	dprintf("\tgsindex = %d fsbase = %d\n", gsindex, fsbase);
	dprintf("\tgsbase = %d ptrace_bps = 0x%p\n", gsbase, ptrace_bps);
	dprintf("\tptrace_dr7 = %d cr2 = %d\n", ptrace_dr7, cr2);
	dprintf("\ttrap_nr = %d error_code = %d\n", trap_nr, error_code);
	dprintf("\tio_bitmap_ptr = 0x%p iopl = %d\n", io_bitmap_ptr, iopl);
	dprintf("\tio_bitmap_max = %d addr_limit = %d\n", io_bitmap_max, addr_limit);
	dprintf("\tsig_on_uaccess_err = 0y%s uaccess_err = 0y%s\n", Base_sig_on_uaccess_err, Base_uaccess_err);

	dprintf("\tfpu = fpu\n");
	GetFieldOffset("thread_struct", "fpu", &Offset);
	ShowFpu(Address + Offset);
}
void ShowMmStruct(ULONG64 Address)//mm_struct
{
	ULONG64 mmap, get_unmapped_area, mmap_base, mmap_legacy_base,
		mmap_compat_base, mmap_compat_legacy_base, task_size, highest_vm_end,
		pgd, map_count, hiwater_rss, hiwater_vm, 
		total_vm, locked_vm, pinned_vm, data_vm,
		exec_vm, stack_vm, def_flags, start_code,
		end_code, start_data, end_data, start_brk,
		brk, start_stack, arg_start, arg_end,
		env_start, env_end, saved_auxv[5], binfmt;
	ULONG Offset;

	dprintf("\ttls_array = desc_struct\n");
	GetFieldOffset("thread_struct", "tls_array", &Offset);
	ShowDescStruct(Address + Offset);

	GetFieldValue(Address, "mm_struct", "mmap", mmap);
	GetFieldValue(Address, "mm_struct", "get_unmapped_area", get_unmapped_area);
	GetFieldValue(Address, "mm_struct", "mmap_base", mmap_base);
	GetFieldValue(Address, "mm_struct", "mmap_legacy_base", mmap_legacy_base);
	GetFieldValue(Address, "mm_struct", "mmap_compat_base", mmap_compat_base);
	GetFieldValue(Address, "mm_struct", "mmap_compat_legacy_base", mmap_compat_legacy_base);
	GetFieldValue(Address, "mm_struct", "task_size", task_size);
	GetFieldValue(Address, "mm_struct", "highest_vm_end", highest_vm_end);
	GetFieldValue(Address, "mm_struct", "pgd", pgd);
	GetFieldValue(Address, "mm_struct", "map_count", map_count);
	GetFieldValue(Address, "mm_struct", "hiwater_rss", hiwater_rss);
	GetFieldValue(Address, "mm_struct", "hiwater_vm", hiwater_vm);
	GetFieldValue(Address, "mm_struct", "total_vm", total_vm);
	GetFieldValue(Address, "mm_struct", "locked_vm", locked_vm);
	GetFieldValue(Address, "mm_struct", "pinned_vm", pinned_vm);
	GetFieldValue(Address, "mm_struct", "data_vm", data_vm);
	GetFieldValue(Address, "mm_struct", "exec_vm", exec_vm);
	GetFieldValue(Address, "mm_struct", "stack_vm", stack_vm);
	GetFieldValue(Address, "mm_struct", "def_flags", def_flags);
	GetFieldValue(Address, "mm_struct", "start_code", start_code);
	GetFieldValue(Address, "mm_struct", "end_code", end_code);
	GetFieldValue(Address, "mm_struct", "start_data", start_data);
	GetFieldValue(Address, "mm_struct", "end_data", end_data);
	GetFieldValue(Address, "mm_struct", "start_brk", start_brk);
	GetFieldValue(Address, "mm_struct", "brk", brk);
	GetFieldValue(Address, "mm_struct", "start_stack", start_stack);
	GetFieldValue(Address, "mm_struct", "arg_start", arg_start);
	GetFieldValue(Address, "mm_struct", "arg_end", arg_end);
	GetFieldValue(Address, "mm_struct", "env_start", env_start);
	GetFieldValue(Address, "mm_struct", "env_end", env_end);
	GetFieldValue(Address, "mm_struct", "saved_auxv", saved_auxv);
	GetFieldValue(Address, "mm_struct", "binfmt", binfmt);

	dprintf("\tmmap = 0x%p get_unmapped_area = 0x%p\n", mmap, get_unmapped_area);
	dprintf("\tmm_rb = rb_root mmap_sem = rw_semaphore");

	dprintf("\tmmlist = list_head\n");
	GetFieldOffset("mm_struct", "mmlist", &Offset);
	ShowListHead(Address + Offset);

	dprintf("\tmmap_base = %d mmap_legacy_base = %d\n", mmap_base, mmap_legacy_base);
	dprintf("\tmmap_compat_base = %d mmap_compat_legacy_base = %d\n", mmap_compat_base, mmap_compat_legacy_base);
	dprintf("\ttask_size = %d highest_vm_end = %d\n", task_size, highest_vm_end);
	dprintf("\tpgd = %p map_count = %d\n", pgd, map_count);
	dprintf("\tmm_users = atomic mm_count = atomic\n");
	dprintf("\tpgtables_bytes = atomic_long page_table_lock = spinlock\n");
	dprintf("\thiwater_rss = %d hiwater_vm = %d\n", hiwater_rss, hiwater_vm);
	dprintf("\ttotal_vm = %d locked_vm = %d\n", total_vm, locked_vm);
	dprintf("\tpinned_vm = %d data_vm = %d\n", pinned_vm, data_vm);
	dprintf("\texec_vm = %d stack_vm = %d\n", exec_vm, stack_vm);
	dprintf("\tdef_flags = %d start_code = %d\n", def_flags, start_code);
	dprintf("\tend_code = %d start_data = %d\n", end_code, start_data);
	dprintf("\trss_stat = mm_rss_stat uprobes_state = uprobes_state\n");
	dprintf("\tend_data = %d start_brk = %d\n", end_data, start_brk);
	dprintf("\tbrk = %d start_stack = %d\n", brk, start_stack);
	dprintf("\targ_start = %d arg_end = %d\n", arg_start, arg_end);
	dprintf("\tenv_start = %d env_end = %d\n", env_start, env_end);
	dprintf("\tsaved_auxv = %d binfmt = %d\n", saved_auxv, binfmt);
	dprintf("\tcontext = mm_context membarrier_state = atomic\n");
	dprintf("\tioctx_lock = spinlock membarrier_state = spinlock\n");
	dprintf("\tpmd_huge_pte = pgtable hugetlb_usage = hugetlb_usage\n");

	ULONG64 flags, core_state, owner, user_ns,
		exe_file, mmu_notifier_mm, numa_next_scan, numa_scan_offset,
		numa_scan_seq, tlb_flush_batched, hmm, cpu_bitmap[5];

	GetFieldValue(Address, "mm_struct", "flags", flags);
	GetFieldValue(Address, "mm_struct", "core_state", core_state);
	GetFieldValue(Address, "mm_struct", "owner", owner);
	GetFieldValue(Address, "mm_struct", "user_ns", user_ns);
	GetFieldValue(Address, "mm_struct", "exe_file", exe_file);
	GetFieldValue(Address, "mm_struct", "mmu_notifier_mm", mmu_notifier_mm);
	GetFieldValue(Address, "mm_struct", "numa_next_scan", numa_next_scan);
	GetFieldValue(Address, "mm_struct", "numa_scan_offset", numa_scan_offset);
	GetFieldValue(Address, "mm_struct", "numa_scan_seq", numa_scan_seq);
	GetFieldValue(Address, "mm_struct", "tlb_flush_batched", tlb_flush_batched);
	GetFieldValue(Address, "mm_struct", "hmm", hmm);
	GetFieldValue(Address, "mm_struct", "cpu_bitmap", cpu_bitmap);
	dprintf("\tflags = 0x%p core_state = 0x%p\n", flags, core_state);
	dprintf("\towner = 0x%p user_ns = 0x%p\n", owner, user_ns);
	dprintf("\texe_file = 0x%p mmu_notifier_mm = 0x%p\n", exe_file, mmu_notifier_mm);
	dprintf("\tuprobes_state = uprobes_state async_put_work = work_struct\n");
	dprintf("\tnuma_next_scan = %d numa_scan_offset = %d\n", numa_next_scan, numa_scan_offset);
	dprintf("\tnuma_scan_seq = %d tlb_flush_batched = %d\n", numa_scan_seq, tlb_flush_batched);
	dprintf("\thmm = 0x%p cpu_bitmap = %d\n", hmm, cpu_bitmap);
}
int	GET_BIT(unsigned __int64 x, int bit)/*获取第比特位*/
{
	return ((x & (1 << bit)) >> bit);
}
void ShowVmAreaStruct(ULONG64 Address)//vm_area_struct
{
	ULONG64 vm_start, vm_end, vm_next, vm_prev,
		rb_subtree_gap, vm_mm, vm_flags, rb_subtree_last,
		vm_page_prot, anon_vma, vm_ops, vm_pgoff, vm_file,
		vm_prfile, vm_private_data, vm_region, vm_policy;
	ULONG Offset;
	char r, w, x, sp, s;
	char p, rw, su, a, d, b, g;
	const char *nx = " ", *pwt = " ", *pcd = " ";
	
	GetFieldValue(Address, "vm_area_struct", "vm_start", vm_start);
	GetFieldValue(Address, "vm_area_struct", "vm_end", vm_end);
	GetFieldValue(Address, "vm_area_struct", "vm_page_prot", vm_page_prot);
	GetFieldValue(Address, "vm_area_struct", "vm_flags", vm_flags);
	vm_flags &= ~VM_SOFTDIRTY;
	r = vm_flags & VM_READ ? 'r' : '-';
	w = vm_flags & VM_WRITE ? 'w' : '-';
	x = vm_flags & VM_EXEC ? 'x' : '-';
	sp = vm_flags & VM_MAYSHARE ? 's' : 'p';
	s = vm_flags & VM_SHARED ? 's' : '-';
	//vm_page_prot &= ~vm_flags;
	//dprintf("%x %x %x %x %x %x %x %x %x \n", GET_BIT(vm_page_prot, 8), GET_BIT(vm_page_prot, 7), GET_BIT(vm_page_prot, 6), GET_BIT(vm_page_prot, 5), GET_BIT(vm_page_prot, 4), GET_BIT(vm_page_prot, 3), GET_BIT(vm_page_prot, 2), GET_BIT(vm_page_prot, 1), GET_BIT(vm_page_prot, 0));
	p = GET_BIT(vm_page_prot, 0) ? 'P' : ' ';
	rw = GET_BIT(vm_page_prot, 1) ? 'W' : 'R';
	su = GET_BIT(vm_page_prot, 2) ? 'U' : 'S';
	pwt = GET_BIT(vm_page_prot, 3) ? "PWT" : " ";
	pcd = GET_BIT(vm_page_prot, 4) ? "PCD" : " ";
	a = GET_BIT(vm_page_prot, 5) ? 'A' : ' ';
	d = GET_BIT(vm_page_prot, 6) ? 'D' : ' ';
	b = GET_BIT(vm_page_prot, 7) ? 'B' : ' ';
	g = GET_BIT(vm_page_prot, 8) ? 'G' : ' ';
	nx = vm_page_prot & GET_BIT(vm_page_prot, 63) ? "NX" : " ";

	if (x == 'x') {
		CodeCount = (vm_end - vm_start) + CodeCount;
	}
	if (w == 'w') {
		WriteCount = (vm_end - vm_start) + WriteCount;
	}
	if (s == 's') {
		ShareCount = (vm_end - vm_start) + ShareCount;
	}
	if (p == 'P') {
		WorkCount = (vm_end - vm_start) + WorkCount;
	}

	dprintf("vma@0x%p\n", Address);

	GetFieldValue(Address, "vm_area_struct", "vm_next", vm_next);
	GetFieldValue(Address, "vm_area_struct", "vm_prev", vm_prev);

	dprintf("\t%p-%p %c%c%c%c", vm_start, vm_end, r, w, x, sp);
	GetFieldValue(Address, "vm_area_struct", "vm_file", vm_file);
	if (vm_file != 0x0)
	{
		//ShowMapFilePath(vm_file);//传入指向file结构体的地址
		ShowMapFileName(vm_file);//传入指向file结构体的地址
	}
	else {
		dprintf("\n");
	}
	dprintf("\tvm_next = 0x%p vm_prev = 0x%p\n", vm_next, vm_prev);
	dprintf("\tvm_rb = rb_node\n");
	GetFieldOffset("mm_struct", "vm_rb", &Offset);
	ShowRbNode(Address + Offset);

	GetFieldValue(Address, "vm_area_struct", "rb_subtree_gap", rb_subtree_gap);
	GetFieldValue(Address, "vm_area_struct", ".rb_subtree_last", rb_subtree_last);
	dprintf("\trb_subtree_gap = %d rb_subtree_last = %d\n", rb_subtree_gap, rb_subtree_last);
	GetFieldValue(Address, "vm_area_struct", "vm_mm", vm_mm);
	dprintf("\tvm_mm = 0x%p\n", vm_mm);

	dprintf("\trb = rb_node\n");
	GetFieldOffset("vm_area_struct", ".rb", &Offset);
	ShowRbNode(Address + Offset);

	//dprintf("\tvm_page_prot = pgprot\n");
	dprintf("\tvm_page_prot = 0x%p", vm_page_prot);
	dprintf("\t[%s-%c-%c-%c-%c-%s-%s-%c-%c-%c]\n", nx, g, b, d, a, pcd, pwt, su, rw, p);
	dprintf("\tvm_flags = 0x%p\n", vm_flags);

	dprintf("\trb = list_head ");
	GetFieldOffset("vm_area_struct", " .prio_list", &Offset);
	ShowListHead(Address + Offset);

	GetFieldValue(Address, "vm_area_struct", " .rb_subtree_last", rb_subtree_last);
	dprintf("\trb_subtree_last = %d\n", rb_subtree_last);

	dprintf("\tanon_vma_chain = list_head ");
	GetFieldOffset("vm_area_struct", "anon_vma_chain", &Offset);
	ShowListHead(Address + Offset);

	GetFieldValue(Address, "vm_area_struct", "anon_vma", anon_vma);
	GetFieldValue(Address, "vm_area_struct", "vm_ops", vm_ops);
	GetFieldValue(Address, "vm_area_struct", "vm_pgoff", vm_pgoff);
	GetFieldValue(Address, "vm_area_struct", "vm_prfile", vm_prfile);
	GetFieldValue(Address, "vm_area_struct", "vm_private_data", vm_private_data);
	GetFieldValue(Address, "vm_area_struct", "vm_region", vm_region);
	GetFieldValue(Address, "vm_area_struct", "vm_policy", vm_policy);

	dprintf("\tanon_vma = 0x%p vm_ops = 0x%p\n", anon_vma, vm_ops);

	dprintf("\tvm_pgoff = 0x%p vm_file = 0x%p\n", vm_pgoff, vm_file);
	dprintf("\tvm_prfile = 0x%p vm_private_data = 0x%p\n", vm_prfile, vm_private_data);
	dprintf("\tswap_readahead_info = atomic_long\n");
	dprintf("\tvm_region = 0x%p vm_policy = 0x%p\n", vm_region, vm_policy);
	dprintf("\tvm_userfaultfd_ctx = vm_userfaultfd_ctx\n");
	
}
void ShowMapFileName(ULONG64 Address)//打印被映射文件名
{
	ULONG Offset;
	unsigned char Buffer[MAX_PATH];

	GetFieldOffset("file", "f_path", &Offset);//获取f_path实例名的偏移地址
	Address = Address + Offset;//file结构体基地址+f_path实例名偏移地址=path结构体基地址
	//dprintf("%p\n", Address);
	GetFieldValue(Address, "path", "dentry", Address);//得到dentry实例名所指向的dentry结构体地址
	//dprintf("%p\n", Address);
	GetFieldOffset("dentry", "d_name", &Offset); //获取d_name实例名的偏移地址
	Address = Address + Offset;//dentry结构体基地址+d_name实例名偏移地址=qstr结构体基地址
	//dprintf("%p\n", Address);
	GetFieldValue(Address, "qstr", "name", Address);//获取name字段中地址
	//dprintf("%p\n", Address);
	ReadMemory(Address, &Buffer, sizeof(Buffer), NULL);//获取被映射文件的文件名
	dprintf("\t%s\n", Buffer);//打印被映射文件的文件名
}
void ShowMapFilePath(ULONG64 Address)//打印被映射文件名路径
{
	ULONG Offset;
	unsigned char Buffer[MAX_PATH];

	GetFieldOffset("file", "f_path", &Offset);//获取f_path实例名的偏移地址
	Address = Address + Offset;//file结构体基地址+f_path实例名偏移地址=path结构体基地址
	dprintf("%p\n", Address);
	GetFieldValue(Address, "path", "mnt", Address);//得到mnt实例名所指向的vfsmount结构体地址
	dprintf("%p\n", Address);
	GetFieldValue(Address, "vfsmount", "mnt_root", Address);//得到mnt_root实例名所指向的dentry结构体地址
	dprintf("%p\n", Address);
	GetFieldOffset("dentry", "d_name", &Offset); //获取d_name实例名的偏移地址
	Address = Address + Offset;//dentry结构体基地址+d_name实例名偏移地址=qstr结构体基地址
	dprintf("%p\n", Address);
	GetFieldValue(Address, "qstr", "name", Address);//获取name字段中地址
	dprintf("%p\n", Address);
	ReadMemory(Address, &Buffer, sizeof(Buffer), NULL);//获取被映射文件的文件名路径
	dprintf("\t%Name = s\n", Buffer);//打印被映射文件的文件名路径
}
void ShowRwSemaphore(ULONG64 Address)//rw_semaphore
{
	ULONG64 count, wait_lock, owner;
	ULONG Offset;

	GetFieldValue(Address, "rw_semaphore", "count", count);
	GetFieldValue(Address, "rw_semaphore", "wait_lock", wait_lock);
	dprintf("\tcount = %d wait_lock = %d\n", count, wait_lock);
	dprintf("\tosq = optimistic_spin_queue\n");

	dprintf("\twait_list = list_head\n");
	GetFieldOffset("rw_semaphore", "wait_list", &Offset);
	ShowListHead(Address + Offset);

	GetFieldValue(Address, "rw_semaphore", "owner", owner);
	dprintf("\towner = 0x%p\n", owner);

	dprintf("\tdep_map = lockdep_map\n");
	GetFieldOffset("rw_semaphore", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
void ShowMmRssStat(ULONG64 Address)//mm_rss_stat
{
	ULONG64 count[10];

	GetFieldValue(Address, "rw_semaphore", "count", count);

	dprintf("\tcount = %d\n", count);
}
void ShowLinuxBinfmt(ULONG64 Address)//linux_binfmt
{
	ULONG64 module, load_binary, load_shlib, core_dump, min_coredump;
	ULONG Offset;

	dprintf("\tlh = list_head\n");
	GetFieldOffset("linux_binfmt", "lh", &Offset);
	ShowListHead(Address + Offset);

	GetFieldValue(Address, "linux_binfmt", "module", module);
	GetFieldValue(Address, "linux_binfmt", "load_binary", load_binary);
	GetFieldValue(Address, "linux_binfmt", "load_shlib", load_shlib);
	GetFieldValue(Address, "linux_binfmt", "core_dump", core_dump);
	GetFieldValue(Address, "linux_binfmt", "min_coredump", min_coredump);

	dprintf("\tmodule = 0x%p load_binary = 0x%p\n", module, load_binary);
	dprintf("\tload_shlib = 0x%p core_dump = 0x%p\n", load_shlib, core_dump);
	dprintf("\tmin_coredump = %d\n", min_coredump);
}
void ShowUprobesState(ULONG64 Address)//uprobes_state
{
	ULONG64 xol_area;

	GetFieldValue(Address, "rw_semaphore", "xol_area", xol_area);

	dprintf("\txol_area = 0x%p\n", xol_area);
}
void ShowWorkStruct(ULONG64 Address)//work_struct
{
	ULONG64 data, func;
	ULONG Offset;

	GetFieldValue(Address, "rw_semaphore", "data", data);
	GetFieldValue(Address, "rw_semaphore", "func", func);
	
	dprintf("\tdata = %d func = %d\n", data, func);

	dprintf("\tentry = list_head\n");
	GetFieldOffset("work_struct", "entry", &Offset);
	ShowListHead(Address + Offset);
	dprintf("\tlockdep_map = lockdep_map\n");
	GetFieldOffset("work_struct", "lockdep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
ULONG GetOffset(IN LPCSTR Type, IN LPCSTR Field) {
	ULONG FieldOffset = 0;
	GetFieldOffset(Type, Field, &FieldOffset);
	
	if (FieldOffset == 0) {
		dprintf("	failed to get %s address",Field);
	}
	return FieldOffset;
}
//---------------------------------------------------------------------------------------------------
/*打印该进程中的所有线程信息*/
void ShowAllThreads(ndx_ctx* ndx, uint32_t proc_pid, ULONG64 Address) {

	char* task_cache = NULL;
	ULONG read = 0;
	uint64_t signal_addr = 0;
	uint64_t next_addr = 0;
	uint64_t thread_task = 0;
	uint64_t init_thread_task = 0;
	ULONG offset_wait_sum = 0;
	ULONG offset_sum_exec_runtime = 0;
	ULONG offset_sum_sleep_runtime = 0;

	int on_cpu, cpu = -1;
	int64_t state = 0;
	char comm[16];
	int pid = 0;
	int prio = 0;
	uint64_t wait_sum = 0;
	uint64_t sum_exec_runtime = 0;
	int64_t sum_sleep_runtime = 0;
	uint64_t se = 0;
	uint64_t statistics = 0;
	int tgid = 0;

	offset_wait_sum = ndx->offset_task_se + ndx->offset_task_statistics + ndx->offset_task_wait_sum;
	offset_sum_exec_runtime = ndx->offset_task_se + ndx->offset_task_sum_exec_runtime;
	offset_sum_sleep_runtime = ndx->offset_task_se + ndx->offset_task_statistics + ndx->offset_task_sum_sleep_runtime;

	ReadPointer(Address+ ndx->offset_signal ,&signal_addr);
	ReadPointer(signal_addr+(uint64_t)ndx->offset_thread_head,&next_addr);
	thread_task = next_addr - ndx->offset_thread_node;
	init_thread_task = thread_task;	
	task_cache = (char*)malloc(ndx->size_task_struct);

	dprintf("Thread list of process %d:\n", proc_pid);
	dprintf("*  cpu  tgid  state     pid    prio   wait-time"
		"  sum-exec  sum-sleep  task_struct\n");
	dprintf("--------------------------------------------"
		"------------------------------------------\n");
	while (1) {
		// dprintf("thread task: %p\n",thread_task);
		ReadMemory(thread_task, task_cache, ndx->size_task_struct, &read);
		if (read < ndx->size_task_struct) {
			//dprintf("read task_struct failed %d < %d \n", read, ndx->size_task_struct);
			break;
		}

		/*输出打印数据*/
		state = *(int*)(task_cache + ndx->offset_task_state);
		on_cpu = *(int*)(task_cache + ndx->offset_task_on_cpu);
		cpu = *(int*)(task_cache + ndx->offset_task_cpu);
		pid = *(int*)(task_cache + ndx->offset_task_pid);
		strcpy(comm, (char*)(task_cache + ndx->offset_task_comm));
		prio = *(int*)(task_cache + ndx->offset_task_prio);
		tgid = *(int*)(task_cache + ndx->offset_task_tgid);
		wait_sum = *(int*)(task_cache + ndx->offset_wait_sum);
		sum_exec_runtime = *(int*)(task_cache + ndx->offset_sum_exec_runtime);
		sum_sleep_runtime = *(int*)(task_cache + ndx->offset_sum_sleep_runtime);

		dprintf("%-3d%-5d%-5d %08X  %-7d%-7d%-11d%-10d%-11d%p\n",
			on_cpu, cpu,tgid,state,pid,prio,wait_sum, sum_exec_runtime, sum_sleep_runtime, thread_task);

		// GetFieldValue(next_addr, "list_head", "next", next_addr);
		ReadPointer(next_addr, &next_addr);
		if (next_addr == signal_addr + ndx->offset_thread_head) {
			break;
		}
		thread_task = next_addr - ndx->offset_thread_node;
	}
	dprintf("--------------------------------------------"
		"------------------------------------------\n");
}

/*打印结构体信息结束*/
void ShowMmStruct_All(ULONG64 Address)
{
	ULONG64 mmap;
	ULONG Offset;

	if (GetFieldValue(Address, "mm_struct", ".mmap", mmap))
	{
		dprintf("Error in reading MM_Struct at 0x%p\n", Address);
	}
	else
	{
		dprintf("\n////////////////////////////////////////////////////////////////////////////////\n");
		dprintf("MM_Struct Address = 0x%p\n", Address);

		ShowNorStruct("mm_struct", ".mm_rb", "rb_root");
		GetFieldOffset("mm_struct", ".mm_rb", &Offset);
		ShowRbRoot(Address + Offset);

		Show0nField(Address, "mm_struct", ".vmacache_seqnum");
		Show0xField(Address, "mm_struct", ".stack");
		Show0xField(Address, "mm_struct", ".mmap_base");
		Show0xField(Address, "mm_struct", ".mmap_legacy_base");
		Show0xField(Address, "mm_struct", ".mmap_compat_base");
		Show0xField(Address, "mm_struct", ".mmap_compat_legacy_base");
		Show0nField(Address, "mm_struct", ".task_size");
		Show0xField(Address, "mm_struct", ".highest_vm_end");
		Show0xField(Address, "mm_struct", ".pgd");
		Show0nField(Address, "mm_struct", ".mm_users");
		Show0nField(Address, "mm_struct", ".mm_count");
		Show0nField(Address, "mm_struct", ".pgtables_bytes");
		Show0nField(Address, "mm_struct", ".page_table_lock");

		ShowNorStruct("mm_struct", ".mmap_sem", "rw_semaphore");
		GetFieldOffset("mm_struct", ".mmap_sem", &Offset);
		ShowRwSemaphore(Address + Offset);
		ShowNorStruct("mm_struct", ".mmlist", "list_head");
		GetFieldOffset("mm_struct", ".mmlist", &Offset);
		ShowListHead(Address + Offset);

		Show0nField(Address, "mm_struct", ".hiwater_rss");
		Show0nField(Address, "mm_struct", ".hiwater_vm");
		Show0nField(Address, "mm_struct", ".total_vm");
		Show0nField(Address, "mm_struct", ".pinned_vm");
		Show0nField(Address, "mm_struct", ".data_vm");
		Show0nField(Address, "mm_struct", ".exec_vm");
		Show0nField(Address, "mm_struct", ".stack_vm");
		Show0nField(Address, "mm_struct", ".def_flags");
		Show0nField(Address, "mm_struct", ".arg_lock");
		Show0xField(Address, "mm_struct", ".start_code");
		Show0xField(Address, "mm_struct", ".end_code");
		Show0xField(Address, "mm_struct", ".start_data");
		Show0xField(Address, "mm_struct", ".end_data");
		Show0xField(Address, "mm_struct", ".start_brk");
		Show0xField(Address, "mm_struct", ".brk");
		Show0xField(Address, "mm_struct", ".start_stack");
		Show0xField(Address, "mm_struct", ".arg_start");
		Show0xField(Address, "mm_struct", ".arg_end");
		Show0xField(Address, "mm_struct", ".env_start");
		Show0xField(Address, "mm_struct", ".env_end");
		//ShowArrayField(Address, "mm_struct", ".saved_auxv");

		ShowNorStruct("mm_struct", ".rss_stat", "mm_rss_stat");
		GetFieldOffset("mm_struct", ".rss_stat", &Offset);
		ShowMmRssStat(Address + Offset);
		ShowPtr64Struct(Address, "mm_struct", ".binfmt", "linux_binfmt");
		GetFieldOffset("mm_struct", ".binfmt", &Offset);
		ShowLinuxBinfmt(Address + Offset);

		//Show0nField(Address, "mm_struct", ".context");
		Show0xField(Address, "mm_struct", ".flags");
		Show0xField(Address, "mm_struct", ".membarrier_state");
		Show0xField(Address, "mm_struct", ".ioctx_lock");

		ShowPtr64Struct(Address, "mm_struct", ".core_state", "core_state");
		ShowPtr64Struct(Address, "mm_struct", ".ioctx_table", "kioctx_table");
		ShowPtr64Struct(Address, "mm_struct", ".owner", "task_struct");
		ShowPtr64Struct(Address, "mm_struct", ".user_ns", "user_namespace");
		ShowPtr64Struct(Address, "mm_struct", ".exe_file", "file");
		ShowPtr64Struct(Address, "mm_struct", ".mmu_notifier_mm", "mmu_notifier_mm");

		ShowNorStruct("mm_struct", ".uprobes_state", "uprobes_state");
		GetFieldOffset("mm_struct", ".uprobes_state", &Offset);
		ShowUprobesState(Address + Offset);
		ShowNorStruct("mm_struct", ".async_put_work", "work_struct");
		GetFieldOffset("mm_struct", ".async_put_work", &Offset);
		ShowWorkStruct(Address + Offset);

		Show0nField(Address, "mm_struct", ".pmd_huge_pte");
		Show0nField(Address, "mm_struct", ".numa_next_scan");
		Show0nField(Address, "mm_struct", ".numa_scan_offset");
		Show0nField(Address, "mm_struct", ".numa_scan_seq");
		Show0nField(Address, "mm_struct", ".tlb_flush_batched");
		Show0nField(Address, "mm_struct", ".hugetlb_usage");

		//ShowPtr64Struct(Address, "mm_struct", ".hmm", "hmm");

		//ShowArrayField(Address, "mm_struct", "cpu_bitmap");

		/*ULONG64 VmaAddress;
		char Parameter[MAX_PATH];

		GetFieldValue(Address, "mm_struct", ".mmap", Address);
		dprintf(
			"If you want to output the specified virtual memory area, please enter the address.\n"
			"No input : Output all virtual memory areas.\n");

		GetInputLine(NULL, Parameter, sizeof(Parameter));
		scanf_s(Parameter, "%llx", &VmaAddress);
		dprintf("%p\n", VmaAddress);
		while (Address != 0x00000000)
		{

			if (Address == VmaAddress) {
				dprintf("--------------------------------------------------------------\n");
				ShowVmAreaStruct(Address);
			}
			if (VmaAddress == 0xcccccccccccccccc) {
				dprintf("--------------------------------------------------------------\n");
				ShowVmAreaStruct(Address);
			}

			GetFieldValue(Address, "vm_area_struct", "vm_next", Address);
		}

		dprintf("--------------------------------------------------------------\n");
		dprintf("Summary(in bytes) : Code = 0x%llx ; Working Set = 0x%llx ; Writetable data = 0x%llx ; Shared = 0x%llx\n",
			CodeCount, WorkCount, WriteCount, ShareCount);*/
		dprintf("////////////////////////////////////////////////////////////////////////////////\n");
	}

	return;
}
int ShowTaskStruct(ndx_ctx* ndx, ULONG64 Address, uint32_t pid_filter, const char * comm_filter, uint32_t flags)
{
	ULONG64 pid;
	ULONG Offset;
	char* comm;
	char* task_cache = NULL;
	ULONG read = 0;
	int ret = 0;
	if (GetFieldValue(Address, "task_struct", "pid", pid)) {
		dprintf("Error in reading task_struct at %p\n", Address);
		return -1;
	}
	if (pid_filter != 0 && pid != pid_filter)
		return 0;

	task_cache = (char*)malloc(ndx->size_task_struct);

	ReadMemory(Address, task_cache, ndx->size_task_struct, &read);
	if (read < ndx->size_task_struct) {
		dprintf("read task_struct failed %d < %d \n", read, ndx->size_task_struct);
		return -1;
	}
	comm = task_cache + ndx->offset_task_comm;
	if (comm_filter != NULL && comm_filter[0] != 0 && strcmp(comm_filter, comm) != 0)
		goto TAG_FREE;

	dprintf("*** task pid: %d task_struct: 0x%p %s\n", pid, Address, comm);	

	if(flags & NDX_FLAG_BASIC){
		Show0nTaskField(task_cache, "task_struct", "tpid");
		Show0nTaskField(task_cache, "task_struct", "state");
		Show0nTaskField(task_cache, "task_struct", "usage");
		Show0xTaskField(task_cache, "task_struct", "flags");
		Show0nTaskField(task_cache, "task_struct", "stack_canary");
	}
	if (flags & NDX_FLAG_PROCESS_ATTRIBUTE) {
		Show0nTaskField(task_cache, "task_struct", "ptrace");
		Show0nTaskField(task_cache, "task_struct", "exit_state");
		Show0nTaskField(task_cache, "task_struct", "exit_code");
		Show0nTaskField(task_cache, "task_struct", "exit_signal");
		ShowPtr64TaskStruct(task_cache, "task_struct", "mm", "mm_struct");
		GetFieldOffset("task_struct", "mm", &Offset);
		ShowMmStruct(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "active_mm", "mm_struct");
		GetFieldOffset("task_struct", "active_mm", &Offset);
		ShowMmStruct(Address + Offset);
		ShowNorStruct("task_struct", "vmacache", "vmacache");
		GetFieldOffset("task_struct", "vmacache", &Offset);
		ShowVmacache(Address + Offset);
		ShowNorStruct("task_struct", "rss_stat", "task_rss_stat");
		GetFieldOffset("task_struct", "rss_stat", &Offset);
		ShowTaskRssStat(Address + Offset);
	}
	if (flags & NDX_FLAG_THREAD_ATTRIBUTE) {
		Show0xTaskField(task_cache, "task_struct", "stack");
		Show0nTaskField(task_cache, "task_struct", "on_cpu");
		Show0nTaskField(task_cache, "task_struct", "cpu");
		Show0nTaskField(task_cache, "task_struct", "wakee_flips");
		Show0nTaskField(task_cache, "task_struct", "wakee_flip_decay_ts");
		ShowNorStruct("task_struct", "thread_info", "thread_info");
		GetFieldOffset("task_struct", "thread_info", &Offset);
		ShowThreadInfo_X86(Address + Offset);
		ShowNorStruct("task_struct", "wake_entry", "llist_node");
		GetFieldOffset("task_struct", "wake_entry", &Offset);
		ShowLlistNode(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "last_wakee", "task_struct");
		ShowNorStruct("task_struct", "thread", "thread_struct");
		GetFieldOffset("task_struct", "thread_struct", &Offset);
		ShowThreadStruct_X86(Address + Offset);
	}
	if (flags & NDX_FLAG_THREAD_LIST) {
		ShowAllThreads(ndx, pid, Address);
	}

	if (flags & NDX_FLAG_SCHEDULE_ATTRINUTE) {
		Show0nTaskField(task_cache, "task_struct", "recent_used_cpu");
		Show0nTaskField(task_cache, "task_struct", "wake_cpu");
		Show0nTaskField(task_cache, "task_struct", "on_rp");
		Show0nTaskField(task_cache, "task_struct", "prio");
		Show0nTaskField(task_cache, "task_struct", "static_prio");
		Show0nTaskField(task_cache, "task_struct", "normal_prio");
		Show0nTaskField(task_cache, "task_struct", "rt_priority");
		ShowPtr64TaskStruct(task_cache, "task_struct", "sched_class", "sched_class");
		ShowNorStruct("task_struct", "se", "sched_entity");
		GetFieldOffset("task_struct", "se", &Offset);
		ShowSchedEntity(Address + Offset);
		ShowNorStruct("task_struct", "rt", "sched_rt_entity");
		GetFieldOffset("task_struct", "rt", &Offset);
		ShowSchedRtEntity(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "sched_task_group", "task_group");
		ShowNorStruct("task_struct", "dl", "sched_dl_entity");
		GetFieldOffset("task_struct", "dl", &Offset);
		ShowSchedDlEntity(Address + Offset);
		ShowNorStruct("task_struct", "preempt_notifiers", "hlist_head");
		GetFieldOffset("task_struct", "preempt_notifiers", &Offset);
		ShowHlistHead(Address + Offset);
		Show0nTaskField(task_cache, "task_struct", "btrace_seq");
		Show0nTaskField(task_cache, "task_struct", "policy");
		Show0nTaskField(task_cache, "task_struct", "nr_cpus_allowed");
		ShowNorStruct("task_struct", "cpus_allowed", "cpumask");
		ShowNorStruct("task_struct", "sched_info", "sched_info");
		GetFieldOffset("task_struct", "sched_info", &Offset);
		ShowSchedInfo(Address + Offset);
		Show0yTaskField(task_cache, "task_struct", "sched_reset_on_fork");
		Show0yTaskField(task_cache, "task_struct", "sched_contributes_to_load");
		Show0yTaskField(task_cache, "task_struct", "sched_migrated");
		Show0yTaskField(task_cache, "task_struct", "sched_remote_wakeup");
		Show0yTaskField(task_cache, "task_struct", "sched_psi_wake_requeue");
		Show0nTaskField(task_cache, "task_struct", "last_switch_count");
		Show0nTaskField(task_cache, "task_struct", "last_switch_time");
	}
	if (flags & NDX_FLAG_RCU_ATTRINUTE) {
		Show0nTaskField(task_cache, "task_struct", "rcu_tasks_nvcsw");
		Show0nTaskField(task_cache, "task_struct", "rcu_tasks_holdout");
		Show0nTaskField(task_cache, "task_struct", "rcu_tasks_idx");
		Show0nTaskField(task_cache, "task_struct", "rcu_tasks_idle_cpu");

		ShowNorStruct("task_struct", "rcu_tasks_holdout_list", "list_head");
		GetFieldOffset("task_struct", "rcu_tasks_holdout_list", &Offset);
		ShowListHead(Address + Offset);
		ShowNorStruct("task_struct", "rcu", "callback_head");
		GetFieldOffset("task_struct", "rcu", &Offset);
		ShowCallbackHead(Address + Offset);
	}
	if (flags & NDX_FLAG_RELATION_ATTRINUTE) {
		ShowNorStruct("task_struct", "tasks", "list_head");
		GetFieldOffset("task_struct", "tasks", &Offset);
		ShowListHead(Address + Offset);
		ShowNorStruct("task_struct", "pushable_tasks", "plist_node");
		GetFieldOffset("task_struct", "pushable_tasks", &Offset);
		ShowPushableTasks(Address + Offset);
		ShowNorStruct("task_struct", "pushable_dl_tasks", "rb_node");
		GetFieldOffset("task_struct", "pushable_dl_tasks", &Offset);
		ShowRbNode(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "real_parent", "task_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "parent", "task_struct");
		ShowNorStruct("task_struct", "children", "list_head");
		GetFieldOffset("task_struct", "children", &Offset);
		ShowListHead(Address + Offset);
		ShowNorStruct("task_struct", "sibling", "list_head");
		GetFieldOffset("task_struct", "sibling", &Offset);
		ShowListHead(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "group_leader", "task_struct");
	}
	if (flags & NDX_FLAG_PTRACE_ATTRINUTE) {
		ShowNorStruct("task_struct", "ptraced", "list_head");
		GetFieldOffset("task_struct", "ptraced", &Offset);
		ShowListHead(Address + Offset);
		ShowNorStruct("task_struct", "ptrace_entry", "list_head");
		GetFieldOffset("task_struct", "ptrace_entry", &Offset);
		ShowListHead(Address + Offset);
		Show0nTaskField(task_cache, "task_struct", "ptrace_message");
		Show0nTaskField(task_cache, "task_struct", "trace_overrun");
		Show0nTaskField(task_cache, "task_struct", "tracing_graph_pause");
		Show0nTaskField(task_cache, "task_struct", "trace");
		Show0nTaskField(task_cache, "task_struct", "trace_recursion");
		ShowPtr64ArrayStruct(Address, "perf_event_ctxp");
		ShowNorStruct("task_struct", "perf_event_mutex", "mutex");
		GetFieldOffset("task_struct", "perf_event_mutex", &Offset);
		ShowMutex(Address + Offset);
		ShowNorStruct("task_struct", "perf_event_list", "list_head");
		GetFieldOffset("task_struct", "perf_event_list", &Offset);
		ShowListHead(Address + Offset);
		Show0nTaskField(task_cache, "task_struct", "curr_ret_stack");
		Show0nTaskField(task_cache, "task_struct", "curr_ret_depth");
		ShowPtr64TaskStruct(task_cache, "task_struct", "utask", "uprobe_task");
	}
	if (flags & NDX_FLAG_TIME_ATTRINUTE) {
		Show0nTaskField(task_cache, "task_struct", "utime");
		Show0xTaskField(task_cache, "task_struct", "stime");
		Show0nTaskField(task_cache, "task_struct", "gtime");

		ShowNorStruct("task_struct", "prev_cputime", "prev_cputime");
		GetFieldOffset("task_struct", "prev_cputime", &Offset);
		ShowPrevCpuTime(Address + Offset);

		Show0nTaskField(task_cache, "task_struct", "nvcsw");
		Show0nTaskField(task_cache, "task_struct", "nivcsw");
		Show0nTaskField(task_cache, "task_struct", "start_time");
		Show0nTaskField(task_cache, "task_struct", "real_start_time");
		Show0nTaskField(task_cache, "task_struct", "min_flt");
		Show0nTaskField(task_cache, "task_struct", "maj_flt");

		ShowNorStruct("task_struct", "cputime_expires", "task_cputime");
		GetFieldOffset("task_struct", "cputime_expires", &Offset);
		ShowTaskCpuTime(Address + Offset);

		ShowNorStruct("task_struct", "cpu_timers", "list_head");
		GetFieldOffset("task_struct", "cpu_timers", &Offset);
		ShowListHead(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "delays", "task_delay_info");

		Show0xTaskField(task_cache, "task_struct", "timer_slack_ns");
		Show0nTaskField(task_cache, "task_struct", "default_timer_slack_ns");
	}
	if (flags & NDX_FLAG_SIGNAL_ATTRINUTE) {
		ShowPtr64TaskStruct(task_cache, "task_struct", "signal", "signal_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "sighand", "sighand_struct");

		Show0nTaskField(task_cache, "task_struct", "blocked");
		Show0nTaskField(task_cache, "task_struct", "real_blocked");
		Show0nTaskField(task_cache, "task_struct", "saved_sigmask");

		ShowNorStruct("task_struct", "pending", "sigpending");
		GetFieldOffset("task_struct", "pending", &Offset);
		ShowSigpending(Address + Offset);

		Show0nTaskField(task_cache, "task_struct", "pdeath_signal");
		ShowNorStruct("task_struct", "last_siginfo", "kernel_siginfo");//kernel_siginfo结构体内无信息
		ShowPtr64TaskStruct(task_cache, "task_struct", "rseq", "rseq");

		Show0nTaskField(task_cache, "task_struct", "rseq_len");
		Show0nTaskField(task_cache, "task_struct", "rseq_sig");
		Show0nTaskField(task_cache, "task_struct", "rseq_event_mask");
	}
	if(flags & NDX_FLAG_THREAD_GROUP_ATTRINUTE){
		ShowPtr64TaskStruct(task_cache, "task_struct", "thread_pid", "pid");
		ShowNorStruct("task_struct", "pid_links", "hlist_node");
		GetFieldOffset("task_struct", "pid_links", &Offset);
		ShowHlistNode(Address + Offset);
		ShowNorStruct("task_struct", "thread_group", "list_head");
		GetFieldOffset("task_struct", "thread_group", &Offset);
		ShowListHead(Address + Offset);
		ShowNorStruct("task_struct", "thread_node", "list_head");
		GetFieldOffset("task_struct", "thread_node", &Offset);
		ShowListHead(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "vfork_done", "completion");

		Show0xTaskField(task_cache, "task_struct", "set_child_tid");
		Show0xTaskField(task_cache, "task_struct", "clear_child_tid");

		ShowPtr64TaskStruct(task_cache, "task_struct", "ptracer_cred", "cred");
		ShowPtr64TaskStruct(task_cache, "task_struct", "real_cred", "cred");
		ShowPtr64TaskStruct(task_cache, "task_struct", "cred", "cred");
	}
	if(flags & NDX_FLAG_FS_ATTRINUTE) {
		ShowPtr64TaskStruct(task_cache, "task_struct", "fs", "fs_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "files", "files_struct");
		ShowNorStruct("task_struct", "ioac", "task_io_accounting");
		GetFieldOffset("task_struct", "ioac", &Offset);
		ShowTaskIoAccounting(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "nsproxy", "nsproxy");
		Show0nTaskField(task_cache, "task_struct", "sas_ss_sp");
		Show0nTaskField(task_cache, "task_struct", "sas_ss_size");
		Show0nTaskField(task_cache, "task_struct", "sas_ss_flags");
		Show0xTaskField(task_cache, "task_struct", "journal_info");
		ShowPtr64TaskStruct(task_cache, "task_struct", "nameidata", "nameidata");

		ShowPtr64TaskStruct(task_cache, "task_struct", "bio_list", "bio_list");
		ShowPtr64TaskStruct(task_cache, "task_struct", "io_context", "io_context");
		ShowPtr64TaskStruct(task_cache, "task_struct", "plug", "blk_plug");
		ShowPtr64TaskStruct(task_cache, "task_struct", "reclaim_state", "reclaim_state");
		ShowPtr64TaskStruct(task_cache, "task_struct", "backing_dev_info", "backing_dev_info");
		Show0nTaskField(task_cache, "task_struct", "sequential_io");
		Show0nTaskField(task_cache, "task_struct", "sequential_io_avg");
		ShowPtr64TaskStruct(task_cache, "task_struct", "splice_pipe", "pipe_inode_info");
		ShowPtr64TaskStruct(task_cache, "task_struct", "throttle_queue", "request_queue");
	}
	if(flags & NDX_FLAG_TASK_WORK_ATTRINUTE) {
		ShowPtr64TaskStruct(task_cache, "task_struct", "task_works", "callback_head");
		GetFieldOffset("task_struct", "task_works", &Offset);
		ShowCallbackHead(Address + Offset);
	}
	if (flags & NDX_FLAG_USER_ATTRINUTE) {
		ShowPtr64TaskStruct(task_cache, "task_struct", "audit_context", "audit_context");

		Show0nTaskField(task_cache, "task_struct", "loginuid");
		Show0xTaskField(task_cache, "task_struct", "sessionid");

		ShowNorStruct("task_struct", "seccomp", "seccomp");
		GetFieldOffset("task_struct", "seccomp", &Offset);
		ShowSeccomp(Address + Offset);

		Show0nTaskField(task_cache, "task_struct", "parent_exec_id");
		Show0nTaskField(task_cache, "task_struct", "self_exec_id");
		ShowPtr64TaskStruct(task_cache, "task_struct", "cgroups", "css_set");
		ShowNorStruct("task_struct", "cg_list", "list_head");
		GetFieldOffset("task_struct", "cg_list", &Offset);
		ShowListHead(Address + Offset);
		Show0nTaskField(task_cache, "task_struct", "jobctl");
		Show0nTaskField(task_cache, "task_struct", "personality");
		Show0nTaskField(task_cache, "task_struct", "patch_state");
		Show0xTaskField(task_cache, "task_struct", "security");
	}
	if (flags & NDX_FLAG_LOCK_ATTRINUTE) {
		Show0yTaskField(task_cache, "task_struct", "in_execve");
		Show0yTaskField(task_cache, "task_struct", "in_iowait");
		Show0yTaskField(task_cache, "task_struct", "restore_sigmask");
		Show0yTaskField(task_cache, "task_struct", "in_user_fault");
		Show0yTaskField(task_cache, "task_struct", "no_cgroup_migration");
		Show0yTaskField(task_cache, "task_struct", "use_memdelay");
		Show0nTaskField(task_cache, "task_struct", "atomic_flags");

		ShowNorStruct("task_struct", "alloc_lock", "spinlock");

		ShowNorStruct("task_struct", "pi_lock", "raw_spinlock");
		GetFieldOffset("task_struct", "pi_lock", &Offset);
		ShowPiLock(Address + Offset);
		ShowNorStruct("task_struct", "wake_q", "wake_q_node");
		GetFieldOffset("task_struct", "wake_q", &Offset);
		ShowWakeQNode(Address + Offset);
		ShowNorStruct("task_struct", "pi_waiters", "rb_root_cached");
		GetFieldOffset("task_struct", "pi_waiters", &Offset);
		ShowRbRootCached(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "pi_top_task", "task_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "pi_blocked_on", "rt_mutex_waiter");
		ShowNorStruct("task_struct", "restart_block", "restart_block");
		ShowNorStruct("task_struct", "pi_state_list", "list_head");
		GetFieldOffset("task_struct", "pi_state_list", &Offset);
		ShowListHead(Address + Offset);
		ShowPtr64TaskStruct(task_cache, "task_struct", "pi_state_cache", "futex_pi_state");
		ShowPtr64TaskStruct(task_cache, "task_struct", "robust_list", "robust_list_head");
		ShowPtr64TaskStruct(task_cache, "task_struct", "compat_robust_list", "compat_robust_list_head");
		ShowNorStruct("task_struct", "sysvsem", "sysv_sem");
		GetFieldOffset("task_struct", "sysvsem", &Offset);
		ShowSysvSem(Address + Offset);
	}
	if (flags & NDX_FLAG_X86_ATTRINUTE) {
		Show0nTaskField(task_cache, "task_struct", "closid"); // rdt
		Show0nTaskField(task_cache, "task_struct", "rmid");
	}
	if (flags & NDX_FLAG_MM_ATTRINUTE) {
		Show0nTaskField(task_cache, "task_struct", "psi_flags");
		Show0nTaskField(task_cache, "task_struct", "acct_rss_mem1");
		Show0nTaskField(task_cache, "task_struct", "acct_vm_mem1");
		Show0nTaskField(task_cache, "task_struct", "acct_timexpd");

		ShowNorStruct("task_struct", "mems_allowed", "nodemask");//无nodemask/nodemask_t结构体
		ShowNorStruct("task_struct", "mems_allowed_seq", "seqcount");
		GetFieldOffset("task_struct", "mems_allowed_seq", &Offset);
		ShowSeqcount(Address + Offset);

		Show0nTaskField(task_cache, "task_struct", "cpuset_mem_spread_rotor");
		Show0nTaskField(task_cache, "task_struct", "cpuset_slab_spread_rotor");

		ShowPtr64TaskStruct(task_cache, "task_struct", "oom_reaper_list", "task_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "stack_vm_area", "vm_struct");
		ShowPtr64TaskStruct(task_cache, "task_struct", "memcg_in_oom", "mem_cgroup");//

		Show0nTaskField(task_cache, "task_struct", "memcg_oom_gfp_mask");
		Show0nTaskField(task_cache, "task_struct", "memcg_oom_order");
		Show0nTaskField(task_cache, "task_struct", "memcg_nr_pages_over_high");

		ShowPtr64TaskStruct(task_cache, "task_struct", "active_memcg", "mem_cgroup");
		Show0nTaskField(task_cache, "task_struct", "pagefault_disabled");
		Show0nTaskField(task_cache, "task_struct", "stack_refcount");
		Show0nTaskField(task_cache, "task_struct", "nr_dirtied");
		Show0nTaskField(task_cache, "task_struct", "nr_dirtied_pause");
		Show0nTaskField(task_cache, "task_struct", "dirty_paused_when");
		ShowNorStruct("task_struct", "task_frag", "page_frag");
		GetFieldOffset("task_struct", "task_frag", &Offset);
		ShowPageFrag(Address + Offset);
		ShowNorStruct("task_struct", "tlb_ubc", "tlbflush_unmap_batch");
		GetFieldOffset("task_struct", "tlb_ubc", &Offset);
		ShowTlbflushUnmapBatch(Address + Offset);
		ShowNorStruct("task_struct", "sysvshm", "sysv_shm");
		GetFieldOffset("task_struct", "sysvshm", &Offset);
		ShowSysvShm(Address + Offset);
	}
	if(flags & NDX_FLAG_NUMA_ATTRINUTE){
		Show0nTaskField(task_cache, "task_struct", "numa_scan_seq");
		Show0xTaskField(task_cache, "task_struct", "numa_scan_period");
		Show0nTaskField(task_cache, "task_struct", "numa_scan_period_max");
		Show0nTaskField(task_cache, "task_struct", "numa_preferred_nid");
		Show0nTaskField(task_cache, "task_struct", "numa_migrate_retry");
		Show0nTaskField(task_cache, "task_struct", "node_stamp");
		Show0nTaskField(task_cache, "task_struct", "last_task_numa_placement");
		Show0nTaskField(task_cache, "task_struct", "last_sum_exec_runtime");

		ShowNorStruct("task_struct", "numa_work", "callback_head");
		GetFieldOffset("task_struct", "numa_work", &Offset);
		ShowCallbackHead(Address + Offset);

		ShowPtr64TaskStruct(task_cache, "task_struct", "numa_group", "numa_group");

		Show0xTaskField(task_cache, "task_struct", "numa_faults");
		Show0nTaskField(task_cache, "task_struct", "total_numa_faults");
		ShowArrayField(Address, "task_struct", "numa_faults_locality");
		Show0nTaskField(task_cache, "task_struct", "numa_pages_migrated");
		Show0nTaskField(task_cache, "task_struct", "il_prev");
		Show0nTaskField(task_cache, "task_struct", "pref_node_fork");
		ShowPtr64TaskStruct(task_cache, "task_struct", "mempolicy", "mempolicy");
	}
TAG_FREE:
	free(task_cache);

	return 0;
}

/////////////////////
void ReadTaskStructSimple(ULONG64 Address,nd_task_struct &simpletask){
	GetFieldValue(Address, "task_struct", "pid" ,simpletask.pid);
	GetFieldValue(Address, "task_struct", "ppid", simpletask.ppid);
	GetFieldValue(Address, "task_struct", "state",simpletask.state);
	GetFieldValue(Address, "task_struct", "flags",simpletask.flags);
}
void ShowTaskStructSimple(nd_task_struct simpletask) {
	dprintf("pid:%d   ppid:%d\n",simpletask.pid,simpletask.ppid);
	dprintf("state:%llx  flags:%llx\n",simpletask.state,simpletask.flags);
}
void get_task_struct_simple(ULONG64 Address)
{
	nd_task_struct simpletask;
	dprintf("------------------\n");
	ReadTaskStructSimple(Address,simpletask);
	//ShowTaskStructSimple(simpletask);
}

/*
* examine the process list or one process in Linux
* this command follows the syntax of the !process command of WinDBG
* !process [/s Session] [/m Module] [Process [Flags]]
* !process [/s Session] [/m Module] 0 Flags ImageName
* or in new style
* !process [-s Session] [-m Module] [-f flags] [-p pid] [-P process_addr] [-i ImageName]
* #define DECLARE_API64(s)                           \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG64                dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )
*/
//int NdxPickNextField(const char* cursor, char* buffer, size_t buf_len, size_t& eaten_length)
//{
//	int len;
//	const char* start = cursor;
//	while (*start == ' ') start++;
//	const char* sep = strchr(start, ' ');
//	len = (sep != NULL) ? (sep - start) : strlen(start);
//	if (len < buf_len) {
//		strncpy(buffer, start, len);
//		buffer[len] = 0;
//		eaten_length = start - cursor + len;
//		return len;
//	}
//	return 0;
//}

void NdxLxpUsage()
{
	dprintf("!lxp to show linux process list or a specified process\n."
		" Syntax: !process [-f flags] [-p pid] [-P task_struct Address] [-i ImageName]\n"
		"  0: using 0 as pid for all processes.\n"
		" display flags:\n"
		"  Bit0: basic information about the process.\n"
		"  Bit1: the detailed information of the process,\n"
		"  Bit2: the thread list of the process.\n"
		"  Bit3: thread attributes of the task_struct.\n"
		"  Bit4: scheudle attributes.\n"
		"  Bit5: rcu attributes.\n"
		"  Bit6: process relation fields.\n"
		"  Bit7: ptrace attributes.\n"
		"  Bit8: time attributes.\n"
		"  Bit9: signal fields.\n"
		"  Bit10: memory management fields.\n"
		"  Bit11: NUMA attributes.\n"
		"  Bit12: file system fields.\n"
		"  Bit13: thread group fields.\n"
		"  Bit14: task works.\n"
		"  Bit15: user, audits and security related fileds.\n"
		"  Bit16: wait and locks.\n"
		"  Bit17: x86 specific fields, such as RDT.\n");
}

DECLARE_API(lxp)
{
	ULONG64  Address, init_addr, next;
	char option[256], comm_filter[100] = {0};
	ULONG  Offset;
	int num = 0, argc = 1, len;
	const char* cursor = args;
	uint32_t flags = 0, pid = 0;
	uint64_t desired_task_addr = 0;
	size_t eaten_length = 0;


	do {
		switch(cursor[1]) {
		case 'f':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				flags = strtoul(option, NULL, 0);
			}
			else {
				dprintf("missing display flags, using default\n");
			}
			break;
		case 'p':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				pid = strtoul(option, NULL, 0);
			}
			else {
				dprintf("missing PID to show, using default\n");
			}
			break;
		case 'P':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				desired_task_addr = strtoull(option, NULL, 0);
			}
			else {
				dprintf("missing task_struct address to show, using default\n");
			}
			break;
		case 'i':
			cursor += 2;
			len = NdxPickNextField(cursor, comm_filter, sizeof(comm_filter), &eaten_length);
			if (len <= 0) {
				dprintf("missing command name to show, using default\n");
			}
			break;
		case 'h':
		case 'H':
		case '?':
			NdxLxpUsage();
			return;
		default:
			// old syntax
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (argc == 1) {
				if (strlen(option) > 7)
					desired_task_addr = strtoull(option, NULL, 0);
				else
					pid = atoi(option);
				argc++;
			}
			else if (argc == 2) {
				flags = strtoul(option, NULL, 0);
				argc++;
			}
			else if (argc == 3) {
				if (strlen(option) < sizeof(comm_filter))
					strcpy(comm_filter, option);
			}
			else {
				dprintf("too many arguments, ignored\n");
			}

			break;
		}
		if (len > 0) {
			cursor += eaten_length;
			while (*cursor == ' ') cursor++;
		}
	} while (len > 0 && *cursor != 0);

	if (!GetExpressionEx("lk!init_task", &init_addr, NULL)) {
		dprintf("failed to get init_task\n");
		return;
	}
	if (desired_task_addr == 0) {
		dprintf("init_tast address:%llx\n", init_addr);
		Address = init_addr;
	}
	GetFieldOffset("task_struct", "tasks", &Offset);

	NdxGetOffsets(&ndx);
	do {
		ShowTaskStruct(&ndx, Address, pid, comm_filter, flags);
		if (desired_task_addr != 0) {
			break;
		}
		GetFieldValue(Address, "task_struct", "tasks.next", next);
		Address = next - Offset;//通用；["task_struct", "tasks.next"]和["list_head", "next"]。
	} while (Address != init_addr);
}