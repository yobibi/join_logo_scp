//
// JLスクリプト グローバル状態保持
//
#include "stdafx.h"
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
	//--- 既にOpenしていたらClose ---
	if ( m_flagOfsScr ){
		m_ofsScr.close();
	}
	//--- オープン ---
	if ( flagAppend ){
		m_ofsScr.open(strName, ios::app);
	}else{
		m_ofsScr.open(strName);
	}
	//--- 確認 ---
	if ( m_ofsScr ){
		m_flagOfsScr = true;
	}else{
		addMsgError("error : file open(" + strName + ")\n");
		return false;
	}
	return true;
}
//--- ファイルクローズ ---
void JlsScrGlobal::fileClose(){
	m_ofsScr.close();
	m_flagOfsScr = false;
}
//--- 文字列を出力 ---
void JlsScrGlobal::fileOutput(const string& strBuf){
	if ( m_flagOfsScr ){
		m_ofsScr << strBuf;
	}else{
		cout << strBuf;
	}
}

//=====================================================================
// レジスタアクセス処理
//=====================================================================

//---------------------------------------------------------------------
// 最上位階層扱いでローカル変数階層作成
// 出力：
//   返り値    : 作成階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegCreateBase(){
	bool flagBase = true;		// 上位階層は検索範囲外とする
	return regvar.createLocal(flagBase);
}
//---------------------------------------------------------------------
// ローカル変数階層作成
// 出力：
//   返り値    : 作成階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegCreateOne(){
	bool flagBase = false;		// 上位階層も検索範囲
	return regvar.createLocal(flagBase);
}
//---------------------------------------------------------------------
// 最上位階層扱いローカル変数階層の終了
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegReleaseBase(){
	bool flagBase = true;		// 上位階層は検索範囲外とする階層を許可
	return regvar.releaseLocal(flagBase);
}
//---------------------------------------------------------------------
// ローカル変数階層の終了
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrGlobal::setLocalRegReleaseOne(){
	bool flagBase = false;		// 上位階層も検索範囲の階層
	return regvar.releaseLocal(flagBase);
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
// エラーメッセージチェック
//---------------------------------------------------------------------
void JlsScrGlobal::checkRegError(bool flagDisp){
	string msg;
	if ( regvar.popMsgError(msg) ){		// エラーメッセージ存在時の出力
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
	cout << strBuf;
}


//---------------------------------------------------------------------
// エラーチェック
//---------------------------------------------------------------------
void JlsScrGlobal::checkErrorGlobalState(bool flagDisp){
	checkMsgError(flagDisp);
	checkRegError(flagDisp);
}
