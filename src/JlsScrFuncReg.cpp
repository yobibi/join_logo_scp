//
// 変数アクセス関連処理
//

#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrFuncReg.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsScrGlobal.hpp"
#include "JlsCmdArg.hpp"
#include "JlsDataset.hpp"


void JlsScrFuncReg::setDataPointer(JlsDataset *pdata, JlsScrGlobal *pglobal, JlsScrFuncList *plist){
	this->pdata = pdata;
	this->pGlobalState = pglobal;
	this->pFuncList = plist;
}

//=====================================================================
// 起動オプション処理
//=====================================================================

//---------------------------------------------------------------------
// スクリプト内で記載する起動オプション
// 入力：
//   argrest    ：引数残り数
//   strv       ：引数コマンド
//   str1       ：引数値１
//   str2       ：引数値２
//   overwrite  ：書き込み済みのオプション設定（false=しない true=する）
//   checklevel ：エラー確認レベル（0=なし 1=認識したオプションチェック）
// 出力：
//   返り値  ：引数取得数(-1の時取得エラー、0の時該当コマンドなし)
//---------------------------------------------------------------------
int JlsScrFuncReg::setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite){
	if (argrest <= 0){
		return 0;
	}
	bool exist2 = false;
	bool exist3 = false;
	if (argrest >= 2){
		exist2 = true;
	}
	if (argrest >= 3){
		exist3 = true;
	}
	int numarg = 0;
	if(strv[0] == '-' && strv[1] != '\0') {
		if (!_stricmp(strv, "-flags")){
			if (!exist2){
				outputMesErr("-flags needs an argument\n");
				return -1;
			}
			else{
				if (setInputFlags(str1, overwrite) == false){
					outputMesErr("-flags bad argument\n");
					return -1;
				}
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-set")){
			if (!exist3){
				outputMesErr("-set needs two arguments\n");
				return -1;
			}
			else{
				if (setInputReg(str1, str2, overwrite) == false){
					outputMesErr("-set bad argument\n");
					return -1;
				}
			}
			numarg = 3;
		}
		else if (!_stricmp(strv, "-cutmrgin")){
			if (!exist2){
				outputMesErr("-cutmrgin needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixCutIn == 0){
				pdata->extOpt.msecCutIn = setOptionsCnvCutMrg(str1);
				pdata->extOpt.fixCutIn = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgout")){
			if (!exist2){
				outputMesErr("-cutmrgout needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixCutOut == 0){
				pdata->extOpt.msecCutOut = setOptionsCnvCutMrg(str1);
				pdata->extOpt.fixCutOut = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgwi")){
			if (!exist2){
				outputMesErr("-cutmrgwi needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixWidCutI == 0){
				int val = atoi(str1);
				pdata->extOpt.wideCutIn  = val;
				pdata->extOpt.fixWidCutI = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgwo")){
			if (!exist2){
				outputMesErr("-cutmrgwo needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixWidCutO == 0){
				int val = atoi(str1);
				pdata->extOpt.wideCutOut = val;
				pdata->extOpt.fixWidCutO = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-sublist")){
			if (!exist2){
				outputMesErr("-sublist needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubList == 0){
				if ( str1[0] == '+' ){
					string tmp = str1;
					pdata->extOpt.subList = tmp.substr(1) + "," + pdata->extOpt.subList;
				}else{
					pdata->extOpt.subList = str1;
				}
				pdata->extOpt.fixSubList = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-subpath")){
			if (!exist2){
				outputMesErr("-subpath needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubPath == 0){
				pdata->extOpt.subPath = str1;
				pdata->extOpt.fixSubPath = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-setup")){
			if (!exist2){
				outputMesErr("-setup needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSetup == 0){
				pdata->extOpt.setup = str1;
				pdata->extOpt.fixSetup = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-syscode")){
			if (!exist2){
				outputMesErr("-syscode needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixNSysCode == 0){
				//--- 標準出力/エラーの文字コードは最優先で設定しておきたいのですぐ実行 ---
				int num = LSys.getUtfNumFromStr(str1);
				if ( num < 0 ){
					outputMesErr("-syscode unsupported data\n");
					return -1;
				}
				pdata->extOpt.nSysCode = num;
				pdata->extOpt.fixNSysCode = 1;
				LSys.setStdUtfCodeFromNum(num);		// すぐ設定
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-vline")){	// debug
			if (!exist2){
				outputMesErr("-vline needs an argument\n");
				return -1;
			}
			if (overwrite || pdata->extOpt.fixVLine == 0){
				int val = atoi(str1);
				pdata->extOpt.vLine = val;
				pdata->extOpt.fixVLine = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inlogo")){	// 名前のみ保持
			pdata->extOpt.logofile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inscp")){	// 名前のみ保持
			pdata->extOpt.scpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-incmd")){	// 名前のみ保持
			pdata->extOpt.cmdfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-o")){	// 名前のみ保持
			pdata->extOpt.outfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-oscp")){	// 名前のみ保持
			pdata->extOpt.outscpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-odiv")){	// 名前のみ保持
			pdata->extOpt.outdivfile = str1;
			numarg = 2;
		}
	}
	return numarg;
}

//---------------------------------------------------------------------
// CutMrgIn / CutMrgOut オプション処理用 30fpsフレーム数入力でミリ秒を返す
//---------------------------------------------------------------------
Msec JlsScrFuncReg::setOptionsCnvCutMrg(const char* str){
	int num = atoi(str);
	int frac = 0;
	const char *tmpstr = strchr(str, '.');
	if (tmpstr != nullptr){
		if (tmpstr[1] >= '0' && tmpstr[1] <= '9'){
			frac = (tmpstr[1] - '0') * 10;
			if (tmpstr[2] >= '0' && tmpstr[2] <= '9'){
				frac += (tmpstr[2] - '0');
			}
		}
	}
	//--- 30fps固定変換処理 ---
	Msec msec_num  = (abs(num) * 1001 + 30/2) / 30;
	Msec msec_frac = (frac * 1001 + 30/2) / 30 / 100;
	Msec msec_result = msec_num + msec_frac;
	if (num < 0) msec_result = -1 * msec_result;
	return msec_result;
}

//---------------------------------------------------------------------
// 変数を外部から設定
// 出力：
//   返り値  ：true=正常終了  false=失敗
//---------------------------------------------------------------------
bool JlsScrFuncReg::setInputReg(const char *name, const char *val, bool overwrite){
	return setJlsRegVar(name, val, overwrite);
}

//---------------------------------------------------------------------
// オプションフラグを設定
// 出力：
//   返り値  ：true=正常終了  false=失敗
//---------------------------------------------------------------------
bool JlsScrFuncReg::setInputFlags(const char *flags, bool overwrite){
	bool ret = true;
	int pos = 0;
	string strBuf = flags;
	while(pos >= 0){
		string strFlag;
		pos = pdata->cnv.getStrWord(strFlag, strBuf, pos);
		if (pos >= 0){
			string strName, strVal;
			//--- 各フラグの値を設定 ---
			int nloc = (int) strFlag.find(":");
			if (nloc >= 0){
				strName = strFlag.substr(0, nloc);
				strVal  = strFlag.substr(nloc+1);
			}
			else{
				strName = strFlag;
				strVal  = "1";
			}
			//--- 変数格納 ---
			bool flagtmp = setJlsRegVar(strName, strVal, overwrite);
			if (flagtmp == false) ret = false;
		}
	}
	return ret;
}

//=====================================================================
// レジスタアクセス処理
//=====================================================================

//---------------------------------------------------------------------
// Call実行した時にローカル変数設定される引数を設定
// 入力：
//   strName   : 変数名
//   strVal    : 変数値
//---------------------------------------------------------------------
//--- 関数の返り値になる変数名を設定 ---
void JlsScrFuncReg::setArgFuncName(const string& strName){
	pGlobalState->setArgFuncName(strName);
}
//--- 参照渡しで設定 ---
bool JlsScrFuncReg::setArgRefReg(const string& strName, const string& strVal){
	if ( !setArgRegCheckName(strName) ) return false;	// 関数引数としての変数名異常チェック
	if ( !isValidAsRegName(strVal) ) return false;		// 一般の変数名として異常チェック
	string strRefVal = strVal;
	int lenFullVar;
	if ( fixJlsRegNameAtList(strRefVal, lenFullVar, true) ){	// リスト要素対応で設定
		return pGlobalState->setArgRefReg(strName, strRefVal);
	}
	return false;
}
//--- 変数値を設定 ---
bool JlsScrFuncReg::setArgRegByVal(const string& strName, const string& strVal){
	if ( !setArgRegCheckName(strName) ) return false;	// 関数引数としての変数名異常チェック
	return pGlobalState->setArgReg(strName, strVal);
}
//--- 値が変数名の場合の設定 ---
bool JlsScrFuncReg::setArgRegByName(const string& strName, const string& strValName){
	string strVal;
	if ( !getJlsRegVarNormal(strVal, strValName) ){
		pGlobalState->addMsgErrorN("error: not exist " + strValName);
		return false;
	}
	return setArgRegByVal(strName, strVal);
}
//--- 値が変数名か値そのものか判別して設定 ---
bool JlsScrFuncReg::setArgRegByBoth(const string& strName, const string& strVal, bool quote){
	if ( isValidAsRegName(strVal) ){
		return setArgRegByName(strName, strVal);
	}
	string strEval = strVal;
	if ( quote ){	// 引用符を外す処理
		pdata->cnv.getStrItemArg(strEval, strVal, 0);
	}
	return setArgRegByVal(strName, strEval);
}
//--- 変数名として有効な名前かチェック ---
bool JlsScrFuncReg::isValidAsRegName(const string& strName){
	string strNamePart = strName;
	if ( strName.find("\"") != string::npos ) return false;
	if ( strName.find("\'") != string::npos ) return false;
	//--- リスト変数はリストを除く ---
	auto npos = strNamePart.find("[");
	if ( npos != string::npos ){
		strNamePart = strNamePart.substr(npos);
	}
	if ( strNamePart.empty() ) return false;
	//--- 確認 ---
	return !(pGlobalState->checkErrRegName(strNamePart));
}
//--- 関数引数名としての変数名異常チェック ---
bool JlsScrFuncReg::setArgRegCheckName(const string& strName){
	bool success = true;
	string mesErr;
	if ( strName.empty() ){
		success = false;
		mesErr = "empty";
	}else if ( !isValidAsRegName(strName) ){
		success = false;
		mesErr = "illegal";
	}else{
		//--- リスト変数の要素で引数宣言は非対応 ---
		auto locSt = strName.find("[");
		if ( locSt != string::npos ){
			success = false;
			mesErr = "argument array";
		}
	}
	if ( !success ){
		mesErr = "error: name " + mesErr;
		mesErr += " ";
		mesErr += strName;
		pGlobalState->addMsgErrorN(mesErr);
	}
	return success;
}
//---------------------------------------------------------------------
// 変数を設定
// 入力：
//   strName   : 変数名
//   strVal    : 変数値
//   overwrite : 0=未定義時のみ設定  1=上書き許可設定
// 出力：
//   返り値    : 通常=true、失敗時=false
//---------------------------------------------------------------------
//--- 変数の未定義化 ---
bool JlsScrFuncReg::unsetJlsRegVar(const string& strName, bool flagLocal){
	return pGlobalState->unsetRegVar(strName, flagLocal);
}
//--- 通常の変数を設定 ---
bool JlsScrFuncReg::setJlsRegVar(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = false;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- ローカル変数を設定（引数は通常変数と同一） ---
bool JlsScrFuncReg::setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = true;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- 通常の変数とローカル変数を選択して設定 ---
bool JlsScrFuncReg::setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	if ( strName.empty() ) return false;
	//--- リスト変数対応 ---
	string strNameWrite = strName;
	int lenFullVar;
	bool success = fixJlsRegNameAtList(strNameWrite, lenFullVar, true);		// exact=true
	//--- 書き込み処理 ---
	if ( success ){
		success = pGlobalState->setRegVarCommon(strNameWrite, strVal, overwrite, flagLocal);
		setJlsRegVarCouple(strNameWrite, strVal);
	}
	return success;
}
bool JlsScrFuncReg::setJlsRegVarCountUp(const string& strName, int step, bool flagLocal){
	//--- 現在値取得 ---
	string strVal;
	if ( !getJlsRegVarNormal(strVal, strName) ) return false;
	//--- 整数に変換して演算後書き戻し ---
	int val;
	int pos = 0;
	pos = pdata->cnv.getStrValNum(val, strVal, pos);
	if ( pos < 0 ) return false;
	val += step;
	return setJlsRegVarWithLocal(strName, to_string(val), true, flagLocal);
}
//--- 変数設定後のシステム変数更新 ---
void JlsScrFuncReg::setJlsRegVarCouple(const string& strName, const string& strVal){
	//--- システム変数の特殊処理 ---
	if (_stricmp(strName.c_str(), "RANGETYPE") == 0){
		pdata->cnv.getStrValNum(pdata->recHold.typeRange, strVal, 0);
	}
	//--- HEAD/TAIL期間処理 ---
	int type_add = 0;
	string strAddName;
	if (_stricmp(strName.c_str(), "HEADFRAME") == 0){
		strAddName = "HEADTIME";
		type_add = 1;
	}
	else if (_stricmp(strName.c_str(), "TAILFRAME") == 0){
		strAddName = "TAILTIME";
		type_add = 1;
	}
	else if (_stricmp(strName.c_str(), "HEADTIME") == 0){
		strAddName = "HEADFRAME";
		type_add = 2;
	}
	else if (_stricmp(strName.c_str(), "TAILTIME") == 0){
		strAddName = "TAILFRAME";
		type_add = 2;
	}
	if (type_add > 0){
		int val;
		if (pdata->cnv.getStrValMsecM1(val, strVal, 0)){
			string strAddVal;
			if (type_add == 2){
				strAddVal = pdata->cnv.getStringFrameMsecM1(val);
			}
			else{
				strAddVal = pdata->cnv.getStringTimeMsecM1(val);
			}
			bool flagLocal = false;
			bool overwrite = true;
			pGlobalState->setRegVarCommon(strAddName, strAddVal, overwrite, flagLocal);
		}
		//--- head/tail情報を更新 ---
		{
			string strHead;
			if ( getJlsRegVarNormal(strHead, "HEADTIME") ){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.st, strHead, 0);
			}
			string strTail;
			if ( getJlsRegVarNormal(strTail, "TAILTIME") ){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.ed, strTail, 0);
			}
			//--- ABSHEAD/ABSTAILを更新 ---
			string strAbsHead = strHead;
			string strAbsTail = strTail;
			if ( pdata->recHold.rmsecHeadTail.st < 0 ){
				int valZero = 0;
				strAbsHead = pdata->cnv.getStringTimeMsecM1(valZero);
			}
			if ( pdata->recHold.rmsecHeadTail.ed < 0 ){
				getJlsRegVarNormal(strAbsTail, "MAXTIME");
			}
			bool flagLocal = false;
			bool overwrite = true;
			pGlobalState->setRegVarCommon("ABSHEAD", strAbsHead, overwrite, flagLocal);
			pGlobalState->setRegVarCommon("ABSTAIL", strAbsTail, overwrite, flagLocal);
		}
	}
	//--- MAXTIME更新 ---
	if ( _stricmp(strName.c_str(), "MAXTIME") == 0 ){
		string strTail;
		bool validTail = false;
		if ( getJlsRegVarNormal(strTail, "TAILTIME") ){
			int val;
			if ( pdata->cnv.getStrValMsecM1(val, strTail, 0) ){
				if ( val >= 0 ) validTail = true;
			}
		}
		if ( !validTail ){
			strTail = strVal;
		}
		bool flagLocal = false;
		bool overwrite = true;
		pGlobalState->setRegVarCommon("ABSTAIL", strTail, overwrite, flagLocal);
	}
}

//---------------------------------------------------------------------
// 変数を読み出し
//---------------------------------------------------------------------
bool JlsScrFuncReg::getJlsRegVarNormal(string& strVal, const string& strName){
	return ( getJlsRegVar(strVal, strName, true) > 0 );
}
int JlsScrFuncReg::getJlsRegVarPartName(string& strVal, const string& strCandName, bool exact){
	return getJlsRegVar(strVal, strCandName, exact);
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
int JlsScrFuncReg::getJlsRegVar(string& strVal, const string& strCandName, bool exact){
	//--- 分離オプションチェック ---
	string strNamePart = strCandName;
	string strDivPart;
	int lenDivFullVar;
	bool flagDivOpt = false;
	if ( exact ){
		flagDivOpt = checkJlsRegDivide(strNamePart, strDivPart, lenDivFullVar);
	}
	//--- リスト変数のチェック ---
	bool flagNum = false;
	if ( strNamePart[0] == '#' ){		// リストの要素数
		flagNum = true;
		strNamePart = strNamePart.substr(1);
	}
	int lenFullVar;
	if ( !fixJlsRegNameAtList(strNamePart, lenFullVar, exact) ){	// リスト要素補正
		strVal.clear();
		return 0;		// リスト要素部分の異常による終了
	}

	//--- 通常のレジスタ読み出し ---
	int lenVar = pGlobalState->getRegVarCommon(strVal, strNamePart, exact);

	//--- リスト変数時の補正 ---
	if ( (int)strNamePart.length() == lenVar ){	// 文字列の最後まで変数だった場合
		lenVar = lenFullVar;		// [項目番号]込みの変数文字列長にする
	}
	if ( flagNum ){		// リストの要素数取得
		int numList = pFuncList->getListStrSize(strVal);
		strVal = to_string(numList);
	}
	//--- 分離オプションによる分離 ---
	if ( flagDivOpt ){
		if ( divideJlsRegVar(strVal, strDivPart) ){
			lenVar = lenDivFullVar;		// 成功時は元の文字列長に戻す
		}else{
			lenVar = 0;		// オプションエラー時
		}
	}
	return lenVar;
}
//---------------------------------------------------------------------
// パス・拡張子分離オプション付きかチェック
// 入力：
//   strNamePart : 変数名（分離オプション込み）
// 出力：
//   返り値  : true=分離オプション付き  false=通常の変数
//   strNamePart : 変数名（分離オプションは除く）
//   strDivPart  : 分離オプション
//   lenFullVar  : 分離オプション込みの変数文字列長
//---------------------------------------------------------------------
bool JlsScrFuncReg::checkJlsRegDivide(string& strNamePart, string& strDivPart, int& lenFullVar){
	lenFullVar = (int) strNamePart.length();

	auto locSt = strNamePart.find("]");		// リスト配列時は要素後から
	if ( locSt == string::npos ){
		locSt = 0;
	}
	locSt = strNamePart.find(":", locSt);
	if ( locSt == string::npos ){
		strDivPart.clear();
		return false;
	}
	strDivPart = strNamePart.substr(locSt);
	strNamePart = strNamePart.substr(0, locSt);
	return true;
}
//---------------------------------------------------------------------
// パス・拡張子分離オプションの実行
// 入力：
//   strVal      : 変数値（分離前）
//   strDivPart  : 分離オプション
// 出力：
//   返り値  : true=分離オプション実行  false=分離オプションエラー
//   strVal      : 変数値（分離後）
//---------------------------------------------------------------------
bool JlsScrFuncReg::divideJlsRegVar(string& strVal, const string& strDivPart){
	auto locSt = strDivPart.find(":");
	if ( locSt == string::npos ) return true;	// 通常（:なし）は正常終了

	string strOpt = strDivPart;
	if ( strDivPart.find("$") != string::npos ){	// 内部に変数が使われていたら置換
		replaceBufVar(strOpt, strDivPart);
	}
	while( locSt != string::npos ){
		string strCmd;
		auto locNext = strOpt.find(":", locSt+1);
		if ( locNext == string::npos ){
			strCmd = strOpt.substr(locSt+1);
		}else{
			strCmd = strOpt.substr(locSt+1, locNext-locSt-1);
		}
		if ( !divideJlsRegVarDecode(strVal, strCmd) ){
			return false;
		}
		locSt = locNext;
	}
	return true;
}
//--- パス・拡張子分離コマンドのデコードと実行。エラーの時はfalseを返す ---
bool JlsScrFuncReg::divideJlsRegVarDecode(string& strVal, const string& strCmd){
	//--- コマンド文字列デコード ---
	VarProcRecord var;
	bool success = divideJlsRegVarDecodeIn(var, strCmd);
	if ( !success ){
		return false;
	}
	//--- 拡張子設定 ---
	string strDelim;
	if ( pGlobalState->getRegVarCommon(strDelim, DefRegExtChar, true) <= 0 ){
		strDelim.clear();
	}
	if ( strDelim.empty() ){	// 場合分け不要にするため必ず長さ1以上の文字列にする
		strDelim = ".";
	}
	string strSubDelim;
	pGlobalState->getRegVarCommon(strSubDelim, DefRegExtCsub, true);
	//--- 各種処理実行 ---
	switch( var.typeProc ){
		case VarProcType::path :		// パス分割
			pdata->cnv.getStrDivPath(strVal, var.selHead, var.withDelim);
			break;
		case VarProcType::divext :		// 拡張子分割
			pdata->cnv.getStrDivide(strVal, strDelim, var.selHead, var.selPath);
			break;
		case VarProcType::substr :	// 部分文字列の開始位置と長さを読み込み実行
			{
				int nSt;
				int pos = pdata->cnv.getStrValFuncNum(nSt, strCmd, 1);
				if ( pos >= 0 ){
					LocalStr lcstr;
					if ( pos == (int) strCmd.length() ){	// 長さ省略時
						strVal = lcstr.getSubStr(strVal, nSt);
					}else if ( strCmd[pos] != ',' ){	// 区切りは , 限定
						pos = -1;
					}else{
						int nLen;
						pos = pdata->cnv.getStrValFuncNum(nLen, strCmd, pos+1);
						strVal = lcstr.getSubStrLen(strVal, nSt, nLen);
					}
				}
				if ( pos < 0 ) return false;
			}
			break;
		case VarProcType::exchg :	// 拡張子文字列を置換
			if ( var.selInStr ){	// 各文字それぞれ置換
				LocalStr lcstr;
				success = lcstr.replaceInStr(strVal, strDelim, strSubDelim);
			}else if ( var.selQuote ){		// クォート置換
				if ( var.selBackup ){
					backupStrQuote(strVal);
				}else{
					restoreStrQuote(strVal);
				}
			}else{
				replaceStrAllFind(strVal, strDelim, strSubDelim);
			}
			break;
		case VarProcType::blank :	// スペース文字を全部除く
			{
				std::regex re("\\s+");	// スペースは正規表現でもマルチバイト処理不要
				strVal = std::regex_replace(strVal, re, "");
			}
			break;
		case VarProcType::trim :	// スペース文字を前後で除く
			{
				if ( var.selHead ){
					std::regex re("^\\s+");
					strVal = std::regex_replace(strVal, re, "");
				}
				if ( var.selTail ){
					std::regex re("\\s+$");
					strVal = std::regex_replace(strVal, re, "");
				}
			}
			break;
		case VarProcType::chpath :	// 最後がパス区切りでなければパス区切りを追加
			pdata->cnv.getStrFileAllPath(strVal);	// 最後に区切り付加
			break;
		case VarProcType::frame :	// 時間をフレーム数に変換
			{
				vector<Msec> listTmp;
				if ( pdata->cnv.getListValMsecM1(listTmp, strVal) ){
					strVal.clear();
					for(int i=0; i<(int)listTmp.size(); i++){
						if ( i>0 ) strVal += ",";
						strVal += pdata->cnv.getStringFrameMsecM1(listTmp[i]);
					}
				}
			}
			break;
		case VarProcType::match :	// 正規表現検索
			{
				LocalStr lcstr;
				strVal = lcstr.getRegMAtch(strVal, strDelim);
			}
			break;
		case VarProcType::count :	// 拡張子出現数を出力
			{
				int mc = 0;
				if ( var.selInStr ){		// いずれかの文字
					LocalStr lcstr;
					mc = lcstr.countInStr(strVal, strDelim);
				}
				else if ( var.selRegEx ){		// 正規表現で
					LocalStr lcstr;
					mc = lcstr.countRegExMatch(strVal, strDelim);
				}
				else{
					auto n = strVal.find(strDelim);
					while( n != string::npos ){
						mc ++;
						n = strVal.find(strDelim, n+1);
					}
				}
				strVal = to_string(mc);
			}
			break;
		case VarProcType::len :	// 文字列長を出力
			{
				LocalStr lcstr;
				int len = lcstr.getStrLen(strVal);
				strVal = to_string(len);
			}
			break;
		default :
			break;
	}
	return true;
}
//--- コマンド部分文字列を認識 ---
bool JlsScrFuncReg::divideJlsRegVarDecodeIn(VarProcRecord& var, const string& strCmd){
	bool success   = true;
	var.typeProc = VarProcType::none;
	var.selHead   = false;
	var.selTail   = false;
	var.selPath   = true;		// 拡張子検索はパス後に限定して行う設定
	var.withDelim = false;		// パスの最終区切りは出力しない設定
	var.selRegEx  = false;		// 正規表現で検索
	var.selInStr  = false;		// 各文字で検索
	var.selQuote  = false;		// 引用符処理
	var.selBackup = false;		// バックアップ選択
	if ( strCmd.length() == 1 ){
		switch( strCmd[0] ){
			case 'r' :			// 拡張子を取り除く
				var.typeProc = VarProcType::divext;
				var.selHead = true;
				break;
			case 'e' :			// 拡張子のみ残す
				var.typeProc = VarProcType::divext;
				var.selTail = true;
				break;
			case 'h' :			// パス部分のみ残す
				var.typeProc = VarProcType::path;
				var.selHead = true;
				break;
			case 't' :			// ファイル名以降のみ残す
				var.typeProc = VarProcType::path;
				var.selTail = true;
				break;
			case '~' :			// 部分文字列を残す
				var.typeProc = VarProcType::substr;
				break;
			case '<' :			// スペース文字を前側で除く
				var.typeProc = VarProcType::trim;
				var.selHead = true;
				break;
			case '>' :			// スペース文字を後側で除く
				var.typeProc = VarProcType::trim;
				var.selTail = true;
				break;
			case '/' :			// 最後がパス区切りでなければパス区切りを追加
				var.typeProc = VarProcType::chpath;
				break;
			case 'f' :			// 時間をフレーム数に変換
				var.typeProc = VarProcType::frame;
				break;
			case 'X' :			// 拡張子文字列を置換
				var.typeProc = VarProcType::exchg;
				break;
			case 'R' :			// 拡張子を取り除く（パスを無視して先頭から検索）
				var.typeProc = VarProcType::divext;
				var.selHead = true;
				var.selPath = false;
				break;
			case 'E' :			// 拡張子のみ残す（パスを無視して先頭から検索）
				var.typeProc = VarProcType::divext;
				var.selTail = true;
				var.selPath = false;
				break;
			case 'H' :			// パス部分のみ残す（最後のパス区切りを含む）
				var.typeProc = VarProcType::path;
				var.selHead = true;
				var.withDelim = true;
				break;
			case 'M' :
				var.typeProc = VarProcType::match;
				break;
			case 'C' :			// 拡張子出現数を出力
				var.typeProc = VarProcType::count;
				break;
			case 'L' :			// 文字列長を出力
				var.typeProc = VarProcType::len;
				break;
			default :
				var.typeProc = VarProcType::none;
				success = false;
				break;
		}
	}
	else{
		if ( strCmd == "<>" ){		// スペース文字を前後で除く
			var.typeProc = VarProcType::trim;
			var.selHead = true;
			var.selTail = true;
		}
		else if ( strCmd == "Xi" ){		// 拡張子いずれか文字を置換
			var.typeProc = VarProcType::exchg;
			var.selInStr = true;
		}
		else if ( strCmd == "Xb" ){		//  スペース文字を全部除く
			var.typeProc = VarProcType::blank;
		}
		else if ( strCmd == "Xq" ){		// クォートを置換退避
			var.typeProc = VarProcType::exchg;
			var.selQuote = true;
			var.selBackup = true;
		}
		else if ( strCmd == "Xqr" ){	// 置換退避を元に戻す
			var.typeProc = VarProcType::exchg;
			var.selQuote = true;
			var.selBackup = false;
		}
		else if ( strCmd == "Ci" ){		// 拡張子いずれか文字の出現数を出力
			var.typeProc = VarProcType::count;
			var.selInStr = true;
		}
		else if ( strCmd == "Cm" ){		// 正規表現で拡張子出現数を出力
			var.typeProc = VarProcType::count;
			var.selRegEx = true;
		}
		else{
			success = false;
		}
	}
	if ( strCmd.length() > 0 ){
		if ( strCmd[0] == '~' ){		// 部分文字列を残す
			var.typeProc = VarProcType::substr;
			success = true;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// リスト変数の要素を修正。要素だった時は要素内の演算をして整数値にする
// 入力：
//   strNamePart : 変数名（[番号]込み）
//   exact       : true=変数名全体  false=先頭から変数名と一致あり
// 出力：
//   返り値  : true=成功  false=失敗
//   strNamePart : 変数名（[番号]は演算済みの数値）
//   lenFullVar  : [番号]込みの変数文字列長（変更前の変数名で）
//---------------------------------------------------------------------
bool JlsScrFuncReg::fixJlsRegNameAtList(string& strNamePart, int& lenFullVar, bool exact){
	//--- 変数名[項目番号] ならリスト変数の要素 ---
	lenFullVar = (int)strNamePart.length();
	auto locSt = strNamePart.find("[");
	if ( locSt == string::npos ){	// 配列がなければ正常終了
		return true;
	}else{
		auto locCt = strNamePart.find("$");
		if ( locCt != string::npos && locCt < locSt ){	// [の前に$があれば今回の変数ではないので除く
			if ( exact ){
				pGlobalState->addMsgErrorN("error: bad $ in " + strNamePart);
				return false;
			}else{
				lenFullVar = (int)locCt;
				strNamePart = strNamePart.substr(0, locCt);
				return true;
			}
		}
	}
	string strNew = strNamePart.substr(0, locSt);
	bool success = true;
	bool cont = true;
	while( cont ){		// 多次元対応ループ
		int dup = 0;
		auto locEd = locSt + 1;
		while( (strNamePart[locEd] != ']' || dup > 0) && strNamePart[locEd] != '\0' ){
			if ( strNamePart[locEd] == ']' ){
				dup --;
			}else if ( strNamePart[locEd] == '[' ){
				dup ++;
			}
			locEd ++;
		}
		success = false;
		cont = false;
		if ( strNamePart[locEd] == ']' && locSt+1 < locEd ){	// []内の文字列を取得し、その中の変数置換を実施
			string strAryBuf = strNamePart.substr(locSt + 1, locEd - locSt - 1);
			string strItem;
			if ( replaceBufVar(strItem, strAryBuf) ){
				//--- 項目番号の取得 ---
				int numItem;
				if ( pdata->cnv.getStrValNum(numItem, strItem, 0) >= 0 ){
					success = true;
					strNew += "[";
					strNew += to_string(numItem);
					strNew += "]";
					lenFullVar = (int) locEd + 1;
					if ( strNamePart[locEd+1] == '[' ){		// 多次元対応
						locSt = locEd + 1;
						cont = true;
					}
				}
			}
		}
	}
	if ( success ){
		if ( (int)strNamePart.length() != lenFullVar && exact ){
			pGlobalState->addMsgErrorN("warning: unrecognized charactor in " + strNamePart);
		}
		strNamePart = strNew;
		return true;
	}
	pGlobalState->addMsgErrorN("error: [value] must be integer in " + strNamePart);
	return false;
}
//--- 対象文字列をすべて置換 ---
void JlsScrFuncReg::replaceStrAllFind(string& strVal, const string& strFrom, const string& strTo){
	if ( !strVal.empty() && !strFrom.empty() ){
		auto n = strVal.find(strFrom);
		while( n != string::npos ){
			strVal = strVal.replace(n, strFrom.length(), strTo);
			n = strVal.find(strFrom, n+strTo.length());
		}
	}
}
//--- 引用符を制御無関係文字に置換退避 ---
void JlsScrFuncReg::backupStrQuote(string& strVal){
	string strFrom = "\"";
	string strTo;
	pGlobalState->getRegVarCommon(strTo, DefRegDQuote, true);
	replaceStrAllFind(strVal, strFrom, strTo);
	strFrom = "\'";
	pGlobalState->getRegVarCommon(strTo, DefRegSQuote, true);
	replaceStrAllFind(strVal, strFrom, strTo);
}
//--- 退避している引用符文字列を元に戻す ---
void JlsScrFuncReg::restoreStrQuote(string& strVal){
	string strFrom;
	pGlobalState->getRegVarCommon(strFrom, DefRegDQuote, true);
	string strTo   = "\"";
	replaceStrAllFind(strVal, strFrom, strTo);
	pGlobalState->getRegVarCommon(strFrom, DefRegSQuote, true);
	strTo   = "\'";
	replaceStrAllFind(strVal, strFrom, strTo);
}

//=====================================================================
// 変数名部分の置換
//=====================================================================

//---------------------------------------------------------------------
// 変数部分を置換した文字列出力
// 入力：
//   strBuf : 文字列
// 出力：
//   返り値  ：置換結果（true=成功  false=失敗）
//   dstBuf  : 出力文字列
//---------------------------------------------------------------------
bool JlsScrFuncReg::replaceBufVar(string& dstBuf, const string& srcBuf){
	dstBuf.clear();
	bool success = true;
	int pos_cmt = pdata->cnv.getStrPosComment(srcBuf, 0);
	int pos_base = 0;
	while(pos_base >= 0){
		//--- 変数部分の置換 ---
		int pos_var = pdata->cnv.getStrPosReplaceVar(srcBuf, pos_base);	// $位置
		if (pos_var >= 0){
			//--- コメント領域確認 ---
			bool bakCmt = pGlobalState->isStopAddMsgError();
			bool flagCmt = ( pos_var >= pos_cmt && pos_cmt >= 0 )? true : bakCmt;
			if ( flagCmt ){
				pGlobalState->stopAddMsgError(true);	// エラー出力停止
			}
			//--- $手前までの文字列を確定 ---
			if (pos_var > pos_base){
				dstBuf += srcBuf.substr(pos_base, pos_var-pos_base);
				pos_base = pos_var;
			}
			//--- 変数を検索して置換 ---
			string strVal;
			int len_var = replaceRegVarInBuf(strVal, srcBuf, pos_var);
			if (len_var > 0){
				string strRev;
				success = replaceBufVar(strRev, strVal);	// 内部再検索（未展開変数対応）
				dstBuf += strRev;
				pos_base += len_var;
			}
			else{
				if ( flagCmt == false ){		// コメントでなければ置換失敗
					success = false;
					pGlobalState->addMsgErrorN("warning: not defined variable in " + srcBuf);
				}
				pos_var = -1;
			}
			if ( flagCmt ){
				pGlobalState->stopAddMsgError(bakCmt);	// エラー出力状態復帰
			}
		}
		//--- 変数がなければ残りすべてコピー ---
		if (pos_var < 0){
			dstBuf += srcBuf.substr(pos_base);
			pos_base = -1;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// 対象位置の変数を読み出し
// 入力：
//   strBuf : 文字列
//   pos    : 認識開始位置
// 出力：
//   返り値  ：変数部分の文字数
//   strVal  : 変数値
//---------------------------------------------------------------------
int JlsScrFuncReg::replaceRegVarInBuf(string& strVal, const string& strBuf, int pos){
	int var_st, var_ed;
	bool exact;
	bool flagNum = false;
	bool flagConv = false;

	int ret = 0;
	if (strBuf[pos] == '$'){
		flagConv = true;
		//--- 変数部分を取得 ---
		pos ++;
		if ( strBuf[pos] == '#' ){		// $#はリスト要素数
			flagNum = true;
			pos ++;
		}
		if (strBuf[pos] == '{'){		// ${変数名}フォーマット時の処理
			exact = true;
			pos ++;
			int dup = 0;
			var_st = pos;
			while( (strBuf[pos] != '}' || dup > 0) && strBuf[pos] != '\0' ){
				if ( strBuf[pos] == '}' ){
					dup --;
				}else if ( strBuf[pos] == '{' ){
					dup ++;
				}
				pos ++;
			}
			if ( strBuf[pos] == '\0' ){
				flagConv = false;
			}
		}
		else{							// $変数名フォーマット時の処理
			exact = false;
			var_st = pos;
			while(strBuf[pos] != ' ' && strBuf[pos] != '\0'){
				pos ++;
			}
		}
		var_ed = pos;
		if (strBuf[pos] == '}' || strBuf[pos] == ' '){
			var_ed -= 1;
		}
		//--- 変数読み出し実行 ---
		if ( var_st <= var_ed && flagConv ){
			string strCandName = strBuf.substr(var_st, var_ed-var_st+1);
			if ( flagNum ){			// リスト要素数
				strCandName = "#" + strCandName;
			}
			int nmatch = getJlsRegVarPartName(strVal, strCandName, exact);
			if (nmatch > 0){
				ret = nmatch + 1;	// 変数名数 + $
				if ( flagNum ){
					ret += 1;		// #
				}
				if ( exact ){
					ret += 2;		// {}
					if ( ret < var_ed-var_st+2 ){
						// エラーであるが異常はエラー出さなくてもわかるのでそのまま
					}
				}
			}
		}
	}
	return ret;
}


//=====================================================================
// システム変数設定
//=====================================================================

//---------------------------------------------------------------------
// 初期設定変数
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegInit(){
	setSystemRegHeadtail(-1, -1);
	setSystemRegUpdate();
	//--- 書き換えにwarningを出すグローバル変数設定の解除 ---
	pGlobalState->setGlobalLock(DefRegExtChar, false);
	pGlobalState->setGlobalLock(DefRegDQuote, false);
	pGlobalState->setGlobalLock(DefRegSQuote, false);
	//--- 固定グローバル変数 ---
	setJlsRegVar(DefRegExtChar, ".", true);
	setJlsRegVar(DefRegExtCsub, ".", true);
	setJlsRegVar(DefRegDQuote, DefStrRepDQ, true);
	setJlsRegVar(DefRegSQuote, DefStrRepSQ, true);
	//--- 書き換えにwarningを出すグローバル変数設定 ---
	pGlobalState->setGlobalLock(DefRegExtChar, true);
	pGlobalState->setGlobalLock(DefRegDQuote, true);
	pGlobalState->setGlobalLock(DefRegSQuote, true);
}

//---------------------------------------------------------------------
// 初期設定変数の現在値による変更
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegUpdate(){
	int n = pdata->getMsecTotalMax();
	string str_val = pdata->cnv.getStringFrameMsecM1(n);
	string str_time = pdata->cnv.getStringTimeMsecM1(n);
	setJlsRegVar("MAXFRAME", str_val, true);
	setJlsRegVar("MAXTIME", str_time, true);
	setJlsRegVar("NOLOGO", to_string(pdata->extOpt.flagNoLogo), true);
	setJlsRegVar("RANGETYPE", to_string(pdata->recHold.typeRange), true);
	setSystemRegFilePath();
	setSystemRegFileOpen();
}
//---------------------------------------------------------------------
// Path関連の現在値による変更
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegFilePath(){
	string strDataLast = "data";		// リストデータ読み込み用JLサブパスからの位置

	//--- JLサブパス読み込み ---
	string strPathSub;
	if ( pdata->extOpt.subPath.empty() == false ){
		strPathSub = pdata->extOpt.subPath;
	}else{
		strPathSub = pGlobalState->getPathNameJL();
	}
	pdata->cnv.getStrFileAllPath(strPathSub);	// 最後に区切り付加
	//--- JLデータパス ---
	string strPathData = strPathSub + strDataLast;
	pdata->cnv.getStrFileAllPath(strPathData);	// 最後に区切り付加
	//--- 初期設定変数の変更 ---
	setJlsRegVar("JLDATAPATH", strPathData, true);
}
//---------------------------------------------------------------------
// Echo出力先がファイルか判別設定
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegFileOpen(){
	string strVal = ( pGlobalState->fileIsOpen() )? "1" : "0";
	setJlsRegVar("FILEOPEN", strVal, true);
}

//---------------------------------------------------------------------
// HEADFRAME/TAILFRAMEを設定
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegHeadtail(int headframe, int tailframe){
	string str_head = pdata->cnv.getStringTimeMsecM1(headframe);
	string str_tail = pdata->cnv.getStringTimeMsecM1(tailframe);
	setJlsRegVar("HEADTIME", str_head, true);
	setJlsRegVar("TAILTIME", str_tail, true);
}

//---------------------------------------------------------------------
// 無効なロゴの確認（ロゴ期間が極端に短かったらロゴなし扱いにする）
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegNologo(bool need_check){
	bool flag_nologo = false;
	//--- ロゴ期間が極端に少ない場合にロゴ無効化する場合の処理 ---
	if (need_check == true && pdata->extOpt.flagNoLogo == 0){
		int msec_sum = 0;
		int nrf_rise = -1;
		int nrf_fall = -1;
		do{
			nrf_rise = pdata->getNrfNextLogo(nrf_fall, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
			nrf_fall = pdata->getNrfNextLogo(nrf_rise, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
			if (nrf_rise >= 0 && nrf_fall >= 0){
				msec_sum += pdata->getMsecLogoNrf(nrf_fall) - pdata->getMsecLogoNrf(nrf_rise);
			}
		} while(nrf_rise >= 0 && nrf_fall >= 0);
		if (msec_sum < pdata->getConfig(ConfigVarType::msecWLogoSumMin)){
			flag_nologo = true;
		}
	}
	else{		// チェックなしでロゴ無効の場合
			flag_nologo = true;
	}
	if (flag_nologo == true){
		// ロゴ読み込みなしに変更
		pdata->extOpt.flagNoLogo = 1;
		// システム変数を更新
		setJlsRegVar("NOLOGO", "1", true);	// 上書き許可で"1"設定
	}
}

//---------------------------------------------------------------------
// Readコマンド結果の有効有無を設定
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegReadValid(bool valid){
	string strVal = ( valid )? "1" : "0";
	setJlsRegVar("READVALID", strVal, true);
}
//---------------------------------------------------------------------
// 前回の実行状態を設定
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegLastexe(bool exe_command){
	//--- 前回の実行状態を変数に設定 ---
	setJlsRegVar("LASTEXE", to_string((int)exe_command), true);
}
//--- 直前の実行状態取得 ---
bool JlsScrFuncReg::isSystemRegLastexe(){
	string strVal = "0";
	getJlsRegVarNormal(strVal, "LASTEXE");
	bool lastExe = ( strVal != "0" )? true : false;
	return lastExe;
}


//---------------------------------------------------------------------
// CMカット位置を直接出力
//---------------------------------------------------------------------
void JlsScrFuncReg::setOutDirect(){
	string strList;
	getJlsRegVarNormal(strList, "OUTDIRECT");
	if ( strList.empty() ){
		return;
	}
	int nsize = pFuncList->getListStrSize(strList);
	if ( nsize % 2 == 1 ){	// 必ず2データ単位
		nsize = nsize - 1;
	}
	vector<Msec> listMsec;
	for(int i=0; i<nsize; i++){
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strList, i+1) == false ){
			return;
		}
		int val;
		if ( pdata->cnv.getStrValMsec(val, strItem, 0) <= 0 ){
			return;
		}
		listMsec.push_back(val);
	}
	pdata->setOutDirect(listMsec);
	if ( !listMsec.empty() ){
		string mes = "OUTDIRECT is set for Trim.";
		pdata->dispSysMesN(mes, JlsDataset::SysMesType::OutDirect);
	}
}


//---------------------------------------------------------------------
// スクリプト内で記載する起動オプション
// 入力：
//   strBuf     ：オプションを含む文字列
//   pos        ：読み込み開始位置
//   overwrite  ：書き込み済みのオプション設定（false=しない true=する）
// 出力：
//   返り値  ：true=正常終了 false=設定エラー
//---------------------------------------------------------------------
bool JlsScrFuncReg::setSystemRegOptions(const string& strBuf, int pos, bool overwrite){
	//--- 文字列区切り認識 ---
	vector <string> listarg;
	string strWord;
	while(pos >= 0){
		pos = pdata->cnv.getStrItem(strWord, strBuf, pos);
		if (pos >= 0){
			listarg.push_back(strWord);
		}
	}
	int argc = (int) listarg.size();
	if (argc <= 0){
		return true;
	}
	//--- スクリプト内で設定可能なオプション ---
	int i = 0;
	while(i >= 0 && i < argc){
		int argrest = argc - i;
		const char* strv = listarg[i].c_str();
		const char* str1 = nullptr;
		const char* str2 = nullptr;
		if (argrest >= 2){
			str1 = listarg[i+1].c_str();
		}
		if (argrest >= 3){
			str2 = listarg[i+2].c_str();
		}
		int numarg = setOptionsGetOne(argrest, strv, str1, str2, overwrite);
		if (numarg < 0){
			return false;
		}
		if (numarg > 0){
			i += numarg;
		}
		else{		// 実行可能コマンドでなければ次に移行
			i ++;
		}
	}
	setSystemRegUpdate();	// Path情報の更新（念のため初期設定変数全体）
	return true;
}


//---------------------------------------------------------------------
// 情報取得
//---------------------------------------------------------------------
void JlsScrFuncReg::getSystemData(JlsCmdArg& cmdarg, const string& strIdent){
	string strData;
	if ( strIdent == "AUTOMODE" ){
		strData = ( pdata->isAutoModeUse() )? "1" : "0";
	}
	else if ( strIdent == "DETMODE" ){
		strData = ( pGlobalState->isExe1st() )? "0" : "1";
	}
	else if ( strIdent == "FCURRENT" ){
		strData = LSys.getCurrentPath();
	}
	else if ( strIdent == "FINLOGO" ){
		strData = pdata->extOpt.logofile;
	}
	else if ( strIdent == "FINSCP" ){
		strData = pdata->extOpt.scpfile;
	}
	else if ( strIdent == "FINCMD" ){
		strData = pdata->extOpt.cmdfile;
	}
	else if ( strIdent == "FOUTAVS" ){
		strData = pdata->extOpt.outfile;
	}
	else if ( strIdent == "FOUTSCP" ){
		strData = pdata->extOpt.outscpfile;
	}
	else if ( strIdent == "FOUTDIV" ){
		strData = pdata->extOpt.outdivfile;
	}
	else if ( strIdent == "SYSCODE" ){
		strData = to_string(pdata->extOpt.nSysCode);
	}
	else if ( strIdent == "FILECODE" ){
		strData = to_string(pGlobalState->fileGetCodeDefaultNum());
	}
	else if ( strIdent == "READCODE" ){
		strData = to_string(pGlobalState->readGCodeNum());
	}
	else if ( strIdent == "TRIMLIST" ){
		//--- 結果作成 ---
		pdata->outputResultTrimGen();
		pFuncList->setListStrClear(strData);
		int num_data = (int) pdata->resultTrim.size();
		for(int i=0; i<num_data-1; i+=2){
			int frm_st = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i] );
			int frm_ed = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i+1] );
			pFuncList->setListStrIns(strData, to_string(frm_st), -1);
			pFuncList->setListStrIns(strData, to_string(frm_ed), -1);
		}
	}
	else{
		pGlobalState->addMsgErrorN("error: No SysData name(" + strIdent + ")");
		return;
	}
	setRegOutSingle(cmdarg, strData, true);	// RegOutに無条件で書き込み
}


//=====================================================================
// コマンド結果による変数更新
//=====================================================================

//--- $POSHOLD/$LISTHOLDのレジスタ更新 ---
void JlsScrFuncReg::updateResultRegWrite(JlsCmdArg& cmdarg){
	//--- 共通設定 ---
	bool   overwrite = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	//--- POSHOLDの更新 ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValPosW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegOut);	// 変数名($POSHOLD)
		string strVal    = cmdarg.getStrOpt(OptType::StrValPosW);
		setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// $POSHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// 更新完了通知
	}
	//--- LISTHOLDの更新 ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValListW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegOut);	// 変数名($LISTHOLD)
		string strList   = cmdarg.getStrOpt(OptType::StrValListW);
		setJlsRegVarWithLocal(strName, strList, overwrite, flagLocal);	// $LISTHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValListW);	// 更新完了通知
	}
}
//--- $SIZEHOLDのレジスタ設定 ---
void JlsScrFuncReg::setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList){
	string strSizeName = cmdarg.getStrOpt(OptType::StrRegSize);	// 変数名($SIZEHOLD)
	int    numList     = pFuncList->getListStrSize(strList);	// 項目数取得
	string strNumList  = std::to_string(numList);
	bool   overwrite   = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal   = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	setJlsRegVarWithLocal(strSizeName, strNumList, overwrite, flagLocal);	// $SIZEHOLD
}
//--- $POSHOLDの更新 ---
void JlsScrFuncReg::setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos){
	string strVal  = pdata->cnv.getStringTimeMsecM1(msecPos);
	cmdarg.setStrOpt(OptType::StrValPosW, strVal);			// $POSHOLD write
	updateResultRegWrite(cmdarg);
}

//--- $LISTHOLDの更新 ---
void JlsScrFuncReg::setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos){
	//--- レジスタの現在値を取得 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write変数値

	//--- 項目追加 ---
	if ( msecPos != -1 ){
		if ( pFuncList->isListStrEmpty(strList) ){
			strList.clear();
		}else{
			strList += ",";
		}
		string strVal = pdata->cnv.getStringTimeMsecM1(msecPos);
		strList += strVal;
	}
	//--- ListHoldに設定 ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListGetAtによる更新 ---
void JlsScrFuncReg::setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosW);
	//--- 対象位置の項目を取得 ---
	{
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strList, numItem) ){
			strValPos = strItem;
		}
	}
	//--- PosHoldに設定 ---
	cmdarg.setStrOpt(OptType::StrValPosW, strValPos);		// $POSHOLD write
	updateResultRegWrite(cmdarg);
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListInsによる更新 ---
void JlsScrFuncReg::setResultRegListIns(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){		// -RegArg
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		if ( !getJlsRegVarNormal(strValPos, strName) ){
			pGlobalState->addMsgErrorN("error: -RegArg " + strName + " at ListIns");
		}
	}
	//--- Ins処理 ---
	if ( pFuncList->setListStrIns(strList, strValPos, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHoldに設定
	}
}
//--- ListDelによる更新 ---
void JlsScrFuncReg::setResultRegListDel(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- Del処理 ---
	if ( pFuncList->setListStrDel(strList, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHoldに設定
	}
}
//--- ListJoinによる更新 ---
void JlsScrFuncReg::setResultRegListJoin(JlsCmdArg& cmdarg){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	int numItem = -1;	// リスト最後に追加
	//--- リスト値データを取得 ---
	string strListIns;
	if ( !setResultRegSubGetRegVal(cmdarg, strListIns, true) ){	// must=true
		return;
	}
	//--- Ins処理 ---
	int numCur = numItem;
	int nsizeIns = pFuncList->getListStrSize(strListIns);
	for(int i=1; i<=nsizeIns; i++){
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strListIns, i) ){
			if ( pFuncList->setListStrIns(strList, strItem, numCur) ){
				if ( numItem > 0 ) numCur++;	// 前から数える時は次の挿入位置を変更
			}
		}
	}
	if ( cmdarg.getOptFlag(OptType::FlagSort) ){	// -sort
		sortResultRegList(cmdarg, strList);
	}
	//--- ListHoldに設定 ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListRemoveによる更新 ---
void JlsScrFuncReg::setResultRegListRemove(JlsCmdArg& cmdarg){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	bool   flagLap   = cmdarg.getOptFlag(OptType::FlagOverlap);
	//--- リスト値データを取得 ---
	string strListRm;
	if ( !setResultRegSubGetRegVal(cmdarg, strListRm, true) ){	// must=true
		return;
	}
	//--- Remove処理 ---
	bool success = true;
	if ( flagLap ){
		success = pFuncList->setListStrRemoveLap(strList, strListRm);
	}else{
		success = pFuncList->setListStrRemove(strList, strListRm);
	}
	//--- ListHoldに設定 ---
	if ( success ){
		writeResultRegListW(cmdarg, strList);	// ListHoldに設定
	}else{
		pGlobalState->addMsgErrorN("error: list data at ListRemove");
	}
}
//--- 引数リスト値を取得 ---
bool JlsScrFuncReg::setResultRegSubGetRegVal(JlsCmdArg& cmdarg, string& strListSub, bool must){
	bool success = false;
	string mesErr;
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		success = getJlsRegVarNormal(strListSub, strName);
		mesErr = "not found data by -RegArg " + strName + " ";
	}
	else if ( cmdarg.isSetStrOpt(OptType::StrArgVal) ){
		strListSub = cmdarg.getStrOpt(OptType::StrArgVal);
		success = true;
	}
	else if ( must ){
		mesErr = "need arg option for the list join/remove command";
	}
	if ( !success && !mesErr.empty() ){
		pGlobalState->addMsgErrorN("error: " + mesErr);
	}
	return success;
}
//--- ListSelによる取り込み ---
void JlsScrFuncReg::setResultRegListSel(JlsCmdArg& cmdarg, string strListNum){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- List選択処理 ---
	if ( pFuncList->setListStrSel(strList, strListNum) ){
		writeResultRegListW(cmdarg, strList);	// ListHoldに設定
	}
}
//--- ListSetAtによる更新 ---
void JlsScrFuncReg::setResultRegListRep(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	//--- Replace処理 ---
	if ( pFuncList->setListStrRep(strList, strValPos, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHoldに設定
	}
}
//--- ListClearによるリスト変数内容を消去 ---
void JlsScrFuncReg::setResultRegListClear(JlsCmdArg& cmdarg){
	//--- リスト変数の初期値 ---
	string strList;
	pFuncList->setListStrClear(strList);
	//--- ListHoldに設定 ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListDimによるリスト変数項目確保 ---
void JlsScrFuncReg::setResultRegListDim(JlsCmdArg& cmdarg, int num){
	string strVal;
	setResultRegSubGetRegVal(cmdarg, strVal, false);	// must=false
	string strList;
	pFuncList->setListStrDim(strList, num, strVal);
	//--- ListHoldに設定 ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListSortによる更新 ---
void JlsScrFuncReg::setResultRegListSort(JlsCmdArg& cmdarg){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	sortResultRegList(cmdarg, strList);
	//--- ListHoldに設定 ---
	writeResultRegListW(cmdarg, strList);
}
void JlsScrFuncReg::sortResultRegList(JlsCmdArg& cmdarg, string& strList){
	bool   flagUni   = ( cmdarg.getOpt(OptType::FlagUnique)>0 )? true : false;
	bool   flagLap   = ( cmdarg.getOpt(OptType::FlagOverlap) > 0 );
	bool   flagMerge = ( cmdarg.getOpt(OptType::FlagMerge) > 0 );
	//--- sort処理 ---
	if ( flagLap ){		// -overlap
		pFuncList->setListStrSortLap(strList, flagMerge);
	}else{
		pFuncList->setListStrSort(strList, flagUni);
	}
}
//--- リスト変数出力更新 ---
void JlsScrFuncReg::writeResultRegListW(JlsCmdArg& cmdarg, const string& strList){
	//--- ListHoldに設定 ---
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}

//--- 直接文字列指定でCSV形式文字列をリスト文字列に分解 ---
string JlsScrFuncReg::getStrRegListByCsvStr(const string& strBuf){
	//--- リストに分解 ---
	string strList;
	pFuncList->setListStrClear(strList);
	int pos = 0;
	while( pos >= 0 ){
		string strItem;
		pos = pdata->cnv.getStrCsv(strItem, strBuf, pos);
		if ( pos >= 0 ){
			pFuncList->setListStrIns(strList, strItem, -1);
		}
	}
	return strList;
}
//--- CSV形式文字列をリストに分解して格納 ---
void JlsScrFuncReg::setStrRegListByCsv(JlsCmdArg& cmdarg){
	string strArg;
	if ( getRegArg(cmdarg, strArg) ){
		string strList = getStrRegListByCsvStr(strArg);
		setRegOutSingle(cmdarg, strList, true);		// 無条件で書き込み
	}
}
//--- スペース区切りの文字列をリストに分解して格納 ---
void JlsScrFuncReg::setStrRegListBySpc(JlsCmdArg& cmdarg){
	string strArg;
	if ( getRegArg(cmdarg, strArg) ){
		setStrRegListCommon(cmdarg, strArg, 0);
	}
}
//--- 文字列をリストに分解して格納の共通処理 ---
void JlsScrFuncReg::setStrRegListCommon(JlsCmdArg& cmdarg, const string& strBuf, int readtype){
	//--- リストに分解 ---
	string strList;
	pFuncList->setListStrClear(strList);
	int pos = 0;
	while( pos >= 0 ){
		string strItem;
		if ( readtype == 1 ){
			pos = pdata->cnv.getStrCsv(strItem, strBuf, pos);
		}else{
			pos = pdata->cnv.getStrItem(strItem, strBuf, pos);
		}
		if ( pos >= 0 ){
			pFuncList->setListStrIns(strList, strItem, -1);
		}
	}
	setRegOutSingle(cmdarg, strList, true);		// 無条件で書き込み
}

//=====================================================================
// データ用ファイル読み込み
//=====================================================================

//---------------------------------------------------------------------
// ファイルが存在するかリストデータに設定
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataCheck(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::Check;	// ファイル有無を確認
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// リストデータ（数値）をファイルから読み込み（１行１データ）
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataList(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::List;	// リストから数値を読み込み
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// リストデータをAVSファイルのTrimから読み込み
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataTrim(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::Trim;	// Trimからの数値を読み込み
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// リストデータ（文字列）をファイルから読み込み（１行１データ）
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataString(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::String;	// 文字列を読み込み
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// リストデータをファイルから読み込みの共通処理
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataCommon(JlsCmdArg& cmdarg, const string& fname, ReadFileType rtype){
	//--- リストデータクリア ---
	updateResultRegWrite(cmdarg);	// 変数($LISTHOLD)クリア
	//--- データ数上限設定 ---
	int nMax = -1;
	if ( cmdarg.isSetOpt(OptType::NumMaxSize) ){
		nMax = cmdarg.getOpt(OptType::NumMaxSize);
	}
	//--- ファイル読み込み ---
	LocalIfs ifs(fname.c_str());
	//--- 存在チェック ---
	if ( rtype == ReadFileType::Check ){
		bool exist = ifs.is_open();
		if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
			string strVal = ( exist )? "1" : "0";
			setRegOutSingle(cmdarg, strVal, true);	// RegOutに無条件で書き込み
		}
		return exist;
	}
	if ( !ifs.is_open() ){
		if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
			pGlobalState->fileOutput("warning: file not found(" + fname + ")\n");
		}
		return false;
	}
	//--- データ読み込み ---
	int nCur = 0;
	if ( rtype == ReadFileType::Trim ){
		string strCmd;
		if ( readDataFileTrim(strCmd, ifs) ){	// Trim行読み込み
			readDataCommonIns(cmdarg, nCur, strCmd, nMax, rtype);
		}
	}else{
		bool cont = true;
		while( cont ){
			string strLine;
			cont = readDataFileLine(strLine, ifs);	// １行読み込み
			if ( cont ){
				cont = readDataCommonIns(cmdarg, nCur, strLine, nMax, rtype);
			}
		}
	}
	return ( nCur > 0 );
}
//--- 1行分データを格納 ---
bool JlsScrFuncReg::readDataCommonIns(JlsCmdArg& cmdarg, int& nCur, const string& strLine, int nMax, ReadFileType rtype){
	//--- データ数上限の確認 ---
	if ( nCur >= nMax && nMax >= 0 ){
		return false;
	}
	//--- 種類別の挿入処理 ---
	switch( rtype ){
		case ReadFileType::Trim :
			{
				string strRest = strLine;
				bool cont = true;
				while( cont ){			// Trimの数だけ継続
					string strLocSt;
					string strLocEd;
					if ( nCur+1 >= nMax && nMax >= 0 ){		// データ数上限
						cont = false;
					}else{
						cont = readDataStrTrimGet(strLocSt, strLocEd, strRest);
					}
					if ( cont ){
						nCur += 2;
						readDataStrAdd(cmdarg, strLocSt);
						readDataStrAdd(cmdarg, strLocEd);
					}
				}
			}
			break;
		case ReadFileType::List :
			{
				string sdata;
				if ( pdata->cnv.getStrItem(sdata, strLine, 0) > 0 ){	// 1項目取得
					int val;
					if ( pdata->cnv.getStrValNum(val, sdata, 0) > 0 ){	// 項目が数値
						nCur ++;
						readDataStrAdd(cmdarg, sdata);
					}
				}
			}
			break;
		default :
			nCur ++;
			readDataStrAdd(cmdarg, strLine);
			break;
	}
	//--- データ数上限の確認 ---
	if ( nCur >= nMax && nMax >= 0 ){
		return false;
	}
	return true;
}
//---------------------------------------------------------------------
// リストに１項目追加
//---------------------------------------------------------------------
void JlsScrFuncReg::readDataStrAdd(JlsCmdArg& cmdarg, const string& sdata){
	//--- レジスタの現在値を取得 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write変数値
	cmdarg.setStrOpt(OptType::StrValListR, strList);
	cmdarg.setStrOpt(OptType::StrValPosR, sdata);
	setResultRegListIns(cmdarg, -1);
}
//---------------------------------------------------------------------
// Trim文字列の開始・終了位置を取得して文字列は次の位置に移行
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataStrTrimGet(string& strLocSt, string& strLocEd, string& strTarget){
	std::regex re(DefRegExpTrim, std::regex_constants::icase);
	std::smatch m;
	bool success = std::regex_search(strTarget, m, re);
	if ( success ){
		strLocSt = m[1].str();
		strLocEd = m[2].str();
		strTarget = m.suffix();
	}
	return success;
}
//---------------------------------------------------------------------
// Trim文字列の存在確認
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataStrTrimDetect(const string& strLine){
	string strTarget = strLine;
	string strLocSt;
	string strLocEd;
	return readDataStrTrimGet(strLocSt, strLocEd, strTarget);
}

//---------------------------------------------------------------------
// データ１行読み込み
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataFileLine(string& strLine, LocalIfs& ifs){
	return pGlobalState->readLineIfs(strLine, ifs);
}
//---------------------------------------------------------------------
// AVSファイルTrim行読み込み
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataFileTrim(string& strCmd, LocalIfs& ifs){
	strCmd = "";
	bool flagTrim = false;
	bool flagKeepNext = false;
	bool cont = true;
	while( cont ){
		string strLine;
		cont = readDataFileLine(strLine, ifs);	// １行読み込み
		if ( cont ){
			int len = (int) strLine.length();
			//--- 継続確認 ---
			bool flagKeepCur = flagKeepNext;
			flagKeepNext = false;
			if ( len >= 1 ){
				if ( strLine.substr(0,1) == R"(\)" ){	// 最初に継続文字
					flagKeepCur = true;
					strLine = strLine.substr(1);
					len = (int) strLine.length();
				}
			}
			if ( len >= 1 ){
				if ( strLine.substr(len-1) == R"(\)" ){	// 最後に継続文字
					flagKeepNext = true;
					strLine = strLine.substr(0, len-1);
					len = (int) strLine.length();
				}
			}
			//--- 継続時は無条件に追加 ---
			if ( flagKeepCur ){
				strCmd += strLine;
			}else{
				//--- 取り込み済みの時は終了 ---
				if ( flagTrim ){
					cont = false;
				}else{
					strCmd = strLine;
				}
			}
		}
		//--- Trim文字列確認 ---
		if ( cont ){
			if ( flagTrim == false ){
				flagTrim = readDataStrTrimDetect(strCmd);
			}
		}
	}
	return flagTrim;
}
//---------------------------------------------------------------------
// グローバル領域作成のファイル読み込み
//---------------------------------------------------------------------
bool JlsScrFuncReg::readGlobalOpen(JlsCmdArg& cmdarg, const string& fname){
	bool success = pGlobalState->readGOpen(fname);
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		string strVal = ( success )? "1" : "0";
		return setRegOutSingle(cmdarg, strVal, true);	// RegOutに無条件で書き込み
	}
	return success;
}
void JlsScrFuncReg::readGlobalClose(JlsCmdArg& cmdarg){
	pGlobalState->readGClose();
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		string strVal = "1";
		setRegOutSingle(cmdarg, strVal, true);	// RegOutに無条件で書き込み
	}
}
bool JlsScrFuncReg::readGlobalLine(JlsCmdArg& cmdarg){
	string strLine;
	bool success = pGlobalState->readGLine(strLine);
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		setRegOutSingle(cmdarg, strLine, true);	// RegOutに無条件で書き込み
	}
	return success;
}
//---------------------------------------------------------------------
// -RegArgで指定された文字列を取得
//---------------------------------------------------------------------
bool JlsScrFuncReg::getRegArg(JlsCmdArg& cmdarg, string& strArg){
	//--- 分解する文字列を取得 ---
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		if ( !getJlsRegVarNormal(strArg, strName) ){
			pGlobalState->addMsgErrorN("error: not found data by -RegArg " + strName);
			return false;
		}
	}else{
		pGlobalState->addMsgErrorN("error: need -RegArg option");
		return false;
	}
	return true;
}
//---------------------------------------------------------------------
// RegOutに単体データ書き込み
//---------------------------------------------------------------------
bool JlsScrFuncReg::setRegOutSingle(JlsCmdArg& cmdarg, const string& strVal, bool certain){
	//--- オプション取得 ---
	string strName   =  cmdarg.getStrOpt(OptType::StrRegOut);	// 変数名
	bool   overwrite = !cmdarg.getOptFlag(OptType::FlagDefault);
	bool   flagLocal =  cmdarg.getOptFlag(OptType::FlagLocal);
	bool   flagClear =  cmdarg.getOptFlag(OptType::FlagClear);
	//--- 変数設定 ---
	if ( certain || flagClear ){
		bool flagW = setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// 変数設定
		if ( !flagW ) return false;	// 書き込み失敗
	}
	return certain;
}

//---------------------------------------------------------------------
// 環境変数からデータ取得
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataEnvGet(JlsCmdArg& cmdarg, const string& strEnvName){
	//--- 値を取得して変数に設定 ---
	string strVal;
	bool success = LSys.getEnvString(strVal, strEnvName);
	//--- 変数設定 ---
	success = setRegOutSingle(cmdarg, strVal, success);
	return success;
}
//---------------------------------------------------------------------
// エラー直接出力
//---------------------------------------------------------------------
void JlsScrFuncReg::outputMesErr(const string& mes){
	lcerr << mes;
}
