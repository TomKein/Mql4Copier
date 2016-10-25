//+------------------------------------------------------------------+
//|                                                  Transmitter.mq4 |
//|                        Copyright 2016, BlackSteel, FairForex.org |
//|                                            https://fairforex.org |
//+------------------------------------------------------------------+
#property copyright "Copyright 2016, BlackSteel, FairForex.org"
#property link      "https://fairforex.org"
#property version   "1.00"
#property strict

#import "testMQL4.dll"
int ffc_Init();
void ffc_DeInit();
void ffc_ordersCount(int orders);
int ffc_OrderSelectError(int ticket);
int ffc_GetTicket();
void ffc_OrderUpdate(int OrderTicket, int orderMagic, string OrderSymbol, int orderType,
		double OrderLots, double OrderOpenPrice, datetime OrderOpenTime,
		double OrderTakeProfit, double OrderStopLoss, double  OrderClosePrice, datetime  OrderCloseTime,
		datetime OrderExpiration, double  OrderProfit, double  OrderCommission, double  OrderSwap, string OrderComment);
void ffc_validation(int orders);
void fnReplaceString(string &text,string from,string to);
#import
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
  {
//--- create timer
   if (!EventSetMillisecondTimer(100)) return(INIT_FAILED);
      
   if (ffc_Init() != 1) return(INIT_FAILED);
   
  //ffc_OrderUpdate(5961235,563,"1",0,0,0,0,0,0,0,0,0,0,0,5,"q");
  //ffc_OrderUpdate(5783,563,"1",0,0,0,0,0,0,0,0,0,0,0,5,"dkla");
//---
   
   return(INIT_SUCCEEDED);
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
  ffc_DeInit();
//--- destroy timer
   EventKillTimer();
      
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
     int ordersCount = OrdersTotal();
     ffc_ordersCount(ordersCount);
     printf(IntegerToString(ordersCount));
      for (int i = 0; i<ordersCount; i++) {
         if (OrderSelect(i, SELECT_BY_POS)) {
             ffc_OrderUpdate(OrderTicket(), OrderMagicNumber(), OrderSymbol(), OrderType(), OrderLots(),
               OrderOpenPrice(), OrderOpenTime(), OrderTakeProfit(), OrderStopLoss(),
               OrderClosePrice(), OrderCloseTime(), OrderExpiration(),
               OrderProfit(), OrderCommission(), OrderSwap(), OrderComment());
         } 
      }
      ffc_validation(ordersCount == OrdersTotal());
      int ticket = 0;
      while ((ticket=ffc_GetTicket())>0) {
         if (OrderSelect(ticket, SELECT_BY_TICKET)) {
             ffc_OrderUpdate(ticket, OrderMagicNumber(), OrderSymbol(), OrderType(), OrderLots(),
               OrderOpenPrice(), OrderOpenTime(), OrderTakeProfit(), OrderStopLoss(),
               OrderClosePrice(), OrderCloseTime(), OrderExpiration(),
               OrderProfit(), OrderCommission(), OrderSwap(), OrderComment());
         } else {
            printf("ticket_id = " + IntegerToString(ffc_OrderSelectError(ffc_GetTicket())));
         }
      }
      EventKillTimer();
  }
//+------------------------------------------------------------------+
