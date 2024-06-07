#pragma once

#define KDEXT_64BIT

#include <windows.h>
#include <dbgexts.h>
#include <wdbgexts.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
void Show0nField(ULONG64 Address, const char *name)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0n%d\n", Offset, name, value);
}/*输出0n格式的字段（0n--》Int4B / Int8B）*/
extern "C"
void Show0xField(ULONG64 Address, const char *name)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0x%p\n", Offset, name, value);
}/*输出0x格式的字段（0x--》Ptr64 / 部分UInt8B）*/
extern "C"
void Show00Field(const char *name)
{
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);

	dprintf("+0x%03lx %s =  \n", Offset, name);
}/*输出空的字段（空--》无输出）*/
extern "C"
void ShowNorField(ULONG64 Address, const char *name)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = %d\n", Offset, name, value);
}/*输出正常格式的字段（Nor--》Uint4B / Uint8B，正常输出无前缀）*/
void ShowCharField(ULONG64 Address, const char *name)
{
	char value[50];
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = %s\n", Offset, name, value);
}/*输出字符格式的字段（字符--》UChar）*/
extern "C"
void Show0yField(ULONG64 Address, const char *name)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0y%04lx\n", Offset, name, value);
}/*输出0y格式的字段(0y-->pos X 4Bit)*/
extern "C"
void ShowChField(ULONG64 Address, const char *name)
{
	ULONG64 value[15];
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0y%04lx\n", Offset, name, value);
}
/*普通字段部分结束*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
void ShowNorStruct(const char *name1, const char *name2)
{
	ULONG Offset;

	GetFieldOffset("task_struct", name1, &Offset);

	dprintf("+0x%03lx %s = %s\n", Offset, name1, name2);
}/*输出普通类型的字段（字段名--》字段结构体类型名）*/
extern "C"
void ShowPtr64Struct(ULONG64 Address, const char *name1, const char *name2)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name1, &Offset);
	GetFieldValue(Address, "task_struct", name1, value);

	dprintf("+0x%03lx %s = 0x%p %s\n", Offset, name1, value, name2);
}/*输出带地址及字段结构体类型名的字段*/
extern "C"
void ShowPtr6400Struct(ULONG64 Address, const char *name)
{
	ULONG64 value;
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0x%p\n", Offset, name, value);
}/*输出带地址的字段*/
extern "C"
void ShowPtr64ChStruct(ULONG64 Address, const char *name)
{
	ULONG64 value[3];
	ULONG Offset;

	GetFieldOffset("task_struct", name, &Offset);
	GetFieldValue(Address, "task_struct", name, value);

	dprintf("+0x%03lx %s = 0x%p\n", Offset, name, value);
}
/*结构体式的字段部分结束*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
void ShowLlistNode(ULONG64 Address)//llist_node
{
	ULONG64 next;

	GetFieldValue(Address, "llist_node", "next", next);

	dprintf("       next = 0x%p\n", next);
}
extern "C"
void ShowHlistHead(ULONG64 Address)//hlist_head
{
	ULONG64 first;

	GetFieldValue(Address, "hlist_head", "first", first);

	dprintf("       first = 0x%p\n", first);
}
extern "C"
void ShowListHead(ULONG64 Address)//list_head
{
	ULONG64 next, prev;

	GetFieldValue(Address, "list_head", "next", next);
	GetFieldValue(Address, "list_head", "prev", prev);

	dprintf("       next = 0x%p prev = 0x%p\n", next, prev);
}
extern "C"
void ShowSchedInfo(ULONG64 Address)//sched_info
{
	ULONG64 pcount, run_delay, last_arrival, last_queued;

	GetFieldValue(Address, "sched_info", "pcount", pcount);
	GetFieldValue(Address, "sched_info", "run_delay", run_delay);
	GetFieldValue(Address, "sched_info", "last_arrival", last_arrival);
	GetFieldValue(Address, "sched_info", "last_queued", last_queued);

	dprintf("       pcount       = %d run_delay   = %d\n", pcount, run_delay);
	dprintf("       last_arrival = %d last_queued = %d\n", last_arrival, run_delay);
}
extern "C"
void ShowPushableTasks(ULONG64 Address)//plist_node
{
	ULONG64 prio;
	ULONG Offset;

	GetFieldValue(Address, "sched_info", "prio", prio);

	dprintf("       prio = 0n%d\n", prio);

	dprintf("       prio_list = list_head\n");
	GetFieldOffset("plist_node", "prio_list", &Offset);
	ShowListHead(Address + Offset);
	dprintf("       node_list = list_head\n");
	GetFieldOffset("plist_node", "node_list", &Offset);
	ShowListHead(Address + Offset);
}
extern "C"
void ShowRbNode(ULONG64 Address)//rb_node
{
	ULONG64 __rb_parent_color, rb_right, rb_left;

	GetFieldValue(Address, "rb_node", "__rb_parent_color", __rb_parent_color);
	GetFieldValue(Address, "rb_node", "rb_right", rb_right);
	GetFieldValue(Address, "rb_node", "rb_left", rb_left);

	dprintf("       __rb_parent_color = 0n%d\n", __rb_parent_color);
	dprintf("       rb_right = 0x%p rb_left = 0x%p\n", rb_right, rb_left);
}
extern "C"
void ShowVmacache(ULONG64 Address)//vmacache
{
	ULONG64 seqnum, vmas[10];

	GetFieldValue(Address, "vmacache", "seqnum", seqnum);
	GetFieldValue(Address, "vmacache", "vmas", vmas);

	dprintf("       seqnum   = %d\n", seqnum);
	dprintf("       rb_right = 0x%p\n", vmas);
}
extern "C"
void ShowTaskRssStat(ULONG64 Address)//task_rss_stat
{
	ULONG64 event, count[10];

	GetFieldValue(Address, "task_rss_stat", "event", event);
	GetFieldValue(Address, "task_rss_stat", "count", count);

	dprintf("       event = 0n%d count = 0n%d\n", event, count);
}
extern "C"
void ShowHlistNode(ULONG64 Address)//hlist_node
{
	ULONG64 next, pprev;

	GetFieldValue(Address, "hlist_node", "next", next);
	GetFieldValue(Address, "hlist_node", "pprev", pprev);

	dprintf("       next = 0x%p pprev = 0x%p\n", next, pprev);
}
extern "C"
void ShowPrevCpuTime(ULONG64 Address)//prev_cputime
{
	ULONG64 utime, stime;

	GetFieldValue(Address, "prev_cputime", "utime", utime);
	GetFieldValue(Address, "prev_cputime", "stime", stime);

	dprintf("       utime = %d stime = %d\n", utime, stime);
	dprintf("       lock  = raw_spinlock\n");
}
extern "C"
void ShowTaskCpuTime(ULONG64 Address)//task_cputime
{
	ULONG64 utime, stime, sum_exec_runtime;

	GetFieldValue(Address, "task_cputime", "utime", utime);
	GetFieldValue(Address, "task_cputime", "stime", stime);
	GetFieldValue(Address, "task_cputime", "sum_exec_runtime", sum_exec_runtime);

	dprintf("       utime = %d stime = %d\n", utime, stime);
	dprintf("       sum_exec_runtime = %d\n", sum_exec_runtime);
}
extern "C"
void ShowSysvSem(ULONG64 Address)//sysv_sem
{
	ULONG64 undo_list;

	GetFieldValue(Address, "sysv_sem", "undo_list", undo_list);

	dprintf("       undo_list = 0x%p\n", undo_list);
}
extern "C"
void ShowSysvShm(ULONG64 Address)//sysv_shm
{
	ULONG Offset;

	dprintf("       shm_clist = list_head\n");
	GetFieldOffset("sysv_shm", "shm_clist", &Offset);
	ShowListHead(Address + Offset);
}
extern "C"
void ShowSigpending(ULONG64 Address)//sigpending
{
	ULONG Offset;

	dprintf("       list = list_head\n");
	GetFieldOffset("sigpending", "list", &Offset);
	ShowListHead(Address + Offset);

	dprintf("       signal  = sigset\n");
}
extern "C"
void ShowSeccomp(ULONG64 Address)//seccomp
{
	ULONG64 mode, filter;

	GetFieldValue(Address, "seccomp", "mode", mode);
	GetFieldValue(Address, "seccomp", "filter", filter);

	dprintf("       mode = 0n%d stime = %d\n", mode, filter);
}
extern "C"
void ShowLockdepMap(ULONG64 Address)//lockdep_map
{
	char name[50];
	ULONG64 key, cpu, ip, class_cache[10];

	GetFieldValue(Address, "lockdep_map", "key", key);
	GetFieldValue(Address, "lockdep_map", "class_cache", class_cache);
	GetFieldValue(Address, "lockdep_map", "name", name);
	GetFieldValue(Address, "lockdep_map", "cpu", cpu);
	GetFieldValue(Address, "lockdep_map", "ip", ip);

	dprintf("       key = 0x%p\n", key);
	dprintf("       class_cache  = 0x%p\n", class_cache);
	dprintf("       name = %s\n", name);
	dprintf("       cpu  = 0n%d ip = %d\n", cpu, ip);
}
extern "C"
void ShowPiLock(ULONG64 Address)//raw_spinlock
{
	ULONG64 magic, owner_cpu, owner;
	ULONG Offset;

	GetFieldValue(Address, "raw_spinlock", "magic", magic);
	GetFieldValue(Address, "raw_spinlock", "owner_cpu", owner_cpu);
	GetFieldValue(Address, "raw_spinlock", "owner", owner);

	dprintf("       raw_lock = arch_spinlock\n");
	dprintf("       magic = %d owner_cpu = %d\n", magic, owner_cpu);
	dprintf("       owner = 0x%p\n", owner);

	dprintf("       dep_map = lockdep_map\n");
	GetFieldOffset("raw_spinlock", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
extern "C"
void ShowWakeQNode(ULONG64 Address)//wake_q_node
{
	ULONG64 next;

	GetFieldValue(Address, "wake_q_node", "next", next);

	dprintf("       next = 0x%p\n", next);
}
extern "C"
void ShowRbRoot(ULONG64 Address)//rb_root
{
	ULONG64 rb_node;

	GetFieldValue(Address, "rb_root", "rb_node", rb_node);

	dprintf("       rb_node = 0x%p\n", rb_node);
}
extern "C"
void ShowRbRootCached(ULONG64 Address)//rb_root_cached
{
	ULONG64 rb_leftmost;
	ULONG Offset;

	GetFieldValue(Address, "wake_q_node", "rb_leftmost", rb_leftmost);

	dprintf("       rb_root = rb_root\n");
	GetFieldOffset("rb_root_cached", "rb_root", &Offset);
	ShowRbRoot(Address + Offset);

	dprintf("       rb_leftmost = 0x%p\n", rb_leftmost);
}
extern "C"
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

	dprintf("       rchar = %d wchar = %d\n", rchar, wchar);
	dprintf("       syscr = %d syscw = %d\n", syscr, syscw);
	dprintf("       read_bytes = %d write_bytes = %d\n", read_bytes, write_bytes);
	dprintf("       cancelled_write_bytes = %d\n", cancelled_write_bytes);
}
extern "C"
void ShowSeqcount(ULONG64 Address)//seqcount
{
	ULONG64 sequence;
	ULONG Offset;

	GetFieldValue(Address, "seqcount", "sequence", sequence);

	dprintf("       sequence = %d\n", sequence);

	dprintf("       dep_map = lockdep_map\n");
	GetFieldOffset("seqcount", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
extern "C"
void ShowOptimisticSpinQueue(ULONG64 Address)//optimistic_spin_queue
{
	dprintf("       tail = atomic\n");
}
extern "C"
void ShowMutex(ULONG64 Address)//mutex
{
	ULONG64 magic;
	ULONG Offset;

	GetFieldValue(Address, "mutex", "magic", magic);

	dprintf("       owner = atomic_long\n");
	dprintf("       wait_lock = spinlock\n");
	dprintf("       magic = %d\n", magic);

	dprintf("       osq = optimistic_spin_queue\n");
	GetFieldOffset("mutex", "osq", &Offset);
	ShowOptimisticSpinQueue(Address + Offset);
	dprintf("       wait_list = list_head\n");
	GetFieldOffset("mutex", "wait_list", &Offset);
	ShowListHead(Address + Offset);
	dprintf("       dep_map = lockdep_map\n");
	GetFieldOffset("mutex", "dep_map", &Offset);
	ShowLockdepMap(Address + Offset);
}
extern "C"
void ShowCallbackHead(ULONG64 Address)//callback_head
{
	ULONG64 next, func;

	GetFieldValue(Address, "callback_head", "next", next);
	GetFieldValue(Address, "callback_head", "func", func);

	dprintf("       next = 0x%p func = 0x%p\n", next, func);
}
extern "C"
void ShowArchTlbflushUnmapBatch(ULONG64 Address)//arch_tlbflush_unmap_batch
{
	dprintf("       cpumask = cpumask\n");
}
extern "C"
void ShowTlbflushUnmapBatch(ULONG64 Address)//tlbflush_unmap_batch
{
	bool flush_required, writable;
	const char *bool_fr = "ture", *bool_w = "ture";
	ULONG Offset;

	GetFieldValue(Address, "tlbflush_unmap_batch", "flush_required", flush_required);
	GetFieldValue(Address, "tlbflush_unmap_batch", "writable", writable);
	if (flush_required == 1)
	{
		bool_fr = "ture";
	}
	else
	{
		bool_fr = "false";
	}
	if (writable == 1)
	{
		bool_w = "ture";
	}
	else
	{
		bool_w = "false";
	}

	dprintf("       flush_required = %s writable = %s\n", bool_fr, bool_w);

	dprintf("       arch = arch_tlbflush_unmap_batch\n");
	GetFieldOffset("tlbflush_unmap_batch", "arch", &Offset);
	ShowArchTlbflushUnmapBatch(Address + Offset);
}
extern "C"
void ShowPageFrag(ULONG64 Address)//page_frag
{
	ULONG64 page, offset, size;

	GetFieldValue(Address, "page_frag", "page", page);
	GetFieldValue(Address, "page_frag", "offset", offset);
	GetFieldValue(Address, "page_frag", "size", size);

	dprintf("       page = 0x%p\n", page);
	dprintf("       offset = %d size = %d\n", offset, size);
}
extern "C"
void ShowLoadWeight(ULONG64 Address)//load_weight
{
	ULONG64 weight, inv_weight;

	GetFieldValue(Address, "load_weight", "weight", weight);
	GetFieldValue(Address, "load_weight", "inv_weight", inv_weight);

	dprintf("       weight = %d inv_weight = %d\n", weight, inv_weight);
}
extern "C"
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

	dprintf("       wait_start = %d wait_max = %d\n", wait_start, wait_max);
	dprintf("       wait_count = %d wait_sum = %d\n", wait_count, wait_sum);
	dprintf("       iowait_count = %d iowait_sum = %d\n", iowait_count, iowait_sum);
	dprintf("       sleep_start = %d sleep_max = %d\n", sleep_start, sleep_max);
	dprintf("       sum_sleep_runtime = %d block_start = %d\n", sum_sleep_runtime, block_start);
	dprintf("       block_max = %d exec_max = %d\n", block_max, exec_max);
	dprintf("       slice_max = %d nr_migrations_cold = %d\n", slice_max, nr_migrations_cold);
	dprintf("       nr_failed_migrations_affine = %d nr_failed_migrations_running = %d\n", nr_failed_migrations_affine, nr_failed_migrations_running);
	dprintf("       nr_failed_migrations_hot = %d nr_forced_migrations = %d\n", nr_failed_migrations_hot, nr_forced_migrations);
	dprintf("       nr_wakeups = %d inv_weight = %d\n", nr_wakeups, nr_wakeups_sync);
	dprintf("       nr_wakeups_migrate = %d nr_wakeups_local = %d\n", nr_wakeups_migrate, nr_wakeups_local);
	dprintf("       nr_wakeups_remote = %d nr_wakeups_affine = %d\n", nr_wakeups_remote, nr_wakeups_affine);
	dprintf("       nr_wakeups_affine_attempts = %d nr_wakeups_passive = %d\n", nr_wakeups_affine_attempts, nr_wakeups_passive);
	dprintf("       nr_wakeups_idle = %dn", nr_wakeups_idle);
}
extern "C"
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

	dprintf("       runnable_weight  = %d on_rq = %d\n", runnable_weight, on_rq);
	dprintf("       exec_start  = %d sum_exec_runtime = %d\n", exec_start, sum_exec_runtime);
	dprintf("       vruntime  = %d prev_sum_exec_runtime = %d\n", vruntime, prev_sum_exec_runtime);
	dprintf("       nr_migrations  = %d depth = 0n%d\n", nr_migrations, depth);
	dprintf("       parent = 0x%p\n", parent);
	dprintf("       cfs_rq = 0x%p\n", cfs_rq);
	dprintf("       my_q = 0x%p\n", my_q);

	dprintf("       run_node = rb_node\n");
	GetFieldOffset("sched_entity", "run_node", &Offset);
	ShowRbNode(Address + Offset);
	dprintf("       group_node = list_head\n");
	GetFieldOffset("sched_entity", "group_node", &Offset);
	ShowListHead(Address + Offset);
	dprintf("       load = load_weight\n", my_q);
	GetFieldOffset("sched_entity", "load", &Offset);
	ShowLoadWeight(Address + Offset);
	dprintf("       statistics = sched_statistics\n");
	GetFieldOffset("sched_entity", "statistics", &Offset);
	ShowSchedStatistics(Address + Offset);
}
extern "C"
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

	dprintf("       timeout  = %d watchdog_stamp = %d\n", timeout, watchdog_stamp);
	dprintf("       time_slice  = %d on_rq = %d\n", time_slice, on_rq);
	dprintf("       on_list  = %d\n", on_list);
	dprintf("       back = 0x%p\n", back);
	dprintf("       parent = 0x%p\n", parent);
	dprintf("       rt_rq = 0x%p\n", rt_rq);
	dprintf("       my_q = 0x%p\n", my_q);

	dprintf("       run_list = list_head\n");
	GetFieldOffset("sched_rt_entity", "run_list", &Offset);
	ShowListHead(Address + Offset);
}
extern "C"
void ShowTimerqueueNode(ULONG64 Address)//timerqueue_node
{
	ULONG64 next;
	ULONG Offset;

	dprintf("       node = rb_node\n");
	GetFieldOffset("timerqueue_node", "node", &Offset);
	ShowRbNode(Address + Offset);

	GetFieldValue(Address, "load_weight", "timerqueue_node", next);

	dprintf("       next = %p\n", next);
}
extern "C"
void ShowHrtimer(ULONG64 Address)//hrtimer
{
	ULONG64 state, is_rel, is_soft, function, base;
	ULONG Offset;

	dprintf("       node = timerqueue_node\n");
	GetFieldOffset("hrtimer", "node", &Offset);
	ShowTimerqueueNode(Address + Offset);

	GetFieldValue(Address, "load_weight", "state", state);
	GetFieldValue(Address, "load_weight", "is_rel", is_rel);
	GetFieldValue(Address, "load_weight", "is_soft", is_soft);
	GetFieldValue(Address, "load_weight", "function", function);
	GetFieldValue(Address, "load_weight", "base", base);

	dprintf("       state = %d is_rel = %d\n", state, is_rel);
	dprintf("       is_soft = %d\n", is_soft);
	dprintf("       function = 0x%p\n", function);
	dprintf("       base = 0x%p\n", base);
}
extern "C"
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

	dprintf("       dl_runtime  = %d dl_deadline = %d\n", dl_runtime, dl_deadline);
	dprintf("       dl_period  = %d dl_bw = %d\n", dl_period, dl_bw);
	dprintf("       dl_density  = %d runtime = %d\n", dl_density, runtime);
	dprintf("       deadline  = %d flags = 0x%p\n", deadline, flags);
	dprintf("       dl_throttled  = 0y%04d dl_boosted = 0y%04d\n", deadline, dl_boosted);
	dprintf("       dl_yielded  = 0y%04d dl_non_contending = 0y%04d\n", dl_yielded, dl_non_contending);
	dprintf("       dl_overrun  = 0y%04d\n", dl_overrun);

	dprintf("       rb_node = rb_node\n");
	GetFieldOffset("sched_dl_entity", "rb_node", &Offset);
	ShowRbNode(Address + Offset);
	dprintf("       dl_timer = hrtimer\n");
	GetFieldOffset("sched_dl_entity", "dl_timer", &Offset);
	ShowHrtimer(Address + Offset);
	dprintf("       inactive_timer = hrtimer\n");
	GetFieldOffset("sched_dl_entity", "inactive_timer", &Offset);
	ShowHrtimer(Address + Offset);
}
extern "C"
void ShowThreadInfo_X86(ULONG64 Address)//thread_info
{
	ULONG64 flags, status;

	GetFieldValue(Address, "thread_info", "flags", flags);
	GetFieldValue(Address, "thread_info", "status", status);

	dprintf("       flags = 0n%d status = %d\n", flags, status);
}
extern "C"
void ShowDescStruct(ULONG64 Address)//desc_struct
{
	ULONG64 limit0, base0, base1, limit1;

	GetFieldValue(Address, "desc_struct", "limit0", limit0);
	GetFieldValue(Address, "desc_struct", "base0", base0);
	GetFieldValue(Address, "desc_struct", "base1", base1);
	GetFieldValue(Address, "desc_struct", "limit1", limit1);

	dprintf("       limit0 = %d base0 = %d\n", limit0, base0);
	dprintf("       base1 = %d limit1 = %d\n", base1, limit1);
}
extern "C"
void ShowFpregsState(ULONG64 Address)//fpregs_state
{
	dprintf("       fsave = fregs_state\n");
	dprintf("       fxsave = fxregs_state\n");
	dprintf("       soft = swregs_state\n");
	dprintf("       xsave = xregs_state\n");
}
extern "C"
void ShowFpu(ULONG64 Address)//fpu
{
	ULONG64 last_cpu;
	UCHAR initialized;
	ULONG Offset;

	GetFieldValue(Address, "desc_struct", "last_cpu", last_cpu);
	GetFieldValue(Address, "desc_struct", "initialized", initialized);

	dprintf("       last_cpu = 0n%d initialized = %s\n", last_cpu, initialized);

	dprintf("       state = fpregs_state\n");
	GetFieldOffset("thread_struct", "state", &Offset);
	ShowFpregsState(Address + Offset);
}
extern "C"
void ShowThreadStruct_X86(ULONG64 Address)//thread_struct
{
	ULONG64 sp, es, ds, fsindex,
		gsindex, fsbase, gsbase, ptrace_bps[5],
		ptrace_dr7, cr2, trap_nr, error_code,
		io_bitmap_ptr, iopl, io_bitmap_max, addr_limit,
		sig_on_uaccess_err, uaccess_err;
	ULONG Offset;

	dprintf("       tls_array = desc_struct\n");
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

	dprintf("       sp = 0n%d es = %d\n", sp, es);
	dprintf("       ds = %d fsindex = %d\n", ds, fsindex);
	dprintf("       gsindex = %d fsbase = 0n%d\n", gsindex, fsbase);
	dprintf("       gsbase = 0n%d ptrace_bps = 0x%p\n", gsbase, ptrace_bps);
	dprintf("       ptrace_dr7 = 0n%d cr2 = 0n%d\n", ptrace_dr7, cr2);
	dprintf("       trap_nr = 0n%d error_code = 0n%d\n", trap_nr, error_code);
	dprintf("       io_bitmap_ptr = 0x%p iopl = 0n%d\n", io_bitmap_ptr, iopl);
	dprintf("       io_bitmap_max = %d addr_limit = 0n%d\n", io_bitmap_max, addr_limit);
	dprintf("       sig_on_uaccess_err = 0y%04d uaccess_err = 0y%04d\n", sig_on_uaccess_err, uaccess_err);

	dprintf("       fpu = fpu\n");
	GetFieldOffset("thread_struct", "fpu", &Offset);
	ShowFpu(Address + Offset);
}