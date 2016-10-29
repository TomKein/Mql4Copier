#include "wchar.h"
#include "utils.h"

int ffc::getMagic(wchar_t* comment) {
	wchar_t* pwc;
	pwc = wcstok(comment, L"_");
	pwc = wcstok(NULL, L"_");
	pwc = wcstok(NULL, L"_");
	if (pwc == NULL) return 0;
	//std::wcout << "getMagic " << comment << "/" << _wtoi(pwc) << "\r\n";
	return _wtoi(pwc);
}

int ffc::getMasterTicket(wchar_t* comment) {
	wchar_t* pwc;
	pwc = wcstok(comment, L"_");
	pwc = wcstok(NULL, L"_");
	if (pwc == NULL) return 0;
	//std::wcout << "getMagic " << comment << "/" << _wtoi(pwc) << "\r\n";
	return _wtoi(pwc);
}

///�������� ������ �++ � MQLSting (�� ������� MQL ������ ������ ������ �� ����)
inline void ffc::writeMqlString(MqlString dest, wchar_t* source) {
	int len = min(wcslen(source), dest.size - 1);  //���������� ����� ������ (�������� ��������������� ������)
	wcscpy_s(dest.buffer, len + 1, source);  //�������� ������� ������������� ����
	*(((int*)dest.buffer) - 1) = len;  // ���������� ����� ������ (���, ����� ���������� � �������� ������� ���������)
}
