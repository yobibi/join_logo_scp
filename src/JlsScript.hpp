//
// JL�R�}���h���s�p
//
// �N���X�\��
//   JlsScript           : JL�R�}���h���s
//     |-JlsScrGlobal    : �O���[�o����ԕێ��i���̊i�[�j
//     |-JlsScriptDecode : �R�}���h��������
//     |-JlsScriptState  : ������������^�[�Q�b�g����
//     |-JlsAutoScript   : Auto�nJL�R�}���h���s
//
///////////////////////////////////////////////////////////////////////
#pragma once

#include "JlsScrGlobal.hpp"

class JlsReformData;
class JlsAutoScript;
class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsDataset;
class JlsScriptState;
class JlsScriptDecode;
class JlsScriptLimit;


///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g���s�N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScript
{
public:
	JlsScript(JlsDataset *pdata);
	virtual ~JlsScript();
	int  setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite);
	int  startCmd(const string& fname);

private:
	void checkInitial();
	// ���W�X�^�A�N�Z�X
	bool setJlsRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal);
	void setJlsRegVarCouple(const string& strName, const string& strVal);
	int  getJlsRegVar(string& strVal, const string& strCandName, bool exact);
	// �N���I�v�V��������
	Msec setOptionsCnvCutMrg(const char* str);
	bool setInputReg(const char *name, const char *val, bool overwrite);
	bool setInputFlags(const char *flags, bool overwrite);
	// �R�}���h���s�J�n���ݒ�
	bool makeFullPath(string& strFull, const string& strSrc, bool flagFull);
	bool makeFullPathIsExist(const string& str);
	// �R�}���h���s�J�n����
	int  startCmdLoop(const string& fname, int loop);
	void startCmdLoopLazyEnd(JlsScriptState& state);
	void startCmdLoopLazyOut(JlsScriptState& state);
	void startCmdLoopSub(JlsScriptState& state, const string& strBufOrg, int loop);
	bool startCmdGetLine(ifstream& ifs, string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineOnlyCache(string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineFromFile(ifstream& ifs, string& strBufOrg);
	void startCmdDispErr(const string& strBuf, CmdErrType errval);
	bool replaceBufVar(string& dstBuf, const string& srcBuf);
	int  replaceRegVarInBuf(string& strVal, const string& strBuf, int pos);
	// �x�����s�̐ݒ�
	bool setStateMem(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	bool setStateMemLazy(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	bool isLazyAutoModeInitial(JlsScriptState& state);
	bool setStateMemLazyRevise(LazyType& typeLazy, JlsScriptState& state, JlsCmdArg& cmdarg);
	// �R�}���h��͌�̕ϐ��W�J
	bool expandDecodeCmd(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	int  getCondFlagGetItem(string& strItem, const string& strBuf, int pos);
	bool getCondFlag(bool& flagCond, const string& strBuf);
	void getCondFlagConnectWord(string& strCalc, const string& strItem);
	void getDecodeReg(JlsCmdArg& cmdarg);
	// �V�X�e���ϐ��ݒ�
	void setSystemRegInit();
	void setSystemRegUpdate();
	void setSystemRegHeadtail(int headframe, int tailframe);
	void setSystemRegNologo(bool need_check);
	void setSystemRegLastexe(bool exe_command);
	bool isSystemRegLastexe();
	bool setSystemRegOptions(const string& strBuf, int pos, bool overwrite);
	// �R�}���h���ʂɂ��ϐ��X�V
	void updateResultRegWrite(JlsCmdArg& cmdarg);
	void setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList);
	void setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListIns(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListDel(JlsCmdArg& cmdarg, int numItem);
	int  getListStrSize(const string& strList);
	int  getListStrPos(const string& strList, int num, bool flagIns);
	int  getListStrPosLen(const string& strList, int pos);
	bool getListStrElement(string& strItem, const string& strList, int num);
	bool setListStrIns(string& strList, const string& strItem, int num);
	bool setListStrDel(string& strList, int num);
	// �ݒ�R�}���h����
	bool setCmdCondIf(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdCall(JlsCmdArg& cmdarg, int loop);
	bool setCmdRepeat(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdSys(JlsCmdArg& cmdarg);
	bool setCmdReg(JlsCmdArg& cmdarg);
	bool setCmdMemFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdMemExe(JlsCmdArg& cmdarg, JlsScriptState& state);
	// �R�}���h���s����
	bool exeCmd(JlsCmdSet& cmdset);
	bool exeCmdCallAutoScript(JlsCmdSet& cmdset);
	bool exeCmdCallAutoSetup(JlsCmdSet& cmdset);
	bool exeCmdCallAutoMain(JlsCmdSet& cmdset, bool setup_only);
	bool exeCmdAutoEach(JlsCmdSet& cmdset);
	bool exeCmdLogo(JlsCmdSet& cmdset);
	bool exeCmdLogoTarget(JlsCmdSet& cmdset);
	bool exeCmdNextTail(JlsCmdSet& cmdset);
	bool getMsecTargetWithForce(Msec& msecResult, JlsCmdSet& cmdset);

private:
	//--- �֐� ---
	JlsDataset *pdata;									// ���̓f�[�^�A�N�Z�X
	unique_ptr <JlsAutoScript>    m_funcAutoScript;		// �����\�������X�N���v�g
	unique_ptr <JlsScriptDecode>  m_funcDecode;			// �R�}���h��������
	unique_ptr <JlsScriptLimit>   m_funcLimit;			// ��������ɂ��^�[�Q�b�g����

	//--- �O���[�o�������� ---
	JlsScrGlobal globalState;
};
