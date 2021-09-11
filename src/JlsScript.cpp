//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScript.hpp"
#include "JlsScriptState.hpp"
#include "JlsScriptDecode.hpp"
#include "JlsScriptLimit.hpp"
#include "JlsReformData.hpp"
#include "JlsAutoScript.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"


///////////////////////////////////////////////////////////////////////
//
// JLスクリプト実行クラス
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------
JlsScript::JlsScript(JlsDataset *pdata){
	this->pdata  = pdata;

	// Decoder/Limiter設定
	m_funcDecode.reset(new JlsScriptDecode(pdata));
	m_funcLimit.reset(new JlsScriptLimit(pdata));
	// Auto系コマンドを拡張使用
	m_funcAutoScript.reset(new JlsAutoScript(pdata));

	// 念のため内部設定異常の確認
	checkInitial();
}

JlsScript::~JlsScript() = default;

//---------------------------------------------------------------------
// 内部設定の異常確認
//---------------------------------------------------------------------
void JlsScript::checkInitial(){
	m_funcDecode->checkInitial();
}

//---------------------------------------------------------------------
// システムから環境変数取得
//---------------------------------------------------------------------
bool JlsScript::getEnvString(string& strVal, const string& strEnvName){
	//--- 環境変数から取得 ---
	// getenvは unsafe warning が出るので getenv_s
	// VS2005以降対応、標準では C11 対応でC++は個別対応みたい
#ifdef OLD_UNSAFE
	const char *pstr = getenv(strEnvName.c_str());
	if ( pstr != nullptr ){
		strVal = pstr;
		return true;
	}
#else
	char buffer[1024];
	size_t retSize;
	getenv_s(&retSize, buffer, sizeof(buffer)-1, strEnvName.c_str());
	if ( retSize > 0 ){
		strVal = buffer;
		return true;
	}
#endif
	strVal = "";
	return false;
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
bool JlsScript::setArgReg(const string& strName, const string& strVal){
	if ( strName.empty() ) return false;
	//--- リスト変数の要素で引数設定は非対応 ---
	auto locSt = strName.find("[");
	if ( locSt != string::npos ){
		globalState.addMsgError("error: argument array " + strName + "\n");
		return false;
	}
	bool success = globalState.setArgReg(strName, strVal);
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
//--- 通常の変数を設定 ---
bool JlsScript::setJlsRegVar(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = false;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- ローカル変数を設定（引数は通常変数と同一） ---
bool JlsScript::setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = true;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- 通常の変数とローカル変数を選択して設定 ---
bool JlsScript::setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	if ( strName.empty() ) return false;
	//--- リスト変数対応 ---
	string strNameWrite = strName;
	string strValWrite  = strVal;
	bool   flagOvwWrite = overwrite;
	bool success = checkJlsRegVarWrite(strNameWrite, strValWrite, flagOvwWrite);
	//--- 書き込み処理 ---
	if ( success ){
		success = globalState.setRegVarCommon(strNameWrite, strValWrite, flagOvwWrite, flagLocal);
		setJlsRegVarCouple(strNameWrite, strVal);
	}
	return success;
}
//--- 変数設定後のシステム変数更新 ---
void JlsScript::setJlsRegVarCouple(const string& strName, const string& strVal){
	//--- システム変数の特殊処理 ---
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
			globalState.setRegVarCommon(strAddName, strAddVal, overwrite, flagLocal);
		}
		//--- head/tail情報を更新 ---
		{
			string strSub;
			if (getJlsRegVar(strSub, "HEADTIME", true) > 0){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.st, strSub, 0);
			}
			if (getJlsRegVar(strSub, "TAILTIME", true) > 0){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.ed, strSub, 0);
			}
		}
	}
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
int JlsScript::getJlsRegVar(string& strVal, const string& strCandName, bool exact){
	//--- リスト変数のチェック ---
	string strNamePart = strCandName;
	bool flagNum = false;
	if ( strCandName[0] == '#' ){		// リストの要素数
		flagNum = true;
		strNamePart = strCandName.substr(1);
	}
	int numItem;
	int lenFullVar;
	bool flagList = checkJlsRegVarList(strNamePart, numItem, lenFullVar);

	//--- 通常のレジスタ読み出し ---
	int lenVar = globalState.getRegVarCommon(strVal, strNamePart, exact);

	//--- リスト変数時の補正 ---
	if ( flagList ){
		if ( (int)strNamePart.length() == lenVar ){	// 要素数前までの文字列と読み込み変数が一致する場合
			string strList = strVal;
			if ( getListStrElement(strVal, strList, numItem) == false ){
				globalState.addMsgError("error: out of range " + strList + "[" + to_string(numItem) + "]\n");
			}
			lenVar = lenFullVar;		// [項目番号]込みの変数文字列長にする
		}
	}
	if ( flagNum ){		// リストの要素数取得
		int numList = getListStrSize(strVal);
		strVal = to_string(numList);
	}
	return lenVar;
}
//---------------------------------------------------------------------
// リスト変数の要素かチェック。要素だった時は変数名と配列で指定された数値に分離
// 入力：
//   strNamePart : 変数名（[番号]込み）
// 出力：
//   返り値  : true=リスト変数の要素  false=通常の変数
//   strNamePart : 変数名（[番号]は除く）
//   numItem     : 番号
//   lenFullVar  : [番号]込みの変数文字列長
//---------------------------------------------------------------------
bool JlsScript::checkJlsRegVarList(string& strNamePart, int& numItem, int& lenFullVar){
	//--- 変数名[項目番号] ならリスト変数の要素 ---
	bool flagList = false;
	string strItem;
	{
		auto locSt = strNamePart.find("[");
		if ( locSt != string::npos && locSt != 0 ){
			auto locEd = strNamePart.find("]", locSt+1);
			if ( locEd != string::npos && locEd != locSt+1 ){
				auto locCt = strNamePart.find("$");
				if ( locCt != string::npos && locCt < locSt ){	// [の前に$があれば今回の変数ではない
				}else{
					int dup = 0;
					locEd = locSt + 1;		// ネストを調べるため再度位置確認
					while( (strNamePart[locEd] != ']' || dup > 0) && strNamePart[locEd] != '\0' ){
						if ( strNamePart[locEd] == ']' ){
							dup --;
						}else if ( strNamePart[locEd] == '[' ){
							dup ++;
						}
						locEd ++;
					}
					if ( strNamePart[locEd] == ']' ){	// []内の文字列を取得し、その中の変数置換を実施
						string strAryBuf = strNamePart.substr(locSt + 1, locEd - locSt - 1);
						replaceBufVar(strItem, strAryBuf);
						strNamePart = strNamePart.substr(0, locSt);
						lenFullVar = (int) locEd + 1;
						flagList    = true;
					}
				}
			}
		}
	}
	//--- リスト変数の要素だった時の要素位置取得 ---
	if ( flagList ){
		//--- 項目番号の取得 ---
		if ( pdata->cnv.getStrValNum(numItem, strItem, 0) < 0 ){
			globalState.addMsgError("error: [value] must be integer in " + strNamePart + "[" + strItem + "]\n");
			flagList = false;
		}
	}
	return flagList;
}
//---------------------------------------------------------------------
// 書き込みデータを（リスト要素だった場合は）リスト全体の書き込みデータに補正
// 入力：
//   strNameWrite : 変数名（[番号]込み）
//   strValWrite  : 書き込みデータ
//   overwrite    : 0=未定義時のみ設定  1=上書き許可設定
// 出力：
//   返り値  : true=書き込みできる  false=失敗
//   strNameWrite : 変数名（[番号]は除く）
//   strValWrite  : 書き込みデータ（リスト全体）
//   overwrite    : 基本入力から変更なし。リストの新規要素時のみ1が強制設定
//---------------------------------------------------------------------
bool JlsScript::checkJlsRegVarWrite(string& strNameWrite, string& strValWrite, bool& overwrite){
	bool success = true;
	//--- リスト変数のチェック ---
	int numItem;
	int lenFullVar;
	bool flagList = checkJlsRegVarList(strNameWrite, numItem, lenFullVar);

	//--- リスト変数の要素だった時の処理 ---
	if ( flagList ){
		success = false;
		string strList = "";
		if ( getJlsRegVar(strList, strNameWrite, true) > 0 ){	// 変数読み出し（リスト全体の値）
			int numList = getListStrSize(strList);
			if ( (numItem > 0 && numItem <= numList) || (numItem < 0 && numList + numItem >= 0) ){
				success = setListStrRep(strList, strValWrite, numItem);		// データを置換
			}else if ( numList + 1 == numItem ){
				success = setListStrIns(strList, strValWrite, numItem);		// データを追加
				overwrite = true;		// 上書き禁止でも設定要素は新規なので書き込み許可
			}
		}else{
			if ( numItem == 1 ){
				success = setListStrIns(strList, strValWrite, numItem);		// データを追加
				success = true;
			}
		}
		if ( success ){
			strValWrite = strList;		// 書き込む変数データ（リスト全体）
		}else{
			globalState.addMsgError("error: out of range " + strNameWrite + "[" + to_string(numItem) + "]\n");
		}
	}
	return success;
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
int JlsScript::setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite){
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
				fprintf(stderr, "-flags needs an argument\n");
				return -1;
			}
			else{
				if (setInputFlags(str1, overwrite) == false){
					fprintf(stderr, "-flags bad argument\n");
					return -1;
				}
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-set")){
			if (!exist3){
				fprintf(stderr, "-set needs two arguments\n");
				return -1;
			}
			else{
				if (setInputReg(str1, str2, overwrite) == false){
					fprintf(stderr, "-set bad argument\n");
					return -1;
				}
			}
			numarg = 3;
		}
		else if (!_stricmp(strv, "-cutmrgin")){
			if (!exist2){
				fprintf(stderr, "-cutmrgin needs an argument\n");
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
				fprintf(stderr, "-cutmrgout needs an argument\n");
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
				fprintf(stderr, "-cutmrgwi needs an argument\n");
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
				fprintf(stderr, "-cutmrgwo needs an argument\n");
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
				fprintf(stderr, "-sublist needs an argument\n");
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
				fprintf(stderr, "-subpath needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubPath == 0){
				pdata->extOpt.subPath = str1;
				pdata->extOpt.fixSubPath = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-vline")){	// debug
			if (!exist2){
				fprintf(stderr, "-vline needs an argument\n");
				return -1;
			}
			if (overwrite || pdata->extOpt.fixVLine == 0){
				int val = atoi(str1);
				pdata->extOpt.vLine = val;
				pdata->extOpt.fixVLine = 1;
			}
			numarg = 2;
		}
	}
	return numarg;
}

//---------------------------------------------------------------------
// CutMrgIn / CutMrgOut オプション処理用 30fpsフレーム数入力でミリ秒を返す
//---------------------------------------------------------------------
Msec JlsScript::setOptionsCnvCutMrg(const char* str){
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
bool JlsScript::setInputReg(const char *name, const char *val, bool overwrite){
	return setJlsRegVar(name, val, overwrite);
}

//---------------------------------------------------------------------
// オプションフラグを設定
// 出力：
//   返り値  ：true=正常終了  false=失敗
//---------------------------------------------------------------------
bool JlsScript::setInputFlags(const char *flags, bool overwrite){
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
// コマンド実行開始時設定
//=====================================================================

//---------------------------------------------------------------------
// 事前設定のサブフォルダも検索に含めファイルが存在するフルパスを取得
// 入力：
//   strSrc   : 入力ファイル名
//   flagFull : 入力ファイル名がフルパスか（0=名前のみ、1=フルパス記載）
// 出力：
//   strFull : ファイル存在確認後のフルパス
//   返り値  ：true=ファイル存在  false=ファイル検索失敗
//---------------------------------------------------------------------
bool JlsScript::makeFullPath(string& strFull, const string& strSrc, bool flagFull){
	string strName;				// ファイル名部分
	string strPathOnly = "";	// パス部分
	//--- 基本ファイル名設定 ---
	if ( flagFull ){			// フルパス入力時はパス情報を取得する
		pdata->cnv.getStrFilePath(strPathOnly, strSrc);	// パス検索保管
		globalState.setPathNameJL(strPathOnly);
		strFull= strSrc;
		strName = strSrc.substr( strPathOnly.length() );
	}
	else{					// Callでファイル名部分のみの場合はパス情報を追加する
		strPathOnly = globalState.getPathNameJL();
		strFull = strPathOnly + strSrc;
		strName = strSrc;
	}
	//--- 指定場所でファイル有無確認 ---
	bool decide = makeFullPathIsExist(strFull.c_str());
	//--- 見つからなかったらサブフォルダ検索 ---
	if ( decide == false ){
		//--- サブフォルダのパス指定あれば変更 ---
		if ( pdata->extOpt.subPath.empty() == false ){
			strPathOnly = pdata->extOpt.subPath;
			pdata->cnv.getStrFileAllPath(strPathOnly);	// 最後に区切り付加
		}
		//--- -sublist設定からコンマ区切りで1つずつ確認 ---
		string remain = pdata->extOpt.subList;		// 事前設定のサブフォルダ取得
		remain = remain + ",";						// サブでない場所も最後に追加
		while( remain.empty() == false && decide == false ){
			string subname;
			auto pos = remain.find(",");
			if ( pos != string::npos ){
				subname = remain.substr(0, pos);
				if ( remain.length() > pos ){
					remain = remain.substr(pos+1);
				}else{
					remain = "";
				}
			}
			else{
				subname = remain;
				remain = "";
			}
			pdata->cnv.getStrFileAllPath(subname);	// 最後に区切り付加
			string strTry = strPathOnly + subname + strName;
			decide = makeFullPathIsExist(strTry);
			if ( decide ){			// サブフォルダのパス存在で結果更新
				strFull = strTry;
			}
		}
	}
	return decide;
}
bool JlsScript::makeFullPathIsExist(const string& str){
	ifstream ifs(str.c_str());
	return ifs.is_open();
}


//=====================================================================
// コマンド実行開始処理
//=====================================================================

//---------------------------------------------------------------------
// コマンド開始
// 出力：
//   返り値  ：0=正常終了 2=ファイル異常
//---------------------------------------------------------------------
int JlsScript::startCmd(const string& fname){
	//--- Call命令用のPath設定、読み込みファイル名設定 ---
	string nameFullPath;
	bool flagFull = true;		// 入力はフルパスファイル名
	makeFullPath(nameFullPath, fname, flagFull);

	//--- システム変数の初期値を設定 ---
	setSystemRegInit();

	//--- ロゴリセット時のバックアップデータ保存 ---
	pdata->backupLogosetSave();

	//--- JLスクリプト実行 ---
	int errnum = startCmdLoop(nameFullPath, 0);

	//--- デバッグ用の表示 ---
	if (pdata->extOpt.verbose > 0 && errnum == 0){
		pdata->displayLogo();
		pdata->displayScp();
	}

	return errnum;
}


//---------------------------------------------------------------------
// コマンド読み込み・実行開始
// 入力：
//   fname   : スクリプトファイル名
//   loop    : 0=初回実行 1-:Callコマンドで呼ばれた場合のネスト数
// 出力：
//   返り値  ：0=正常終了 2=ファイル異常
//---------------------------------------------------------------------
int JlsScript::startCmdLoop(const string& fname, int loop){
	//--- 前回コマンドの実行状態をfalseに設定 ---
	setSystemRegLastexe(false);

	//--- 初回実行 ---
	if (loop == 0){
		globalState.setExe1st(true);
	}
	//--- 制御信号 ---
	JlsScriptState state(&globalState);
	//--- ローカル変数階層作成 ---
	int numLayerStart = globalState.setLocalRegCreateBase();	// 最上位階層扱いで作成
	//--- ファイル読み込み ---
	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		cerr << "error: failed to open " << fname << "\n";
		return 2;
	}
	//--- 各行の実行 ---
	bool forceExit = false;
	string strBufOrg;
	while( startCmdGetLine(ifs, strBufOrg, state) ){
		if ( state.isCmdReturnExit() ){
			forceExit = true;	// Exit/Returnによる終了
			break;
		}
		startCmdLoopSub(state, strBufOrg, loop);
	}
	//--- ローカル変数階層終了 ---
	int numLayerEnd = globalState.setLocalRegReleaseBase();
	//--- ネストエラー確認 ---
	if ( forceExit == false ){
		if ( numLayerStart != numLayerEnd ){
			cerr << "error : { and } are not matched\n";
		}
		int flags_remain = state.isRemainNest();
		if (flags_remain & 0x01){
			fprintf(stderr, "error : EndIf is not found\n");
		}
		if (flags_remain & 0x02){
			fprintf(stderr, "error : EndRepeat is not found\n");
		}
		if ( state.isLazyArea() ){
			cerr << "error : EndLazy is not found\n";
		}
		if ( state.isMemArea() ){
			cerr << "error : EndMemory is not found\n";
		}
	}
	//--- 変数階層の解放不足は元に戻す ---
	if ( numLayerStart < numLayerEnd ){
		for(int i=0; i < (numLayerEnd - numLayerStart); i++){
			globalState.setLocalRegReleaseBase();
		}
	}
	//--- 一番最後に-lazy_e設定されたコマンドを実行 ---
	if (loop == 0){
		startCmdLoopLazyEnd(state);
		startCmdLoopLazyOut(state);		// 出力用も最後に実行
	}
	return 0;
}
//---------------------------------------------------------------------
// 最後に-lazy_e設定されたコマンドを実行
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyEnd(JlsScriptState& state){
	//--- lazy_eによるコマンドを取り出して実行キャッシュに設定 ---
//	state.setLazyExe(LazyType::LazyE, "");
	state.setLazyExe(LazyType::FULL, "");	// LAZY_EだけではなくLAZY_E以前も実行

	//--- lazy実行キャッシュから読み出し実行 ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// 出力用にmemory識別子OUTPUTのコマンドを実行
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyOut(JlsScriptState& state){
	//--- lazy_eによるコマンドを取り出して実行キャッシュに設定 ---
	state.setMemCall("OUTPUT");

	//--- lazy実行キャッシュから読み出し実行 ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// コマンド１行分の実行開始
// 入出力：
//   state        : 制御状態
// 出力：
//   返り値    : 現在行の実行有無（0=実行なし  1=実行あり）
//---------------------------------------------------------------------
void JlsScript::startCmdLoopSub(JlsScriptState& state, const string& strBufOrg, int loop){
	//--- 前コマンドの実行有無を代入 ---
	bool exe_command = isSystemRegLastexe();
	//--- 変数を置換 ---
	string strBuf;
	replaceBufVar(strBuf, strBufOrg);

	//--- デコード処理（コマンド部分） ---
	JlsCmdSet cmdset;									// コマンド格納
	bool onlyCmd = true;
	CmdErrType errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// コマンドのみ解析

	//--- デコード処理（全体） ---
	if ( errval == CmdErrType::None ){
		if ( state.isNeedRaw(cmdset.arg.category) ){	// LazyStartとMemory区間中は変数を展開しない
			strBuf = strBufOrg;
		}
		if ( state.isNeedFullDecode(cmdset.arg.cmdsel, cmdset.arg.category) ){
			onlyCmd = false;
			errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// コマンド全体解析
		}else{
			globalState.clearRegError();			// コマンドのみのケースは展開中のエラー消去
		}
	}
	//--- 遅延処理、コマンド解析後の変数展開 ---
	bool enable_exe = false;
	if ( errval == CmdErrType::None ){
		//--- 遅延実行処理（現在実行しない場合はfalseを返す） ---
		enable_exe = setStateMem(state, cmdset.arg, strBuf);

		//--- 変数展開（IF文判定式処理、コマンド内使用変数取得） ---
		if ( enable_exe ){
			bool success = expandDecodeCmd(state, cmdset.arg, strBuf);
			//--- エラー時は実行しない ---
			if ( success == false ){
				enable_exe = false;
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- コマンド実行処理 ---
	bool update_exe = false;
	if (enable_exe){
		bool success = true;
		switch( cmdset.arg.category ){
			case CmdCat::NONE:						// コマンドなし
				break;
			case CmdCat::COND:						// 条件分岐
				success = setCmdCondIf(cmdset.arg, state);
				break;
			case CmdCat::CALL:						// Call文
				success = setCmdCall(cmdset.arg, loop);
				break;
			case CmdCat::REP:						// 繰り返し文
				success = setCmdRepeat(cmdset.arg, state);
				break;
			case CmdCat::FLOW:						// 実行制御
				success = setCmdFlow(cmdset.arg, state);
				break;
			case CmdCat::SYS:						// システムコマンド
				success = setCmdSys(cmdset.arg);
				break;
			case CmdCat::REG:						// 変数設定
				success = setCmdReg(cmdset.arg, state);
				break;
			case CmdCat::LAZYF:						// Lazy制御
			case CmdCat::MEMF:						// Memory制御
			case CmdCat::MEMLAZYF:					// Memory/Lazy共通制御
				success = setCmdMemFlow(cmdset.arg, state);
				break;
			case CmdCat::MEMEXE:					// 遅延設定実行動作
				success = setCmdMemExe(cmdset.arg, state);
				break;
			default:								// 一般コマンド
				if ( globalState.isExe1st() ){		// 初回のみのチェック
					globalState.setExe1st(false);
					if ( pdata->isSetupAdjInitial() ){
						pdata->setFlagSetupAdj( true );
						//--- 読み込みデータ微調整 ---
						JlsReformData func_reform(pdata);
						func_reform.adjustData();
						setSystemRegNologo(true);
					}
				}
				exe_command = exeCmd(cmdset);
				update_exe  = true;
				break;
		}
		if ( success == false ){
			errval = CmdErrType::ErrCmd;
		}
	}
	//--- エラーチェック ---
	if ( errval != CmdErrType::None ){
		exe_command = false;
	}
	startCmdDispErr(strBuf, errval);
	//--- debug ---
	if ( pdata->extOpt.vLine == 2 ){
		cout << enable_exe << update_exe << exe_command << ":" << strBuf << endl;
	}else if ( pdata->extOpt.vLine > 0 ){
		cout << enable_exe << exe_command << ":" << strBuf << endl;
	}

	//--- 前コマンドの実行有無を代入 ---
	setSystemRegLastexe(exe_command);
}

//---------------------------------------------------------------------
// 次の文字列取得
// 入出力：
//   ifs          : ファイル情報
//   state        : 制御状態
//   fromFile     : ファイルからのread許可
// 出力：
//   返り値    : 文字列取得結果（0=取得なし  1=取得あり）
//   strBufOrg : 取得文字列
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLine(ifstream& ifs, string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- cacheからの読み込み ---
	flagRead = startCmdGetLineOnlyCache(strBufOrg, state);

	if ( flagRead == false ){
		//--- ファイルからの読み込み ---
		if ( startCmdGetLineFromFile(ifs, strBufOrg) ){
			flagRead = true;
			//--- Repeat用キャッシュに保存 ---
			state.addCmdCache(strBufOrg);
		}
	}
	return flagRead;
}
//---------------------------------------------------------------------
// ファイルreadなしのキャッシュデータからのみ
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineOnlyCache(string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- 遅延実行からの読み込み ---
	if ( state.readLazyMemNext(strBufOrg) ){
		flagRead = true;
	}
	//--- Repeat用キャッシュ読み込み ---
	else if ( state.readCmdCache(strBufOrg) ){
		flagRead = true;
	}
	return flagRead;
}
//---------------------------------------------------------------------
// ファイルから１行読み込み（ \による行継続に対応）
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineFromFile(ifstream& ifs, string& strBufOrg){
	strBufOrg = "";
	bool success = false;
	bool cont = true;
	while( cont ){
		cont = false;
		string buf;
		if ( getline(ifs, buf) ){
			auto len = buf.length();
			if ( len >= INT_MAX/4 ){		// 面倒事は最初にカット
				return false;
			}
			//--- 改行継続確認 ---
			if ( len >= 2 ){
				auto pos = buf.find(R"( \ )");
				if ( pos != string::npos ){
					buf = buf.substr(0, pos+1);		// 空白は残す
					cont = true;
				}
				else if ( buf.substr(len-2) == R"( \)" ){
					buf = buf.substr(0, len-1);		// 空白は残す
					cont = true;
				}
			}
			//--- 設定 ---
			strBufOrg += buf;
			success = true;
		}
	};
	return success;
}

//---------------------------------------------------------------------
// エラー表示
//---------------------------------------------------------------------
void JlsScript::startCmdDispErr(const string& strBuf, CmdErrType errval){
	if ( errval != CmdErrType::None ){
		string strErr = "";
		switch(errval){
			case CmdErrType::ErrOpt:
				strErr = "error: wrong argument";
				break;
			case CmdErrType::ErrRange:
				strErr = "error: wrong range argument";
				break;
			case CmdErrType::ErrSEB:
				strErr = "error: need Start or End";
				break;
			case CmdErrType::ErrVar:
				strErr = "error: failed variable setting";
				break;
			case CmdErrType::ErrTR:
				strErr = "error: need auto command TR/SP/EC";
				break;
			case CmdErrType::ErrCmd:
				strErr = "error: wrong command";
				break;
			default:
				break;
		}
		if ( strErr.empty() == false ){
			cerr << strErr << " in " << strBuf << "\n";
		}
	}
	globalState.checkErrorGlobalState(true);
}

//---------------------------------------------------------------------
// 変数部分を置換した文字列出力
// 入力：
//   strBuf : 文字列
// 出力：
//   返り値  ：置換結果（true=成功  false=失敗）
//   dstBuf  : 出力文字列
//---------------------------------------------------------------------
bool JlsScript::replaceBufVar(string& dstBuf, const string& srcBuf){
	string strName, strVal;
	int len_var, pos_var;

	dstBuf.clear();
	bool success = true;
	int pos_cmt = pdata->cnv.getStrPosComment(srcBuf, 0);
	int pos_base = 0;
	while(pos_base >= 0){
		//--- 変数部分の置換 ---
		pos_var = (int) srcBuf.find("$", pos_base);
		if (pos_var >= 0){
			//--- コメント領域確認 ---
			bool flagCmt = ( pos_var >= pos_cmt && pos_cmt >= 0 )? true : false;
			if ( flagCmt ){
				globalState.stopAddMsgError(true);	// エラー出力停止
			}
			//--- $手前までの文字列を確定 ---
			if (pos_var > pos_base){
				dstBuf += srcBuf.substr(pos_base, pos_var-pos_base);
				pos_base = pos_var;
			}
			//--- 変数を検索して置換 ---
			len_var = replaceRegVarInBuf(strVal, srcBuf, pos_var);
			if (len_var > 0){
				dstBuf += strVal;
				pos_base += len_var;
			}
			else{
				if ( flagCmt == false ){		// コメントでなければ置換失敗
					success = false;
					globalState.addMsgError("error: not defined variable in " + srcBuf + "\n");
				}
				pos_var = -1;
			}
			if ( flagCmt ){
				globalState.stopAddMsgError(true);	// エラー出力再開
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
int JlsScript::replaceRegVarInBuf(string& strVal, const string& strBuf, int pos){
	int var_st, var_ed;
	bool exact;
	bool flagNum = false;

	int ret = 0;
	if (strBuf[pos] == '$'){
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
		if (var_st <= var_ed){
			string strCandName = strBuf.substr(var_st, var_ed-var_st+1);
			if ( flagNum ){			// リスト要素数
				strCandName = "#" + strCandName;
			}
			int nmatch = getJlsRegVar(strVal, strCandName, exact);
			if (nmatch > 0){
				ret = nmatch + 1;	// 変数名数 + $
				if ( flagNum ){
					ret += 1;		// #
				}
				if ( exact ){
					ret += 2;		// {}
				}
			}
		}
	}
	return ret;
}



//=====================================================================
// 遅延実行の設定
//=====================================================================

//---------------------------------------------------------------------
// Memoryによる記憶処理 + コマンドオプション内容からLazy処理の設定
// 入力：
//   cmdset   : コマンドオプション内容
//   strBuf   : 現在行の文字列
// 入出力：
//   state     : mem/lazy処理追加
// 出力：
//   返り値   ：現在行のコマンド実行有無（キャッシュに移した時は実行しない）
//---------------------------------------------------------------------
bool JlsScript::setStateMem(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool enable_exe = true;
	//--- If文等によるskip（非実行期間）中は遅延関連処理を何もしない ---
	if ( state.isSkipCmd() ){
	}
	//--- Memory期間中ならMemory期間制御以外の文字列は記憶領域に格納 ---
	else if ( state.isMemArea() ){
		if ( state.isFlowMem(cmdarg.category) == false ){	// Memory制御は除く
			enable_exe = state.setMemStore(state.getMemName(), strBuf);
		}
	}
	//--- Memory期間中以外はLazy処理確認 ---
	else{
		if ( state.isFlowLazy(cmdarg.category) == false ){	// Lazy制御は除く
			enable_exe = setStateMemLazy(state, cmdarg, strBuf);
		}
	}
	//--- 引数変数挿入を行う処理 ---
	if ( state.isMemArea() || state.isLazyArea() ){
		if ( state.checkArgRegInsert(cmdarg.cmdsel) ){	// 引数変数挿入チェック
			setArgStateInsertReg(state);				// 必要あれば引数変数設定を挿入
		}
	}

	//--- 制御状態からのコマンド実行有効性 ---
	if ( state.isInvalidCmdLine(cmdarg.category) ){
		enable_exe = false;
	}
	return enable_exe;
}
//---------------------------------------------------------------------
// コマンドオプション内容からLazy処理の設定
// 引数は setStateMem() 参照
//---------------------------------------------------------------------
bool JlsScript::setStateMemLazy(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool enable_exe = true;
	bool flagNormalCmd = false;		// 通常コマンドだった場合は後でtrueが設定される

	//--- lazy実行中でなければlazyチェック実施 ---
	if ( state.isLazyExe() == false ){
		//--- Lazy種類取得 ---
		LazyType typeLazy = state.getLazyStartType();	// LazyStartによる区間Lazyタイプ取得
		//--- Lazy種類補正 ---
		if ( state.isLazyArea() == false ){		// LazyStartによる区間内の補正は実行時行うため除く
			setStateMemLazyRevise(typeLazy, state, cmdarg);	// 現コマンドの必要な状態
		}
		//--- lazy設定 ---
		switch( typeLazy ){
			case LazyType::LazyS:				// -lazy_s
			case LazyType::LazyA:				// -lazy_a
			case LazyType::LazyE:				// -lazy_e
				//--- lazy種類に対応する場所へlazyコマンドを保管する ---
				enable_exe = state.setLazyStore(typeLazy, strBuf);
				break;
			default:						// lazy指定なし
				flagNormalCmd = true;		// 通常のコマンド
				break;
		}
	}
	//--- lazy処理中の確認 ---
	else{
		LazyType lazyPassed = state.getLazyExeType();	// 読込時のLazy状態
		LazyType lazyNeed = LazyType::None;
		bool needA = setStateMemLazyRevise(lazyNeed, state, cmdarg);	// 読込コマンドの必要な状態

		//--- Auto必要処理でAuto未実行の場合、既にLAZY_Eなら実行しない ---
		if ( isLazyAutoModeInitial(state) && needA && lazyPassed == LazyType::LazyE ){
			enable_exe = false;
		}else{
			//--- 現時点ではまだ実行できない状態確認 ---
			LazyType lazyNext = LazyType::None;
			switch( lazyNeed ){
				case LazyType::LazyE :
					if ( lazyPassed == LazyType::LazyA ||
					     lazyPassed == LazyType::LazyS ){
						lazyNext = LazyType::LazyE;
					}
					break;
				case LazyType::LazyA :
					if ( lazyPassed == LazyType::LazyS ){
						lazyNext = LazyType::LazyA;
					}
					break;
				default :
					break;
			}
			if ( lazyNext != LazyType::None ){		// また次のLazy状態に入れ直し
					enable_exe = state.setLazyStore(lazyNext, strBuf);
			}
		}
	}
	//--- コマンド実行時のLazy実行開始 ---
	if ( enable_exe ){
		//--- 対象コマンドを実行した場合にはlazy保管行をlazy実行行に移す ---
		switch( cmdarg.category ){
			case CmdCat::AUTO:
			case CmdCat::AUTOEACH:
			case CmdCat::AUTOLOGO:
				if ( flagNormalCmd ){
					enable_exe = state.setLazyExe(LazyType::LazyA, strBuf);
				}
				if ( enable_exe ){
					state.setLazyStateIniAuto(false);	// Auto実行済み状態にする
				}
				break;
			case CmdCat::LOGO:
				if ( flagNormalCmd ){
					enable_exe = state.setLazyExe(LazyType::LazyS, strBuf);
				}
				break;
			default:
				break;
		}
	}
	return enable_exe;
}
//--- Lazy動作用のAuto未実行状態確認 ---
bool JlsScript::isLazyAutoModeInitial(JlsScriptState& state){
	return pdata->isAutoModeInitial() || state.isLazyStateIniAuto();
}
//---------------------------------------------------------------------
// コマンドによるLazy種類の補正
//---------------------------------------------------------------------
bool JlsScript::setStateMemLazyRevise(LazyType& typeLazy, JlsScriptState& state, JlsCmdArg& cmdarg){
	//--- -nowオプション時は補正しない ---
	if ( cmdarg.getOpt(OptType::FlagNow) > 0 ){
		return false;
	}
	//--- オプション引数があればLazyタイプ変更 ---
	if ( cmdarg.tack.typeLazy != LazyType::None ){
		typeLazy = cmdarg.tack.typeLazy;
	}
	//--- コマンド分類別にLazyコマンド動作補正 ---
	bool flagAuto = state.isLazyAuto();		// LazyAutoコマンドによる自動付加フラグ
	bool flagInit = isLazyAutoModeInitial(state);
	bool flagInitAuto = flagInit && flagAuto;
	bool needS = false;
	bool needA = false;
	bool needE = false;
	switch( cmdarg.category ){
		case CmdCat::AUTOEACH:
		case CmdCat::AUTOLOGO:
			needA = flagInit;
			break;
		case CmdCat::AUTO:
			needA = flagInitAuto;
			break;
		case CmdCat::LOGO:
			needS = flagInitAuto;
			break;
		default :
			break;
	}
	if ( flagInitAuto ){
		if ( cmdarg.getOpt(OptType::FlagAutoChg) > 0 ){	// Auto系に設定するオプション
				needA = true;
		}
		if ( cmdarg.tack.needAuto ){	// オプションにAuto系構成が必要な場合(-AC -NoAC)
				needA = true;
		}
	}
	if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){
		if ( needA || needS ){
			needE = true;		// -final設定時は変更時の動作をlazy_eにする
		}
	}
	//--- 設定変更 ---
	if ( needE ){
		typeLazy = LazyType::LazyE;
	}
	else if ( needA && (typeLazy == LazyType::None || typeLazy == LazyType::LazyS) ){
		typeLazy = LazyType::LazyA;
	}
	else if ( needS && typeLazy == LazyType::None ){
		typeLazy = LazyType::LazyS;
	}
	return needA;		// 返り値はAutoが必要なコマンドかどうか
}



//=====================================================================
// コマンド解析後の変数展開（IF文判定式処理、コマンド内使用変数取得）
//=====================================================================

//---------------------------------------------------------------------
// 変数展開（IF条件式判定と使用変数(POSHOLD/LISTHOLD)取得）
//---------------------------------------------------------------------
bool JlsScript::expandDecodeCmd(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool success = true;

	//--- IF条件式判定 ---
	bool flagCond = true;					// 条件なければtrue
	int  num = cmdarg.getNumCheckCond();	// 判定必要な引数位置取得
	if ( num > 0 ){
		success = getCondFlag(flagCond, cmdarg.getStrArg(num));
	}
	cmdarg.setCondFlag(flagCond);			// IF条件の結果設定

	//--- コマンドで必要となる変数を取得 ---
	getDecodeReg(cmdarg);

	return success;
}
//---------------------------------------------------------------------
// 文字列対象位置以降のフラグを判定
// 入力：
//   strBuf : 文字列
// 出力：
//   返り値  : 成功
//   flagCond  ：フラグ判定（0=false  1=true）
//---------------------------------------------------------------------
bool JlsScript::getCondFlag(bool& flagCond, const string& strBuf){
	string strItem;
	string strCalc = "";
	//--- コメントカット ---
	string strBufRev;
	pdata->cnv.getStrWithoutComment(strBufRev, strBuf);
	//--- １単語ずつ確認 ---
	int pos = 0;
	while(pos >= 0){
		pos = getCondFlagGetItem(strItem, strBufRev, pos);
		if (pos >= 0){
			getCondFlagConnectWord(strCalc, strItem);
		}
	}
	int pos_calc = 0;
	int val;
	if ((int)strCalc.find(":") >= 0 || (int)strCalc.find(".") >= 0){		// 時間表記だった場合
		pos_calc = pdata->cnv.getStrValMsec(val, strCalc, 0);	// 時間単位で比較
	}
	else{
		pos_calc = pdata->cnv.getStrValNum(val, strCalc, 0);	// strCalcの先頭から取得
	}
	//--- 結果確認 ---
	bool success = true;
	if (pos_calc < 0){
		val = 0;
		success = false;
		cerr << "error: can not evaluate(" << strCalc << ")" << endl;
	}
	flagCond = (val != 0)? true : false;
	return success;
}
//--- 次の項目を取得、文字列の場合は比較結果を確認する ---
int JlsScript::getCondFlagGetItem(string& strItem, const string& strBuf, int pos){
	pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, pos);
	if ( pos >= 0 ){
		//--- quoteだった場合のみ文字列比較を行う ---
		if ( strItem[0] == '\"' ){
			string str2;
			string str3;
			int pos2 = pdata->cnv.getStrItemWithQuote(str2, strBuf, pos);
			int pos3 = pdata->cnv.getStrItemWithQuote(str3, strBuf, pos2);
			if ( pos3 >= 0 ){
				bool flagEq = ( strItem == str3 )? true : false;
				if ( str2 == "==" ){
					pos = pos3;
					strItem = ( flagEq )? "1" : "0";
				}
				else if ( str2 == "!=" ){
					pos = pos3;
					strItem = ( flagEq )? "0" : "1";
				}
			}
		}
	}
	return pos;
}

//---------------------------------------------------------------------
// フラグ用に文字列を連結
// 入出力：
//   strCalc : 連結先文字列
// 入力：
//   strItem : 追加文字列
//---------------------------------------------------------------------
void JlsScript::getCondFlagConnectWord(string& strCalc, const string& strItem){

	//--- 連結文字の追加（比較演算子が２項間になければOR(||)を追加する） ---
	char chNextFront = strItem.front();
	char chNextFr2   = strItem[1];
	if (strCalc.length() > 0 && strItem.length() > 0){
		char chPrevBack = strCalc.back();
		if (chPrevBack  != '=' && chPrevBack  != '<' && chPrevBack  != '>' &&
			chPrevBack  != '|' && chPrevBack  != '&' &&
			chNextFront != '|' && chNextFront != '&' &&
			chNextFront != '=' && chNextFront != '<' && chNextFront != '>'){
			if (chNextFront == '!' && chNextFr2 == '='){
			}
			else{
				strCalc += "||";
			}
		}
	}
	//--- 反転演算の判定 ---
	string strRemain;
	if (chNextFront == '!'){
		strCalc += "!";
		strRemain = strItem.substr(1);
	}
	else{
		strRemain = strItem;
	}
	//--- フラグ変数の判定 ---
	char chFront = strRemain.front();
	if ((chFront >= 'A' && chFront <= 'Z') || (chFront >= 'a' && chFront <= 'z')){
		string strVal;
		//--- 変数からフラグの値を取得 ---
		int nmatch = getJlsRegVar(strVal, strRemain, true);
		if (nmatch > 0 && strVal != "0"){	// 変数が存在して0以外の場合
			strVal = "1";
		}
		else{
			strVal = "0";
		}
		strCalc += strVal;				// フラグの値（0または1）を追加
	}
	else{
		strCalc += strRemain;			// 追加文字列をそのまま追加
	}
//printf("(rstr:%s)",strCalc.c_str());
}

//---------------------------------------------------------------------
// 解析で必要となる変数を取得(POSHOLD/LISTHOLD)
//---------------------------------------------------------------------
void JlsScript::getDecodeReg(JlsCmdArg& cmdarg){
	//--- 変数名取得 ---
	string strRegPos  = cmdarg.getStrOpt(OptType::StrRegPos);
	string strRegList = cmdarg.getStrOpt(OptType::StrRegList);
	//--- 初期値保持 ---
	string strDefaultPos  = cmdarg.getStrOpt(OptType::StrValPosW);
	string strDefaultList = cmdarg.getStrOpt(OptType::StrValListW);
	//--- 変数値取得設定 ---
	{
		string strVal;
		//--- POSHOLDの値を設定 ---
		if ( getJlsRegVar(strVal, strRegPos, true) > 0 ){	// 変数取得
			cmdarg.setStrOpt(OptType::StrValPosR, strVal);	// 変更
			cmdarg.setStrOpt(OptType::StrValPosW, strVal);	// 変更
			cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// レジスタ更新不要
		}
		//--- -valオプション定義時のPOSHOLD読み出し値変更 ---
		if ( cmdarg.isSetStrOpt(OptType::StrArgVal) ){
			strVal = cmdarg.getStrOpt(OptType::StrArgVal);
			cmdarg.setStrOpt(OptType::StrValPosR, strVal);	// 変更
		}

		//--- LISTHOLDの値を設定 ---
		if ( getJlsRegVar(strVal, strRegList, true) > 0 ){	// 変数取得
			cmdarg.setStrOpt(OptType::StrValListR, strVal);	// 変更
			cmdarg.setStrOpt(OptType::StrValListW, strVal);	// 変更
			cmdarg.clearStrOptUpdate(OptType::StrValListW);	// レジスタ更新不要
		}
	}
	//--- 結果格納用の初期値変更 ---
	switch( cmdarg.cmdsel ){
		case CmdType::GetPos:
		case CmdType::ListGetAt:
			if ( cmdarg.getOpt(OptType::FlagClear) > 0 ){		// -clearオプション時のみ
				cmdarg.setStrOpt(OptType::StrValPosW, strDefaultPos);	// 変更
			}
			break;
		case CmdType::GetList:
		case CmdType::ReadData:
		case CmdType::ReadTrim:
		case CmdType::ReadString:
			{
				cmdarg.setStrOpt(OptType::StrValListW, strDefaultList);	// 変更
			}
			break;
		default:
			break;
	}
}



//=====================================================================
// システム変数設定
//=====================================================================

//---------------------------------------------------------------------
// 初期設定変数
//---------------------------------------------------------------------
void JlsScript::setSystemRegInit(){
	setSystemRegHeadtail(-1, -1);
	setSystemRegUpdate();
}

//---------------------------------------------------------------------
// 初期設定変数の現在値による変更
//---------------------------------------------------------------------
void JlsScript::setSystemRegUpdate(){
	int n = pdata->getMsecTotalMax();
	string str_val = pdata->cnv.getStringFrameMsecM1(n);
	string str_time = pdata->cnv.getStringTimeMsecM1(n);
	setJlsRegVar("MAXFRAME", str_val, true);
	setJlsRegVar("MAXTIME", str_time, true);
	setJlsRegVar("NOLOGO", to_string(pdata->extOpt.flagNoLogo), true);
	setSystemRegFilePath();
}
//---------------------------------------------------------------------
// Path関連の現在値による変更
//---------------------------------------------------------------------
void JlsScript::setSystemRegFilePath(){
	string strDataLast = "data";		// リストデータ読み込み用JLサブパスからの位置

	//--- JLサブパス読み込み ---
	string strPathSub;
	if ( pdata->extOpt.subPath.empty() == false ){
		strPathSub = pdata->extOpt.subPath;
	}else{
		strPathSub = globalState.getPathNameJL();
	}
	pdata->cnv.getStrFileAllPath(strPathSub);	// 最後に区切り付加
	//--- JLデータパス ---
	string strPathData = strPathSub + strDataLast;
	pdata->cnv.getStrFileAllPath(strPathData);	// 最後に区切り付加
	//--- 初期設定変数の変更 ---
	setJlsRegVar("JLDATAPATH", strPathData, true);
}

//---------------------------------------------------------------------
// HEADFRAME/TAILFRAMEを設定
//---------------------------------------------------------------------
void JlsScript::setSystemRegHeadtail(int headframe, int tailframe){
	string str_head = pdata->cnv.getStringTimeMsecM1(headframe);
	string str_tail = pdata->cnv.getStringTimeMsecM1(tailframe);
	setJlsRegVar("HEADTIME", str_head, true);
	setJlsRegVar("TAILTIME", str_tail, true);
}

//---------------------------------------------------------------------
// 無効なロゴの確認（ロゴ期間が極端に短かったらロゴなし扱いにする）
//---------------------------------------------------------------------
void JlsScript::setSystemRegNologo(bool need_check){
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
// 前回の実行状態を設定
//---------------------------------------------------------------------
void JlsScript::setSystemRegLastexe(bool exe_command){
	//--- 前回の実行状態を変数に設定 ---
	setJlsRegVar("LASTEXE", to_string((int)exe_command), true);
}
//--- 直前の実行状態取得 ---
bool JlsScript::isSystemRegLastexe(){
	string strVal = "0";
	getJlsRegVar(strVal, "LASTEXE", true);
	bool lastExe = ( strVal != "0" )? true : false;
	return lastExe;
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
bool JlsScript::setSystemRegOptions(const string& strBuf, int pos, bool overwrite){
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


//=====================================================================
// コマンド結果による変数更新
//=====================================================================

//--- $POSHOLD/$LISTHOLDのレジスタ更新 ---
void JlsScript::updateResultRegWrite(JlsCmdArg& cmdarg){
	//--- POSHOLDの更新 ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValPosW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegPos);	// 変数名($POSHOLD)
		string strVal    = cmdarg.getStrOpt(OptType::StrValPosW);
		bool   overwrite = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
		bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
		setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// $POSHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// 更新完了通知
	}
	//--- LISTHOLDの更新 ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValListW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegList);	// 変数名($LISTHOLD)
		string strList   = cmdarg.getStrOpt(OptType::StrValListW);
		bool   overwrite = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
		bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
		setJlsRegVarWithLocal(strName, strList, overwrite, flagLocal);	// $LISTHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValListW);	// 更新完了通知
	}
}
//--- $SIZEHOLDのレジスタ設定 ---
void JlsScript::setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList){
	string strSizeName = cmdarg.getStrOpt(OptType::StrRegSize);	// 変数名($SIZEHOLD)
	int    numList     = getListStrSize(strList);		// 項目数取得
	string strNumList  = std::to_string(numList);
	bool   overwrite   = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal   = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	setJlsRegVarWithLocal(strSizeName, strNumList, overwrite, flagLocal);	// $SIZEHOLD
}
//--- $POSHOLDの更新 ---
void JlsScript::setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos){
	string strVal  = pdata->cnv.getStringTimeMsecM1(msecPos);
	cmdarg.setStrOpt(OptType::StrValPosW, strVal);			// $POSHOLD write
	updateResultRegWrite(cmdarg);
}

//--- $LISTHOLDの更新 ---
void JlsScript::setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos){
	//--- レジスタの現在値を取得 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write変数値

	//--- 項目追加 ---
	if ( msecPos != -1 ){
		if ( strList.empty() == false ){
			strList += ",";
		}
		string strVal = pdata->cnv.getStringTimeMsecM1(msecPos);
		strList += strVal;
	}
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListGetAtによる更新 ---
void JlsScript::setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosW);
	//--- 対象位置の項目を取得 ---
	{
		string strItem;
		if ( getListStrElement(strItem, strList, numItem) ){
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
void JlsScript::setResultRegListIns(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	//--- Ins処理 ---
	if ( setListStrIns(strList, strValPos, numItem) ){
		//--- ListHoldに設定 ---
		cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
		updateResultRegWrite(cmdarg);
	}
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListDelによる更新 ---
void JlsScript::setResultRegListDel(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- Del処理 ---
	if ( setListStrDel(strList, numItem) ){
		//--- ListHoldに設定 ---
		cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
		updateResultRegWrite(cmdarg);
	}
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListSetAtによる更新 ---
void JlsScript::setResultRegListRep(JlsCmdArg& cmdarg, int numItem){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	//--- Replace処理 ---
	if ( setListStrRep(strList, strValPos, numItem) ){
		//--- ListHoldに設定 ---
		cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
		updateResultRegWrite(cmdarg);
	}
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListClearによるリスト変数内容を消去 ---
void JlsScript::setResultRegListClear(JlsCmdArg& cmdarg){
	//--- リスト変数の初期値 ---
	string strList   = "";
	//--- ListHoldに設定 ---
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListSortによる更新 ---
void JlsScript::setResultRegListSort(JlsCmdArg& cmdarg){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	bool   flagUni   = ( cmdarg.getOpt(OptType::FlagUnique)>0 )? true : false;
	//--- sort処理 ---
	setListStrSort(strList, flagUni);
	//--- ListHoldに設定 ---
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- リスト数更新 ---
	setResultRegWriteSize(cmdarg, strList);
}


//=====================================================================
// リスト共通処理
//=====================================================================

//--- リストの項目数を返す ---
int JlsScript::getListStrSize(const string& strList){
	int numList = 0;
	//--- リスト項目数を取得 ---
	if ( strList.empty() == false ){
		//--- Comma格納用特殊文字列の確認 ---
		int posCmmSt;
		int posCmmEd;
		bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
		bool flagNoDetect = false;
		//--- 順番に検索 ---
		numList = 1;
		bool flagFirstChar = true;
		for(int i=0; i < (int)strList.size(); i++){
			//--- Comma格納用特殊文字列の確認 ---
			if ( flagNoDetect ){		// 特殊文字列期間中
				if ( i == posCmmEd ){
					flagNoDetect = false;
				}
			}else{
				if ( flagComma ){	// 特殊文字列あり
					if ( i == posCmmSt && flagFirstChar ){
						flagNoDetect = true;
					}else if ( i > posCmmSt ){	// 次の特殊文字列を取得
						flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, i);
					}
				}
				if ( strList[i] == ',' ){
					numList++;
					flagFirstChar = true;
				}else{
					flagFirstChar = false;
				}
			}
		}
	}
	return numList;
}

//--- Commaがリスト変数内に含む場合の特殊文字列開始から終了までの文字列位置取得 ---
bool JlsScript::getListStrCommaCheck(int& posSt, int& posEd, const string& strList, int pos){
	bool flagComma = false;
	posSt = -1;
	posEd = -1;
	auto posIn = strList.find(DefStrCommaIn, pos);
	if ( posIn != string::npos ){
		auto posOut = strList.find(DefStrCommaOut, posIn);
		if ( posOut != string::npos ){
			flagComma = true;
			posSt = (int) posIn;
			posEd = (int) (posOut + DefStrCommaOut.length() - 1);
		}
	}
	return flagComma;
}
//--- リスト変数に入れる要素文字列を取得（Comma対策を付加） ---
void JlsScript::getListStrBaseStore(string& strStore, const string& strRaw){
	//--- Commaが含まれていたら特殊文字列にする ---
	if ( strRaw.find(",") != string::npos ){
		strStore = DefStrCommaIn + strRaw + DefStrCommaOut;
	}else{
		strStore = strRaw;
	}
}
//--- リスト変数から要素文字列を取り出し（Comma対策を解除） ---
void JlsScript::getListStrBaseLoad(string& strRaw, const string& strStore){
	strRaw = strStore;
	//--- 特殊文字列確認 ---
	if ( strStore.find(DefStrCommaIn) == 0 ){
		auto pos = strStore.find(DefStrCommaOut);
		if ( pos != string::npos ){
			auto len = DefStrCommaIn.length();
			strRaw = strStore.substr(len, pos-len);
		}
	}
}
//--- 指定項目のある開始位置と文字列長を返す。項目数num>0のみ対応 ---
bool JlsScript::getListStrBasePosItem(int& posItem, int& lenItem, const string& strList, int num){
	//--- Comma格納用特殊文字列の確認 ---
	int posCmmSt;
	int posCmmEd;
	bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
	//--- 順番に位置確認 ---
	int nCur = 0;
	int pos = -1;
	int posNext = 0;
	while ( posNext >= 0 && nCur < num && num > 0 ){
		pos = posNext;
		//--- 次の特殊文字列を取得 ---
		if ( pos > posCmmSt && flagComma ){
			flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, pos);
		}
		//--- 次の項目位置 ---
		int posSend = pos;
		if ( pos == posCmmSt && flagComma ){
			posSend = posCmmEd;
		}
		auto posFind = strList.find(",", posSend);
		if ( posFind == string::npos ){
			posNext = -1;
		}else{
			posNext = (int) posFind + 1;
		}
		nCur ++;
	}
	//--- 結果位置の格納 ---
	if ( pos >= 0 && nCur == num ){
		posItem = pos;
		if ( pos <= posNext ){
			lenItem = posNext - pos - 1;
		}else{
			lenItem = (int) strList.length() - pos;
		}
		return true;
	}
	posItem = -1;
	lenItem = 0;
	return false;
}
//--- リストの指定項目位置が文字列の何番目および文字列長を取得 ---
bool JlsScript::getListStrPosItem(int& posItem, int& lenItem, const string& strList, int num, bool flagIns){
	int numList = getListStrSize(strList);	// 項目数取得
	//--- 項目を取得 ---
	int numAbs = ( num >= 0 )? num : numList + num + 1;
	if ( flagIns && num < 0 ){		// Ins時は最大項目数が１多い
		numAbs += 1;
	}
	//--- 挿入時の最後尾 ---
	if ( numAbs > 0 && (numAbs == numList + 1) && flagIns ){
		posItem = (int)strList.size();
		lenItem = 0;
		return true;
	}else if ( numAbs > numList || numAbs == 0 ){
		return false;
	}
	//--- 位置を取得 ---
	return getListStrBasePosItem(posItem, lenItem, strList, numAbs);
}
//--- リストの指定項目位置が文字列の何番目か取得 ---
int JlsScript::getListStrPosHead(const string& strList, int num, bool flagIns){
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		return posItem;
	}
	return -1;
}
//--- リストの指定項目位置にある文字列を返す ---
bool JlsScript::getListStrElement(string& strItem, const string& strList, int num){
	strItem = "";
	bool flagIns = false;
	//--- リスト内の位置取得 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		string strStore = strList.substr(posItem, lenItem);
		getListStrBaseLoad(strItem, strStore);		// リスト保管用特殊文字列から復元
		return true;
	}
	return false;
}
//--- リストの指定項目位置に文字列を挿入 ---
bool JlsScript::setListStrIns(string& strList, const string& strItem, int num){
	int lenList = (int)strList.length();
	//--- 対象項目の先頭位置取得 ---
	bool flagIns = true;
	int locSt   = getListStrPosHead(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	//--- 保管用文字列作成 ---
	string strStore;
	getListStrBaseStore(strStore, strItem);
	//--- 挿入処理 ---
	if ( locSt == 0 ){			// 先頭項目
		if ( strList.empty() ){	// 最初の項目
			strList = strStore;
		}else{
			strList = strStore + "," + strList;
		}
	}else if ( locSt == lenList ){	// 最後
		strList = strList + "," + strStore;
	}else{
		string strTmp = strList.substr(locSt-1);
		if ( locSt == 1 ){
			strList = "," + strStore + strTmp;
		}else{
			strList = strList.substr(0, locSt-1) + "," + strStore + strTmp;
		}
	}
	return true;
}
//--- リストの指定項目位置の文字列を削除 ---
bool JlsScript::setListStrDel(string& strList, int num){
	int lenList = (int)strList.length();
	//--- 対象項目の先頭位置と文字数を取得 ---
	bool flagIns = false;
	//--- リスト内の位置取得 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) == false ){
		return false;
	}
	//--- 削除処理 ---
	if ( posItem == 0 ){
		if ( lenItem + 1 >= lenList ){
			strList.clear();
		}else{
			strList = strList.substr(lenItem + 1);
		}
	}else if ( posItem == 1 ){
		strList = strList.substr(lenItem + 1);
	}else{
		string strTmp = "";
		if ( posItem + lenItem < lenList ){
			strTmp = strList.substr(posItem + lenItem);
		}
		strList = strList.substr(0, posItem-1) + strTmp;
	}
	return true;
}
//--- リストの指定項目位置の文字列を置換 ---
bool JlsScript::setListStrRep(string& strList, const string& strItem, int num){
	if ( setListStrDel(strList, num) ){
		if ( setListStrIns(strList, strItem, num) ){
			return true;
		}
	}
	return false;
}

//--- リストデータを昇順にソート ---
bool JlsScript::setListStrSort(string& strList, bool flagUni){
	//--- ソート用 ---
	struct data_t {
		Msec ms;
		string str;
		bool operator<( const data_t& right ) const {
			return ms < right.ms;
		}
		bool operator==( const data_t& right ) const {
			return ms == right.ms;
		}
	};
	//--- リスト内データ数 ---
	int numList = getListStrSize(strList);	// 項目数取得
	//--- リストデータを取得し比較用の値に変換 ---
	vector<data_t>  listSort;
	for(int i=1; i<=numList; i++){
		string strItem;
		if ( getListStrElement(strItem, strList, i) ){
			Msec msecVal;
			if ( pdata->cnv.getStrValMsec(msecVal, strItem, 0) >= 0 ){
				data_t dItem;
				dItem.ms  = msecVal;
				dItem.str = strItem;
				listSort.push_back( dItem );
			}
		}
	}
	//--- ソート ---
	std::sort(listSort.begin(), listSort.end());
	if ( flagUni ){		// 重複要素の削除
		listSort.erase(std::unique(listSort.begin(), listSort.end()), listSort.end());
	}
	//--- リスト書き換え ---
	bool success = false;
	strList.clear();
	int numSort = (int)listSort.size();
	if ( numSort > 0 ){
		success = true;
		for(int i=0; i < numSort; i++){
			setListStrIns(strList, listSort[i].str, -1);
		}
	}
	return success;
}

//=====================================================================
// データ用ファイル読み込み
//=====================================================================

//---------------------------------------------------------------------
// リストデータ（数値）をファイルから読み込み（１行１データ）
//---------------------------------------------------------------------
bool JlsScript::readDataList(JlsCmdArg& cmdarg, const string& fname){
	bool flagNum = true;	// 数値を読み込み
	return readDataListCommon(cmdarg, fname, flagNum);
}
//---------------------------------------------------------------------
// リストデータ（文字列）をファイルから読み込み（１行１データ）
//---------------------------------------------------------------------
bool JlsScript::readDataString(JlsCmdArg& cmdarg, const string& fname){
	bool flagNum = false;	// 文字列を読み込み
	return readDataListCommon(cmdarg, fname, flagNum);
}
//---------------------------------------------------------------------
// リストデータをファイルから読み込みの共通処理
//---------------------------------------------------------------------
bool JlsScript::readDataListCommon(JlsCmdArg& cmdarg, const string& fname, bool flagNum){
	//--- リストデータクリア ---
	updateResultRegWrite(cmdarg);	// 変数($LISTHOLD)クリア
	//--- ファイル読み込み ---
	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		globalState.fileOutput("warning: file not found(" + fname + ")\n");
		return false;
	}
	//--- データ読み込み ---
	bool success = false;
	bool cont = true;
	while( cont ){
		cont = false;
		string strLine;
		cont = readDataFileLine(strLine, ifs);	// １行読み込み
		if ( cont ){
			if ( flagNum ){		// 数値のみ
				string sdata;
				if ( pdata->cnv.getStrItem(sdata, strLine, 0) > 0 ){	// 1項目取得
					int val;
					if ( pdata->cnv.getStrValNum(val, sdata, 0) > 0 ){	// 項目が数値
						readDataStrAdd(cmdarg, sdata);
						success = true;
					}
				}
			}else{		// 文字列データ
				readDataStrAdd(cmdarg, strLine);
				success = true;
			}
		}
	}
	return success;
}
//---------------------------------------------------------------------
// リストデータをAVSファイルのTrimから読み込み
//---------------------------------------------------------------------
bool JlsScript::readDataTrim(JlsCmdArg& cmdarg, const string& fname){
	//--- リストデータクリア ---
	updateResultRegWrite(cmdarg);	// 変数($LISTHOLD)クリア
	//--- ファイル読み込み ---
	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		globalState.fileOutput("warning: file not found(" + fname + ")\n");
		return false;
	}
	//--- データ読み込み ---
	bool success = false;
	string strCmd;
	if ( readDataFileTrim(strCmd, ifs) ){
		bool cont = true;
		while( cont ){			// Trimの数だけ継続
			string strLocSt;
			string strLocEd;
			cont = readDataStrTrimGet(strLocSt, strLocEd, strCmd);
			if ( cont ){
				readDataStrAdd(cmdarg, strLocSt);
				readDataStrAdd(cmdarg, strLocEd);
				success = true;
			}
		}
	}
	return success;
}

//---------------------------------------------------------------------
// リストに１項目追加
//---------------------------------------------------------------------
void JlsScript::readDataStrAdd(JlsCmdArg& cmdarg, const string& sdata){
	//--- レジスタの現在値を取得 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write変数値
	cmdarg.setStrOpt(OptType::StrValListR, strList);
	cmdarg.setStrOpt(OptType::StrValPosR, sdata);
	setResultRegListIns(cmdarg, -1);
}
//---------------------------------------------------------------------
// Trim文字列の開始・終了位置を取得して文字列は次の位置に移行
//---------------------------------------------------------------------
bool JlsScript::readDataStrTrimGet(string& strLocSt, string& strLocEd, string& strTarget){
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
bool JlsScript::readDataStrTrimDetect(const string& strLine){
	string strTarget = strLine;
	string strLocSt;
	string strLocEd;
	return readDataStrTrimGet(strLocSt, strLocEd, strTarget);
}

//---------------------------------------------------------------------
// データ１行読み込み
//---------------------------------------------------------------------
bool JlsScript::readDataFileLine(string& strLine, ifstream& ifs){
	strLine = "";
	string buf;
	if ( getline(ifs, buf) ){
		auto len = buf.length();
		if ( len >= INT_MAX/4 ){		// 面倒事は最初にカット
			return false;
		}
		strLine = buf;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
// AVSファイルTrim行読み込み
//---------------------------------------------------------------------
bool JlsScript::readDataFileTrim(string& strCmd, ifstream& ifs){
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
// 環境変数からデータ取得
//---------------------------------------------------------------------
bool JlsScript::readDataEnvGet(JlsCmdArg& cmdarg, const string& strEnvName){
	//--- 値を取得して変数に設定 ---
	string strVal;
	bool success = getEnvString(strVal, strEnvName);
	//--- オプション取得 ---
	string strName   = cmdarg.getStrOpt(OptType::StrRegEnv);	// 変数名
	bool   overwrite = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	bool   flagClear = ( cmdarg.getOpt(OptType::FlagClear) > 0 )? true : false;
	//--- 変数設定 ---
	if ( success || flagClear ){
		bool flagW = setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// 変数設定
		if ( flagW == false ){
			success = false;
		}
	}
	return success;
}


//=====================================================================
// ロゴ直接設定
//=====================================================================

//---------------------------------------------------------------------
// リスト変数からロゴ位置を直接設定
//---------------------------------------------------------------------
void JlsScript::setLogoDirect(JlsCmdArg& cmdarg){
	//--- レジスタの現在値を取得 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	setLogoDirectString(strList);
}
void JlsScript::setLogoDirectString(const string& strList){
	//--- リスト項目数を取得 ---
	int numList = getListStrSize(strList);

	//--- 既にLogoOffが設定されていた場合は解除 ---
	if ( pdata->extOpt.flagNoLogo ){
		pdata->extOpt.flagNoLogo = 0;	// ロゴは読み込みに設定
		setSystemRegUpdate();
	}

	//--- ロゴデータ情報削除更新 ---
	pdata->clearDataLogoAll();		// 現在のロゴは削除
	pdata->extOpt.flagDirect = 1;	// ロゴ直接設定値
	pdata->extOpt.fixFDirect = 1;	// ロゴ直接設定済み
	pdata->extOpt.nLgExact = 3;		// ロゴ位置は正確として扱う

	//--- ロゴデータ設定 ---
	Msec msecLast = 0;
	bool flagFall = false;
	for(int i=0; i<numList; i++){
		Msec msecCur;
		bool flagValid = false;
		{
			string strCur;
			if ( getListStrElement(strCur, strList, i+1) ){
				if ( pdata->cnv.getStrValMsec(msecCur, strCur, 0) >= 0 ){
					flagValid = true;
				}
			}
		}
		if ( flagValid && (msecLast <= msecCur) ){
			if ( (flagFall == false) && (i%2 == 0) ){
				msecLast = msecCur;
				flagFall = true;
			}else if ( flagFall && (i%2 != 0) ){
				struct DataLogoRecord dtlogo;
				pdata->clearRecordLogo(dtlogo);
				Msec msecRise = msecLast;
				Msec msecFall = msecCur;
				dtlogo.rise   = msecRise;
				dtlogo.fall   = msecFall;
				dtlogo.rise_l = msecRise;
				dtlogo.rise_r = msecRise;
				dtlogo.fall_l = msecFall;
				dtlogo.fall_r = msecFall;
				dtlogo.org_rise   = msecRise;
				dtlogo.org_fall   = msecFall;
				dtlogo.org_rise_l = msecRise;
				dtlogo.org_rise_r = msecRise;
				dtlogo.org_fall_l = msecFall;
				dtlogo.org_fall_r = msecFall;
				dtlogo.unit       = LOGO_UNIT_DIVIDE;	// Trim出力はロゴ別に分離
				pdata->pushRecordLogo(dtlogo);			// add data
				msecLast = msecCur;
				flagFall = false;
			}
		}
	}
}

//---------------------------------------------------------------------
// ロゴ状態を開始時に戻す再設定
//---------------------------------------------------------------------
void JlsScript::setLogoReset(){
	pdata->backupLogosetLoad();		// ロゴを保存状態に戻す
	globalState.setExe1st(true);	// 検索実行前に状態を戻す
}


//=====================================================================
// Call用引数追加処理
//=====================================================================

//---------------------------------------------------------------------
// 引数設定領域で変数設定した時の処理
//---------------------------------------------------------------------
bool JlsScript::setArgStateSetCmd(JlsCmdArg& cmdarg, JlsScriptState& state){
	//--- Defaultコマンド以外は無関係 ---
	if ( cmdarg.cmdsel != CmdType::Default ){
		return false;
	}
	//--- 引数設定領域以外は無関係 ---
	if ( state.isArgArea() == false ){
		return false;
	}
	//--- 引数として設定する変数の処理 ---
	string strName = cmdarg.getStrArg(1);
	string strVal;
	{	// Default値の読み込み
		bool exact = true;
		if ( getJlsRegVar(strVal, strName, exact) <= 0 ){
			strVal  = cmdarg.getStrArg(2);
		}
	}
	bool overwrite = true;		// Defaultでも値を読み込んだ上で再設定
	bool flagLocal = true;		// ローカル変数として設定
	bool success = setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
	if ( success ){
		state.addArgName(strName);	// 変数名を引数として記憶
	}
	return success;
}
//---------------------------------------------------------------------
// 変数展開を後で行う領域であれば最初に引数変数を挿入
//---------------------------------------------------------------------
bool JlsScript::setArgStateInsertReg(JlsScriptState& state){
	//--- 引数保管があれば遅延実行バッファに現在の変数設定を追加する ---
	bool flagExe = false;
	int numArg = state.sizeArgNameList();
	if ( numArg > 0 ){
		//--- 現在の格納エリアを取得 ---
		LazyType typeLazy = state.getLazyStartType();
		string strMemName;
		if ( state.isMemArea() ){
		  strMemName = state.getMemName();
		}
		//--- 引数の数だけ挿入 ---
		for(int i=0; i<numArg; i++){
			string strName;
			string strVal;
			bool cont = state.getArgName(strName, i);	// 変数名
			if ( cont ){
				bool exact = true;
				if ( getJlsRegVar(strVal, strName, exact) <= 0 ){	// 変数値
					cont = false;
				}
			}
			if ( cont ){
				string strBuf = "LocalSet " + strName + " " + strVal;
				if ( typeLazy != LazyType::None ){
					flagExe = state.setLazyStore(typeLazy, strBuf);
				}else if ( strMemName.empty() == false ){
					flagExe = state.setMemStore(strMemName, strBuf);
				}
			}
		}
	}
	return flagExe;
}


//=====================================================================
// 設定コマンド処理
//=====================================================================

//---------------------------------------------------------------------
// If文処理
//---------------------------------------------------------------------
bool JlsScript::setCmdCondIf(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;
	bool flagCond = cmdarg.getCondFlag();
	switch( cmdarg.cmdsel ){
		case CmdType::If:						// If文
			state.ifBegin(flagCond);
			break;
		case CmdType::EndIf:					// EndIf文
			{
				int errnum = state.ifEnd();
				if (errnum > 0){
					globalState.addMsgError("error: too many EndIf.\n");
					success = false;
				}
			}
			break;
		case CmdType::Else:						// Else文
		case CmdType::ElsIf:					// ElsIf文
			{
				int errnum = state.ifElse(flagCond);
				if (errnum > 0){
					globalState.addMsgError("error: not exist 'If' but exist 'Else/ElsIf' .\n");
					success = false;
				}
			}
			break;
		default:
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Call処理
//---------------------------------------------------------------------
bool JlsScript::setCmdCall(JlsCmdArg& cmdarg, int loop){
	bool success = true;

	if ( cmdarg.cmdsel == CmdType::Call ){
		//--- Call命令用のPath設定、読み込みファイル名設定 ---
		string strFileName;
		bool flagFull = false;		// 入力はファイル名部分のみ
		makeFullPath(strFileName, cmdarg.getStrArg(1), flagFull);
		//--- Call実行処理 ---
		loop ++;
		if (loop < SIZE_CALL_LOOP){				// 再帰呼び出しは回数制限
			startCmdLoop(strFileName, loop);
		}
		else{
			// 無限呼び出しによるバッファオーバーフロー防止のため
			globalState.addMsgError("error: many recursive call\n");
			success = false;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// リピートコマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdRepeat(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::Repeat:				// Repeat文
			{
				int val = cmdarg.getValStrArg(1);
				int errnum = state.repeatBegin(val);
				if (errnum > 0){
					globalState.addMsgError("error: overflow at Repeat\n");
					success = false;
				}
			}
			break;
		case CmdType::EndRepeat:			// EndRepeat文
			{
				int errum = state.repeatEnd();
				if (errum > 0){
					globalState.addMsgError("error: Repeat - EndRepeat not match\n");
					success = false;
				}
			}
			break;
		default:
			break;
	}
	return success;
}
//---------------------------------------------------------------------
// スクリプト実行制御コマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdFlow(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::LocalSt:
			globalState.setLocalRegCreateOne();
			break;
		case CmdType::LocalEd:
			globalState.setLocalRegReleaseOne();
			break;
		case CmdType::ArgBegin:
			state.setArgArea(true);
			if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -localオプション
				globalState.setLocalOnly(true);
			}
			break;
		case CmdType::ArgEnd:
			state.setArgArea(false);
			globalState.setLocalOnly(false);
			break;
		case CmdType::Exit:
			globalState.setCmdExit(true);
			break;
		case CmdType::Return:
			state.setCmdReturn(true);
			break;
		default:
			globalState.addMsgError("error:internal setting\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// システム関連コマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdSys(JlsCmdArg& cmdarg){
	bool success = true;

	switch ( cmdarg.cmdsel ){
		case CmdType::FileOpen:
			globalState.fileOpen(cmdarg.getStrArg(1), false);
			break;
		case CmdType::FileAppend:
			globalState.fileOpen(cmdarg.getStrArg(1), true);
			break;
		case CmdType::FileClose:
			globalState.fileClose();
			break;
		case CmdType::Echo:
			globalState.fileOutput(cmdarg.getStrArg(1) + "\n");
			break;
		case CmdType::LogoOff:
			setSystemRegNologo(false);
			break;
		case CmdType::OldAdjust:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.oldAdjust = val;
			}
			break;
		case CmdType::LogoDirect:
			setLogoDirect(cmdarg);
			break;
		case CmdType::LogoExact:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.nLgExact = val;
				pdata->extOpt.fixNLgExact = 1;
			}
			break;
		case CmdType::LogoReset:
			setLogoReset();
			break;
		case CmdType::ReadData:
			readDataList(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadTrim:
			readDataTrim(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadString:
			readDataString(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::EnvGet:
			readDataEnvGet(cmdarg, cmdarg.getStrArg(1));
			break;
		default:
			globalState.addMsgError("error:internal setting\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// レジスタ設定関連処理
//---------------------------------------------------------------------
bool JlsScript::setCmdReg(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	//--- 引数設定処理だった場合は実行して完了 ---
	if ( setArgStateSetCmd(cmdarg, state) ){
		return true;
	}

	switch( cmdarg.cmdsel ){
		case CmdType::Default:
		case CmdType::Set:
		case CmdType::EvalFrame:
		case CmdType::EvalTime:
		case CmdType::EvalNum:
		case CmdType::LocalSet:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::Default )? false : true;
				bool flagLocal = ( cmdarg.cmdsel == CmdType::LocalSet )? true : false;
				if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -localオプション
					flagLocal = true;
				}
				if ( cmdarg.getOpt(OptType::FlagDefault) > 0 ){	// -defaultオプション
					overwrite = false;
				}
				success = setJlsRegVarWithLocal(cmdarg.getStrArg(1), cmdarg.getStrArg(2), overwrite, flagLocal);
			}
			break;
		case CmdType::SetParam:
			{
				ConfigVarType typePrm = (ConfigVarType) cmdarg.getValStrArg(1);
				int val = cmdarg.getValStrArg(2);
				pdata->setConfig(typePrm, val);
			}
			break;
		case CmdType::CountUp:
			{
				success = false;
				bool flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
				setJlsRegVarWithLocal(cmdarg.getStrArg(1), "0", false, flagLocal);	// 未設定時に0書き込み
				string strVal;
				bool exact = true;
				if ( getJlsRegVar(strVal, cmdarg.getStrArg(1), exact) > 0 ){
					int val;
					int pos = 0;
					pos = pdata->cnv.getStrValNum(val, strVal, pos);
					if ( pos >= 0 ){
						val += 1;			// count up
						success = setJlsRegVarWithLocal(cmdarg.getStrArg(1), to_string(val), true, flagLocal);
					}
				}
			}
			break;
		case CmdType::OptSet:
		case CmdType::OptDefault:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::OptDefault )? false : true;
				setSystemRegOptions(cmdarg.getStrArg(1), 0, overwrite);
			}
			break;
		case CmdType::UnitSec:					// 特定レジスタ設定
			{
				pdata->cnv.changeUnitSec(cmdarg.getValStrArg(1));
				setSystemRegUpdate();
			}
			break;
		case CmdType::ArgSet:
			success = setArgReg(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::ListGetAt:
			setResultRegListGetAt(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListIns:
			setResultRegListIns(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListDel:
			setResultRegListDel(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListSetAt:
			setResultRegListRep(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListClear:
			setResultRegListClear(cmdarg);
			break;
		case CmdType::ListSort:
			setResultRegListSort(cmdarg);
			break;
		default:
			globalState.addMsgError("error:internal setting(RegCmd)\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Lazy/Memory制御コマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdMemFlow(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::LazyStart:			// LazyStart文
			{
				LazyType typeLazy = LazyType::LazyE;			// デフォルトのlazy設定
				if ( cmdarg.tack.typeLazy != LazyType::None ){	// オプション指定があれば優先
					typeLazy = cmdarg.tack.typeLazy;
				}
				state.setLazyStartType(typeLazy);		// lazy設定
				state.checkArgRegInsert(cmdarg.cmdsel);	// 引数変数の挿入のための処理
			}
			break;
		case CmdType::EndLazy:				// EndLazy文
			state.setLazyStartType(LazyType::None);	// lazy解除
			break;
		case CmdType::Memory:				// Memory文
			state.startMemArea(cmdarg.getStrArg(1));
			state.checkArgRegInsert(cmdarg.cmdsel);	// 引数変数の挿入のための処理
			break;
		case CmdType::EndMemory:			// EndMemory文
			state.endMemArea();
			break;
		case CmdType::ExpandOn:
			state.setMemExpand(true);
			break;
		case CmdType::ExpandOff:
			state.setMemExpand(false);
			break;
		default:
			globalState.addMsgError("error:internal setting(LazyCategory)\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Lazy/Memory実行コマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdMemExe(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	//--- 実行処理 ---
	switch( cmdarg.cmdsel ){
		case CmdType::MemCall:			// MemCall文
			state.setMemCall(cmdarg.getStrArg(1));
			break;
		case CmdType::MemErase:			// MemErase文
			state.setMemErase(cmdarg.getStrArg(1));
			break;
		case CmdType::MemCopy:			// MemCopy文
			state.setMemCopy(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::MemMove:			// MemMove文
			state.setMemMove(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::MemAppend:		// MemAppend文
			state.setMemAppend(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::LazyFlush:		// LazyFlush文
			state.setLazyFlush();
			break;
		case CmdType::LazyAuto:			// LazyAuto文
			state.setLazyAuto(true);
			break;
		case CmdType::LazyStInit:		// LazyStInit(Lazy用Auto未実行状態に戻す。念のため残す)
			state.setLazyStateIniAuto(true);
			break;
		case CmdType::MemOnce:			// MemOnce文
			{
				int val = cmdarg.getValStrArg(1);
				bool dupe = ( val != 1 )? true : false;
				state.setMemDupe(dupe);
			}
			break;
		case CmdType::MemEcho:			// MemEcho文
			state.setMemEcho(cmdarg.getStrArg(1));
			break;
		case CmdType::MemDump:			// Debug
			state.setMemGetMapForDebug();
			break;
		default:
			globalState.addMsgError("error:internal setting(MemCategory)\n");
			success = false;
			break;
	}
	return success;
}


//=====================================================================
// コマンド実行処理
//=====================================================================

//---------------------------------------------------------------------
// スクリプト各行のコマンド実行
//---------------------------------------------------------------------
bool JlsScript::exeCmd(JlsCmdSet& cmdset){
	//--- コマンド実行の確認フラグ ---
	bool valid_exe = true;				// 今回の実行
	bool exe_command = false;			// 実行状態
	//--- 前コマンド実行済みか確認 (-else option) ---
	if (cmdset.arg.getOpt(OptType::FlagElse) > 0){
		if ( isSystemRegLastexe() ){	// 直前コマンドを実行した場合
			valid_exe = false;			// 今回コマンドは実行しないが
			exe_command = true;			// 実行済み扱い
		}
	}
	//--- 前コマンド実行済みか確認 (-cont option) ---
	if (cmdset.arg.getOpt(OptType::FlagCont) > 0){
		if ( isSystemRegLastexe() == false ){	// 直前コマンドを実行していない場合
			valid_exe = false;			// 今回コマンドも実行しない
		}
	}
	//--- コマンド実行 ---
	if (valid_exe){
		//--- 共通設定 ---
		m_funcLimit->limitCommonRange(cmdset);		// コマンド共通の範囲限定

		//--- オプションにAuto系構成が必要な場合(-AC -NoAC)の構成作成 ---
		if ( cmdset.arg.tack.needAuto ){
			exeCmdCallAutoSetup(cmdset);
		}
		//--- 分類別にコマンド実行 ---
		switch( cmdset.arg.category ){
			case CmdCat::AUTO:
				exe_command = exeCmdCallAutoScript(cmdset);		// Auto処理クラス呼び出し
				break;
			case CmdCat::AUTOEACH:
				exe_command = exeCmdAutoEach(cmdset);			// 各ロゴ期間でAuto系処理
				break;
			case CmdCat::LOGO:
			case CmdCat::AUTOLOGO:
				exe_command = exeCmdLogo(cmdset);				// ロゴ別に実行
				break;
			case CmdCat::NEXT:
				exe_command = exeCmdNextTail(cmdset);			// 次の位置取得処理
				break;
			default:
				break;
		}
	}

	return exe_command;
}

//---------------------------------------------------------------------
// AutoScript拡張を実行
//---------------------------------------------------------------------
//--- コマンド解析後の実行 ---
bool JlsScript::exeCmdCallAutoScript(JlsCmdSet& cmdset){
	bool setup_only = false;
	return exeCmdCallAutoMain(cmdset, setup_only);
}
//--- コマンド解析のみ ---
bool JlsScript::exeCmdCallAutoSetup(JlsCmdSet& cmdset){
	bool setup_only = true;
	return exeCmdCallAutoMain(cmdset, setup_only);
}

//--- 実行メイン処理 ---
bool JlsScript::exeCmdCallAutoMain(JlsCmdSet& cmdset, bool setup_only){
	//--- 初回のみ実行 ---
	if ( pdata->isAutoModeInitial() ){
		//--- ロゴ使用レベルを設定 ---
		if (pdata->isExistLogo() == false){		// ロゴがない場合はロゴなしに設定
			pdata->setLevelUseLogo(CONFIG_LOGO_LEVEL_UNUSE_ALL);
		}
		else{
			int level = pdata->getConfig(ConfigVarType::LogoLevel);
			if (level <= CONFIG_LOGO_LEVEL_DEFAULT){		// 未設定時は値を設定
				level = CONFIG_LOGO_LEVEL_USE_HIGH;
			}
			pdata->setLevelUseLogo(level);
		}
		if (pdata->isUnuseLogo()){				// ロゴ使用しない場合
			pdata->extOpt.flagNoLogo = 1;		// ロゴなしに設定
			setSystemRegUpdate();				// NOLOGO更新
		}
	}
	//--- Autoコマンド実行 ---
	return m_funcAutoScript->startCmd(cmdset, setup_only);		// AutoScriptクラス呼び出し
}

//---------------------------------------------------------------------
// 各ロゴ期間を範囲として実行するAutoコマンド (-autoeachオプション)
//---------------------------------------------------------------------
bool JlsScript::exeCmdAutoEach(JlsCmdSet& cmdset){
	bool exeflag_total = false;
	NrfCurrent logopt = {};
	while( pdata->getNrfptNext(logopt, LOGO_SELECT_VALID) ){
		RangeMsec rmsec_logo;
		LogoResultType rtype_rise;
		LogoResultType rtype_fall;
		//--- 確定状態を確認 ---
		pdata->getResultLogoAtNrf(rmsec_logo.st, rtype_rise, logopt.nrfRise);
		pdata->getResultLogoAtNrf(rmsec_logo.ed, rtype_fall, logopt.nrfFall);
		//--- 確定時以外は候補場所にする ---
		if (rtype_rise != LOGO_RESULT_DECIDE){
			rmsec_logo.st = logopt.msecRise;
		}
		if (rtype_fall != LOGO_RESULT_DECIDE){
			rmsec_logo.ed = logopt.msecFall;
		}
		//--- 各ロゴ期間を範囲として位置を設定 ---
		m_funcLimit->resizeRangeHeadTail(cmdset, rmsec_logo);
		//--- Autoコマンド実行 ---
		int exeflag = exeCmdCallAutoScript(cmdset);	// Auto処理クラス呼び出し
		//--- 実行していたら実行フラグ設定 ---
		if (exeflag){
			exeflag_total = true;
		}
	};
	return exeflag_total;
}

//---------------------------------------------------------------------
// 全ロゴの中で選択ロゴを実行
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogo(JlsCmdSet& cmdset){
	//--- ロゴ番号オプションから有効なロゴ番号位置をすべて取得 ---
	int nmax_list = m_funcLimit->limitLogoList(cmdset);
	//--- １箇所だけのコマンドか ---
	bool flag_onepoint = cmdset.arg.tack.onePoint;
	//--- １箇所だけコマンドの場合は一番近い候補だけを有効にする ---
	int nlist_base = -1;
	if (flag_onepoint){
		int difmsec_base = 0;
		for(int i=0; i<nmax_list; i++){
			//--- 制約条件を満たしているロゴか確認 ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			if (exeflag){
				WideMsec wmsec = cmdset.limit.getTargetRangeWide();
				Msec msec_target = wmsec.just;
				Nsc  nsc_sel = cmdset.limit.getResultTargetSel();
				Msec msec_sel = pdata->getMsecScp(nsc_sel);
				Msec difmsec_sel = abs(msec_target - msec_sel);
				if (difmsec_sel < difmsec_base || nlist_base < 0){
					nlist_base = i;
					difmsec_base = difmsec_sel;
				}
			}
		}
	}
	//--- リスト作成時は開始前に内容消去 ---
	if ( cmdset.arg.cmdsel == CmdType::GetList ||
	     cmdset.arg.cmdsel == CmdType::GetPos ){
		updateResultRegWrite(cmdset.arg);	// 変数($POSHOLD/$LISTHOLD)クリア
	}
	//--- 各有効ロゴを実行 ---
	bool exeflag_total = false;
	for(int i=0; i<nmax_list; i++){
		if (flag_onepoint == false || nlist_base == i){
			//--- 制約条件を満たしているロゴか確認 ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			//--- 実行分岐 ---
			if (exeflag){
				switch(cmdset.arg.category){
					case CmdCat::LOGO :
						//--- ロゴ位置を直接設定するコマンド ---
						exeflag = exeCmdLogoTarget(cmdset);
						break;
					case CmdCat::AUTOLOGO :
						//--- 推測構成から生成するコマンド ---
						exeflag = exeCmdCallAutoScript(cmdset);
						break;
					default:
						break;
				}
			}
			//--- 実行していたら実行フラグ設定 ---
			if (exeflag){
				exeflag_total = true;
			}
		}
	}

	return exeflag_total;
}

//---------------------------------------------------------------------
// ロゴ位置別の実行コマンド
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogoTarget(JlsCmdSet& cmdset){
	bool exe_command = false;
	Msec msec_force   = cmdset.limit.getTargetRangeForce();
	Nsc nsc_scpos_sel = cmdset.limit.getResultTargetSel();
	Nsc nsc_scpos_end = cmdset.limit.getResultTargetEnd();
	Nrf nrf = cmdset.limit.getLogoBaseNrf();

	switch(cmdset.arg.cmdsel){
		case CmdType::Find:
			if (nsc_scpos_sel >= 0){
				if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){	// 推測構成に反映
					exeCmdCallAutoScript(cmdset);					// Auto処理クラス呼び出し
				}
				else{								// 従来構成に反映
					Msec msec_tmp = pdata->getMsecScpEdge(nsc_scpos_sel, jlsd::edgeFromNrf(nrf));
					pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
				}
				exe_command = true;
			}
			break;
		case CmdType::MkLogo:
			if (nsc_scpos_sel >= 0 && nsc_scpos_end >= 0){
				int msec_st, msec_ed;
				if (nsc_scpos_sel < nsc_scpos_end){
					msec_st = pdata->getMsecScp(nsc_scpos_sel);
					msec_ed = pdata->getMsecScpBk(nsc_scpos_end);
				}
				else{
					msec_st = pdata->getMsecScp(nsc_scpos_end);
					msec_ed = pdata->getMsecScpBk(nsc_scpos_sel);
				}
				bool overlap = ( cmdset.arg.getOpt(OptType::FlagOverlap) != 0 )? true : false;
				bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
				bool unit    = ( cmdset.arg.getOpt(OptType::FlagUnit)    != 0 )? true : false;
				int nsc_ins = pdata->insertLogo(msec_st, msec_ed, overlap, confirm, unit);
				exe_command = (nsc_ins >= 0)? true : false;
			}
			break;
		case CmdType::DivLogo:
			if (nsc_scpos_sel >= 0 || msec_force >= 0){
				//--- 対象位置取得 ---
				Msec msec_target;
				Msec msec_st, msec_ed;
				if (nsc_scpos_sel >= 0){
					msec_target = pdata->getMsecScp(nsc_scpos_sel);
				}
				else{
					msec_target = msec_force;
				}
				if (jlsd::isLogoEdgeRiseFromNrf(nrf)){		// start edge
					msec_st = pdata->getMsecLogoNrf(nrf);
					msec_ed = pdata->cnv.getMsecAdjustFrmFromMsec(msec_target, -1);
				}
				else{										// end edge
					msec_st = pdata->cnv.getMsecAlignFromMsec(msec_target);
					msec_ed = pdata->getMsecLogoNrf(nrf);
				}
				bool overlap = true;
				bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
				bool unit = true;
				Nsc nsc_ins = pdata->insertLogo(msec_st, msec_ed, overlap, confirm, unit);
				exe_command = (nsc_ins >= 0)? true : false;
			}
			break;
		case CmdType::Select:
			if (nsc_scpos_sel >= 0){
				// 従来の確定位置を解除
				Nsc nsc_scpos = pdata->sizeDataScp();
				Msec msec_nrf = pdata->getMsecLogoNrf(nrf);
				for(int j=1; j<nsc_scpos - 1; j++){
					Msec msec_j = pdata->getMsecScp(j);
					if (msec_j == msec_nrf){
						if (pdata->getScpStatpos(j) > SCP_PRIOR_NONE){
							pdata->setScpStatpos(j, SCP_PRIOR_NONE);
						}
					}
				}
				// 先頭区切り位置の保持
				if (nrf == 0){
					pdata->recHold.msecSelect1st = msec_nrf;
				}
				// 新しい確定位置を設定
				Msec msec_tmp = pdata->getMsecScp(nsc_scpos_sel);
				pdata->setMsecLogoNrf(nrf, msec_tmp);
				pdata->setScpStatpos(nsc_scpos_sel, SCP_PRIOR_DECIDE);
				exe_command = true;
				if (cmdset.arg.getOpt(OptType::FlagConfirm) > 0){
					pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
				}
			}
			break;
		case CmdType::Force:
			{
				WideMsec wmsec = cmdset.limit.getTargetRangeWide();
				Msec msec_tmp = pdata->cnv.getMsecAlignFromMsec( wmsec.just );
				if (msec_tmp >= 0){
					exe_command = true;
					if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){		// 推測構成に反映
						exeCmdCallAutoScript(cmdset);						// Auto処理クラス呼び出し
					}
					else{
						pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
					}
				}
			}
			break;
		case CmdType::Abort:
			exe_command = true;
			pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf);
			if (cmdset.arg.getOpt(OptType::FlagWithN) > 0){
				pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf+1);
			}
			if (cmdset.arg.getOpt(OptType::FlagWithP) > 0){
				pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf-1);
			}
			break;
		case CmdType::GetPos:
			{
				Msec msecHold;
				exe_command = getMsecTargetWithForce(msecHold, cmdset);
				if ( exe_command ){
					setResultRegPoshold(cmdset.arg, msecHold);		// 変数に設定
				}
			}
			break;
		case CmdType::GetList:
			{
				Msec msecHold;
				exe_command = getMsecTargetWithForce(msecHold, cmdset);
				if ( exe_command ){
					setResultRegListhold(cmdset.arg, msecHold);	// 変数に設定
				}
			}
			break;
		case CmdType::DivFile:
			{
				Msec msec = cmdset.limit.getTargetRangeWide().just;
				exe_command = true;
				auto it = std::lower_bound(pdata->divFile.begin(), pdata->divFile.end(), msec);
				if (it == pdata->divFile.end() || *it != msec) {
					pdata->divFile.insert(it, msec);
				}
			}
			break;
		default:
			break;
	}
	return exe_command;
}

//---------------------------------------------------------------------
// 次のHEADTIME/TAILTIMEを取得
//---------------------------------------------------------------------
bool JlsScript::exeCmdNextTail(JlsCmdSet& cmdset){
	//--- TAILFRAMEを次のHEADFRAMEに ---
	string cstr;
	Msec msec_headframe = -1;
	if (getJlsRegVar(cstr, "TAILTIME", true) > 0){
		pdata->cnv.getStrValMsecM1(msec_headframe, cstr, 0);
	}
	//--- 範囲を取得 ---
	WideMsec wmsec_target;
	wmsec_target.just  = msec_headframe + cmdset.arg.wmsecDst.just;
	wmsec_target.early = msec_headframe + cmdset.arg.wmsecDst.early;
	wmsec_target.late  = msec_headframe + cmdset.arg.wmsecDst.late;
	bool force = false;
	if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){
		force = true;
	}
	//--- 一番近いシーンチェンジ位置を取得 ---
	m_funcLimit->selectTargetByRange(cmdset, wmsec_target, force);

	//--- 結果を格納 --
	bool exeflag = false;
	Msec msec_tailframe;
	exeflag = getMsecTargetWithForce(msec_tailframe, cmdset);
	if ( exeflag ){
		setSystemRegHeadtail(msec_headframe, msec_tailframe);
		//--- -autochgオプション対応 ---
		if ( cmdset.arg.getOpt(OptType::FlagAutoChg) > 0 ){		// 推測構成に反映
			exeCmdCallAutoScript(cmdset);						// Auto処理クラス呼び出し
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// cmdset情報から対象となる無音シーンチェンジを取得、なければ設定によっては強制作成
// 出力
//  返り値     ：結果取得有無
//  msecResult : 取得した無音シーンチェンジ位置
//---------------------------------------------------------------------
bool JlsScript::getMsecTargetWithForce(Msec& msecResult, JlsCmdSet& cmdset){
	Nsc  nscSel     = cmdset.limit.getResultTargetSel();
	Msec msecForce  = cmdset.limit.getTargetRangeForce();
	LogoEdgeType edgeOut = cmdset.limit.getTargetOutEdge();

	bool success = false;
	if ( nscSel >= 0 ){		// 対応する無音シーンチェンジが存在する場合
		msecResult = pdata->getMsecScpEdge(nscSel, edgeOut);
		success = true;
	}
	else if ( msecForce >= 0 ){	// 無音シーンチェンジなく-force/-noforceオプションあり
		msecResult = pdata->cnv.getMsecAlignFromMsec(msecForce);
		RangeMsec rmsec;
		if ( jlsd::isLogoEdgeRise(edgeOut) ){		// start edge
			rmsec.st = msecResult;
			rmsec.ed = pdata->cnv.getMsecAdjustFrmFromMsec(msecResult, -1);
		}else{
			rmsec.st = pdata->cnv.getMsecAdjustFrmFromMsec(msecResult, +1);
			rmsec.ed = msecResult;
		}
		if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){	// -forceオプションで強制作成時
			pdata->insertScpos(rmsec.st, rmsec.ed, -1, SCP_PRIOR_DECIDE);
		}
		success = true;
	}
	return success;
}

