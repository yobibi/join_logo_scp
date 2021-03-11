//
// JLコマンド実行用
//
// クラス構成
//   JlsScript           : JLコマンド実行
//     |-JlsScrGlobal    : グローバル状態保持（実体格納）
//     |-JlsScriptDecode : コマンド文字列解析
//     |-JlsScriptState  : 引数条件からターゲット限定
//     |-JlsAutoScript   : Auto系JLコマンド実行
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
// JLスクリプト実行クラス
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
	// レジスタアクセス
	bool setJlsRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal);
	void setJlsRegVarCouple(const string& strName, const string& strVal);
	int  getJlsRegVar(string& strVal, const string& strCandName, bool exact);
	// 起動オプション処理
	Msec setOptionsCnvCutMrg(const char* str);
	bool setInputReg(const char *name, const char *val, bool overwrite);
	bool setInputFlags(const char *flags, bool overwrite);
	// コマンド実行開始時設定
	bool makeFullPath(string& strFull, const string& strSrc, bool flagFull);
	bool makeFullPathIsExist(const string& str);
	// コマンド実行開始処理
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
	// 遅延実行の設定
	bool setStateMem(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	bool setStateMemLazy(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	bool isLazyAutoModeInitial(JlsScriptState& state);
	bool setStateMemLazyRevise(LazyType& typeLazy, JlsScriptState& state, JlsCmdArg& cmdarg);
	// コマンド解析後の変数展開
	bool expandDecodeCmd(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf);
	int  getCondFlagGetItem(string& strItem, const string& strBuf, int pos);
	bool getCondFlag(bool& flagCond, const string& strBuf);
	void getCondFlagConnectWord(string& strCalc, const string& strItem);
	void getDecodeReg(JlsCmdArg& cmdarg);
	// システム変数設定
	void setSystemRegInit();
	void setSystemRegUpdate();
	void setSystemRegHeadtail(int headframe, int tailframe);
	void setSystemRegNologo(bool need_check);
	void setSystemRegLastexe(bool exe_command);
	bool isSystemRegLastexe();
	bool setSystemRegOptions(const string& strBuf, int pos, bool overwrite);
	// コマンド結果による変数更新
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
	// 設定コマンド処理
	bool setCmdCondIf(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdCall(JlsCmdArg& cmdarg, int loop);
	bool setCmdRepeat(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdSys(JlsCmdArg& cmdarg);
	bool setCmdReg(JlsCmdArg& cmdarg);
	bool setCmdMemFlow(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool setCmdMemExe(JlsCmdArg& cmdarg, JlsScriptState& state);
	// コマンド実行処理
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
	//--- 関数 ---
	JlsDataset *pdata;									// 入力データアクセス
	unique_ptr <JlsAutoScript>    m_funcAutoScript;		// 自動構成推測スクリプト
	unique_ptr <JlsScriptDecode>  m_funcDecode;			// コマンド文字列解析
	unique_ptr <JlsScriptLimit>   m_funcLimit;			// 制約条件によるターゲット限定

	//--- グローバル制御状態 ---
	JlsScrGlobal globalState;
};
