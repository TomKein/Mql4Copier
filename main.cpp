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
FxcOrder orders[MAX_ORDER_COUNT] = { 0 }; //надо бы отрефакторить в master_orders, а то уже неудобно
int ordersCount = 0;
int ordersTotal = 0;
bool ordersValid = false;
bool transmitterBusy = false;
#pragma data_seg()
#pragma comment(linker, "/SECTION:shared,RWS")

FxcOrder orders_old[MAX_ORDER_COUNT] = { 0 };
int ordersRCount = 0;

namespace ffc {

	void openROrder(FxcOrder* orders_client);
	void modifyROrder(FxcOrder* order_master, int ticket);
	void closeOrder(FxcOrder* orders_client);


	/// опируем строку с++ в MQLSting (на стороне MQL больше ничего делать не надо)
	inline void writeMqlString(MqlString dest, wchar_t* source) {
		int len = min(wcslen(source), dest.size - 1);  //ќпредел€ем длину строки (небольше распределенного буфера)
		wcscpy_s(dest.buffer, len + 1, source);  // опируем включа€ терминирующий ноль
		*(((int*)dest.buffer) - 1) = len;  // «аписываем длину строки (хак, может изменитьс€ в будующих верси€х терминала)
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

	
	//---------------------------------------- Alex Way --------------------------------------------
	MqlOrderAction* actions = {0};
	int actionsCount = 0;
	void ffc_InitActions(MqlOrderAction* action_array) {
		actions = action_array;
		actionsCount = 0;
		//To-do: ¬озможно надо перед присваиванием проверить на null, чтобы избежать случайного переопределени€, тогда в деините надо будет обнул€ть null ом
		//ј возможно и не надо, вреда от переопределени€ меньше чем от не переопределени€
	}

	int ffc_RGetJob() {
		if (transmitterBusy == true) {
			ordersRCount = 0;  //ѕока не разобралс€, зачем это
			return 0;
		}
		int master_index = 0;
		int client_index = 0; 
		while (master_index < ordersTotal) {
			auto master_order = orders + master_index;
			if (client_index >= ordersRCount) {  //если на клиенте нет ордеров, то открываем.       € бы не наде€лс€ не терминацию нулем, все же количество надежнее
				openROrder(master_order);  //функци€ еще не написана, также нет смысла управл€ть списком акшинов извне, пусть эти функции сами с ними развлекаютс€
				master_index++;   //был бы цикл for, этого бы не пришлось писать :)
				continue;  //ƒл€ простоты понимани€, цепочки выполнени€ должны быть максимально короткими, после этого дальше просматривать код не нужно
			}
			auto client_order = orders_old + client_index;
			
			if (master_order->ticket == masterTickets[client_index]) { // тикет найден, провер€ем модификацию
				if (master_order->tpprice != client_order->tpprice || master_order->slprice != client_order->slprice) { // тикет изменен.      ---?? ¬ начали убрали индексирование, а тут нет, исправил
					modifyROrder(master_order, client_order->ticket);
					std::wcout << "ticket find and is changed - " << client_order->ticket << "\r\n";
				}
				else {
					std::wcout << "ticket is original - " << client_order->ticket << "\r\n";
				}
				client_index++;
				master_index++;
				continue;
			}
			if (master_order->ticket > masterTickets[client_index]) { // закрылс€ ордер на мастере, закрываем на клиенте
				std::wcout << "ticket is not find (close ticket) - " << client_order->ticket << "\r\n";
				closeOrder(client_order);
				client_index++;
				continue;
			}
			//“ут остались случаи ручного закрыти€ ордеров на клиенте
			master_index++;
		}
		for (; client_index < ordersRCount; client_index++) {
			auto client_order = orders_old + client_index;
			std::wcout << "ticket is not find (close ticket) - " << client_order->ticket << "\r\n";
			closeOrder(client_order);
		}
		ordersRCount = 0;
		return actionsCount;
	}
//--------------------------------------------------------------------------------------------
	void openROrder(FxcOrder* order_master) {
		actions->action = 1;
		actions->ticket = order_master->ticket;
		actions->magic = order_master->magic;
		actions->type = order_master->type;
		actions->lots = order_master->lots;
		actions->openprice = order_master->openprice;

		writeMqlString(actions->symbol, order_master->symbol);
		writeMqlString(actions->comment, order_master->comment);

		actionsCount++;
	}

	void modifyROrder(FxcOrder* order_master, int ticket) {
		actions->action = 3;
		actions->ticket = ticket;
		actions->slprice = order_master->slprice;
		actions->tpprice = order_master->tpprice;
		actionsCount++;
	}

	void closeOrder(FxcOrder* orders_client) {
		actions->action = 2;
		actions->ticket = orders_client->ticket;
		actions->type = orders_client->type;
		actions->lots = orders_client->lots;
		actions->openprice = orders_client->openprice;

		writeMqlString(actions->symbol, orders_client->symbol);
		std::wcout << "orders close lots - " << actions->lots << " ticket - " << actions->ticket << "\r\n";
		actionsCount++;
	}


}