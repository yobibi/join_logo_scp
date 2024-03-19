//
// 文字列と時間とフレーム位置の相互変換クラス
//
//#include "stdafx.h"
#include <iostream>

using namespace std;
using Msec = int;
#include "CnvStrTime.hpp"
#include "LocalEtc.hpp"


//---------------------------------------------------------------------
// 構築時初期設定
//---------------------------------------------------------------------
CnvStrTime::CnvStrTime(){
	m_frate_n = 30000;
	m_frate_d = 1001;
	m_unitsec = 0;
	m_delimiter = LocalEtc::LSys.getPathDelimiter();		// パス区切りは機種別に固定
}



//=====================================================================
// ファイル名・パスの分解処理
//=====================================================================

//---------------------------------------------------------------------
// 文字列はすべてパス部分として最後に区切りがなければ付加
// 付加した場合は返り値をtrue、そのままならfalse
// 入力：
//   pathname : パス名
// 出力：
//   pathname : パス名（最後は区切り文字）
//---------------------------------------------------------------------
bool CnvStrTime::getStrFileAllPath(string &pathname){
	string strTmp;
	getStrFilePath(strTmp, pathname);	// 区切りまでの文字取得
	if ( strTmp != pathname ){		// 区切りまでの文字が全体か確認
		string delimiter = getStrFileDelimiter();	// 区切り文字
		pathname += delimiter;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
// 文字列からファイルパス部分とファイル名部分を分離
// 読み終わった位置を返り値とする（失敗時は-1）
// 入力：
//   fullpath : フルパス名
// 出力：
//   pathname : パス部分
//---------------------------------------------------------------------
int CnvStrTime::getStrFilePath(string &pathname, const string &fullname){
	string strTmp;
	return getStrFilePathName(pathname, strTmp, fullname);
}

//---------------------------------------------------------------------
// 文字列からファイルパス部分とファイル名部分を分離
// 読み終わった位置を返り値とする（失敗時は-1）
// 入力：
//   fullpath : フルパス名
// 出力：
//   pathname : パス部分
//   fname    : 名前以降部分
//---------------------------------------------------------------------
int CnvStrTime::getStrFilePathName(string &pathname, string &fname, const string &fullname){
	bool flag_find = false;
	//--- "\"区切りを検索 ---
	int nloc = (int) fullname.rfind("\\");
	if (nloc >= 0){
		flag_find = true;
//		m_delimiter = "\\";		// 区切り文字変更
	}
	//--- "/"区切りを検索 ---
	int nloc_sl = (int) fullname.rfind("/");
	if (nloc_sl >= 0){
		if (flag_find == false || nloc < nloc_sl){
			flag_find = true;
			nloc = nloc_sl;
//			m_delimiter = "/";		// 区切り文字変更
		}
	}
	if (flag_find){
		pathname = fullname.substr(0, nloc+1);
		fname    = fullname.substr(nloc+1);
	}
	else{
		pathname = "";
		fname = fullname;
		nloc = -1;
	}
	return nloc;
}

//---------------------------------------------------------------------
// ファイルの区切り文字取得
//---------------------------------------------------------------------
string CnvStrTime::getStrFileDelimiter(){
	return m_delimiter;
}

//---------------------------------------------------------------------
// バッファサイズ取得
//---------------------------------------------------------------------
int CnvStrTime::getBufLineSize(){
	return SIZE_BUF_MAX;
}
//---------------------------------------------------------------------
// 関数タイプの引数モジュール名であるか確認
//---------------------------------------------------------------------
bool CnvStrTime::isStrFuncModule(const string &cstr, int pos){
	//--- モジュール名 ---
	string strWord;
	pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_NAME);
	if ( pos < 0 ) return false;
	//--- 引数先頭取得 ---
	string strNext;
	int posnext = getStrItemHubFunc(strNext, cstr, pos, DELIMIT_FUNC_ARGS);
	if ( posnext < 0 ) return false;
	if ( strNext[0] != '(' ) return false;		// "("以外なら関数ではない
	//--- モジュール名簡易確認 ---
	bool match = true;
	for(int i=0; i<(int)strWord.length(); i++){
		char ch = strWord[i];
		if ( ch >= 0 && ch <= 0x7F ){
			if ( (ch >= '0' && ch <= '9') ||
			     (ch >= 'A' && ch <= 'Z') ||
			     (ch >= 'a' && ch <= 'z') ||
			     (ch == '_') ){
			}else{
				match = false;
			}
		}
	}
	return match;
}
//---------------------------------------------------------------------
// モジュール名と引数のリストを取得 フォーマット： モジュール名(引数 引数 ...)
//---------------------------------------------------------------------
int CnvStrTime::getListModuleArg(vector<string>& listMod, const string &cstr, int pos){
	listMod.clear();
	string strWord;
	//--- モジュール名設定 ---
	pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_NAME);
	if ( pos < 0 ) return pos;
	listMod.push_back(strWord);
	//--- 引数先頭取得 ---
	int posBak = pos;
	pos = skipCharSpace(cstr, pos);
	if ( cstr[pos] != '(' ){		// "("以外なら引数なしで終了
		return posBak;
	}
	pos = getStrItemHubFunc(strWord, cstr, pos+1, DELIMIT_FUNC_ARGS);
	//--- 引数設定 ---
	bool cont = true;
	while( cont && pos >= 0 ){
		if ( strWord == ")" ){
			listMod.push_back("");	// 最後に空文字列を格納
			cont = false;
		}else{
			listMod.push_back(strWord);
			pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_ARGS);
		}
	}
	return pos;
}
//---------------------------------------------------------------------
// パスで分割した文字列を取得
// 入力：
//   strDiv   : フルパス名
//   selHead  : 分割の出力（true=前側、false=後側）
//   withDelim : true=前側出力時に区切り位置の区切りも出力
// 出力：
//   返り値 : 区切り存在有無
//   strDiv   : 区切り分割後文字列（Head=前側。Tail=後側）
//---------------------------------------------------------------------
bool CnvStrTime::getStrDivPath(string& strDiv, bool selHead, bool withDelim){
	int pos = getStrPosPath(strDiv);
	if ( selHead ){
		if ( pos < 0 ){
			strDiv.clear();
			return false;
		}
		if ( withDelim ){	// 区切り含む
			pos ++;
		}
		if ( pos == 0 ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(0, pos);
		}
	}else{
		if ( pos < 0 ){
			return false;
		}
		pos ++;		// 区切りを除く
		if ( pos >= (int) strDiv.length() ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(pos);
		}
	}
	return true;
}
//---------------------------------------------------------------------
// 区切りで分割した文字列を取得（拡張子取得用）
// 入力：
//   strDiv   : フルパス名
//   strDelim : 区切り文字
//   selHead  : 分割の出力（true=前側、false=後側）
//   typePath : パス後に限定する時はtrue
// 出力：
//   返り値 : 区切り存在有無
//   strDiv   : 区切り分割後文字列（Head=前側。Tail=後側）
//---------------------------------------------------------------------
bool CnvStrTime::getStrDivide(string& strDiv, const string& strDelim, bool selHead, bool typePath){
	int posdiv = getStrPosDivide(strDiv, strDelim, typePath);
	if ( selHead ){
		if ( posdiv < 0 ){
			return false;
		}
		if ( posdiv == 0 ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(0, posdiv);
		}
	}else{
		if ( posdiv < 0 ){
			strDiv.clear();
			return false;
		}
		posdiv += (int) strDelim.length();
		if ( posdiv >= (int) strDiv.length() ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(posdiv);
		}
	}
	return true;
}

//---------------------------------------------------------------------
// パス区切り位置を取得
//---------------------------------------------------------------------
int CnvStrTime::getStrPosPath(const string& fullname){
	bool reverse = true;
	int pos = getStrPosDivideCore(fullname, "\\", reverse);
	int postmp = getStrPosDivideCore(fullname, "/", reverse);
	if ( pos < postmp ){
		pos = postmp;
	}
	return pos;
}
//---------------------------------------------------------------------
// 区切り文字の位置取得
// 入力：
//   fullpath : フルパス名
//   strDelim : 区切り文字
//   typePath : パス後に限定する時はtrue
// 出力：
//   返り値 : 区切り位置（見つからない時はマイナス）
//---------------------------------------------------------------------
int CnvStrTime::getStrPosDivide(const string& fullname, const string& strDelim, bool typePath){
	//--- パス後の時はパス位置を取得 ---
	int posmin = -1;
	if ( typePath ){
		posmin = getStrPosPath(fullname);
	}
	//--- 対象位置取得 ---
	int posr = getStrPosDivideCore(fullname, strDelim, typePath);
	if ( posr < posmin ){
		posr = -1;
	}
	return posr;
}
//---------------------------------------------------------------------
// 区切り文字の位置を取得
// 入力：
//   fullpath : フルパス名
//   strDelim : 区切り文字
//   reverse  : true=後側から false=前側から
// 出力：
//   返り値 : 区切り位置（見つからない時は-1）
//---------------------------------------------------------------------
int CnvStrTime::getStrPosDivideCore(const string& fullname, const string& strDelim, bool reverse){
	int pos = -1;
	//--- 区切りを検索 ---
	if ( reverse ){
		auto posFind = fullname.rfind(strDelim);
		if ( posFind != string::npos ){
			pos = (int) posFind;
		}
	}else{
		auto posFind = fullname.find(strDelim);
		if ( posFind != string::npos ){
			pos = (int) posFind;
		}
	}
	return pos;
}

//=====================================================================
//  時間とフレーム位置の変換
//  注意点：フレーム位置からの変換は先頭フレームを0とした絶対位置で指定するようにしておく
//=====================================================================

//---------------------------------------------------------------------
// ミリ秒をフレーム数に変換
//---------------------------------------------------------------------
int CnvStrTime::getFrmFromMsec(Msec msec){
	int r = ((((long long)abs(msec) * m_frate_n) + (m_frate_d*1000/2)) / (m_frate_d*1000));
	return (msec >= 0)? r : -r;
}

//---------------------------------------------------------------------
// フレーム数に対応するミリ秒数を取得
//---------------------------------------------------------------------
int CnvStrTime::getMsecFromFrm(int frm){
	int r = (((long long)abs(frm) * m_frate_d * 1000 + (m_frate_n/2)) / m_frate_n);
	return (frm >= 0)? r : -r;
}

//---------------------------------------------------------------------
// ミリ秒を一度フレーム数に換算した後ミリ秒に変換（フレーム単位になるように）
//---------------------------------------------------------------------
int CnvStrTime::getMsecAlignFromMsec(Msec msec){
	int frm = getFrmFromMsec(msec);
	return getMsecFromFrm(frm);
}

//---------------------------------------------------------------------
// ミリ秒を一度フレーム数に換算した後微調整してミリ秒に変換
//---------------------------------------------------------------------
int CnvStrTime::getMsecAdjustFrmFromMsec(Msec msec, int frm){
	int frm_new = getFrmFromMsec(msec) + frm;
	return getMsecFromFrm(frm_new);
}

//---------------------------------------------------------------------
// ミリ秒を秒数に変換
//---------------------------------------------------------------------
int CnvStrTime::getSecFromMsec(Msec msec){
	if (msec < 0){
		return -1 * ((-msec + 500) / 1000);
	}
	return ((msec + 500) / 1000);
}

//---------------------------------------------------------------------
// フレームレート変更関数（未使用）
//---------------------------------------------------------------------
int CnvStrTime::changeFrameRate(int n, int d){
	m_frate_n = n;
	m_frate_d = d;
	return 1;
}

//---------------------------------------------------------------------
// 整数入力時の単位設定
//---------------------------------------------------------------------
int CnvStrTime::changeUnitSec(int n){
	m_unitsec = n;
	return 1;
}



//=====================================================================
// 文字列から数値を取得
// [基本動作]
//   文字列から１単語を読み込み数値として出力
//   src文字列の位置posから１単語を読み込み、数値を出力
//   読み終わった位置を返り値とする（失敗時は-1）
// 入力：
//   cstr : 文字列
//   pos  : 認識開始位置
// 出力：
//   返り値： 読み終わった位置を返り値とする（失敗時は-1）
//   val    : 結果数値
//=====================================================================

//---------------------------------------------------------------------
// １単語を読み込み数値として出力（数値以外があればそこで終了）
//---------------------------------------------------------------------
int CnvStrTime::getStrValNumHead(int &val, const string &cstr, int pos){
	// unitsec=2（単位変換しない）
	// type=EXNUM（数値以外があればそこで終了）
	return getStrValSubDelimit(val, cstr, pos, 2, DELIMIT_SPACE_EXNUM);
}

//---------------------------------------------------------------------
// １単語を読み込み数値として出力（数値以外があれば読み込み失敗を返す）
//---------------------------------------------------------------------
int CnvStrTime::getStrValNum(int &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, 2);		// unitsec=2（単位変換しない）
}

//---------------------------------------------------------------------
// １単語を読み込み数値（ミリ秒）として出力（数値以外があれば読み込み失敗を返す）
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsec(Msec &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, m_unitsec);
}

//---------------------------------------------------------------------
// 数値（ミリ秒）を返すが、整数入力は設定にかかわらずフレーム数として扱う
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecFromFrm(Msec &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, 0);			// unitsec=0:整数時はフレーム数
}

//---------------------------------------------------------------------
// 数値（ミリ秒）を返すが、マイナス１の時は特殊扱いで変換せずそのまま返す
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecM1(Msec &val, const string &cstr, int pos){
	int posnew = getStrValSub(val, cstr, pos, m_unitsec);
	if ((m_unitsec == 0 && getFrmFromMsec(val) == -1) ||
		(m_unitsec == 1 && val == -1000)){
		if (posnew > 0){
			if ((int)cstr.substr(pos, posnew-pos).find(".") < 0){
				val = -1;
			}
		}
	}
	return posnew;
}

//---------------------------------------------------------------------
// １単語を読み込み数値（秒）として出力（数値以外があれば読み込み失敗を返す）
//---------------------------------------------------------------------
int CnvStrTime::getStrValSec(int &val, const string &cstr, int pos){
	int tmpval;
	pos = getStrValSub(tmpval, cstr, pos, m_unitsec);
	val = (abs(tmpval) + 500) / 1000;
	if (tmpval < 0){
		val = -val;
	}

	return pos;
}

//---------------------------------------------------------------------
// 数値（秒）を返すが、整数入力は設定にかかわらず秒数入力として扱う
//---------------------------------------------------------------------
int CnvStrTime::getStrValSecFromSec(int &val, const string &cstr, int pos){
	int tmpval;
	pos = getStrValSub(tmpval, cstr, pos, 1);			// unitsec=1:整数時は秒数
	val = (abs(tmpval) + 500) / 1000;
	if (tmpval < 0){
		val = -val;
	}

	return pos;
}

//---------------------------------------------------------------------
// 関数引数文字列を取得して、結果の数値を返す
//---------------------------------------------------------------------
int CnvStrTime::getStrValFuncNum(int &val, const string &cstr, int pos){
	string dstr;
	pos = getStrItemHubFunc(dstr, cstr, pos, DELIMIT_FUNC_CALC);
	if ( getStrValNum(val, dstr, 0) < 0 ){
		return -1;
	}
	return pos;
}

//=====================================================================
// リストデータ取得
//=====================================================================

//---------------------------------------------------------------------
// リスト文字列から全項目の時間情報（ミリ秒）をリストで取得
//---------------------------------------------------------------------
//--- -1の特殊扱いなし ---
bool CnvStrTime::getListValMsec(vector<Msec>& listMsec, const string& strList){
	int pos = 0;
	string dstr;
	listMsec.clear();
	while( (pos = getStrWord(dstr, strList, pos)) > 0 ){
		int val;
		if ( getStrValMsec(val, dstr, 0) > 0 ){
			listMsec.push_back(val);
		}
		while ( strList[pos] == ',' ) pos++;
	}
	if ( listMsec.empty() ) return false;
	return true;
}
//--- -1は特殊扱いでそのまま ---
bool CnvStrTime::getListValMsecM1(vector<Msec>& listMsec, const string& strList){
	int pos = 0;
	string dstr;
	listMsec.clear();
	while( (pos = getStrWord(dstr, strList, pos)) > 0 ){
		int val;
		if ( getStrValMsecM1(val, dstr, 0) > 0 ){
			listMsec.push_back(val);
		}
		while ( strList[pos] == ',' ) pos++;
	}
	if ( listMsec.empty() ) return false;
	return true;
}



//=====================================================================
// 文字列から単語を取得
// [基本動作]
//   文字列から１単語を読み込み出力
//   src文字列の位置posから１単語を読み込みdstに出力
//   読み終わった位置を返り値とする（失敗時は-1）
// 入力：
//   cstr : 文字列
//   pos  : 認識開始位置
// 出力：
//   返り値： 読み終わった位置を返り値とする（失敗時は-1）
//   dst    : 出力文字列
//=====================================================================

// getStrItem    : 先頭からのquoteは認識、途中文字からのquoteは無視
// getStrWord    : スペース+コンマ区切り、コンマ自体は飛ばして読む
// getStrCsv     : CSV形式の1項目を取得
// getStrItemCmd : コマンド読み込み用
// getStrItemArg : 引数用。quote囲みを続けて連結を許可
// getStrItemMonitor : 表示用。quoteは消さない
// getStrItemWithQuote : 最初と最後のquoteを残す。文字列条件の判定用

//--- 文字列からスペース区切りで１単語を読み込む ---
int CnvStrTime::getStrItem(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- 文字列から１単語を読み込む（スペース以外に","を区切りとして認識） ---
int CnvStrTime::getStrWord(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_COMMA;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- 文字列からCSV形式の1項目を取得 ---
int CnvStrTime::getStrCsv(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_CSV;
	itype.concat = true;		// ダブルクォート結合（特殊処理）あり
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = true;		// データなしも許可
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- 文字列からスペース区切りで１単語を読み込む（コマンド取得用） ---
int CnvStrTime::getStrItemCmd(string &dst, const string &cstr, int pos){
	return getStrItemHubFunc(dst, cstr, pos, DELIMIT_FUNC_NAME);
}
//--- 文字列からスペース区切りで１単語を読み込む（quote前後に区切りなければ結合） ---
int CnvStrTime::getStrItemArg(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// 結合あり
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- quoteはそのまま表示する ---
int CnvStrTime::getStrItemMonitor(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// 結合あり
	itype.separate = false;
	itype.remain = true;		// 先頭最後quoteはそのまま出力
	itype.defstr = false;
	itype.qdisp  = true;		// 内部のquoteはそのまま出力
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- quoteも区切りで残したまま文字列から１単語を読み込む（連続quoteの場合のみ結合） ---
int CnvStrTime::getStrItemWithQuote(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// 結合あり
	itype.separate = true;		// "aa"=="bb"のケースで==の前後で分離
	itype.remain = true;		// 項目の先頭と最後のqyoteは残す
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}

//---------------------------------------------------------------------
// コメントを除いて文字列取得
//---------------------------------------------------------------------
int CnvStrTime::getStrWithoutComment(string &dst, const string &cstr){
	int poscmt = getStrPosComment(cstr, 0);
	if ( poscmt > 0 ){
		dst = cstr.substr(0, poscmt);
	}else if ( poscmt == 0 ){
		dst = "";
	}else{
		dst = cstr;
	}
	return poscmt;
}
//---------------------------------------------------------------------
// コメントとしての#位置を取得
//---------------------------------------------------------------------
int CnvStrTime::getStrPosComment(const string &cstr, int pos){
	return getStrPosChar(cstr, '#', false, pos);
}
//---------------------------------------------------------------------
// 変数としての$位置を取得
//---------------------------------------------------------------------
int CnvStrTime::getStrPosReplaceVar(const string &cstr, int pos){
	return getStrPosChar(cstr, '$', true, pos);
}
//---------------------------------------------------------------------
// 指定制御文字がpos以降で最初に現れる位置を取得（展開しない引用符内は除く）
//   expand : true=ダブルクォート内は展開する
//---------------------------------------------------------------------
int CnvStrTime::getStrPosChar(const string &cstr, char chsel, bool expand, int pos){
	int possel = -1;
	int poscmt = -1;
	bool flagQw = false;
	bool flagQs = false;
	bool nextSp = false;
	int len = (int) cstr.length();
	for(int i=0; i<len; i++){
		char cht = cstr[i];
		if ( nextSp ){		// 特殊文字処理
			if ( cht == '#' ){	// $#は特殊処理
				nextSp = false;
				continue;
			}
		}
		bool check = ( expand || flagQw == false ) && ( flagQs == false );
		if ( check && i >= pos && cht == chsel ){
			if ( chsel == '$' ){	// 変数検索時の特殊処理
				char chn = cstr[i+1];
				if ( (0x00 <= chn && chn <  '0' ) ||
				     ( '9' <  chn && chn <  'A' ) ||
				     ( 'Z' <  chn && chn <  'a' ) ||
				     ( 'z' <  chn && chn <= 0x7F) ){
					if ( chn != '#' && chn != '{' && chn != '_' ){
						continue;		// $直後が変数と無関係の記号であれば変数と認識しない
					}
				}
			}
			possel = i;
			break;			// 結果格納したら終了
		}
		else if ( flagQw ){
			if ( cht == '\"' ) flagQw = false;
		}
		else if ( flagQs ){
			if ( cht == '\'' ) flagQs = false;
		}
		else{
			switch( cht ){
				case '#' :
					if ( poscmt < 0 ) poscmt = i;	// コメント位置
					break;
				case '$' :
					nextSp = true;
					break;
				case '\"' :
					flagQw = true;
					break;
				case '\'' :
					flagQs = true;
					break;
				default:
					break;
			}
		}
	}
	return possel;
}
//---------------------------------------------------------------------
// 文字列から .. による範囲指定を含む整数を取得して文字列として返す
// 取得できなかった時は返り値がマイナスになるが、正常終了ならdstが空、異常なら文字列が入る
//---------------------------------------------------------------------
int CnvStrTime::getStrMultiNum(string &dst, const string &cstr, int pos){
	int posBak = pos;
	string strTmp;
	pos = getStrWord(strTmp, cstr, pos);
	if ( pos >= 0 ){
		int rloc = (int)strTmp.find("..");
		if ( rloc != (int)string::npos ){			// ..による範囲設定時
			string strSt = strTmp.substr(0, rloc);
			string strEd = strTmp.substr(rloc+2);
			int valSt;
			int valEd;
			int posSt = getStrValNum(valSt, strSt, 0);
			int posEd = getStrValNum(valEd, strEd, 0);
			if ( posSt >= 0 && posEd >= 0 ){
				string strValSt = std::to_string(valSt);
				string strValEd = std::to_string(valEd);
				dst = strValSt + ".." + strValEd;
			}else{
				pos = -1;
				dst = strTmp;
			}
		}else if ( strTmp == "odd" || strTmp == "even" ){
			dst = strTmp;
		}else{
			int val1;
			if ( getStrValNum(val1, strTmp, 0) >= 0 ){
				dst = std::to_string(val1);
			}else{
				pos = -1;
				dst = strTmp;
			}
		}
	}else{
		pos = getStrItem(strTmp, cstr, posBak);
		if ( pos >= 0 ){		// リストで取得できないデータが残っている時は文字を入れる
			pos = -1;
			dst = strTmp;
		}else{					// データ終了なら空文字列
			dst.clear();
		}
	}
	return pos;
}
//---------------------------------------------------------------------
// 最大値maxNumの範囲あり数値文字列内にcurNumが存在するか
//---------------------------------------------------------------------
bool CnvStrTime::isStrMultiNumIn(const string &cstr, int curNum, int maxNum){
	bool exist = false;
	int pos = 0;
	while( pos >= 0 && !exist ){
		string strVal;
		pos = getStrWord(strVal, cstr, pos);
		if ( pos < 0 ) break;

		int rloc = (int)strVal.find("..");
		if ( rloc == (int)string::npos ){		// 通常の数値
			int val = stoi(strVal);
			if ( (val == 0) || (val == curNum) || (maxNum + val + 1 == curNum) ){
				exist = true;
			}
		}else if ( strVal == "odd" ){
			if ( curNum % 2 == 1 ){
				exist = true;
			}
		}else if ( strVal == "even" ){
			if ( curNum % 2 == 0 ){
				exist = true;
			}
		}else{								// 範囲指定
			string strSt = strVal.substr(0, rloc);
			string strEd = strVal.substr(rloc+2);
			int valSt = stoi(strSt);
			int valEd = stoi(strEd);
			if ( valSt < 0 ){
				valSt = maxNum + valSt + 1;
			}
			if ( valEd < 0 ){
				valEd = maxNum + valEd + 1;
			}
			if ( (valSt <= curNum) && (curNum <= valEd) ){
				exist = true;
			}
		}
	}
	return exist;
}
//=====================================================================
// 時間を文字列（フレームまたはミリ秒）に変換
//=====================================================================

//---------------------------------------------------------------------
// フレーム数または時間表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1(Msec msec_val){
	bool type_frm = false;
	if (m_unitsec == 0){
		type_frm = true;
	}
	return getStringMsecM1All(msec_val, type_frm);
}

//---------------------------------------------------------------------
// フレーム表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringFrameMsecM1(Msec msec_val){
	return getStringMsecM1All(msec_val, true);
}

//---------------------------------------------------------------------
// 時間表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringTimeMsecM1(Msec msec_val){
	return getStringMsecM1All(msec_val, false);
}

//---------------------------------------------------------------------
// 時間を文字列（フレームまたは時間表記）に変換
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1All(Msec msec_val, bool type_frm){
	string str_val;
	if (msec_val == -1){
		str_val = to_string(-1);
	}
	else if (type_frm){
		int n = getFrmFromMsec(msec_val);
		str_val = to_string(n);
	}
	else{
		int val_abs = (msec_val < 0)? -msec_val : msec_val;
		int val_t = val_abs / 1000;
		int val_h = val_t / 3600;
		int val_m = (val_t / 60) % 60;
		int val_s = val_t % 60;
		int val_x = val_abs % 1000;
		string str_h = getStringZeroRight(val_h, 2);
		string str_m = getStringZeroRight(val_m, 2);
		string str_s = getStringZeroRight(val_s, 2);
		string str_x = getStringZeroRight(val_x, 3);
		str_val = str_h + ":" + str_m + ":" + str_s;
		if (val_x > 0){
			str_val = str_val + "." + str_x;
		}
		if (msec_val < 0){
			str_val = "-" + str_val;
		}
	}
	return str_val;
}


//---------------------------------------------------------------------
// 数値を文字列に変換（上位0埋め）
//---------------------------------------------------------------------
string CnvStrTime::getStringZeroRight(int val, int len){
	string str_val = "";
	for(int i=0; i<len-1; i++){
		str_val += "0";
	}
	str_val += to_string(val);
	return str_val.substr( str_val.length()-len );
}




//=====================================================================
//
// 文字列処理の内部関数
//
//=====================================================================
//--- 指定位置の文字（マルチバイト）バイト数を返す ---
int CnvStrTime::getMbStrSize(const string& str, int n){
	return getMbStrSizeUtf8(str, n);
}
int CnvStrTime::getMbStrSizeSjis(const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0) return 0;
	if ( n >= len-1 ) return 1;
	unsigned char code = (unsigned char) str[n];
	if ((code >= 0x81 && code <= 0x9F) ||
		(code >= 0xE0 && code <= 0xFC)){		// Shift-JIS 1st-byte
		code = (unsigned char) str[n+1];
		if ((code >= 0x40 && code <= 0x7E) ||
			(code >= 0x80 && code <= 0xFC)){	// Shift-JIS 2nd-byte
			return 2;
		}
	}
	return 1;
}
int CnvStrTime::getMbStrSizeUtf8(const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0 ) return 0;
	unsigned char code = (unsigned char) str[n];
	if ( (code & 0x80) == 0 ) return 1;
	for(int i=1; i<4; i++){
		if ( n+i >= len ) return i;
		code = (unsigned char) str[n+i];
		if ( (code & 0xC0) != 0x80 ) return i;
	}
	return 4;
}
//--- n文字目がマルチバイトで2番目以降の文字であればtrueを返す ---
bool CnvStrTime::isStrMbSecond(const string& str, int n){
	if ( n >= (int)str.length() ) return false;	// 異常を除く
	std::mblen(nullptr, 0);		// 変換状態をリセット
	int i = 0;
	while( i<n && i>=0 ){
		int mbsize = getMbStrSize(str, i);
		if ( mbsize <= 0 ){
			return false;
		}
		i += mbsize;
	}
	return ( i != n );
}

//---------------------------------------------------------------------
// 文字列から１単語を読み込み数値（ミリ秒）として格納（数値以外があれば読み込み失敗を返す）
// cstr文字列の位置posから１単語を読み込み、数値をvalに出力
// 入力：
//   cstr : 文字列
//   pos  : 認識開始位置
//   unitsec : 整数部分の単位（0=フレーム数  1=秒数  2=単位変換なし）
// 出力：
//   返り値： 読み終わった位置を返り値とする（失敗時は-1）
//   val    : 数値（ミリ秒）
//---------------------------------------------------------------------
int CnvStrTime::getStrValSub(int &val, const string &cstr, int pos, int unitsec){
	return getStrValSubDelimit(val, cstr, pos, unitsec, DELIMIT_SPACE_ONLY);
}

//---------------------------------------------------------------------
// 区切り選択追加して演算実行
//---------------------------------------------------------------------
int CnvStrTime::getStrValSubDelimit(int &val, const string &cstr, int pos, int unitsec, DelimtStrType type){
	int st, ed;

	pos = getStrItemHubRange(st, ed, cstr, pos, type);
	if ( pos < 0 ) return pos;
	try{
		val = getStrCalc(cstr, st, ed-1, unitsec);
	}
	catch(int errloc){
//		printf("err:%d\n",errloc);
		val = errloc;
		pos = -1;
	}
	return pos;
}


//---------------------------------------------------------------------
// １項目の文字列位置範囲を取得
// 入力：
//   cstr : 文字列
//   pos  : 読み込み開始位置
//   type : 種類（0=スペース区切りQUOTE可  1=スペース区切り  2=1+コンマも区切り  3=最初の数字部分のみ
// 出力：
//   返り値： 読み込み終了位置
//   st   : 認識開始位置
//   ed   : 認識終了位置
//---------------------------------------------------------------------
int CnvStrTime::getStrItemHubRange(int &st, int &ed, const string &cstr, int pos, DelimtStrType type){
	ArgItemType itype = {};
	itype.dstype = type;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	string strDmy;
	return getStrItemCommon(strDmy, st, ed, cstr, pos, itype);
}
//---------------------------------------------------------------------
// 関数タイプ１項目の文字列取得
//---------------------------------------------------------------------
int CnvStrTime::getStrItemHubFunc(string& dstr, const string &cstr, int pos, DelimtStrType dstype){
	ArgItemType itype = {};
	if ( dstype == DELIMIT_FUNC_NAME ){		// 関数名前部分
		itype.dstype = dstype;
		itype.concat = false;
		itype.separate = false;
		itype.remain = false;
		itype.defstr = false;
		itype.qdisp  = false;
		itype.emptyok = false;
	}else{
		itype.dstype = dstype;
		itype.concat = true;
		itype.separate = false;
		itype.remain = true;
		itype.defstr = true;
		itype.qdisp  = false;
		itype.emptyok = false;
	}
	int st, ed;
	return getStrItemCommon(dstr, st, ed, cstr, pos, itype);
}
//--- 取得位置不要で文字列と終了位置を返す ---
int CnvStrTime::getStrItemHubStr(string& dstr, const string &cstr, int pos, ArgItemType itype){
	int st, ed;
	return getStrItemCommon(dstr, st, ed, cstr, pos, itype);
}
//---------------------------------------------------------------------
// １項目の文字列を取得
//---------------------------------------------------------------------
// 文字列と範囲を取得（共通設定）
int CnvStrTime::getStrItemCommon(string& dstr, int &st, int &ed, const string &cstr, int pos, ArgItemType itype){
	if (pos < 0) return pos;

	int pos_before = pos;
	QuoteType qtype = {};

	//--- trim left（空白が区切りの場合、次の項目先頭まで移動） ---
	if ( isCharTypeDelim(' ', itype.dstype) ){
		pos = skipCharSpace(cstr, pos);
	}
	//--- check quote ---
	dstr.clear();
	bool validDquote = isCharValidDquote(itype.dstype);
	bool validSquote = isCharValidSquote(itype.dstype);
	if ( validDquote || validSquote ){
		char ch = cstr[pos];
		bool flagPos = false;
		if ( ch == '\"' && validDquote ){
			flagPos = true;
			qtype.flagQw = true;
			qtype.existQ = true;
			qtype.edgeQw = true;
		}else if ( ch == '\''  && validSquote ){
			flagPos = true;
			qtype.flagQs = true;
			qtype.existQ = true;
			qtype.edgeQw = false;
		}
		if ( flagPos ){
			pos ++;
			if ( itype.qdisp ){
				dstr += ch;
			}
		}
	}
	//--- 開始位置設定 ---
	st = pos;
	ed = pos;
	//--- データ位置確認 ---
	bool flagEnd = false;
	do{
		char ch = cstr[pos];
		if (ch == '\0' || dstr.length() >= SIZE_BUF_MAX-1){	// 強制終了条件
			break;
		}
		QuoteState qstate;
		flagEnd = getStrItemCommonCh(qstate, qtype, ch, dstr.empty(), itype);
		if ( qstate.add ){
			dstr += ch;
			ed = pos + 1;
		}
		if ( qstate.pos || !flagEnd ){
			pos ++;
		}
	} while( !flagEnd );
	//--- 終了位置設定 ---
	if ( qtype.flagQw || qtype.flagQs ) {				// QUOTE異常
		pos = -1;
	}
	else if ( qtype.existQ ){		// QUOTEで囲まれた文字列
		if ( itype.remain && !itype.qdisp ){	// 内部quoteは消すが前後囲みは残す場合
			if ( qtype.edgeQw ){
				dstr = "\"" + dstr + "\"";
			}else{
				dstr = "\'" + dstr + "\'";
			}
		}else if ( !itype.remain && itype.qdisp ){	// 内部quoteは残すが外周quoteは残さない場合
			auto len = dstr.length();
			if ( len == 2 ){
				dstr.clear();
			}else if ( len >= 2){
				char ch = dstr[dstr.length()-1];
				if ( ch == '\"' || ch == '\'' ){
					dstr = dstr.substr(1,len-2);
				}
			}
		}
		if ( itype.defstr ){
			if ( dstr.empty() && pos > st+1 ){
				dstr = "\"\"";		// 出力なしでQUOTE内にQUOTEある場合はQUOTE
			}
		}
	}
	else if ( dstr.empty() ){	// 読み込みデータない場合
		if ( itype.emptyok ){
			if ( pos == pos_before ){	// データなし許可時は最初から文字列最後の場合のみ無効
				pos = -1;
			}
		}else{
			pos = -1;
		}
	}
	return pos;
}
//--- 文字認識 ---
bool CnvStrTime::getStrItemCommonCh(QuoteState& qstate, QuoteType& qtype, char ch, bool yet, ArgItemType itype){
	//--- 引用中は区切り以外はチェックせず出力 ---
	if ( qtype.flagQw || qtype.flagQs ){
		qstate.end = false;
		qstate.add = true;
		qstate.pos = true;
		if ( (qtype.flagQw && ch != '\"') ||
		     (qtype.flagQs && ch != '\'') ){
			return qstate.end;
		}
	}
	//--- 通常の区切り判定 ---
	qstate.end = isCharTypeDelim(ch, itype.dstype);
	qstate.add = !qstate.end;
	qstate.pos = !qstate.end;
	//--- 特殊文字処理 ---
	bool refind_dq = ( isCharValidDquote(itype.dstype) && (itype.concat || itype.separate) );
	bool refind_sq = ( isCharValidSquote(itype.dstype) && (itype.concat || itype.separate) );
	switch(ch){
		case '\"':
			if ( qtype.flagQw ){				// 引用符２回目
				qstate.add = false;
				qstate.pos = true;
				qtype.flagQw = false;
				qtype.edgeQw = true;
				if ( itype.separate || !itype.concat ){
					qstate.end = true;
				}
			}
			else if ( refind_dq ){			// 途中から引用符１回目
				qstate.add = false;
				if ( itype.separate ){		// QUOTEで分離する時
					qstate.end = true;
					qstate.pos = false;
				}else{
					qstate.pos = true;
					qtype.flagQw = true;
					qtype.existQ = true;
					qtype.edgeQw = true;
					if ( itype.dstype == DELIMIT_CSV ){
						qstate.add = true;
					}
				}
			}
			if ( itype.qdisp && qstate.pos ){
				qstate.add = true;
			}
			break;
		case '\'':
			if ( qtype.flagQs ){				// 引用符２回目
				qstate.add = false;
				qstate.pos = true;
				qtype.flagQs = false;
				qtype.edgeQw = false;
				if ( itype.separate || !itype.concat ){
					qstate.end = true;
				}
			}
			else if ( refind_sq ){			// 途中から引用符１回目
				qstate.add = false;
				if ( itype.separate ){		// QUOTEで分離する時
					qstate.end = true;
					qstate.pos = false;
				}else{
					qstate.pos = true;
					qtype.flagQs = true;
					qtype.existQ = true;
					qtype.edgeQw = false;
				}
			}
			if ( itype.qdisp && qstate.pos ){
				qstate.add = true;
			}
			break;
		case '(':
			qtype.numPar ++;
			break;
		case ')':
			qtype.numPar --;
			if ( qtype.numPar < 0 &&
			    ( itype.dstype == DELIMIT_FUNC_ARGS ||
			      itype.dstype == DELIMIT_FUNC_CALC )){
				qstate.end = true;
				qstate.add = yet;		// 先頭文字の時は追加して終了
				qstate.pos = qstate.add;
			}
			break;
		case ',':
			if ( itype.dstype == DELIMIT_SPACE_COMMA ||
			     itype.dstype == DELIMIT_CSV         ){
				qstate.pos = true;		// 次回飛ばす区切り文字を認識
			}
			break;
		default:
			break;
	}
	if ( isCharTypeSpace(ch) && itype.dstype == DELIMIT_FUNC_CALC ){
		qstate.add = false;		// 演算式中のスペースは省略
	}
	return qstate.end;
}
//--- 空白文字を飛ばす ---
int CnvStrTime::skipCharSpace(const string &cstr, int pos){
	//--- trim left（空白が区切りの場合、次の項目先頭まで移動） ---
	while( isCharTypeSpace(cstr[pos]) ){
		pos ++;
	}
	return pos;
}
//---------------------------------------------------------------------
// 文字の種類を取得
//---------------------------------------------------------------------
//--- 種類を取得 ---
CnvStrTime::CharCtrType CnvStrTime::getCharTypeSub(char ch){
	CharCtrType typeMark;

	switch(ch){
		case '\0':
			typeMark = CHAR_CTR_NULL;
			break;
		case ' ':
		case '\t':
			typeMark = CHAR_CTR_SPACE;
			break;
		default:
			if (ch >= 0 && ch < ' '){
				typeMark = CHAR_CTR_CTRL;
			}
			else{
				typeMark = CHAR_CTR_OTHER;
			}
			break;
	}
	return typeMark;
}
//--- 文字が空白をチェック ---
bool CnvStrTime::isCharTypeSpace(char ch){
	CharCtrType typeMark;

	typeMark = getCharTypeSub(ch);
	if (typeMark == CHAR_CTR_SPACE){
		return true;
	}
	return false;
}
//--- 空白か終了をチェック ---
bool CnvStrTime::isCharTypeSpaceEnd(char ch){
	CharCtrType typeMark;

	typeMark = getCharTypeSub(ch);
	if ( typeMark == CHAR_CTR_SPACE || typeMark == CHAR_CTR_NULL ){
		return true;
	}
	return false;
}
//--- 区切り文字かチェック ---
bool CnvStrTime::isCharTypeDelim(char ch, DelimtStrType dstype){
	bool flagDelim = false;
	bool typeSpace = isCharTypeSpace(ch);
	bool typeComma = (ch == ',');
	bool typeNonum = (ch < '0' || ch > '9');
	switch( dstype ){
		case DELIMIT_SPACE_QUOTE:
		case DELIMIT_SPACE_ONLY:
		case DELIMIT_FUNC_ARGS:
			flagDelim = typeSpace;
			break;
		case DELIMIT_SPACE_COMMA:
		case DELIMIT_FUNC_CALC:
			flagDelim = typeSpace || typeComma;
			break;
		case DELIMIT_SPACE_EXNUM:
			flagDelim = typeSpace || typeNonum;
			break;
		case DELIMIT_CSV:
			flagDelim = typeComma;
			break;
		case DELIMIT_FUNC_NAME:
			flagDelim = typeSpace || typeComma || ch == '(' || ch == ')';
			break;
		default:
			break;
	}
	return flagDelim;
}
//--- double quoteが有効な検索かチェック ---
bool CnvStrTime::isCharValidDquote(DelimtStrType dstype){
	return ( dstype == DELIMIT_SPACE_QUOTE ||
	         dstype == DELIMIT_SPACE_COMMA ||
	         dstype == DELIMIT_FUNC_ARGS   ||
	         dstype == DELIMIT_FUNC_CALC   ||
	         dstype == DELIMIT_CSV         );
}
//--- single quoteのみ無効かチェック ---
bool CnvStrTime::isCharValidSquote(DelimtStrType dstype){
	return ( dstype == DELIMIT_SPACE_QUOTE ||
	         dstype == DELIMIT_SPACE_COMMA ||
	         dstype == DELIMIT_FUNC_ARGS   ||
	         dstype == DELIMIT_FUNC_CALC   );
}

//---------------------------------------------------------------------
// 文字列を演算処理してミリ時間を取得
// 入力：
//   cstr : 文字列
//   st   : 認識開始位置
//   ed   : 認識終了位置
//   unitsec : 整数部分の単位（0=フレーム数  1=秒数  2=変換なし）
// 出力：
//   返り値： 演算結果ミリ秒
//---------------------------------------------------------------------
int CnvStrTime::getStrCalc(const string &cstr, int st, int ed, int unitsec){
	return getStrCalcDecode(cstr, st, ed, unitsec, 0);
}

//---------------------------------------------------------------------
// 文字列を演算処理してミリ時間を取得の範囲指定演算
// 入力：
//   cstr : 文字列
//   st   : 認識開始位置
//   ed   : 認識終了位置
//   dsec : 整数時の値（0=フレーム数  1=秒数）
//   draw : 乗除算直後の処理（0=通常  1=単位変換中止  2=変換なし）
// 出力：
//   返り値： 演算結果ミリ秒
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcDecode(const string &cstr, int st, int ed, int dsec, int draw){
	//--- 次に演算を行う２項に分解する ---
	int codeMark_op  = 0;				// 演算子タイプ
	int priorMark_op = 0;				// 演算子優先順位
	int nPar_i    = 0;					// 現在の括弧数
	int nPar_op   = -1;					// 演算のある括弧数の最小値
	int posOpS    = -1;					// 分解する演算子位置（開始）
	int posOpE    = -1;					// 分解する演算子位置（終了）
	int flagHead  = 1;					// 単項演算子フラグ
	int flagTwoOp = 0;					// 2文字演算子
	for(int i=st; i<=ed; i++){
		if (flagTwoOp > 0){				// 前回2文字演算子だった場合は次の文字へ
			flagTwoOp = 0;
			continue;
		}
		int codeMark_i = getStrCalcCodeChar(cstr[i], flagHead);
		if (i < ed){								// 2文字演算子チェック
			int codeMark_two = getStrCalcCodeTwoChar(cstr[i], cstr[i+1], flagHead);
			if (codeMark_two > 0){
				codeMark_i = codeMark_two;
				flagTwoOp = 1;
			}
		}
		if ( codeMark_i == D_CALCOP_ERROR ){	// 演算できない文字
			throw i;
		}
		int categMark_i = getMarkCategory(codeMark_i);
		int priorMark_i = getMarkPrior(codeMark_i);
		if (codeMark_i == D_CALCOP_PARS){			// 括弧開始
			nPar_i ++;
		}
		else if (codeMark_i == D_CALCOP_PARE){		// 括弧終了
			nPar_i --;
			if (nPar_i < 0){						// 括弧の数が合わないエラー
				throw i;
			}
		}
		else if (categMark_i == D_CALCCAT_OP1){		// 単項演算子
			int next_i = ( flagTwoOp > 0 )? i+2 : i+1;
			if ( next_i > ed ){						// 単項演算子の後に何もない
				throw i;
			}
			if ( codeMark_i == D_CALCOP_SEC ||		// S(秒数)
			     codeMark_i == D_CALCOP_FRM ){		// F(フレーム数)
				int codeMarkNext = getStrCalcCodeChar(cstr[next_i], flagHead);
				if ( codeMarkNext != D_CALCOP_PARS ){
					throw i;
				}
			}
		}
		else if (categMark_i == D_CALCCAT_OP2){		// 2項演算子
			if ((nPar_op == nPar_i && priorMark_op <= priorMark_i) ||
				(nPar_op > nPar_i) || posOpS < 0){
				posOpS = i;							// 位置
				posOpE = (flagTwoOp > 0)? i+1 : i;	// 位置
				priorMark_op = priorMark_i;			// 優先順位
				codeMark_op  = codeMark_i;			// 2項演算子データ
				nPar_op      = nPar_i;				// 括弧数
			}
			flagHead = 1;							// 次に現れる文字は単項演算子
		}
		else{										// 数値扱い
			flagHead = 0;							// 単項演算子フラグは消す
			if (posOpS < 0 && (nPar_op > nPar_i || nPar_op < 0)){
				nPar_op = nPar_i;					// 演算子ない場合の括弧数保持
			}
		}
	}
	if (nPar_i != 0){								// 括弧の数が合わないエラー
		throw ed;
	}
	//--- 不要な外側の括弧は外す ---
	int flagLoop = 1;
	while(nPar_op > 0 && flagLoop > 0){				// 括弧外演算がない場合が対象
		int codeMark_s = getStrCalcCodeChar(cstr[st], 1);
		int codeMark_e = getStrCalcCodeChar(cstr[ed], 0);
		if (codeMark_s == D_CALCOP_PARS && codeMark_e == D_CALCOP_PARE){
			st ++;
			ed --;
			nPar_op --;
		}
		else{										// 外側が括弧以外なら終了
			flagLoop = 0;
		}
	}
	//--- 演算の実行 ---
	int dr;
	if (posOpS > 0 && nPar_op == 0){				// 次の処理が2項演算子の場合
		if (posOpS == st || posOpE == ed){			// 前後に項目がない場合はエラー
			throw posOpS;
		}
		int raw2 = draw;
		if (codeMark_op == D_CALCOP_MUL ||			// 乗除算では２項目の単位変換しない
			codeMark_op == D_CALCOP_DIV){
			raw2 = 1;
		}
		int d1 = getStrCalcDecode(cstr, st, posOpS-1, dsec, draw);	// 範囲選択して再デコード
		int d2 = getStrCalcDecode(cstr, posOpE+1, ed, dsec, raw2);	// 範囲選択して再デコード
		dr = getStrCalcOp2(d1, d2, codeMark_op);					// 2項演算処理
	}
	else{											// 次の処理が2項演算子でない場合
		int codeMark_s = getStrCalcCodeChar(cstr[st], 1);
		int categMark_s = getMarkCategory(codeMark_s);
		if (categMark_s == D_CALCCAT_OP1){						// 次の処理が単項演算子の場合
			if (codeMark_s == D_CALCOP_SEC){
				dsec = 1;
			}
			else if (codeMark_s == D_CALCOP_FRM){
				dsec = 0;
			}
			int d1 = getStrCalcDecode(cstr, st+1, ed, dsec, draw);	// 範囲選択して再デコード
			dr = getStrCalcOp1(d1, codeMark_s);					// 単項演算処理
		}
		else{
			dr = getStrCalcTime(cstr, st, ed, dsec, draw);			// 数値時間の取得
//printf("[%c,%d,%d,%d]",cstr[st],dr,st,ed);
		}
	}

	return dr;
}


//---------------------------------------------------------------------
// 文字種類の取得 - 分類
//---------------------------------------------------------------------
int CnvStrTime::getMarkCategory(int code){
	return  (code / 0x1000);
}

//---------------------------------------------------------------------
// 文字種類の取得 - 優先順位
//---------------------------------------------------------------------
int CnvStrTime::getMarkPrior(int code){
	return ((code % 0x1000) / 0x100);
}

//---------------------------------------------------------------------
// 演算用に文字認識
// 入力：
//   ch   : 認識させる文字
//   head : 0=通常  1=先頭文字として認識
// 出力：
//   返り値： 認識コード
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcCodeChar(char ch, int head){
	int codeMark;

	if (ch >= '0' && ch <= '9'){
		codeMark = ch - '0';
	}
	else{
		switch(ch){
			case '.':
				codeMark = D_CALCOP_PERD;
				break;
			case ':':
				codeMark = D_CALCOP_COLON;
				break;
			case '+':
				codeMark = D_CALCOP_PLUS;
				if (head > 0){
					codeMark = D_CALCOP_SIGNP;
				}
				break;
			case '-':
				codeMark = D_CALCOP_MINUS;
				if (head > 0){
					codeMark = D_CALCOP_SIGNM;
				}
				break;
			case '*':
				codeMark = D_CALCOP_MUL;
				break;
			case '/':
				codeMark = D_CALCOP_DIV;
				break;
			case '%':
				codeMark = D_CALCOP_MOD;
				break;
			case '!':
				codeMark = D_CALCOP_NOT;
				break;
			case 'S':
				codeMark = D_CALCOP_SEC;
				break;
			case 'F':
				codeMark = D_CALCOP_FRM;
				break;
			case '(':
				codeMark = D_CALCOP_PARS;
				break;
			case ')':
				codeMark = D_CALCOP_PARE;
				break;
			case '<':
				codeMark = D_CALCOP_CMPLT;
				break;
			case '>':
				codeMark = D_CALCOP_CMPGT;
				break;
			case '&':
				codeMark = D_CALCOP_B_AND;
				break;
			case '^':
				codeMark = D_CALCOP_B_XOR;
				break;
			case '|':
				codeMark = D_CALCOP_B_OR;
				break;
			default:
				codeMark = D_CALCOP_ERROR;
				break;
		}
	}
	return codeMark;
}

//---------------------------------------------------------------------
// 演算用に２文字演算子の文字認識
// 入力：
//   ch1   : 認識させる文字（１文字目）
//   ch2   : 認識させる文字（２文字目）
//   head : 0=通常  1=先頭文字として認識
// 出力：
//   返り値： 認識コード
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcCodeTwoChar(char ch1, char ch2, int head){
	int codeMark = -1;

	switch(ch1){
		case '=':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPEQ;
			}
			else if (ch2 == '<'){
				codeMark = D_CALCOP_CMPLE;
			}
			else if (ch2 == '>'){
				codeMark = D_CALCOP_CMPGE;
			}
			break;
		case '<':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPLE;
			}
			break;
		case '>':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPGE;
			}
			break;
		case '!':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPNE;
			}
			break;
		case '&':
			if (ch2 == '&'){
				codeMark = D_CALCOP_L_AND;
			}
			break;
		case '|':
			if (ch2 == '|'){
				codeMark = D_CALCOP_L_OR;
			}
			break;
		case '+':
			if (ch2 == '+' && head == 1){
				codeMark = D_CALCOP_P_INC;
			}
			break;
		case '-':
			if (ch2 == '-' && head == 1){
				codeMark = D_CALCOP_P_DEC;
			}
			break;
	}
	return codeMark;
}

//---------------------------------------------------------------------
// 文字列をミリ秒時間に変換
// 入力：
//   cstr : 文字列
//   st   : 認識開始位置
//   ed   : 認識終了位置
//   dsec : 整数時の値（0=フレーム数  1=秒数  2=変換なし）
//   draw : 乗除算直後の処理（0=通常  1=単位変換中止）
// 出力：
//   返り値： 演算結果ミリ秒
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcTime(const string &cstr, int st, int ed, int dsec, int draw){

	//--- 文字列から数値を取得 ---
	int categMark_i;
	int codeMark_i;
	int vin = 0;				// 整数部分演算途中
	int val = 0;				// 整数部分数値結果
	int vms = 0;				// ミリ秒部分数値結果
	int flag_sec = 0;			// 1:時間での記載
	int flag_prd = 0;			// ミリ秒のピリオド認識
	int mult_prd = 0;			// ミリ秒の加算単位
	for(int i=st; i<=ed; i++){
		codeMark_i = getStrCalcCodeChar(cstr[i], 0);
		categMark_i = getMarkCategory(codeMark_i);
		if (categMark_i == D_CALCCAT_IMM){			// データ
			if (codeMark_i == D_CALCOP_COLON){		// 時分秒の区切り
				flag_sec = 1;
				val = (val + vin) * 60;
				vin = 0;
			}
			else if (codeMark_i == D_CALCOP_PERD){	// ミリ秒位置の区切り
				flag_sec = 1;
				flag_prd ++;
				mult_prd = 100;
				val += vin;
				vin = 0;
			}
			else{
				if (flag_prd == 0){				// 整数部分
					vin = vin * 10 + codeMark_i;
				}
				else if (flag_prd == 1){		// ミリ秒部分
					vms = codeMark_i * mult_prd + vms;
					mult_prd = mult_prd / 10;
				}
			}
		}
		else{
			throw i;				// 時間を表す文字ではないエラー
		}
	}
	val += vin;
	//--- 単位変換して出力 ---
	int data;
	if (draw > 0 || dsec == 2){		// 単位変換しない場合
		data = val;
	}
	else if (flag_sec == 0){		// 入力文字列は整数データ
		if (dsec == 0){				// 整数時はフレーム単位の設定時
			data = getMsecFromFrm(val);
		}
		else{						// 整数時は秒単位の設定時
			data = val * 1000;
		}
	}
	else{							// 入力文字列は時間データ
		data = val * 1000 + vms;
	}
	return data;
}


//---------------------------------------------------------------------
// 単項演算
// 入力：
//   din   : 演算数値
//   codeMark : 単項演算子
// 出力：
//   返り値： 演算結果ミリ秒
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcOp1(int din, int codeMark){
	int ret;

	switch(codeMark){
		case D_CALCOP_NOT:
			ret = 0;
			if (din == 0){
				ret = 1;
			}
			break;
		case D_CALCOP_SIGNP:
			ret = din;
			break;
		case D_CALCOP_SIGNM:
			ret = din * -1;
			break;
		default:
			ret = din;
			break;
	}
	return ret;
}

//---------------------------------------------------------------------
// ２項演算
// 入力：
//   din1  : 演算数値
//   din2  : 演算数値
//   codeMark : ２項演算子
// 出力：
//   返り値： 演算結果ミリ秒
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcOp2(int din1, int din2, int codeMark){
	int ret;

//printf("(%d,%d,%d)",din1,din2,codeMark);
	switch(codeMark){
		case D_CALCOP_PLUS:
			ret = din1 + din2;
			break;
		case D_CALCOP_MINUS:
			ret = din1 - din2;
			break;
		case D_CALCOP_MUL:
			ret = din1 * din2;
			break;
		case D_CALCOP_DIV:
			if ( din2 == 0 ){
				throw din2;
			}
			ret = din1 / din2;
//			ret = (din1 + (din2/2)) / din2;
			break;
		case D_CALCOP_MOD:
			ret = din1 % din2;
			break;
		case D_CALCOP_CMPLT:
			ret = (din1 < din2)? 1 : 0;
			break;
		case D_CALCOP_CMPLE:
			ret = (din1 <= din2)? 1 : 0;
			break;
		case D_CALCOP_CMPGT:
			ret = (din1 > din2)? 1 : 0;
			break;
		case D_CALCOP_CMPGE:
			ret = (din1 >= din2)? 1 : 0;
			break;
		case D_CALCOP_CMPEQ:
			ret = (din1 == din2)? 1 : 0;
			break;
		case D_CALCOP_CMPNE:
			ret = (din1 != din2)? 1 : 0;
			break;
		case D_CALCOP_B_AND:
			ret = (din1 & din2);
			break;
		case D_CALCOP_B_XOR:
			ret = (din1 ^ din2);
			break;
		case D_CALCOP_B_OR:
			ret = (din1 | din2);
			break;
		case D_CALCOP_L_AND:
			ret = (din1 && din2);
			break;
		case D_CALCOP_L_OR:
			ret = (din1 || din2);
			break;
		default:
			ret = din1;
			break;
	}
	return ret;
}



