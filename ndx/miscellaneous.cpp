/*
DECLARE_API(tsaddr)
{
	ULONG64  Address;
	ULONG  Offset;
	int num = 0;

	GetExpressionEx("init_task", &Address, &args);
	GetFieldOffset("task_struct", "tasks", &Offset);

	while (num < 5)
	{
		ShowTaskStructAddress(Address);

		//Address = Address + Offset;//list
		//GetFieldValue(Address, "list_head", "next", Address);
		GetFieldValue(Address, "task_struct", "tasks.next", Address);
		Address = Address - Offset;//tasks&&list

		num = num + 1;

	}

	return;
}

	char Parameter[MAX_PATH];
	GetInputLine(
		"Please enter the address:\n"
		, Parameter, sizeof(Parameter));
	sscanf_s(Parameter, "%llx", &Address);

ShowArrayField(Address, "mm_struct", "cpu_bitmap");
ShowPtr64Struct(Address, "mm_struct", " .mmap", "vm_area_struct");
ShowVmAreaStruct(Address + Offset);

void ShowTaskStructAddress(ULONG64 Address)
{
	ULONG64  pid;
	char comm[300];

	GetFieldValue(Address, "task_struct", "pid", pid);
	GetFieldValue(Address, "task_struct", "comm", comm);

	dprintf("pid = %d, comm = %s\n", pid, &comm);
	dprintf("Tasks_Struct Address = %p\n", Address);

	return;
}

HRESULT XmLoader::ReadNameForFileObject(uint64_t ul64FileObject, char* pszName, int nMaxSize)
{
	HRESULT hr = 0;
	uint64_t ulDentry, ulDname;
	LINUX_QSTR* qstrName;
	ndp_os_sym_t* pSym = &m_pKdBoss->GetSysPara()->os_syms_;
	DWORD dwRead;

	/*
		struct file {
			union {
				struct llist_node fu_llist;
				struct rcu_head  fu_rcuhead;
			} f_u;
			struct path  f_path;
	*/
	/*
		struct path {
			struct vfsmount *mnt;
			struct dentry *dentry;
		} __randomize_layout;
	
ulDentry = m_pKdBoss->ReadPointer(ul64FileObject + pSym->file_path + pSym->path_dentry);
/*
 struct dentry {

	struct qstr d_name;

	unsigned char d_iname[DNAME_INLINE_LEN]; // small names

this->EnsureScratchBuffer(pSym->dentry_size);
hr = m_pNdRemote->ReadTarget(NTS_VIRTUAL_MEMORY, ulDentry, pSym->dentry_size, 1, this->m_pbScratchBuffer, &dwRead);
if (hr != S_OK || dwRead != pSym->dentry_size)
{
	return E_FAIL;
}
qstrName = (LINUX_QSTR*)(m_pbScratchBuffer + pSym->dentry_d_name);
/*
	struct qstr {
		union {
			struct {
				HASH_LEN_DECLARE;
			};
			u64 hash_len;
		};
		const unsigned char *name;
	};

// read the qstr now
if (qstrName->len + 1 > nMaxSize)
return E_NOT_SUFFICIENT_BUFFER;
hr = m_pNdRemote->ReadTarget(NTS_VIRTUAL_MEMORY, (uint64_t)qstrName->name, qstrName->len + 1, 1, (PBYTE)pszName, &dwRead);

return hr;
}
void ShowMapFileName(ULONG64 Address)//打印被映射文件名
{
	ULONG Offset;
	ULONG64 test;
	if (GetFieldValue(Address, "file", "f_flags", test))
	{
		dprintf("file address error\n");
	}
	else
	{
		dprintf("file address = %p %p\n", Address, test);
	}
	GetFieldOffset("file", "f_path", &Offset);//获取f_path实例名的偏移地址
	Address = Address + Offset;//file结构体基地址+f_path实例名偏移地址=path结构体基地址
	if (GetFieldValue(Address, "path", "mnt", test))
	{
		dprintf("path address error\n");
	}
	else
	{
		dprintf("path address = %p %p\n", Address, test);
	}
	GetFieldValue(Address, "path", "dentry", Address);//得到dentry实例名所指向的dentry结构体地址
	if (GetFieldValue(Address, "dentry", "d_flags", test))
	{
		dprintf("dentry address error\n");
	}
	else
	{
		dprintf("dentry address = %p %p\n", Address, test);
	}
	GetFieldOffset("dentry", "d_name", &Offset); //获取d_name实例名的偏移地址
	Address = Address + Offset;//dentry结构体基地址+d_name实例名偏移地址=qstr结构体基地址
	if (GetFieldValue(Address, "qstr", ".hash_len", test))
	{
		dprintf("qstr address error\n");
	}
	else
	{
		dprintf("qstr address = %p\n", Address);
	}
	ShowCharField(Address, "qstr", "name");
}
void ShowMmStruct_All(ULONG64 Address)
{
	ULONG64 mmap;
	ULONG Offset;

	if (GetFieldValue(Address, "mm_struct", ".mmap", mmap))
	{
		dprintf("Error in reading MM_Struct at %p\n", Address);
	}
	else
	{
		dprintf("////////////////////////////////////////////////////////////////////////////////\n");
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

		ShowPtr64Struct(Address, "mm_struct", ".hmm", "hmm");

		ShowArrayField(Address, "mm_struct", "cpu_bitmap");

		int num = 0;
		GetFieldValue(Address, "mm_struct", ".mmap", Address);
		while (num < 5)
		{
			dprintf("--------------------------------------------------------------\n");
			ShowVmAreaStruct(Address);

			GetFieldValue(Address, "vm_area_struct", "vm_next", Address);

			num = num + 1;
		}
	}

	return;
}
void ShowIdtData(ULONG64 Address)//idt_data
{
	ULONG Offset;
	ULONG64 SymOffset, Addr;
	char Buffer[MAX_PATH];

	Show0xField(Address, "idt_data", "vector");
	Show0xField(Address, "idt_data", "segment");
	dprintf("\n");

	ShowNorStruct("idt_data", "bits", "idt_bits");
	GetFieldOffset("idt_data", "bits", &Offset);
	ShowIdtBits(Address + Offset);

	Show0xField(Address, "idt_data", "addr");

	GetSymbol(Address, Buffer, &SymOffset);
	dprintf("%s\n", Buffer);
	dprintf("--------------------------------------------------------------\n");
}
	Show0yField(Address, "idt_bits", "ist");
	Show0yField(Address, "idt_bits", "zero");
	Show0yField(Address, "idt_bits", "type");
	dprintf("\n");
	Show0yField(Address, "idt_bits", "dpl");
	Show0yField(Address, "idt_bits", "p");
	dprintf("\n");

		GetSymbol(SymOffset, Buffer, &SymOffset);
	if (num == 0) {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(SymOffset, Test, &SymOffset);
		num = num + 1;
		return num;
	}
	if (strcmp(Buffer, Test) == 0) {//判断函数是否重复
		return num;
	}
	else {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(Address, Test, &SymOffset);
		num = num + 1;
		return num;
	}

		GetSymbol(SymOffset, Buffer, &SymOffset);
	if (num == 0) {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(SymOffset, Test, &SymOffset);
		num = num + 1;
		return num;
	}
	if (strcmp(Buffer, Test) == 0) {//判断函数是否重复
		return num;
	}
	else {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(Address, Test, &SymOffset);
		num = num + 1;
		return num;
	}

	--------------------------
		GetSymbol(SymOffset, Buffer, &SymOffset);
	if (num == 0) {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(SymOffset, Test, &SymOffset);
		num = num + 1;
		return num;
	}
	if (strcmp(Buffer, Test) == 0) {//判断函数是否重复
		return num;
	}
	else {
		dprintf("%03d", num);
		GetFieldOffset("gate_struct", "bits", &Offset);
		ShowIdtBits(Address + Offset);
		dprintf(": %p", Address);
		dprintf(" --> %s\n", Buffer);

		GetSymbol(Address, Test, &SymOffset);
		num = num + 1;
		return num;
	}

	dprintf("%llx %llx %llx %d\n", ArrayLen, ArrayOffset, ArrayCount, ArrayCount);

			GetExpressionEx("apic_idts", &Address, &args);
		dprintf("--------------------------------------------------------------\n");
		dprintf("Apic_Idts First_Address = %p\n", Address);
		ArrayLen = GetTypeSize("apic_idts");
		ArrayOffset = GetTypeSize("idt_data");
		ArrayCount = ArrayLen / ArrayOffset;
		ShowIdtData(Address, (int)ArrayCount);

		GetExpressionEx("def_idts", &Address, &args);
		dprintf("--------------------------------------------------------------\n");
		dprintf("Def_Idts First_Address = %p\n", Address);
		ArrayLen = GetTypeSize("def_idts");
		ArrayOffset = GetTypeSize("idt_data");
		ArrayCount = ArrayLen / ArrayOffset;
		ShowIdtData(Address, (int)ArrayCount);

		GetExpressionEx("ist_idts", &Address, &args);
		dprintf("--------------------------------------------------------------\n");
		dprintf("Ist_Idts First_Address = %p\n", Address);
		ArrayLen = GetTypeSize("ist_idts");
		ArrayOffset = GetTypeSize("idt_data");
		ArrayCount = ArrayLen / ArrayOffset;
		ShowIdtData(Address, (int)ArrayCount);

		GetExpressionEx("dbg_idts", &Address, &args);
		dprintf("--------------------------------------------------------------\n");
		dprintf("Dbg_Idts First_Address = %p\n", Address);
		ArrayLen = GetTypeSize("dbg_idts");
		ArrayOffset = GetTypeSize("idt_data");
		ArrayCount = ArrayLen / ArrayOffset;
		ShowIdtData(Address, (int)ArrayCount);

DECLARE_API(slab)
{
	ULONG64 Address, Address_Fnext;
	ULONG Offset;
	int num;

	num = 0;
	//GetExpressionEx("slab_caches", &Address, &args);
	GetExpressionEx("kmem_cache", &Address, &args);
	GetFieldOffset("kmem_cache", "list", &Offset);

	while (num < 10) {
		dprintf("%p\n", Address);
		//ShowKmemCache(Address);

		//GetFieldValue(Address, "slab_caches", "next", Address);
		Address = Address + Offset;
		GetFieldValue(Address, "list_head", "next", Address);
		Address = Address - Offset;

		num = num + 1;
	}
}

	if (GetFieldValue(Address, "kmem_cache", "kmem_cache.object_size", object_size)) {
		dprintf("wula");
	}

	dprintf("%-30s %-8s %-8s %-8s %-8s %-8s %-8s\n", "name", "object_size", "size", "align", "useroffset", "usersize", "refcount");

		/*while (mchunk_prev_size != 0) {
		dprintf("\tNext Chunk : \n");
		ShowMallocChunk(chunk_address);
		mchunk_prev_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_prev_size");
	}
		ULONG64 ArrayLen, ArrayOffset, ArrayCount;
	ArrayOffset = GetTypeSize("mchunkptr");
	dprintf("%llx\n", ArrayOffset);
	bins_arrary_num

	dprintf("0x30\t\t    5\t\t0x150\n");
	dprintf("0x410\t\t    1\t\t0x410\n");
	dprintf("0x120\t\t    1\t\t0x120\n");

	dprintf("0x30\t\t    5\t\t0x150\t      71.4%%\n");
	dprintf("0x410\t\t    1\t\t0x410\t      14.3%%\n");
	dprintf("0x120\t\t    1\t\t0x120\t      14.3%%\n");
 */