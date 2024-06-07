#include "ndx.h"
#include<iostream>
#include <algorithm>
#include <vector>
using namespace std;

typedef struct tag_list_list_head {
	struct list_head* next, * prev;
}linux_list_head;

struct nd_slabinfo {
	char name[64];
	int refs;
	int  align;
	unsigned int  object_size;
	unsigned int  slab_size, useroffset;
	unsigned int usersize;
	linux_list_head list;
	unsigned int objects_total,total, objperslab;
}; 
void ReadSlabinfo(ULONG64 Address, nd_slabinfo& slab);
void ShowSlabinfo(nd_slabinfo& slab);

void ReadSlabinfo(ULONG64 Address, nd_slabinfo& slab)
{
	ULONG64 FieldPtr;
	unsigned int temp,objperslab,pagesperslab;
	ULONG FieldOffset;
	char name[64];
	slab.object_size = ReadUINT32(Address, "kmem_cache", "object_size");
	slab.slab_size = ReadUINT32(Address, "kmem_cache", "size");
	slab.align = ReadUINT32(Address, "kmem_cache", "align");
	slab.useroffset = ReadUINT32(Address, "kmem_cache", "useroffset");
	slab.usersize = ReadUINT32(Address, "kmem_cache", "usersize");
	slab.refs = ReadUINT32(Address, "kmem_cache", "refcount");
	GetFieldOffset("kmem_cache", "name", &FieldOffset);
	ReadPointer(Address + FieldOffset, &FieldPtr);
	temp = ReadUINT32(Address, "kmem_cache", "oo") ;
	slab. objperslab = temp & (0x0000ffff);
	slab.objects_total = slab.objperslab;
	slab.total = slab.slab_size * slab.objects_total;
	ReadMemory(FieldPtr, &name,sizeof(name), NULL);
	strcpy(slab.name, name);
	// read list
	GetFieldOffset("kmem_cache", "list", &FieldOffset);
	ReadMemory(Address + FieldOffset, &slab.list, sizeof(linux_list_head), NULL);

}

void ShowSlabinfo(nd_slabinfo& slab)
{
	dprintf("%-60s %-16d %-11d %-11d %-11d %-6d %-6d %-11d %-8d %-8d \n", slab.name,slab.total,slab.objects_total, slab.objperslab,slab.object_size, slab.slab_size, slab.align,slab.useroffset,slab.usersize,slab.refs);
}
bool compare_vec_slab_name_g(nd_slabinfo i1, nd_slabinfo i2)
{		
		return (strcmp(i1.name , i2.name)>0);		
}

bool compare_vec_slab_name(nd_slabinfo i1, nd_slabinfo i2)
{
	if (strcmp(i1.name,i2.name))
	{
		return (i1.name == i2.name);
	}
}

bool compare_vec_slab_Great(nd_slabinfo i1,nd_slabinfo i2)
{
	return (i1.total >i2.total);
}
void ShowSlabWithSortBySize(ULONG64 ListHeadAddress, ULONG64 FirstEntryAddress, ULONG ListFieldOffset )
{
	int num=0;
	ULONG64 addr = FirstEntryAddress;
	nd_slabinfo slab;
	vector <nd_slabinfo>vec_slab;
	do{
		ReadSlabinfo(addr - ListFieldOffset, slab);
		num++;
		vec_slab.push_back(slab);
		if (CheckControlC()) {
			break;
		}
		ShowSlabinfo(slab);
		addr = (ULONG64)slab.list.next;
	} while (addr != ListHeadAddress);
	/// ÅÅÐò»ã×Ü
	dprintf("\n");
	dprintf("Aggregate,sort by total:\n");
	dprintf(" name                                                       total_obj       num_obj   objperslab    object_size   size  align useroffset usersize refcount      \n");

	vector <nd_slabinfo>vec_s;
	for (int i = 0; i < num; i++) {
		strtok(vec_slab[i].name, "(");
	}
	std::sort(vec_slab.begin(), vec_slab.end(), compare_vec_slab_name_g);
	nd_slabinfo t;
	strcpy(t.name, vec_slab[0].name);
	t.align = vec_slab[0].align;
	t.object_size = vec_slab[0].object_size;
	t.objperslab = vec_slab[0].objperslab;
	t.refs = vec_slab[0].refs;
	t.useroffset = vec_slab[0].useroffset;
	t.usersize = vec_slab[0].usersize;
	t.objects_total = vec_slab[0].objects_total;
	t.total = vec_slab[0].total;
	t.slab_size = vec_slab[0].slab_size;

	for (int i = 1; i < vec_slab.size() - 1; i++) {
		if (strcmp(t.name, vec_slab[i].name) == 0) {
			t.objects_total += vec_slab[i].objperslab;
			t.total = t.objects_total * t.slab_size;
		}
		else {
			vec_s.push_back(t);
			strcpy(t.name, vec_slab[i].name);
			t.align = vec_slab[i].align;
			t.object_size = vec_slab[i].object_size;
			t.objperslab = vec_slab[i].objperslab;
			t.refs = vec_slab[i].refs;
			t.useroffset = vec_slab[i].useroffset;
			t.usersize = vec_slab[i].usersize;
			t.objects_total = vec_slab[i].objects_total;
			t.total = vec_slab[i].total;
			t.slab_size = vec_slab[i].slab_size;
		}	
	}
	std::sort(vec_s.begin(), vec_s.end(), compare_vec_slab_Great);

	for (int i = 0; i < vec_s.size()-1; i++)
	{
		ShowSlabinfo(vec_s[i]);
	}
}
void show_direct(ULONG64 FirstAddress, ULONG64 Address, ULONG FieldOffset)
{
	nd_slabinfo slab;
	int num = 0;
	ULONG64 addr;
	addr = Address;
	do {
		if (CheckControlC())
		{
			break;
		}

		ReadSlabinfo(addr - FieldOffset, slab);
		ShowSlabinfo(slab);
		num++;
		addr = (ULONG64)slab.list.next;
		}while (addr != FirstAddress);
	
}

unsigned int ReadUINT32(ULONG64 StructAddress, const char* StructName, const char* FieldName)
{
	unsigned int Value;
	ULONG FieldOffset;

	GetFieldOffset(StructName, FieldName, &FieldOffset);
	ReadMemory(StructAddress + FieldOffset, &Value, sizeof(Value), NULL);

	return Value;

}
ULONG64 ReadDigitField(ULONG64 StructAddress, const char *StructName, const char *FieldName)
{
	ULONG64 Value;
	ULONG FieldOffset;

	GetFieldOffset(StructName, FieldName, &FieldOffset);
	ReadMemory(StructAddress + FieldOffset, &Value, sizeof(Value), NULL);

	return Value;
}