//+------------------------------------------------------------------+
//|                                                  Transmitter.mq4 |
//|                        Copyright 2016, BlackSteel, FairForex.org |
//|                                            https://fairforex.org |
//+------------------------------------------------------------------+
#property copyright "Copyright 2016, BlackSteel, FairForex.org"
#property link      "https://fairforex.org"
#property version   "1.00"
#property strict


struct OrderAction
{
	int			action;
	int			ticket;
	int			magic;
	string		symbol;
	int			type;
	double		lots;
	double		openprice;
	double		tpprice;
	double		slprice;
	datetime	   expiration;
	string		comment;
};

#import "testMQL4.dll"
int ffc_Init();
void ffc_DeInit();
int ffc_UpdateMasterArray(OrderAction& mql_order_action[]);
int ffc_OrdersTotal();
string ffc_getSymbol(int index);
int ffc_getInt(int index);
void ffc_ROrdersCount(int orders);
int ffc_RGetJob(OrderAction& mql_order_action[]);
int ffc_ROrdersUpdate(int OrderTicket, int orderMagic, string OrderSymbol, int orderType,
		double OrderLots, double OrderOpenPrice, datetime OrderOpenTime,
		double OrderTakeProfit, double OrderStopLoss, double  OrderClosePrice, datetime  OrderCloseTime,
		datetime OrderExpiration, double  OrderProfit, double  OrderCommission, double  OrderSwap, string OrderComment);
#import
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

#define MAX_ORDER_COUNT 200

int totalOrders = 0;
OrderAction mql_order_action[MAX_ORDER_COUNT];
  
int OnInit()
  {
    if (!EventSetMillisecondTimer(100)) return(INIT_FAILED);
      
   if (ffc_Init() != 1) { 
      Print("Повторный запуск!");
      return(INIT_FAILED);
   }
   
   for (int i=0; i<MAX_ORDER_COUNT; i++) {  //Выделяем память под строку (надо сделать один раз в начале)
      StringInit(mql_order_action[i].symbol, 16);  
      StringInit(mql_order_action[i].comment, 32);
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
  
   //EventKillTimer();
   /*
   totalOrders = ffc_UpdateMasterArray(mql_order_action);
   Print("Total orders - ", totalOrders);
   */
   int out = 0;
   int out2 = 0;
   int mode = 0;
   double modeClose;
   double ask_price   = 0;
   double bid_price = 0;
   int ordersTotal = OrdersTotal();
   int ordersCount = 0;
   bool res = false;
   int magicNumber = 0;
   Alert ("ordersTotal=",ordersTotal);
   while (ordersCount<ordersTotal) {
      if (OrderSelect(ordersCount, SELECT_BY_POS) && (magicNumber=OrderMagicNumber()) > 0) {
          ffc_ROrdersUpdate(OrderTicket(), magicNumber, OrderSymbol(), OrderType(), OrderLots(),
            OrderOpenPrice(), OrderOpenTime(), OrderTakeProfit(), OrderStopLoss(),
            OrderClosePrice(), OrderCloseTime(), OrderExpiration(),
            OrderProfit(), OrderCommission(), OrderSwap(), OrderComment());
            //Print("OrderComment = ",OrderComment()," - i= ",OrderTicket());
      }
      ordersCount++;
   }
   
   int action = 0;
   bool symbol_init;
   action = ffc_RGetJob(mql_order_action);
   if (action>0) {
      for (int i=0; i<action; i++) {
      modeClose = 0;
         switch (mql_order_action[i].type) {
            case 0:
            case 2:
            case 5:
               mode = MODE_ASK;
               modeClose = Bid;
               break;
            case 1:
            case 3:
            case 4:
               mode = MODE_BID;
               modeClose = Ask;
               break;
         }
         Alert ("index=",i," - action= ", action," - mql_action=",mql_order_action[i].action," - ticket=",mql_order_action[i].ticket);
         if (mql_order_action[i].action>0) {
         
            switch (mql_order_action[i].action) {
               case 1: // открытие нового ордера
                  ask_price   = MarketInfo(mql_order_action[i].symbol,mode); // Запрос текущей цены
                  if ((ask_price <= mql_order_action[i].openprice && mql_order_action[i].type==0) || (ask_price >= mql_order_action[i].openprice && mql_order_action[i].type==1)) {  // расписать для buy & sell ------------------- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                     if (ask_price == 0) {
                        symbol_init = SymbolSelect(mql_order_action[i].symbol, 1);
                     } else symbol_init = true;
                     
                     if (symbol_init) {
                        ask_price   = MarketInfo(mql_order_action[i].symbol,mode); // Запрос текущей цены
                        out = OrderSend(mql_order_action[i].symbol,mql_order_action[i].type,mql_order_action[i].lots,ask_price,3,0,0,mql_order_action[i].comment,mql_order_action[i].magic);
                     }
                  } else
                  Alert ("Цена не соответствует требованиям, текущая цена = ", ask_price, " - мастер-цена = ", mql_order_action[i].openprice);
                  //Alert (GetLastError());
                  break;
               case 2: // закрытие ордера
                  closeOrder(mql_order_action[i].type,mql_order_action[i].ticket,mql_order_action[i].lots);
                  break;
               case 3: // модификация ордера
                  res = OrderModify(mql_order_action[i].ticket,mql_order_action[i].openprice,mql_order_action[i].slprice,mql_order_action[i].tpprice,0,Blue);
                  if(!res)
                     Print("Error in OrderModify. Error code=",GetLastError());
                  else
                     Print("Order modified successfully.");
                     Alert (mql_order_action[i].ticket," - ",mql_order_action[i].openprice," - ",mql_order_action[i].slprice," - ",mql_order_action[i].tpprice);
                 break;
            }
         }
      }
   }
   
  }
  
  void closeOrder(int type, int Ticket, double Lot) { 
  double Price_Cls = 0;
  string Text;
   while(true)                                  // Цикл закрытия орд.
     {
      switch(type)                        // По типу ордера
        {
         case 0 :
            Price_Cls = Bid;          // Ордер Buy
            Text = "Buy ";                 // Текст для Buy
            break;                 // Из switch
         case 1: 
            Price_Cls = Ask;                 // Ордер Sell
            Text="Sell ";                       // Текст для Sell
        }
      Alert("Попытка закрыть ",Text," ",Ticket,". Ожидание ответа..");
      bool Ans=OrderClose(Ticket,Lot,Price_Cls,2);// Закрытие ордера
      //--------------------------------------------------------- 8 --
      if (Ans==true)                            // Получилось :)
        {
         Alert ("Закрыт ордер ",Text," ",Ticket);
         break;                                 // Выход из цикла закр
        }
      //--------------------------------------------------------- 9 --
      int Error=GetLastError();                 // Не получилось :(
      switch(Error)                             // Преодолимые ошибки
        {
         case 135:Alert("Цена изменилась. Пробуем ещё раз..");
            RefreshRates();                     // Обновим данные
            continue;                           // На след. итерацию
         case 136:Alert("Нет цен. Ждём новый тик..");
            while(RefreshRates()==false)        // До нового тика
               Sleep(1);                        // Задержка в цикле
            continue;                           // На след. итерацию
         case 146:Alert("Подсистема торговли занята. Пробуем ещё..");
            Sleep(500);                         // Простое решение
            RefreshRates();                     // Обновим данные
            continue;                           // На след. итерацию
                                     // На след. итерацию
        }
      switch(Error)                             // Критические ошибки
        {
         case 2 : Alert("Общая ошибка.");
            break;                              // Выход из switch
         case 5 : Alert("Старая версия клиентского терминала.");
            break;                              // Выход из switch
         case 64: Alert("Счет заблокирован.");
            break;                              // Выход из switch
         case 133:Alert("Торговля запрещена");
            break;                              // Выход из switch
         case 4108:Alert("Такого тикета не существует..");
            break;
         default: Alert("Возникла ошибка ",Error);//Другие варианты   
        }
      break;                                    // Выход из цикла закр
     }
   }
//+------------------------------------------------------------------+

