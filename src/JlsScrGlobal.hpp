//
// JLスクリプト グローバル状態保持
//
// クラス構成
//   JlsScrGlobal    : グローバル状態保持
//     |- JlsScrReg  : Set/Defaultコマンドによるレジスタ値の保持
//     |- JlsScrMem  : 遅延実行コマンドの保管
//
///////////////////////////////////////////////////////////////////////
#pragma once

#include "JlsScrReg.hpp"
#include "JlsScrMem.hpp"


///////////////////////////////////////////////////////////////////////
//
// JLスクリプト グローバル状態保持クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScrGlobal
{
public:
	// ファイル出力
	bool fileOpen(const string& strName, bool flagAppend);
	bool fileSetCodeDefault(const string& str);
	int  fileGetCodeDefaultNum();
	bool fileSetCodeNum(const string& str);
	void fileClose();
	void fileOutput(const string& strBuf);
	void fileMemoOnly(bool flag);
	void fileMemoFlush();
	bool fileIsOpen();
	// ファイル入力
	bool readGOpen(const string& strName);
	void readGClose();
	int  readGCodeNum();
	bool readGLine(string& strLine);
	bool readLineIfs(string& strLine, LocalIfs& ifs);

	// レジスタアクセス
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

	// 遅延実行保管領域へのアクセス（stateからのコマンドをスルー）
	bool isLazyExist(LazyType typeLazy);
	bool popListByLazy(queue <string>& queStr, LazyType typeLazy);
	bool getListByName(queue <string>& queStr, const string& strName);
	// 遅延実行保管領域へのアクセス
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

	// エラー処理
	void checkErrorGlobalState(bool flagDisp);

	//--- 個別データ ---
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
	//--- 保持データクラス ---
	JlsScrReg    regvar;				// set/defaultコマンドによる変数値の保持
	JlsScrMem    memcmd;				// 遅延動作用のコマンド・記憶領域保持

	//--- 個別データ ---
	bool m_exe1st         = true;	// 実行初回の設定用
	bool m_exit           = false;	// Exit終了フラグ
	bool m_lazyStIniAuto  = false;	// LazyFlushによる強制Auto未実行状態
	bool m_stopMsgErr     = false;	// エラーメッセージ追加を一時的に停止
	string m_pathFullJL   = "";		// JLスクリプトのフルパス
	string m_pathNameJL   = "";		// JLスクリプトのPath
	string m_msgBufLine   = "";		// コマンドライン文字列（エラー表示用）
	string m_msgErr       = "";		// エラーメッセージ
	//--- ファイル出力用 ---
	LocalOfs m_ofsScr;		// ファイル出力情報保持
	int  m_outCodeNum     = 0;		// 次に設定する文字コード番号（0は設定なし）
	bool m_outMemoOnly    = false;	// ログのみ出力
	//--- ファイル入力用 ---
	LocalIfs m_ifsScr;		// ファイル入力情報保持
};

//--- 個別データ単純アクセス ---
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
