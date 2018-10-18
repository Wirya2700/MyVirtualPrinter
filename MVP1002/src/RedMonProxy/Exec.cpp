//
// Exec.cpp  �v���Z�X����
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"
#include <wtsapi32.h>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "userenv.lib")

#define MODNAME_GHOSTSCRIPT   L"gswin32c.exe"
#define MODNAME_IMAGEMAGICK   L"convert.exe"

#define ARGS_GHOSTSCRIPT \
	L" -q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=pdfwrite"  \
	L" -dCompatibilityLevel=1.4 -dPDFSETTING=/prepress" \
	L" -sOutputFile=\"%s\" -f \"%s\""

#define DENSITY_IMAGEMAGICK  L"120" // �o�͉摜�̃s�N�Z�����x
#define ARGS_IMAGEMAGICK \
	L" -density " DENSITY_IMAGEMAGICK L" \"%s\" \"%s\""
#define ARGS_IMAGEMAGICK_ADJOIN \
	L" -density " DENSITY_IMAGEMAGICK L" -adjoin \"%s\" \"%s\""

// PostScript -> PDF
BOOL Ps2Pdf(IN LPCWSTR pszPSFileName, IN LPCWSTR pszPdfFileName, IN DWORD dwTimeoutMsec)
{
	WCHAR szCmd[MAX_PATH];
	WCHAR szArgs[512];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
	WCHAR *p = wcsrchr(szCmd, L'\\');
	*(p+1) = L'\0';

	StringCchCat(szCmd, ARRAY_SIZE(szCmd), MODNAME_GHOSTSCRIPT);
	StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_GHOSTSCRIPT, pszPdfFileName, pszPSFileName);

	if (!CreateProcess(szCmd, szArgs, NULL, NULL, FALSE,
					CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		return FALSE;
	}
	// �w�莞�Ԃ��o�߂��Ă��v���Z�X���I�����Ȃ���΋����I��
	if (WaitForSingleObject(pi.hProcess, dwTimeoutMsec) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, -1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return FALSE;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

// PDF -> �摜�`��
BOOL Pdf2Image(IN LPCWSTR pszPdfFileName, IN LPCWSTR pszImageFileName, IN LPCWSTR pszExt, IN DWORD dwTimeoutMsec)
{
	WCHAR szCmd[MAX_PATH];
	WCHAR szArgs[512];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	if (!pszPdfFileName || wcslen(pszPdfFileName) <= 0 ||
		!pszImageFileName || wcslen(pszImageFileName) <= 0) {
		return FALSE;
	}

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
	WCHAR *p = wcsrchr(szCmd, L'\\');
	*(p+1) = L'\0';

	// ImageMagick��convert.exe��gswin32c.exe�Ɉˑ�����B
	// �����t�H���_���gswin32c.exe���Q�Ƃ����邽�߂�
	// �J�����g�f�B���N�g�����ړ� -> gs��T���ă��W�X�g�������ɍs���̂����
	SetCurrentDirectory(szCmd);

	StringCchCat(szCmd, ARRAY_SIZE(szCmd), MODNAME_IMAGEMAGICK);
	if (_wcsicmp(pszExt, L"tiff") == 0) {
		StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_IMAGEMAGICK, pszPdfFileName, pszImageFileName);
	} else {
		StringCchPrintf(szArgs, ARRAY_SIZE(szArgs), ARGS_IMAGEMAGICK_ADJOIN, pszPdfFileName, pszImageFileName); // �摜���ʂɏo��
	}
	if (!CreateProcess(szCmd, szArgs, NULL, NULL, FALSE,
				CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		return FALSE;
	}
	// �w�莞�Ԃ��o�߂��Ă��v���Z�X���I�����Ȃ���΋����I��
	if (WaitForSingleObject(pi.hProcess, dwTimeoutMsec) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, -1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return FALSE;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

// ���݂̃��O�I�����[�U�̃Z�L�����e�B�R���e�L�X�g��
// �V�����v���Z�X���N��
// �{�֐��� LocalSystem Account ����Ăяo���K�v������
DWORD ExecAsUser(IN LPCWSTR szApp, IN LPWSTR szCmdline, IN OUT PROCESS_INFORMATION *ppi)
{
	DWORD err = ERROR_SUCCESS;
	HANDLE hToken = INVALID_HANDLE_VALUE;
	HANDLE hPrimaryToken = INVALID_HANDLE_VALUE;
	VOID *pUserEnv = NULL;
	STARTUPINFO si;

	// ���݂̃A�N�e�B�u�Z�b�V������ID�𓾂�
	DWORD dwSessionId = WTSGetActiveConsoleSessionId();

	// �Z�b�V����ID�ɑΉ����郍�O�I�����[�U�̃A�N�Z�X�g�[�N���𓾂�
	if (!WTSQueryUserToken(dwSessionId, &hToken)) {
		err = GetLastError();
		dbg(L"WTSQueryUserToken err=%u session=%u", err, dwSessionId);
		goto DONE;
	}
	// CreateProcessAsUser �p�Ƀv���C�}���A�N�Z�X�g�[�N���𓾂�
	if (!DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation,
								TokenPrimary, &hPrimaryToken)) {
		err = GetLastError();
		dbg(L"DuplicateTokenEx err=%u", err);
		goto DONE;
	}
	CloseHandle(hToken);

	// �v���C�}���A�N�Z�X�g�[�N�����猻���[�U�̊��u���b�N���擾
	if (!CreateEnvironmentBlock(&pUserEnv, hPrimaryToken, FALSE)) {
		err = GetLastError();
		dbg(L"CreateEnvironmentBlock err=%u", err);
		goto DONE;
	}

	// �����O�I�����[�U�̃Z�L�����e�B�R���e�L�X�g�Ńv���Z�X���N��
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(ppi, sizeof(PROCESS_INFORMATION));
	si.wShowWindow = SW_SHOW;
	si.lpDesktop = L"Winsta0\\Default"; // �f�t�H���g�̃A�v���P�[�V�����f�X�N�g�b�v
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcessAsUser(hPrimaryToken, szApp, szCmdline,
								NULL, NULL, FALSE,
								CREATE_UNICODE_ENVIRONMENT | DETACHED_PROCESS,
								pUserEnv, // ���[�U���u���b�N�𖾎�
								NULL, &si, ppi)) {
		err = GetLastError();
		dbg(L"CreateProcessAsUser err=%u", err);
		goto DONE;
	}

DONE:
	if (pUserEnv) {
		DestroyEnvironmentBlock(pUserEnv);
	}
	if (hToken != INVALID_HANDLE_VALUE) {
		CloseHandle(hToken);
	}
	if (hPrimaryToken != INVALID_HANDLE_VALUE) {
		CloseHandle(hPrimaryToken);
	}
    return err;
}

