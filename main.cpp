#include <ctime>
#include <iostream>
#include "Poco/NamedMutex.h"
#include "utils.h"

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

int masterTickets[MAX_ORDER_COUNT];

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
	int			magic;
	MqlString	symbol;   // 12 bytes
	int			type;
	double		lots;
	double		openprice;
	double		tpprice;
	double		slprice;
	__time64_t	expiration;
	MqlString	comment;   // 12 bytes
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
int ordersRCount = 0;

namespace ffc {

	void openROrder(MqlOrderAction* copy_order, FxcOrder* orders_client);
	void modifyROrder(MqlOrderAction* action_order, FxcOrder* order_master, int ticket);
	void closeOrder(MqlOrderAction* action_order, FxcOrder* orders_client);


	///Копируем строку с++ в MQLSting (на стороне MQL больше ничего делать не надо)
	inline void writeMqlString(MqlString dest, wchar_t* source) {
		int len = min(wcslen(source), dest.size - 1);  //Определяем длину строки (небольше распределенного буфера)
		wcscpy_s(dest.buffer, len + 1, source);  //Копируем включая терминирующий ноль
		*(((int*)dest.buffer) - 1) = len;  // Записываем длину строки (хак, может измениться в будующих версиях терминала)
	}

	Poco::NamedMutex mutex("ffc_mutex");

	bool is_init = false;

	int ffc_Init() {
		ordersRCount = 0;
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
		orders[ordersCount] = { OrderTicket, getMagic(OrderComment) , L"default", orderType, OrderLots, OrderOpenPrice, OrderOpenTime, OrderTakeProfit, OrderStopLoss, OrderClosePrice, OrderCloseTime, OrderExpiration, OrderProfit, OrderCommission, OrderSwap, L"" };

		wcscpy_s(orders[ordersCount].symbol, SYMBOL_LENGTH, OrderSymbol);

		wchar_t s2[20];
		_itow(OrderTicket, s2, 10);
		wcscpy_s(orders[ordersCount].comment, COMMENT_LENGTH, L"ffc_");
		wcscat(orders[ordersCount].comment, s2);

		std::wcout << "order #" << OrderTicket << " magic=" << orders[ordersCount].magic << " comment = " << orders[ordersCount].comment << "\r\n";
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

	int ffc_ROrdersUpdate(int ROrderTicket, int Rmagic, wchar_t* ROrderSymbol, int RorderType,
		double ROrderLots, double ROrderOpenPrice, __time64_t ROrderOpenTime,
		double ROrderTakeProfit, double ROrderStopLoss, double  ROrderClosePrice, __time64_t  ROrderCloseTime,
		__time64_t ROrderExpiration, double  ROrderProfit, double  ROrderCommission, double  ROrderSwap, wchar_t* ROrderComment) {

		orders_old[ordersRCount] = { ROrderTicket, Rmagic, L"default", RorderType, ROrderLots, ROrderOpenPrice, ROrderOpenTime, ROrderTakeProfit, ROrderStopLoss, ROrderClosePrice, ROrderCloseTime, ROrderExpiration, ROrderProfit, ROrderCommission, ROrderSwap, L"" };

		wcscpy_s(orders_old[ordersRCount].symbol, SYMBOL_LENGTH, ROrderSymbol);
		wcscpy_s(orders_old[ordersRCount].comment, COMMENT_LENGTH, ROrderComment);

		masterTickets[ordersRCount] = getMasterTicket(ROrderComment);

		std::wcout << "order #" << ordersRCount << " " << ROrderTicket << " " << orders_old[ordersRCount].comment << "\r\n";
		ordersRCount++;
		return ordersRCount;
	}

	int ffc_RGetJob(MqlOrderAction* action_array) {
		MqlOrderAction* action_order;
		int count = 0;
		if (transmitterBusy == true) {
			ordersRCount = 0;
			return count;
		}
		int top = 0;
		int i = 0;
		int k = 0; // корректировка индекса клиента
		// сравнение (orders_old, orders) и создание структуры с действиями
		while (i < ordersTotal) {
			action_order = action_array + count;
			auto orders_client = orders_old + k;
			auto orders_master = orders + i;
			if (orders_master->ticket == masterTickets[k]) { // тикет найден, сравниваем остальные данные
				if (orders[i].tpprice != orders_client->tpprice || orders[i].slprice != orders_client->slprice) { // тикет изменен
					count++;
					modifyROrder(action_order, orders_master, orders_client->ticket);
					std::wcout << "ticket find and is changed - " << orders_client->ticket << "\r\n";
				}
				else {
					std::wcout << "ticket is original - " << orders_client->ticket << "\r\n";
				}
			} else if (orders_master->ticket > masterTickets[k]) {
				if (masterTickets[k] != 0) { // закрываем тикет
					i--;
					count++;
					std::wcout << "ticket is not find (close ticket) - " << orders_client->ticket << "\r\n";
					closeOrder(action_order, orders_client);
				}
				else { // открываем тикет
					count++;
					std::wcout << "ticket is not find (ticket open) - " << orders_client->ticket << "\r\n";
					openROrder(action_order, orders_master);
				}
			} else { // вероятно, что тикет был закрыт вручную
				k--;
				//count++;
				//std::wcout << "ticket is not find (close ticket) - " << orders_client->ticket << "\r\n";
				//closeOrder(action_order, orders_client);
			}

			std::wcout << "orders_master->ticket - " << orders_master->ticket << " masterTickets - " << masterTickets[k] << " count client order - " << ordersRCount << "\r\n";
			k++;
			i++;
		}

		if ((ordersRCount - k) > 0) { // в том случае, если был закрыт последний тикет на мастере
			for (int f = k; f < ordersRCount; f++) {
				action_order = action_array + count;
				auto orders_client = orders_old + f;
				count++;
				std::wcout << "ticket is not find (close ticket) - " << orders_client->ticket << "\r\n";
				closeOrder(action_order, orders_client);
			}
		}

		if (ordersTotal == 0) { // в том случае, если были закрыты все тикеты
			for (int i = 0; i < ordersRCount; i++) {
				action_order = action_array + count;
				auto orders_client = orders_old + i;
				count++;
				std::wcout << "ticket is not find (close ticket) - " << orders_client->ticket << "\r\n";
				closeOrder(action_order, orders_client);
			}
		}
		
		ordersRCount = 0;
		return count;
	}

	void openROrder(MqlOrderAction* action_order, FxcOrder* order_master) {
		action_order->action = 1;
		action_order->ticket = order_master->ticket;
		action_order->magic = order_master->magic;
		action_order->type = order_master->type;
		action_order->lots = order_master->lots;
		action_order->openprice = order_master->openprice;

		writeMqlString(action_order->symbol, order_master->symbol);
		writeMqlString(action_order->comment, order_master->comment);
	}

	void modifyROrder(MqlOrderAction* action_order, FxcOrder* order_master, int ticket) {
		action_order->action = 3;
		action_order->ticket = ticket;
		action_order->magic = order_master->magic;
		action_order->type = order_master->type;
		action_order->lots = order_master->lots;
		action_order->slprice = order_master->slprice;
		action_order->tpprice = order_master->tpprice;
		action_order->openprice = order_master->openprice;

	}

	void closeOrder(MqlOrderAction* action_order, FxcOrder* orders_client) {
		action_order->action = 2;
		action_order->ticket = orders_client->ticket;
		action_order->type = orders_client->type;
		action_order->lots = orders_client->lots;
		action_order->openprice = orders_client->openprice;
		std::wcout << "orders close lots - " << action_order->lots << " ticket - " << action_order->ticket << "\r\n";
	}


}