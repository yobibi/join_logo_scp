//
// JL�X�N���v�g �O���[�o����ԕێ�
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrGlobal.hpp"

///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g �O���[�o����ԕێ��N���X
//
///////////////////////////////////////////////////////////////////////

//=====================================================================
// �t�@�C���o�͏���
//=====================================================================

//--- �t�@�C���I�[�v�� ---
bool JlsScrGlobal::fileOpen(const string& strName, bool flagAppend){
	//--- ����Open���Ă�����Close ---
	if ( m_flagOfsScr ){
		m_ofsScr.close();
	}
	//--- �I�[�v�� ---
	if ( flagAppend ){
		m_ofsScr.open(strName, ios::app);
	}else{
		m_ofsScr.open(strName);
	}
	//--- �m�F ---
	if ( m_ofsScr ){
		m_flagOfsScr = true;
	}else{
		addMsgError("error : file open(" + strName + ")\n");
		return false;
	}
	return true;
}
//--- �t�@�C���N���[�Y ---
void JlsScrGlobal::fileClose(){
	m_ofsScr.close();
	m_flagOfsScr = false;
}
//--- ��������o�� ---
void JlsScrGlobal::fileOutput(const string& strBuf){
	if ( m_flagOfsScr ){
		m_ofsScr << strBuf;
	}else{
		cout << strBuf;
	}
}

//=====================================================================
// ���W�X�^�A�N�Z�X����
//=====================================================================

//---------------------------------------------------------------------
// �ŏ�ʊK�w�����Ń��[�J���ϐ��K�w�쐬
// �o�́F
//   �Ԃ�l    : �쐬�K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegCreateBase(){
	bool flagBase = true;		// ��ʊK�w�͌����͈͊O�Ƃ���
	return regvar.createLocal(flagBase);
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�쐬
// �o�́F
//   �Ԃ�l    : �쐬�K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegCreateOne(){
	bool flagBase = false;		// ��ʊK�w�������͈�
	return regvar.createLocal(flagBase);
}
//---------------------------------------------------------------------
// �ŏ�ʊK�w�������[�J���ϐ��K�w�̏I��
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegReleaseBase(){
	bool flagBase = true;		// ��ʊK�w�͌����͈͊O�Ƃ���K�w������
	return regvar.releaseLocal(flagBase);
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�̏I��
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegReleaseOne(){
	bool flagBase = false;		// ��ʊK�w�������͈͂̊K�w
	return regvar.releaseLocal(flagBase);
}
//---------------------------------------------------------------------
// �ϐ���ݒ�i�ʏ�A���[�J���ϐ����ʗ��p�j
//---------------------------------------------------------------------
bool JlsScrGlobal::setRegVarCommon(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	bool success;
	if ( flagLocal ){
		//--- ���[�J���ϐ��̃��W�X�^�������� ---
		success = regvar.setLocalRegVar(strName, strVal, overwrite);
	}else{
		//--- �ʏ�̃��W�X�^�������� ---
		success = regvar.setRegVar(strName, strVal, overwrite);
	}
	return success;
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo��
// ���́F
//   strCandName : �ǂݏo���ϐ����i���j
//   excact      : 0=���͕����ɍő�}�b�`����ϐ�  1=���͕����Ɗ��S��v����ϐ�
// �o�́F
//   �Ԃ�l  : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsScrGlobal::getRegVarCommon(string& strVal, const string& strCandName, bool exact){
	//--- �ʏ�̃��W�X�^�ǂݏo�� ---
	return regvar.getRegVar(strVal, strCandName, exact);
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�`�F�b�N
//---------------------------------------------------------------------
void JlsScrGlobal::checkRegError(bool flagDisp){
	string msg;
	if ( regvar.popMsgError(msg) ){		// �G���[���b�Z�[�W���ݎ��̏o��
		if ( flagDisp ){
			cerr << msg;
		}
	}
	checkMsgError(flagDisp);
}
void JlsScrGlobal::clearRegError(){
	bool flagDisp = false;
	checkRegError(flagDisp);
}


//=====================================================================
// �x�����s�ۊǗ̈�ւ̃A�N�Z�X
//=====================================================================

//--- state(JlsScriptState)����̃A�N�Z�X ---
bool JlsScrGlobal::isLazyExist(LazyType typeLazy){
	return memcmd.isLazyExist(typeLazy);
}
bool JlsScrGlobal::popListByLazy(queue <string>& queStr, LazyType typeLazy){
	return memcmd.popListByLazy(queStr, typeLazy);
}
bool JlsScrGlobal::getListByName(queue <string>& queStr, const string& strName){
	return memcmd.getListByName(queStr, strName);
}

//---------------------------------------------------------------------
// lazy�����ɂ��R�}���h�̕ۊ�
// ���́F
//   typeLazy  : LazyS, LazyA, LazyE
//   strBuf   : �ۊǂ��錻�ݍs�̕�����
// �o�́F
//   �Ԃ�l   �F���ݍs�̃R�}���h���s�L���i���s�L���b�V���Ɉڂ������͎��s���Ȃ��j
//---------------------------------------------------------------------
bool JlsScrGlobal::setLazyStore(LazyType typeLazy, const string& strBuf){
	bool enableExe = true;
	//--- Lazy�ɓ����ꍇ�A��荞��Ō��ݍs�͎��s���Ȃ� ---
	if ( typeLazy != LazyType::None ){
		bool success = memcmd.pushStrByLazy(typeLazy, strBuf);
		enableExe = false;				// �ۊǂ���R�}���h�͂��̏�Ŏ��s���Ȃ�
		if ( success == false ){
			addMsgError("error : failed Lazy push: " + strBuf + "\n");
		}
	}
	return enableExe;
}
//---------------------------------------------------------------------
// memory�����ɂ��R�}���h�̕ۊ�
// ���́F
//   strName  : �ۊǗ̈�̎��ʎq
//   strBuf   : �ۊǂ��錻�ݍs�̕�����
// �o�́F
//   �Ԃ�l   �F���ݍs�̃R�}���h���s�L���i���s�L���b�V���Ɉڂ������͎��s���Ȃ��j
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemStore(const string& strName, const string& strBuf){
	bool enableExe = false;			// �ۊǂ���R�}���h�͂��̏�Ŏ��s���Ȃ��i���s���������j
	bool success = memcmd.pushStrByName(strName, strBuf);
	if ( success == false ){
		addMsgError("error : failed memory push: " +  strBuf + "\n");
	}
	return enableExe;
}
//---------------------------------------------------------------------
// �L���̈���폜(MemErase)
// �Ԃ�l   �Ftrue=�����Afalse=���s
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemErase(const string& strName){
	return memcmd.eraseMemByName(strName);
}
//---------------------------------------------------------------------
// �L���̈��ʂ̋L���̈�ɃR�s�[(MemCopy)
// �Ԃ�l   �Ftrue=�����Afalse=���s
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemCopy(const string& strSrc, const string& strDst){
	return memcmd.copyMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// �L���̈��ʂ̋L���̈�Ɉړ�(MemMove)
// �Ԃ�l   �Ftrue=�����Afalse=���s
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemMove(const string& strSrc, const string& strDst){
	return memcmd.moveMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// �ۊǗ̈��ʂ̕ۊǗ̈�ɒǉ�(MemAppend)
// �Ԃ�l   �Ftrue=�����Afalse=���s
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemAppend(const string& strSrc, const string& strDst){
	return memcmd.appendMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// �ۊǗ̈�̓��e��W���o�͂ɕ\��
//---------------------------------------------------------------------
void JlsScrGlobal::setMemEcho(const string& strName){
	queue <string> queStr;
	bool enable_exe = memcmd.getListByName(queStr, strName);
	if ( enable_exe ){
		while( queStr.empty() == false ){
			fileOutput(queStr.front() + "\n");
			queStr.pop();
		}
	}
}
//---------------------------------------------------------------------
// �x�����s�p�̂��ׂĂ̕ۊǓ��e�擾�i�f�o�b�O�p�j
//---------------------------------------------------------------------
void JlsScrGlobal::setMemGetMapForDebug(){
	string strBuf;
	memcmd.getMapForDebug(strBuf);
	cout << strBuf;
}


//---------------------------------------------------------------------
// �G���[�`�F�b�N
//---------------------------------------------------------------------
void JlsScrGlobal::checkErrorGlobalState(bool flagDisp){
	checkMsgError(flagDisp);
	checkRegError(flagDisp);
}
