//
// Sd.cpp  �Z�L�����e�B�f�B�X�N���v�^�̑���
//
// Copyright (C) 2010 KLab Inc.
//
#include "stdafx.h"

// �Z�L�����e�B�f�B�X�N���v�^�� Everyone �ւ̋��� ACE ���Z�b�g��
// �������� DACL �ւ̃|�C���^��Ԃ�
PACL MySetSecurityDescriptorDacl(IN OUT PSECURITY_DESCRIPTOR pSd, IN DWORD dwMask) {

	PSID pSid = NULL;
	PACL pDacl = NULL;
	DWORD dwLen;
	// SECURITY_WORLD_RID �ɑΉ�
	SID_IDENTIFIER_AUTHORITY IdAuthority = SECURITY_WORLD_SID_AUTHORITY;
	BOOL bSts = FALSE;

	// �Z�L�����e�B�f�B�X�N���v�^������
	if (!InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION)) {
		dbg(L"MySetSecurityDescriptorDacl: InitializeSecurityDescriptor err=%u", GetLastError());
		goto DONE;
	}
	// Everyone ���������� SID ���擾
	if (!AllocateAndInitializeSid(&IdAuthority, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSid)) {
		dbg(L"MySetSecurityDescriptorDacl: AllocateAndInitializeSid err=%u",GetLastError());
		goto DONE;
	}
	// DACL �̐����Ə�����
	dwLen = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSid);
	pDacl = (PACL)GlobalAlloc(GPTR, dwLen);
	if (!pDacl) {
		dbg(L"MySetSecurityDescriptorDacl: GlobalAlloc err=%u", GetLastError());
		goto DONE;
	}
	if (!InitializeAcl(pDacl, dwLen, ACL_REVISION)) {
		dbg(L"MySetSecurityDescriptorDacl: InitializeAcl err=%u", GetLastError());
		goto DONE;
	}
	// Everyone �ւ̋��� ACE �� DACL �֒ǉ�
	if (!AddAccessAllowedAce(pDacl, ACL_REVISION, dwMask, pSid)) {
		dbg(L"MySetSecurityDescriptorDacl: AddAccessAllowedAce err=%u", GetLastError());
		goto DONE;
	}
	// �쐬���� DACL ���Z�L�����e�B�f�B�X�N���v�^�ɃZ�b�g
	if (!SetSecurityDescriptorDacl(pSd, TRUE, pDacl, FALSE)) {
		dbg(L"MySetSecurityDescriptorDacl: SetSecurityDescriptorDacl err=%u", GetLastError());
		goto DONE;
	}
	bSts = TRUE;

DONE:
	if (pSid) {
		FreeSid(pSid);
	}
	if (!bSts) { // ���炩�̃G���[
		if (pDacl) {
			GlobalFree(pDacl);
		}
		return NULL;
	}
	return pDacl;
}

// MySetSecurityDescriptorDacl() �Ԓl�� DACL ���J��
void MyFreeDacl(IN OUT PACL pDacl)
{
	if (pDacl) {
		GlobalFree(pDacl);
	}
}
