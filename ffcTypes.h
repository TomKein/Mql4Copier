#pragma once
#include "windows.h"

#define SYMBOL_LENGTH	16
#define COMMENT_LENGTH	32

//��������� ��� �������� �����
#pragma pack(push,1)
struct MqlString
{
	int    size;     // 4 bytes ������ ������ ������, �� ������
	wchar_t* buffer;   // 4 bytes ����� ������ ������, �� ������
	//����� ������ �������� �� ������ *(((int*)buffer) - 1)
	int    reserved; // 4 bytes
};
#pragma pack(pop,1)

//��������� ��� �������� �������
#pragma pack(push,1)
struct MqlOrder
{
	int			ticket;
	int			magic;
	MqlString	symbol;   // 12 bytes
	int			type;
	double		lots;
	double		openprice;
	__time64_t	opentime;
	double		tpprice;
	double		slprice;
	double		closeprice;
	__time64_t	closetime;
	__time64_t	expiration;
	double		profit;
	double		comission;
	double		swap;
	MqlString	comment;   // 12 bytes
};
#pragma pack(pop,1)

struct FfcOrder
{
	int			ticket;
	int			magic;
	wchar_t		symbol[SYMBOL_LENGTH];
	int			type;
	double		lots;
	double		openprice;
	__time64_t	opentime;
	double		tpprice;
	double		slprice;
	double		closeprice;
	__time64_t	closetime;
	__time64_t	expiration;
	double		profit;
	double		comission;
	double		swap;
	wchar_t		comment[COMMENT_LENGTH];
};