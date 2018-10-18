//
// main.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

#pragma comment(lib, "comctl32.lib")

#define NAME_SHAREDEVENT L"Global\\$$" APPNAME
#define TIMEOUT  60000

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	int exitcode = 0;
	WCHAR szTempPath[MAX_PATH];
	WCHAR szTempPs[MAX_PATH];

	// �����Ȃ��̏ꍇ�� �X�v�[���T�[�r�X�`RedMon �o�R�ł̋N���Ƃ݂Ȃ�
	// �����v���Z�X���N���BUI �������܂ގq�v���Z�X�͂��̃v���Z�X���ŋN������
	if (wcslen(lpCmdLine) <= 0) {
		HANDLE hEvent = INVALID_HANDLE_VALUE;
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;
		PACL  pDacl;

		// �q�v���Z�X�Ƃ̘A�g�p�C�x���g���쐬
		// Vista �ȍ~�� session 0 �����ւ̑Ή��ɂ̓Z�L�����e�B�f�X�N���v�^�̐ݒ肪�K�{
		pDacl = MySetSecurityDescriptorDacl(&sd, GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE);
		if (!pDacl) {
			dbg(L"parent: BuildRestrictedSD failed");
			exitcode = 1;
			goto EXIT_PARENT;
		}
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = FALSE;
		hEvent = CreateEvent(&sa, TRUE, TRUE, NAME_SHAREDEVENT);
		//dbg(L"parent: event=%x", hEvent);
		MyFreeDacl(pDacl);

		// PostScript�p�ꎞ�t�@�C�������쐬
		GetTempPath(sizeof(szTempPath)/sizeof(WCHAR), szTempPath);
		if (!MyGetTempFileName(szTempPath, L".ps", szTempPs)) {
			exitcode = 2;
			goto EXIT_PARENT;
		}

		// ���O�I�����[�U�̃Z�L�����e�B�R���e�L�X�g�Ŗ{ exe �̐V�����v���Z�X���N��
		WCHAR szCmd[MAX_PATH];
		WCHAR szArg[MAX_PATH];
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		pi.hProcess = INVALID_HANDLE_VALUE;
		GetModuleFileName(NULL, szCmd, sizeof(szCmd)/sizeof(WCHAR));
		StringCchPrintf(szArg, ARRAY_SIZE(szArg), L" %s", szTempPs);
		DWORD sts = ExecAsUser(szCmd, szArg, &pi);
		if (sts != ERROR_SUCCESS) {
			dbg(L"parent: ExecAsUser err=%u", GetLastError());
			exitcode = 3;
			goto EXIT_PARENT;
		}
		WaitForInputIdle(pi.hProcess, TIMEOUT);

		// PostScript��W�����͂���t�@�C���֐؂�o��
		static BYTE buf[2048];
		FILE *fp;
		size_t len;
		errno_t err = _wfopen_s(&fp, szTempPs, L"wb");
		if (err != 0) {
			dbg(L"parent: open err=%d [%s]", err, szTempPs);
			exitcode = 4;
			TerminateProcess(pi.hProcess, -1);
			goto EXIT_PARENT;
		}
	    _setmode(_fileno(stdin), O_BINARY);
	    while ((len = fread(buf, 1, sizeof(buf), stdin)) > 0) {
			fwrite(buf, 1, len, fp);
	    }
	    fclose(fp);

		// �o�͂�����������q�v���Z�X�֒ʒm
		SetEvent(hEvent);

		// �q�v���Z�X�̊�����҂��ďI��
		WaitForSingleObject(pi.hProcess, INFINITE);

EXIT_PARENT:
		if (GetFileAttributes(szTempPs) != 0xFFFFFFFF) {
			DeleteFile(szTempPs);
		}
		if (hEvent != INVALID_HANDLE_VALUE) {
			CloseHandle(hEvent);
		}
		if (pi.hProcess != INVALID_HANDLE_VALUE) {
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		dbg(L"parent: exitcode=%d", exitcode);
		return exitcode;
	}

	//// �ȉ��A�q�v���Z�X�p�R�[�h ////
	WCHAR szTempPdf[MAX_PATH];
	WCHAR szOutFileName[MAX_PATH];

	InitCommonControls();
	HWND hFgWnd = GetForegroundWindow();

	// �����œn���ꂽ PostScript �ꎞ�t�@�C�������擾
	StringCchCopy(szTempPs, ARRAY_SIZE(szTempPs), __wargv[1]);

	// PostScript �ꎞ�t�@�C���o�͂̊�����҂�
	AppStartWaitMsg(NULL, L"�������E�E�E");
	HANDLE hEventChild = OpenEvent(SYNCHRONIZE, TRUE, NAME_SHAREDEVENT);
	if (!hEventChild) {
		dbg(L"child: OpenEvent err=%u", GetLastError());
		exitcode = 1;
		goto EXIT_CHILD;
	}
	DWORD sts = WaitForSingleObject(hEventChild, TIMEOUT);
	if (sts != WAIT_OBJECT_0) {
		dbg(L"child: WaitForSingleObject sts=%x", sts);
		exitcode = 2;
		goto EXIT_CHILD;
	}
	AppEndWaitMsg();

	// �o�͂���t�@�C���������[�U�ɖ₢���킹��
	LPWSTR ofp = MyGetSaveFileName(
		hFgWnd,
		L"MyVirtualPrinter�F�o�̓t�@�C�������w�肵�ĉ�����",
		L"PDF  �t�@�C�� (*.pdf)\0*.pdf\0"   \
		L"JPEG �t�@�C�� (*.jpg)\0*.jpg\0"   \
		L"PNG  �t�@�C�� (*.png)\0*.png\0"   \
		L"GIF  �t�@�C�� (*.gif)\0*.gif\0"   \
		L"TIFF �t�@�C�� (*.tiff)\0*.tiff\0" \
		L"BMP  �t�@�C�� (*.bmp)\0*.bmp\0"   \
		L"\0",
		L"C:\\",
		L"Document");

	if (!ofp) {
		dbg(L"child: MyGetSaveFileName cancelled");
		exitcode = 0;
		goto EXIT_CHILD;
	}
	StringCchCopy(szOutFileName, ARRAY_SIZE(szOutFileName), ofp);

	AppStartWaitMsg(NULL, L"�o�͒��F���΂炭���҂��������E�E�E");

	// �w�肳�ꂽ�o�̓t�@�C���̊g���q�𓾂�
	WCHAR *pExt = wcsrchr(szOutFileName, L'.');
	if (!pExt) {
		StringCchCat(szOutFileName, ARRAY_SIZE(szOutFileName), L".pdf"); // �f�t�H���g
		pExt = wcsrchr(szOutFileName, L'.');
	}
	pExt++;

	// PDF�̏ꍇ
	if (_wcsicmp(pExt, L"pdf") == 0) {
		if (!Ps2Pdf(szTempPs, szOutFileName, TIMEOUT)) {
			MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"PDF�o�͒��ɃG���[���������܂���");
		}
	}
	// �摜�`���̏ꍇ
	else if (_wcsicmp(pExt, L"jpg") == 0 ||
		     _wcsicmp(pExt, L"png") == 0 ||
			 _wcsicmp(pExt, L"gif") == 0 ||
			 _wcsicmp(pExt, L"tiff") == 0 ||
 			 _wcsicmp(pExt, L"bmp") == 0) {
		GetTempPath(sizeof(szTempPath)/sizeof(WCHAR), szTempPath);
		if (!MyGetTempFileName(szTempPath, L".pdf", szTempPdf)) {
			exitcode = 3;
			goto EXIT_CHILD;
		}
		if (!Ps2Pdf(szTempPs, szTempPdf, TIMEOUT) ||
			!Pdf2Image(szTempPdf, szOutFileName, pExt, TIMEOUT)) {
			MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"�摜�o�͒��ɃG���[���������܂���");
		}
		if (GetFileAttributes(szTempPdf) != 0xFFFFFFFF) {
			DeleteFile(szTempPdf);
		}
	}
	else {
		MB(hFgWnd, MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND, L"���̌`���ɂ͑Ή����Ă��܂���");
		exitcode = 4;
		goto EXIT_CHILD;
	}

EXIT_CHILD:
	AppEndWaitMsg();
	dbg(L"child: exitcode=%d", exitcode);
	return exitcode;
}
