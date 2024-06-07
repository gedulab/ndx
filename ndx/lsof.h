#pragma once

/*打印被映射文件挂载点根目录*/
void ShowFileMount(ULONG64 Address);

/*用递归方法读取该文件的完整路径*/
void ShowFilePath(ULONG64 dentry_addr);

/*获取file结构体中的dentry字段地址*/
void GetFileDentry(ULONG64 file_addr);

/*打印文件详细信息*/
void PrintDetailsFile(uint64_t file_addr, int i);

void GetTaskField(uint64_t ts_addr, int detail);

int GetTaskAddr(uint64_t init_ts_addr, int ts_num, int detail);

int NdxDmesgUsage(const char* msg);




