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
      Print("��������� ������!");
      return(INIT_FAILED);
   }
   
   for (int i=0; i<MAX_ORDER_COUNT; i++) {  //�������� ������ ��� ������ (���� ������� ���� ��� � ������)
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
               case 1: // �������� ������ ������
                  ask_price   = MarketInfo(mql_order_action[i].symbol,mode); // ������ ������� ����
                  if ((ask_price <= mql_order_action[i].openprice && mql_order_action[i].type==0) || (ask_price >= mql_order_action[i].openprice && mql_order_action[i].type==1)) {  // ��������� ��� buy & sell ------------------- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                     if (ask_price == 0) {
                        symbol_init = SymbolSelect(mql_order_action[i].symbol, 1);
                     } else symbol_init = true;
                     
                     if (symbol_init) {
                        ask_price   = MarketInfo(mql_order_action[i].symbol,mode); // ������ ������� ����
                        out = OrderSend(mql_order_action[i].symbol,mql_order_action[i].type,mql_order_action[i].lots,ask_price,3,0,0,mql_order_action[i].comment,mql_order_action[i].magic);
                     }
                  } else
                  Alert ("���� �� ������������� �����������, ������� ���� = ", ask_price, " - ������-���� = ", mql_order_action[i].openprice);
                  //Alert (GetLastError());
                  break;
               case 2: // �������� ������
                  closeOrder(mql_order_action[i].type,mql_order_action[i].ticket,mql_order_action[i].lots);
                  break;
               case 3: // ����������� ������
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
   while(true)                                  // ���� �������� ���.
     {
      switch(type)                        // �� ���� ������
        {
         case 0 :
            Price_Cls = Bid;          // ����� Buy
            Text = "Buy ";                 // ����� ��� Buy
            break;                 // �� switch
         case 1: 
            Price_Cls = Ask;                 // ����� Sell
            Text="Sell ";                       // ����� ��� Sell
        }
      Alert("������� ������� ",Text," ",Ticket,". �������� ������..");
      bool Ans=OrderClose(Ticket,Lot,Price_Cls,2);// �������� ������
      //--------------------------------------------------------- 8 --
      if (Ans==true)                            // ���������� :)
        {
         Alert ("������ ����� ",Text," ",Ticket);
         break;                                 // ����� �� ����� ����
        }
      //--------------------------------------------------------- 9 --
      int Error=GetLastError();                 // �� ���������� :(
      switch(Error)                             // ����������� ������
        {
         case 135:Alert("���� ����������. ������� ��� ���..");
            RefreshRates();                     // ������� ������
            continue;                           // �� ����. ��������
         case 136:Alert("��� ���. ��� ����� ���..");
            while(RefreshRates()==false)        // �� ������ ����
               Sleep(1);                        // �������� � �����
            continue;                           // �� ����. ��������
         case 146:Alert("���������� �������� ������. ������� ���..");
            Sleep(500);                         // ������� �������
            RefreshRates();                     // ������� ������
            continue;                           // �� ����. ��������
                                     // �� ����. ��������
        }
      switch(Error)                             // ����������� ������
        {
         case 2 : Alert("����� ������.");
            break;                              // ����� �� switch
         case 5 : Alert("������ ������ ����������� ���������.");
            break;                              // ����� �� switch
         case 64: Alert("���� ������������.");
            break;                              // ����� �� switch
         case 133:Alert("�������� ���������");
            break;                              // ����� �� switch
         case 4108:Alert("������ ������ �� ����������..");
            break;
         default: Alert("�������� ������ ",Error);//������ ��������   
        }
      break;                                    // ����� �� ����� ����
     }
   }
//+------------------------------------------------------------------+

