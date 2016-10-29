#pragma once
#include "ffcTypes.h"

namespace ffc {
int getMagic(wchar_t* comment);
int getMasterTicket(wchar_t* comment);
void writeMqlString(MqlString dest, wchar_t* source);
}