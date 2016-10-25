//+------------------------------------------------------------------+
//|                                                  Transmitter.mq4 |
//|                        Copyright 2016, BlackSteel, FairForex.org |
//|                                            https://fairforex.org |
//+------------------------------------------------------------------+
#property copyright "Copyright 2016, BlackSteel, FairForex.org"
#property link      "https://fairforex.org"
#property version   "1.00"
#property strict


struct Order
{
	int			ticket;
	int			magic;
	string		symbol;
	int			type;
	double		lots;
	double		openprice;
	datetime	   opentime;
	double		tpprice;
	double		slprice;
	double		closeprice;
	datetime	   closetime;
	datetime	   expiration;
	double		profit;
	double		comission;
	double		swap;
	string		comment;
};

#import "testMQL4.dll"
int ffc_Init();
void ffc_DeInit();
/*
void ffc_ordersCount(int orders);
int ffc_OrderSelectError(int ticket);
int ffc_GetTicket();
void ffc_OrderUpdate(int OrderTicket, int orderMagic, string OrderSymbol, int orderType,
		double OrderLots, double OrderOpenPrice, datetime OrderOpenTime,
		double OrderTakeProfit, double OrderStopLoss, double  OrderClosePrice, datetime  OrderCloseTime,
		datetime OrderExpiration, double  OrderProfit, double  OrderCommission, double  OrderSwap, string OrderComment);
void ffc_validation(int orders);
*/
int ffc_UpdateMasterArray(Order& master_array[]);
int ffc_OrdersTotal();
string ffc_getSymbol(int index);
int ffc_getInt(int index);
#import
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

#define MAX_ORDER_COUNT 200

int totalOrders = 0;
Order master_orders[MAX_ORDER_COUNT];
  
int OnInit()
  {
    if (!EventSetMillisecondTimer(100)) return(INIT_FAILED);
      
   if (ffc_Init() != 1) { 
      Print("Повторный запуск!");
      return(INIT_FAILED);
   }
   
   for (int i=0; i<MAX_ORDER_COUNT; i++) {  //Выделяем память под строку (надо сделать один раз в начале)
      StringInit(master_orders[i].symbol, 16);  
      StringInit(master_orders[i].comment, 32);
   }

   return(INIT_SUCCEEDED);
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
   Print("Deinit");
   EventKillTimer();
   ffc_DeInit();
      
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Timer function                                                   |
//+------------------------------------------------------------------+
void OnTimer()
  {
   totalOrders = ffc_UpdateMasterArray(master_orders);
   Print("Total orders - ", totalOrders);
   for (int i=0; i<totalOrders; i++) {
      Print("order #", i, " ", master_orders[i].ticket, " (", master_orders[i].symbol, ")");
   }
   
   EventKillTimer();
  }
//+------------------------------------------------------------------+

