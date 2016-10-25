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

struct MqlOrderAction
{
	int			action;
	int			ticket;
};

#pragma data_seg ("shared")
FxcOrder orders[MAX_ORDER_COUNT] = { 0 };
int ordersCount = 0;
int ordersTotal = 0;
bool ordersValid = false;
bool transmitterBusy = false;
#pragma data_seg()
#pragma comment(linker, "/SECTION:shared,RWS")

FxcOrder orders_old[MAX_ORDER_COUNT] = { 0 };
MqlOrderAction actionOrder[MAX_ORDER_COUNT] = { 0 };
int ordersRCount = 0;

namespace fxc {
	///Копируем строку с++ в MQLSting (на стороне MQL больше ничего делать не надо)
	inline void writeMqlString(MqlString dest, wchar_t* source) {
		int len = min(wcslen(source), dest.size - 1);  //Определяем длину строки (небольше распределенного буфера)
		wcscpy_s(dest.buffer, len + 1, source);  //Копируем включая терминирующий ноль
		*(((int*)dest.buffer) - 1) = len;  // Записываем длину строки (хак, может измениться в будующих версиях терминала)
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

			copy_order->ticket = master_order->ticket;
			copy_order->magic = master_order->magic;
			copy_order->type = master_order->type;
			copy_order->lots = master_order->lots;
			copy_order->openprice = master_order->openprice;
			copy_order->opentime = master_order->opentime;
			copy_order->tpprice = master_order->tpprice;
			copy_order->slprice = master_order->slprice;
			copy_order->closeprice = master_order->closeprice;
			copy_order->closetime = master_order->closetime;
			copy_order->expiration = master_order->expiration;
			copy_order->profit = master_order->profit;
			copy_order->comission = master_order->comission;
			copy_order->swap = master_order->swap;

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

	void ffc_DeInit() {
		is_init = false;
	}



	// ------------------------------------------------------------------- //
	// ----------------------- Receiver Part ----------------------------- //
	// ------------------------------------------------------------------- //

	int ffc_ROrdersUpdate(int ROrderTotal, int ROrderTicket, int Rmagic, wchar_t* ROrderSymbol, int RorderType,
		double ROrderLots, double ROrderOpenPrice, __time64_t ROrderOpenTime,
		double ROrderTakeProfit, double ROrderStopLoss, double  ROrderClosePrice, __time64_t  ROrderCloseTime,
		__time64_t ROrderExpiration, double  ROrderProfit, double  ROrderCommission, double  ROrderSwap, wchar_t* ROrderComment) {

		orders_old[ROrderTotal] = { ROrderTicket, Rmagic, L"default", RorderType, ROrderLots, ROrderOpenPrice, ROrderOpenTime, ROrderTakeProfit, ROrderStopLoss, ROrderClosePrice, ROrderCloseTime, ROrderExpiration, ROrderProfit, ROrderCommission, ROrderSwap, L"" };

		wcscpy_s(orders_old[ROrderTotal].symbol, SYMBOL_LENGTH, ROrderSymbol);
		wcscpy_s(orders_old[ROrderTotal].comment, COMMENT_LENGTH, ROrderComment);

		std::wcout << "order #" << ROrderTotal << " " << ROrderTicket << " " << orders_old[ROrderTotal].symbol << "\r\n";
		ordersRCount = ROrderTotal;
		return orders_old[ROrderTotal].ticket;
	}

	void parseComment() {

	}

	int ffc_CompareOrders() {
		int count = 0;
		// сравнение (orders_old, orders) и создание структуры с действиями
		for (int i = 0; i < ordersTotal; i++) {
			parseComment();
			if (orders[i].ticket == orders_old[i].ticket) { // тикет найден, сравниваем остальные данные
				if (orders[i].tpprice != orders_old[i].tpprice || orders[i].slprice != orders_old[i].slprice) { // тикет изменен
					count++;
					std::wcout << "ticket find and is changed - " << orders_old[i].ticket << "\r\n";
				}
				else {
					std::wcout << "ticket is original - " << orders_old[i].ticket << "\r\n";
				}
			} else if (orders[i].ticket < orders_old[i].ticket) { // тикет закрыть
				count++;
				std::wcout << "ticket is not find (close ticket) - " << orders_old[i].ticket << "\r\n";
				actionOrder[count].action = 1;
			} else { // тикет открыт вручную
				std::wcout << "ticket is not find (ticket open) - " << orders_old[i].ticket << "\r\n";
				actionOrder[count].action = 1;
			}
		}
		return count;
	}


}