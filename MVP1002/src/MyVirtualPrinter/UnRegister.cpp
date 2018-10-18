//
// UnRegister.cpp  ���z�v�����^���\�[�X�̍폜
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"


// WINDIR\system32\spool\drivers\w32x86\3\ �z���̏���̃t�@�C�����폜
BOOL DoDeleteMyDriverFile(LPCWSTR pszDriverPath, LPCWSTR szDriverName)
{
	WCHAR szDriverFile[MAX_PATH];
	StringCchPrintf(szDriverFile, ARRAY_SIZE(szDriverFile), L"%s\\3\\%s", pszDriverPath, szDriverName);
	if (GetFileAttributes(szDriverFile) != 0xFFFFFFFF) {
		DeleteFile(szDriverFile);
	}
	return TRUE;
}

// �v�����^�G���g�� MyVirtualPrinter ���V�X�e������폜
BOOL DeleteMyPrinter(HWND hDlg, BOOL bShowError)
{
	HANDLE hPrinter;
	PRINTER_DEFAULTS Defaults;

	ZeroMemory(&Defaults, sizeof(PRINTER_DEFAULTS)); 
	Defaults.DesiredAccess = PRINTER_ALL_ACCESS;

	if (!OpenPrinter(NAME_MYPRINTER, &hPrinter, &Defaults)) {
		if (bShowError) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"DeleteMyPrinter: OpenPrinter err=%u", GetLastError());
		}
		return FALSE;
	}
	if (!DeletePrinter(hPrinter)) {
		ClosePrinter(hPrinter);
		if (bShowError) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"DeleteMyPrinter: DeletePrinter err=%u", GetLastError());
		}
		return FALSE;
	}
	ClosePrinter(hPrinter);

	// �������� MyVirtualPrinter.ppd �� "3\" �z������폜
	// �iWindows �W���� pscript5.dll, ps5ui.dll �͍폜���Ȃ��j
	WCHAR szPath[MAX_PATH];
	DWORD uSize=0; 
	if (GetPrinterDriverDirectory(NULL, ENVIRONMENT, 1, (BYTE*)szPath, MAX_PATH, &uSize)) {
		DoDeleteMyDriverFile(szPath, DRIVER_MYPPD);
	}
	return TRUE;
}

// �h���C�o�G���g�� MyVirtualPrinterDriver ���V�X�e������폜
BOOL DeleteMyPrinterDriver(HWND hDlg, BOOL bShowError)
{
	if(!DeletePrinterDriver(NULL, ENVIRONMENT, NAME_MYPRINTERDRIVER)) { 
		if (bShowError) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"DeleteMyPrinterDriver: DeletePrinterDriver err=%u", GetLastError());
		}
		return FALSE;
	} 
	return TRUE;
}

// �|�[�g�G���g�� MyVirtualPrinterPort: ���V�X�e������폜
BOOL DeleteMyPort(HWND hDlg, BOOL bShowError)
{
	if (!DeletePort(NULL, NULL, NAME_MYPORT)) {
		if (bShowError) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"DeleteMyPort: DeletePort err=%u", GetLastError());
		}
		return FALSE;
	}
	return TRUE;
}

// �|�[�g���j�^�G���g�� MyVirtualPrinterDriver Redirected Port ���V�X�e������폜
BOOL DeleteMyPortMonitor(HWND hDlg, BOOL bShowError)
{
	if(!DeleteMonitor(NULL, ENVIRONMENT, NAME_MYPORTMONITOR)) { 
		if (bShowError) {
			MB(hDlg, MB_OK|MB_ICONWARNING, L"DeleteMyPortMonitor: DeleteMonitor err=%u", GetLastError());
		}
		return FALSE;
	}
	return TRUE;
}

