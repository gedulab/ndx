#pragma once

/*��ӡ��ӳ���ļ����ص��Ŀ¼*/
void ShowFileMount(ULONG64 Address);

/*�õݹ鷽����ȡ���ļ�������·��*/
void ShowFilePath(ULONG64 dentry_addr);

/*��ȡfile�ṹ���е�dentry�ֶε�ַ*/
void GetFileDentry(ULONG64 file_addr);

/*��ӡ�ļ���ϸ��Ϣ*/
void PrintDetailsFile(uint64_t file_addr, int i);

void GetTaskField(uint64_t ts_addr, int detail);

int GetTaskAddr(uint64_t init_ts_addr, int ts_num, int detail);

int NdxDmesgUsage(const char* msg);




