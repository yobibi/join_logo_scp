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
	m_funcDecode.reset(new JlsScriptDecode(pdata, &funcList));
	m_funcLimit.reset(new JlsScriptLimit(pdata));
	// Auto系コマンドを拡張使用
	m_funcAutoScript.reset(new JlsAutoScript(pdata));

	// レジスタアクセス関連設定 ---
	funcList.setDataPointer(pdata);
	funcReg.setDataPointer(pdata, &globalState, &funcList);

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

	// 起動オプション処理は funcReg で実施
	return funcReg.setOptionsGetOne(argrest, strv, str1, str2, overwrite);
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
	pdata->cnv.getStrFilePath(strPathOnly, strSrc);	// パス検索保管
	//--- 基本ファイル名設定 ---
	if ( flagFull ){			// メインファイルのフルパス入力時はパス情報を取得する
		globalState.setFullPathJL(strSrc);
		globalState.setPathNameJL(strPathOnly);
		strFull= strSrc;
		strName = strSrc.substr( strPathOnly.length() );
	}
	else{					// Callでファイル名部分のみの場合はパス情報を追加する
		bool needPath = strPathOnly.empty();
		if ( !needPath ){	// フォルダ指定なしor指定場所にファイルなしの時は保管パス情報を付加
			needPath = !makeFullPathIsExist(strSrc.c_str() );
		}
		if ( needPath ){
			strPathOnly = globalState.getPathNameJL();
			strFull = strPathOnly + strSrc;
			strName = strSrc;
		}else{
			strFull= strSrc;
			strName = strSrc.substr( strPathOnly.length() );
		}
	}
	//--- -sublist設定を取得 ---
	string remain = pdata->extOpt.subList;		// 事前設定のサブフォルダ取得
	//--- サブフォルダのパス指定あれば変更 ---
	if ( !pdata->extOpt.subPath.empty() ){
		strPathOnly = pdata->extOpt.subPath;
		pdata->cnv.getStrFileAllPath(strPathOnly);	// 最後に区切り付加
		if ( flagFull ){	// メインファイルのフルパス入力時は-sublistzある時フルパス指定場所を最優先
			remain = "<," + remain;
		}
		remain = remain + ",.";		// サブでない場所も最後に追加
	}else{
		remain = remain + ",";		// 次のフルパス位置検索用
	}
	//--- フルパス位置が設定なければ追加する処理 ---
	{
		auto posCur = remain.find(",<,");
		if ( posCur == string::npos ){
			if ( remain.substr(0,2) != "<," ){
				remain = "<," + remain;				// フルパス位置を最初に追加
			}
		}
	}
	//--- -sublist設定からコンマ区切りで1つずつ確認 ---
	bool decide = false;
	while( !remain.empty() && !decide ){
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
		string strTry;
		if ( subname == "<" ){		// -incmd場所
			strTry = strFull;
		}else if ( subname == "." ){	// -sublist変更後の場所
			strTry = strPathOnly + strName;
		}else{
			pdata->cnv.getStrFileAllPath(subname);	// 最後に区切り付加
			strTry = strPathOnly + subname + strName;
		}
		decide = makeFullPathIsExist(strTry);
		if ( decide ){			// サブフォルダのパス存在で結果更新
			strFull = strTry;
		}
	}
	//--- ファイルパス表示用 ---
	if ( decide ){
		string mes = "join_logo_scp_Call : " + strFull;
		pdata->dispSysMesN(mes, JlsDataset::SysMesType::CallFile);
	}
	return decide;
}
bool JlsScript::makeFullPathIsExist(const string& str){
	LocalIfs ifs(str.c_str());
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

	//--- 共通先頭実行ファイル設定 ---
	string nameSetup = pdata->extOpt.setup;
	if ( !nameSetup.empty() ){
		string strPart = nameSetup;
		bool flagFull = false;		// 基準となるフルパスではない
		if ( !makeFullPath(nameSetup, strPart, flagFull) ){
			nameSetup.clear();
			globalState.addMsgErrorN("warning: not found setup-file " + strPart);
		}
	}
	//--- システム変数の初期値を設定 ---
	funcReg.setSystemRegInit();

	//--- ロゴリセット時のバックアップデータ保存 ---
	pdata->backupLogosetSave();

	//--- JLスクリプト実行 ---
	int errnum = startCmdEnter(nameFullPath, nameSetup);

	//--- デバッグ用の表示 ---
	if (pdata->extOpt.verbose > 0 && errnum == 0){
		pdata->displayLogo();
		pdata->displayScp();
	}

	return errnum;
}


//---------------------------------------------------------------------
// コマンド実行開始
// 入力：
//   fnameMain    : スクリプトファイル名（メイン）
//   fnameSetup   : スクリプトファイル名（共通先頭実行）
// 出力：
//   返り値  ：0=正常終了 2=ファイル異常
//---------------------------------------------------------------------
int JlsScript::startCmdEnter(const string& fnameMain, const string& fnameSetup){
	//--- 初回実行 ---
	globalState.setExe1st(true);

	//--- 制御信号（終端階層） ---
	JlsScriptState stateEnd(&globalState);

	//--- 共通先頭実行ファイル読み込み ---
	if ( !fnameSetup.empty() ){
		startCmdLoop(fnameSetup, 0);
	}
	globalState.checkMsgError(true);

	//--- メインファイル読み込み ---
	int errnum = startCmdLoop(fnameMain, 0);

	//--- 一番最後に実行設定されたコマンドを実行 ---
	startCmdLoopLazyEnd(stateEnd);
	startCmdLoopLazyOut(stateEnd, "FINALIZE");		// 最終処理
	funcReg.setOutDirect();							// Trim直接出力対応
	startCmdLoopLazyOut(stateEnd, "OUTPUT");		// 出力用も最後に実行
	globalState.checkMemUnused();		// 未使用MemSet確認
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
	funcReg.setSystemRegLastexe(false);

	//--- Fcallによる呼び出し対応 ---
	bool byFcall = false;
	if ( isFcallName(fname) ){	// Fcallによる呼び出し
		byFcall = true;
	}
	//--- ファイル読み込み ---
	LocalIfs ifs;
	if ( !byFcall ){	// Fcall以外はファイルから
		ifs.open(fname.c_str());
		if ( !ifs.is_open() ){
			globalState.addMsgErrorN("error: failed to open " + fname);
			return 2;
		}
	}
	//--- 制御信号 ---
	JlsScriptState state(&globalState);
	//--- ローカル変数階層作成 ---
	int numLayerStart = globalState.setLocalRegCreateCall();	// 最上位階層扱いで作成
	//--- Fcall時のコマンド設定 ---
	if ( byFcall ){
		state.setMemCall( getFcallName(fname) );
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
	int numLayerEnd = globalState.setLocalRegReleaseAny();
	//--- ネストエラー確認 ---
	if ( forceExit == false ){
		bool flagErr = false;
		if ( numLayerStart != numLayerEnd ){
			globalState.addMsgError("error: { and } are not matched");
			flagErr = true;
		}
		int flags_remain = state.isRemainNest();
		if (flags_remain & 0x01){
			globalState.addMsgError("error: EndIf is not found");
			flagErr = true;
		}
		if (flags_remain & 0x02){
			globalState.addMsgError("error: EndRepeat is not found");
			flagErr = true;
		}
		if ( state.isLazyArea() ){
			globalState.addMsgError("error: EndLazy is not found");
			flagErr = true;
		}
		if ( state.isMemArea() ){
			globalState.addMsgError("error: EndMemory or EndFunction is not found");
			flagErr = true;
		}
		if ( flagErr ){
			globalState.addMsgErrorN("in " + fname);
		}
	}
	//--- 変数階層の解放不足は元に戻す ---
	if ( numLayerStart < numLayerEnd ){
		for(int i=0; i < (numLayerEnd - numLayerStart); i++){
			globalState.setLocalRegReleaseAny();
		}
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
//		if ( state.isCmdReturnExit() ){	// Exitで抜けた時に遅延実行の残りを実行するかで決める
//			break;
//		}
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// 出力用にmemory識別子OUTPUTのコマンドを実行
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyOut(JlsScriptState& state, const string& name){
	//--- 最後に実行するメモリを実行キャッシュに設定 ---
	state.setMemCall(name);

	//--- lazy実行キャッシュから読み出し実行 ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
//		if ( state.isCmdReturnExit() ){	// Exitで抜けた時にFINALIZE/OUTPUTを実行するかで決める
//			break;
//		}
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
	bool exe_command = funcReg.isSystemRegLastexe();
	//--- 変数を置換 ---
	string strBuf;
	funcReg.replaceBufVar(strBuf, strBufOrg);

	//--- デコード処理（コマンド部分） ---
	JlsCmdSet cmdset;									// コマンド格納
	bool onlyCmd = true;
	CmdErrType errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// コマンドのみ解析

	//--- デコード処理（全体） ---
	bool fullDecode = false;
	if ( errval == CmdErrType::None ){
		if ( state.isNeedRaw(cmdset.arg.category) ){	// LazyStartとMemory区間中は変数を展開しない
			strBuf = strBufOrg;
		}
		errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, false);	// コマンド全体解析
		//--- スキップ行のエラー消去 ---
		fullDecode = state.isNeedFullDecode(cmdset.arg.cmdsel, cmdset.arg.category);
		if ( !fullDecode ){
			globalState.clearRegError();			// コマンドのみのケースは変数展開中のエラー消去
			if ( errval != CmdErrType::ErrCmd ){	// コマンド認識以外の解析エラーも消去
				errval = CmdErrType::None;
			}
		}
		//--- コマンドネスト(End*による終了まで)、; のEND種類判別対応 ---
		state.addNestInfoForEnd(cmdset.arg.cmdsel, cmdset.arg.category);
	}
	//--- 遅延処理、コマンド解析後の変数展開 ---
	bool enable_exe = false;
	if ( errval == CmdErrType::None ){
		//--- 遅延実行処理（現在実行しない場合はfalseを返す） ---
		enable_exe = setStateMem(state, cmdset.arg, strBuf);

		//--- 変数展開（IF文判定式処理、コマンド内使用変数取得） ---
		if ( enable_exe ){
			bool success = true;
			if ( fullDecode ){
				success = expandDecodeCmd(state, cmdset.arg, strBuf);
			}
			//--- 変数展開のエラー時処理 ---
			if ( success == false ){
				//enable_exe = false;		// 条件判定エラーでコマンド自体を中止するかどうか
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
				success = setCmdCall(cmdset.arg, state, loop);
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
			case CmdCat::READ:						// Readコマンド
				success = setCmdRead(cmdset.arg);
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
						funcReg.setSystemRegNologo(true);
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
	if ( pdata->extOpt.vLine > 0 ){
		string mesB = ":" + strBuf;
		if ( pdata->extOpt.vLine == 2 ){
			string mes = to_string(enable_exe) + to_string(update_exe) + to_string(exe_command) + mesB;
			lcout << mes << endl;
		}else{
			string mes = to_string(enable_exe) + to_string(exe_command) + mesB;
			lcout << mes << endl;
		}
	}

	//--- 前コマンドの実行有無を代入 ---
	funcReg.setSystemRegLastexe(exe_command);
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
bool JlsScript::startCmdGetLine(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- cacheからの読み込み ---
	flagRead = startCmdGetLineOnlyCache(strBufOrg, state);

	if ( flagRead == false ){
		if ( ifs.is_open() ){
			//--- ファイルからの読み込み ---
			if ( startCmdGetLineFromFile(ifs, strBufOrg, state) ){
				flagRead = true;
				//--- Repeat用キャッシュに保存 ---
				state.addCmdCache(strBufOrg);
			}
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
	if ( flagRead ){
		globalState.setMsgBufForErr(strBufOrg);		// バッファをエラー表示用に保管
	}
	return flagRead;
}
//---------------------------------------------------------------------
// ファイルから１行読み込み（ \による行継続に対応）
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineFromFile(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state){
	strBufOrg = "";
	bool success = false;
	//--- コマンド分割からの読み込み、なければ通常ファイル読み込み ---
	if ( startCmdGetLineFromFileDivCache(strBufOrg, state) ){
		success = true;
	}else{
		bool cont = true;
		while( cont ){
			cont = false;
			string buf;
			if ( ifs.getline(buf) ){
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
		}
		if ( success ){
			globalState.setMsgBufForErr(strBufOrg);		// 分割前バッファをエラー表示用に保管
		}
	}
	if ( success ){
		startCmdGetLineFromFileParseDiv(strBufOrg, state);		// コマンド分割あれば設定
	}
	return success;
}
//--- {}による行内分割処理（取り出し） ---
bool JlsScript::startCmdGetLineFromFileDivCache(string& strBufOrg, JlsScriptState& state){
	string bufHold;
	if ( !state.popBufDivCmd(bufHold) ){		// 通常の分割保持していない状態
		return false;
	}
	strBufOrg = bufHold;
	return true;
}
//--- {}による行内分割処理（保管） ---
bool JlsScript::startCmdGetLineFromFileParseDiv(string& strBufOrg, JlsScriptState& state){
	//--- コメント除去 ---
	string strBuf;
	pdata->cnv.getStrWithoutComment(strBuf, strBufOrg);
	//--- 分割文字検索 ---
	int divDet = -1;
	auto braceS = strBuf.find("{");
	auto braceE = strBuf.find("}");
	if ( braceS == string::npos && braceE == string::npos ){
		return false;
	}
	int braceB = (int)braceE;		// 先頭の出現位置
	if ( braceE == string::npos || (braceS != string::npos && braceS < braceE) ){
		braceB = (int)braceS;
	}
	//--- 分割位置の取得 ---
	string strItem;
	int pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, 0);
	if ( pos < 0 ) return false;
	if ( braceB < pos ){		// 先頭コマンド内に出現
		if ( braceB > 0 ){
			string strP = strBuf.substr(0, braceB);
			string strTmp;
			if ( pdata->cnv.getStrItemWithQuote(strTmp, strP, 0) >= 0 ){
				divDet = braceB;	// brace前に文字列があり分割
			}
		}
		if ( divDet < 0 ){
			string strN = strBuf.substr(braceB+1);
			string strTmp;
			if ( pdata->cnv.getStrItemWithQuote(strTmp, strN, 0) >= 0 ){
				divDet = braceB+1;	// brace後に文字列があり分割
			}
		}
		if ( divDet < 0 ){			// 前後に文字列なければ分割しない
			return false;
		}
	}else{		// 2番目以降のコマンドに出現
		bool chkFunc = true;
		while( pos >= 0 && divDet < 0 ){
			int posBak = pos;
			if ( chkFunc ){		// function構文の可能性あり
				chkFunc = pdata->cnv.isStrFuncModule(strBuf, pos);
			}
			if ( chkFunc ){		// function構文
				vector<string> listMod;
				pos = pdata->cnv.getListModuleArg(listMod, strBuf, pos);
				strItem = "";
				chkFunc = false;
			}else{
				pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, pos);
			}
			if ( pos >= 0 ){
				if ( strItem == "{" || strItem == "}" || strItem == "};" ){
					divDet = posBak;
				}
			}
		}
	}
	if ( divDet < 0 ) return false;
	//--- 分割設定処理 ---
	state.pushBufDivCmd( strBuf.substr(divDet) );
	strBufOrg = strBuf.substr(0, divDet);
	return true;
}
//---------------------------------------------------------------------
// エラー表示
//---------------------------------------------------------------------
void JlsScript::startCmdDispErr(const string& strBuf, CmdErrType errval){
	if ( errval != CmdErrType::None ){
		string strErr = "";
		bool flagAdd = false;
		switch(errval){
			case CmdErrType::ErrOpt:
				strErr = "error: wrong argument";
				flagAdd = true;
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
				flagAdd = true;
				break;
			default:
				break;
		}
		if ( strErr.empty() == false ){
			string mesErr = strErr;
			if ( flagAdd ){
				mesErr += "(";
				mesErr += m_funcDecode->getErrItem();
				mesErr += ")";
			}
			globalState.addMsgErrorN(mesErr + " in " + strBuf);
			string strBufLine = globalState.getMsgBufForErr();
			if ( strBuf != strBufLine ){
				globalState.addMsgErrorN("  (read data)" + strBufLine);
			}
		}
	}
	globalState.checkErrorGlobalState(true);
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
	//--- ローカル変数自動挿入の処理 ---
	state.exeArgMstoreInsert(cmdarg.cmdsel);		// 引数変数挿入判断実行

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
		globalState.addMsgErrorN("error: can not evaluate(" + strCalc + ")");
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
		bool match = funcReg.getJlsRegVarNormal(strVal, strRemain);
		if (match && strVal != "0"){	// 変数が存在して0以外の場合
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
	funcList.revListStrEmpty(strDefaultList);		// 空リスト設定
	//--- 変数値取得設定 ---
	{
		string strVal;
		//--- POSHOLDの値を設定 ---
		if ( funcReg.getJlsRegVarNormal(strVal, strRegPos) ){	// 変数取得
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
		if ( funcReg.getJlsRegVarNormal(strVal, strRegList) ){	// 変数取得
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
	int numList = funcList.getListStrSize(strList);

	//--- 既にLogoOffが設定されていた場合は解除 ---
	if ( pdata->extOpt.flagNoLogo ){
		pdata->extOpt.flagNoLogo = 0;	// ロゴは読み込みに設定
		funcReg.setSystemRegUpdate();
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
			if ( funcList.getListStrElement(strCur, strList, i+1) ){
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
				dtlogo.unit_rise  = LOGO_UNIT_DIVIDE;	// Trim出力はロゴ別に分離
				dtlogo.unit_fall  = LOGO_UNIT_DIVIDE;	// Trim出力はロゴ別に分離
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
// CallとMemory用引数追加処理
//=====================================================================

//---------------------------------------------------------------------
// 引数設定領域(ArgBegin - ArgEnd)で変数設定した時の処理
//---------------------------------------------------------------------
bool JlsScript::setArgAreaDefault(JlsCmdArg& cmdarg, JlsScriptState& state){
	//--- Defaultコマンド以外は無関係 ---
	if ( cmdarg.cmdsel != CmdType::Default ){
		return false;
	}
	//--- 引数設定領域以外は無関係 ---
	if ( state.isArgAreaEnter() == false ){
		return false;
	}
	//--- 引数として設定する変数の処理 ---
	string strName = cmdarg.getStrArg(1);
	string strVal;
	{	// Default値の読み込み
		if ( !funcReg.getJlsRegVarNormal(strVal, strName) ){
			strVal  = cmdarg.getStrArg(2);
		}
	}
	bool overwrite = true;		// Defaultでも値を読み込んだ上で再設定
	bool flagLocal = true;		// ローカル変数として設定
	bool success = funcReg.setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
	if ( success ){
		state.addArgAreaName(strName);	// 変数名を引数として記憶
	}
	return success;
}
//---------------------------------------------------------------------
// 保管型引数を設定するコマンドの処理（MemSet/LazyStart/Memoryコマンド）
//---------------------------------------------------------------------
//--- 設定時の変数を格納するための引数設定 ---
bool JlsScript::makeArgMemStore(JlsCmdArg& cmdarg, JlsScriptState& state){
	state.clearArgMstoreBuf();				// 引数をクリア
	bool success1 = makeArgMemStoreByDefault(state);
	bool success2 = makeArgMemStoreByMemSet(cmdarg, state);
	return ( success1 && success2 );
}
//--- 格納コマンド文字列 ---
void JlsScript::makeArgMemStoreLocalSet(JlsScriptState& state, const string& strName, const string& strVal){
	string strBuf = "LocalSet " + strName + " " + strVal;
	state.addArgMstoreBuf(strBuf);
}
//--- 引数設定領域(ArgBegin - ArgEnd)で設定された変数の現在値を格納 ---
bool JlsScript::makeArgMemStoreByDefault(JlsScriptState& state){
	//--- 引数保管があれば遅延実行バッファに現在の変数設定を追加する ---
	int numArg = state.sizeArgAreaNameList();
	if ( numArg <= 0 ){
		return true;
	}
	if ( state.isMemExe() ){	// メモリからの実行中は対象外
		return true;
	}
	//--- 引数の数だけ挿入 ---
	bool success = true;
	for(int i=0; i<numArg; i++){
		string strName;
		bool cont = state.getArgAreaName(strName, i);	// 変数名
		if ( cont ){
			string strVal;
			if ( funcReg.getJlsRegVarNormal(strVal, strName) ){	// 変数値
				makeArgMemStoreLocalSet(state, strName, strVal);
			}else{
				success = false;
			}
		}
	}
	if ( !success ){
		globalState.addMsgErrorN("error: not defined variable by ArgBegin");
	}
	return success;
}
//--- MemSetコマンドの引数変数値を格納 ---
bool JlsScript::makeArgMemStoreByMemSet(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;
	if ( !cmdarg.isSetStrOpt(OptType::ListArgVar) ){	// -arg がなければ追加しない
		return success;
	}
	string strArgList = cmdarg.getStrOpt(OptType::ListArgVar);
	int pos = 0;
	while( pos >= 0 && success ){
		string strName;
		pos = pdata->cnv.getStrWord(strName, strArgList, pos);
		if ( pos > 0 ){
			string strVal;
			if ( funcReg.getJlsRegVarNormal(strVal, strName) ){	// 変数値
				makeArgMemStoreLocalSet(state, strName, strVal);
			}else{
				globalState.addMsgErrorN("error: not defined variable by -arg.");
				success = false;
			}
		}
	}
	return success;
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
					globalState.addMsgErrorN("error: too many EndIf.");
					success = false;
				}
			}
			break;
		case CmdType::Else:						// Else文
		case CmdType::ElsIf:					// ElsIf文
			{
				int errnum = state.ifElse(flagCond);
				if (errnum > 0){
					globalState.addMsgErrorN("error: not exist 'If' but exist 'Else/ElsIf' .");
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
bool JlsScript::setCmdCall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::Call:
			success = taskCmdCall(cmdarg.getStrArg(1), loop, false);	// fcall=false
			break;
		case CmdType::Fcall:
			success = taskCmdFcall(cmdarg, state, loop);
		default:
			break;
	}
	return success;
}
//--- Call/Fcall処理 ---
bool JlsScript::taskCmdCall(string strName, int loop, bool fcall){
	string strFileName;
	if ( fcall ){	// Fcallによる呼び出し
		strFileName = setFcallName(strName);	// Callと共通化のためFcall用認識子付加
	}
	else{			// 通常のCall呼び出し
		//--- Call命令用のPath設定、読み込みファイル名設定 ---
		bool flagFull = false;		// 入力はファイル名部分のみ
		makeFullPath(strFileName, strName, flagFull);
	}
	//--- Call実行処理 ---
	loop ++;
	if (loop < SIZE_CALL_LOOP){				// 再帰呼び出しは回数制限
		startCmdLoop(strFileName, loop);
	}
	else{
		// 無限呼び出しによるバッファオーバーフロー防止のため
		globalState.addMsgErrorN("error: many recursive call");
		return false;
	}
	return true;
}
//--- Fcall処理 ---
bool JlsScript::taskCmdFcall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop){
	bool success = true;

	//--- 引数を取得して、引数が同数か確認 ---
	vector<string> listMod;
	vector<string> listMem;
	int sizeMod = cmdarg.getListStrArgs(listMod);	// コマンド引数取得
	int sizeMem = -1;
	if ( sizeMod <= 0 ){
		success = false;
	}else if ( state.getMemDefArg(listMem, listMod[0]) ){	// Memory保持内容取得
		sizeMem = (int) listMem.size();
	}
	if ( sizeMod != sizeMem ){		// 引数の数が不一致
		success = false;
	}
	string msgErrItem;
	if ( !success ){
		msgErrItem = "(argnum) " + to_string(sizeMod-2) + " defined: " + to_string(sizeMem-2);
	}
	//--- 関数名からの設定 ---
	string strFuncName;	// 関数名
	if ( success ){
		strFuncName = listMod[0];
		funcReg.setArgFuncName(strFuncName);	// 関数名を返り値に確保する設定
		if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){		// 返り値設定
			string strOut = cmdarg.getStrOpt(OptType::StrRegOut);
			bool flagLocal = cmdarg.getOptFlag(OptType::FlagLocal);
			bool overwrite = true;
			funcReg.setJlsRegVarWithLocal(strOut, "", overwrite, flagLocal);	// 変数初期化
			funcReg.setArgRefReg(strFuncName, strOut);
		}
	}
	//--- 各引数の設定 ---
	if ( success && sizeMod >= 3 ){			// 最後の引数は空定義のため2以下は引数追加なし
		for(int i=1; i<sizeMod-1; i++){		// 最初のモジュール名と最後の空定義は除く
			if ( success ){
				string strName = listMem[i];	// メモリ内のFunction引数
				string strVal  = listMod[i];	// 呼び出し側の引数
				if ( strName.substr(0, 4) == "ref(" ){		// 参照渡し
					if ( strName.substr(strName.length()-1) == ")" ){
						strName = strName.substr(4, strName.length()-5);
						success = funcReg.setArgRefReg(strName, strVal);	// 参照渡し
					}else{
						success = false;
						msgErrItem += " ref:" + strVal + " ";
					}
				}else{
					success = funcReg.setArgRegByBoth(strName, strVal, true); // quote展開=true
				}
			}
		}
	}
	//--- 実行処理 ---
	if ( success ){
		success = taskCmdCall(strFuncName, loop, true);	// fcall=true
	}else{
		string msg = "error: not match argument for function call.  ";
		msg += msgErrItem;
		globalState.addMsgErrorN(msg);
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
					globalState.addMsgErrorN("error: overflow at Repeat");
					success = false;
				}
				else if ( cmdarg.isSetStrOpt(OptType::StrCounter) ){	// -counter
					string strName = cmdarg.getStrOpt(OptType::StrCounter);
					int valI = cmdarg.getOpt(OptType::NumCounterI);
					int valS = cmdarg.getOpt(OptType::NumCounterS);
					state.repeatVarSet(strName, valS);
					funcReg.setJlsRegVarLocal(strName, to_string(valI), true);	// overwrite
				}
			}
			break;
		case CmdType::EndRepeat:			// EndRepeat文
			{
				{
					string strName;
					int valS;
					if ( state.repeatVarGet(strName, valS) ){	// -counter更新
						funcReg.setJlsRegVarCountUp(strName, valS, true);	// local=true
					}
				}
				int errum = state.repeatEnd();
				if (errum > 0){
					globalState.addMsgErrorN("error: Repeat - EndRepeat not match");
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
		case CmdType::Break:
			if ( cmdarg.getCondFlag() ){
				state.setBreak();
			}
			break;
		case CmdType::LocalSt:
			globalState.setLocalRegCreateOne();
			break;
		case CmdType::LocalEd:
			globalState.setLocalRegReleaseOne();
			break;
		case CmdType::ArgBegin:
			state.setArgAreaEnter(true);
			if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -localオプション
				globalState.setLocalOnly(true);
			}
			break;
		case CmdType::ArgEnd:
			state.setArgAreaEnter(false);
			globalState.setLocalOnly(false);
			break;
		case CmdType::Exit:
			globalState.setCmdExit(true);
			break;
		case CmdType::Return:
			state.setCmdReturn(true);
			break;
		case CmdType::EndMulti:	// 自動判別失敗
			globalState.addMsgErrorN("error:not match End* for ;");
			success = false;
			break;
		default:
			globalState.addMsgErrorN("error:internal setting");
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
		case CmdType::Mkdir:
			{
				string strName = cmdarg.getStrArg(1);
				if ( !LSys.cmdMkdir(strName) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error: failed mkdir " + strName);
					}
				}
			}
			break;
		case CmdType::FileCopy:
			{
				string strFrom = cmdarg.getStrArg(1);
				string strTo   = cmdarg.getStrArg(2);
				if ( !LSys.cmdCopy(strFrom, strTo) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error: failed copy from:" + strFrom + " to:" + strTo);
					}
				}
			}
			break;
		case CmdType::FileOpen:
		case CmdType::FileAppend:
			{
				if ( cmdarg.isSetStrOpt(OptType::StrFileCode) ){	// 文字コード設定
					string strCode = cmdarg.getStrOpt(OptType::StrFileCode);
					if ( !globalState.fileSetCodeNum(strCode) ){
						globalState.addMsgErrorN("error : -filecode(" + strCode + ")");
					}
				}
				string strName = cmdarg.getStrArg(1);
				bool append = ( cmdarg.cmdsel == CmdType::FileAppend );
				if ( !globalState.fileOpen(strName, append) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error : file open(" + strName + ")");
					}
				}
				funcReg.setSystemRegFileOpen();
			}
			break;
		case CmdType::FileClose:
			globalState.fileClose();
			funcReg.setSystemRegFileOpen();
			break;
		case CmdType::FileCode:
			{
				string strCode = cmdarg.getStrArg(1);
				if ( !globalState.fileSetCodeDefault(strCode) ){
					globalState.addMsgErrorN("error : FileCode(" + strCode + ")");
				}
			}
			break;
		case CmdType::FileToMemo:
			{
				int val = cmdarg.getValStrArg(1);
				globalState.fileMemoOnly( (val != 0) );
			}
			break;
		case CmdType::Echo:
			globalState.fileOutput(cmdarg.getStrArg(1) + "\n");
			break;
		case CmdType::EchoItem:
		case CmdType::EchoItemQ:
			{
				string strVal = cmdarg.getStrArg(1);
				if ( cmdarg.getOptFlag(OptType::FlagRestore) ){	// -restoreオプション
					funcReg.restoreStrQuote(strVal);
				}
				if ( !cmdarg.getOptFlag(OptType::FlagMerge) ){
					strVal += "\n";
				}
				globalState.fileOutput(strVal);
			}
			break;
		case CmdType::EchoFile:
			setCmdEchoTextFile(cmdarg.getStrArg(1));
			break;
		case CmdType::EchoOavs:
			setCmdEchoResultTrim();
			break;
		case CmdType::EchoOscp:
			setCmdEchoResultDetail();
			break;
		case CmdType::EchoMemo:
			globalState.fileMemoFlush();
			break;
		case CmdType::LogoOff:
			funcReg.setSystemRegNologo(false);
			break;
		case CmdType::OldAdjust:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.oldAdjust = val;
			}
			break;
		case CmdType::IgnoreCase:
			{
				int val = cmdarg.getValStrArg(1);
				globalState.setIgnoreCase( (val != 0) );
			}
			break;
		case CmdType::SysMesDisp:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.dispSysMes = val;
			}
			break;
		case CmdType::SysMemoSel:
			{
				int val = cmdarg.getValStrArg(1);
				LSys.setMemoSel(val);
			}
			break;
		case CmdType::SysDataGet:
			funcReg.getSystemData(cmdarg, cmdarg.getStrArg(1));
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
		default:
			globalState.addMsgErrorN("error:internal setting");
			success = false;
			break;
	}
	return success;
}
//--- テキストファイル内容をそのまま出力 ---
void JlsScript::setCmdEchoTextFile(const string& fname){
	LocalIfs ifs(fname.c_str());
	if ( !ifs.is_open() ){
		globalState.addMsgErrorN("error:not exist the filename:" + fname);
		return;
	}
	string strBuf;
	while( ifs.getline(strBuf) ){
		globalState.fileOutput(strBuf + "\n");
	}
}
//--- Trim出力（JlsIf内outputResultTrimの内容）---
void JlsScript::setCmdEchoResultTrim(){
	//--- 結果作成 ---
	pdata->outputResultTrimGen();
	string strBuf;
	int num_data = (int) pdata->resultTrim.size();
	for(int i=0; i<num_data-1; i+=2){
		if (i > 0){
			strBuf += " ++ ";
		}
		int frm_st = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i] );
		int frm_ed = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i+1] );
		strBuf += "Trim(" + to_string(frm_st) + "," + to_string(frm_ed) + ")";
	}
	strBuf += "\n";
	globalState.fileOutput(strBuf);
}
//--- 詳細情報結果出力（JlsIf内outputResultDetailの内容）---
void JlsScript::setCmdEchoResultDetail(){
	//--- 初期化 ---
	pdata->outputResultDetailReset();

	//--- データ読み込み・出力 ---
	string strBuf;
	while( pdata->outputResultDetailGetLine(strBuf) == 0){
		globalState.fileOutput(strBuf + "\n");
	}
}

//---------------------------------------------------------------------
// Read関連コマンド処理
//---------------------------------------------------------------------
bool JlsScript::setCmdRead(JlsCmdArg& cmdarg){
	bool success = true;
	bool valid = true;

	switch ( cmdarg.cmdsel ){
		case CmdType::ReadData:
			valid = funcReg.readDataList(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadTrim:
			valid = funcReg.readDataTrim(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadString:
			valid = funcReg.readDataString(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadCheck:
			valid = funcReg.readDataCheck(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadOpen:
			valid = funcReg.readGlobalOpen(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadClose:
			funcReg.readGlobalClose(cmdarg);
			break;
		case CmdType::ReadLine:
			valid = funcReg.readGlobalLine(cmdarg);
			break;
		case CmdType::EnvGet:
			valid = funcReg.readDataEnvGet(cmdarg, cmdarg.getStrArg(1));
			break;
		default:
			globalState.addMsgErrorN("error:internal setting");
			success = false;
			break;
	}
	funcReg.setSystemRegReadValid(valid);	// READVALID変数設定
	return success;
}

//---------------------------------------------------------------------
// レジスタ設定関連処理
//---------------------------------------------------------------------
bool JlsScript::setCmdReg(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	//--- 引数設定処理だった場合は実行して完了 ---
	if ( setArgAreaDefault(cmdarg, state) ){
		return true;
	}

	switch( cmdarg.cmdsel ){
		case CmdType::SetReg:
		case CmdType::Default:
		case CmdType::Set:
		case CmdType::EvalFrame:
		case CmdType::EvalTime:
		case CmdType::EvalNum:
		case CmdType::LocalSet:
		case CmdType::LocalSetF:
		case CmdType::LocalSetT:
		case CmdType::LocalSetN:
		case CmdType::SetList:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::Default )? false : true;
				bool flagLocal = ( cmdarg.cmdsel == CmdType::LocalSet )? true : false;
				if ( cmdarg.cmdsel == CmdType::LocalSetF ||
				     cmdarg.cmdsel == CmdType::LocalSetT ||
				     cmdarg.cmdsel == CmdType::LocalSetN ){
					flagLocal = true;
				}
				if ( cmdarg.getOptFlag(OptType::FlagLocal) ){	// -localオプション
					flagLocal = true;
				}
				if ( cmdarg.getOptFlag(OptType::FlagDefault) ){	// -defaultオプション
					overwrite = false;
				}
				string strVal = cmdarg.getStrArg(2);
				if ( cmdarg.cmdsel == CmdType::SetReg ){	// SetRegは変数値を取得
					string strName = strVal;
					if ( !funcReg.getJlsRegVarNormal(strVal, strName) ){
						globalState.addMsgErrorN("error: not found the registrer name(" + strName + ")");
					}
				}else if ( cmdarg.cmdsel == CmdType::SetList ){	// SetListはcsv認識する
					strVal = funcReg.getStrRegListByCsvStr(strVal);
				}
				if ( cmdarg.getOptFlag(OptType::FlagRestore) ){	// -restoreオプション
					funcReg.restoreStrQuote(strVal);
				}
				success = funcReg.setJlsRegVarWithLocal(cmdarg.getStrArg(1), strVal, overwrite, flagLocal);
			}
			break;
		case CmdType::Unset:
			{
				bool flagLocal = false;
				if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -localオプション
					flagLocal = true;
				}
				success = funcReg.unsetJlsRegVar(cmdarg.getStrArg(1), flagLocal);
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
				funcReg.setJlsRegVarWithLocal(cmdarg.getStrArg(1), "0", false, flagLocal);	// 未設定時に0書き込み
				int step = 1;
				if ( cmdarg.isSetOpt(OptType::NumStep) ){
					step = cmdarg.getOpt(OptType::NumStep);
				}
				success = funcReg.setJlsRegVarCountUp(cmdarg.getStrArg(1), step, flagLocal);
			}
			break;
		case CmdType::OptSet:
		case CmdType::OptDefault:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::OptDefault )? false : true;
				funcReg.setSystemRegOptions(cmdarg.getStrArg(1), 0, overwrite);
			}
			break;
		case CmdType::UnitSec:					// 特定レジスタ設定
			{
				pdata->cnv.changeUnitSec(cmdarg.getValStrArg(1));
				funcReg.setSystemRegUpdate();
			}
			break;
		case CmdType::ArgSet:
			success = funcReg.setArgRegByVal(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::ArgSetReg:
			success = funcReg.setArgRegByName(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::ListGetAt:
			funcReg.setResultRegListGetAt(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListIns:
			funcReg.setResultRegListIns(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListDel:
			funcReg.setResultRegListDel(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListJoin:
			funcReg.setResultRegListJoin(cmdarg);
			break;
		case CmdType::ListRemove:
			funcReg.setResultRegListRemove(cmdarg);
			break;
		case CmdType::ListSel:
			funcReg.setResultRegListSel(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ListSetAt:
			funcReg.setResultRegListRep(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListClear:
			funcReg.setResultRegListClear(cmdarg);
			break;
		case CmdType::ListDim:
			funcReg.setResultRegListDim(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListSort:
			funcReg.setResultRegListSort(cmdarg);
			break;
		case CmdType::SplitCsv:
			funcReg.setStrRegListByCsv(cmdarg);
			break;
		case CmdType::SplitItem:
			funcReg.setStrRegListBySpc(cmdarg);
			break;
		default:
			globalState.addMsgErrorN("error:internal setting(RegCmd)");
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
				makeArgMemStore(cmdarg, state);			// 保管型引数変数の挿入内容を決定する処理
			}
			break;
		case CmdType::EndLazy:				// EndLazy文
			if ( state.isLazyArea() == false ){
				globalState.addMsgErrorN("warning:exist no effect EndLazy");
			}
			state.setLazyStartType(LazyType::None);	// lazy解除
			break;
		case CmdType::Memory:				// Memory文
		case CmdType::MemSet:				// MemSet文
			{
				string strMemName = cmdarg.getStrArg(1);
				state.startMemArea(strMemName);
				//--- 引数定義の設定 ---
				makeArgMemStore(cmdarg, state);	// 保管型引数変数の挿入内容を決定する処理
				if ( cmdarg.isSetOpt(OptType::NumOrder) ){	// 実行順位を設定
					state.setMemOrder(cmdarg.getOpt(OptType::NumOrder));
				}
				vector<string> listMod;
				cmdarg.getListStrArgs(listMod);
				bool valid = state.setMemDefArg(listMod);	// 引数なしで識別子使用に設定
				if ( valid ){
					if ( cmdarg.cmdsel == CmdType::MemSet ){
						state.setMemUnusedFlag(strMemName);		// MemSetでは未使用状態に設定
					}
				}else{
					globalState.addMsgErrorN("error:already defined the name as Functiont");
				}
			}
			break;
		case CmdType::Function:				// Function文
			{
				string strMemName = cmdarg.getStrArg(1);
				state.startMemArea(strMemName);
				//--- 引数定義の設定 ---
				vector<string> listMod;
				cmdarg.getListStrArgs(listMod);
				if ( !state.setMemDefArg(listMod) ){	// 関数型引数の定義を保存
					globalState.addMsgErrorN("error:new argument insertion");
				}
			}
			break;
		case CmdType::EndMemory:			// EndMemory文
		case CmdType::EndFunc:				// EndFunc文
			if ( state.isMemArea() == false ){
				globalState.addMsgErrorN("warning:exist no effect EndMemory");
			}
			state.endMemArea();
			break;
		case CmdType::ExpandOn:
			state.setMemExpand(true);
			break;
		case CmdType::ExpandOff:
			state.setMemExpand(false);
			break;
		default:
			globalState.addMsgErrorN("error:internal setting(LazyCategory)");
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
			globalState.addMsgErrorN("error:internal setting(MemCategory)");
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
		if ( funcReg.isSystemRegLastexe() ){	// 直前コマンドを実行した場合
			valid_exe = false;			// 今回コマンドは実行しないが
			exe_command = true;			// 実行済み扱い
		}
	}
	//--- 前コマンド実行済みか確認 (-cont option) ---
	if (cmdset.arg.getOpt(OptType::FlagCont) > 0){
		if ( funcReg.isSystemRegLastexe() == false ){	// 直前コマンドを実行していない場合
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
			funcReg.setSystemRegUpdate();		// NOLOGO更新
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
	//--- リスト作成時は開始前に内容消去 ---
	if ( cmdset.arg.cmdsel == CmdType::GetList ||
	     cmdset.arg.cmdsel == CmdType::GetPos ){
		funcReg.updateResultRegWrite(cmdset.arg);	// 変数($POSHOLD/$LISTHOLD)クリア
	}
	//--- -pickin/-pick選別。-pickオプション使用時は実行箇所が全部揃ってから実行する ---
	bool picki = cmdset.arg.tack.pickIn;
	bool picko = cmdset.arg.tack.pickOut;
	cmdset.limit.clearPickList();		// 念のため初期化
	//--- 各有効ロゴを実行 ---
	bool exeflag_total = false;
	for(int i=0; i<nmax_list; i++){
			//--- 制約条件を満たしているロゴか確認 ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			if ( exeflag ){
				 exeflag = exeCmdLogoIsValidExe(cmdset);	// -pick用に事前実行判定
			}
			if (exeflag){
				if ( picko ){
					cmdset.limit.addPickListCurrent();	// 後で判断するためここでは格納
				}else{
					if ( picki ){
						string strArgPick = cmdset.arg.getStrOpt(OptType::ListPickIn);	// -pickin
						exeflag = pdata->cnv.isStrMultiNumIn(strArgPick, i+1, nmax_list);	// 先頭=1
					}
					if ( exeflag ){
						exeflag = exeCmdLogoFromCat(cmdset);	// 実際の実行
					}
				}
			}
			//--- 実行していたら実行フラグ設定 ---
			if (exeflag){
				exeflag_total = true;
			}
	}
	//--- -pickオプション使用時の後から実行 ---
	if ( picko ){
		exeflag_total = false;	// やり直し
		int sizeListExe = cmdset.limit.sizePickList();		// 実行条件を満たした全体数
		for(int i=1; i<=sizeListExe; i++){
			string strArgPick = cmdset.arg.getStrOpt(OptType::ListPickOut);	// -pick
			if ( pdata->cnv.isStrMultiNumIn(strArgPick, i, sizeListExe) ){
				cmdset.limit.selectPickList(i);		// 使用する結果を選択
				bool exeflag = exeCmdLogoFromCat(cmdset);	// 実際の実行
				//--- 実行していたら実行フラグ設定 ---
				if (exeflag){
					exeflag_total = true;
				}
			}
		}
		cmdset.limit.clearPickList();		// 念のため初期化
	}
	return exeflag_total;
}
//--- -pickコマンドのために事前実行判定 ---
bool JlsScript::exeCmdLogoIsValidExe(JlsCmdSet& cmdset){
	//--- 結果位置／終了位置取得 ---
	TargetLocInfo tgDst = cmdset.limit.getResultDst();
	TargetLocInfo tgEnd = cmdset.limit.getResultEnd();

	bool valid = true;
	switch(cmdset.arg.cmdsel){
		case CmdType::GetPos:
		case CmdType::GetList:
			valid = (( tgDst.valid || (tgDst.tp == TargetScpType::Direct)) &&
			         ( tgEnd.valid || (tgEnd.tp == TargetScpType::Direct)));
			break;
		case CmdType::AutoIns:
		case CmdType::AutoDel:
			valid = ( tgDst.valid && tgEnd.valid );
			break;
		default:
			break;
	}
	return valid;
}
//--- カテゴリ別に分岐する実行 ---
bool JlsScript::exeCmdLogoFromCat(JlsCmdSet& cmdset){
	bool exeflag = true;
	//--- 実行分岐 ---
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
	return exeflag;
}
//---------------------------------------------------------------------
// ロゴ位置別の実行コマンド
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogoTarget(JlsCmdSet& cmdset){
	bool exe_command = false;
	Nsc nsc_scpos_sel = cmdset.limit.getResultDstNsc();
	Nrf nrf = cmdset.limit.getLogoBaseNrf();

	switch(cmdset.arg.cmdsel){
		case CmdType::Find:
			{
				Msec msec;
				if ( getMsecTargetDst(msec, cmdset, false) ){
					if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){	// 推測構成に反映
						exeCmdCallAutoScript(cmdset);					// Auto処理クラス呼び出し
					}
					else{								// 従来構成に反映
						pdata->setResultLogoAtNrf(msec, LOGO_RESULT_DECIDE, nrf);
					}
				}
				exe_command = true;
			}
			break;
		case CmdType::MkLogo:
			{
				RangeMsec rmsec;
				if ( getRangeMsecTarget(rmsec, cmdset) ){
					bool overlap = ( cmdset.arg.getOpt(OptType::FlagOverlap) != 0 )? true : false;
					bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
					bool unit    = ( cmdset.arg.getOpt(OptType::FlagUnit)    != 0 )? true : false;
					int nsc_ins = pdata->insertLogo(rmsec.st, rmsec.ed, overlap, confirm, unit);
					exe_command = (nsc_ins >= 0)? true : false;
				}
			}
			break;
		case CmdType::DivLogo:
			{
				Msec msec;
				if ( getMsecTargetDst(msec, cmdset, false) ){
					bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
					bool unit    = true;
					LogoEdgeType edge = cmdset.limit.getResultDstEdge();
					Nsc nsc_ins = pdata->insertDivLogo(msec, confirm, unit, edge);
					exe_command = (nsc_ins >= 0)? true : false;
				}
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
			exe_command = true;  // exeCmdLogoIsValidExe(cmdset);  // 実施済み
			if ( exe_command ){
				Msec msecDst;
				Msec msecEnd;
				getMsecTargetDst(msecDst, cmdset, true);	// allow forceSCP
				getMsecTargetEnd(msecEnd, cmdset, true);
				if ( cmdset.arg.getOptFlag(OptType::FlagHoldEnd) ){	// -HoldE
					funcReg.setResultRegPoshold(cmdset.arg, msecEnd);		// 変数に設定
				}else{
					funcReg.setResultRegPoshold(cmdset.arg, msecDst);		// 変数に設定
				}
			}
			break;
		case CmdType::GetList:
			exe_command = true;  // exeCmdLogoIsValidExe(cmdset);  // 実施済み
			if ( exe_command ){
				Msec msecDst;
				Msec msecEnd;
				getMsecTargetDst(msecDst, cmdset, true);	// allow forceSCP
				getMsecTargetEnd(msecEnd, cmdset, true);
				if ( cmdset.arg.getOptFlag(OptType::FlagHoldBoth) ){	// -HoldB
					Msec msec1 = ( msecDst <= msecEnd )? msecDst : msecEnd;
					Msec msec2 = ( msecDst <= msecEnd )? msecEnd : msecDst;
					funcReg.setResultRegListhold(cmdset.arg, msec1);	// 変数に設定
					funcReg.setResultRegListhold(cmdset.arg, msec2);	// 変数に設定
				}else if ( cmdset.arg.getOptFlag(OptType::FlagHoldEnd) ){	// HoldE
					// END用変数
					funcReg.setResultRegListhold(cmdset.arg, msecEnd);		// 変数に設定
				}else{
					funcReg.setResultRegListhold(cmdset.arg, msecDst);		// 変数に設定
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
	if (funcReg.getJlsRegVarNormal(cstr, "TAILTIME") ){
		pdata->cnv.getStrValMsecM1(msec_headframe, cstr, 0);
	}
	//--- 範囲を取得 ---
	WideMsec wmsecBase;
	wmsecBase.just  = msec_headframe;	// 基準位置のみ設定してターゲット,forceは処理共通化
	wmsecBase.early = msec_headframe;
	wmsecBase.late  = msec_headframe;
	//--- 一番近いシーンチェンジ位置を取得 ---
	m_funcLimit->selectTargetByRange(cmdset, wmsecBase);

	//--- 結果を格納 --
	bool exeflag = false;
	Msec msec_tailframe;
	exeflag = getMsecTargetDst(msec_tailframe, cmdset, true);	// allow forceSCP
	if ( exeflag ){
		funcReg.setSystemRegHeadtail(msec_headframe, msec_tailframe);
		//--- -autochgオプション対応 ---
		if ( cmdset.arg.getOpt(OptType::FlagAutoChg) > 0 ){		// 推測構成に反映
			exeCmdCallAutoScript(cmdset);						// Auto処理クラス呼び出し
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// cmdset情報から対象となる無音シーンチェンジを取得。なければ設定によっては強制作成
// 出力
//  返り値     ：結果取得有無
//  msecResult : 取得した位置
//---------------------------------------------------------------------
bool JlsScript::getMsecTargetDst(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp){
	return getMsecTargetSub(msecResult, cmdset, allowForceScp, false);
}
bool JlsScript::getMsecTargetEnd(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp){
	return getMsecTargetSub(msecResult, cmdset, allowForceScp, true);
}
bool JlsScript::getMsecTargetSub(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp, bool flagEnd){
	TargetLocInfo tgLoc;
	if ( flagEnd ){
		tgLoc = cmdset.limit.getResultEnd();
	}else{
		tgLoc = cmdset.limit.getResultDst();
	}
	msecResult = tgLoc.msout;

	bool success = ( msecResult >= 0 );
	if ( tgLoc.tp == TargetScpType::Force ){
		if ( msecResult < 0 || msecResult > pdata->getMsecTotalMax() ){
			success = false;
		}
		//--- forceによる無音シーンチェンジ作成 ---
		else if ( allowForceScp ){
			if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){	// -forceオプションで強制作成時
				pdata->getNscForceMsecExact(tgLoc.msout, tgLoc.edge, tgLoc.exact);
			}
		}
	}
	return success;
}
//---------------------------------------------------------------------
// cmdset情報から対象範囲となる無音シーンチェンジ２点を取得
// 出力
//  返り値     ：結果取得有無
//  rmsecResult : 取得した範囲位置
//---------------------------------------------------------------------
bool JlsScript::getRangeMsecTarget(RangeMsec& rmsecResult, JlsCmdSet& cmdset){
	TargetLocInfo tgDst = cmdset.limit.getResultDst();
	TargetLocInfo tgEnd = cmdset.limit.getResultEnd();
	Msec msecDst = tgDst.msout;
	Msec msecEnd = tgEnd.msout;

	if ( msecDst < msecEnd ){
		rmsecResult.st = msecDst;
		rmsecResult.ed = msecEnd;
	}else{
		rmsecResult.st = msecEnd;
		rmsecResult.ed = msecDst;
	}
	return ( rmsecResult.st >= 0 && rmsecResult.ed >= 0 );
}

