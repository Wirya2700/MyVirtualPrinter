//
// main.cpp
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// �X�e�[�^�X�o�[�փ��b�Z�[�W��\��
void MyStatusMessage(HWND hDlg, LPCWSTR pszText)
{
	static HWND hStsBar = NULL;
	if (!hStsBar) {
		hStsBar = GetDlgItem(hDlg, IDC_STATUSBAR);
	}
	SendMessage(hStsBar, SB_SETTEXT, 0, (LPARAM)pszText);
}

// WM_INITDIALOG �n���h��
LRESULT OnInitDialog(HWND hDlg, WPARAM wp, LPARAM lp)
{
	// Print Sppler �T�[�r�X���J�n����Ă��Ȃ���ΊJ�n
	DWORD dwErr;
	DWORD ret = MyQueryServiceStatus(hDlg, L"Spooler", &dwErr);
	if (dwErr == ERROR_SUCCESS) {
		if (ret != SERVICE_RUNNING) {
			MyStartService(hDlg, L"Spooler", &dwErr);
		}
	}
	// �X�e�[�^�X�o�[���쐬
	CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, L"", hDlg, IDC_STATUSBAR);

	// MyVirtualPrinter �C���X�g�[���󋵂��`�F�b�N
	int sts = IsPrinterExist(hDlg, NAME_MYPRINTER);
	if (sts == 0) {
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), FALSE);
		CheckDlgButton(hDlg, IDC_RADIO1, BST_CHECKED);
		MyStatusMessage(hDlg, L"���s�{�^���������Ɠo�^���J�n���܂�");
	} else if (sts == 1) {
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
		CheckDlgButton(hDlg, IDC_RADIO2, BST_CHECKED);
		MyStatusMessage(hDlg, L"���s�{�^���������ƍ폜���J�n���܂�");
	} else {
		MB(hDlg, MB_OK|MB_ICONINFORMATION, L"���m�̃G���[");
		SendMessage(hDlg, WM_CLOSE, 0, 0);
	}
	return TRUE;
}

// WM_COMMAND �n���h��
LRESULT OnCommand(HWND hDlg, WPARAM wp, LPARAM lp)
{
	HWND hOK = GetDlgItem(hDlg, IDOK);
	HWND hCANCEL = GetDlgItem(hDlg, IDCANCEL);
	HWND hR1 = GetDlgItem(hDlg, IDC_RADIO1);
	HWND hR2 = GetDlgItem(hDlg, IDC_RADIO2);
	BOOL bDoInstall;

	switch (LOWORD(wp)) {
	case IDOK:
		if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
			bDoInstall = TRUE;
		} else {
			bDoInstall = FALSE;
		}
		if (bDoInstall && IDNO == MB(hDlg, MB_YESNO|MB_ICONQUESTION,
								NAME_MYPRINTER L" ���V�X�e���ɓo�^���܂��B��낵���ł����H")) {
			return TRUE;
		}
		if (!bDoInstall && IDNO == MB(hDlg, MB_YESNO|MB_ICONQUESTION,
								NAME_MYPRINTER L" ���V�X�e������폜���܂��B��낵���ł����H")) {
			return TRUE;
		}
		EnableWindow(hOK, FALSE);
		EnableWindow(hCANCEL, FALSE);
		EnableWindow(hR1, FALSE);
		EnableWindow(hR2, FALSE);
		UpdateWindow(hDlg);

		if (bDoInstall) {
			MyStatusMessage(hDlg, L"�|�[�g���j�^��ݒ肵�Ă��܂�");
			if (!InstallMyPortMonitor(hDlg)) {
				MyStatusMessage(hDlg, L"�|�[�g���j�^�ݒ蒆�ɃG���[���������܂���");
				goto ERR;
			}
			MyStatusMessage(hDlg, L"��p�|�[�g��ǉ����Ă��܂�");
			if (!CreateMyPort(hDlg)) {
				MyStatusMessage(hDlg, L"�|�[�g�ǉ����ɃG���[���������܂���");
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MyStatusMessage(hDlg, L"�v�����^�h���C�o��ݒ肵�Ă��܂�");
			if (!InstallMyPrinterDriver(hDlg)) {
				MyStatusMessage(hDlg, L"�v�����^�h���C�o�ݒ蒆�ɃG���[���������܂���");
				DeleteMyPort(hDlg, FALSE);
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MyStatusMessage(hDlg, L"���z�v�����^��o�^���Ă��܂�");
			if (!CreateMyPrinter(hDlg)) {
				MyStatusMessage(hDlg, L"�v�����^�o�^���ɃG���[���������܂���");
				DeleteMyPrinterDriver(hDlg, FALSE);
				DeleteMyPort(hDlg, FALSE);
				DeleteMyPortMonitor(hDlg, FALSE);
				goto ERR;
			}
			MB(hDlg, MB_OK|MB_ICONINFORMATION, 
				L"���z�v�����^�u" NAME_MYPRINTER L"�v���V�X�e���ɓo�^���܂����B"
				L"\n\n"
				NAME_MYPRINTER L" ���V�X�e������폜����܂ł́A"
				L"\n\n"
				L"���݂̃t�H���_���폜���Ȃ��ŉ������B");
		}
		else {
			BOOL sts = TRUE;
			DWORD err;
			MyStatusMessage(hDlg, L"�X�v�[���T�[�r�X���ċN�����Ă��܂�");
			MyRestartService(hDlg, L"Spooler", &err);

			MyStatusMessage(hDlg, L"���z�v�����^���폜���Ă��܂�");
			if (!DeleteMyPrinter(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"�v�����^�h���C�o���Đݒ肵�Ă��܂�");
			if (!DeleteMyPrinterDriver(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"��p�|�[�g���폜���Ă��܂�");
			if (!DeleteMyPort(hDlg, TRUE)) {
				sts = FALSE;
			}
			MyStatusMessage(hDlg, L"�|�[�g���j�^���Đݒ肵�Ă��܂�");
			if (!DeleteMyPortMonitor(hDlg, TRUE)) {
				sts = FALSE;
			}
			if (sts) {
				MB(hDlg, MB_OK|MB_ICONINFORMATION, L"���z�v�����^�u" NAME_MYPRINTER L"�v���V�X�e������폜���܂���");
			}
		}
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		return TRUE;

	case IDCANCEL:
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;
	}
	return TRUE;

ERR:
	EnableWindow(hCANCEL, TRUE);
	return TRUE;
}

// �_�C�A���O�v���V�[�W��
LRESULT CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		return OnInitDialog(hDlg, wp, lp);

	case WM_COMMAND:
		return OnCommand(hDlg, wp, lp);

	case WM_SYSCOMMAND:
		return (wp == SC_CLOSE) ? TRUE : FALSE;

	case WM_CLOSE:
		EndDialog(hDlg, IDOK);
		return TRUE;

	default:
		return FALSE;
	}
	return FALSE;
}

// WinMain
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine, int nCmdShow)
{
	if (!IsTargetPlatform()) {
		MB(NULL, MB_OK, L"���̃v���b�g�t�H�[���ɂ͑Ή����Ă��܂���");
		return 0;
	}
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgProc);

	return 0;
}
