#include <ctime>
#include <iostream>

#include "Poco/NamedMutex.h"

#define SYMBOL_LENGTH	16
#define COMMENT_LENGTH	32
#define MAX_ORDER_COUNT	200

struct FxcOrder
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

//—труктура дл€ передачи строк
#pragma pack(push,1)
struct MqlString
{
	int    size;     // 4 bytes
	wchar_t* buffer;   // 4 bytes
	int    reserved; // 4 bytes
};
#pragma pack(pop,1)

//—труктура дл€ передачи ордеров
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
FxcOrder orders[MAX_ORDER_COUNT] = { 0 };
FxcOrder orders_old[MAX_ORDER_COUNT] = { 0 };
int ordersCount = 0;
int ordersTotal = 0;
bool ordersValid = false;
bool transmitterBusy = false;
#pragma data_seg()
#pragma comment(linker, "/SECTION:shared,RWS")

namespace fxc {
	/// опируем строку с++ в MQLSting (на стороне MQL больше ничего делать не надо)
	inline void writeMqlString(MqlString dest, wchar_t* source) {
		int len = min(wcslen(source), dest.size - 1);  //ќпредел€ем длину строки (небольше распределенного буфера)
		wcscpy_s(dest.buffer, len + 1, source);  // опируем включа€ терминирующий ноль
		*(((int*)dest.buffer) - 1) = len;  // «аписываем длину строки (хак, может изменитьс€ в будующих верси€х терминала)
	}


	//+------------------------------------------------------------------+
	//| ¬ текстовой строке замен€ем подстроку на подстроку               |
	//| string передаетс€ в виде пр€мой ссылки на контент строки         |
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
			if (AllocConsole()) {
				freopen("CONOUT$", "w", stdout);
				freopen("conout$", "w", stderr);
				SetConsoleOutputCP(CP_UTF8);// GetACP());
				SetConsoleCP(CP_UTF8);
				std::cout << "DLL inited.\r\n";
			}
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

		orders[ordersCount] = { OrderTicket, magic, L"default", orderType, OrderLots, OrderOpenPrice, OrderOpenTime, OrderTakeProfit, OrderStopLoss, OrderClosePrice, OrderCloseTime, OrderExpiration, OrderProfit, OrderCommission, OrderSwap, L"" };

		wcscpy_s(orders[ordersCount].symbol, SYMBOL_LENGTH, OrderSymbol);
		wcscpy_s(orders[ordersCount].comment, COMMENT_LENGTH, OrderComment);
		

		std::wcout << "order #" << ordersCount << " " << OrderTicket << " " << orders[ordersCount].symbol << "\r\n";
		ordersCount++;
	}

	int ffc_UpdateMasterArray(MqlOrder* master_array) {
		std::wcout << "ffc_UpdateMasterArray() orders count: " << ordersTotal << "\r\n";
		for (int i = 0; i < ordersTotal; i++) {
			MqlOrder* copy_order = master_array + i;
			FxcOrder* master_order = orders + i;
		
			copy_order->ticket		= master_order->ticket;
			copy_order->magic		= master_order->magic;
			copy_order->type		= master_order->type;
			copy_order->lots		= master_order->lots;
			copy_order->openprice	= master_order->openprice;
			copy_order->opentime	= master_order->opentime;
			copy_order->tpprice		= master_order->tpprice;
			copy_order->slprice		= master_order->slprice;
			copy_order->closeprice	= master_order->closeprice;
			copy_order->closetime	= master_order->closetime;
			copy_order->expiration	= master_order->expiration;
			copy_order->profit		= master_order->profit;
			copy_order->comission	= master_order->comission;
			copy_order->swap		= master_order->swap;

			writeMqlString(copy_order->symbol, master_order->symbol);
			writeMqlString(copy_order->comment, master_order->comment);

			std::wcout << "order #" << i << " " << master_order->ticket << " (" << master_order->symbol << "->" << copy_order->symbol.buffer << ")\r\n";
		}
		return ordersTotal;
	}

	int ffc_OrdersTotal() {
		std::wcout << "ffc_OrdersTotal: " << ordersTotal << "\r\n";
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
		std::cout << "ordersTotal - " << ordersTotal << "\r\n";
		mutex.unlock();
	}

	void ffc_validation(bool flag) {
		ordersValid = flag;
		std::cout << "Orders validation: " << ordersValid << "\r\n";
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