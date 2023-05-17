//
// JL�X�N���v�g �O���[�o����ԕێ�
//
// �N���X�\��
//   JlsScrGlobal    : �O���[�o����ԕێ�
//     |- JlsScrReg  : Set/Default�R�}���h�ɂ�郌�W�X�^�l�̕ێ�
//     |- JlsScrMem  : �x�����s�R�}���h�̕ۊ�
//
///////////////////////////////////////////////////////////////////////
#pragma once

#include "JlsScrReg.hpp"
#include "JlsScrMem.hpp"


///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g �O���[�o����ԕێ��N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScrGlobal
{
public:
	// �t�@�C���o��
	bool fileOpen(const string& strName, bool flagAppend);
	bool fileSetCodeDefault(const string& str);
	int  fileGetCodeDefaultNum();
	bool fileSetCodeNum(const string& str);
	void fileClose();
	void fileOutput(const string& strBuf);
	void fileMemoOnly(bool flag);
	void fileMemoFlush();
	bool fileIsOpen();
	// �t�@�C������
	bool readGOpen(const string& strName);
	void readGClose();
	int  readGCodeNum();
	bool readGLine(string& strLine);
	bool readLineIfs(string& strLine, LocalIfs& ifs);

	// ���W�X�^�A�N�Z�X
	int  setLocalRegCreateCall();
	int  setLocalRegCreateFunc();
	int  setLocalRegCreateOne();
	int  setLocalRegReleaseAny();
	int  setLocalRegReleaseCall();
	int  setLocalRegReleaseFunc();
	int  setLocalRegReleaseOne();
	int  getLocalRegLayer();
	bool setRegVarCommon(const string& strName, const string& strVal, bool overwrite, bool flagLocal);
	bool unsetRegVar(const string& strName, bool flagLocal);
	int  getRegVarCommon(string& strVal, const string& strCandName, bool exact);
	bool setArgReg(const string& strName, const string& strVal);
	bool setArgRefReg(const string& strName, const string& strVal);
	void setArgFuncName(const string& strName);
	void setLocalOnly(bool flag);
	void setIgnoreCase(bool valid);
	void setGlobalLock(const string& strName, bool flag);
	bool checkErrRegName(const string& strName);
	void checkRegError(bool flagDisp);
	void clearRegError();

	// �x�����s�ۊǗ̈�ւ̃A�N�Z�X�istate����̃R�}���h���X���[�j
	bool isLazyExist(LazyType typeLazy);
	bool popListByLazy(queue <string>& queStr, LazyType typeLazy);
	bool getListByName(queue <string>& queStr, const string& strName);
	// �x�����s�ۊǗ̈�ւ̃A�N�Z�X
	void setOrderStore(int order);
	void resetOrderStore();
	bool setMemDefArg(vector<string>& argDef);
	bool getMemDefArg(vector<string>& argDef, const string& strName);
	void setMemUnusedFlag(const string& strName);
	void checkMemUnused();
	bool setLazyStore(LazyType typeLazy, const string& strBuf);
	bool setMemStore(const string& strName, const string& strBuf);
	bool setMemErase(const string& strName);
	bool setMemCopy(const string& strSrc, const string& strDst);
	bool setMemMove(const string& strSrc, const string& strDst);
	bool setMemAppend(const string& strSrc, const string& strDst);
	void setMemEcho(const string& strName);
	void setMemGetMapForDebug();

	// �G���[����
	void checkErrorGlobalState(bool flagDisp);

	//--- �ʃf�[�^ ---
	void setExe1st(bool flag);
	bool isExe1st();
	void setCmdExit(bool flag);
	bool isCmdExit();
	void setLazyStateIniAuto(bool flag);
	bool isLazyStateIniAuto();
	void setFullPathJL(const string& msg);
	string getFullPathJL();
	void setPathNameJL(const string& msg);
	string getPathNameJL();
	void setMsgBufForErr(const string& msg);
	string getMsgBufForErr();
	void addMsgError(const string& msg);
	void addMsgErrorN(const string& msg);
	void checkMsgError(bool flagDisp);
	void stopAddMsgError(bool flag);
	bool isStopAddMsgError();

private:
	//--- �ێ��f�[�^�N���X ---
	JlsScrReg    regvar;				// set/default�R�}���h�ɂ��ϐ��l�̕ێ�
	JlsScrMem    memcmd;				// �x������p�̃R�}���h�E�L���̈�ێ�

	//--- �ʃf�[�^ ---
	bool m_exe1st         = true;	// ���s����̐ݒ�p
	bool m_exit           = false;	// Exit�I���t���O
	bool m_lazyStIniAuto  = false;	// LazyFlush�ɂ�鋭��Auto�����s���
	bool m_stopMsgErr     = false;	// �G���[���b�Z�[�W�ǉ����ꎞ�I�ɒ�~
	string m_pathFullJL   = "";		// JL�X�N���v�g�̃t���p�X
	string m_pathNameJL   = "";		// JL�X�N���v�g��Path
	string m_msgBufLine   = "";		// �R�}���h���C��������i�G���[�\���p�j
	string m_msgErr       = "";		// �G���[���b�Z�[�W
	//--- �t�@�C���o�͗p ---
	LocalOfs m_ofsScr;		// �t�@�C���o�͏��ێ�
	int  m_outCodeNum     = 0;		// ���ɐݒ肷�镶���R�[�h�ԍ��i0�͐ݒ�Ȃ��j
	bool m_outMemoOnly    = false;	// ���O�̂ݏo��
	//--- �t�@�C�����͗p ---
	LocalIfs m_ifsScr;		// �t�@�C�����͏��ێ�
};

//--- �ʃf�[�^�P���A�N�Z�X ---
inline void JlsScrGlobal::setExe1st(bool flag){
	m_exe1st = flag;
}
inline bool JlsScrGlobal::isExe1st(){
	return m_exe1st;
}
inline void JlsScrGlobal::setCmdExit(bool flag){
	m_exit = flag;
}
inline bool JlsScrGlobal::isCmdExit(){
	return m_exit;
}
inline void JlsScrGlobal::setLazyStateIniAuto(bool flag){
	m_lazyStIniAuto = flag;
}
inline bool JlsScrGlobal::isLazyStateIniAuto(){
	return m_lazyStIniAuto;
}
inline void JlsScrGlobal::setFullPathJL(const string& msg){
	m_pathFullJL = msg;
}
inline string JlsScrGlobal::getFullPathJL(){
	return m_pathFullJL;
}
inline void JlsScrGlobal::setPathNameJL(const string& msg){
	m_pathNameJL = msg;
}
inline string JlsScrGlobal::getPathNameJL(){
	return m_pathNameJL;
}
inline void JlsScrGlobal::setMsgBufForErr(const string& msg){
	m_msgBufLine = msg;
}
inline string JlsScrGlobal::getMsgBufForErr(){
	return m_msgBufLine;
}
inline void JlsScrGlobal::addMsgError(const string& msg){
	if ( m_stopMsgErr == false ){
		m_msgErr += msg;
	}
}
inline void JlsScrGlobal::addMsgErrorN(const string& msg){
	if ( m_stopMsgErr == false ){
		m_msgErr += msg;
		m_msgErr += "\n";
	}
}
inline void JlsScrGlobal::checkMsgError(bool flagDisp){
	if ( flagDisp ){
		lcerr << m_msgErr;
	}
	m_msgErr = "";
}
inline void JlsScrGlobal::stopAddMsgError(bool flag){
	m_stopMsgErr = flag;
}
inline bool JlsScrGlobal::isStopAddMsgError(){
	return m_stopMsgErr;
}
