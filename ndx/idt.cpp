#include "ndx.h"
#include<iostream>
#include <algorithm>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
using namespace std;

char Test[MAX_PATH] = "Test";

int ShowGateStruct(ULONG64 Address, int Num)//gate_struct
{
	ULONG64 SymOffset, offset_low, offset_middle, offset_high;
	ULONG FieldOffset;
	char SymName[MAX_PATH], low[MAX_PATH], middle[MAX_PATH], high[MAX_PATH];

	GetSymbol(Address, SymName, &SymOffset);
	if (Num == 0) {
		dprintf("%03d", Num);
		GetFieldOffset("gate_struct", "bits", &FieldOffset);
		ShowIdtBits(Address + FieldOffset);
		GetFieldValue(Address, "gate_struct", "offset_low", offset_low);
		GetFieldValue(Address, "gate_struct", "offset_middle", offset_middle);
		GetFieldValue(Address, "gate_struct", "offset_high", offset_high);
		sprintf_s(low, "%04llx", offset_low);
		sprintf_s(middle, "%04llx", offset_middle);
		sprintf_s(high, "%llx", offset_high);
		strcat_s(high, middle);
		strcat_s(high, low);
		sscanf_s(high, "%llx", &Address);//获取函数地址
		GetSymbol(Address, SymName, &SymOffset);
		dprintf("%p %s\n", Address, SymName);

		GetSymbol(Address, Test, &SymOffset);
		Num = Num + 1;

		return Num;
	}
	if (strcmp(SymName, Test) == 0) {//判断函数是否重复
		return Num;
	}
	else {
		dprintf("%03d", Num);
		GetFieldOffset("gate_struct", "bits", &FieldOffset);
		ShowIdtBits(Address + FieldOffset);
		GetFieldValue(Address, "gate_struct", "offset_low", offset_low);
		GetFieldValue(Address, "gate_struct", "offset_middle", offset_middle);
		GetFieldValue(Address, "gate_struct", "offset_high", offset_high);
		sprintf_s(low, "%04llx", offset_low);
		sprintf_s(middle, "%04llx", offset_middle);
		sprintf_s(high, "%llx", offset_high);
		strcat_s(high, middle);
		strcat_s(high, low);
		sscanf_s(high, "%llx", &Address);//获取函数地址
		GetSymbol(Address, SymName, &SymOffset);
		dprintf("%p %s\n", Address, SymName);

		GetSymbol(Address, Test, &SymOffset);
		Num = Num + 1;

		return Num;
	}
}

void ShowIdtData(ULONG64 Address)//idt_data
{
	ULONG64 vector, segment, addr;
	ULONG FieldOffset;

	GetFieldValue(Address, "idt_data", "vector", vector);
	GetFieldValue(Address, "idt_data", "segment", segment);
	GetFieldValue(Address, "idt_data", "addr", addr);

	dprintf("vector = %llx segment = %llx");
	GetFieldOffset("idt_data", "bits", &FieldOffset);
	ShowIdtBits(Address + FieldOffset);
	dprintf(" addr = %p\n", addr);
}
void IdxShowTable(const char *table_name)//idx -g循环
{
	ULONG64 Address, ArrayOffset, ArrayCount;
	ULONG ArrayLen, nEntrySize;
	PCSTR args;
	int ArrayNum;

	GetExpressionEx(table_name, &Address, &args);
	dprintf("--------------------------------------------------------------\n");
	dprintf("%s\t base = %p\n", table_name, Address);
	ArrayLen = GetTypeSize(table_name);
	nEntrySize = GetTypeSize("idt_data");
	ArrayCount = ArrayLen / nEntrySize;

	ArrayNum = 0;

	while (ArrayNum < (int)ArrayCount) {
		ShowIdtData(Address);

		Address = Address + nEntrySize;
		ArrayNum = ArrayNum + 1;
	}
}

void ShowIdtBits(ULONG64 Address)//idt_bits
{
	ULONG32 type, bit0, bit1;

	GetFieldValue(Address, "idt_bits", "type", type);
	bit0 = GET_BIT(type, 0);
	bit1 = GET_BIT(type, 1);
	if (bit0 == 0 && bit1 == 1) {
		dprintf(" GATE_INTERRUPT  ");
	}
	else if (bit0 == 1 && bit1 == 1) {
		dprintf(" GATE_TRAP       ");
	}
	else if (bit0 == 0 && bit1 == 0) {
		dprintf(" GATE_CALL       ");
	}
	else if (bit0 == 1 && bit1 == 0) {
		dprintf(" GATE_TASK       ");
	}
}
/// @brief 把大小为count个字节的16进制字符串转换为数字
/// @param str 大小为count字节的16进制字符串，例如 ffffffff80808080
/// @param count 传入字符串的字节数
/// @return 转换得到的数字
static ULONG64 atoi_HEX(const char* str, int count)
{
	ULONG64 numbs = 0;
	ULONG64 bits = 0;
	for (size_t i = 0; i < count; i++) {
		char ch = str[count - 1 - i];
		size_t numb = (ch > '9') ? ch - 'a' + 10 : ch - '0';
		numbs += (numb << bits);
		bits += 4;
	}
	return numbs;
}

/// @brief 根据带有b指令的一行汇编代码，解析这条指令跳转到的地址和函数名，（并根据AddrSets进行去重）
/// @param OneCmd 要解析的汇编指令
/// @param addr_name 函数地址为key，函数名为value的集合，用来去重
/// @param FuncName 跳转到的函数名
/// @param FuncAddr 跳转到的函数地址
/// @return true--这个函数是新遇到的，fasle--这个函数是之前遇到的
static bool GetFuncName(const char* OneCmd, unordered_map<ULONG64, string>& addr_name, char* FuncName, ULONG64& FuncAddr)
{
	ULONG64 SymOffset;
	size_t index = 26;
	while (index < 40 && OneCmd[index] != '#') ++index; // 指向 #0xffff 的 #
	const char* FuncAddrStr = OneCmd + index + 3;

	FuncAddr = atoi_HEX(FuncAddrStr, 16);
	if (0 == addr_name.count(FuncAddr))		// 根据地址去重
	{
		GetSymbol(FuncAddr, FuncName, &SymOffset);
		addr_name.insert({ FuncAddr, FuncName });

		dprintf("  |___%p %s\n", FuncAddr, FuncName);
		return true;	// 函数名是是新的，
	}
	else {
		dprintf("  |___%p %s\n", FuncAddr, addr_name[FuncAddr].c_str());
	}
	return false;		// 函数名是旧的
}


/// @brief 根据带有b指令的一行汇编代码，解析这条指令跳转到的地址和函数名
/// @param OneCmd 要解析的汇编指令
/// @param FuncName 跳转到的函数名
/// @param FuncAddr 跳转到的函数地址
static void GetFuncName2(const char* OneCmd, char* FuncName, ULONG64& FuncAddr)
{
	//dprintf("%s", OneCmd);
	ULONG64 SymOffset;
	size_t index = 26;
	while (index < 40 && OneCmd[index] != '#') ++index; // 指向 #0xffff 的 #
	const char* FuncAddrStr = OneCmd + index + 3;

	FuncAddr = atoi_HEX(FuncAddrStr, 16);
	GetSymbol(FuncAddr, FuncName, &SymOffset);
}


/// @brief 根据函数名称和函数地址，解析当前函数的下一层函数
/// @param FuncName 函数名
/// @param FuncAddr 函数地址
/// @param NameSets 储存函数名的集合，只解析该集合中的函数
static void HanderFunc(const char* FuncName, ULONG64 FuncAddr, unordered_map<string, size_t>& NameSets)
{
	if (0 == NameSets.count(FuncName)) return;

	// case1 按照bl来解析
	if (0 != strcmp(FuncName, "lk!el1_sync") && 0 != strcmp(FuncName, "lk!el0_sync")) {
		size_t MaxOffSet2 = NameSets[FuncName];
		size_t j = 0;
		while (j < MaxOffSet2) {
			char OneCmd4[100] = "";
			ULONG64 CmdAddr4 = FuncAddr + j;
			Disasm(&CmdAddr4, OneCmd4, 100);

			if (OneCmd4[26] == 'b' && OneCmd4[28] != 'r') {
				char FuncName4[50] = "";
				ULONG64 FuncAddr4 = 0;
				GetFuncName2(OneCmd4, FuncName4, FuncAddr4);
				//dprintf("0x%x\t%s\n", FuncAddr4 - FuncAddr, FuncName4);
				dprintf("  |     |___%s\n", FuncName4);
			}

			j += 4;
		}
	}
	else // case2 按照switch case 来解析
	{
		size_t MaxOffSet = NameSets[FuncName];
		size_t i = 0;
		while (i < MaxOffSet) {
			char OneCmd[100] = "";
			ULONG64 CmdAddr = FuncAddr + i;
			Disasm(&CmdAddr, OneCmd, 100);
			if (OneCmd[26] == 'c' && OneCmd[27] == 'm' && OneCmd[28] == 'p') {
				size_t index = 28;
				while (index < 50 && OneCmd[index] != '#') ++index; // 指向 #0x26 的 #
				char OffSetNumb[5] = { OneCmd[index + 1],OneCmd[index + 2],OneCmd[index + 3],OneCmd[index + 4], '\0' };
				i += 4;

				// 解析第二条汇编指令
				char OneCmd2[100] = "";
				ULONG64 CmdAddr2 = FuncAddr + i;
				Disasm(&CmdAddr2, OneCmd2, 100);
				char FuncName2[50] = "";
				ULONG64 FuncAddr2 = 0;
				GetFuncName2(OneCmd2, FuncName2, FuncAddr2);
				if ('x' != OffSetNumb[1])
					dprintf("  |     |___0x00 %s\n", FuncName2);
				else
					dprintf("  |     |___%s %s\n", OffSetNumb, FuncName2);

				// 解析第三条汇编指令
				char OneCmd3[100] = "";
				ULONG64 CmdAddr3 = FuncAddr + i + 4;
				Disasm(&CmdAddr3, OneCmd3, 100);
				if (OneCmd3[26] == 'b') {
					char FuncName3[50] = "";
					ULONG64 FuncAddr3 = 0;
					GetFuncName2(OneCmd3, FuncName3, FuncAddr3);
					dprintf("  |     |___default %s\n", FuncName3);
				}
			}

			i += 4;
		}
	}
	NameSets.erase(FuncName);
}


// interrupt vector table for ARMv8
DECLARE_API(ivt)
{
	unordered_map<string, size_t> NameSets;	// string表示表名，int最大偏移量

	// 插入哪个表可以按照 switch case 来解析
	NameSets.insert({ "lk!el1_sync", 0x100 });
	NameSets.insert({ "lk!el0_sync", 0x1c0 });

	// 插入哪个表可以按照 bl 和 b 来解析
	NameSets.insert({ "lk!__bad_stack", 0xc0 });
	NameSets.insert({ "lk!el1_irq", 0x100 });
	NameSets.insert({ "lk!el1_error", 0xc0 });
	NameSets.insert({ "lk!el0_irq", 0x1a8 });
	NameSets.insert({ "lk!el0_error", 0x16c });

	ULONG64 VBAR_EL1_ADDR, SymOffset;
	// 获取 lk!vectors 函数的地址，并把地址存储到 变量 VBAR_EL1_ADDR 中
	GetExpressionEx("lk!vectors", &VBAR_EL1_ADDR, NULL);
	dprintf("VBAR_EL1_ADDR: %p\n", VBAR_EL1_ADDR);

	unordered_map<ULONG64, string> addr_name_maps;	// ULONG64是函数地址，string是函数名

	// 一次循环处理一个向量表
	for (size_t i = 0; i < 16; i++) {
		ULONG64 TableAddr = VBAR_EL1_ADDR + i * 128;
		char TableName[40] = "";
		GetSymbol(TableAddr, TableName, &SymOffset);
		dprintf("%s\n", TableName);

		// 一次循环处理一条指令
		for (size_t j = 0; j < 32; j++) {
			char OneCmd[100] = "";
			ULONG64 CmdAddr = TableAddr + j * 4;
			Disasm(&CmdAddr, OneCmd, 100);

			// 如果这条指令是跳转指令
			if (OneCmd[26] == 'b' && (OneCmd[27] == ' ' || OneCmd[27] == '.' || OneCmd[27] == '\t')) {
				char FuncName[50] = "";
				ULONG64 FuncAddr = 0;

				// 如果这条指令是新的指令，则需要深层解析
				if (true == GetFuncName(OneCmd, addr_name_maps, FuncName, FuncAddr))
					HanderFunc(FuncName, FuncAddr, NameSets);
			}
		}
	}
}

DECLARE_API(idt)
{
	ULONG64 Address;
	ULONG ulGateSize;
	int Num, Count;
	char Parameter0[MAX_PATH], Parameter1[MAX_PATH];
	const char* table_name = NULL;

	Num = 0;
	Count = 0;
	ulGateSize = GetTypeSize("gate_struct");
	if (ulGateSize <= 0) {
		ivt(hCurrentProcess, hCurrentThread, dwCurrentPc, dwProcessor, args);
		return;
	}
	//取idt_table地址
	if (GetExpressionEx("lk!idt_table", &Address, &args) == FALSE) {
		dprintf("failed to get idt_table address\n");
		return;
	}

	if (args == NULL || args[0] == 'a') {
		dprintf("idt_table address = 0x%p\n", Address);
		while (Num < 256) {
			Num = ShowGateStruct(Address, Num);

			Address = Address + ulGateSize;
		}
	}

	if (args[0] == 'g') {
		while (Num < 5) {
			if (Num == 0) {
				table_name = "apic_idts";
			}
			else if (Num == 1) {
				table_name = "def_idts";
			}
			else if (Num == 2) {
				table_name = "early_idts";
			}
			else if (Num == 3) {
				table_name = "ist_idts";
			}
			else if (Num == 4) {
				table_name = "dbg_idts";
			}

			IdxShowTable(table_name);

			Num = Num + 1;
		}
	}
}
/*idt扩展命令结束*/
/*
.sympath D:\gedu_work\wucheng
.load ndx
.reload
!idt

sync
  |___ffff0808 lk!el1_sync
  |     |___0x25: lk!el1_da
  | 	|___0x21: lk!el1_ia
  | 	|___0x00: lk!xxxx
  |___ffff0808 lk!__bad_stack
  |___ffff0808 lk!el0_sync
		|___0x23: lk!xxx
		|___0x18: lk!xxx
syshkjh
  |___ffff0808 lk!__bad_stack
  |___ffff0808 lk!sdfgasg


*/
typedef unsigned long long ULONG64;
DECLARE_API(ndxplay)
{
	// GetExpressionEx ：传入函数名称，获取该函数的地址
	ULONG64 VBAR_EL1_ADDR = 0;
	GetExpressionEx("lk!vectors", &VBAR_EL1_ADDR, NULL);
	dprintf("VBAR_EL1_ADDR: %p\n", VBAR_EL1_ADDR);

	// GetSymbol ：根据传入的地址，获取函数名
	ULONG64 FuncAddr = 0xffffff8008081800, SymOffset;
	char FuncName[50] = "";
	GetSymbol(FuncAddr, FuncName, &SymOffset);

	// Disasm ：根据传入的地址，获取该地址处的一条汇编指令
	char Assembly[100] = "";
	ULONG64 Address = 0xffffff8008081800;
	Disasm(&Address, Assembly, 100);
}