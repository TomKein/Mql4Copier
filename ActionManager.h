#pragma once
#include "windows.h"
#include <string>
#include "ffcTypes.h"

#define JOB_EXIT        0
#define JOB_CREATE      1
#define JOB_MODIFY      2
#define JOB_DELETE      3
#define JOB_CLOSE       4
#define JOB_PRINT_ORDER 5
#define JOB_PRINT_TEXT  6
#define JOB_DRAW_ORDER  7
#define JOB_SHOW_VALUE  8
#define JOB_MSG_BOX     9


namespace ffc {
	//Структура для передачи действий
#pragma pack(push,1)
	struct MqlAction			// 80 bytes
	{
		int         actionId;	// 4 bytes
		int         ticket;		// 4 bytes
		int         type;		// 4 bytes
		int         magic;		// 4 bytes
		MqlString	symbol;		// 12 bytes
		double      lots;		// 8 bytes
		double      openprice;	// 8 bytes
		double      tpprice;	// 8 bytes
		double      slprice;	// 8 bytes
		__time64_t	expiration;	// 8 bytes
		MqlString	comment;	// 12 bytes
	};
#pragma pack(pop,1)

	int actionsCount = 0;
	int actionsMaxCount = 0;
	MqlAction* actions = null;

	void initActions(MqlAction* arrayPtr, int length);

	void resetActions();

	void createOrder(wchar_t* symbol, int type, double lots, double openPrice, double slPrice, double tpPrice, wchar_t* comment = L"", int cfgMagic = 0);
	void createOrder(FfcOrder* order);
	void modOrder(int ticket, double openprice, double slprice, double tpprice);
	void deleteOrder(int ticket);
	void closeOrder(int ticket, double lots, double openprice);
	void closeOrder(FfcOrder* order);


	void showValue(int line, wchar_t* value);

}