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
	//--- �����R�[�h�ԍ��ݒ肪����΍X�V ---
	if ( m_outCodeNum > 0 ){
		m_ofsScr.setCodeTypeFromNum(m_outCodeNum);
		m_outCodeNum = 0;		// �ݒ�Ȃ��ɖ߂�
	}
	//--- ����Open���Ă�����Close ---
	if ( m_ofsScr.is_open() ){
		m_ofsScr.close();
	}
	//--- �I�[�v�� ---
	if ( flagAppend ){
		m_ofsScr.append(strName);
	}else{
		m_ofsScr.open(strName);
	}
	//--- �m�F ---
	if ( !m_ofsScr.is_open() ){
		return false;
	}
	return true;
}
//--- �W���̕����R�[�h�ԍ���ݒ� ---
bool JlsScrGlobal::fileSetCodeDefault(const string& str){
	int num = LSys.getUtfNumFromStr(str);
	if ( num < 0 ) return false;
	LSys.setFileUtfCodeFromNum(num);
	return true;
}
int JlsScrGlobal::fileGetCodeDefaultNum(){
	return LSys.getFileUtfNum();
}
//--- ���ɐݒ肷�镶���R�[�h�ԍ���ݒ� ---
bool JlsScrGlobal::fileSetCodeNum(const string& str){
	int num = LSys.getUtfNumFromStr(str);
	if ( num < 0 ) return false;
	m_outCodeNum = num;
	return true;
}
//--- �t�@�C���N���[�Y ---
void JlsScrGlobal::fileClose(){
	m_ofsScr.close();
}
//--- ��������o�� ---
void JlsScrGlobal::fileOutput(const string& strBuf){
	if ( m_ofsScr.is_open() ){
		m_ofsScr.write(strBuf);
	}else if ( m_outMemoOnly ){	// ���������݂̂ɏo��
		LSys.bufMemoIns(strBuf);
	}else{
		lcout << strBuf;
	}
}
//--- �W���o�͂̂����ɓ��������݂̂ɏo�͂���ݒ� ---
void JlsScrGlobal::fileMemoOnly(bool flag){
	m_outMemoOnly = flag;
}
//--- �����������t�@�C���ɏo�� ---
void JlsScrGlobal::fileMemoFlush(){
	LSys.bufMemoFlush(m_ofsScr);
}
//--- �o�͐悪�t�@�C���� ---
bool JlsScrGlobal::fileIsOpen(){
	return ( m_ofsScr.is_open() );
}
//=====================================================================
// �t�@�C�����͏���
//=====================================================================

//--- �t�@�C���I�[�v�� ---
bool JlsScrGlobal::readGOpen(const string& strName){
	//--- ����Open���Ă�����Close ---
	if ( m_ifsScr.is_open() ){
		m_ifsScr.close();
	}
	//--- �I�[�v�� ---
	m_ifsScr.open(strName);
	//--- �m�F ---
	if ( !m_ifsScr.is_open() ){
		addMsgError("error : file read open(" + strName + ")\n");
		return false;
	}
	return true;
}
//--- �t�@�C���N���[�Y ---
void JlsScrGlobal::readGClose(){
	m_ifsScr.close();
}
//--- �t�@�C���̕����R�[�h�ԍ����擾 ---
int JlsScrGlobal::readGCodeNum(){
	return m_ifsScr.getCodeNum();
}
//--- �������1�s���� ---
bool JlsScrGlobal::readGLine(string& strLine){
	if ( !m_ifsScr.is_open() ){
		return false;
	}
	return readLineIfs(strLine, m_ifsScr);
}
//--- �������1�s���́i�t�@�C�����w��j ---
bool JlsScrGlobal::readLineIfs(string& strLine, LocalIfs& ifs){
	strLine.clear();
	if ( ifs.getline(strLine) ){
		auto len = strLine.length();
		if ( len >= INT_MAX/4 ){		// �ʓ|���͍ŏ��ɃJ�b�g
			strLine.clear();
			return false;
		}
		return true;
	}
	return false;
}

//=====================================================================
// ���W�X�^�A�N�Z�X����
//=====================================================================

//---------------------------------------------------------------------
// ��ގw��Ń��[�J���ϐ��K�w�쐬
// �o�́F
//   �Ԃ�l    : �쐬�K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegCreateCall(){
	return regvar.createLocalCall();
}
int JlsScrGlobal::setLocalRegCreateFunc(){
	return regvar.createLocalFunc();
}
int JlsScrGlobal::setLocalRegCreateOne(){
	return regvar.createLocalOne();
}
//---------------------------------------------------------------------
// ��ގw��Ń��[�J���ϐ��K�w�̏I��
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegReleaseAny(){
	return regvar.releaseLocalAny();
}
int JlsScrGlobal::setLocalRegReleaseCall(){
	return regvar.releaseLocalCall();
}
int JlsScrGlobal::setLocalRegReleaseFunc(){
	return regvar.releaseLocalFunc();
}
int JlsScrGlobal::setLocalRegReleaseOne(){
	return regvar.releaseLocalOne();
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�̎擾
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrGlobal::getLocalRegLayer(){
	return regvar.getLocalLayer();
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
//--- �ϐ��̖���`�� ---
bool JlsScrGlobal::unsetRegVar(const string& strName, bool flagLocal){
	return regvar.unsetRegVar(strName, flagLocal);
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
// Call�ň����Ƃ��Ďg����ϐ���ݒ�
// ���́F
//   strName : �����Ɏg����ϐ���
//   strVal  : �����Ɏg����ϐ��l
//---------------------------------------------------------------------
bool JlsScrGlobal::setArgReg(const string& strName, const string& strVal){
	return regvar.setArgReg(strName, strVal);
}
//--- �Q�Ɠn�����W�X�^�ݒ� ---
bool JlsScrGlobal::setArgRefReg(const string& strName, const string& strVal){
	return regvar.setArgRefReg(strName, strVal);
}
//---  �Ԃ�l�ϐ��ƂȂ�֐�����ݒ� ---
void JlsScrGlobal::setArgFuncName(const string& strName){
	regvar.setArgFuncName(strName);
}
//---------------------------------------------------------------------
// �ǂݏo���ŃO���[�o���ϐ������Ȃ��ݒ�
// ���́F
//   flag : ���[�J���ϐ��ɂȂ����̃O���[�o���ϐ��Q�Ɓifalse=����  true=�֎~�j
//---------------------------------------------------------------------
void JlsScrGlobal::setLocalOnly(bool flag){
	regvar.setLocalOnly(flag);
}
//--- �ϐ��̑啶���������𖳎����邩 ---
void JlsScrGlobal::setIgnoreCase(bool valid){
	regvar.setIgnoreCase(valid);
}
//--- ����������warning���o���O���[�o���ϐ��ݒ� ---
void JlsScrGlobal::setGlobalLock(const string& strName, bool flag){
	regvar.setGlobalLock(strName, flag);
}
//--- ���W�X�^�����`�F�b�N ---
bool JlsScrGlobal::checkErrRegName(const string& strName){
	bool silent = true;		// �g�p�O�̃`�F�b�N�ł̓G���[��\�����Ȃ�
	return regvar.checkErrRegName(strName, silent);
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�`�F�b�N
//---------------------------------------------------------------------
void JlsScrGlobal::checkRegError(bool flagDisp){
	string msg;
	if ( regvar.popMsgError(msg) ){		// �G���[���b�Z�[�W���ݎ��̏o��
		if ( flagDisp ){
			lcerr << msg;
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
// �R�}���h�ۊǎ��̎��s���ʁE������`��ݒ�
//---------------------------------------------------------------------
void JlsScrGlobal::setOrderStore(int order){
	memcmd.setOrderForPush(order);
}
void JlsScrGlobal::resetOrderStore(){
	memcmd.resetOrderForPush();
}
bool JlsScrGlobal::setMemDefArg(vector<string>& argDef){
	return memcmd.setDefArg(argDef);
}
bool JlsScrGlobal::getMemDefArg(vector<string>& argDef, const string& strName){
	return memcmd.getDefArg(argDef, strName);
}
void JlsScrGlobal::setMemUnusedFlag(const string& strName){
	memcmd.setUnusedFlag(strName);
}
void JlsScrGlobal::checkMemUnused(){
	string msg;
	if ( memcmd.getUnusedStr(msg) ){
		addMsgError(msg);
	}
	checkMsgError(true);
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
	lcout << strBuf;
}


//---------------------------------------------------------------------
// �G���[�`�F�b�N
//---------------------------------------------------------------------
void JlsScrGlobal::checkErrorGlobalState(bool flagDisp){
	checkMsgError(flagDisp);
	checkRegError(flagDisp);
}
