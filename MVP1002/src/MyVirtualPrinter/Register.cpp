//
// Register.cpp  ���z�v�����^���\�[�X�̃V�X�e���ւ̓o�^
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// Windows �W���� PostScript �v�����^�h���C�o���V�X�e�����璊�o��
// �{ EXE �p�X���� \Drivers �փR�s�[����
// 0: OK
// ���l: �G���[
int CollectPSDrivers()
{
#define NUM_PSCRIPTDRIVERS 2
	WCHAR szPScriptDrivers[NUM_PSCRIPTDRIVERS][16] = {
		DRIVER_PSCRIPT5,
		DRIVER_PSCRIPT5UI
	};
	WCHAR szPath[MAX_PATH];
	WCHAR szWork[MAX_PATH];
	WCHAR szOut[MAX_PATH];

	int os = GetPlatform();
	if (os <= 0) { // 1:2000 2:xp 3:vista 4:7 or later
		return -1;
	}
	// %WINDIR% ���E��
	GetEnvironmentVariable(L"WINDIR", szPath, sizeof(szPath));
	// 2000, XP �̏ꍇ�� cab �t�@�C���Ɋi�[����Ă���
	if (os <= 2) { 
		StringCchCat(szPath, ARRAY_SIZE(szPath), L"\\Driver Cache\\i386\\");
		WIN32_FIND_DATA fd;
		WCHAR szPat[MAX_PATH];
		HANDLE hFind;
		time_t timestamp = 0;
		WCHAR szCab[MAX_PATH] = L"\0";
		// \Driver Cache\i386\*.cab �̍ŐV�̂��̂�T��
		StringCchPrintf(szPat, ARRAY_SIZE(szPat), L"%s*.cab", szPath);
		hFind = FindFirstFile(szPat, &fd);
		if(hFind == INVALID_HANDLE_VALUE) {
			return -2;
		}
		do {
			// �^�C���X�^���v�̂����Ƃ��V�����t�@�C����T��
			StringCchPrintf(szWork, ARRAY_SIZE(szWork), L"%s%s", szPath, fd.cFileName);
			struct _stat st;
			_wstat(szWork, &st);
			if (st.st_mtime >= timestamp) {
				StringCchCopy(szCab, ARRAY_SIZE(szCab), fd.cFileName);
				timestamp = st.st_mtime;
			}
		} while(FindNextFile(hFind, &fd));
		FindClose(hFind);

		if (wcslen(szCab) <= 0) {
			return -3;
		}
		StringCchCat(szPath, ARRAY_SIZE(szPath), szCab);
		GetModuleFileName(NULL, szOut, sizeof(szOut));
		WCHAR *pt = wcsrchr(szOut, L'\\');
		*pt = L'\0';
		StringCchCat(szOut, ARRAY_SIZE(szOut), L"\\Drivers");

		// �W���� expand.exe ���g���� cab ����K�v�t�@�C���Q�𒊏o�� �茳��"Drives\"��
		WCHAR szCmd[MAX_PATH];
		WCHAR szArgs[512];
		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(si);
		GetEnvironmentVariable(L"WINDIR", szCmd, sizeof(szCmd));
		StringCchCat(szCmd, ARRAY_SIZE(szCmd), L"\\system32\\expand.exe");
		for (int i = 0; i < NUM_PSCRIPTDRIVERS; i++) {
			StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), L" \"%s\" -f:%s \"%s\"", szPath, szPScriptDrivers[i], szOut);
			if (!CreateProcess(szCmd, szArgs, NULL, NULL, FALSE,
				CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
				return -4;;
			}
			// �v���Z�X�I���܂ő҂�
			if (WaitForSingleObject(pi.hProcess, 5000) == WAIT_TIMEOUT) {
				TerminateProcess(pi.hProcess, -1);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				pi.hProcess = NULL;
				pi.hThread = NULL;
				return -5;
			}
			if (pi.hProcess) {
				CloseHandle(pi.hProcess);
			}
			if (pi.hThread) {
				CloseHandle(pi.hThread);
			}
		}
	}
	else { // vista, 7 or later �̏ꍇ�A
		// PS�h���C�o�� System32\DriverStore\FileRepository\ntprint.inf*\I386\ ���Ɏ��̂�����
		StringCchCat(szPath, ARRAY_SIZE(szPath), L"\\System32\\DriverStore\\FileRepository\\");
		WIN32_FIND_DATA fd;
		WCHAR szPat[MAX_PATH];
		HANDLE hFind;
		time_t timestamp = 0;
		WCHAR szDir[MAX_PATH] = L"\0";
		// �ŐV�� ntprint.inf*\ �f�B���N�g����T��
		StringCchPrintf(szPat, ARRAY_SIZE(szPat), L"%sntprint.inf*", szPath);
		hFind = FindFirstFile(szPat, &fd);
		if(hFind == INVALID_HANDLE_VALUE) {
			return -6;
		}
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				continue;
			}
			// �^�C���X�^���v�̂����Ƃ��V�����f�B���N�g�����E��
			StringCchPrintf(szWork, ARRAY_SIZE(szWork), L"%s%s", szPath, fd.cFileName);
			struct _stat st;
			_wstat(szWork, &st);
			if (st.st_mtime >= timestamp) {
				StringCchCopy(szDir, ARRAY_SIZE(szDir), fd.cFileName);
				timestamp = st.st_mtime;
			}
		} while(FindNextFile(hFind, &fd));
		FindClose(hFind);

		if (wcslen(szDir) <= 0) {
			return -7;
		}
		StringCchCat(szPath, ARRAY_SIZE(szPath), szDir);
		StringCchCat(szPath, ARRAY_SIZE(szPath), L"\\I386\\");

		// �m�肵���f�B���N�g������K�v�t�@�C���Q���茳��"Drives\"�փR�s�[
		GetModuleFileName(NULL, szOut, sizeof(szOut));
		WCHAR *pt = wcsrchr(szOut, L'\\');
		*pt = L'\0';
		StringCchCat(szOut, ARRAY_SIZE(szOut), L"\\Drivers\\");

		for (int i = 0; i < NUM_PSCRIPTDRIVERS; i++) {
			WCHAR src[MAX_PATH];
			WCHAR dst[MAX_PATH];
			StringCchPrintf(src, ARRAY_SIZE(src), L"%s%s", szPath, szPScriptDrivers[i]);
			StringCchPrintf(dst, ARRAY_SIZE(dst), L"%s%s", szOut, szPScriptDrivers[i]);

			if (!CopyFile(src, dst, FALSE)) {
				return -8;
			}
		}
	}
	return 0;
}

// �K�v�ȃV�X�e���W���̃v�����^�h���C�o������̃t�H���_�փR�s�[
BOOL DoCopyPrinterDriverFile(HWND hDlg, LPCWSTR pszDriverPath, LPCWSTR szDriverName)
{
	DWORD sts;
	WCHAR szDriverPath3[MAX_PATH];
	WCHAR szDest[MAX_PATH];
	WCHAR szWork[MAX_PATH];
	static BOOL bDonePickup = FALSE;

	StringCchPrintf(szDriverPath3, ARRAY_SIZE(szDriverPath3), L"%s\\3\\%s", pszDriverPath, szDriverName);
	StringCchPrintf(szDest, ARRAY_SIZE(szDest), L"%s\\%s", pszDriverPath, szDriverName);

	sts = GetFileAttributes(szDriverPath3);

	// WINDIR\system32\spool\drivers\w32x86\3\ �z����
	// ���Y PostScript �h���C�o�����ݒu�̏ꍇ
	if (sts == 0xFFFFFFFF) {
		if (!bDonePickup) {
			int ret = CollectPSDrivers(); // Windows�̎���PS�h���C�o���E���Ă���
			if (ret != 0) {
				MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyPrinterDriverFile: CollectPSDrivers ret=%d", ret);
			}
			bDonePickup = TRUE;
		}
		GetModuleFileName(NULL, szWork, sizeof(szWork)/sizeof(WCHAR));
		WCHAR *wp = wcsrchr(szWork, L'\\');
		*wp = L'\0';
		StringCchCat(szWork, ARRAY_SIZE(szWork), L"\\Drivers\\");
		StringCchCat(szWork, ARRAY_SIZE(szWork), szDriverName);

		if (!CopyFile(szWork, szDriverPath3, FALSE)) {
			MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyPrinterDriverFile: CopyFile1 [%s] err=%u", szDriverName, GetLastError());
			return FALSE;
		}
	}
	// WINDIR\system32\spool\drivers\w32x86\ �z���ɃR�s�[
	// -> AddPrinterDriver �����p
	if (!CopyFile(szDriverPath3, szDest, FALSE)) {
		MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyPrinterDriverFile: CopyFile2 [%s] err=%u", szDriverPath3, GetLastError());
		return FALSE;
	}
	return TRUE;
}

// �{ EXE �p�X���� \Drivers �z���̃v�����^�h���C�o���V�X�e���t�H���_�փR�s�[
BOOL DoCopyMyDriverFile(HWND hDlg, LPCWSTR pszDriverPath, LPCWSTR szDriverName)
{
	WCHAR szDriverPath[MAX_PATH];
	WCHAR szData[MAX_PATH];

	GetModuleFileName(NULL, szData, sizeof(szData)/sizeof(WCHAR));
	WCHAR *wp = wcsrchr(szData, L'\\');
	*wp = L'\0';
	StringCchCat(szData, ARRAY_SIZE(szData), L"\\Drivers\\");
	StringCchCat(szData, ARRAY_SIZE(szData), szDriverName);

	if (GetFileAttributes(szData) == 0xFFFFFFFF) {
		MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyMyDriverFile: not found [%s]", szData);
		return FALSE;
	}

	// WINDIR\system32\spool\drivers\w32x86\3\ �z���ւ̃R�s�[
	// -> ����������Ɏg�p��������
	StringCchPrintf(szDriverPath, ARRAY_SIZE(szDriverPath), L"%s\\3\\%s", pszDriverPath, szDriverName);
	if (!CopyFile(szData, szDriverPath, FALSE)) {
		MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyMyDriverFile: CopyFile1 [%s] err=%u", szData, GetLastError());
		return FALSE;
	}

	// WINDIR\system32\spool\drivers\w32x86\ �z���ւ̃_�~�[�R�s�[
	// -> AddPrinterDriver �p
	StringCchPrintf(szDriverPath, ARRAY_SIZE(szDriverPath), L"%s\\%s", pszDriverPath, szDriverName);
	if (!CopyFile(szData, szDriverPath, FALSE)) {
		MB(hDlg, MB_OK|MB_ICONERROR, L"DoCopyMyDriverFile: CopyFile2 [%s] err=%u", szData, GetLastError());
		return FALSE;
	}
	return TRUE;
}

// AddPrinterDriver �p�Ƀh���C�o�p�X�փR�s�[�����h���C�o�t�@�C�����폜
BOOL DoDeleteFakePrinterDriverFiles(LPCWSTR pszDriverPath)
{
	WCHAR szDriverFile[MAX_PATH];

	StringCchPrintf(szDriverFile, ARRAY_SIZE(szDriverFile), L"%s\\%s", pszDriverPath, DRIVER_PSCRIPT5);
	if (GetFileAttributes(szDriverFile) != 0xFFFFFFFF) {
		DeleteFile(szDriverFile);
	}
	StringCchPrintf(szDriverFile, ARRAY_SIZE(szDriverFile), L"%s\\%s", pszDriverPath, DRIVER_PSCRIPT5UI);
	if (GetFileAttributes(szDriverFile) != 0xFFFFFFFF) {
		DeleteFile(szDriverFile);
	}
	StringCchPrintf(szDriverFile, ARRAY_SIZE(szDriverFile), L"%s\\%s", pszDriverPath, DRIVER_MYPPD);
	if (GetFileAttributes(szDriverFile) != 0xFFFFFFFF) {
		DeleteFile(szDriverFile);
	}
	return TRUE;
}

// RedMon ���V�X�e���t�H���_�փR�s�[��
// �|�[�g���j�^�G���g�� MyVirtualPrinterDriver Redirected Port ���V�X�e���ɓo�^
BOOL InstallMyPortMonitor(HWND hDlg)
{
	MONITOR_INFO_1 *pminfo1;
	MONITOR_INFO_2 minfo2;
	WCHAR szSrc[MAX_PATH];
	WCHAR szDst[MAX_PATH];
	DWORD dwCbNeeded, dwNum;

	// �����̃|�[�g���j�^�𒲍�
	BOOL sts = EnumMonitors(NULL, 1, NULL, 0, &dwCbNeeded, &dwNum); // �K�v�T�C�Y�擾
	if (!sts && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return FALSE;
	}
	pminfo1 = (MONITOR_INFO_1*)malloc(dwCbNeeded);
	if (!EnumMonitors(NULL, 1, (BYTE*)pminfo1, dwCbNeeded, &dwCbNeeded, &dwNum)) {
		return FALSE;
	}
	for (DWORD i = 0; i < dwNum; i++) {
		if (wcscmp(pminfo1[i].pName, NAME_MYPORTMONITOR) == 0) {
			free(pminfo1);
			return TRUE; // �����̏ꍇ�͔�����i��G���[�j
		}
	}
	free(pminfo1);

	// �R�s�[���͖{ EXE �p�X���� \Drivers\redmonnt.dll
	GetModuleFileName(NULL, szSrc, ARRAY_SIZE(szSrc));
	WCHAR *p = wcsrchr(szSrc, L'\\');
	*p = L'\0';
	StringCchCat(szSrc, ARRAY_SIZE(szSrc), L"\\Drivers\\");
	StringCchCat(szSrc, ARRAY_SIZE(szSrc), NAME_REDMONDLL);

	// �R�s�[��� WINDIR\system32
	if (!GetSystemDirectory(szDst, ARRAY_SIZE(szDst))) {
		return FALSE;
	}
	StringCchCat(szDst, ARRAY_SIZE(szDst), L"\\");
	StringCchCat(szDst, ARRAY_SIZE(szDst), NAME_REDMONDLL);

	// �R�s�[��� redmonnt.dll �����݂��Ȃ��ꍇ�̂݃R�s�[
	if (GetFileAttributes(szDst) == 0xFFFFFFFF) {
		DWORD sts = GetFileAttributes(szSrc);
		if (sts == 0xFFFFFFFF) { // �R�s�[���� RedMonNt.dll �����݂��Ȃ�
			MB(hDlg, MB_OK|MB_ICONWARNING, L"InstallMyPortMonitor: not found [%s]", szSrc);
			return FALSE;
		}
		// RedMonNt.dll�� System32\ �փR�s�[
		if (!CopyFile(szSrc, szDst, FALSE)) {
			DWORD err = GetLastError();
			MB(hDlg, MB_OK|MB_ICONWARNING, L"InstallMyPortMonitor: CopyFile [%s] err=%u", szDst, GetLastError());
			return FALSE;
		}
	}
	// �|�[�g���j�^�Ƃ��ēo�^
	minfo2.pName        = NAME_MYPORTMONITOR;
	minfo2.pEnvironment = ENVIRONMENT;
	minfo2.pDLLName     = NAME_REDMONDLL;
	if (!AddMonitor(NULL, 2, (BYTE*)&minfo2)) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"InstallMyPortMonitor: AddMonitor err=%u", GetLastError());
		return FALSE;
	}
	return TRUE;
}

// �|�[�g�G���g�� MyVirtualPrinterPort: ���V�X�e���ɓo�^
BOOL CreateMyPort(HWND hDlg)
{
	BYTE *pData = NULL;
	HANDLE hXcv;
	DWORD dwNeededCb, dwStatus;
	PRINTER_DEFAULTS Defaults;

	Defaults.pDatatype     = NULL;
	Defaults.pDevMode      = NULL;
	Defaults.DesiredAccess = SERVER_ACCESS_ADMINISTER;
	
	// RedMon �o�R�Ő�p�|�[�g��ǉ�����
	if(!OpenPrinter(L",XcvMonitor " NAME_MYPORTMONITOR, &hXcv, &Defaults)) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPort: OpenPrinter err=%u", GetLastError());
		return FALSE;
	}
	BOOL sts = XcvData(hXcv, L"AddPort", (BYTE*)NAME_MYPORT, (wcslen(NAME_MYPORT)+1)*sizeof(WCHAR),
						NULL, 0, &dwNeededCb, &dwStatus); // �K�v�T�C�Y���擾
	DWORD err = GetLastError();
	if (!sts) {
		if (err == ERROR_INSUFFICIENT_BUFFER) {
			pData = (BYTE*)malloc(dwNeededCb);
			if(!XcvData(hXcv, L"AddPort", (BYTE*)NAME_MYPORT, (wcslen(NAME_MYPORT)+1)*sizeof(WCHAR),
						pData, dwNeededCb, &dwNeededCb, &dwStatus)) {
				free(pData);
			    ClosePrinter(hXcv);
				MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPort: XcvData2 err=%u", GetLastError());
				return FALSE;
			}
		} else {
		    ClosePrinter(hXcv);
			MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPort: XcvData1 err=%u", err);
			return FALSE;
		}
	}
    ClosePrinter(hXcv);

	// RedMon ����R�[������Command �Ƃ��Ė{ EXE �p�X����
	// \Modules\RedMonProxy.exe ��ݒ�
	HKEY hKey;
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
				L"SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\"
					NAME_MYPORTMONITOR L"\\Ports\\" NAME_MYPORT,
				0, KEY_SET_VALUE, &hKey);
	if (ret != ERROR_SUCCESS) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPort: RegOpenKeyEx err=%u", GetLastError());
		return FALSE;
	}
	WCHAR szData[MAX_PATH];
	GetModuleFileName(NULL, szData, ARRAY_SIZE(szData));
	WCHAR *wp = wcsrchr(szData, L'\\');
	*wp = L'\0';
	StringCchCat(szData, ARRAY_SIZE(szData), L"\\Modules\\RedMonProxy.exe");
	BYTE *p = (BYTE*)szData;
	ret = RegSetValueEx(hKey, L"Command", 0, REG_SZ, p, (DWORD)(wcslen(szData)+1)*sizeof(WCHAR));
	if (ret != ERROR_SUCCESS) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPort: RegSetValueEx err=%u", GetLastError());
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);

	return TRUE;
}

// �h���C�o�t�@�C�����V�X�e���t�H���_�փR�s�[��
// �h���C�o�G���g�� MyVirtualPrinterDriver ���V�X�e���ɓo�^
BOOL InstallMyPrinterDriver(HWND hDlg)
{
	WCHAR buf[MAX_PATH];
	DRIVER_INFO_3 dinfo3; 

	// �v�����^�h���C�o�f�B���N�g�����Ɖ�
	DWORD dwSize=0; 
	if (!GetPrinterDriverDirectory(NULL, ENVIRONMENT, 1, (LPBYTE)buf, sizeof(buf), &dwSize)) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"InstallMyPrinterDriver: GetPrinterDriverDirectory err=%u", GetLastError());
		return FALSE;
	}
	// pscript5.dll, ps5ui.dll ���V�X�e���t�H���_�փR�s�[
	if (!DoCopyPrinterDriverFile(hDlg, buf, DRIVER_PSCRIPT5)) {
		return FALSE;
	}
	if (!DoCopyPrinterDriverFile(hDlg, buf, DRIVER_PSCRIPT5UI)) {
		return FALSE;
	}
	// MyVirtualPrinter.ppd ���V�X�e���t�H���_�փR�s�[
	if (!DoCopyMyDriverFile(hDlg, buf, DRIVER_MYPPD)) {
		return FALSE;
	}

	// MyVirtualPrinterDriver ���V�X�e���ɓo�^
	WCHAR szDriverPath[MAX_PATH]; 
	WCHAR szConfigFilePath[MAX_PATH]; 
	WCHAR szDataFilePath[MAX_PATH]; 

	StringCchPrintf(szDriverPath, ARRAY_SIZE(szDriverPath), L"%s\\" DRIVER_PSCRIPT5, buf);
	StringCchPrintf(szConfigFilePath, ARRAY_SIZE(szConfigFilePath), L"%s\\" DRIVER_PSCRIPT5UI, buf);
	StringCchPrintf(szDataFilePath, ARRAY_SIZE(szDataFilePath), L"%s\\" DRIVER_MYPPD, buf);

	ZeroMemory(&dinfo3, sizeof(DRIVER_INFO_3));
	dinfo3.cVersion         = 0x03;             // 3 �Œ�
	dinfo3.pConfigFile      = szConfigFilePath; // ps5ui.dll
	dinfo3.pDataFile        = szDataFilePath;   // MyVirtualPrinter.ppd
	dinfo3.pDependentFiles  = NULL;
	dinfo3.pDriverPath      = szDriverPath;     // pscript5.dll
	dinfo3.pEnvironment     = ENVIRONMENT;
	dinfo3.pHelpFile        = NULL;
	dinfo3.pMonitorName     = NULL; 
	dinfo3.pName            = NAME_MYPRINTERDRIVER; // MyVirtualPrinterDriver
	dinfo3.pDefaultDataType = NULL;
	if(!AddPrinterDriver(NULL, 3, (LPBYTE)&dinfo3)) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"InstallMyPrinterDriver: AddPrinterDriver err=%u", GetLastError());
		DoDeleteFakePrinterDriverFiles(buf);
		return FALSE;
	}
	// AddPrinterDriver �p�̃_�~�[�R�s�[���폜
	DoDeleteFakePrinterDriverFiles(buf);
    return TRUE;
}

// �v�����^�G���g�� MyVirtualPrinter ���V�X�e���ɓo�^
BOOL CreateMyPrinter(HWND hDlg)
{
	PRINTER_INFO_2 pinfo2;
	HANDLE hPrinter;

	ZeroMemory(&pinfo2, sizeof(PRINTER_INFO_2)); 
	pinfo2.pServerName     = NULL;
	pinfo2.pPrinterName    = NAME_MYPRINTER;       // MyVirtualPrinter
	pinfo2.pPortName       = NAME_MYPORT;          // MyVirtualPrinterPort:
	pinfo2.pDriverName     = NAME_MYPRINTERDRIVER; // MyVirtualPrinterDriver
	pinfo2.pPrintProcessor = L"WinPrint";          // �W���̃v�����g�v���Z�b�T
	pinfo2.pDatatype       = NULL; 
	hPrinter = AddPrinter(NULL, 2, (BYTE*)&pinfo2);
	if(!hPrinter) {
		MB(hDlg, MB_OK|MB_ICONWARNING, L"CreateMyPrinter: AddPrinter err=%u", GetLastError());
		return FALSE;
	}
	ClosePrinter(hPrinter);
	return TRUE;
}
