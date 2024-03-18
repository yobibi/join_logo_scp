//
// JLコマンド実行用
//
// クラス構成
//   JlsScript           : JLコマンド実行
//     |-JlsScrGlobal    : グローバル状態保持（実体格納）
//     |-JlsScriptDecode : コマンド文字列解析
//     |-JlsScriptLimit  : 引数条件からターゲット限定
//     |-JlsAutoScript   : Auto系JLコマンド実行
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
	// コマンド実行開始時設定
	bool makeFullPath(string& strFull, const string& strSrc, bool flagFull);
	bool makeFullPathIsExist(const string& str);
	// コマンド実行開始処理
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
	// ロゴ直接設定
	void setLogoDirect(JlsCmdArg& cmdarg);
	void setLogoDirectString(const string& strList);
	void setLogoReset();
	// CallとMemory用引数追加処理
	bool setArgAreaDefault(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool makeArgMemStore(JlsCmdArg& cmdarg, JlsScriptState& state);
	void makeArgMemStoreLocalSet(JlsScriptState& state, const string& strName, const string& strVal);
	bool makeArgMemStoreByDefault(JlsScriptState& state);
	bool makeArgMemStoreByMemSet(JlsCmdArg& cmdarg, JlsScriptState& state);
	bool makeArgMemFunc(JlsCmdArg& cmdarg, JlsScriptState& state);
	// 設定コマンド処理
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
	// コマンド実行処理
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
	//--- 関数 ---
	JlsDataset *pdata;									// 入力データアクセス
	unique_ptr <JlsAutoScript>    m_funcAutoScript;		// 自動構成推測スクリプト
	unique_ptr <JlsScriptDecode>  m_funcDecode;			// コマンド文字列解析
	unique_ptr <JlsScriptLimit>   m_funcLimit;			// 制約条件によるターゲット限定

	//--- レジスタアクセス処理 ---
	JlsScrFuncReg  funcReg;
	JlsScrFuncList funcList;
	//--- グローバル制御状態 ---
	JlsScrGlobal globalState;
};
