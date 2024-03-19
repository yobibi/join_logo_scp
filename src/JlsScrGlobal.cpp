//
// JLスクリプト グローバル状態保持
//
//#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrGlobal.hpp"

///////////////////////////////////////////////////////////////////////
//
// JLスクリプト グローバル状態保持クラス
//
///////////////////////////////////////////////////////////////////////

//=====================================================================
// ファイル出力処理
//=====================================================================

//--- ファイルオープン ---
bool JlsScrGlobal::fileOpen(const string& strName, bool flagAppend){
	//--- 文字コード番号設定があれば更新 ---
	if ( m_outCodeNum > 0 ){
		m_ofsScr.setUtfNum(m_outCodeNum);
		m_outCodeNum = 0;		// 設定なしに戻す
	}
	//--- 既にOpenしていたらClose ---
	if ( m_ofsScr.is_open() ){
		m_ofsScr.close();
	}
	//--- オープン ---
	if ( flagAppend ){
		m_ofsScr.append(strName);
	}else{
		m_ofsScr.open(strName);
	}
	//--- 確認 ---
	if ( !m_ofsScr ){
		return false;
	}
	return true;
}
//--- 標準の文字コード番号を設定 ---
bool JlsScrGlobal::fileSetCodeDefault(const string& str){
	int num = LSys.getUtfNumFromStr(str);
	if ( num < 0 ) return false;
	LSys.setFileUtfNum(num);
	return true;
}
int JlsScrGlobal::fileGetCodeDefaultNum(){
	return LSys.getFileUtfNum();
}
//--- 次に設定する文字コード番号を設定 ---
bool JlsScrGlobal::fileSetCodeNum(const string& str){
	int num = LSys.getUtfNumFromStr(str);
	if ( num < 0 ) return false;
	m_outCodeNum = num;
	return true;
}
//--- ファイルクローズ ---
void JlsScrGlobal::fileClose(){
	m_ofsScr.close();
}
//--- 文字列を出力 ---
void JlsScrGlobal::fileOutput(const string& strBuf){
	if ( m_ofsScr.is_open() ){
		m_ofsScr.write(strBuf);
	}else if ( m_outMemoOnly ){	// 内部メモのみに出力
		LSys.bufMemoIns(strBuf);
	}else{
		lcout << strBuf;
	}
}
//--- 標準出力のかわりに内部メモのみに出力する設定 ---
void JlsScrGlobal::fileMemoOnly(bool flag){
	m_outMemoOnly = flag;
}
//--- 内部メモをファイルに出力 ---
void JlsScrGlobal::fileMemoFlush(){
	LSys.bufMemoFlush(m_ofsScr);
}
//--- 出力先がファイルか ---
bool JlsScrGlobal::fileIsOpen(){
	return ( m_ofsScr.is_open() );
}
//=====================================================================
// ファイル入力処理
//=====================================================================

//--- ファイルオープン ---
bool JlsScrGlobal::readGOpen(const string& strName){
	//--- 既にOpenしていたらClose ---
	if ( m_ifsScr.is_open() ){
		m_ifsScr.close();
	}
	//--- オープン ---
	m_ifsScr.open(strName);
	//--- 確認 ---
	if ( !m_ifsScr.is_open() ){
		addMsgError("error : file read open(" + strName + ")\n");
		return false;
	}
	return true;
}
//--- ファイルクローズ ---
void JlsScrGlobal::readGClose(){
	m_ifsScr.close();
}
//--- ファイルの文字コード番号を取得 ---
int JlsScrGlobal::readGCodeNum(){
	return m_ifsScr.getUtfNum();
}
//--- 文字列を1行入力 ---
bool JlsScrGlobal::readGLine(string& strLine){
	if ( !m_ifsScr.is_open() ){
		return false;
	}
	return readLineIfs(strLine, m_ifsScr);
}
//--- 文字列を1行入力（ファイル情報指定） ---
bool JlsScrGlobal::readLineIfs(string& strLine, LocalIfs& ifs){
	strLine.clear();
	if ( ifs.getline(strLine) ){
		auto len = strLine.length();
		if ( len >= INT_MAX/4 ){		// 面倒事は最初にカット
			strLine.clear();
			return false;
		}
		return true;
	}
	return false;
}

//=====================================================================
// レジスタアクセス処理
//=====================================================================

//---------------------------------------------------------------------
// 種類指定でローカル変数階層作成
// 出力：
//   返り値    : 作成階層（0=失敗、1以上=階層）
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
// 種類指定でローカル変数階層の終了
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
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
// ローカル変数階層の取得
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrGlobal::getLocalRegLayer(){
	return regvar.getLocalLayer();
}
//---------------------------------------------------------------------
// 変数を設定（通常、ローカル変数共通利用）
//---------------------------------------------------------------------
bool JlsScrGlobal::setRegVarCommon(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	bool success;
	if ( flagLocal ){
		//--- ローカル変数のレジスタ書き込み ---
		success = regvar.setLocalRegVar(strName, strVal, overwrite);
	}else{
		//--- 通常のレジスタ書き込み ---
		success = regvar.setRegVar(strName, strVal, overwrite);
	}
	return success;
}
//--- 変数の未定義化 ---
bool JlsScrGlobal::unsetRegVar(const string& strName, bool flagLocal){
	return regvar.unsetRegVar(strName, flagLocal);
}
//---------------------------------------------------------------------
// 変数を読み出し
// 入力：
//   strCandName : 読み出し変数名（候補）
//   excact      : 0=入力文字に最大マッチする変数  1=入力文字と完全一致する変数
// 出力：
//   返り値  : 変数名の文字数（0の時は対応変数なし）
//   strVal  : 変数値
//---------------------------------------------------------------------
int JlsScrGlobal::getRegVarCommon(string& strVal, const string& strCandName, bool exact){
	//--- 通常のレジスタ読み出し ---
	return regvar.getRegVar(strVal, strCandName, exact);
}
//---------------------------------------------------------------------
// Callで引数として使われる変数を設定
// 入力：
//   strName : 引数に使われる変数名
//   strVal  : 引数に使われる変数値
//---------------------------------------------------------------------
bool JlsScrGlobal::setArgReg(const string& strName, const string& strVal){
	return regvar.setArgReg(strName, strVal);
}
//--- 参照渡しレジスタ設定 ---
bool JlsScrGlobal::setArgRefReg(const string& strName, const string& strVal){
	return regvar.setArgRefReg(strName, strVal);
}
//---  返り値変数となる関数名を設定 ---
void JlsScrGlobal::setArgFuncName(const string& strName){
	regvar.setArgFuncName(strName);
}
//---------------------------------------------------------------------
// 読み出しでグローバル変数を見ない設定
// 入力：
//   flag : ローカル変数にない時のグローバル変数参照（false=許可  true=禁止）
//---------------------------------------------------------------------
void JlsScrGlobal::setLocalOnly(bool flag){
	regvar.setLocalOnly(flag);
}
//--- 変数の大文字小文字を無視するか ---
void JlsScrGlobal::setIgnoreCase(bool valid){
	regvar.setIgnoreCase(valid);
}
//--- 書き換えにwarningを出すグローバル変数設定 ---
void JlsScrGlobal::setGlobalLock(const string& strName, bool flag){
	regvar.setGlobalLock(strName, flag);
}
//--- レジスタ名かチェック ---
bool JlsScrGlobal::checkErrRegName(const string& strName){
	bool silent = true;		// 使用前のチェックではエラーを表示しない
	return regvar.checkErrRegName(strName, silent);
}
//---------------------------------------------------------------------
// エラーメッセージチェック
//---------------------------------------------------------------------
void JlsScrGlobal::checkRegError(bool flagDisp){
	string msg;
	if ( regvar.popMsgError(msg) ){		// エラーメッセージ存在時の出力
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
// 遅延実行保管領域へのアクセス
//=====================================================================

//--- state(JlsScriptState)からのアクセス ---
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
// コマンド保管時の実行順位・引数定義を設定
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
// lazy処理によるコマンドの保管
// 入力：
//   typeLazy  : LazyS, LazyA, LazyE
//   strBuf   : 保管する現在行の文字列
// 出力：
//   返り値   ：現在行のコマンド実行有無（実行キャッシュに移した時は実行しない）
//---------------------------------------------------------------------
bool JlsScrGlobal::setLazyStore(LazyType typeLazy, const string& strBuf){
	bool enableExe = true;
	//--- Lazyに入れる場合、取り込んで現在行は実行しない ---
	if ( typeLazy != LazyType::None ){
		bool success = memcmd.pushStrByLazy(typeLazy, strBuf);
		enableExe = false;				// 保管するコマンドはその場で実行しない
		if ( success == false ){
			addMsgError("error : failed Lazy push: " + strBuf + "\n");
		}
	}
	return enableExe;
}
//---------------------------------------------------------------------
// memory処理によるコマンドの保管
// 入力：
//   strName  : 保管領域の識別子
//   strBuf   : 保管する現在行の文字列
// 出力：
//   返り値   ：現在行のコマンド実行有無（実行キャッシュに移した時は実行しない）
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemStore(const string& strName, const string& strBuf){
	bool enableExe = false;			// 保管するコマンドはその場で実行しない（失敗した時も）
	bool success = memcmd.pushStrByName(strName, strBuf);
	if ( success == false ){
		addMsgError("error : failed memory push: " +  strBuf + "\n");
	}
	return enableExe;
}
//---------------------------------------------------------------------
// 記憶領域を削除(MemErase)
// 返り値   ：true=成功、false=失敗
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemErase(const string& strName){
	return memcmd.eraseMemByName(strName);
}
//---------------------------------------------------------------------
// 記憶領域を別の記憶領域にコピー(MemCopy)
// 返り値   ：true=成功、false=失敗
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemCopy(const string& strSrc, const string& strDst){
	return memcmd.copyMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// 記憶領域を別の記憶領域に移動(MemMove)
// 返り値   ：true=成功、false=失敗
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemMove(const string& strSrc, const string& strDst){
	return memcmd.moveMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// 保管領域を別の保管領域に追加(MemAppend)
// 返り値   ：true=成功、false=失敗
//---------------------------------------------------------------------
bool JlsScrGlobal::setMemAppend(const string& strSrc, const string& strDst){
	return memcmd.appendMemByName(strSrc, strDst);
}
//---------------------------------------------------------------------
// 保管領域の内容を標準出力に表示
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
// 遅延実行用のすべての保管内容取得（デバッグ用）
//---------------------------------------------------------------------
void JlsScrGlobal::setMemGetMapForDebug(){
	string strBuf;
	memcmd.getMapForDebug(strBuf);
	lcout << strBuf;
}


//---------------------------------------------------------------------
// エラーチェック
//---------------------------------------------------------------------
void JlsScrGlobal::checkErrorGlobalState(bool flagDisp){
	checkMsgError(flagDisp);
	checkRegError(flagDisp);
}
