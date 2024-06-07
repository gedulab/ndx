#include <iostream>
#include "ndx.h"
#include "lsof.h"

/*打印被映射文件挂载点根目录*/
void ShowFileMount(ULONG64 Address)
{
	/*从mnt字段中获取其挂载点名称*/
	ULONG Offset;
	unsigned char Buffer[32];

	GetFieldOffset("file", "f_path", &Offset);
	Address = Address + Offset;
	GetFieldValue(Address, "path", "mnt", Address);
	GetFieldValue(Address, "vfsmount", "mnt_root", Address);
	GetFieldOffset("dentry", "d_name", &Offset);
	Address = Address + Offset;
	GetFieldValue(Address, "qstr", "name", Address);
	ReadMemory(Address, &Buffer, sizeof(Buffer), NULL);
	dprintf("%s", Buffer);
}

/*用递归方法读取该文件的完整路径*/
void ShowFilePath(ULONG64 dentry_addr) {
	ULONG Offset;
	unsigned char Buffer[32];
	ULONG64 d_parent = 0;

	GetFieldOffset("dentry", "d_parent", &Offset);
	//d_parent是dentry中的一个字段,保存有他的父目录
	d_parent = dentry_addr + Offset;
	ReadPointer(d_parent,&d_parent);
	if (d_parent != dentry_addr)
	{
		ShowFilePath(d_parent);
		dprintf("/");
	}
	GetFieldOffset("dentry", "d_iname", &Offset);
	dentry_addr = dentry_addr + Offset;
	ReadMemory(dentry_addr, &Buffer, sizeof(Buffer), NULL);
	dprintf("%s", Buffer);
}

/*获取file结构体中的dentry字段地址*/
void GetFileDentry(ULONG64 file_addr)
{
	/*该函数用于判断该文件是否是系统的虚文件或匿名文件*/
	ULONG Offset;
	ULONG64 dentry_addr = 0;
	ULONG64 d_parent = 0;

	GetFieldOffset("file", "f_path", &Offset);
	dentry_addr = file_addr + Offset;
	GetFieldValue(dentry_addr, "path", "dentry", dentry_addr);
	//用于判断该文件是否有上一级目录
	GetFieldOffset("dentry", "d_parent", &Offset);
	//d_parent是dentry中的一个字段,保存有他的父目录
	d_parent = dentry_addr + Offset;
	ReadPointer(d_parent, &d_parent);
	if (dentry_addr==d_parent) {
		ShowFileMount(file_addr);
		ShowFilePath(dentry_addr);
	}
	else
	{
		ShowFilePath(dentry_addr);
	}
	dprintf("\n");
}

/*打印文件详细信息*/
void PrintDetailsFile(uint64_t file_addr,int i) {
	//char* name = NULL;
	//ULONG read = 0;
	
	uint32_t f_flags = 0;
	uint64_t f_pos = 0;
	uint32_t mnt_flags = 0;
	uint64_t i_ino = 0;
	uint64_t mnt_addr = 0;
	uint64_t f_inode_addr = 0;
	uint64_t dentry_addr = 0;
	uint64_t name_addr = 0;


	static int f_inode_offset = -1;
	static int i_ino_offset = -1;
	static int f_path_offset = -1;
	static int mnt_offset = -1;
	static int mnt_flags_offset = -1;
	static int dentry_offset = -1;
	static int d_name_offset = -1;
	static int name_offset = -1;

	if (f_inode_offset<0) {
		f_inode_offset = NdxGetOffset("file", "f_inode");
		i_ino_offset = NdxGetOffset("inode", "i_ino");
		f_path_offset = NdxGetOffset("file", "f_path");
		mnt_offset = NdxGetOffset("path", "mnt");
		mnt_flags_offset = NdxGetOffset("vfsmount", "mnt_flags");
		dentry_offset = NdxGetOffset("path", "dentry");
		d_name_offset = NdxGetOffset("dentry", "d_name");
		name_offset = NdxGetOffset("qstr", "name");
	}

	uint64_t dentry_ptr = file_addr + f_path_offset + dentry_offset;
	uint64_t mnt_ptr = file_addr + f_path_offset + mnt_offset;
	uint64_t f_inode_ptr = file_addr + f_inode_offset;
	ReadPointer(mnt_ptr,&mnt_addr);
	ReadPointer(f_inode_ptr,&f_inode_addr);
	ReadPointer(dentry_ptr,&dentry_addr);
	uint64_t name_ptr = dentry_addr + d_name_offset + name_offset;
	ReadPointer(name_ptr,&name_addr);
	uint64_t qstr_addr = dentry_addr + d_name_offset;
	
	//int qstr_size=GetTypeSize("qstr");
	//int name_len = 11;
	//name = (char*)malloc(name_len);
	//ReadMemory(name_addr,name,name_len,&read);

	GetFieldValue(file_addr,"file","f_flags",f_flags);
	GetFieldValue(file_addr,"file","f_pos",f_pos);
	GetFieldValue(mnt_addr,"vfsmount","mnt_flags",mnt_flags);
	GetFieldValue(f_inode_addr,"inode","i_ino",i_ino);
	
	dprintf("%-7lli%-10o%-11i%-8lu%p  %p  ",f_pos,f_flags,mnt_flags,i_ino,file_addr,dentry_addr);
	dprintf("%d->",i);
	//ShowMapFilePath_2(file_addr);
	GetFileDentry(file_addr);
	//int idx = 0;
	//while (idx < name_len)
	//{
	//	char c = name[idx];
	//	if (c == '\0')
	//		break;
	//	else
	//	{
	//		dprintf("%c", c);
	//	}
	//	idx = idx + 1;
	//}
	//dprintf("\n");
	//free(name);
}

void GetTaskField(uint64_t ts_addr,int detail) {
	
	char* fd_cache = NULL;
	ULONG read = 0;

	uint64_t files_addr = 0;
	uint64_t files_ptr = 0;
	uint64_t fdtab_addr = 0;
	uint64_t fd_ptr = 0;
	uint64_t fd_addr = 0;
	uint32_t max_fds = 0;
	int pid = 0;
	int file_size = -1;

	static int files_offset = -1;
	static int fdtab_offset = -1;
	static int fd_offset = -1;
	
	if (files_offset<0) {
		files_offset = NdxGetOffset("task_struct", "files");
		fdtab_offset = NdxGetOffset("files_struct", "fdtab");
		fd_offset = NdxGetOffset("fdtable", "fd");
	}

	GetFieldValue(ts_addr, "task_struct", "pid", pid);
	dprintf("pid: %d  ",pid);
	//获取fd的地址
	files_ptr = ts_addr + files_offset;
	ReadPointer(files_ptr, &files_addr);

	fdtab_addr = files_addr + fdtab_offset;
	GetFieldValue(fdtab_addr, "fdtable", "max_fds", max_fds);
	fd_ptr = fdtab_addr + fd_offset;
	ReadPointer(fd_ptr, &fd_addr);
	dprintf("fd_addr :%p\n ", fd_addr);

	//读取该指针指向的file数组
	file_size = GetTypeSize("file");
	int ptr_size = sizeof(uint64_t);
	int total_size = ptr_size * max_fds;
	fd_cache = (char*)malloc(total_size);
	ReadMemory(fd_addr, fd_cache, total_size, &read);
	if (read < total_size) {
		dprintf("read task_struct failed %d < %d \n", read, total_size);
		return;
	}
	
	//具体的读信息操作
	int i = 0;
	uint64_t temp_file_addr = 0;
	if (detail) {
		dprintf("----------------------------------------------------------"
			"----------------\n");
		dprintf("f_pos  f_flags   mnt_flags  i_ino   file_addr         " 
			"dentry_addr       name\n");
	}
	while (i<max_fds)
	{
		uint64_t file_addr = *(uint64_t*)(fd_cache+i*ptr_size);
		
		if (file_addr == 0)
		{
			i++;
			continue;
		}
		else
		{
			if(!detail)
			dprintf("%d ",i);
		}		
		if (detail) {
			PrintDetailsFile(file_addr,i);
		}
		i++;
	}
	if (detail) {
		dprintf("----------------------------------------------------------"
			"----------------\n");
	}
	dprintf("\n");
	free(fd_cache);
}

int GetTaskAddr(uint64_t init_ts_addr,int ts_num,int detail) {

	uint64_t ts_addr = 0; 
	ts_addr = init_ts_addr;
	ULONG task_Offset = 0;
	GetFieldOffset("task_struct", "tasks", &task_Offset);
	
	int i = 1;

	while (i) {
		dprintf("ts_addr :%p  ", ts_addr);
		GetTaskField(ts_addr, detail);
		GetFieldValue(ts_addr, "task_struct", "tasks.next", ts_addr);
		ts_addr = ts_addr - task_Offset;
		if (ts_addr == init_ts_addr) {				
			break;
		}
		if (ts_num == 1)
			break;
		i++;
		if (CheckControlC())
		{
			break;
		}
	}
	return 0;
}

int NdxDmesgUsage(const char* msg)
{
	if (msg)
		dprintf("error: %s\n", msg);

	dprintf("dmesg[options]\n"
		"-P displays the specified address\n"
		"-a Lists all processes\n"
		"-v Displays details\n");
	return -1;
};

DECLARE_API(lsof) {

	uint64_t ts_addr;
	int ts_num = -1;
	int detail = 0;
	PCSTR re;

	const char* cursor = args;
	char option[256];
	size_t eaten_length = 0;
	int num = 0, argc = 1, len = 0;

	do {
		switch (cursor[1])
		{
		//Displays the specified address
		case'P':
			cursor += 2;
			len = NdxPickNextField(cursor, option, sizeof(option), &eaten_length);
			if (len > 0) {
				ts_addr = strtoull(option, NULL, 0);
				ts_num = 1;
			}
			break;
		//Lists all processes
		case'a':
			cursor += 2;
			eaten_length = 0;
			break;
		//Displays details
		case'v':
			cursor += 2;
			eaten_length = 0;
			detail = 1;
			break;
		case'h':
		case'H':
		case'?':
			NdxDmesgUsage(NULL);
			return;
		default:
			break;
		}
		cursor += eaten_length;
		while (*cursor == ' ') cursor++;
	}while (*cursor != 0);

	if (ts_num == -1) {
		GetExpressionEx("lk!init_task", &ts_addr, &re);
		dprintf("init_task :%p\n ", ts_addr);
	}

	GetTaskAddr(ts_addr, ts_num, detail);
}