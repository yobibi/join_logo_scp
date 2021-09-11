//
// 文字列と時間とフレーム位置の相互変換クラス
//
#include "stdafx.h"

using namespace std;
#include "CnvStrTime.hpp"


//---------------------------------------------------------------------
// 構築時初期設定
//---------------------------------------------------------------------
CnvStrTime::CnvStrTime(){
	m_frate_n = 30000;
	m_frate_d = 1001;
	m_unitsec = 0;
	m_delimiter = "\\";
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
		//--- Shift-JISの2バイト目ではない"\"を検索 ---
		while(nloc >= 0 && flag_find == false){
			if (nloc == 0){
				flag_find = true;			// 先頭が"\"のファイル区切り
			}
			else if (isStrSjisSecond(fullname, nloc) == false){
				flag_find = true;			// 前文字からの２バイト文字でないので区切り
			}
			else{							// ２バイト文字の一部だったら次を検索
				nloc = (int) fullname.rfind("\\", nloc-1);
			}
		}
		if ( flag_find ){
			m_delimiter = "\\";		// 区切り文字変更
		}
	}
	//--- "/"区切りを検索 ---
	int nloc_sl = (int) fullname.rfind("/");
	if (nloc_sl >= 0){
		if (flag_find == false || nloc < nloc_sl){
			flag_find = true;
			nloc = nloc_sl;
			m_delimiter = "/";		// 区切り文字変更
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



//=====================================================================
//  時間とフレーム位置の変換
//  注意点：フレーム位置からの変換は先頭フレームを0とした絶対位置で指定するようにしておく
//=====================================================================

//---------------------------------------------------------------------
// ミリ秒をフレーム数に変換
//---------------------------------------------------------------------
int CnvStrTime::getFrmFromMsec(int msec){
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
int CnvStrTime::getMsecAlignFromMsec(int msec){
	int frm = getFrmFromMsec(msec);
	return getMsecFromFrm(frm);
}

//---------------------------------------------------------------------
// ミリ秒を一度フレーム数に換算した後微調整してミリ秒に変換
//---------------------------------------------------------------------
int CnvStrTime::getMsecAdjustFrmFromMsec(int msec, int frm){
	int frm_new = getFrmFromMsec(msec) + frm;
	return getMsecFromFrm(frm_new);
}

//---------------------------------------------------------------------
// ミリ秒を秒数に変換
//---------------------------------------------------------------------
int CnvStrTime::getSecFromMsec(int msec){
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
int CnvStrTime::getStrValMsec(int &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, m_unitsec);
}

//---------------------------------------------------------------------
// 数値（ミリ秒）を返すが、整数入力は設定にかかわらずフレーム数として扱う
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecFromFrm(int &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, 0);			// unitsec=0:整数時はフレーム数
}

//---------------------------------------------------------------------
// 数値（ミリ秒）を返すが、マイナス１の時は特殊扱いで変換せずそのまま返す
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecM1(int &val, const string &cstr, int pos){
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

//---------------------------------------------------------------------
// 文字列からスペース区切りで１単語を読み込む
//---------------------------------------------------------------------
int CnvStrTime::getStrItem(string &dst, const string &cstr, int pos){
	int st, ed;

	pos = getStrItemRange(st, ed, cstr, pos, DELIMIT_SPACE_QUOTE);
	if (pos >= 0){
		if (ed > st) {
			dst = cstr.substr(st, ed - st);
		}
		else {
			dst.clear();
		}
	}

	return pos;
}

//---------------------------------------------------------------------
// 文字列から１単語を読み込む（スペース以外に","を区切りとして認識）
//---------------------------------------------------------------------
int CnvStrTime::getStrWord(string &dst, const string &cstr, int pos){
	int st, ed;

	pos = getStrItemRange(st, ed, cstr, pos, DELIMIT_SPACE_COMMA);
	if (pos >= 0){
		dst = cstr.substr(st, ed-st);
	}

	return pos;
}

//---------------------------------------------------------------------
// quoteも区切りで残したまま文字列から１単語を読み込む
//---------------------------------------------------------------------
int CnvStrTime::getStrItemWithQuote(string &dst, const string &cstr, int pos){
	int posbk = pos;
	pos = getStrItem(dst, cstr, pos);
	int loc1 = (int)cstr.find("\"", posbk);
	if ( loc1 < posbk || loc1 >= pos || pos < 0 ){	// quoteがなければ終了
		return pos;
	}
	int locd = (int)dst.find("\"");
	if ( locd > 0 && locd < pos ){	// 文字列中にquoteなら手前で終了
		dst = dst.substr(0, locd);
		pos = loc1;
		return pos;
	}
	int loc2 = (int)cstr.find("\"", loc1 + 1);
	if ( loc2 > loc1 && loc2 < pos ){
		dst = cstr.substr(loc1, loc2 - loc1 + 1);
		pos = loc2 + 1;
	}
	return pos;
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
	int poscmt = -1;
	int posdol = -1;
	int len = (int) cstr.length();
	bool flagQw = false;
	for(int i=0; i<len; i++){
		if ( poscmt < 0 ){
			switch( cstr[i] ){
				case '#' :
					if ( (i >= pos && flagQw == false) &&
						 (posdol < 0 || posdol+1 != i) ){
							poscmt = i;
					}
					break;
				case '$' :
					posdol = i;
					break;
				case '\"' :
					flagQw = (flagQw)? false : true;
					break;
				default:
					break;
			}
		}
	}
	return poscmt;
}



//=====================================================================
// 時間を文字列（フレームまたはミリ秒）に変換
//=====================================================================

//---------------------------------------------------------------------
// フレーム数または時間表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1(int msec_val){
	bool type_frm = false;
	if (m_unitsec == 0){
		type_frm = true;
	}
	return getStringMsecM1All(msec_val, type_frm);
}

//---------------------------------------------------------------------
// フレーム表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringFrameMsecM1(int msec_val){
	return getStringMsecM1All(msec_val, true);
}

//---------------------------------------------------------------------
// 時間表記（-1はそのまま残す）
//---------------------------------------------------------------------
string CnvStrTime::getStringTimeMsecM1(int msec_val){
	return getStringMsecM1All(msec_val, false);
}

//---------------------------------------------------------------------
// 時間を文字列（フレームまたは時間表記）に変換
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1All(int msec_val, bool type_frm){
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

//---------------------------------------------------------------------
// 対象位置がシフトJISの2バイト目か判別
//---------------------------------------------------------------------
bool CnvStrTime::isStrSjisSecond(const string &str, int n){
	//--- 最初から2バイト認識していたら除く ---
	for(int i=0; i < (int) str.length(); i++){
		int code = str[n];
		if (code < -128 || code > 255) return false;
	}
	//--- 文字位置を順番にチェック ---
	{
		int i = 0;
		while(i < n){
			if ( isStrSjisMultiByte(str, i) ){
				i++;
			}
			i++;
		}
		if (i != n) return true;
	}
	return false;
}

//---------------------------------------------------------------------
// Shift-JISの２バイト文字チェック
// 2バイト文字だった時は1を返り値とする（1バイト文字は0）
//---------------------------------------------------------------------
bool CnvStrTime::isStrSjisMultiByte(const string &str, int n){
	int code = str[n];
	if (code < 0){
		code = code & 0xFF;
	}
	else if (code >= 0x80){
		return false;
	}
//printf("code:%x\n", code);
	if ((code >= 0x81 && code <= 0x9F) ||
		(code >= 0xE0 && code <= 0xFC)){		// Shift-JIS 1st-byte
		code = (str[n+1] & 0xFF);
		if ((code >= 0x40 && code <= 0x7E) ||
			(code >= 0x80 && code <= 0xFC)){	// Shift-JIS 2nd-byte
			return true;
		}
	}
	return false;
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

	pos = getStrItemRange(st, ed, cstr, pos, type);
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
int CnvStrTime::getStrItemRange(int &st, int &ed, const string &cstr, int pos, DelimtStrType type){
	if (pos < 0) return pos;

	char ch;
	int flag_quote = 0;
	int len = 0;

	//--- trim left ---
	while( isCharTypeSpace(cstr[pos]) ){
		pos ++;
	}
	//--- check quote ---
	if (cstr[pos] == '\"' && type == DELIMIT_SPACE_QUOTE){
		flag_quote ++;
		pos ++;
	}
	//--- 開始位置設定 ---
	st = pos;
	//--- データ位置確認 ---
	int flag_end = 0;
	do{
		ch = cstr[pos];
		if (ch == '\"' && flag_quote > 0){				// 引用符２回目
			flag_quote ++;
			flag_end = 1;
		}
		else if (isCharTypeSpace(ch) ||				// スペース
			(ch == ',' && type == DELIMIT_SPACE_COMMA) ||					// コンマ
			((ch < '0' || ch > '9') && type == DELIMIT_SPACE_EXNUM)){		// 数字以外
			if (type != DELIMIT_SPACE_QUOTE || flag_quote != 1){
				flag_end = 1;				// type=DELIMIT_SPACE_QUOTEでは引用中確認
			}
		}
		else if (ch == '\0' || len >= SIZE_BUF_MAX-1){	// 強制終了条件
			flag_end = 1;
		}
		if (flag_end == 0){
			pos ++;
			len ++;
		}
	} while(flag_end == 0);
	//--- 終了位置設定 ---
	ed = pos;
	if (flag_quote == 1) {					// QUOTE異常
		pos = -1;
	}
	else if (st == ed && flag_quote <= 1){	// 読み込みデータない場合
		pos = -1;
	}
	else if (ch == ',' || ch == '\"'){		// 区切り文字は次回スキップ
		pos ++;
	}
	return pos;
}

//---------------------------------------------------------------------
// 文字の種類を取得
//---------------------------------------------------------------------
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


//---------------------------------------------------------------------
// 文字が空白をチェック
//---------------------------------------------------------------------
bool CnvStrTime::isCharTypeSpace(char ch){
	CharCtrType typeMark;

	typeMark = getCharTypeSub(ch);
	if (typeMark == CHAR_CTR_SPACE){
		return true;
	}
	return false;
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



