#include "ActionManager.h"
#include "utils.h"

void ffc::initActions(MqlAction* arrayPtr, int length)
{
	actions = arrayPtr;
	actionsMaxCount = length;
}

void ffc::resetActions() {
	actionsCount = 0;
}


void ffc::createOrder(wchar_t* symbol, int type, double lots, double openPrice, double slPrice, double tpPrice, wchar_t* comment, int stMagic) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId	= JOB_CREATE;
	action->ticket		= 0;
	action->type		= type;
	action->magic		= stMagic;
	action->lots		= lots;
	action->openprice	= openPrice;
	action->slprice		= slPrice;
	action->tpprice		= tpPrice;
	action->expiration	= 0;

	writeMqlString(action->symbol, symbol);
	writeMqlString(action->comment, comment);
}
void ffc::createOrder(FfcOrder* order) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId	= JOB_CREATE;
	action->ticket		= 0;
	action->type		= order->type;
	action->magic		= order->magic;
	action->lots		= order->lots;
	action->openprice	= order->openprice;
	action->slprice		= order->slprice;
	action->tpprice		= order->tpprice;
	action->expiration	= 0;

	writeMqlString(action->symbol, order->symbol);
	writeMqlString(action->comment, order->comment);
}

void ffc::modOrder(int ticket, double openprice, double slprice, double tpprice) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId = JOB_MODIFY;
	action->ticket = ticket;
	action->type = 0;
	action->magic = 0;
	action->lots = 0;
	action->openprice = openprice;
	action->slprice = slprice;
	action->tpprice = tpprice;
	action->expiration = 0;

	writeMqlString(action->symbol, L"");
	writeMqlString(action->comment, L"");
}

void ffc::deleteOrder(int ticket) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId = JOB_DELETE;
	action->ticket = ticket;
	action->type = 0;
	action->magic = 0;
	action->lots = 0;
	action->openprice = 0;
	action->slprice = 0;
	action->tpprice = 0;
	action->expiration = 0;

	writeMqlString(action->symbol, L"");
	writeMqlString(action->comment, L"");
}

void ffc::closeOrder(int ticket, double lots, double openprice) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId = JOB_CLOSE;
	action->ticket = ticket;
	action->type = 0;
	action->magic = 0;
	action->lots = lots;
	action->openprice = openprice;
	action->slprice = 0;
	action->tpprice = 0;
	action->expiration = 0;

	writeMqlString(action->symbol, L"");
	writeMqlString(action->comment, L"");
}
void ffc::closeOrder(FfcOrder* order) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId = JOB_CLOSE;
	action->ticket = order->ticket;
	action->type = 0;
	action->magic = 0;
	action->lots = order->lots;
	action->openprice = order->openprice;
	action->slprice = 0;
	action->tpprice = 0;
	action->expiration = 0;

	writeMqlString(action->symbol, L"");
	writeMqlString(action->comment, L"");
}

void ffc::showValue(int line, wchar_t* value) {
	if (actionsCount + 1 >= actionsMaxCount) return;
	auto action = actions + actionsCount;
	actionsCount++;

	action->actionId = JOB_SHOW_VALUE;
	action->ticket = line;
	action->type = 0;
	action->magic = 0;
	action->lots = 0;
	action->openprice = 0;
	action->slprice = 0;
	action->tpprice = 0;
	action->expiration = 0;

	writeMqlString(action->symbol, L"");
	writeMqlString(action->comment, value);
}


