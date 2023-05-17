//
// JL�R�}���h���s�p
//
// �N���X�\��
//   JlsScript           : JL�R�}���h���s
//     |-JlsScrGlobal    : �O���[�o����ԕێ��i���̊i�[�j
//     |-JlsScriptDecode : �R�}���h��������
//     |-JlsScriptLimit  : ������������^�[�Q�b�g����
//     |-JlsAutoScript   : Auto�nJL�R�}���h���s
//
///////////////////////////////////////////////////////////////////////
#pragma once

#include "JlsScrFuncReg.hpp"
#include "JlsScrFuncList.hpp"
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
	// �R�}���h���s�J�n���ݒ�
	bool makeFullPath(string& strFull, const string& strSrc, bool flagFull);
	bool makeFullPathIsExist(const string& str);
	// �R�}���h���s�J�n����
	int  startCmdEnter(const string& fnameMain, const string& fnameSetup);
	int  startCmdLoop(const string& fname, int loop);
	void startCmdLoopLazyEnd(JlsScriptState& state);
	void startCmdLoopLazyOut(JlsScriptState& state, const string& name);
	void startCmdLoopSub(JlsScriptState& state, const string& strBufOrg, int loop);
	bool startCmdGetLine(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineOnlyCache(string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineFromFile(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineFromFileDivCache(string& strBufOrg, JlsScriptState& state);
	bool startCmdGetLineFromFileParseDiv(string& strBufOrg, JlsScriptState& state);
	void startCmdDispErr(const string& strBuf, CmdErrType errval);
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
	// ���S���ڐݒ�
	void setLogoDirect(JlsCmdArg& cmdarg);
	void setLogoDirectString(const string& strList);
	void setLogoReset();
	// Call��Memory�p�����ǉ�����
	bool setArgAreaDefault(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool makeArgMemStore(JlsCmdArg& cmdarg, JlsScriptState& state);
	void makeArgMemStoreLocalSet(JlsScriptState& state, const string& strName, const string& strVal);
	bool makeArgMemStoreByDefault(JlsScriptState& state);
	bool makeArgMemStoreByMemSet(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool makeArgMemFunc(JlsCmdArg& cmdarg, JlsScriptState& state);
	// �ݒ�R�}���h����
	bool setCmdCondIf(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdCall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop);
	bool taskCmdCall(string strName, int loop, bool fcall);
	bool taskCmdFcall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop);
	bool setCmdRepeat(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdSys(JlsCmdArg& cmdarg);
	bool setCmdRead(JlsCmdArg& cmdarg);
	void setCmdEchoTextFile(const string& fname);
	void setCmdEchoResultTrim();
	void setCmdEchoResultDetail();
	bool setCmdReg(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdMemFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdMemExe(JlsCmdArg& cmdarg, JlsScriptState& state);
	// �R�}���h���s����
	bool exeCmd(JlsCmdSet& cmdset);
	bool exeCmdCallAutoScript(JlsCmdSet& cmdset);
	bool exeCmdCallAutoSetup(JlsCmdSet& cmdset);
	bool exeCmdCallAutoMain(JlsCmdSet& cmdset, bool setup_only);
	bool exeCmdAutoEach(JlsCmdSet& cmdset);
	bool exeCmdLogo(JlsCmdSet& cmdset);
	bool exeCmdLogoIsValidExe(JlsCmdSet& cmdset);
	bool exeCmdLogoFromCat(JlsCmdSet& cmdset);
	bool exeCmdLogoTarget(JlsCmdSet& cmdset);
	bool exeCmdNextTail(JlsCmdSet& cmdset);
	bool getMsecTargetDst(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp);
	bool getMsecTargetEnd(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp);
	bool getMsecTargetSub(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp, bool flagEnd);
	bool getRangeMsecTarget(RangeMsec& rmsecResult, JlsCmdSet& cmdset);

	bool isFcallName(const string& str){ return (str.substr(0,2)=="::"); };
	string setFcallName(const string& str){ return "::"+str; };
	string getFcallName(const string& str){ return str.substr(2); };

private:
	//--- �֐� ---
	JlsDataset *pdata;									// ���̓f�[�^�A�N�Z�X
	unique_ptr <JlsAutoScript>    m_funcAutoScript;		// �����\�������X�N���v�g
	unique_ptr <JlsScriptDecode>  m_funcDecode;			// �R�}���h��������
	unique_ptr <JlsScriptLimit>   m_funcLimit;			// ��������ɂ��^�[�Q�b�g����

	//--- ���W�X�^�A�N�Z�X���� ---
	JlsScrFuncReg  funcReg;
	JlsScrFuncList funcList;
	//--- �O���[�o�������� ---
	JlsScrGlobal globalState;
};
