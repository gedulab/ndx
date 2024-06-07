#include"ndx.h"
#include<iostream>
#include <algorithm>
#include <vector>
using namespace std;

typedef struct nd_vm_area_struct {
	
	ULONG64 vm_start;        //��vm_mm�е���ʼ��ַ
	ULONG64 vm_end;          //��vm_mm�еĽ�����ַ
	ULONG64 next;//����Ѱ����һ���ṹ���ַ
	ULONG64 Address;//�˽ṹ���ַ
	char filename[64];
	char filepath[MAX_PATH];
	char r, w, x, sp, s;
	ULONG64 vm_flags;//����
	ULONG64 vm_pgoff;//ƫ����
}nd_vm_area_struct;

void get_mmap_byname(ULONG64 mmap);

void read_vma64(ULONG64 mmap, nd_vm_area_struct &vma) {
	ULONG64 address, Address, vm_file;
	ULONG FieldOffset;
	GetFieldValue(mmap, "vm_area_struct", "vm_next", vma.next);
	GetFieldValue(mmap, "vm_area_struct", "vm_start", vma.vm_start);
	GetFieldValue(mmap, "vm_area_struct", "vm_end", vma.vm_end);
	GetFieldValue(mmap, "vm_area_struct", "vm_file", vm_file);
	vma.Address = mmap;
	GetFieldValue(mmap, "vm_area_struct", "vm_pgoff", vma.vm_pgoff);
	GetFieldValue(mmap, "vm_area_struct", "vm_flags", vma.vm_flags);
	vma.vm_flags &= ~VM_SOFTDIRTY;
	vma.r = vma.vm_flags & VM_READ ? 'r' : '-';
	vma.w = vma.vm_flags & VM_WRITE ? 'w' : '-';
	vma.x = vma.vm_flags & VM_EXEC ? 'x' : '-';
	vma.sp = vma.vm_flags & VM_MAYSHARE ? 's' : 'p';
	vma.s = vma.vm_flags & VM_SHARED ? 's' : '-';
	
	if (vm_file != 0x0)
	{
		ULONG Offset,offset2;
		char Buffer[MAX_PATH];
		GetFieldOffset("file", "f_path", &Offset);//��ȡf_pathʵ������ƫ�Ƶ�ַ
		Address = vm_file + Offset;//file�ṹ�����ַ+f_pathʵ����ƫ�Ƶ�ַ=path�ṹ�����ַ
		GetFieldValue(Address, "path", "dentry", Address);//�õ�dentryʵ������ָ���dentry�ṹ���ַ
		GetFieldOffset("dentry", "d_name", &Offset); //��ȡd_nameʵ������ƫ�Ƶ�ַ
		Address = Address + Offset;//dentry�ṹ�����ַ+d_nameʵ����ƫ�Ƶ�ַ=qstr�ṹ�����ַ
		GetFieldValue(Address, "qstr", "name", Address);//��ȡname�ֶ��е�ַ
		ReadMemory(Address, &vma.filename, sizeof(vma.filename), NULL);//��ȡ��ӳ���ļ����ļ���

		GetFieldOffset("file", "f_path", &Offset);//��ȡf_pathʵ������ƫ�Ƶ�ַ
		Address = vm_file + Offset;//file�ṹ�����ַ+f_pathʵ����ƫ�Ƶ�ַ=path�ṹ�����ַ
		GetFieldValue(Address, "path", "dentry", Address);//�õ�dentryʵ������ָ���dentry�ṹ���ַ
		GetFieldOffset("dentry", "d_parent", &Offset);
		ReadPointer(Address + Offset, &Address);//get parent dentry address
		//address = Address;
		GetFieldOffset("dentry", "d_name", &Offset);
		GetFieldValue(Address + Offset, "qstr", "name", address);
		ReadMemory(address, &vma.filepath, sizeof(vma.filepath), NULL);//��һ�����ļ���
		GetFieldOffset("dentry", "d_parent", &offset2);
		ReadPointer(Address+offset2,&Address);//more parent dentry address
		GetFieldOffset("dentry", "d_name",&offset2);
		GetFieldValue(Address + offset2, "qstr", "name", address);
		ReadMemory(address, &Buffer, sizeof(Buffer), NULL);
		while(strcmp(Buffer, "/") != 0&&Address!=0x0)//���һ�����ļ�����name����"/"
		{
			strcat(Buffer, "/");
			strcat(Buffer, vma.filepath);
			strcpy(vma.filepath, Buffer);
			GetFieldOffset("dentry", "d_parent", &Offset);
			ReadPointer(Address + Offset, &Address);
			GetFieldOffset("dentry", "d_name", &Offset);
			address = Address + Offset;
			GetFieldValue(address, "qstr", "name", address);
			ReadMemory(address, &Buffer, sizeof(Buffer), NULL);
		}
		strcpy(Buffer,"/");
		strcat(Buffer,vma.filepath);
		strcat(Buffer, "/");
		strcat(Buffer,vma.filename);//ֻ��Ϊ�˺�linux��ʽ�����ǰ�߲���/��Ȼ���nameҲ���ϣ�����ܵ�̫�����������Ż�
		strcpy(vma.filepath,Buffer);
	}
	else {
		strcpy(vma.filename,"");
		strcpy(vma.filepath, "");
	}
}

bool Great_name(nd_vm_area_struct vma1, nd_vm_area_struct vma2){

	return(strcmp(vma1.filename,vma2.filename)>0);
}
void showmmap(nd_vm_area_struct vma) {

	dprintf("%llx--%llx  %c%c%c%c  %08p  %08p  0x%llx          %s \n", vma.vm_start, vma.vm_end, vma.r, vma.w, vma.x, vma.sp, vma.vm_pgoff, vma.vm_flags, vma.Address, vma.filepath);
}
void get_mmap(ULONG64 mmap) {
	int num = 0;
	nd_vm_area_struct vm;
	ULONG64 address =mmap;
	do {	
		if (CheckControlC()) {
			break;
		}
		read_vma64(address, vm);
		address = vm.next;
		showmmap(vm);
		num++;
	} while (address != mmap&&address!=NULL);

}

void get_mmap_byname(ULONG64 mmap) {
	int num = 0;
	nd_vm_area_struct vm;
	vector<nd_vm_area_struct>vec_vma;
	ULONG64 address = mmap;
	do {
		if (CheckControlC()) {
			break;
		}
		read_vma64(address, vm);
		vec_vma.push_back(vm);
		address = vm.next;
		num++;
	} while (address != mmap && address != NULL);
	sort(vec_vma.begin(), vec_vma.end(), Great_name);
	for (int i = 0; i < num; i++) {
		showmmap(vec_vma[i]);
	}
}

DECLARE_API(address)
{
	const char* option, * filename;
	ULONG64  mm_mmap, mmap, Address;
	ULONG offset;
	ReadMsr(NDB_MSR_MMAP, &mm_mmap);
	GetFieldOffset("mm_struct", ".start_code", &offset);
	ReadPointer(mm_mmap + offset, &mmap);
	GetFieldValue(mm_mmap, "vm_area_struct", "vm_mm", mm_mmap);

	if (option = strstr(args, "-s")) {
		//ShowMmStruct_All(mm_mmap);
		dprintf("\nstart--end             property pgoff    flags      vma_address               filepath \n");
		get_mmap_byname(mmap);
	}
	else if (option = strstr(args, "0x")) {
		filename = strchr(option, '0');
		if (filename == NULL) {
			dprintf("Missing vma address to decode\n");
			return;
		}
		sscanf_s(filename, "%llx", &Address);
		dprintf("%p\n", Address);
		ShowVmAreaStruct(Address);
	}
	else if (option = strstr(args, "-m")) {
		ShowMmStruct_All(mm_mmap);
	}
	else {
		//ShowMmStruct_All(mm_mmap);
		dprintf("\nstart--end             property pgoff    flags      vma_address               filepath \n");
		get_mmap(mmap);
	}
}
