//
// 実行スクリプトコマンド文字列解析
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptDecode.hpp"
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
JlsScriptDecode::JlsScriptDecode(JlsDataset *pdata){
	this->pdata  = pdata;
}

//---------------------------------------------------------------------
// 内部設定の異常確認
//---------------------------------------------------------------------
void JlsScriptDecode::checkInitial(){
	for(int i=0; i<SIZE_JLCMD_SEL; i++){
		if ( CmdDefine[i].cmdsel != (CmdType) i ){
			cerr << "error:internal mismatch at CmdDefine.cmdsel " << i << endl;
		}
	}
	for(int i=0; i<SIZE_CONFIG_VAR; i++){
		if ( ConfigDefine[i].prmsel != (ConfigVarType) i ){
			cerr << "error:internal mismatch at ConfigDefine.prmsel " << i << endl;
		}
	}
	if (strcmp(OptDefine[SIZE_JLOPT_DEFINE-1].optname, "-dummy") != 0){
		cerr << "error:internal mismatch at OptDefine.data1 ";
		cerr << OptDefine[SIZE_JLOPT_DEFINE-1].optname << endl;
	}
}

//=====================================================================
// デコード処理
//=====================================================================

//---------------------------------------------------------------------
// コマンド内容を文字列１行から解析
// 入力：
//  strBuf  : 解析文字列
//  onlyCmd : 先頭のコマンド部分だけの解析か（0=全体、1=コマンドのみ）
// 出力：
//   返り値：エラー状態
//   cmdarg: コマンド解析結果
//---------------------------------------------------------------------
CmdErrType JlsScriptDecode::decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd){
	CmdErrType retval = CmdErrType::None;

	//--- コマンド内容初期化 ---
	cmdarg.clear();
	//--- コマンド受付(cmdsel) ---
	string strCmd;
	int csel = 0;
	int pos = pdata->cnv.getStrItem(strCmd, strBuf, 0);
	if (pos >= 0){
		csel = decodeCmdId(strCmd);
	}
	//--- コマンド異常時の終了 ---
	if (csel < 0){
		retval = CmdErrType::ErrCmd;
		return retval;
	}

	//--- コマンド情報設定 ---
	CmdType  cmdsel   = CmdDefine[csel].cmdsel;
	CmdCat   category = CmdDefine[csel].category;
	int muststr   = CmdDefine[csel].muststr;
	int mustchar  = CmdDefine[csel].mustchar;
	int mustrange = CmdDefine[csel].mustrange;
	int needopt   = CmdDefine[csel].needopt;
	cmdarg.cmdsel = cmdsel;
	cmdarg.category = category;

	//--- コマンド受付のみで終了する場合 ---
	if ( onlyCmd ){
		return retval;
	}

	//--- コマンド解析 ---
	if ( muststr > 0 || mustchar > 0 || mustrange > 0 ){
		pos = decodeCmdArgMust(cmdarg, retval, strBuf, pos, muststr, mustchar, mustrange);
	}

	//--- オプション受付 ---
	if (needopt > 0 && pos >= 0){
		pos = decodeCmdArgOpt(cmdarg, retval, strBuf, pos);
	}
	//--- 引数を演算加工 ---
	if ( muststr > 0 ){
		bool success = calcCmdArg(cmdarg);
		if ( success == false ){
			retval = CmdErrType::ErrOpt;
		}
	}

	return retval;
}

//---------------------------------------------------------------------
// コマンド名を取得
// 出力：
//   返り値  ：取得コマンド番号（失敗時は-1）
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdId(const string& cstr){
	int det = -1;
	const char *cmdname = cstr.c_str();

	if (cmdname[0] == '\0' || cmdname[0] == '#'){
		det = 0;
	}
	else{
		for(int i=0; i<SIZE_JLCMD_SEL; i++){
			if (_stricmp(cmdname, CmdDefine[i].cmdname) == 0){
				det = i;
				break;
			}
		}
		//--- 見つからなければ別名を検索 ---
		if (det < 0){
			bool flag = false;
			CmdType target;
			for(int i=0; i<SIZE_JLSCR_CMDALIAS; i++){
				if (_stricmp(cmdname, CmdAlias[i].cmdname) == 0){
					target = CmdAlias[i].cmdsel;
					flag = true;
					break;
				}
			}
			if ( flag ){
				for(int i=0; i<SIZE_JLCMD_SEL; i++){
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
//   tps: 文字列引数（0-3=取得数 9=残り全体）
//   tpc: 種類設定（0=設定なし  1=S/E/B  2=TR/SP/EC 3=省略可能なS/E/B）
//   tpw: 期間設定（0=設定なし  1=center  3=center+left+right）
// 出力：
//   返り値  : 読み込み位置（-1=オプション異常）
//   errval  : エラー番号
//   cmdarg  : コマンド解析結果
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, int tps, int tpc, int tpw){
	//--- 文字列として引数を取得 ---
	if ( tps > 0 && pos >= 0){
		if ( tps == 9 ){
			//--- 残り全部を文字列として取得 ---
			while( strBuf[pos] == ' ' && pos >= 0) pos++;
			string strArg = strBuf.substr(pos);
			cmdarg.addArgString(strArg);
		}else{
			int sizeArg = tps;
			for(int i=0; i<sizeArg; i++){
				string strArg;
				pos = pdata->cnv.getStrItem(strArg, strBuf, pos);
				if ( pos >= 0 ){
					cmdarg.addArgString(strArg);
				}
			}
			if (pos < 0){
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- 種類文字 ---
	if (tpc > 0 && pos >= 0){
		string strTmp;
		int posbak = pos;
		pos = pdata->cnv.getStrItem(strTmp, strBuf, pos);
		if (pos >= 0){
			//--- 項目１（文字指定） ---
			if (tpc == 1 || tpc == 3){
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
					if (tpc == 3) pos = posbak;
					else{
						pos    = -1;
						errval = CmdErrType::ErrSEB;
					}
				}
			}
			//--- TR/SP/EC 文字列判別 ---
			else if (tpc == 2){
				bool flagOption = false;
				if ( getTrSpEcID(cmdarg.selectAutoSub, strTmp, flagOption) == false ){
					pos    = -1;
					errval = CmdErrType::ErrTR;
				}
			}
		}
	}
	//--- 範囲指定 ---
	if (tpw > 0 && pos >= 0){
		if (tpw == 1 || tpw == 3){
			JlscrDecodeRangeRecord infoDec = {};
			if (tpw == 1){
				infoDec.numRead  = 1;		// データ読み込み数=1
				infoDec.needs    = 0;		// 最低読み込み数=0（全項目省略可）
				infoDec.numFrom  = 0;		// 省略時の開始指定なし（標準動作）
				infoDec.flagM1   = false;	// -1は通常の数値として読み込む
				infoDec.flagSort = false;	// １データなので並び替えなし
			}
			else if (tpw == 3){
				infoDec.numRead  = 3;		// データ読み込み数=3
				infoDec.needs    = 0;		// 最低読み込み数=0（全項目省略可）
				infoDec.numFrom  = 0;		// 省略時の開始指定なし（標準動作）
				infoDec.flagM1   = false;	// -1は通常の数値として読み込む
				infoDec.flagSort = true;	// 小さい順の並び替えあり
			}
			pos = decodeRangeMsec(infoDec, strBuf, pos);
			cmdarg.wmsecDst = infoDec.wmsecVal;
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
	while(pos >= 0){
		pos = decodeCmdArgOptOne(cmdarg, errval, strBuf, pos);
	}
	reviseCmdRange(cmdarg);		// オプションによる範囲補正
	setCmdTackOpt(cmdarg);		// 実行オプション設定
	setArgScOpt(cmdarg);		// 一時保持した-SC系オプションを設定
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
	string strWord;
	pos = pdata->cnv.getStrItem(strWord, strBuf, pos);
	if (pos >= 0){
		//--- コメント除去 ---
		int poscut = (int) strWord.find("#");
		if (poscut == 0){
			pos = -1;
		}
		else if (poscut > 0){
			strWord = strWord.substr(0, poscut);
		}
	}
	int optsel = -1;
	if (pos >= 0){
		//--- オプション識別 ---
		const char *pstr = strWord.c_str();
		for(int i=0; i<SIZE_JLOPT_DEFINE; i++){
			if (!_stricmp(pstr, OptDefine[i].optname)){
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
				string strSub;
				pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
				if ( pos >= 0 ){
					vector<string> listStrNum;
					if ( getListStrNumFromStr(listStrNum, strSub) ){
						for(int i=0; i < (int)listStrNum.size(); i++){
							cmdarg.addLgOpt(listStrNum[i]);
						}
					}else{
						pos = -1;
					}
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
					keepSc.relative = (OptDefine[optsel].subType != 0)? true : false;
					keepSc.wmsec    = infoDec.wmsecVal;
					keepSc.abbr     = infoDec.numAbbr;
					m_listKeepSc.push_back(keepSc);		// 後で範囲補正が入るので一時保持
				}
			}
			break;
		case OptCat::STR :						// 文字列の設定
			{
				string strSub;
				pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
				if ( pos >= 0 ){
					if ( strSub[0] != '-' ){
						cmdarg.setStrOpt(optType, strSub);
					}else{
						pos = -1;				// 次が"-"だった時はオプション用文字列ではない
					}
				}
			}
			break;
		case OptCat::NUM :						// 数値の設定
			{
				vector<OptType> listOptType(4);
				int numUsed = getOptionTypeList(listOptType, optType, infoDec.numRead);
				int listVal[3];
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
							pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
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
	cerr << "error:internal setting" << msg << endl;
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
		string strTmp;
		pos = pdata->cnv.getStrWord(strTmp, strBuf, pos);
		if (pos >= 0){
			int rloc = (int)strTmp.find("..");
			if ( rloc != (int)string::npos ){			// ..による範囲設定時
				string strSt = strTmp.substr(0, rloc);
				string strEd = strTmp.substr(rloc+2);
				int valSt;
				int valEd;
				int posSt = pdata->cnv.getStrValNum(valSt, strSt, 0);
				int posEd = pdata->cnv.getStrValNum(valEd, strEd, 0);
				if ( posSt >= 0 && posEd >= 0 ){
					string strValSt = std::to_string(valSt);
					string strValEd = std::to_string(valEd);
					string strVal = strValSt + ".." + strValEd;
					listStrNum.push_back(strVal);
				}else{
					success = false;
				}
			}else{
				int val1;
				if (pdata->cnv.getStrValNum(val1, strTmp, 0) >= 0){
					string strVal = std::to_string(val1);
					listStrNum.push_back(strVal);
				}else{
					success = false;
				}
			}
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

	//--- オプション -Emargin が指定された時の処理 ---
	if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecEmargin));
		//--- EndLenの引数が一部省略された場合 ---
		if ( cmdarg.getOpt(OptType::AbbrEndlen) >= 2 ){	// 範囲2か所省略
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndlenC);
			cmdarg.setOpt(OptType::MsecEndlenL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndlenR, msecCenter + msecMargin);
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
	//--- 比較位置を対象位置に変更 ---
	{
		bool floatbase = false;
		if ( cmdsel == CmdType::Select ||
		     cmdsel == CmdType::NextTail ){				// コマンドによる変更
			floatbase = true;
		}
		if (cmdarg.isSetOpt(OptType::MsecSftC) ||		// -shift
			cmdarg.getOpt(OptType::FlagRelative) > 0){	// -relative
			floatbase = true;
		}
		cmdarg.tack.floatBase = floatbase;
	}
	//--- ロゴを推測位置に変更 ---
	{
		bool vtlogo = false;
		if (category == CmdCat::AUTO ||
			category == CmdCat::AUTOEACH){				// Auto系
			vtlogo = true;
		}
		if (category == CmdCat::AUTOLOGO &&				// ロゴも見るAuto系
			(OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNlogo){	// -Nlogo以外
			vtlogo = true;
		}
		if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){		// -final
			vtlogo = true;
		}
		cmdarg.tack.virtualLogo = vtlogo;
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
		if (cmdsel == CmdType::DivLogo){
			limbylogo = true;
		}
		if ( cmdarg.getOpt(OptType::FlagNoLap) > 0 ){		// -nolap指定時に限定
			limbylogo = true;
		}
		cmdarg.tack.limitByLogo = limbylogo;
	}
	//--- 絶対位置指定時のロゴ検索は１箇所のみにする ---
	{
		bool onepoint = false;
		if ( cmdarg.isSetOpt(OptType::MsecFromAbs)  ||	// -fromabs
			 cmdarg.isSetOpt(OptType::MsecFromHead) ||	// -fromhead
			 cmdarg.isSetOpt(OptType::MsecFromTail) ){	// -fromtail
			onepoint = true;
		}
		if (cmdsel == CmdType::GetPos){	// １箇所のみ検索
			onepoint = true;
		}
		cmdarg.tack.onePoint = onepoint;
	}
	//--- Auto構成を必要とするコマンド ---
	{
		bool needauto = false;
		int numlist = cmdarg.sizeScOpt();
		if (numlist > 0){
			for(int i=0; i<numlist; i++){
				OptType sctype = cmdarg.getScOptType(i);
				if (sctype == OptType::ScAC || sctype == OptType::ScNoAC){
					needauto = true;
				}
			}
		}
		cmdarg.tack.needAuto = needauto;
	}
	//--- F系未定義時の範囲制限常時なし ---
	{
		bool full = false;
		if ( cmdsel == CmdType::GetPos  ||
			 cmdsel == CmdType::GetList ){
			full = true;
		}
		cmdarg.tack.fullFrame = full;
	}
	//--- 遅延実行の設定種類 ---
	{
		LazyType typelazy = LazyType::None;
		if ( cmdarg.getOpt(OptType::FlagLazyS) > 0 ) typelazy = LazyType::LazyS;
		if ( cmdarg.getOpt(OptType::FlagLazyA) > 0 ) typelazy = LazyType::LazyA;
		if ( cmdarg.getOpt(OptType::FlagLazyE) > 0 ) typelazy = LazyType::LazyE;
		cmdarg.tack.typeLazy = typelazy;
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
			cmdarg.addScOpt(m_listKeepSc[i].type, m_listKeepSc[i].relative,
			                m_listKeepSc[i].wmsec.early, m_listKeepSc[i].wmsec.late);
		}
	}
}

//---------------------------------------------------------------------
// 引数をコマンド別に演算加工
//---------------------------------------------------------------------
bool JlsScriptDecode::calcCmdArg(JlsCmdArg& cmdarg){
	bool success = true;
	//--- テーブルを順番に参照 ---
	for(int i=0; i<SIZE_JLCMD_CALC_DEFINE; i++){
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
				default:
					string strVal  = cmdarg.getStrArg(nList);
					success = convertStringValue(strVal, typeVal);
					if ( success ){
						success = cmdarg.replaceArgString(nList, strVal);
					}
					break;
			}
		}
		if ( success == false ) break;
	}
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
		for(int i=0; i<SIZE_CONFIG_VAR; i++){
			if ( _stricmp(varname, ConfigDefine[i].namestr) == 0 ){
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
		default:
			break;
	}
	if ( pos < 0 ){
		return false;
	}
	return true;
}
