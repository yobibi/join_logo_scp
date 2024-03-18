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
			if ( isSameInLen(strName, strOrgName, n) ){
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
// 変数を削除
// 入力：
//   strName   : 変数名
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
bool JlsRegFile::unsetRegVar(const string& strName){
	int nloc   = -1;
	int nlenvar = (int) strName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;

	//--- 位置取得 ---
	for(int i=0; i<nMaxList; i++){
		int n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		if (nlenvar == n){
			if ( isSameInLen(strName, strOrgName, n) ){
				nloc = i;
			}
		}
	}
	if ( nloc < 0 ) return false;
	//--- 削除 ---
	m_strListVar.erase(m_strListVar.begin() + nloc);
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
			if (isSameInLen(strCandName, strOrgName, n) &&		// 先頭位置からマッチ
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
// 大文字小文字関連
//---------------------------------------------------------------------
void JlsRegFile::setIgnoreCase(bool valid){
	m_ignoreCase = valid;
}
bool JlsRegFile::isSameInLen(const string& s1, const string& s2, int nLen){
	if ( m_ignoreCase ){
		return ( _strnicmp(s1.c_str(), s2.c_str(), nLen) == 0 );	// 先頭位置からマッチ
	}
	return ( s1.substr(0, nLen) == s2.substr(0, nLen) );
}
//---------------------------------------------------------------------
// 参照渡し変数としての設定
//---------------------------------------------------------------------
void JlsRegFile::setFlagAsRef(const string& strName){
	m_flagListRef[strName] = true;
}
bool JlsRegFile::isRegNameRef(const string& strName){
	return ( m_flagListRef.find(strName) != m_flagListRef.end() );
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

JlsScrReg::JlsScrReg(){
	onlyLocal = false;
	ignoreCase = true;		// 現在の初期設定値
	globalReg.setIgnoreCase(ignoreCase);
}
//---------------------------------------------------------------------
// ローカル変数階層を作成
// 出力：
//   返り値    : 作成階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrReg::createLocalCall(){
	return createLocalCommon(RegOwner::Call);
}
int JlsScrReg::createLocalFunc(){
	return createLocalCommon(RegOwner::Func);
}
int JlsScrReg::createLocalOne(){
	return createLocalCommon(RegOwner::One);
}
//---------------------------------------------------------------------
// ローカル変数階層の終了
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrReg::releaseLocalAny(){
	return releaseLocalCommon(RegOwner::Any);
}
int JlsScrReg::releaseLocalCall(){
	return releaseLocalCommon(RegOwner::Call);
}
int JlsScrReg::releaseLocalFunc(){
	return releaseLocalCommon(RegOwner::Func);
}
int JlsScrReg::releaseLocalOne(){
	return releaseLocalCommon(RegOwner::One);
}
//---------------------------------------------------------------------
// ローカル変数階層の取得
// 出力：
//   返り値    : 終了階層（0=失敗、1以上=階層）
//---------------------------------------------------------------------
int JlsScrReg::getLocalLayer(){
	return (int) layerReg.size();
}
//---------------------------------------------------------------------
// 共通処理
//---------------------------------------------------------------------
int JlsScrReg::createLocalCommon(RegOwner owner){
	if ( layerReg.size() >= INT_MAX/4 ){		// 念のためサイズ制約
		msgErr += "error:too many create local-register\n";
		return 0;
	}
	bool flagBase = ( owner != RegOwner::One );	// 検索階層（0=上位階層検索許可  1=最上位階層扱い）
	RegLayer layer;
	layer.owner  = owner;
	layer.base   = flagBase;
	layer.regfile.setIgnoreCase(ignoreCase);
	//--- ローカル変数階層を作成 ---
	layerReg.push_back(layer);
	//--- 最上位階層扱いのCallであれば引数をローカル変数に格納 ---
	if ( flagBase ){
		setRegFromArg();
	}else{
		clearArgReg();			// 使わなかった引数削除
	}

	return (int) layerReg.size();
}
int JlsScrReg::releaseLocalCommon(RegOwner owner){
	int numLayer = 0;
	if ( layerReg.empty() == false ){
		numLayer = (int) (layerReg.size() - 1);
		bool allow = ( layerReg[numLayer].owner == owner || owner == RegOwner::Any );
		if ( allow ){	// 終了条件
			layerReg.pop_back();
			clearArgReg();			// 使わなかった引数削除
			numLayer = (int) (layerReg.size() + 1);	
		}else{
			msgErr += "error:not match release local-register layer\n";
			return 0;
		}
	}
	if ( numLayer == 0 ){
		msgErr += "error:too many release local-register layer\n";
		return 0;
	}
	return numLayer;
}
//---------------------------------------------------------------------
// 変数を消去
// 入力：
//   strName   : 変数名
//   flagLocal : 0=すべての変数  1=ローカル変数のみ１箇所
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
bool JlsScrReg::unsetRegVar(const string& strName, bool flagLocal){
	bool success = false;
	int numLayer = -1;
	bool cont = true;
	while( cont ){
		int numLast = numLayer;
		cont = findRegForUnset(numLayer, strName, flagLocal);
		if ( cont ){
			cont = unsetRegCore(strName, numLayer);		// 変数消去
		}
		if ( cont ){
			success = true;
			if ( numLayer <= 0 || flagLocal ){
				cont = false;
			}else if ( numLast <= numLayer && numLast >= 0 ){	// 念のため
				cont = false;
			}
		}
	}
	return success;
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
	//--- 現在のローカル変数階層に書き込み ---
	int numLayer = (int) layerReg.size();
	return setRegCore(strName, strVal, overwrite, numLayer);
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
	//--- 階層は自動検索で書き込み ---
	int  numLayer = -1;
	return setRegCore(strName, strVal, overwrite, numLayer);
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
	return findRegForRead(strCandName, strVal, exact);
}
//---------------------------------------------------------------------
// Callで引数として使われる変数を設定
// 入力：
//   strName : 引数に使われる変数名
//   strVal  : 引数に使われる変数値
//---------------------------------------------------------------------
bool JlsScrReg::setArgReg(const string& strName, const string& strVal){
	//--- 引数リストに追加 ---
	if ( listValArg.size() >= INT_MAX/4 ){		// 念のためサイズ制約
		msgErr += "error:too many create arg-registers\n";
		return false;
	}
	listValArg.push_back(strName);
	listValArg.push_back(strVal);
	return true;
}
//--- 参照渡し用 ---
bool JlsScrReg::setArgRefReg(const string& strName, const string& strVal){
	//--- 引数リストに追加 ---
	if ( listRefArg.size() >= INT_MAX/4 ){		// 念のためサイズ制約
		msgErr += "error:too many create arg-registers\n";
		return false;
	}
	listRefArg.push_back(strName);
	listRefArg.push_back(strVal);
	return true;
}
//--- 返り値変数となる関数名を設定 ---
void JlsScrReg::setArgFuncName(const string& strName){
	nameFuncReg = strName;
}
//---------------------------------------------------------------------
// 読み出しでグローバル変数を見ない設定
// 入力：
//   flag : ローカル変数にない時のグローバル変数参照（false=許可  true=禁止）
//---------------------------------------------------------------------
void JlsScrReg::setLocalOnly(bool flag){
	onlyLocal = flag;
}
void JlsScrReg::setIgnoreCase(bool valid){
	ignoreCase = valid;
	int numLayer = (int) layerReg.size();
	for(int i=0; i<numLayer; i++){
		layerReg[numLayer-1].regfile.setIgnoreCase(ignoreCase);
	}
	globalReg.setIgnoreCase(ignoreCase);
}
void JlsScrReg::setGlobalLock(const string& strName, bool flag){
	if ( flag ){
		m_mapGlobalLock[strName] = true;
	}else{
		if ( isGlobalLocked(strName) ){
			m_mapGlobalLock.erase(strName);
		}
	}
}
bool JlsScrReg::isGlobalLocked(const string& strName){
	return ( m_mapGlobalLock.count(strName) != 0 );
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
// 変数を階層指定で書き込み
//---------------------------------------------------------------------
//--- 変数の消去 ---
bool JlsScrReg::unsetRegCore(const string& strName, int numLayer){
	if ( checkErrRegName(strName) ){	// 変数名異常時の終了
		return false;
	}
	bool success;
	if ( numLayer > 0 ){
		success = layerReg[numLayer-1].regfile.unsetRegVar(strName);
	}else{
		success = globalReg.unsetRegVar(strName);
	}
	return success;
}
//--- 通常の変数書き込み ---
// numLayer=-1 の時は階層を自動検索する
bool JlsScrReg::setRegCore(const string& strName, const string& strVal, bool overwrite, int numLayer){
	if ( checkErrRegName(strName) ){	// 変数名異常時の終了
		return false;
	}
	//--- 既存変数を確認 ---
	string strWriteName = strName;
	string strWriteVal = strVal;
	bool flagOvw = overwrite;
	int  numWriteLayer = numLayer;
	if ( !findRegForWrite(strWriteName, strWriteVal, flagOvw, numWriteLayer) ){
		return false;		// アクセスできない時は無効
	}
	if ( numWriteLayer < 0 ){
		numWriteLayer = 0;		// 既存が見つからなければグローバル変数に書き込み
	}
	if ( numWriteLayer > 0 ){
		if ( isRegNameRef(strWriteName, numWriteLayer) && overwrite ){
			msgErr += "error: overwrite ref register " + strWriteName + "\n";
			return false;		// 参照渡し変数自体のローカル書き換えは禁止
		}
	}
	//--- 実際の書き込み ---
	bool success;
	if ( numWriteLayer > 0 ){
		success = layerReg[numWriteLayer-1].regfile.setRegVar(strWriteName, strWriteVal, flagOvw);
	}else{
		success = globalReg.setRegVar(strWriteName, strWriteVal, flagOvw);
		if ( isGlobalLocked(strWriteName) && success ){
			msgErr += "warning: global fixed register is changed(" + strWriteName + ")\n";
		}
	}
	return success;
}
//--- 参照渡しとして変数書き込み ---
bool JlsScrReg::setRegCoreAsRef(const string& strName, const string& strVal, int numLayer){
	if ( checkErrRegName(strName) ){	// 変数名異常時の終了
		return false;
	}
	bool success;
	if ( numLayer > 0 ){
		bool overwrite = true;
		success = layerReg[numLayer-1].regfile.setRegVar(strName, strVal, overwrite);
		layerReg[numLayer-1].regfile.setFlagAsRef(strName);
	}else{
		success = false;		// グローバル変数は参照渡しにできない
	}
	return success;
}
//---------------------------------------------------------------------
// 変数を読み出し
//---------------------------------------------------------------------
//--- 消去する変数階層を検索 ---
bool JlsScrReg::findRegForUnset(int& numLayer, const string& strName, bool flagLocal){
	RegSearch data(strName);
	data.numLayer = numLayer;
	data.stopRef = true;	// 参照渡しは止める
	bool success = findRegData(data);
	if ( success ){
		numLayer = data.numLayer;
		if ( numLayer < 0 || data.flagRef ){	// 参照渡しは対象外
			success = false;
		}else if ( numLayer == 0 && flagLocal){
			success = false;
		}
	}
	return success;
}
//--- 書き込み前に既存変数を検索 ---
// 参照変数やリストだった時は最終的に書き込む変数情報に変更する
bool JlsScrReg::findRegForWrite(string& strName, string& strVal, bool& overwrite, int& numLayer){
	RegSearch data(strName);
	if ( numLayer >= 0 ){		// 階層限定の場合
		data.numLayer = numLayer;
		data.onlyOneLayer = true;
	}
	if ( !findRegData(data) ){	// データ取得
		//--- 既存データがない場合の処理 ---
		auto nList = data.regOrg.listElem.size();
		if ( nList >= 2 ){	// 2次配列以上は無効
			return false;
		}else if ( nList == 1 ){	// 配列は最初の要素のみ書き込み可能できる
			if ( data.regOrg.listElem[0] != 1 ) return false;
			strName = data.regOrg.nameBase;
			string listTmp;
			funcList.setListStrIns(listTmp, strVal, 1);
			strVal = listTmp;
		}
		return true;
	}
	//--- 既存データがある場合の処理 ---
	strName = data.regSel.nameBase;
	numLayer = data.numLayer;
	return findRegListForWrite(data.regSel, strVal, overwrite, data.strVal);
}
//--- 読み出しデータを取得 ---
int JlsScrReg::findRegForRead(const string& strName, string& strVal, bool exact){
	int retMatch = 0;
	//--- 変数に存在するか検索 ---
	RegSearch data(strName);
	data.exact = exact;
	if ( findRegData(data) ){
		strVal = data.strVal;
		retMatch = data.regOrg.nMatch;		// 元データのマッチ数
		//--- リスト要素対応 ---
		findRegListForRead(data.regSel, strVal);
	}
	return retMatch;
}
//--- リスト要素だった場合は全体のデータから対象部分のみ差し替える ---
bool JlsScrReg::findRegListForWrite(RegDivList& regName, string& strVal, bool& overwrite, const string& strRead){
	if ( regName.listElem.empty() ){	// リストのない通常データは何もせず終了
		return true;
	}
	bool success = true;
	string strParts = strRead;
	//--- 各要素を取得 ---
	int nElem = (int)regName.listElem.size();
	vector<string> listHold;
	for(int i=nElem-1; i>=1; i--){
		listHold.push_back(strParts);
		string strBak = strParts;
		if ( !funcList.getListStrElement(strParts, strBak, regName.listElem[i]) ){
			success = false;
		}
	}
	//--- 書き込みデータ設定 ---
	if ( success ){
		int sizeParts = funcList.getListStrSize(strParts);
		int selElem = regName.listElem[0];
		if ( selElem > sizeParts + 1 ){		// 要素書き込みは既存+1要素まで
			success = false;
		}else if ( selElem == sizeParts + 1 ){		// 要素最後に追加
			overwrite = true;		// 追加でも全体としては既存変数なので設定変更
			success = funcList.setListStrIns(strParts, strVal, selElem);
		}else{
			success = funcList.setListStrRep(strParts, strVal, selElem);
		}
	}
	//--- 各要素に設定後データを戻す ---
	if ( success ){
		for(int i=1; i<=nElem-1; i++){
			string strItem = strParts;
			strParts = listHold[nElem-i-1];
			funcList.setListStrRep(strParts, strItem, regName.listElem[i]);
		}
		strVal = strParts;
	}
	if ( !success ){
		msgErr += "error: not exist the list element in the register " + regName.nameBase + "\n";
	}
	return success;
}
//--- リスト要素だった場合は該当リストのみ抜き出す ---
bool JlsScrReg::findRegListForRead(RegDivList& regName, string& strVal){
	if ( regName.listElem.empty() ){	// リストのない通常データは何もせず終了
		return true;
	}
	int nElem = (int)regName.listElem.size();
	for(int i=nElem-1; i>=0; i--){
		string strBak = strVal;
		if ( !funcList.getListStrElement(strVal, strBak, regName.listElem[i]) ){
			msgErr += "error: out of range at register read\n";
			strVal.clear();
			return false;
		}
	}
	return true;
}
//
// 変数を階層検索して読み出し
// 出力：
//   返り値   : 変数有無
//   data     : 読み出した変数情報 階層（-1:該当なし、0:グローバル階層、1-:ローカル階層）
//
bool JlsScrReg::findRegData(RegSearch& data){
	//--- 変数の階層と値を取得 ---
	bool success = findRegDataFromLayer(data);
	data.decide();		// 設定実行
	if ( !success ){	// 変数が見つからなければ失敗
		return false;
	}
	//--- ローカル変数の時は参照渡しか確認 ---
	if ( data.numLayer > 0 ){
		data.flagRef = isRegNameRef(data.strName, data.numLayer);
	}
	if ( !data.flagRef ){		// 参照渡しでなければ完了
		return true;
	}
	if ( data.stopRef || data.onlyOneLayer ){		// 参照渡し先までチェックでなければ完了
		return true;
	}
	//--- 参照渡しの参照先変数を取得 ---
	bool cont = true;
	while( success && cont ){
		//--- １階層下の参照先変数を取得 ---
		success = data.updateRef(data.strVal);		// 参照先を新しい変数名にする
		if ( success ){
			success = findRegDataFromLayer(data);
		}
		if ( success ){
			data.flagRef = isRegNameRef(data.strName, data.numLayer);
			if ( !data.flagRef ){	// 参照渡しではない実変数が見つかったら終了
				cont = false;
			}
		}
	}
	return success;
}
// 入力: data(strName, numLayer)
// 出力: data(numLayer, numMatch, [strName, strVal]) []は状況により出力
bool JlsScrReg::findRegDataFromLayer(RegSearch& data){
	//--- 検索階層の設定 ---
	int n = (int) layerReg.size();
	if ( n > data.numLayer && data.numLayer >= 0 ){		// 最大レイヤー指定がある時
		n = data.numLayer;
	}
	bool skipGlobal = ( data.numLayer != 0 && data.onlyOneLayer );
	data.numLayer = -1;
	data.numMatch = 0;
	bool scope = true;
	//--- 下位階層から検索許可階層まで変数検索 ---
	while( scope && n > 0 ){
		n --;
		string str;
		int nmatch = layerReg[n].regfile.getRegVar(str, data.strName, data.exact);
		if ( nmatch > 0 ){				// 変数発見
			data.numLayer = n+1;
			data.numMatch = nmatch;
			data.strVal   = str;
			scope         = false;
			popErrLower(layerReg[n].regfile);
		}else if ( layerReg[n].base ){	// 上位階層を検索しない階層
			scope    = false;
		}else if ( data.onlyOneLayer ){	// 1階層しか検索しない
			scope    = false;
		}
	}
	//--- なければグローバル変数を検索 ---
	if ( data.numMatch == 0 && !onlyLocal && !skipGlobal ){
		string str;
		int nMatch = globalReg.getRegVar(str, data.strName, data.exact);
		if ( nMatch > 0 ){
			data.numLayer = 0;
			data.numMatch = nMatch;
			data.strVal   = str;
			popErrLower(globalReg);
		}
	}
	bool success = ( data.numMatch > 0 );
	if ( success && !data.exact ){
		data.strName = data.strName.substr(0, data.numMatch);
	}
	return success;
}
//--- 参照渡しフラグ取得 ---
bool JlsScrReg::isRegNameRef(const string& strName, int numLayer){
	if ( numLayer <= 0 ){		// グローバル変数は参照なし
		return false;
	}
	return layerReg[numLayer-1].regfile.isRegNameRef(strName);
}

//---------------------------------------------------------------------
// 引数格納データを削除
//---------------------------------------------------------------------
void JlsScrReg::clearArgReg(){
	//--- 引数リストを削除 ---
	listValArg.clear();
	listRefArg.clear();
	nameFuncReg.clear();
}
//---------------------------------------------------------------------
// 引数をローカル変数に設定
//---------------------------------------------------------------------
void JlsScrReg::setRegFromArg(){
	//--- 設定 ---
	setRegFromArgSub( listValArg, false );		// ref=false
	setRegFromArgSub( listRefArg, true );		// ref=true
	//--- 返り値の受け取り先がない時のダミーローカル変数設定 ---
	if ( !nameFuncReg.empty() ){
		bool overwrite = false;		// 未設定時のみダミーレジスタとして返り値変数を作成
		setLocalRegVar(nameFuncReg, "", overwrite);
	}
	//--- 引数リストを削除 ---
	clearArgReg();
}
void JlsScrReg::setRegFromArgSub(vector<string>& listArg, bool ref){
	int sizeList = (int) listArg.size();
	if ( sizeList > 0 ){
		//--- 引数リストをローカル変数に設定 ---
		if ( ref ){
			int numLayer = (int) layerReg.size();
			for(int i=0; i<sizeList-1; i+=2){
				setRegCoreAsRef(listArg[i], listArg[i+1], numLayer);
			}
		}else{
			bool overwrite = true;
			for(int i=0; i<sizeList-1; i+=2){
				setLocalRegVar(listArg[i], listArg[i+1], overwrite);
			}
		}
	}
}
//---------------------------------------------------------------------
// 変数名の最低限の違反文字確認
// 出力：
//   返り値   : エラー有無（0=正常、1=エラーあり）
//---------------------------------------------------------------------
bool JlsScrReg::checkErrRegName(const string& strName, bool silent){
	//--- 最低限の違反文字確認 ---
	string strCheckFull  = "!#$%&'()*+,-./:;<=>?\"";		// 変数文字列として使用禁止
	string strCheckFirst = strCheckFull + "0123456789";		// 変数先頭文字として使用禁止
	string strFirst = strName.substr(0, 1);
	if ( strCheckFirst.find(strFirst) != string::npos ){
		if ( !silent ){
			msgErr += "error: register setting, invalid first char(" + strName + ")\n";
		}
		return true;
	}
	for(int i=0; i < (int)strCheckFull.length(); i++){
		string strNow = strCheckFull.substr(i, 1);
		if ( strName.find(strNow) != string::npos ){
			if ( !silent ){
				msgErr += "error: register setting, bad char exist(" + strName + ")\n";
			}
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
