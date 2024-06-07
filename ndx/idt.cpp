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
		sscanf_s(high, "%llx", &Address);//��ȡ������ַ
		GetSymbol(Address, SymName, &SymOffset);
		dprintf("%p %s\n", Address, SymName);

		GetSymbol(Address, Test, &SymOffset);
		Num = Num + 1;

		return Num;
	}
	if (strcmp(SymName, Test) == 0) {//�жϺ����Ƿ��ظ�
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
		sscanf_s(high, "%llx", &Address);//��ȡ������ַ
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
void IdxShowTable(const char *table_name)//idx -gѭ��
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
/// @brief �Ѵ�СΪcount���ֽڵ�16�����ַ���ת��Ϊ����
/// @param str ��СΪcount�ֽڵ�16�����ַ��������� ffffffff80808080
/// @param count �����ַ������ֽ���
/// @return ת���õ�������
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

/// @brief ���ݴ���bָ���һ�л����룬��������ָ����ת���ĵ�ַ�ͺ���������������AddrSets����ȥ�أ�
/// @param OneCmd Ҫ�����Ļ��ָ��
/// @param addr_name ������ַΪkey��������Ϊvalue�ļ��ϣ�����ȥ��
/// @param FuncName ��ת���ĺ�����
/// @param FuncAddr ��ת���ĺ�����ַ
/// @return true--����������������ģ�fasle--���������֮ǰ������
static bool GetFuncName(const char* OneCmd, unordered_map<ULONG64, string>& addr_name, char* FuncName, ULONG64& FuncAddr)
{
	ULONG64 SymOffset;
	size_t index = 26;
	while (index < 40 && OneCmd[index] != '#') ++index; // ָ�� #0xffff �� #
	const char* FuncAddrStr = OneCmd + index + 3;

	FuncAddr = atoi_HEX(FuncAddrStr, 16);
	if (0 == addr_name.count(FuncAddr))		// ���ݵ�ַȥ��
	{
		GetSymbol(FuncAddr, FuncName, &SymOffset);
		addr_name.insert({ FuncAddr, FuncName });

		dprintf("  |___%p %s\n", FuncAddr, FuncName);
		return true;	// �����������µģ�
	}
	else {
		dprintf("  |___%p %s\n", FuncAddr, addr_name[FuncAddr].c_str());
	}
	return false;		// �������Ǿɵ�
}


/// @brief ���ݴ���bָ���һ�л����룬��������ָ����ת���ĵ�ַ�ͺ�����
/// @param OneCmd Ҫ�����Ļ��ָ��
/// @param FuncName ��ת���ĺ�����
/// @param FuncAddr ��ת���ĺ�����ַ
static void GetFuncName2(const char* OneCmd, char* FuncName, ULONG64& FuncAddr)
{
	//dprintf("%s", OneCmd);
	ULONG64 SymOffset;
	size_t index = 26;
	while (index < 40 && OneCmd[index] != '#') ++index; // ָ�� #0xffff �� #
	const char* FuncAddrStr = OneCmd + index + 3;

	FuncAddr = atoi_HEX(FuncAddrStr, 16);
	GetSymbol(FuncAddr, FuncName, &SymOffset);
}


/// @brief ���ݺ������ƺͺ�����ַ��������ǰ��������һ�㺯��
/// @param FuncName ������
/// @param FuncAddr ������ַ
/// @param NameSets ���溯�����ļ��ϣ�ֻ�����ü����еĺ���
static void HanderFunc(const char* FuncName, ULONG64 FuncAddr, unordered_map<string, size_t>& NameSets)
{
	if (0 == NameSets.count(FuncName)) return;

	// case1 ����bl������
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
	else // case2 ����switch case ������
	{
		size_t MaxOffSet = NameSets[FuncName];
		size_t i = 0;
		while (i < MaxOffSet) {
			char OneCmd[100] = "";
			ULONG64 CmdAddr = FuncAddr + i;
			Disasm(&CmdAddr, OneCmd, 100);
			if (OneCmd[26] == 'c' && OneCmd[27] == 'm' && OneCmd[28] == 'p') {
				size_t index = 28;
				while (index < 50 && OneCmd[index] != '#') ++index; // ָ�� #0x26 �� #
				char OffSetNumb[5] = { OneCmd[index + 1],OneCmd[index + 2],OneCmd[index + 3],OneCmd[index + 4], '\0' };
				i += 4;

				// �����ڶ������ָ��
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

				// �������������ָ��
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
	unordered_map<string, size_t> NameSets;	// string��ʾ������int���ƫ����

	// �����ĸ�����԰��� switch case ������
	NameSets.insert({ "lk!el1_sync", 0x100 });
	NameSets.insert({ "lk!el0_sync", 0x1c0 });

	// �����ĸ�����԰��� bl �� b ������
	NameSets.insert({ "lk!__bad_stack", 0xc0 });
	NameSets.insert({ "lk!el1_irq", 0x100 });
	NameSets.insert({ "lk!el1_error", 0xc0 });
	NameSets.insert({ "lk!el0_irq", 0x1a8 });
	NameSets.insert({ "lk!el0_error", 0x16c });

	ULONG64 VBAR_EL1_ADDR, SymOffset;
	// ��ȡ lk!vectors �����ĵ�ַ�����ѵ�ַ�洢�� ���� VBAR_EL1_ADDR ��
	GetExpressionEx("lk!vectors", &VBAR_EL1_ADDR, NULL);
	dprintf("VBAR_EL1_ADDR: %p\n", VBAR_EL1_ADDR);

	unordered_map<ULONG64, string> addr_name_maps;	// ULONG64�Ǻ�����ַ��string�Ǻ�����

	// һ��ѭ������һ��������
	for (size_t i = 0; i < 16; i++) {
		ULONG64 TableAddr = VBAR_EL1_ADDR + i * 128;
		char TableName[40] = "";
		GetSymbol(TableAddr, TableName, &SymOffset);
		dprintf("%s\n", TableName);

		// һ��ѭ������һ��ָ��
		for (size_t j = 0; j < 32; j++) {
			char OneCmd[100] = "";
			ULONG64 CmdAddr = TableAddr + j * 4;
			Disasm(&CmdAddr, OneCmd, 100);

			// �������ָ������תָ��
			if (OneCmd[26] == 'b' && (OneCmd[27] == ' ' || OneCmd[27] == '.' || OneCmd[27] == '\t')) {
				char FuncName[50] = "";
				ULONG64 FuncAddr = 0;

				// �������ָ�����µ�ָ�����Ҫ������
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
	//ȡidt_table��ַ
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
/*idt��չ�������*/
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
	// GetExpressionEx �����뺯�����ƣ���ȡ�ú����ĵ�ַ
	ULONG64 VBAR_EL1_ADDR = 0;
	GetExpressionEx("lk!vectors", &VBAR_EL1_ADDR, NULL);
	dprintf("VBAR_EL1_ADDR: %p\n", VBAR_EL1_ADDR);

	// GetSymbol �����ݴ���ĵ�ַ����ȡ������
	ULONG64 FuncAddr = 0xffffff8008081800, SymOffset;
	char FuncName[50] = "";
	GetSymbol(FuncAddr, FuncName, &SymOffset);

	// Disasm �����ݴ���ĵ�ַ����ȡ�õ�ַ����һ�����ָ��
	char Assembly[100] = "";
	ULONG64 Address = 0xffffff8008081800;
	Disasm(&Address, Assembly, 100);
}