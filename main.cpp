#include <ctime>
#include <iostream>

#include "Poco/NamedMutex.h"

struct FxcOrder
{
	int			ticket;
	int			magic;
	wchar_t		symbol[16];
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
	wchar_t		comment[32];
};

//Структура для передачи строк
#pragma pack(push,1)
struct MqlString
{
	int    size;     // 4 bytes
	wchar_t* buffer;   // 4 bytes
	int    reserved; // 4 bytes
};
#pragma pack(pop,1)

//Структура для передачи ордеров
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

MqlOrder actList[64];


#pragma data_seg ("shared")
FxcOrder orders[200] = { 0 };
FxcOrder orders_old[200] = { 0 };
int ordersCount = 0;
int ordersTotal = 0;
bool ordersValid = false;
bool transmitterBusy = false;
#pragma data_seg()
#pragma comment(linker, "/SECTION:shared,RWS")

namespace fxc {


	//+------------------------------------------------------------------+
	//| В текстовой строке заменяем подстроку на подстроку               |
	//| string передается в виде прямой ссылки на контент строки         |
	//+------------------------------------------------------------------+
	void fnReplaceString(wchar_t *text, wchar_t *from)
	{
		//--- проверка параметров
		if (text == NULL || from == NULL) return;
		//--- копирование
		memcpy(text, from, wcslen(from)*sizeof(wchar_t));
	}

	Poco::NamedMutex mutex("ffc_mutex");

	bool is_init = false;

	int ffc_Init() {
		if (!is_init) {
			is_init = true; 
			return 1;
		}
		return 0;
	}

	void ffc_OrderUpdate(int OrderTicket, int magic, wchar_t* OrderSymbol, int orderType,
		double OrderLots, double OrderOpenPrice, __time64_t OrderOpenTime,
		double OrderTakeProfit, double OrderStopLoss, double  OrderClosePrice, __time64_t  OrderCloseTime,
		__time64_t OrderExpiration, double  OrderProfit, double  OrderCommission, double  OrderSwap, wchar_t* OrderComment) {

		//memcpy(orders_old, orders, sizeof(orders));

		orders[ordersCount] = { OrderTicket, magic, L"", orderType, OrderLots, OrderOpenPrice, OrderOpenTime, OrderTakeProfit, OrderStopLoss, OrderClosePrice, OrderCloseTime, OrderExpiration, OrderProfit, OrderCommission, OrderSwap, L"" };

		wcscpy_s(orders[ordersCount].symbol, OrderSymbol);
		wcscpy_s(orders[ordersCount].comment, OrderComment);
		ordersCount++;

		std::cout << "order[] - " << orders << "\r\n";
	}

	void ffc_setOrderInfo(MqlOrder* pntr, int _index) {
		if (AllocConsole()) {
			freopen("CONOUT$", "w", stdout);
			freopen("conout$", "w", stderr);
			SetConsoleOutputCP(CP_UTF8);// GetACP());
			SetConsoleCP(CP_UTF8);
		}

		//char* work = (orders + 1)->symbol;

		std::cout << "Hi - " << (orders + _index)->comment << "\r\n";
		//printf(work);

		size_t s1 = sizeof((orders + _index)->comment);
		std::cout << "Hi2 - " << s1 << "\r\n";

		//pntr[_index] = { (orders+_index)->ticket, (orders + _index)->magic, 0, (orders + _index)->type, (orders + _index)->lots,(orders + _index)->openprice,(orders + _index)->opentime,(orders + _index)->tpprice,(orders + _index)->slprice,(orders + _index)->closeprice,(orders + _index)->closetime,(orders + _index)->expiration,(orders + _index)->profit,(orders + _index)->comission,(orders + _index)->swap, 0};
		pntr->ticket	=	(orders + _index)->ticket;
		pntr->magic = (orders + _index)->magic;
		//fnReplaceString(pntr->symbol, (orders + _index)->symbol);
		//wcscpy_s(pntr->symbol.buffer, s1, (orders + _index)->symbol);
		pntr->type		=	(orders + _index)->type;
		pntr->lots		=	(orders + _index)->lots;
		pntr->openprice	=	(orders + _index)->openprice;
		pntr->opentime	=	(orders + _index)->opentime;
		pntr->tpprice	=	(orders + _index)->tpprice;
		pntr->slprice	=	(orders + _index)->slprice;
		pntr->closeprice =	(orders + _index)->closeprice;
		pntr->closetime =	(orders + _index)->closetime;
		pntr->expiration =	(orders + _index)->expiration;
		pntr->profit	=	(orders + _index)->profit;
		pntr->comission =	(orders + _index)->comission;
		pntr->swap = (orders + _index)->swap;
		//fnReplaceString(pntr->comment, (orders + _index)->comment);

		//wcscpy_s(pntr->comment.buffer, pntr->comment.size, (orders + _index)->comment);


	}

	int ffc_OrdersTotal() {
		return ordersTotal;
	}

	int ffc_GetOrderInfo() {
		MqlOrder mqlGetOrder[1] = { orders[ordersCount].ticket, orders[ordersCount].magic };
		return 1;
	}




	void ffc_DeInit() {
		is_init = false;
	}

	void ffc_ordersCount(int num) {
		mutex.lock();
		transmitterBusy = true;
		ordersCount = 0;
		ordersTotal = num;
		//std::cout << "ordersTotal - " << ordersTotal << "\r\n";
		mutex.unlock();
	}

	void ffc_validation(bool flag) {
		ordersValid = flag;
	}

	int ffc_GetTicket() { // выдаем тикет(ticket_id), о котором нам хотелось бы узнать
		transmitterBusy = false;
		return 0;
	}

	int ffc_OrderSelectError(int Ticket) {
		return Ticket;
	}

	int ffc_EndSession() {
		return true;
	}
}