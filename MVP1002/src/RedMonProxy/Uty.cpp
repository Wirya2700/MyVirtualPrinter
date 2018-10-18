//
// Uty.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// �w��g���q���܂ރ��j�[�N�Ȉꎞ�t�@�C�������擾
BOOL MyGetTempFileName(IN LPCWSTR pszTargetDir, // �Ώۃf�B���N�g��
					   IN LPCWSTR pszExt,       // �g���q�w�� ".txt" �̗v��
					   OUT WCHAR szOutFileName[MAX_PATH]) // �o�̓o�b�t�@
{
	time_t now;
	WCHAR szDt[16];
	WCHAR buf[MAX_PATH];

	time(&now);
	StringCchPrintf(szDt, ARRAY_SIZE(szDt), L"%u", now);
	_wcsrev(szDt);

	for (int i = 0; i < 10; i++) {
		GetTempFileName(pszTargetDir, szDt, 0, buf);
		DeleteFile(buf); // API �̎��@�t�@�C�����폜
		if (pszExt) {
			// API���瓾��ꂽ "xxxx.tmp" �� "xxxx.[ext]" �ɍ����ւ�
			WCHAR *p = wcsrchr(buf, '.');
			StringCchCopy(p, ARRAY_SIZE(buf)-(p-buf), pszExt);
		}
		if (GetFileAttributes(buf) == 0xFFFFFFFF) {
			StringCchCopy(szOutFileName, MAX_PATH, buf);
			return TRUE;
		}
	}
	*szOutFileName = L'\0';
	return FALSE;
}

// GetSaveFileName ���b�p
LPWSTR MyGetSaveFileName(HWND hParentWnd,
		LPCWSTR pszTitle,
		LPCWSTR pszFilter,
		LPCWSTR pszInitDir,
		LPCWSTR pszDefaultName)
{
	OPENFILENAME ofn;
	static WCHAR wszFile[MAX_PATH] = L"\0";

	if (pszDefaultName) {
		StringCchCopy(wszFile, ARRAY_SIZE(wszFile), pszDefaultName);
	}
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hParentWnd;
	ofn.lpstrCustomFilter = NULL;
	ofn.lpstrInitialDir   = pszInitDir;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrDefExt       = L"";
	ofn.lpstrFile         = wszFile;
	ofn.nMaxFile          = MAX_PATH;
	ofn.lpstrTitle        = pszTitle;
	ofn.lpstrFilter       = pszFilter;
	ofn.Flags            |= OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName(&ofn)) {
		return NULL;
	}
	return wszFile;
}

// OutputDebugString ���b�p
void dbg(LPCWSTR format, ...)
{
	WCHAR buf[2048];
	va_list args;
	va_start(args, format);
	StringCchVPrintf(buf, ARRAY_SIZE(buf), format, args);
	va_end(args);
	if (buf[wcslen(buf)-1] != L'\n') {
		StringCchCat(buf, ARRAY_SIZE(buf), L"\n");
	}
	OutputDebugString(buf);
}

// MessageBox ���b�p
int MB(HWND hParentWnd, UINT uType, LPCWSTR format, ...)
{
	WCHAR buf[2048];
	va_list args;
	va_start(args, format);
	StringCchVPrintf(buf, ARRAY_SIZE(buf), format, args);
	va_end(args);
	return MessageBox(hParentWnd, buf, APPNAME, uType);
}
