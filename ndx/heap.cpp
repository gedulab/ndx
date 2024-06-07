#include "ndx.h"

ULONG64 main_arnea_addr = 0;

void ShowMainArena(ULONG64 arena_address)//main arena
{
	ULONG64 mutex, flags, top, last_remainder,
		next, next_free, have_fastchunks, attached_threads,
		system_mem, max_system_mem, fastbinsY, bins,
		binmap, ArrayOffset, busychunk_address, mchunk_size;
	char fastchunks, noncontiuous;
	char *chunk_size = 0;

	ArrayOffset = GetTypeSize("mchunkptr");
	int bins_arrary_num = 0;

	mutex = ReadDigitField(arena_address, "malloc_state", "mutex");
	flags = ReadDigitField(arena_address, "malloc_state", "flags");
	top = ReadDigitField(arena_address, "malloc_state", "top");
	last_remainder = ReadDigitField(arena_address, "malloc_state", "last_remainder");
	next = ReadDigitField(arena_address, "malloc_state", "next");
	next_free = ReadDigitField(arena_address, "malloc_state", "next_free");
	have_fastchunks = ReadDigitField(arena_address, "malloc_state", "have_fastchunks");
	attached_threads = ReadDigitField(arena_address, "malloc_state", "attached_threads");
	system_mem = ReadDigitField(arena_address, "malloc_state", "system_mem");
	max_system_mem = ReadDigitField(arena_address, "malloc_state", "max_system_mem");
	fastbinsY = ReadDigitArrayField(arena_address, "malloc_state", "fastbinsY");
	bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
	binmap = ReadDigitArrayField(arena_address, "malloc_state", "binmap");
	fastchunks = flags & 1U ? '-' : 'F';
	noncontiuous = flags & 2U ? 'N' : 'C';

	dprintf("main_arena: 0x%p\n", arena_address);
	dprintf("mutex : %d have_fastchunks : %d binmap : %d attached_threads : %d system_mem : %d max_system_mem : %d\n",
		mutex, have_fastchunks, binmap, attached_threads, system_mem, max_system_mem);
	dprintf("flags : 0x%llx [%c %c] last_remainder : 0x%llx fastbinsY : 0x%llx\n",
		flags, fastchunks, noncontiuous, last_remainder, fastbinsY);
	dprintf("malloc_state   next : 0x%p next_free : 0x%p\n", next, next_free);
	dprintf("malloc_chunk   top  : 0x%p\n", top);
	ShowMallocChunk(top);
	mchunk_size = ShowMchunkSize(top);
	dprintf("malloc_chunk   bins : 0x%p\n", bins);
	ShowMallocChunk(bins);

	busychunk_address = top - (system_mem - mchunk_size);
	mchunk_size = ShowMchunkSize(busychunk_address);
	busychunk_address = busychunk_address + mchunk_size;
	dprintf("Busy Chunk\n");
	dprintf("\taddress            | [N M P] | prev size  | size\n");
	while (busychunk_address != top) {
		mchunk_size = ShowBusyChunk(busychunk_address);

		busychunk_address = busychunk_address + mchunk_size;
		if (busychunk_address >= top) {
			break;
		}
		else if (mchunk_size == 0) {
			break;
		}
	}

	return;
}

void ShowThreadArena(ULONG64 arena_address)//thread arena
{
	ULONG64 mutex, flags, top, last_remainder,
		next, next_free, have_fastchunks, attached_threads, 
		system_mem, max_system_mem, fastbinsY, bins, 
		binmap, ArrayOffset, busychunk_address, mchunk_size;
	char fastchunks, noncontiuous;
	char *chunk_size = 0;

	ArrayOffset = GetTypeSize("mchunkptr");
	int bins_arrary_num = 0;

	mutex = ReadDigitField(arena_address, "malloc_state", "mutex");
	flags = ReadDigitField(arena_address, "malloc_state", "flags");
	top = ReadDigitField(arena_address, "malloc_state", "top");
	last_remainder = ReadDigitField(arena_address, "malloc_state", "last_remainder");
	next = ReadDigitField(arena_address, "malloc_state", "next");
	next_free = ReadDigitField(arena_address, "malloc_state", "next_free");
	have_fastchunks = ReadDigitField(arena_address, "malloc_state", "have_fastchunks");
	attached_threads = ReadDigitField(arena_address, "malloc_state", "attached_threads");
	system_mem = ReadDigitField(arena_address, "malloc_state", "system_mem");
	max_system_mem = ReadDigitField(arena_address, "malloc_state", "max_system_mem");
	fastbinsY = ReadDigitArrayField(arena_address, "malloc_state", "fastbinsY");
	bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
	binmap = ReadDigitArrayField(arena_address, "malloc_state", "binmap");
	fastchunks = flags & 1U ? '-' : 'F';
	noncontiuous = flags & 2U ? 'N' : 'C';

	dprintf("non_main_arena : 0x%p\n", arena_address);
	dprintf("mutex : %d have_fastchunks : %d binmap : %d attached_threads : %d system_mem : %d max_system_mem : %d\n",
		mutex, have_fastchunks, binmap, attached_threads, system_mem, max_system_mem);
	dprintf("flags : 0x%llx [%c %c] last_remainder : 0x%llx fastbinsY : 0x%llx\n",
		flags, fastchunks, noncontiuous, last_remainder, fastbinsY);
	dprintf("malloc_state   next : 0x%p next_free : 0x%p\n", next, next_free);
	dprintf("malloc_chunk   top  : 0x%p\n", top); 
	ShowMallocChunk(top);
	dprintf("malloc_chunk   bins : 0x%p\n", bins);
	ShowMallocChunk(bins);
	mchunk_size = ShowMchunkSize(bins);

	//busychunk_address = bins + mchunk_size;
	busychunk_address = arena_address + 0xaf0;
	dprintf("Busy Chunk\n");
	dprintf("\taddress            | [N M P] | prev size  | size\n");
	while (busychunk_address != top) {
		mchunk_size = ShowBusyChunk(busychunk_address);
		busychunk_address = busychunk_address + mchunk_size;
		if (busychunk_address >= busychunk_address + 0x4000000) {
			break;
		}
	}

	return;
}

void ShowMallocStateSimple(ULONG64 arena_address, int arena_num)//malloc_state显示部分信息
{
	ULONG64 flags, top, next, next_free,
		have_fastchunks, attached_threads, system_mem, max_system_mem,
		bins;
	char fastchunks, noncontiuous;
	PCSTR args;

	GetExpressionEx("main_arena", &main_arnea_addr, &args);

	flags = ReadDigitField(arena_address, "malloc_state", "flags");
	top = ReadDigitField(arena_address, "malloc_state", "top");
	next = ReadDigitField(arena_address, "malloc_state", "next");
	next_free = ReadDigitField(arena_address, "malloc_state", "next_free");
	have_fastchunks = ReadDigitField(arena_address, "malloc_state", "have_fastchunks");
	attached_threads = ReadDigitField(arena_address, "malloc_state", "attached_threads");
	system_mem = ReadDigitField(arena_address, "malloc_state", "system_mem");
	max_system_mem = ReadDigitField(arena_address, "malloc_state", "max_system_mem");
	bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
	fastchunks = flags & 1U ? '-' : 'F';
	noncontiuous = flags & 2U ? 'N' : 'C';
	if (arena_address != main_arnea_addr) {
		dprintf("arena_%d:0x%p flags:0x%llx [%c %c] have_fastchunks:0x%llx attached_threads:%d system_mem : 0x%llx max_system_mem : 0x%llx\n",
			arena_num, arena_address, flags, fastchunks, noncontiuous, have_fastchunks, attached_threads, system_mem, max_system_mem);
	}
	else {
		dprintf("arena_%d:0x%p flag:0x%llx [%c %c] have_fastchunks:0x%llx attached_threads:%d system_mem : 0x%llx max_system_mem : 0x%llx\n",
			arena_num, arena_address, flags, fastchunks, noncontiuous, have_fastchunks, attached_threads, system_mem, max_system_mem);
	}

	return;
}

void ShowHeapInfo(ULONG64 heap_address, int sub_num)//_heap_info
{
	ULONG64 prev, size, mprotect_size;
	char pad;
	
	//ar_prt = ReadDigitField(heap_address, "_heap_info", "ar_prt");
	prev = ReadDigitField(heap_address, "_heap_info", "prev");
	size = ReadDigitField(heap_address, "_heap_info", "size");
	mprotect_size = ReadDigitField(heap_address, "_heap_info", "mprotect_size");
	pad = ReadCharArrayField(heap_address, "_heap_info", "pad");
	if (prev == 0) {
		return;
	}

	dprintf("%d prev:0x%p size:%llx mprotect_size:%llx pad:%c\n",
		sub_num, prev, size, mprotect_size, pad);

	return;
}

void ShowSubHeap(ULONG64 top)
{
	ULONG64 heap_address, HEAP_MAX_SIZE;
	int sub_num;

	sub_num = 0;
	HEAP_MAX_SIZE = 0x4000000;
	heap_address = top & ~(HEAP_MAX_SIZE - 1);
	//ar_prt = ReadDigitField(heap_address, "_heap_info", "ar_prt");

	//dprintf("arena_%d 0x%p\n", ar_prt);
	do
	{
		ShowHeapInfo(heap_address, sub_num);
		heap_address = ReadDigitArrayField(heap_address, "_heap_info", "prev");
		//dprintf("0x%p\n", heap_address);
		sub_num = sub_num + 1;
	} while (heap_address != NULL);
}

void ShowMallocChunk(ULONG64 chunk_address)//malloc_chunk
{
	ULONG64 mchunk_prev_size, mchunk_size, fd, bk,
		fd_nextsize, bk_nextsize, FLAG_N, FLAG_I, 
		FLAG_P, mask;

	mchunk_prev_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_prev_size");
	mchunk_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_size");
	fd = ReadDigitField(chunk_address, "malloc_chunk", "fd");
	bk = ReadDigitField(chunk_address, "malloc_chunk", "bk");
	fd_nextsize = ReadDigitField(chunk_address, "malloc_chunk", "fd_nextsize");
	bk_nextsize = ReadDigitField(chunk_address, "malloc_chunk", "bk_nextsize");

	dprintf("\tmchunk_prev_size : %I64u mchunk_size : %I64u\n",
		mchunk_prev_size, mchunk_size);
	dprintf("\tfd          : 0x%p bk          : 0x%p\n", fd, bk);
	dprintf("\tfd_nextsize : 0x%p bk_nextsize : 0x%p\n", fd_nextsize, bk_nextsize);

	FLAG_N = GET_BIT(mchunk_size, 2) ? 4 : 0;
	FLAG_I = GET_BIT(mchunk_size, 1) ? 2 : 0;
	FLAG_P = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mask = FLAG_P | FLAG_I | FLAG_N;
	mchunk_size = mchunk_size ^ mask;

	return;
}
ULONG64 ShowMchunkSize(ULONG64 chunk_address)//malloc_chunk-->mchunk_size
{
	ULONG64  mchunk_size, FLAG_N, FLAG_I,
		FLAG_P, mask;

	mchunk_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_size");

	FLAG_N = GET_BIT(mchunk_size, 2) ? 4 : 0;
	FLAG_I = GET_BIT(mchunk_size, 1) ? 2 : 0;
	FLAG_P = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mask = FLAG_P | FLAG_I | FLAG_N;
	mchunk_size = mchunk_size ^ mask;

	return mchunk_size;
}
ULONG64 ShowBusyChunk(ULONG64 chunk_address)//busy chunk
{
	ULONG64 mchunk_prev_size, mchunk_size, FLAG_N, FLAG_I,
		FLAG_P, mask;
	int n, m, p;

	mchunk_prev_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_prev_size");
	mchunk_size = ReadDigitField(chunk_address, "malloc_chunk", "mchunk_size");
	FLAG_N = GET_BIT(mchunk_size, 2) ? 4 : 0;
	FLAG_I = GET_BIT(mchunk_size, 1) ? 2 : 0;
	FLAG_P = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mask = FLAG_P | FLAG_I | FLAG_N;
	n = GET_BIT(mchunk_size, 2) ? 1 : 0;
	m = GET_BIT(mchunk_size, 1) ? 1 : 0;
	p = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mchunk_size = mchunk_size ^ mask;

	dprintf("\t0x%p | [%d %d %d] | 0x%08llx | 0x%08llx\n", chunk_address, n, m, p, mchunk_prev_size, mchunk_size);

	return mchunk_size;
}

ULONG64 ReadDigitArrayField(ULONG64 StructAddress, const char *StructName, const char *FieldName)
{
	ULONG64 Value[1];
	ULONG FieldOffset;

	GetFieldOffset(StructName, FieldName, &FieldOffset);
	ReadMemory(StructAddress + FieldOffset, &Value[0], sizeof(Value), NULL);

	return Value[0];
}

char ReadCharArrayField(ULONG64 StructAddress, const char *StructName, const char *FieldName)
{
	char Value[1];
	ULONG FieldOffset;

	GetFieldOffset(StructName, FieldName, &FieldOffset);
	ReadMemory(StructAddress + FieldOffset, &Value[0], sizeof(Value), NULL);

	return Value[0];
}

int CheckArenaAddress(char Parameter[])
{
	ULONG64 ParameterAddress, ArenaAddress, ArenaFirstAddress;
	PCSTR args;

	sscanf_s(Parameter, "%llx", &ParameterAddress);

	GetExpressionEx("main_arena", &ArenaAddress, &args);
	ArenaFirstAddress = ArenaAddress;

	do {
		if (ParameterAddress == ArenaAddress) {
			return 1;
		}
		GetFieldValue(ArenaAddress, "malloc_state", "next", ArenaAddress);
	} while (ArenaAddress != ArenaFirstAddress);

	return 0;
}

ULONG64 *CheckChunkAddress(char Parameter[]) {
	ULONG64 ParameterAddress, arena_address_first, arena_address, top,
		bins, system_mem, mchunk_size, busychunk_address, size;
	PCSTR args;
	int i = 0, check = 0;
	static ULONG64 chunk_address[2];
	ULONG64 heap_address, HEAP_MAX_SIZE;

	HEAP_MAX_SIZE = 0x4000000;

	sscanf_s(Parameter, "%llx", &ParameterAddress);

	GetExpressionEx("main_arena", &arena_address, &args);
	arena_address_first = arena_address;
	do {
		if (i == 0) {
			top = ReadDigitField(arena_address, "malloc_state", "top");
			bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
			system_mem = ReadDigitField(arena_address, "malloc_state", "system_mem");
			mchunk_size = ShowMchunkSize(top);
			busychunk_address = top - system_mem + mchunk_size;
			mchunk_size = ShowMchunkSize(busychunk_address);
			busychunk_address = busychunk_address + mchunk_size;

			if (ParameterAddress >= busychunk_address && ParameterAddress <= top) {
				while (busychunk_address != top) {
					mchunk_size = ShowMchunkSize(busychunk_address);
					if (ParameterAddress >= busychunk_address && ParameterAddress < (busychunk_address + mchunk_size)) {
						chunk_address[0] = busychunk_address;
						chunk_address[1] = arena_address;
						return chunk_address;
					}
					else {
						check = 1;
					}
					busychunk_address = busychunk_address + mchunk_size;
				}
			}
			else if(check == 1){
				chunk_address[0] = 0;
				return chunk_address;
			}
		}
		else {
			top = ReadDigitField(arena_address, "malloc_state", "top");
			bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
			mchunk_size = ShowMchunkSize(bins);
			busychunk_address = bins + mchunk_size;
			size = ReadDigitField(arena_address - 0x20, "_heap_info", "size");

			if (arena_address < top) {
				while (busychunk_address != top) {
					mchunk_size = ShowMchunkSize(busychunk_address);
					if (ParameterAddress >= busychunk_address && ParameterAddress < (busychunk_address + mchunk_size)) {
						chunk_address[0] = busychunk_address;
						chunk_address[1] = arena_address;
						return chunk_address;
					}
					busychunk_address = busychunk_address + mchunk_size;
				}
			}
			else {
				if (ParameterAddress >= busychunk_address && ParameterAddress < (busychunk_address + size)) {
					mchunk_size = ShowMchunkSize(busychunk_address);
					while (busychunk_address != busychunk_address + size) {
						mchunk_size = ShowMchunkSize(busychunk_address);
						if (ParameterAddress >= busychunk_address && ParameterAddress < (busychunk_address + mchunk_size)) {
							chunk_address[0] = busychunk_address;
							chunk_address[1] = arena_address;
							return chunk_address;
						}
						busychunk_address = busychunk_address + mchunk_size;
					}
				}
				else if (ParameterAddress < busychunk_address) {
					heap_address = top & ~(HEAP_MAX_SIZE - 1);
					do
					{
						heap_address = ReadDigitArrayField(heap_address, "_heap_info", "prev");
						busychunk_address = heap_address + 0x20;
						mchunk_size = ShowMchunkSize(heap_address + 0x20);
						size = ReadDigitField(heap_address, "_heap_info", "size");
						while (busychunk_address != (busychunk_address + size) && ParameterAddress < (busychunk_address + size)) {
							mchunk_size = ShowMchunkSize(busychunk_address);
							if (ParameterAddress >= busychunk_address && ParameterAddress < (busychunk_address + mchunk_size)) {
								chunk_address[0] = busychunk_address;
								chunk_address[1] = arena_address;
								return chunk_address;
							}
							busychunk_address = busychunk_address + mchunk_size;
						}
						//dprintf("0x%p\n", heap_address);
					} while (heap_address != NULL);
				}
			}
		}
		i = i + 1;
		GetFieldValue(arena_address, "malloc_state", "next", arena_address);
	} while (arena_address != arena_address_first);
	chunk_address[0] = 0;
	return chunk_address;
}

void ShowMallocChunkSimple(ULONG64 chunk_address_start, ULONG64 arena_address)//malloc_chunk simple
{
	ULONG64 mchunk_prev_size, mchunk_size, FLAG_N, FLAG_I,
		FLAG_P, mask;
	ULONG64 *chunk_address_start_ptr = 0, chunk_address_start_nptr;
	chunk_address_start_ptr = &chunk_address_start;
	chunk_address_start_nptr = *chunk_address_start_ptr;
	ULONG64 *arena_address_ptr = 0, arena_address_nptr;
	arena_address_ptr = &chunk_address_start;
	arena_address_nptr = *arena_address_ptr;
	int n, m, p;

	mchunk_prev_size = ReadDigitField(chunk_address_start_nptr, "malloc_chunk", "mchunk_prev_size");
	mchunk_size = ReadDigitField(chunk_address_start_nptr, "malloc_chunk", "mchunk_size");

	FLAG_N = GET_BIT(mchunk_size, 2) ? 4 : 0;
	FLAG_I = GET_BIT(mchunk_size, 1) ? 2 : 0;
	FLAG_P = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mask = FLAG_P | FLAG_I | FLAG_N;
	n = GET_BIT(mchunk_size, 2) ? 1 : 0;
	m = GET_BIT(mchunk_size, 1) ? 1 : 0;
	p = GET_BIT(mchunk_size, 0) ? 1 : 0;
	mchunk_size = mchunk_size ^ mask;



	dprintf("Entry              User               Heap               Segment    Szie       PrevSize   [N M P]\n");
	dprintf("-----------------------------------------------------------\n");
	dprintf("0x%-16p 0x%-16p 0x%-16p 0x%-8llx 0x%-8llx 0x%-8llx [%d %d %d]\n",
		chunk_address_start_nptr, chunk_address_start_nptr + 0x10, arena_address_nptr - 0x20, arena_address_nptr -0x20 - 0x7f00000000, mchunk_size, mchunk_prev_size, n, m, p);

	return;
}

void ShowMainArenaChunkType(ULONG64 arena_address)
{
	ULONG64 ChunkSize[MAX_PATH] = { 0 }, TotalSize[MAX_PATH] = { 0 };
	double Num[MAX_PATH] = { 0 }, Percentage[MAX_PATH] = { 0 };
	ULONG64 top, bins, system_mem,
		busychunk_address, mchunk_size;
	int i = 0, TypeNum = 0, SizeCheck = -1, TotalNum = 0;
	top = ReadDigitField(arena_address, "malloc_state", "top");
	bins = ReadDigitArrayField(arena_address, "malloc_state", "bins");
	system_mem = ReadDigitField(arena_address, "malloc_state", "system_mem");
	mchunk_size = ShowMchunkSize(top);
	busychunk_address = top - system_mem + mchunk_size;
	mchunk_size = ShowMchunkSize(busychunk_address);
	busychunk_address = busychunk_address + mchunk_size;

	while (busychunk_address != top) {
		mchunk_size = ShowMchunkSize(busychunk_address);
		if (TypeNum == 0) {
			ChunkSize[0] = mchunk_size;
			TypeNum = 1;
		}

		for (i = 0; i <= TypeNum; i++) {
			if (ChunkSize[i] == 0 && SizeCheck == -1) {
				SizeCheck = 0 - 1 - i;
			}
			if (ChunkSize[i] == mchunk_size) {
				SizeCheck = i;
			}
		}
		if (SizeCheck >= 0) {
			Num[SizeCheck] = Num[SizeCheck] + 1;
		}
		else {
			ChunkSize[0 - SizeCheck -1] = mchunk_size;
			Num[0 - SizeCheck - 1] = Num[0 - SizeCheck - 1] + 1;
		}
		for (i = 0; ChunkSize[i] != 0; i++) {
			TypeNum = i + 1;
		}

		mchunk_size = ShowMchunkSize(busychunk_address);
		busychunk_address = busychunk_address + mchunk_size;
		SizeCheck = -1;

		if (busychunk_address >= top) {
			break;
		}
		else if (mchunk_size == 0) {
			break;
		}
	}

	dprintf("     ChunkSize[in bytes] Num     TotalSize[in bytes] Percentage\n");
	for (i = 0; ChunkSize[i] != 0; i++) {
		TotalNum = TotalNum + Num[i];
	}
	for (i = 0; ChunkSize[i] != 0; i++) {
		TotalSize[i] = ChunkSize[i] * Num[i];
		Percentage[i] = (Num[i] / TotalNum) * 100;
		dprintf("%4d 0x%-17llx %-7.0lf 0x%-17llx %.3lf%%\n", i, ChunkSize[i], Num[i], TotalSize[i], Percentage[i]);
	}
}

void ShowNonMainArenaChunkType(ULONG64 arena_address)
{
	ULONG64 ChunkSize[MAX_PATH] = { 0 }, TotalSize[MAX_PATH] = { 0 };
	double Num[MAX_PATH] = { 0 }, Percentage[MAX_PATH] = { 0 };
	ULONG64 size, address, mchunk_size;
	int i = 0, TypeNum = 0, SizeCheck = -1, TotalNum = 0;
	PCSTR args;
	size = ReadDigitField(arena_address - 0x20, "_heap_info", "size");

	while (arena_address <= (arena_address + size)) {
		mchunk_size = ShowMchunkSize(arena_address);
		if (mchunk_size == 0) {
			arena_address = arena_address + 0xaf0;
			mchunk_size = ShowMchunkSize(arena_address);
		}
		if (TypeNum == 0) {
			ChunkSize[0] = mchunk_size;
			TypeNum = 1;
		}

		for (i = 0; i <= TypeNum; i++) {
			if (ChunkSize[i] == 0 && SizeCheck == -1) {
				SizeCheck = 0 - 1 - i;
			}
			if (ChunkSize[i] == mchunk_size) {
				SizeCheck = i;
			}
		}
		if (SizeCheck >= 0) {
			Num[SizeCheck] = Num[SizeCheck] + 1;
		}
		else {
			ChunkSize[0 - SizeCheck - 1] = mchunk_size;
			Num[0 - SizeCheck - 1] = Num[0 - SizeCheck - 1] + 1;
		}
		for (i = 0; ChunkSize[i] != 0; i++) {
			TypeNum = i + 1;
		}

		mchunk_size = ShowMchunkSize(arena_address);
		arena_address = arena_address + mchunk_size;
		SizeCheck = -1;

		if (mchunk_size == 0) {
			break;
		}
	}

	dprintf("     ChunkSize[in bytes] Num     TotalSize[in bytes] Percentage\n");
	for (i = 0; ChunkSize[i] != 0; i++) {
		TotalNum = TotalNum + Num[i];
	}
	for (i = 0; ChunkSize[i] != 0; i++) {
		TotalSize[i] = ChunkSize[i] * Num[i];
		Percentage[i] = (Num[i] / TotalNum) * 100;
		dprintf("%4d 0x%-17llx %-7.0lf 0x%-17llx %.3lf%%\n", i, ChunkSize[i], Num[i], TotalSize[i], Percentage[i]);
	}
}