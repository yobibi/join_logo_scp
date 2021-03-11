//
// 変数の格納
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrReg.hpp"


///////////////////////////////////////////////////////////////////////
//
// 変数クラス
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// 変数を設定
// 入力：
//   strName   : 変数名
//   strVal    : 変数値
//   overwrite : 0=未定義時のみ設定  1=上書き許可設定
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
bool JlsRegFile::setRegVar(const string& strName, const string& strVal, bool overwrite){
	int n;
	int nloc   = -1;
	int nlenvar = (int) strName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;
	string strPair;

	//--- 既存変数の書き換えかチェック ---
	for(int i=0; i<nMaxList; i++){
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		if (nlenvar == n){
			if (_stricmp(strName.c_str(), strOrgName.c_str()) == 0){
				nloc = i;
			}
		}
	}
	//--- 設定文字列作成 ---
	strPair = strName + ":" + strVal;
	//--- 既存変数の書き換え ---
	if (nloc >= 0){
		if (overwrite){
			m_strListVar[nloc] = strPair;
		}
	}
	//--- 新規変数の追加 ---
	else{
		if (nMaxList < SIZE_VARNUM_MAX){		// 念のため変数最大数まで
			m_strListVar.push_back(strPair);
		}
		else{
			return false;
		}
	}
	return true;
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
int JlsRegFile::getRegVar(string& strVal, const string& strCandName, bool exact){
	int n;
	int nmatch = 0;
	int nloc   = -1;
	int nlencand = (int) strCandName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;

	//--- 名前とマッチする位置を検索 ---
	for(int i=0; i<nMaxList; i++){
		//--- 変数名と値を内部テーブルから取得 ---
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		//--- 内部テーブル変数名長が今までの最大一致より長ければ検索 ---
		if (nmatch < n){
			if (_strnicmp(strCandName.c_str(), strOrgName.c_str(), n) == 0 &&	// 先頭位置からマッチ
				(n == nlencand || exact == false)){								// 同一文字列かexact=false
				nloc   = i;
				nmatch = n;
			}
		}
	}
	//--- マッチした場合の値の読み出し ---
	if (nloc >= 0){
		n = getRegNameVal(strOrgName, strVal, m_strListVar[nloc]);			// 変数値を出力
		if ( strOrgName != strCandName.substr(0, n) ){
			msgErr += "warning : mismatch capital letter of register name(";
			msgErr += strCandName.substr(0, n) + " " + strOrgName + ")\n";
		}
	}
	return nmatch;
}

//---------------------------------------------------------------------
// 格納変数を名前と値に分解（変数読み書き関数からのサブルーチン）
//---------------------------------------------------------------------
int JlsRegFile::getRegNameVal(string& strName, string& strVal, const string& strPair){
	//--- 最初のデリミタ検索 ---
	int n = (int) strPair.find(":");
	//--- デリミタを分解して出力に設定 ---
	if (n > 0){
		strName = strPair.substr(0, n);
		int nLenPair = (int) strPair.length();
		if (n < nLenPair-1){
			strVal = strPair.substr(n+1);
		}
		else{
			strVal = "";
		}
	}
	return n;
}
//---------------------------------------------------------------------
// エラーメッセージが存在したら取り出す
// 出力：
//   返り値   : エラーメッセージ有無（0=なし、1=あり）
//   msg      : 取得したエラーメッセージ
//---------------------------------------------------------------------
bool JlsRegFile::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}


///////////////////////////////////////////////////////////////////////
//
// 階層構造変数クラス
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ローカル変数階層を作成
// 入力：
//   flagBase  : 検索階層（0=上位階層検索許可  1=最上位階層扱い）
// 出力：
//   返り値    : 作成階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrReg::createLocal(bool flagBase){
	if ( layerReg.size() >= INT_MAX/4 ){		// 念のためサイズ制約
		msgErr += "error:too many create local-register\n";
		return 0;
	}
	RegLayer layer;
	layer.base   = flagBase;
	//--- ローカル変数階層を作成 ---
	layerReg.push_back(layer);
	return (int) layerReg.size();
}
//---------------------------------------------------------------------
// ローカル変数階層の終了
// 入力：
//   flagBase  : 終了許可階層（0=上位階層検索許可階層のみ  1=最上位階層扱い）
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrReg::releaseLocal(bool flagBase){
	int numLayer = 0;
	if ( layerReg.empty() == false ){
		numLayer = (int) (layerReg.size() - 1);
		if ( flagBase || (layerReg[numLayer].base == false) ){	// 終了条件
			layerReg.pop_back();
			numLayer = (int) (layerReg.size() + 1);	
		}
	}
	if ( numLayer == 0 ){
		msgErr += "error:too many release local-register layer\n";
		return 0;
	}
	return numLayer;
}
//---------------------------------------------------------------------
// ローカル変数を設定
// 入力：
//   strName   : 変数名
//   strVal    : 変数値
//   overwrite : 0=未定義時のみ設定  1=上書き許可設定
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
bool JlsScrReg::setLocalRegVar(const string& strName, const string& strVal, bool overwrite){
	if ( layerReg.empty() ){	// ローカル変数階層の存在を念のためチェック
		msgErr += "error:internal setting(empty local-register layer)\n";
		return false;
	}
	if ( checkErrRegName(strName) ){	// 変数名異常時の終了
		return false;
	}
	//--- 現在のローカル変数階層に書き込み ---
	int numLayer = (int) (layerReg.size() - 1);
	return layerReg[numLayer].regfile.setRegVar(strName, strVal, overwrite);
}
//---------------------------------------------------------------------
// 変数を設定（ローカル変数に存在したら優先、なければグローバル変数に）
// 入力：
//   strName   : 変数名
//   strVal    : 変数値
//   overwrite : 0=未定義時のみ設定  1=上書き許可設定
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
bool JlsScrReg::setRegVar(const string& strName, const string& strVal, bool overwrite){
	if ( checkErrRegName(strName) ){	// 変数名異常時の終了
		return false;
	}
	//--- ローカル変数に存在するか検索 ---
	int numLayer;
	{
		bool exact = true;		// 入力文字と完全一致する変数
		string strTmp;
		getLayerRegVar(numLayer, strTmp, strName, exact);
	}
	//--- ローカル変数に存在したら優先、なければグローバル変数に書き込み ---
	bool success;
	if ( numLayer >= 0 ){
		success = layerReg[numLayer].regfile.setRegVar(strName, strVal, overwrite);
	}else{
		success = globalReg.setRegVar(strName, strVal, overwrite);
	}
	return success;
}
//---------------------------------------------------------------------
// 変数を読み出し（ローカル変数優先、なければグローバル変数）
// 入力：
//   strCandName : 読み出し変数名（候補）
//   excact      : 0=入力文字に最大マッチする変数  1=入力文字と完全一致する変数
// 出力：
//   返り値  : 変数名の文字数（0の時は対応変数なし）
//   strVal  : 変数値
//---------------------------------------------------------------------
int JlsScrReg::getRegVar(string& strVal, const string& strCandName, bool exact){
	//--- ローカル変数から検索 ---
	int numLayer;
	int numMatch = getLayerRegVar(numLayer, strVal, strCandName, exact);
	//--- ローカル変数になければグローバル変数読み込み ---
	if ( numLayer < 0 ){
		numMatch = globalReg.getRegVar(strVal, strCandName, exact);
		if ( numMatch > 0 ){
			popErrLower(globalReg);
		}
	}
	return numMatch;
}
//---------------------------------------------------------------------
// エラーメッセージが存在したら取り出す
// 出力：
//   返り値   : エラーメッセージ有無（0=なし、1=あり）
//   msg      : 取得したエラーメッセージ
//---------------------------------------------------------------------
bool JlsScrReg::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}

//---------------------------------------------------------------------
// ローカル変数階層から変数を読み出し
// 出力：
//   返り値   : 変数名の文字数（0の時は対応変数なし）
//   numLayer : 読み出したローカル変数階層（-1:該当なし、0スタートの階層）
//   strVal   : 変数値
//---------------------------------------------------------------------
int JlsScrReg::getLayerRegVar(int& numLayer, string& strVal, const string& strCandName, bool exact){
	numLayer = -1;
	int numMatch = 0;
	if ( layerReg.empty() == false ){	// ローカル変数階層がある場合のみ検索
		int n = (int) layerReg.size();
		bool scope = true;
		//--- 下位階層から検索許可階層まで変数検索 ---
		while( scope && n > 0 ){
			n --;
			string str;
			int nmatch = layerReg[n].regfile.getRegVar(str, strCandName, exact);
			if ( nmatch > 0 ){				// 変数発見
				numLayer = n;
				numMatch = nmatch;
				strVal   = str;
				scope    = false;
				popErrLower(layerReg[n].regfile);
			}else if ( layerReg[n].base ){	// 上位階層を検索しない階層
				scope    = false;
			}
		}
	}
	return numMatch;
}
//---------------------------------------------------------------------
// 変数名の最低限の違反文字確認
// 出力：
//   返り値   : エラー有無（0=正常、1=エラーあり）
//---------------------------------------------------------------------
bool JlsScrReg::checkErrRegName(const string& strName){
	//--- 最低限の違反文字確認 ---
	string strCheckFull  = "!#$%&'()*+,-./:;<=>?";			// 変数文字列として使用禁止
	string strCheckFirst = strCheckFull + "0123456789";		// 変数先頭文字として使用禁止
	string strFirst = strName.substr(0, 1);
	if ( strCheckFirst.find(strFirst) != string::npos ){
		msgErr += "error: register setting, invalid first char(" + strName + ")\n";
		return true;
	}
	for(int i=0; i < (int)strCheckFull.length(); i++){
		string strNow = strCheckFull.substr(i, 1);
		if ( strName.find(strNow) != string::npos ){
			msgErr += "error: register setting, bad char exist(" + strName + ")\n";
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------
// 下位階層のエラー取得
//---------------------------------------------------------------------
bool JlsScrReg::popErrLower(JlsRegFile& regfile){
	string msgTmp;
	if ( regfile.popMsgError(msgTmp) ){
		msgErr += msgTmp;
		return true;
	}
	return false;
}
