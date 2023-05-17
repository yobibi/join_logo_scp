//
// 実行スクリプトコマンド文字列解析
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptDecode.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"

///////////////////////////////////////////////////////////////////////
//
// 実行スクリプトコマンド文字列解析クラス
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// 初期化
//  pdataは文字列・時間変換機能(cnv)のみ使われる
//---------------------------------------------------------------------
JlsScriptDecode::JlsScriptDecode(JlsDataset *pdata, JlsScrFuncList* pFuncList){
	this->pdata  = pdata;
	this->pFuncList = pFuncList;
}

//---------------------------------------------------------------------
// 内部設定の異常確認
//---------------------------------------------------------------------
void JlsScriptDecode::checkInitial(){
	int SIZE_JLCMD_SEL = static_cast<int>(jlscmd::CmdType::MAXSIZE);
	if ( (int)CmdDefine.size() != SIZE_JLCMD_SEL ){
		castErrInternal("mismatch at CmdDefine size from CmdType size");
	}
	if ( (int)ConfigDefine.size() != SIZE_CONFIG_VAR ){
		castErrInternal("mismatch at ConfigDefine size from SIZE_CONFIG_VAR");
	}
	for(int i=0; i<(int)CmdDefine.size(); i++){
		if ( CmdDefine[i].cmdsel != (CmdType) i ){
			castErrInternal("error:internal mismatch at CmdDefine.cmdsel " + to_string(i));
		}
	}
	for(int i=0; i<(int)ConfigDefine.size(); i++){
		if ( ConfigDefine[i].prmsel != (ConfigVarType) i ){
			castErrInternal("error:internal mismatch at ConfigDefine.prmsel " + to_string(i));
		}
	}
}

//=====================================================================
// デコード処理
//=====================================================================

//---------------------------------------------------------------------
// コマンド内容を文字列１行から解析
// 入力：
//  strBuf  : 解析文字列
//  onlyCmd : 先頭のコマンド部分だけの解析か（false=全体、true=コマンドのみ）
// 出力：
//   返り値：エラー状態
//   cmdarg: コマンド解析結果
//---------------------------------------------------------------------
CmdErrType JlsScriptDecode::decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd){
	CmdErrType retval = CmdErrType::None;

	//--- コマンド内容初期化 ---
	cmdarg.clear();
	//--- コマンド受付 ---
	JlscrCmdRecord cmddef;
	int pos = decodeCmdName(cmddef, retval, strBuf);
	if ( retval != CmdErrType::None ){
		return retval;		// コマンド異常時の終了
	}
	//--- コマンド格納 ---
	cmdarg.cmdsel   = cmddef.cmdsel;
	cmdarg.category = cmddef.category;
	//--- コマンド受付のみで終了する場合 ---
	if ( onlyCmd ){
		return retval;
	}
	//--- コメント除去 ---
	string strNewBuf;
	if ( cmddef.muststr == 9 ){			// 無条件全部読みの時は残す
		strNewBuf = strBuf;
	}else{
		pdata->cnv.getStrWithoutComment(strNewBuf, strBuf);
		if ( pos >= (int)strNewBuf.length() ){
			pos = -1;	// データなし
		}
	}
	//--- コマンド解析 ---
	if ( cmddef.muststr > 0 || cmddef.mustchar > 0 || cmddef.mustrange > 0 ){
		pos = decodeCmdArgMust(cmdarg, retval, strNewBuf, pos, cmddef);
	}
	//--- オプション受付 ---
	if (cmddef.needopt > 0 && pos >= 0){
		pos = decodeCmdArgOpt(cmdarg, retval, strNewBuf, pos);
	}
	//--- 引数を演算加工 ---
	if ( cmddef.muststr > 0 ){
		bool success = calcCmdArg(cmdarg);
		if ( success == false ){
			setErrItem("mismatch fix argument type - evaluation failed");
			retval = CmdErrType::ErrOpt;
		}
	}
	return retval;
}

//---------------------------------------------------------------------
// 文字列からコマンド名を取得
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdName(JlscrCmdRecord& cmddef, CmdErrType& errval, const string& strBuf){
	//--- コマンド受付(cmdsel) ---
	string strCmd;
	int pos = pdata->cnv.getStrItemCmd(strCmd, strBuf, 0);
	int csel = ( pos >= 0 )? decodeCmdNameId(strCmd) : 0;
	if (csel < 0){
		setErrItem(strCmd);
		errval = CmdErrType::ErrCmd;
		return -1;
	}
	//--- コマンド格納 ---
	cmddef = CmdDefine[csel];
	return pos;
}
//---------------------------------------------------------------------
// コマンド名をリストから番号で取得
// 出力：
//   返り値  ：取得コマンド番号（失敗時は-1）
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdNameId(const string& cstr){
	int det = -1;
	const char *cmdname = cstr.c_str();

	if (cmdname[0] == '\0' || cmdname[0] == '#'){
		det = 0;
	}
	else{
		for(int i=0; i<(int)CmdDefine.size(); i++){
			if (_stricmp(cmdname, CmdDefine[i].cmdname.c_str()) == 0){
				det = i;
				break;
			}
		}
		//--- 見つからなければ別名を検索 ---
		if (det < 0){
			bool flag = false;
			CmdType target;
			for(int i=0; i<(int)CmdAlias.size(); i++){
				if (_stricmp(cmdname, CmdAlias[i].cmdname.c_str()) == 0){
					target = CmdAlias[i].cmdsel;
					flag = true;
					break;
				}
			}
			if ( flag ){
				for(int i=0; i<(int)CmdDefine.size(); i++){
					if ( CmdDefine[i].cmdsel == target ){
						det = i;
						break;
					}
				}
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// 必須引数の取得
// 入力：
//   strBuf : 文字列
//   pos    : 認識開始位置
//   cmddef
//   muststr  : 文字列引数（0-3=取得数 9=残り全体 8=条件つき残り全体 7=関数型引数）
//   mustchar : 種類設定（0=設定なし  1=S/E/B  2=TR/SP/EC 3=省略可能なS/E/B） tps>0時は連続quoteを残す処理に使用
//   mustrange: 期間設定（0=設定なし  1=center  3=center+left+right）
// 出力：
//   返り値  : 読み込み位置（-1=オプション異常）
//   errval  : エラー番号
//   cmdarg  : コマンド解析結果
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, const JlscrCmdRecord& cmddef){
	//--- 文字列として引数を取得 ---
	if ( cmddef.muststr > 0 && pos >= 0){
		if ( cmddef.muststr == 9 ){
			//--- 残り全部を文字列として取得 ---
			while( strBuf[pos] == ' ' && pos >= 0) pos++;
			string strArg = strBuf.substr(pos);
			cmdarg.addArgString(strArg);
			pos = -1;
		}else if ( cmddef.muststr == 7 ){
			//--- モジュール名と引数を取得 ---
			if ( pdata->cnv.isStrFuncModule(strBuf, pos) ){
				vector<string> listMod;
				pos = pdata->cnv.getListModuleArg(listMod, strBuf, pos);
				if ( pos >= 0 ){
					for(int i=0; i<(int)listMod.size(); i++){
						cmdarg.addArgString(listMod[i]);
					}
				}else{
					setErrItem("not module format");
					errval = CmdErrType::ErrOpt;
				}
			}else{
				pos = -1;
				setErrItem("not module format");
				errval = CmdErrType::ErrOpt;
			}
		}else{
			int sizeArg = cmddef.muststr;
			for(int i=0; i<sizeArg; i++){
				string strArg;
				if ( cmddef.mustchar == 9 ){
					pos = pdata->cnv.getStrItemMonitor(strArg, strBuf, pos);
				}else{
					pos = pdata->cnv.getStrItemArg(strArg, strBuf, pos);
				}
				if ( pos >= 0 ){
					cmdarg.addArgString(strArg);
				}
			}
			if (pos < 0){
				setErrItem("need argSize:" + to_string(sizeArg));
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- 種類文字 ---
	if (cmddef.mustchar > 0 && cmddef.muststr == 0 && pos >= 0){
		string strTmp;
		int posbak = pos;
		pos = pdata->cnv.getStrItemArg(strTmp, strBuf, pos);
		if (pos >= 0){
			//--- 項目１（文字指定） ---
			if (cmddef.mustchar == 1 || cmddef.mustchar == 3){
				if (strTmp[0] == 'S' || strTmp[0] == 's'){
					cmdarg.selectEdge = LOGO_EDGE_RISE;
				}
				else if (strTmp[0] == 'E' || strTmp[0] == 'e'){
					cmdarg.selectEdge = LOGO_EDGE_FALL;
				}
				else if (strTmp[0] == 'B' || strTmp[0] == 'b'){
					cmdarg.selectEdge = LOGO_EDGE_BOTH;
				}
				else{
					if ( cmddef.mustchar == 1 || cmddef.mustchar == 3 ){
						pos = posbak;
						cmdarg.selectEdge = LOGO_EDGE_RISE;
					}
					else{
						pos    = -1;
						errval = CmdErrType::ErrSEB;
					}
				}
			}
			//--- TR/SP/EC 文字列判別 ---
			else if (cmddef.mustchar == 2){
				bool flagOption = false;
				if ( getTrSpEcID(cmdarg.selectAutoSub, strTmp, flagOption) == false ){
					pos    = -1;
					errval = CmdErrType::ErrTR;
				}
			}
		}
	}
	//--- 範囲指定 ---
	if (cmddef.mustrange > 0 && pos >= 0){
		if (cmddef.mustrange == 1 || cmddef.mustrange == 3){
			JlscrDecodeRangeRecord infoDec = {};
			if (cmddef.mustrange == 1){
				infoDec.numRead  = 1;		// データ読み込み数=1
				infoDec.needs    = 0;		// 最低読み込み数=0（全項目省略可）
				infoDec.numFrom  = 0;		// 省略時の開始指定なし（標準動作）
				infoDec.flagM1   = false;	// -1は通常の数値として読み込む
				infoDec.flagSort = false;	// １データなので並び替えなし
			}
			else if (cmddef.mustrange == 3){
				infoDec.numRead  = 3;		// データ読み込み数=3
				infoDec.needs    = 0;		// 最低読み込み数=0（全項目省略可）
				infoDec.numFrom  = 0;		// 省略時の開始指定なし（標準動作）
				infoDec.flagM1   = false;	// -1は通常の数値として読み込む
				infoDec.flagSort = true;	// 小さい順の並び替えあり
			}
			pos = decodeRangeMsec(infoDec, strBuf, pos);
			cmdarg.wmsecDst = infoDec.wmsecVal;
			if ( cmddef.mustrange == 3 && infoDec.numAbbr != 3 ){
				cmdarg.setOpt(OptType::FlagDstPoint, 1);	// Dst設定ありとする
			}
		}
		if (pos < 0){
			errval = CmdErrType::ErrRange;
		}
	}

	return pos;
}


//---------------------------------------------------------------------
// 引数オプションの取得
// バッファ残り部分から１設定を検索
// 出力：
//   返り値  : 読み込み位置（-1=オプション異常）
//   errval  : エラー番号
//   cmdarg  : コマンド解析結果
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgOpt(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos){
	m_listKeepSc.clear();		// -SC系のオプションデータを一時保持の初期化
	//--- 各引数取得 ---
	while(pos >= 0){
		pos = decodeCmdArgOptOne(cmdarg, errval, strBuf, pos);
	}
	reviseCmdRange(cmdarg);		// オプションによる範囲補正
	setCmdTackOpt(cmdarg);		// 実行オプション設定
	setArgScOpt(cmdarg);		// 一時保持した-SC系オプションを設定
	mirrorOptToUndef(cmdarg);	// 未指定のオプションに複写
	return pos;
}
//---------------------------------------------------------------------
// 引数オプションの取得
// バッファ残り部分から１設定を検索
// 出力：
//   返り値  : 読み込み位置（-1=オプション異常）
//   errval  : エラー番号
//   cmdarg  : コマンド解析結果
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgOptOne(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos){
	//--- 次のオプション読み込み ---
	string strWord;
	if (pos >= 0){
		pos = pdata->cnv.getStrItemArg(strWord, strBuf, pos);
	}
	//--- 通常の引数取得 ---
	int optsel = -1;
	if (pos >= 0){
		//--- オプション識別 ---
		const char *pstr = strWord.c_str();
		for(int i=0; i<(int)OptDefine.size(); i++){
			if (!_stricmp(pstr, OptDefine[i].optname.c_str())){
				optsel = i;
			}
		}
		if (optsel < 0){		// オプション対応文字列なし
			pos = -1;
		}
		if (pos >= 0){
			//--- 設定 ---
			pos = decodeCmdArgOptOneSub(cmdarg, optsel, strBuf, pos);
		}
		//--- 引数不足時のエラー ---
		if (pos < 0){
			setErrItem(strWord);
			errval = CmdErrType::ErrOpt;
		}
	}
	return pos;
}

//--- optselに対応したオプション情報を設定 ---
int JlsScriptDecode::decodeCmdArgOptOneSub(JlsCmdArg& cmdarg, int optsel, const string& strBuf, int pos){
	//--- optselの文字列に対応するコマンド情報を取得 ---
	OptType optType = OptDefine[optsel].optType;
	int optTypeInt = static_cast<int>(optType);
	OptCat category;
	if ( cmdarg.getOptCategory(category, optType) == false ){
		pos = -1;
		category = OptCat::None;
		castErrInternal("(OptDefine-category)" + strBuf);
		return pos;
	}
	//--- Msec取得用情報作成 ---
	JlscrDecodeRangeRecord infoDec = {};
	infoDec.numRead  = OptDefine[optsel].numArg;	// データ読み込み数
	infoDec.needs    = OptDefine[optsel].minArg;	// 最低読み込み数
	infoDec.numFrom  = OptDefine[optsel].numFrom;	// 省略時開始番号設定
	infoDec.flagM1   = false;			// -1は通常の数字として読み込む
	infoDec.flagSort = false;			// 小さい順の並び替えなし
	if ( OptDefine[optsel].convType == ConvStrType::MsecM1 ){
		infoDec.flagM1   = true;			// -1は変換しないで読み込む
	}
	if ( OptDefine[optsel].sort == 12 && infoDec.numRead == 2){
		infoDec.flagSort = true;	// 小さい順の並び替えあり
	}else if ( OptDefine[optsel].sort == 23 && infoDec.numRead == 3){
		infoDec.flagSort = true;	// 小さい順の並び替えあり
	}else if ( OptDefine[optsel].sort > 0 ){
		castErrInternal("(OptDefine-sort)" + strBuf);
	}
	//--- 設定 ---
	switch( category ){
		case OptCat::NumLG :					// ロゴ番号の限定
			if ( cmdarg.isSetOpt(OptType::TypeNumLogo) == false ){
				// 種類を設定
				cmdarg.setOpt(OptType::TypeNumLogo, optTypeInt);
				// 番号を設定
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				string strSub;
				pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
				if ( pos >= 0 ){
					vector<string> listStrNum;
					if ( getListStrNumFromStr(listStrNum, strSub) ){
						for(int i=0; i < (int)listStrNum.size(); i++){
							cmdarg.addLgOpt(listStrNum[i]);
						}
					}else{
						if ( posBak >= 0 ){
							cmdarg.addLgOpt("0");	// 省略設定
							pos = posBak;
						}else{
							pos = -1;			// 変換失敗
						}
					}
				}else if ( posBak >= 0 ){
					cmdarg.addLgOpt("0");	// 省略設定
					pos = posBak;
				}
			}
			break;
		case OptCat::FRAME :					// フレーム位置による限定
			{
				pos = decodeRangeMsec(infoDec, strBuf, pos);
				if ( pos >= 0 ){
					cmdarg.setOpt(OptType::MsecFrameL, infoDec.wmsecVal.early);
					cmdarg.setOpt(OptType::MsecFrameR, infoDec.wmsecVal.late);
					cmdarg.setOpt(OptType::TypeFrame, optTypeInt);
					cmdarg.setOpt(OptType::TypeFrameSub, OptDefine[optsel].subType);
				}
			}
			break;
		case OptCat::PosSC :					// 無音SCによる限定
			{
				pos = decodeRangeMsec(infoDec, strBuf, pos);
				if ( pos >= 0 ){
					JlscrDecodeKeepSc keepSc;
					keepSc.type     = optType;
					keepSc.subtype  = OptDefine[optsel].subType;
					keepSc.wmsec    = infoDec.wmsecVal;
					keepSc.abbr     = infoDec.numAbbr;
					m_listKeepSc.push_back(keepSc);		// 後で範囲補正が入るので一時保持
				}
			}
			break;
		case OptCat::STR :						// 文字列の設定（リスト変数も含む）
			{
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				string strSub;
				pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
				if ( pos >= 0 ){
					ConvStrType tp = OptDefine[optsel].convType;
					if ( tp == ConvStrType::None ){		// 変換ない場合
						if ( strSub[0] == '-' ){
							int tmpval;			// 次が'-'でも数値なら続行
							if ( pdata->cnv.getStrValNum(tmpval, strSub, 0) < 0 ){
								pos = -1;
							}
						}
					}else{
						if ( convertStringFromListStr(strSub, tp) == false ){
							if ( posBak >= 0 ){
								strSub = "0";		// 省略設定
								pos = posBak;
							}else{
								pos = -1;			// 変換失敗
							}
						}
					}
				}else if ( posBak >= 0 ){
					strSub = "0";
					pos = posBak;
				}
				if ( pos >= 0 ){
					cmdarg.setStrOpt(optType, strSub);
				}
				if ( OptDefine[optsel].numArg > 1 ){	// 2引数以上の追加処理
					pos = getOptionStrMulti(cmdarg, optType, strBuf, pos);
				}
			}
			break;
		case OptCat::NUM :						// 数値の設定
			{
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				vector<OptType> listOptType(4);
				int numUsed = getOptionTypeList(listOptType, optType, infoDec.numRead);
				vector<int> listVal(4);
				switch( OptDefine[optsel].convType ){
					case ConvStrType::MsecM1 :
						pos = decodeRangeMsec(infoDec, strBuf, pos);
						if ( pos >= 0 ){
							if ( infoDec.numRead == 3 ){
								listVal[0] = (int)infoDec.wmsecVal.just;
								listVal[1] = (int)infoDec.wmsecVal.early;
								listVal[2] = (int)infoDec.wmsecVal.late;
							}else if ( infoDec.numRead == 2 ){
								listVal[0] = (int)infoDec.wmsecVal.early;
								listVal[1] = (int)infoDec.wmsecVal.late;
							}else{
								listVal[0] = (int)infoDec.wmsecVal.just;
							}
							if ( numUsed > infoDec.numRead ){
								listVal[infoDec.numRead] = infoDec.numAbbr;
							}
						}
						break;
					case ConvStrType::Num :
						pos = pdata->cnv.getStrValNum(listVal[0], strBuf, pos);
						if ( pos < 0 && posBak >= 0 ){	// 省略時
							listVal[0] = OptDefine[optsel].numFrom;
							pos = posBak;
						}
						if ( infoDec.numRead > 1 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
					case ConvStrType::Sec :
						pos = pdata->cnv.getStrValSecFromSec(listVal[0], strBuf, pos);
						if ( infoDec.numRead > 1 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
					case ConvStrType::TrSpEc :
						{
							string strSub;
							pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
							if ( pos >= 0 ){
								CmdTrSpEcID idSub;
								bool flagOption = true;
								if ( getTrSpEcID(idSub, strSub, flagOption) ){
									listVal[0] = static_cast<int>(idSub);
								}else{
									pos = -1;
								}
							}
							if ( infoDec.numRead > 1 ){
								castErrInternal("(OptDefine-numArg)" + strBuf);
							}
						}
						break;
					default :	// for flag
						listVal[0] = 1;
						if ( OptDefine[optsel].subType > 0 ){
							listVal[0] = OptDefine[optsel].subType;
						}
						if ( infoDec.numRead > 0 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
				}
				for(int i=0; i < numUsed; i++){
					cmdarg.setOpt(listOptType[i], listVal[i]);
				}
			}
			break;
		default :
			castErrInternal("(OptDefine-category)" + strBuf);
			break;
	}
	return pos;
}
//--- 文字列取得で2引数以上必要とするケースの処理 ---
int JlsScriptDecode::getOptionStrMulti(JlsCmdArg& cmdarg, OptType optType, const string& strBuf, int pos){
	switch( optType ){
		case OptType::StrCounter :
			{
				bool exist1 = false;
				int val1;
				if ( pos >= 0 ){
					int posBak = pos;
					pos = pdata->cnv.getStrValNum(val1, strBuf, pos);
					if ( pos >= 0 ){
						exist1 = true;
					}else{
						pos = posBak;
					}
				}
				bool exist2 = false;
				int val2;
				if ( exist1 ){
					int posBak = pos;
					pos = pdata->cnv.getStrValNum(val2, strBuf, pos);
					if ( pos >= 0 ){
						exist2 = true;
					}else{
						pos = posBak;
					}
				}
				if ( !exist1 ) val1 = 0;	// 省略時設定
				if ( !exist2 ) val2 = 1;	// 省略時設定
				cmdarg.setOpt(OptType::NumCounterI, val1);		// ccounter initial
				cmdarg.setOpt(OptType::NumCounterS, val2);		// ccounter step
			}
			break;
		default :
			break;
	}
	return pos;
}
//--- オプションの格納先を取得 ---
int JlsScriptDecode::getOptionTypeList(vector<OptType>& listOptType, OptType orgOptType, int numArg){
	int numUsed = 0;
	//--- 格納先を取得 ---
	switch( orgOptType ){
		case OptType::MsecEndlenC :
			numUsed = 4;
			listOptType[0] = OptType::MsecEndlenC;
			listOptType[1] = OptType::MsecEndlenL;
			listOptType[2] = OptType::MsecEndlenR;
			listOptType[3] = OptType::AbbrEndlen;
			break;
		case OptType::MsecEndSftC :
			numUsed = 4;
			listOptType[0] = OptType::MsecEndSftC;
			listOptType[1] = OptType::MsecEndSftL;
			listOptType[2] = OptType::MsecEndSftR;
			listOptType[3] = OptType::AbbrEndSft;
			break;
		case OptType::MsecSftC :
			numUsed = 4;
			listOptType[0] = OptType::MsecSftC;
			listOptType[1] = OptType::MsecSftL;
			listOptType[2] = OptType::MsecSftR;
			listOptType[3] = OptType::AbbrSft;
			break;
		case OptType::MsecTgtLimL :
			numUsed = 2;
			listOptType[0] = OptType::MsecTgtLimL;
			listOptType[1] = OptType::MsecTgtLimR;
			break;
		case OptType::MsecLenPMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenPMin;
			listOptType[1] = OptType::MsecLenPMax;
			break;
		case OptType::MsecLenNMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenNMin;
			listOptType[1] = OptType::MsecLenNMax;
			break;
		case OptType::MsecLenPEMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenPEMin;
			listOptType[1] = OptType::MsecLenPEMax;
			break;
		case OptType::MsecLenNEMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenNEMin;
			listOptType[1] = OptType::MsecLenNEMax;
			break;
		case OptType::MsecFromHead :
			numUsed = 2;
			listOptType[0] = OptType::MsecFromHead;
			listOptType[1] = OptType::AbbrFromHead;
			break;
		case OptType::MsecFromTail :
			numUsed = 2;
			listOptType[0] = OptType::MsecFromTail;
			listOptType[1] = OptType::AbbrFromTail;
			break;
		case OptType::MsecLogoExtL :
			numUsed = 2;
			listOptType[0] = OptType::MsecLogoExtL;
			listOptType[1] = OptType::MsecLogoExtR;
			break;
		case OptType::MsecDrangeL :
			numUsed = 2;
			listOptType[0] = OptType::MsecDrangeL;
			listOptType[1] = OptType::MsecDrangeR;
			break;
		default :
			numUsed = 1;
			listOptType[0] = orgOptType;
			break;
	}
	if ( numUsed < numArg ){
		castErrInternal("(numArg) type:" + static_cast<int>(orgOptType));
	}
	return numUsed;
}

void JlsScriptDecode::castErrInternal(const string& msg){
	string mes = "error:internal setting" + msg;
	lcerr << mes << endl;
}


//---------------------------------------------------------------------
// TR/SP/EC文字列の判別
// 出力：
//   返り値   : 判別成功
//   autoSub  : 判別文字列種類
//---------------------------------------------------------------------
bool JlsScriptDecode::getTrSpEcID(CmdTrSpEcID& idSub, const string& strName, bool flagOption){
	bool det = false;
	if ( ! _stricmp(strName.c_str(), "TR") ){
		det = true;
		idSub = CmdTrSpEcID::TR;
	}
	else if ( ! _stricmp(strName.c_str(), "SP") ){
		det = true;
		idSub = CmdTrSpEcID::SP;
	}
	else if ( ! _stricmp(strName.c_str(), "EC") ){
		det = true;
		idSub = CmdTrSpEcID::EC;
	}
	else if ( ! _stricmp(strName.c_str(), "LG") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::LG;
	}
	else if ( ! _stricmp(strName.c_str(), "NLG") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::NLG;
	}
	else if ( ! _stricmp(strName.c_str(), "NTR") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::NTR;
	}
	else if ( ! _stricmp(strName.c_str(), "Off") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::Off;
	}
	else{
		det = false;
		idSub = CmdTrSpEcID::None;
	}
	return det;
}
//---------------------------------------------------------------------
// 文字列から最大３項目（中心指定 範囲先頭 範囲末尾）のミリ秒数値を取得
// 取得できなかったらデフォルト値を代入して読み込み位置は取得できた所まで戻す
// 入力：
//   numRead : 読み込むデータ数
//   needs   : 読み込み最低必要数
//   flagM1  : -1はそのまま残す設定（0=特別扱いなし変換、1=-1は変換しない）
//   strBuf  : 文字列
//   pos     : 認識開始位置
// 出力：
//   返り値   : 次の読み込み位置
//   wmsecVal : ３項目取得ミリ秒
//   flagAbbr : 省略データ数
//---------------------------------------------------------------------
int JlsScriptDecode::decodeRangeMsec(JlscrDecodeRangeRecord& infoDec, const string& strBuf, int pos){
	WideMsec wmsecVal = {};
	int pos1 = -1;
	int pos2 = -1;
	int pos3 = -1;
	//--- 文字列から読み出し ---
	switch( infoDec.numRead ){
		case 3:
			if ( infoDec.flagM1 ){		// -1は変換しない処理
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.just,  strBuf, pos);
				pos2 = pdata->cnv.getStrValMsecM1(wmsecVal.early, strBuf, pos1);
				pos3 = pdata->cnv.getStrValMsecM1(wmsecVal.late,  strBuf, pos2);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.just,  strBuf, pos);
				pos2 = pdata->cnv.getStrValMsec(wmsecVal.early, strBuf, pos1);
				pos3 = pdata->cnv.getStrValMsec(wmsecVal.late,  strBuf, pos2);
			}
			break;
		case 2:
			if ( infoDec.flagM1 ){		// -1は変換しない処理
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.early, strBuf, pos);
				pos2 = pdata->cnv.getStrValMsecM1(wmsecVal.late,  strBuf, pos1);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.early, strBuf, pos);
				pos2 = pdata->cnv.getStrValMsec(wmsecVal.late,  strBuf, pos1);
			}
			break;
		case 1:
			if ( infoDec.flagM1 ){		// -1は変換しない処理
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.just,  strBuf, pos);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.just,  strBuf, pos);
			}
			break;
		default:
			break;
	}
	//--- 数値以外だった場合はデフォルト値を設定 ---
	if ( pos1 < 0 ){
		wmsecVal.just  = 0;			// デフォルト値：0ms
		wmsecVal.early = 0;			// デフォルト値：0ms
		wmsecVal.late  = 0;			// デフォルト値：0ms
	}
	//--- 省略時のデフォルト設定 ---
	switch( infoDec.numRead ){
		case 3:
			//--- 3項目目が数値以外だった場合は2,3項目目にデフォルト値を設定 ---
			if ( pos3 < 0 ){
				setRangeMargin(wmsecVal, -1);	// デフォルト値生成
			}
			break;
		case 2:
			//--- 2項目readで2項目が数値以外だった場合 ---
			if ( pos2 < 0 ){
				if ( infoDec.numFrom != 0 ){
					//--- 省略した項目をスキップして設定する場合 ---
					if ( infoDec.numFrom == 2 ){
						wmsecVal.late  = wmsecVal.early;
						wmsecVal.early = 0;
					}
				}else{
					//--- 1項目目を中心としてマージンはデフォルト値を設定 ---
					if ( pos1 >= 0 ){
						wmsecVal.just = wmsecVal.early;		// 1項目目を中心に設定
					}
					setRangeMargin(wmsecVal, -1);		// デフォルト値生成
				}
			}
			break;
		default:
			break;
	}
	//--- 小さい順並び替え ---
	if ( infoDec.flagSort ){
		switch( infoDec.numRead ){
			case 3:
			case 2:
				//--- 2項目目と3項目目は小さいほうを先にする ---
				if ( infoDec.flagM1 ){		// -1は変換しない処理
					sortTwoValM1(wmsecVal.early, wmsecVal.late);	// 範囲は小さい順に並び替え
				}else{
					if (wmsecVal.early > wmsecVal.late){			// 範囲は小さい順に並び替え
						swap(wmsecVal.early, wmsecVal.late);
					}
				}
				break;
			default:
				break;
		}
	}
	//--- 読み込み成功した所まで読み込み位置更新 ---
	int numAbbr = 0;
	switch( infoDec.numRead ){
		case 3:
			if ( pos3 >= 0 || infoDec.needs >= 3 ){
				pos = pos3;
				numAbbr = 0;		// 省略なし
			}else if ( pos2 >= 0 ){	// 3項目中2項目だけ読み出せた場合は失敗とする（設定ミス早期発見のため）
				pos = -1;
				numAbbr = 1;
			}else if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 2;
			}else{
				numAbbr = 3;
			}
			break;
		case 2:
			if ( pos2 >= 0 || infoDec.needs >= 2 ){
				pos = pos2;
				numAbbr = 0;		// 省略なし
			}else if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 1;
			}else{
				numAbbr = 2;
			}
			break;
		case 1:
			if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 0;		// 省略なし
			}else{
				numAbbr = 1;
			}
			break;
		default:
			break;
	}
	infoDec.numAbbr  = numAbbr;
	infoDec.wmsecVal = wmsecVal;
	return pos;
}
//---------------------------------------------------------------------
// 中心指定時に前後同間隔のマージンを設定
// マージンにマイナスを指定した場合はデフォルト間隔を設定
//---------------------------------------------------------------------
void JlsScriptDecode::setRangeMargin(WideMsec& wmsecVal, Msec margin){
	if ( margin >= 0 ){
		wmsecVal.early = wmsecVal.just - margin;
		wmsecVal.late  = wmsecVal.just + margin;
	}
	else{
		wmsecVal.early = wmsecVal.just - msecDecodeMargin;		// デフォルト値：中心-1200ms;
		wmsecVal.late  = wmsecVal.just + msecDecodeMargin;		// デフォルト値：中心-1200ms
	}
}
//---------------------------------------------------------------------
// 文字列から番号リストを取得（-N系オプション用）
//---------------------------------------------------------------------
bool JlsScriptDecode::getListStrNumFromStr(vector<string>& listStrNum, const string& strBuf){
	listStrNum.clear();
	bool success = true;
	int pos = 0;
	while(pos >= 0){		// comma区切りで複数値読み込み
		string strVal;
		pos = pdata->cnv.getStrMultiNum(strVal, strBuf, pos);
		if ( pos >= 0 ){
			listStrNum.push_back(strVal);
		}else if ( !strVal.empty() ){
			success = false;
		}
	}
	if ( success ){
		if ( listStrNum.empty() ){
			success = false;
		}
	}
	return success;
}
//---------------------------------------------------------------------
// 引数オプションの並び替え
// 両方-1以外の時、小さい値を先にする
//---------------------------------------------------------------------
void JlsScriptDecode::sortTwoValM1(int& val_a, int& val_b){
	if (val_a != -1 && val_b != -1){
		if (val_a > val_b){
			int tmp = val_a;
			val_a = val_b;
			val_b = tmp;
		}
	}
}


//=====================================================================
// デコード後の追加処理
//=====================================================================

//---------------------------------------------------------------------
// コマンドオプション内容から範囲設定を再設定
//---------------------------------------------------------------------
void JlsScriptDecode::reviseCmdRange(JlsCmdArg& cmdarg){
	//--- 中心指定 ---
	if ( cmdarg.isSetOpt(OptType::MsecDcenter) ){
		cmdarg.wmsecDst.just = cmdarg.getOpt(OptType::MsecDcenter);
		//--- 範囲外の指定なら範囲も再設定 ---
		if (cmdarg.wmsecDst.just < cmdarg.wmsecDst.early ||
			cmdarg.wmsecDst.just > cmdarg.wmsecDst.late){
			setRangeMargin(cmdarg.wmsecDst, -1);	// 範囲マージンをデフォルト値に設定
		}
	}
	//--- 範囲指定（マージン指定） ---
	if ( cmdarg.isSetOpt(OptType::MsecDmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecDmargin));
		setRangeMargin(cmdarg.wmsecDst, msecMargin);	// 範囲マージン設定
	}
	//--- 範囲先頭 ---
	if ( cmdarg.isSetOpt(OptType::MsecDrangeL) ){
		cmdarg.wmsecDst.early = cmdarg.getOpt(OptType::MsecDrangeL);
	}
	//--- 範囲末尾 ---
	if ( cmdarg.isSetOpt(OptType::MsecDrangeR) ){
		cmdarg.wmsecDst.late = cmdarg.getOpt(OptType::MsecDrangeR);
	}
	//--- 中心未設定＋範囲設定時の中心補正 ---
	if ( !cmdarg.isSetOpt(OptType::MsecDcenter) &&
	     (cmdarg.isSetOpt(OptType::MsecDrangeL) || cmdarg.isSetOpt(OptType::MsecDrangeR)) ){
		if ( cmdarg.wmsecDst.just < cmdarg.wmsecDst.early && cmdarg.wmsecDst.early != -1 ){
			cmdarg.wmsecDst.just = cmdarg.wmsecDst.early;
		}
		else if ( cmdarg.wmsecDst.just > cmdarg.wmsecDst.late && cmdarg.wmsecDst.late != -1 ){
			cmdarg.wmsecDst.just = cmdarg.wmsecDst.late;
		}
	}

	//--- オプション -Emargin が指定された時の処理 ---
	if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecEmargin));
		//--- EndLenの引数が一部省略された場合 ---
		if ( cmdarg.getOpt(OptType::AbbrEndlen) >= 2 ){	// 範囲2か所省略
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndlenC);
			cmdarg.setOpt(OptType::MsecEndlenL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndlenR, msecCenter + msecMargin);
		}
		//--- EndSftの引数が一部省略された場合 ---
		if ( cmdarg.getOpt(OptType::AbbrEndSft) >= 2 ){	// 範囲2か所省略
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndSftC);
			cmdarg.setOpt(OptType::MsecEndSftL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndSftR, msecCenter + msecMargin);
		}
		//--- Shiftの引数が一部省略された場合 ---
		if ( cmdarg.getOpt(OptType::AbbrSft) >= 2 ){	// 範囲2か所省略
			Msec msecCenter = cmdarg.getOpt(OptType::MsecSftC);
			cmdarg.setOpt(OptType::MsecSftL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecSftR, msecCenter + msecMargin);
		}
		//--- -SC系の省略を確認 ---
		if ( m_listKeepSc.empty() == false ){
			int sizeSc = (int)m_listKeepSc.size();
			for(int i=0; i < sizeSc; i++){
				if ( m_listKeepSc[i].abbr >= 1 ){	// 範囲省略時
					WideMsec wmsecVal = m_listKeepSc[i].wmsec;	// 省略時の中心指定を使用
					setRangeMargin(wmsecVal, msecMargin);		// マージン設定
					m_listKeepSc[i].wmsec = wmsecVal;			// 書き戻す
				}
			}
		}
	}
}
//---------------------------------------------------------------------
// コマンドオプション内容から実行オプションの設定
// 出力：
//   cmdarg.tack  : コマンド解析結果
//---------------------------------------------------------------------
void JlsScriptDecode::setCmdTackOpt(JlsCmdArg& cmdarg){
	CmdType  cmdsel    = cmdarg.cmdsel;
	CmdCat   category  = cmdarg.category;
	//--- 推測構成from ---
	bool comUsed = false;		// 共通使用の設定
	{
		bool comFrom = false;
		if ( cmdarg.getOpt(OptType::FnumFromAllC) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTr  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromSp  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromEc  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromBd  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromMx  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTra ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTrr ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTrc ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromAea ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromAec ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromCm  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromNl  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromL   ) > 0 ){
			comFrom = true;
		}
		cmdarg.tack.comFrom = comFrom;

		//--- -Cオプション付加 ---
		bool useScC = false;
		if ( cmdarg.getOptFlag(OptType::FlagScCon) || comFrom ){
			if ( !cmdarg.getOptFlag(OptType::FlagScCoff) &&
			     !cmdarg.getOptFlag(OptType::FlagScCdst) &&
			     !cmdarg.getOptFlag(OptType::FlagScCend) ){
				useScC = true;
			}
		}
		cmdarg.tack.useScC = useScC;

		//--- 共通使用の設定 ---
		if ( cmdarg.getOptFlag(OptType::FlagScCon)  ||
		     cmdarg.getOptFlag(OptType::FlagScCdst) ||
		     cmdarg.getOptFlag(OptType::FlagScCend) ||
		     comFrom ){
			comUsed = true;
		}
	}
	//--- 比較位置を対象位置に変更 ---
	{
		bool floatbase = false;
		if ( cmdsel == CmdType::Select ||
		     cmdsel == CmdType::NextTail ){				// コマンドによる変更
			floatbase = true;
		}
		if (cmdarg.getOpt(OptType::FlagRelative) > 0){	// -relative
			floatbase = true;
		}
		cmdarg.tack.floatBase = floatbase;
	}
	//--- シフト基準位置 ---
	{
		bool sft = false;
		if ( cmdarg.isSetOpt(OptType::MsecSftC) ){		// -shift
			sft = true;
		}
		cmdarg.tack.shiftBase = sft;
	}
	//--- ロゴを推測位置に変更 ---
	{
		bool vtlogo = false;
		if (category == CmdCat::AUTO ||
			category == CmdCat::AUTOEACH){				// Auto系
			vtlogo = true;
		}
		if ( category == CmdCat::AUTOLOGO || comUsed ){		// ロゴも見るAuto系と推測構成使用
			if ((OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNlogo &&	// -Nlogo以外
			    (OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNFlogo &&	// -NFlogo以外
			    (OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNFXlogo ){	// -NFXlogo以外
				vtlogo = true;
			}
		}
		if ( (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNauto ||	// -Nauto
			 (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNFauto ){	// -NFauto
			if ( cmdsel == CmdType::GetPos  ||
				 cmdsel == CmdType::GetList ){
				vtlogo = true;
			}
		}
		if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){		// -final
			vtlogo = true;
		}
		cmdarg.tack.virtualLogo = vtlogo;
	}
	//--- ロゴAbort状態でも実行するコマンド ---
	{
		bool ignabort = false;
		if ( (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNFXlogo ){	// -NFXlogo
			if ( cmdsel == CmdType::GetPos  ||
				 cmdsel == CmdType::GetList ){
				ignabort = true;
			}
		}
		cmdarg.tack.ignoreAbort = ignabort;
	}
	//--- ロゴ確定状態でも実行するコマンド ---
	{
		bool igncomp = false;
		if (cmdsel == CmdType::MkLogo  ||
			cmdsel == CmdType::DivLogo ||
			cmdsel == CmdType::DivFile ||
			cmdsel == CmdType::GetPos  ||
			cmdsel == CmdType::GetList){
			igncomp = true;
		}
		cmdarg.tack.ignoreComp = igncomp;
	}
	//--- 前後のロゴ位置以内に範囲限定する場合（-nolap指定とDivLogoコマンド） ---
	{
		bool limbylogo = false;
		if ( cmdarg.getOpt(OptType::FlagNoLap) > 0 ){		// -nolap指定時に限定
			limbylogo = true;
		}
		cmdarg.tack.limitByLogo = limbylogo;
	}
	//--- Auto構成を必要とするコマンド ---
	{
		bool needauto = false;
		int numlist = cmdarg.sizeScOpt();
		if (numlist > 0){
			for(int i=0; i<numlist; i++){
				OptType sctype = cmdarg.getScOptType(i);
				if (sctype == OptType::ScAC || sctype == OptType::ScNoAC ||
				    sctype == OptType::ScACC || sctype == OptType::ScNoACC){
					needauto = true;
				}
			}
		}
		if ( comUsed ){
			needauto = true;
		}
		cmdarg.tack.needAuto = needauto;
	}
	//--- F系未定義時の範囲制限常時なし ---
	{
		bool fullA = false;
		bool fullB = false;
		if ( cmdsel == CmdType::GetPos  ||
			 cmdsel == CmdType::GetList ||
			 cmdsel == CmdType::NextTail ){
			fullA = true;
		}
		if ( cmdsel == CmdType::NextTail ){
			fullB = true;
		}
		cmdarg.tack.fullFrameA = fullA;
		cmdarg.tack.fullFrameB = fullB;
	}
	//--- 遅延実行の設定種類 ---
	{
		LazyType typelazy = LazyType::None;
		if ( cmdarg.getOpt(OptType::FlagLazyS) > 0 ) typelazy = LazyType::LazyS;
		if ( cmdarg.getOpt(OptType::FlagLazyA) > 0 ) typelazy = LazyType::LazyA;
		if ( cmdarg.getOpt(OptType::FlagLazyE) > 0 ) typelazy = LazyType::LazyE;
		cmdarg.tack.typeLazy = typelazy;
	}
	//--- 直接フレーム指定from ---
	{
		bool immfrom = false;
		if ( cmdarg.isSetStrOpt(OptType::ListFromAbs) ||
			 cmdarg.isSetStrOpt(OptType::ListFromHead) ||
			 cmdarg.isSetStrOpt(OptType::ListFromTail) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFD) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFE) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFX) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetXF) ){
			immfrom = true;
		}
		cmdarg.tack.immFrom = immfrom;
	}
	//--- Dst位置指定オプション存在 ---
	{
		bool existDstOpt = false;
		if ( cmdarg.isSetStrOpt(OptType::ListTgDst  ) ||
		     cmdarg.isSetStrOpt(OptType::ListDstAbs ) ||
		     cmdarg.isSetOpt(OptType::MsecDcenter   ) ||
		     cmdarg.isSetOpt(OptType::MsecDrangeL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstNextL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstPrevL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstNextC   ) ||
		     cmdarg.isSetOpt(OptType::NumDstPrevC   ) ||
		     cmdarg.getOptFlag(OptType::FlagDstPoint) ){
			existDstOpt = true;
		}
		cmdarg.tack.existDstOpt = existDstOpt;
	}
	//--- 強制設定 ---
	{
		bool fc = false;
		if ( cmdarg.getOpt(OptType::FlagForce) > 0 ||
		     cmdarg.getOpt(OptType::FlagNoForce) > 0 ||
		     cmdarg.getOpt(OptType::FlagFixPos) > 0 ){
			fc = true;
		}
		cmdarg.tack.forcePos = fc;
	}
	//--- 出力選別 ---
	{
		bool picki = false;
		bool picko = false;
		if ( cmdsel == CmdType::GetPos  ||
		     cmdsel == CmdType::GetList ||
		     cmdsel == CmdType::AutoIns ||
		     cmdsel == CmdType::AutoDel ){
			if ( cmdarg.isSetStrOpt(OptType::ListPickIn) ){
				if ( !cmdarg.getStrOpt(OptType::ListPickIn).empty() ){
					picki = true;
				}
			}
			if ( cmdarg.isSetStrOpt(OptType::ListPickOut) ){
				if ( !cmdarg.getStrOpt(OptType::ListPickOut).empty() ){
					picko = true;
				}
			}
		}
		cmdarg.tack.pickIn  = picki;
		cmdarg.tack.pickOut = picko;
	}
	//--- 各ロゴ個別オプションのAutoコマンド ---
	{
		if (cmdsel == CmdType::AutoCut ||
			cmdsel == CmdType::AutoAdd){
			if (cmdarg.getOpt(OptType::FlagAutoEach) > 0){
				category = CmdCat::AUTOEACH;
				cmdarg.category = category;		// オプション(-autoeach)によるコマンド体系変更
			}
		}
	}
}


//---------------------------------------------------------------------
// -SC系オプションを設定
//---------------------------------------------------------------------
void JlsScriptDecode::setArgScOpt(JlsCmdArg& cmdarg){
	if ( m_listKeepSc.empty() == false ){
		int sizeSc = (int)m_listKeepSc.size();
		for(int i=0; i < sizeSc; i++){
			TargetCatType tgcat;
			switch( m_listKeepSc[i].subtype ){
				case 0:
					tgcat = TargetCatType::From;
					break;
				case 1:
					tgcat = TargetCatType::Dst;
					break;
				case 2:
					tgcat = TargetCatType::End;
					break;
				case 3:
					tgcat = TargetCatType::RX;
					break;
				default:
					tgcat = TargetCatType::None;
					break;
			}
			cmdarg.addScOpt(m_listKeepSc[i].type, tgcat,
			                m_listKeepSc[i].wmsec.early, m_listKeepSc[i].wmsec.late);
		}
	}
	//--- -C系オプション用マージン取得 ---
	WideMsec wmsec = {0,0,0};
	{
		Msec mgn = -1;
		if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
			mgn = abs(cmdarg.getOpt(OptType::MsecEmargin));
		}
		setRangeMargin(wmsec, mgn);
	}
	//--- -Cオプション追加 ---
	if ( cmdarg.tack.useScC ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::RX, wmsec.early, wmsec.late);
	}
	if ( cmdarg.getOptFlag(OptType::FlagScCdst) ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::Dst, wmsec.early, wmsec.late);
	}
	if ( cmdarg.getOptFlag(OptType::FlagScCend) ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::End, wmsec.early, wmsec.late);
	}
}
//---------------------------------------------------------------------
// オプションの未指定時複写
//---------------------------------------------------------------------
void JlsScriptDecode::mirrorOptToUndef(JlsCmdArg& cmdarg){
	for(int i=0; i<(int)OptCmdMirror.size(); i++){
		if ( cmdarg.cmdsel == OptCmdMirror[i].cmdsel ){		// 対象コマンド
			OptType optTo = OptCmdMirror[i].optTypeTo;
			if ( !cmdarg.isSetStrOpt(optTo) ){			// 対象オプションが未指定時
				OptType optFrom = OptCmdMirror[i].optTypeFrom;
				cmdarg.setStrOpt(optTo, cmdarg.getStrOpt(optFrom) );
			}
		}
	}
}

//---------------------------------------------------------------------
// 引数をコマンド別に演算加工
//---------------------------------------------------------------------
bool JlsScriptDecode::calcCmdArg(JlsCmdArg& cmdarg){
	bool success = true;
	//--- テーブルを順番に参照 ---
	for(int i=0; i<(int)CmdCalcDefine.size(); i++){
		//--- 対象コマンド時に実行 ---
		if ( cmdarg.cmdsel == CmdCalcDefine[i].cmdsel ){
			int          nList   = CmdCalcDefine[i].numArg;		// 引数のリスト番号
			ConvStrType  typeVal = CmdCalcDefine[i].typeVal;	// 演算種類

			//--- 引数を条件に合わせて演算加工 ---
			switch( typeVal ){
				case ConvStrType::CondIF :			// IF条件式は変数の確認あるので後で実行
					cmdarg.setNumCheckCond(nList);
					break;
				case ConvStrType::Param :
					if ( nList >= 2 ){	// 引数２つ必要
						string strName = cmdarg.getStrArg(nList-1);
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringRegParam(strName, strVal);
						if ( success ){
							success = cmdarg.replaceArgString(nList-1, strName);
						}
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
				case ConvStrType::Frame :
				case ConvStrType::Time :
					{
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringFromListStr(strVal, typeVal);	// リスト対応
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
				default:
					{
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringValue(strVal, typeVal);
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
			}
		}
		if ( success == false ) break;
	}
	return success;
}

//=====================================================================
// 文字列変換処理
//=====================================================================

//---------------------------------------------------------------------
// 文字列のリスト各項目を変換して元の文字列に戻す
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringFromListStr(string& strBuf, ConvStrType typeVal){
	bool success = true;
	if ( strBuf.empty() ){
		pFuncList->setListStrClear(strBuf);
		return success;
	}
	string strDst = "";
	int pos = 0;
	while( pos >= 0 ){		// comma区切りで複数値読み込み
		string strTmp;
		pos = pdata->cnv.getStrWord(strTmp, strBuf, pos);
		if ( pos >= 0 ){
			if ( convertStringValue(strTmp, typeVal) ){
				pFuncList->setListStrIns(strDst, strTmp, -1);
			}else{
				success = false;
				pos = -1;
			}
		}
	}
	strBuf = strDst;
	return success;
}
//---------------------------------------------------------------------
// setParamの変数名を番号文字列に変換、値も演算して数値文字列に変換
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringRegParam(string& strName, string& strVal){
	//--- 入力文字列に対応する番号を取得 ---
	int csel = -1;
	{
		const char *varname = strName.c_str();
		//--- 文字列からパラメータを識別 ---
		for(int i=0; i<(int)ConfigDefine.size(); i++){
			if ( _stricmp(varname, ConfigDefine[i].namestr.c_str()) == 0 ){
				csel = i;
				break;
			}
		}
	}
	//--- 文字列を演算して変換 ---
	if ( csel >= 0 ){
		ConfigVarType typeParam  = ConfigDefine[csel].prmsel;
		ConvStrType   typeVal    = ConfigDefine[csel].valsel;
		strName = std::to_string((int)typeParam);		// 名前は番号に変換
		return convertStringValue(strVal, typeVal);		// 値は演算して変換
	}
	return false;
}
//---------------------------------------------------------------------
// 文字列を演算して変換した文字列を格納
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringValue(string& strVal, ConvStrType typeVal){
	int pos = 0;
	int val;
	switch( typeVal ){
		case ConvStrType::Msec :
			pos = pdata->cnv.getStrValMsec(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::MsecM1 :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Sec :
			pos = pdata->cnv.getStrValSec(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Num :
			pos = pdata->cnv.getStrValNum(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Frame :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = pdata->cnv.getStringFrameMsecM1(val);
			}
			break;
		case ConvStrType::Time :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = pdata->cnv.getStringTimeMsecM1(val);
			}
			break;
		case ConvStrType::NumR :
			pos = pdata->cnv.getStrMultiNum(strVal, strVal, 0);
			break;
		default:
			break;
	}
	if ( pos < 0 ){
		return false;
	}
	return true;
}
