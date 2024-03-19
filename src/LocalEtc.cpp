//
// Copyright (c) 2024 Yobi
// Released under the MIT License
// http://opensource.org/licenses/mit-license.php
//
// 環境依存で面倒そうな処理をまとめたもの
//
//#include "stdafx.h"

#include "LocalEtc.hpp"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#if defined(LOCALETC_USE_ICONV_SJIS)
#include <iconv.h>
#endif

using namespace std;
using namespace LocalEtcCore;
using namespace LocalEtcCore::LcParam;

//---------------------------------------------------------------------
// 共通利用の標準出力／エラー変数
//---------------------------------------------------------------------
namespace LocalEtcCore
{
	LocalEtcCore::LocalOutStream lcout;
	LocalEtcCore::LocalErrStream lcerr;
	LocalEtcCore::LocalSys       LSys;
	LocalEtcCore::LocalStr       LStr;
	LocalEtcCore::LocalWbCnv     LWbCnv;
}
namespace LocalEtc
{
	LocalEtcCore::LocalOutStream& lcout  = LocalEtcCore::lcout;
	LocalEtcCore::LocalErrStream& lcerr  = LocalEtcCore::lcerr;
	LocalEtcCore::LocalSys&       LSys   = LocalEtcCore::LSys;
	LocalEtcCore::LocalStr&       LStr   = LocalEtcCore::LStr;
}

//---------------------------------------------------------------------
// システム制御
//---------------------------------------------------------------------
LocalEtcCore::LocalSys::LocalSys(){
	m_utfSys = UtfType::standard;
	m_nMemoHold = 0;
	m_nMsgUtf = 0;
	m_strMsgUtf.clear();
	m_listMemo.clear();
}
//
// 標準出力／標準エラー処理
//
void LocalEtcCore::LocalSys::bufcout(const string& buf){
	cout << wbc.cnvToFileString(buf, m_utfSys);
	bufMemoInsSel(buf, true, false);	// 内部メモ設定
}
void LocalEtcCore::LocalSys::bufcerr(const string& buf){
	cerr << wbc.cnvToFileString(buf, m_utfSys);
	bufMemoInsSel(buf, false, true);	// 内部メモ設定
}
void LocalEtcCore::LocalSys::bufMemoIns(const string& buf){
	bufMemoInsSel(buf, false, false);
}
void LocalEtcCore::LocalSys::bufMemoInsSel(const string& buf, bool chkStd, bool chkErr){
	if ( !isMemoHoldStd() && chkStd ) return;
	if ( !isMemoHoldErr() && chkErr ) return;
	m_listMemo.push_back(buf);		// ログとして保管
}
bool LocalEtcCore::LocalSys::isMemoHoldStd(){
	return ( (m_nMemoHold & 0x01) != 0 );
}
bool LocalEtcCore::LocalSys::isMemoHoldErr(){
	return ( (m_nMemoHold & 0x02) != 0 );
}
bool LocalEtcCore::LocalSys::isMsgUtfUndecided(){
	return ( (m_nMsgUtf & 0x01) != 0 );
}
void LocalEtcCore::LocalSys::bufMemoFlush(LocalOfs& ofs){
	int sizeLine = (int)m_listMemo.size();
	for(int i=0; i<sizeLine; i++){
		if ( ofs.is_open() ){
			ofs.write(m_listMemo[i]);	// ログをファイルに出力
		}else{
			cout << wbc.cnvToFileString(m_listMemo[i], m_utfSys);
		}
	}
	m_listMemo.clear();
}
//--- 文字コード関連のシステム表示 ---
void LocalEtcCore::LocalSys::bufOutCodeMsg(const string& msg){
	//--- 前回と全く同じメッセージの時は再表示しない ---
	if ( msg == m_strMsgUtf ) return;
	m_strMsgUtf = msg;
	//--- 標準出力に表示するか選択し内部メモも出力 ---
	bool needMemo = true;
	if ( isMsgUtfUndecided() ){
		bufcout(msg);
		if ( isMemoHoldStd() ){	// 既に内部メモにも出力しているか確認
			needMemo = false;
		}
	}
	if ( needMemo ){
		bufMemoIns(msg);
	}
}
//
// フォルダ作成
//
bool LocalEtcCore::LocalSys::cmdMkdir(const string& strName){
	filesystem::path fname = wbc.cnvToPathString(strName);
	return filesystem::create_directory(fname);
}
//
// ファイルコピー
//
bool LocalEtcCore::LocalSys::cmdCopy(const string& strFrom, const string& strTo){
	filesystem::path fstrFrom = wbc.cnvToPathString(strFrom);
	filesystem::path fstrTo   = wbc.cnvToPathString(strTo);
	return filesystem::copy_file(fstrFrom, fstrTo, filesystem::copy_options::overwrite_existing);
}
//
// カレントフォルダ取得
//
string LocalEtcCore::LocalSys::getCurrentPath(){
	filesystem::path fpath = filesystem::current_path();
	return wbc.cnvUtf8FromFileSystemPath(fpath);
}
//
// 環境変数を取得
//   WindowsでUnicode文字列を使いたければワイドバイトで取得してUTF-8に変換する
//
bool LocalEtcCore::LocalSys::getEnvString(string& strVal, const string& strEnvName){
#if defined(_WIN32) && defined(LOCALETC_USE_SAFETY_CALL)		// ワイドバイトによるC11定義の取得
	wstring wname = wbc.getWstrFromUtf8(strEnvName);
	size_t retSize;
	_wgetenv_s(&retSize, NULL, 0, wname.c_str());
	bool success = false;
	if ( retSize > 0 ){
		wchar_t* wbuf = new(std::nothrow) wchar_t[retSize];
		if ( wbuf != nullptr ){
			_wgetenv_s(&retSize, wbuf, retSize, wname.c_str());
			strVal = wbc.getUtf8FromWstr(wbuf);
			delete [] wbuf;
			success = true;
		}
	}
	if ( success ) return true;
#elif defined(_WIN32)		// ワイドバイトによる取得
	wstring wname = wbc.getWstrFromUtf8(strEnvName);
	const wchar_t *pstr = _wgetenv(wname.c_str());
	if ( pstr != nullptr ){
		strVal = wbc.getUtf8FromWstr(pstr);
		return true;
	}
#elif defined(LOCALETC_USE_SAFETY_CALL)		// C11定義の安全な取り込み（C++では実装依存）
	size_t retSize;
	getenv_s(&retSize, NULL, 0, strEnvName.c_str());
	if ( retSize > 0 ){
		char* buf = new(std::nothrow) char[retSize];
		if ( buf == nullptr ) return false;
		getenv_s(&retSize, buf, retSize, strEnvName.c_str());
		strVal = buf;
		delete [] buf;
		return true;
	}
#else
	const char *pstr = getenv(strEnvName.c_str());
	if ( pstr != nullptr ){
		strVal = pstr;
		return true;
	}
#endif
	strVal = "";
	return false;
}
//
// 起動時の引数を取得
//   設定なければ起動時のmain引数をそのまま使用する
//   WindowsでUnicode文字列を使いたければワイドバイト(Windows用)で取得してUTF-8に変換
//
vector<string> LocalEtcCore::LocalSys::getMainArg(int argc, char *argv[]){
	vector<string> listArg;
#if defined(_WIN32)			// ワイドバイトによる引数取得(MS-windows)
	//--- windowsコマンドで引数取得 ---
	wchar_t* lpLine = ::GetCommandLineW();
	int nArgc = 0;
	wchar_t** lppArgv = ::CommandLineToArgvW( lpLine, &nArgc );
	for(int i=0; i<nArgc; i++){
		string str = wbc.getUtf8FromWstr(lppArgv[i]);
		listArg.push_back(str);
	}
#else		// 設定なければmain引数をそのまま使用
	for(int i=0; i<argc; i++){
		string str = argv[i];
		listArg.push_back(str);
	}
#endif
	return listArg;
}
//
// パス区切り文字を取得
//
string LocalEtcCore::LocalSys::getPathDelimiter(){
	return LcParam::delimPath;
}
//
// ログ情報保持設定
//
//--- 内部メモに標準出力／標準エラー内容を追加する設定 ---
void LocalEtcCore::LocalSys::setMemoSel(int n){
	m_nMemoHold = n;
	if ( n == -1 ){
		m_listMemo.clear();
		m_nMemoHold = 0;
	}
}
//--- 文字コード関連のシステム表示設定 ---
void LocalEtcCore::LocalSys::setMsgUtf(int n){
	if ( m_nMsgUtf != n ){
		m_strMsgUtf.clear();	// 設定を変更したら重複確認の文字列はクリア
	}
	m_nMsgUtf = n;
}
//
// 文字コードの番号-文字列間を変換
//
string LocalEtcCore::LocalSys::getUtfStrFromNum(int num){
	return wbc.varUtfStrFromNum(num);
}
int LocalEtcCore::LocalSys::getUtfNumFromStr(const string& strUtf){
	return wbc.varUtfNumFromStr(strUtf);
}
bool LocalEtcCore::LocalSys::isUtfNumValid(int num){
	return wbc.isUtfNumValid(num);
}
//
// 標準コードの設定を変更
//
void LocalEtcCore::LocalSys::setUtfDefaultNum(int num){
	if ( isUtfNumValid(num) ){
		LcParam::UtfType utype = wbc.varUtfCodeFromNum(num);
		wbc.setDefaultUtfStdCode(utype);		// 変換処理のデフォルト設定を変更
	}
}
int LocalEtcCore::LocalSys::getUtfDefaultNum(){
	LcParam::UtfType utype = wbc.getUtfTypeRevised(UtfType::standard);
	return wbc.varUtfNumFromCode(utype);
}
//
// 標準出力／標準エラーの文字コード設定
//
void LocalEtcCore::LocalSys::setSysUtfNum(int num){
	if ( isUtfNumValid(num) ){
		m_utfSys = wbc.varUtfCodeFromNum(num);
	}
}
int LocalEtcCore::LocalSys::getSysUtfNum(){
	return wbc.varUtfNumFromCode(m_utfSys);
}
//
// ファイル出力のデフォルト文字コード設定
//
void LocalEtcCore::LocalSys::setFileUtfNum(int num){
	if ( isUtfNumValid(num) ){
		LcParam::UtfType utype = wbc.varUtfCodeFromNum(num);
		wbc.setFileDefaultUtfCode(utype);
	}
}
int LocalEtcCore::LocalSys::getFileUtfNum(){
	LcParam::UtfType utype = wbc.getFileDefaultUtfCode();
	return wbc.varUtfNumFromCode(utype);
}
//
// デバッグ用
//
void LocalEtcCore::LocalSys::echoCodeWB(wstring str, int len){	// WideByte文字列を16進数で表示
	if ( len < 0 ) len = (int)str.length();
	cout << "len(WB):" << len << "\n";
	for(int i=0; i<len; i++){
		wchar_t ch = str[i];
		fprintf(stdout, "%04x ", (unsigned int)ch );
	}
	cout << "\n";
}
void LocalEtcCore::LocalSys::echoCodeByte(string str, int len){	// UTF-8の文字列を16進数で表示
	if ( len < 0 ) len = (int)str.length();
	cout << "len(byte):" << len << "\n";
	for(int i=0; i<len; i++){
		unsigned char ch = str[i];
		fprintf(stdout, "%02x ", (unsigned int)ch );
	}
	cout << "\n";
}
//---------------------------------------------------------------------
// 標準ストリーム(lcout/lcerr用)
//---------------------------------------------------------------------
//--- 内部処理用 ---
iostream::int_type LocalEtcCore::LocalUtf8StreamBuf::overflow(iostream::int_type ich){
	if ( ich == EOF ) return ich;
	if ( m_pos < 0 || m_pos >= 4 ){		// 念のため範囲内確認
		m_pos = 0;
	}
	char ch = (char) ich;
	m_buf[m_pos++] = ch;
	if ( m_pos == 1 ){		// 先頭文字
		m_size = wbc.getNeedByteFromUtf8Head(ch);
	}
	if ( m_pos == m_size ){		// UTF-8の文字単位で送信
		xsputn(m_buf, m_size);
		m_pos = 0;
	}else if ( m_size < 0 ){
		m_pos = 0;
	}
	return 1;
}
std::streamsize LocalEtcCore::LocalOutStreamBuf::xsputn(const iostream::char_type* s, std::streamsize count){
	string str(s, count);
	LSys.bufcout(str);
	return count;
}
std::streamsize LocalEtcCore::LocalErrStreamBuf::xsputn(const iostream::char_type* s, std::streamsize count){
	string str(s, count);
	LSys.bufcerr(str);
	return count;
}
std::streamsize LocalEtcCore::LocalOfsStreamBuf::xsputn(const iostream::char_type* s, std::streamsize count){
	string str(s, count);
	m_ofs->write(str);
	return count;
}
void LocalEtcCore::LocalOfsStreamBuf::setStreamBufOfs(LocalOfs *ofs){
	m_ofs = ofs;
}
//---------------------------------------------------------------------
// ファイルタイプ情報
//---------------------------------------------------------------------
//--- ファイルのBOMから設定 ---
bool LocalEtcCore::LocalFileType::setFromFileBom(ifstream& ifs, const string& strName){
	filesystem::path fname = wbc.cnvToPathString(strName);
	//--- BOM取得 ---
	ifs.open(fname);
	if ( ifs.fail() ) return false;
	unsigned long bomid = 0;
	for(int i=0; i<4; i++){
		unsigned char ch = ifs.get();
		bomid = ( bomid << 8 ) + (unsigned long) ch;
	}
	//--- BOM判定 ---
	LcParam::UtfType selUtf;
	bool selBom = false;
	if ( (bomid >> 8) == 0xEFBBBF ){
		selUtf = UtfType::UTF8;
		selBom = true;
	}
	else if ( (bomid >> 16) == 0xFFFE ){
		selUtf = UtfType::UTF16LE;
		selBom = true;
	}
	else if ( (bomid >> 16) == 0xFEFF ){
		selUtf = UtfType::UTF16BE;
		selBom = true;
	}
	else{
		selUtf = UtfType::standard;		// BOM未存在時は標準設定としておく
		selBom = false;
	}
	setDirect(selUtf);
	if ( !selBom ) m_bom = 0;	// BOM未存在時
	ifs.close();
	return true;
}
//--- ファイル内容から設定 ---
bool LocalEtcCore::LocalFileType::setFromFile(ifstream& ifs, const string& strName){
	m_unfix = false;	// 不確実状態クリア
	//--- BOM確認 ---
	if ( !setFromFileBom(ifs, strName) ){
		return false;
	}
	if ( m_bom != 0 ){	// BOM存在時は終了
		return true;
	}
	//--- BOMがない時のファイル内容確認 ---
	filesystem::path fname = wbc.cnvToPathString(strName);
	ifs.open(fname);
	if ( ifs.fail() ) return false;
	LcParam::UtfType selUtf;
	bool onlyAscii = true;
	bool limitSjis = false;
	bool nochange = false;
	bool det = false;
	while( !det ){
		string lfbuf;
		if ( !std::getline(ifs, lfbuf) ){		// ファイル終了
			break;
		}
		bool okAscii = wbc.isCodeOkAsAscii(lfbuf);
		bool okUtf8 = wbc.isCodeOkAsUtf8(lfbuf);
		bool okSjis = wbc.isCodeOkAsSjis(lfbuf);
		bool okSjisLimit = wbc.isCodeOkAsSjisLimit(lfbuf);
		if ( !okAscii ){
			onlyAscii = false;
		}
		if ( okUtf8 && !okSjis ){
			selUtf = UtfType::UTF8N;
			det = true;
		}
		if ( !okUtf8 && okSjis ){
			selUtf = UtfType::SJIS;
			det = true;
		}
		if ( okSjis && !okSjisLimit ){	// 半角カナ系統を認識した場合
			limitSjis = true;
		}
		if ( !okUtf8 && !okSjis ){	// どちらのコードにも合わなければ何もしない（標準設定とする）
			m_unfix = true;		// 文字化けしている可能性あり
			nochange = true;
			det = true;
		}
	}
	if ( nochange ){
		// 何も変更しない
	}else if ( det ){		// どちらか片方に確定時
		setDirect(selUtf);
	}else if ( onlyAscii ){		// 文字コード無関係のASCIIコード時
		LcParam::UtfType utype = wbc.getUtfTypeRevised(utf());
		if ( utype != UtfType::SJIS ){	// 標準がShift-JIS以外の時はUTF-8としておく
			utype = UtfType::UTF8N;
		}
		setDirect(utype);
	}else{
		m_unfix = true;		// 文字化けしている可能性あり
		if ( limitSjis ){
			// どちらのコードにも合うがShift-JISに半角カナを使っている場合はUTF-8として扱う
			setDirect(UtfType::UTF8N);
		}else{
			// どちらのコードにも合う場合、Shift-JISとして扱う
			setDirect(UtfType::SJIS);
		}
	}
	ifs.close();
	return true;
};
//--- 直接設定 ---
void LocalEtcCore::LocalFileType::setDirect(LcParam::UtfType utfcode){
	m_utf = utfcode;

	LcParam::UtfType urev = wbc.getUtfTypeRevised(utfcode);
	switch( urev ){
		case UtfType::UTF8 :
			m_unit = 1;
			m_bom = 3;
			m_strbom.assign({(char)0xEF, (char)0xBB, (char)0xBF});
			break;
		case UtfType::UTF16LE :
			m_unit = 2;
			m_bom = 2;
			m_strbom.assign({(char)0xFF, (char)0xFE});
			break;
		case UtfType::UTF16BE :
			m_unit = 2;
			m_bom = 2;
			m_strbom.assign({(char)0xFE, (char)0xFF});
			break;
		default :
			m_unit = 1;
			m_bom = 0;
			break;
	}
	m_set = true;
}
//--- 設定されているか初期状態か判定 ---
bool LocalEtcCore::LocalFileType::isSet(){
	return m_set;
}
//---------------------------------------------------------------------
// ifstream処理
//---------------------------------------------------------------------
void LocalEtcCore::LocalIfs::open(const string& strName, std::ios::openmode mode){
	//--- ファイル種類取得 ---
	if ( !attr.setFromFile(ifs, strName) ){
		return;
	}
	if ( attr.isUnfix() ){	// 文字コードが不確実な場合
		string msg = "info: undecided code (select ";
		int num = wbc.varUtfNumFromCode(attr.utf());
		msg += wbc.varUtfStrFromNum(num);
		msg += ") filename: " + strName + "\n";
		LSys.bufOutCodeMsg(msg);		// 判別不可の情報を出力
	}
	//--- Windowsパス対応 ---
	filesystem::path fname = wbc.cnvToPathString(strName);
	//--- open処理 ---
	if ( attr.unit() != 2 ){
		ifs.open(fname, mode);
	}else{		// 2バイト単位読み出しはバイナリ処理
		ifs.open(fname, mode | ios::binary);
	}
	if ( !ifs ) return;
	ifs.ignore(attr.bom());	// BOM読み飛ばし
	return;
}
void LocalEtcCore::LocalIfs::open(const string& strName){
	open(strName, ios::in);
}
bool LocalEtcCore::LocalIfs::getline(string& buf){
	string lfbuf;
	if ( !getlineCore(lfbuf) ){
		return false;
	}
	buf = wbc.cnvFromFileString(lfbuf, attr.utf());
	if ( !buf.empty() ){
		char chBack = buf.back();
		if ( chBack == 0x0A || chBack == 0x0D ){	// 改行が残っていたらカット
			buf.pop_back();
		}
	}
	return true;
}
bool LocalEtcCore::LocalIfs::getlineCore(string& buf){	// 改行コードの補正付き
	if ( attr.unit() != 2 ){
		if ( std::getline(ifs, buf) ){		// 2バイト単位以外は通常読み出し
			return true;
		}
		return false;
	}
	//--- 2バイト単位は改行コードを認識するため別途作成 ---
	buf.clear();
	auto posBase = ifs.tellg();
	auto posCur  = posBase;
	unsigned int codeCR = 0x000D;
	unsigned int codeLF = 0x000A;
	int sw = ( attr.utf() == UtfType::UTF16LE )? 1 : 0;
	bool cont = true;
	bool needBk = false;
	bool flagCR = false;
	bool flagLF = false;
	while( cont && !needBk && !flagLF ){
		posCur = ifs.tellg();
		unsigned char ch1 = ifs.get();
		unsigned char ch2 = ifs.get();
		unsigned int codeCur;
		if ( sw ){
			codeCur = (((unsigned int)ch2) << 8) + ((unsigned int)ch1);
		}else{
			codeCur = (((unsigned int)ch1) << 8) + ((unsigned int)ch2);
		}
		if ( ifs.fail() ){
			cont = false;
		}else if ( ifs.eof() ){
			cont = false;
		}else if ( codeCur == codeCR ){
			if ( flagCR ){
				needBk = true;
			}else{
				flagCR = true;
			}
		}else if ( codeCur == codeLF ){
			flagLF = true;
		}else if ( flagCR ){
			needBk = true;
		}else{
			buf += ch1;
			buf += ch2;
		}
	}
	if ( needBk ){
		ifs.seekg(posCur, ios_base::beg);
	}
	if ( posBase == posCur ){
		if ( !flagCR && !flagLF ) return false;
	}
	return true;
}
void LocalEtcCore::LocalIfs::close(){
	ifs.close();
}
int LocalEtcCore::LocalIfs::getUtfNum(){
	return wbc.varUtfNumFromCode(attr.utf());
}
//---------------------------------------------------------------------
// ofstream処理
//---------------------------------------------------------------------
void LocalEtcCore::LocalOfs::open(const std::string& strName, ios::openmode mode){
	//--- ファイル種類設定 ---
	if ( !attr.isSet() ){		// コード未設定ならファイル出力のデフォルト文字コードを使用
		attr.setDirect(wbc.getFileDefaultUtfCode());
	}
	//--- Windowsパス対応 ---
	filesystem::path fname = wbc.cnvToPathString(strName);
	//--- open処理 ---
	if ( attr.unit() != 2 ){
		ofs.open(fname, mode);
	}else{		// WideByteはバイナリ処理
		ofs.open(fname, mode | ios::binary);
	}
	if ( !ofs ) return;
	bool append = ( (mode & ios::app) != 0 );
	if ( attr.bom() > 0 && !append ){
		ofs << attr.strbom();	// BOM設定
	}
}
void LocalEtcCore::LocalOfs::open(const string& strName){
	open(strName, ios::out);
}
void LocalEtcCore::LocalOfs::append(const string& strName){
	open(strName, ios::app);
}
bool LocalEtcCore::LocalOfs::write(const string& buf){
	string revbuf = writeRevStr(buf);		// WideByteコードはバイナリで処理するため補正が必要
	string lfbuf = wbc.cnvToFileString(revbuf, attr.utf());
	ofs << lfbuf;
	return true;
}
string LocalEtcCore::LocalOfs::writeRevStr(const string& buf){		// WideByteコード対応補正
	if ( attr.unit() != 2 ) return buf;	// 通常テキスト処理は何もしない

//--- バイナリ処理では、WindowsのみLFをCR+LFに変える処理 ---
	string revbuf = buf;
#if defined(_WIN32)
	unsigned char chCR = 0x0D;
	unsigned char chLF = 0x0A;
	auto pos = revbuf.rfind(chLF);
	while( pos != string::npos ){
		bool needCR = true;
		if ( pos > 0 ){		// 既にCR+LFになっていたら挿入しない
			if ( revbuf[pos-1] == chCR ) needCR = false;
		}
		if ( needCR ){
			revbuf.insert(pos, 1, chCR);
		}
		pos = revbuf.rfind(chLF, pos-1);
	}
#endif
	return revbuf;
}	
void LocalEtcCore::LocalOfs::close(){
	ofs.close();
}
void LocalEtcCore::LocalOfs::setUtfNum(int num){
	LcParam::UtfType utfcode = wbc.varUtfCodeFromNum(num);
	attr.setDirect(utfcode);
}
//---------------------------------------------------------------------
// UTF-8の文字列操作
//---------------------------------------------------------------------
//--- 指定位置の部分文字列を返す（最後までと指定文字数で共通処理） ---
string LocalEtcCore::LocalStr::getSubStrCommon(const string& str, int st, int len, bool validLen){
	u32string wstr = wbc.getU32strFromUtf8(str);
	int wlen = (int)wstr.length();
	string strSub;
	if ( wlen > 0 ){
		int wlocS = ( st >= 0 )? st : wlen - st;
		int wlocE = wlen;
		if ( validLen ){
			wlocE = ( len >= 0 )? wlocS + len : wlen - len;
		}
		bool valid = true;
		if ( wlocS >= wlen ) valid = false;
		if ( wlocE < wlocS ) valid = false;
		if ( wlocE > wlen  ) wlocE = wlen;
		if ( valid ){
			u32string wsub = wstr.substr(wlocS, wlocE - wlocS);
			strSub = wbc.getUtf8FromU32str(wsub);
		}
	}
	return strSub;
}
//--- 実際の文字列長を返す ---
int LocalEtcCore::LocalStr::getStrLen(const string& str){
	return wbc.getU32lenFromUtf8(str);
}
//--- 指定位置の部分文字列を返す（指定位置から最後まで） ---
string LocalEtcCore::LocalStr::getSubStr(const string& str, int st){
	return getSubStrCommon(str, st, 0, false);		// validLen=false
}
//--- 指定位置の部分文字列を返す（指定位置から指定文字数） ---
string LocalEtcCore::LocalStr::getSubStrLen(const string& str, int st, int len){
	return getSubStrCommon(str, st, len, true);		// validLen=true
}
//--- 文字リストに含まれる文字の数を返す ---
int LocalEtcCore::LocalStr::countInStr(const string& strSrc, const string& strEach){
	int count = 0;
	u32string wstrSrc = wbc.getU32strFromUtf8(strSrc);
	u32string wstrEach = wbc.getU32strFromUtf8(strEach);
	for(int i=0; i<(int)wstrSrc.length(); i++){
		char32_t wchSrc = wstrSrc[i];
		for(int j=0; j<(int)wstrEach.length(); j++){
			if ( wstrEach[j] == wchSrc ){
				count ++;
				break;
			}
		}
	}
	return count;
}
//--- 文字リスト(strEach)の文字を対応する指定文字(strSub)に変換 ---
bool LocalEtcCore::LocalStr::replaceInStr(string& strSrc, const string& strEach, const string& strSub){
	u32string wstrSrc  = wbc.getU32strFromUtf8(strSrc);
	u32string wstrEach = wbc.getU32strFromUtf8(strEach);
	u32string wstrSub  = wbc.getU32strFromUtf8(strSub);
	if ( wstrEach.length() != wstrSub.length() ) return false;
	for(int i=0; i<(int)wstrSrc.length(); i++){
		char32_t wchSrc = wstrSrc[i];
		for(int j=0; j<(int)wstrEach.length(); j++){
			if ( wstrEach[j] == wchSrc ){
				wstrSrc[i] = wstrSub[j];
				break;
			}
		}
	}
	strSrc = wbc.getUtf8FromU32str(wstrSrc);
	return true;
}
//--- 正規表現でマッチした位置の合計を返す ---
int LocalEtcCore::LocalStr::countRegExMatch(const string& strSrc, const string& strRe){
	//--- 汎用正規表現使用にはワイドバイトが必要なため変換 ---
	wstring wstrSrc = wbc.getWstrFromUtf8(strSrc);
	wstring wstrRe  = wbc.getWstrFromUtf8(strRe);
	if ( wstrSrc.empty() || wstrRe.empty() ){
		return 0;
	}
	//--- ワイドバイトで正規表現の確認 ---
	int count = 0;
	std::wregex re(wstrRe);
	auto iter = wstrSrc.cbegin();
	auto end  = wstrSrc.cend();
	std::wsmatch m;
	while( std::regex_search(iter, end, m, re) ){
		count ++;
		iter = m[0].second;
	}
	return count;
}
//--- 正規表現でマッチした文字列を返す ---
string LocalEtcCore::LocalStr::getRegMatch(const string& strSrc, const string& strRe){
	wstring wstrSrc = wbc.getWstrFromUtf8(strSrc);
	wstring wstrRe  = wbc.getWstrFromUtf8(strRe);
	string strRet;
	if ( wstrSrc.empty() || wstrRe.empty() ){
		return strRet;
	}
	//--- ワイドバイトで正規表現の確認 ---
	std::wregex re(wstrRe);
	std::wsmatch m;
	if ( std::regex_search(wstrSrc, m, re) ){
		wstring wstrRet = m[0].str();
		strRet = wbc.getUtf8FromWstr(wstrRet);
	}
	return strRet;
}


//---------------------------------------------------------------------
// UTF-8 - 他形式 文字列変換処理
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// Shift-JIS処理
//---------------------------------------------------------------------
#if defined(LOCALETC_USE_WINOS_SJIS)		// Windows-OSによるShift-JIS処理
//--- Shift-JIS -> WideByte （Windows専用） ---
wstring LocalEtcCore::LocalWbCnv::getWstrFromSjis(const string& str){
	wstring wstr;
	// Windowsコマンド(CP_ACP=932 : Shift-JIS)
	const int wlen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	wchar_t* wbuf = new(std::nothrow) wchar_t[wlen];
	if ( wbuf == nullptr ) return wstr;
	if ( ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuf, wlen) ){
		wstr = wbuf;
	}
	delete [] wbuf;
	return wstr;
}
//--- WideByte -> Shift-JIS （Windows専用） ---
string LocalEtcCore::LocalWbCnv::getSjisFromWstr(const wstring& wstr){
	string str;
	// Windowsコマンド(CP_ACP=932 : Shift-JIS)
	const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* buf = new(std::nothrow) char[len];
	if ( buf == nullptr ) return str;
	if ( ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf, len, nullptr, nullptr) ){
		str = buf;
	}
	delete [] buf;
	return str;
}
#endif
#if defined(LOCALETC_USE_ICONV_SJIS)	// iconvライブラリによる変換
//--- iconvによるShift-JIS関連変換処理コア ---
string LocalEtcCore::LocalWbCnv::getIconvStr(const string& strSrc, const string& encDst, const string& encSrc){
	string strResult;
	size_t lenSrc = strSrc.length();
	size_t lenDst = lenSrc * 3;
	char* bufSrc = new(std::nothrow) char[lenSrc+1];
	if ( bufSrc == nullptr ){
		return strResult;
	}
	strcpy(bufSrc, strSrc.c_str());
	char* bufDst = new(std::nothrow) char[lenDst+1];
	if ( bufDst != nullptr ){
		int cntLock = (int)lenSrc;
		char* pSrc = bufSrc;
		char* pDst = bufDst;
		size_t nSrc = lenSrc;
		size_t nDst = lenDst;
		iconv_t icd;
		while( 0 < nSrc ){
			char *chEncSrc = (char *)encSrc.c_str();
			char *chEncDst = (char *)encDst.c_str();
			icd = iconv_open(chEncDst, chEncSrc);
			iconv(icd, &pSrc, &nSrc, &pDst, &nDst);
			iconv_close(icd);
			// 念のため異常による無限ループを対策
			cntLock --;
			if ( cntLock < 0 ) break;
		}
		*pDst = '\0';
		strResult = bufDst;
		delete [] bufDst;
	}
	delete [] bufSrc;
	return strResult;
}
#endif

//--- Shift-JIS -> UTF-8 ---
string LocalEtcCore::LocalWbCnv::getUtf8FromSjis(const string& str){
#if defined(LOCALETC_USE_WINOS_SJIS)	// Windows-OSによるShift-JIS処理
	wstring wstr = getWstrFromSjis(str);
	return getUtf8FromWstr(wstr);
#elif defined(LOCALETC_USE_ICONV_SJIS)	// iconvライブラリによる変換
	return getIconvStr(str, "UTF-8", "CP932");
#endif
	// return str;	// 必ずどちらかが定義される前提で返り値がない場合はShift-JIS不可
}
//--- UTF-8 -> Shift-JIS ---
string LocalEtcCore::LocalWbCnv::getSjisFromUtf8(const string& str){
#if defined(LOCALETC_USE_WINOS_SJIS)	// Windows-OSによるShift-JIS処理
	wstring wstr = getWstrFromUtf8(str);
	return getSjisFromWstr(wstr);
#elif defined(LOCALETC_USE_ICONV_SJIS)	// iconvライブラリによる変換
	return getIconvStr(str, "CP932", "UTF-8");
#endif
	// return str;	// 必ずどちらかが定義される前提で返り値がない場合はShift-JIS不可
}

//---------------------------------------------------------------------
// 文字コード設定
//---------------------------------------------------------------------
//--- 初期コード設定 ---
LocalEtcCore::LocalWbCnv::LocalWbCnv(){
	m_utfDefault = LcParam::UtfDefault;
	m_utfFileDefault = UtfType::standard;
}
//--- デフォルト設定を変更 ---
void LocalEtcCore::LocalWbCnv::setDefaultUtfStdCode(LcParam::UtfType utype){
	if ( utype != UtfType::standard ){		// 標準コードの設定なので標準は除く
		m_utfDefault = utype;
	}
}
//--- デフォルト設定は実際のコードに変換 ---
LcParam::UtfType LocalEtcCore::LocalWbCnv::getUtfTypeRevised(LcParam::UtfType utype){
	if ( utype == UtfType::standard ) return m_utfDefault;
	return utype;
}
//--- ファイルのデフォルト文字コード設定 ---
void LocalEtcCore::LocalWbCnv::setFileDefaultUtfCode(LcParam::UtfType utype){
	m_utfFileDefault = utype;
}
//--- ファイルのデフォルト文字コード取得 ---
LcParam::UtfType LocalEtcCore::LocalWbCnv::getFileDefaultUtfCode(){
	return m_utfFileDefault;
}

//---------------------------------------------------------------------
// OSパス - UTF-8 文字列変換処理
//---------------------------------------------------------------------
//--- パス区切り変更処理 ---
string LocalEtcCore::LocalWbCnv::replacePathDelimiter(const string& str){
	// パス区切りは7bitコード前提（UTF-8で変換したら重複しない）
	string s = str;
	string delimRm = LcParam::delimXPath;
	if ( delimRm != "" ){
		size_t posBak = string::npos;
		size_t pos;
		while( (pos = s.find(delimRm)) != string::npos ){
			s.replace(pos, 1, LcParam::delimPath);
			if ( pos <= posBak ){
				if ( posBak != string::npos ) break;	// 念のため何かのミスによる無限ループを防ぐ
			}
			posBak = pos;
		}
	}
	return s;
}
//--- OSアクセス用パス（必要ならマルチバイト化） ---
LcParam::PathString LocalEtcCore::LocalWbCnv::cnvToPathString(const string& str){
	string str_rev = replacePathDelimiter(str);
#if defined(LOCALETC_WIDE_PATH)
	return getWstrFromUtf8(str_rev);
#else
	return str_rev;
#endif
}
//--- OSアクセス用パスからUTF-8取得 ---
string LocalEtcCore::LocalWbCnv::cnvUtf8FromPathString(const LcParam::PathString& str){
#if defined(LOCALETC_WIDE_PATH)
	return getUtf8FromWstr(str);
#else
	return str;
#endif
}
string LocalEtcCore::LocalWbCnv::cnvUtf8FromFileSystemPath(filesystem::path& fpath){
#if defined(LOCALETC_WIDE_PATH)
	return cnvUtf8FromPathString(fpath.wstring());
#else
	return cnvUtf8FromPathString(fpath.string());
#endif
}

//---------------------------------------------------------------------
// ファイルIO - UTF-8 文字コード変換処理
//---------------------------------------------------------------------
//--- 指定形式の文字列をUTF-8に変換 ---
string LocalEtcCore::LocalWbCnv::cnvFromFileString(const string& lstr, LcParam::UtfType utype){
	LcParam::UtfType urev = getUtfTypeRevised(utype);

	switch( urev ){
		case UtfType::UTF8 :
		case UtfType::UTF8N :
			return lstr;
			break;
		case UtfType::SJIS :
			return getUtf8FromSjis(lstr);
			break;
		case UtfType::UTF16LE :
		case UtfType::UTF16BE :
			{
				int wlen = (int)((lstr.length()) / 2);
				int sw = ( utype == UtfType::UTF16LE )? 1 : 0;
				wstring wstr(wlen, 'a');	// 領域確保
				for(auto i=0; i<wlen; i++){
					unsigned char chh = lstr[i*2+sw];
					unsigned char chl = lstr[i*2+1-sw];
					unsigned long val = (((unsigned long) chh)<<8) + chl;
					wstr[i] = (wchar_t) val;
				}
				return getUtf8FromWstr(wstr);
			}
			break;
		default:
			break;
	}
	string ustr;
	return ustr;
}
//--- UTF-8の文字列を指定形式に変換 ---
string LocalEtcCore::LocalWbCnv::cnvToFileString(const string& ustr, LcParam::UtfType utype){
	LcParam::UtfType urev = getUtfTypeRevised(utype);

	switch( urev ){
		case UtfType::UTF8 :
		case UtfType::UTF8N :
			return ustr;
			break;
		case UtfType::SJIS :
			return getSjisFromUtf8(ustr);
			break;
		case UtfType::UTF16LE :
		case UtfType::UTF16BE :
			{
				bool force16 = true;	// Unicodeを16bitで格納
				wstring wstr = getWstrFromUtf8(ustr, force16);
				int wlen = (int)wstr.length();
				int sw = ( utype == UtfType::UTF16LE )? 1 : 0;
				string lstr(wlen*2, 'a');	// 領域確保
				for(auto i=0; i<wlen; i++){
					wchar_t wch = wstr[i];
					unsigned long val = (unsigned long) wch;
					unsigned char chh = (unsigned char)(val >> 8);
					unsigned char chl = (unsigned char)(val & 0xFF);
					lstr[i*2+sw] = chh;
					lstr[i*2+1-sw] = chl;
				}
				return lstr;
			}
			break;
		default:
			break;
	}
	string lstr;
	return lstr;
}

//---------------------------------------------------------------------
// wstring/u32 - UTF-8 文字列変換処理
//---------------------------------------------------------------------
//--- UTF-8のWideByte文字列長を取得（サロゲートペアが必要な時は2データで計算） ---
int LocalEtcCore::LocalWbCnv::getWlenFromUtf8(const string& str, bool force16){
	int nlen = (int) str.length();
	int outlen = 0;
	int i = 0;
	while( i < nlen && i >= 0 ){
		int lench = getWordByteFromUtf8(str, i);
		if ( lench <= 0 ){
			i = -1;			// 異常終了
		}else{
			if ( sizeof(wchar_t) == 2 || force16 ){		// wchar_tが2バイトの時は無条件でサロゲートペア使用
				if ( isSurrogatesPairFromUtf8(str, i) ){
					outlen ++;
				}
			}
			outlen ++;
			i += lench;
		}
	}
	return outlen;
}
//--- UTF-8の実際の文字列長を取得 ---
int LocalEtcCore::LocalWbCnv::getU32lenFromUtf8(const string& str){
	int nlen = (int) str.length();
	int outlen = 0;
	int i = 0;
	while( i < nlen && i >= 0 ){
		int lench = getWordByteFromUtf8(str, i);
		if ( lench <= 0 ){
			i = -1;			// 異常終了
		}else{
			outlen ++;
			i += lench;
		}
	}
	return outlen;
}
//--- WideByteをUTF-8に変換した時に必要な文字列長を取得 ---
int LocalEtcCore::LocalWbCnv::getLenToUtf8(const wstring& wstr){
	int len = 0;
	for(int i=0; i<(int)wstr.length(); i++){
		wchar_t ch = wstr[i];
		unsigned long val = (unsigned long) ch;
		if ( sizeof(wchar_t) == 2 ) val &= 0xFFFF;
		if ( val <= 0x007F ) len += 1;
		else if ( val <= 0x07FF ) len += 2;
		else if ( val <= 0xFFFF ) len += 3;
		else len += 4;
		if ( isSurrogatesPair(wstr, i) ){
			len += 1;	// 元の3から+1
			i ++;
		}
	}
	return len;
}
//--- u32をUTF-8に変換した時に必要な文字列長を取得 ---
int LocalEtcCore::LocalWbCnv::getLenToUtf8(const u32string& qstr){
	int len = 0;
	for(int i=0; i<(int)qstr.length(); i++){
		char32_t ch = qstr[i];
		unsigned long val = (unsigned long) ch;
		if ( val <= 0x007F ) len += 1;
		else if ( val <= 0x07FF ) len += 2;
		else if ( val <= 0xFFFF ) len += 3;
		else len += 4;
	}
	return len;
}
//--- UTF-8をWideByte文字列にして取得 ---
wstring LocalEtcCore::LocalWbCnv::getWstrFromUtf8(const string& str, bool force16){
	wstring wstr;
	int wlen = getWlenFromUtf8(str, force16);
	if ( wlen <= 0 ) return wstr;

	wchar_t* wbuf = new(std::nothrow) wchar_t[wlen+1];
	if ( wbuf == nullptr ) return wstr;
	int pos = 0;
	for(int i=0; i<wlen+1; i++){
		int sz;
		bool sgpair;
		wchar_t wchex;
		wbuf[i] = getWcharFromUtf8(sz, str, pos, sgpair, wchex, force16);
		if ( sgpair && i < wlen-1 ){		// サロゲートペア
			i ++;
			wbuf[i] = wchex;
		}
		pos += sz;
	}
	if ( pos == (int)str.length() ){		// 念のため確認
		wstr = wbuf;
	}
	delete [] wbuf;
	return wstr;
}
//--- UTF-8をu32string文字列にして取得 ---
u32string LocalEtcCore::LocalWbCnv::getU32strFromUtf8(const string& str){
	u32string qstr;
	int ulen = getU32lenFromUtf8(str);
	if ( ulen <= 0 ) return qstr;

	char32_t* ubuf = new(std::nothrow) char32_t[ulen+1];
	if ( ubuf == nullptr ) return qstr;
	int pos = 0;
	for(int i=0; i<ulen+1; i++){
		int sz;
		ubuf[i] = getU32charFromUtf8(sz, str, pos);
		pos += sz;
	}
	if ( pos == (int)str.length() ){		// 念のため確認
		qstr = ubuf;
	}
	delete [] ubuf;
	return qstr;
}
//--- WideByte文字列をUTF-8文字列にして取得 ---
string LocalEtcCore::LocalWbCnv::getUtf8FromWstr(const wstring& wstr){
	string str;
	int wlen = (int)wstr.length();
	int len = getLenToUtf8(wstr);
	if ( len <= 0 ) return str;

	char* buf = new(std::nothrow) char[len+1];
	if ( buf == nullptr ) return str;
	int pos = 0;
	for(int i=0; i<wlen+1; i++){
		bool sgpair;
		int sz = getWordUtf8FromWstr(&buf[pos], wstr, i, sgpair);
		if ( sgpair ){		// サロゲートペア
			i ++;
		}
		pos += sz;
	}
	if ( pos == len ){		// 念のため確認
		str = buf;
	}
	delete [] buf;
	return str;
}
//--- u32文字列をUTF-8文字列にして取得 ---
string LocalEtcCore::LocalWbCnv::getUtf8FromU32str(const u32string& qstr){
	string str;
	int ulen = (int)qstr.length();
	int len = getLenToUtf8(qstr);
	if ( len <= 0 ) return str;

	char* buf = new(std::nothrow) char[len+1];
	if ( buf == nullptr ) return str;
	int pos = 0;
	for(int i=0; i<ulen+1; i++){
		int sz = getWordUtf8FromU32str(&buf[pos], qstr, i);
		pos += sz;
	}
	if ( pos == len ){		// 念のため確認
		str = buf;
	}
	delete [] buf;
	return str;
}
//--- UTF-8の先頭文字から必要なバイト数を取得（-1の時は先頭文字ではない） ---
int LocalEtcCore::LocalWbCnv::getNeedByteFromUtf8Head(const char ch){
	if ( (ch & 0x80) == 0 ) return 1;
	if ( (ch & 0x40) == 0 ) return -1;
	if ( (ch & 0x20) == 0 ) return 2;
	if ( (ch & 0x10) == 0 ) return 3;
	return 4;
}
//--- UTF-8の対象位置から1文字のbyte数を取得 ---
int LocalEtcCore::LocalWbCnv::getWordByteFromUtf8(const string& str, int n){
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
//--- UTF-8の対象位置から1文字をWideByteで取得 ---
//  サロゲートペアが必要な時は、sgpair=true を設定して下位データを wchex に格納する
wchar_t LocalEtcCore::LocalWbCnv::getWcharFromUtf8(int& nSize, const string& str, int n, bool& sgpair, wchar_t& wchex, bool force16){
	sgpair = false;		// 通常は出力の拡張不要
	char32_t qch = getU32charFromUtf8(nSize, str, n);
	return getWcharFromU32char(qch, sgpair, wchex, force16);
}
//--- char32_tの対象位置から1文字をWideByteで取得 ---
//  サロゲートペアが必要な時は、sgpair=true を設定して下位データを wchex に格納する
wchar_t LocalEtcCore::LocalWbCnv::getWcharFromU32str(const u32string& qstr, int n, bool& sgpair, wchar_t& wchex, bool force16){
	sgpair = false;		// 通常は出力の拡張不要
	// データ取得
	int qlen = (int)qstr.length();
	if ( n >= qlen || n < 0 ){
		return (wchar_t) 0;
	}
	char32_t qch = qstr[n];
	return getWcharFromU32char(qch, sgpair, wchex, force16);
}
//--- char32_tの1文字をWideByteで取得 ---
wchar_t LocalEtcCore::LocalWbCnv::getWcharFromU32char(const char32_t qch, bool& sgpair, wchar_t& wchex, bool force16){
	sgpair = false;		// 通常は出力の拡張不要
	unsigned long val = (unsigned long) qch;
	if ( val > 0xFFFF ){
		if ( sizeof(wchar_t) == 2 || force16 ){		// wchar_tが2バイトの時は無条件でサロゲートペア使用
			unsigned long val1;
			val1 = (((val - 0x10000) >> 10) & 0x3FF) + 0xD800;
			// 下位10bitからの値をval2に設定
			unsigned long val2;
			val2 = (val & 0x0003FF) + 0xDC00;
			// サロゲートペア設定
			val = val1;
			sgpair = true;
			wchex = (wchar_t) val2;
		}
	}
	return (wchar_t) val;
}
//--- UTF-8の対象位置から1文字をchar32_tで取得 ---
char32_t LocalEtcCore::LocalWbCnv::getU32charFromUtf8(int& nSize, const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0 ){
		nSize = 0;
		return (char32_t) 0;
	}
	unsigned char code = (unsigned char) str[n];
	if ( (code & 0x80) == 0 ){
		nSize = 1;
		return (char32_t) code;
	}
	nSize = len - n;
	unsigned long val = 0;
	if ( (code & 0xE0) == 0xC0 ){
		if ( nSize >= 2 ){
			nSize = 2;
			unsigned char code2 = (unsigned char) str[n+1];
			val = (((unsigned long)(code  & 0x1F)) << 6) +
			       ((unsigned long)(code2 & 0x3F));
		}else{
			val = 0x3F;
		}
	}else if ( (code & 0xF0) == 0xE0 ){
		if ( nSize >= 3 ){
			nSize = 3;
			unsigned char code2 = (unsigned char) str[n+1];
			unsigned char code3 = (unsigned char) str[n+2];
			val = (((unsigned long)(code  & 0x0F)) << 12) +
			      (((unsigned long)(code2 & 0x3F)) << 6 ) +
			       ((unsigned long)(code3 & 0x3F));
		}else{
			val = 0x3F;
		}
	}else if ( (code & 0xF1) == 0xF1 ){		// 存在しないコード
			val = 0x3F;
	}else{
		if ( nSize >= 4 ){
			nSize = 4;
			unsigned char code2 = (unsigned char) str[n+1];
			unsigned char code3 = (unsigned char) str[n+2];
			unsigned char code4 = (unsigned char) str[n+3];
			val = (((unsigned long)(code  & 0x07)) << 18) +
			      (((unsigned long)(code2 & 0x3F)) << 12) +
			      (((unsigned long)(code3 & 0x3F)) << 6 ) +
			       ((unsigned long)(code4 & 0x3F));
			if ( val > 0x10FFFF ){	// 上限超え
					val = 0x3F;
			}
		}
		else{
			val = 0x3F;
		}
	}
	return (char32_t) val;
}
//--- WideStringの対象位置から1文字をchar32_tで取得 ---
char32_t LocalEtcCore::LocalWbCnv::getU32charFromWstr(int& nSize, const wstring& wstr, int n){
	int wlen = (int) wstr.length();
	if ( n >= wlen || n < 0 ){
		nSize = 0;
		return (char32_t) 0;
	}
	nSize = 1;
	wchar_t wch =wstr[n];
	char32_t qch = (char32_t) ((uint32_t) wch);
	// サロゲートペア設定
	if ( isSurrogatesHigher(wch) ){
		unsigned long val = 0x3F;	// 取得できなかった時のコード
		if ( n+1 < wlen ){
			wchar_t wch2 =wstr[n+1];
			if ( isSurrogatesLower(wch2) ){
				nSize = 2;
				val = ((((unsigned long) wch) - 0xD800) << 10) +
				      (((unsigned long) wch2) - 0xDC00) +
				      0x10000;
			}
		}
		qch = (char32_t) val;
	}
	return qch;
}
//--- WideByteの対象位置1文字をUTF-8文字列で取得 ---
// サロゲートペアの時は、sgpair=true が設定される
int LocalEtcCore::LocalWbCnv::getWordUtf8FromWstr(char* str, const wstring& wstr, int n, bool& sgpair){
	int nWide;
	char32_t qch = getU32charFromWstr(nWide, wstr, n);
	sgpair = false;		// 通常は拡張不要
	if ( nWide == 0 ){
		str[0] = '\0';
		return 0;
	}
	u32string qstr{qch};
	int nSize = getWordUtf8FromU32str(str, qstr, 0);
	if ( nWide == 2 ){
		sgpair = true;
	}
	return nSize;
}
//--- char32_tの対象位置1文字をUTF-8文字列で取得 ---
int LocalEtcCore::LocalWbCnv::getWordUtf8FromU32str(char* str, const u32string& qstr, int n){
	int qlen = (int) qstr.length();
	if ( n >= qlen || n < 0 ){
		str[0] = '\0';
		return 0;
	}
	char32_t qch =qstr[n];
	unsigned long val = (unsigned long) qch;
	int nSize = 0;
	if ( val <= 0x007F ){
		nSize = 1;
		str[0] = (char) ( val & 0x7F );
	}
	else if ( val <= 0x07FF ){
		nSize = 2;
		str[0] = (char) ( 0xC0 + (0x1F & (val >> 6)) );
		str[1] = (char) ( 0x80 + (0x3F & val) );
	}
	else if ( val <= 0xFFFF ){
		nSize = 3;
		str[0] = (char) ( 0xE0 + (0x0F & (val >> 12)) );
		str[1] = (char) ( 0x80 + (0x3F & (val >> 6)) );
		str[2] = (char) ( 0x80 + (0x3F & val) );
	}
	else{
		nSize = 4;
		str[0] = (char) ( 0xF0 + (0x07 & (val >> 18)) );
		str[1] = (char) ( 0x80 + (0x3F & (val >> 12)) );
		str[2] = (char) ( 0x80 + (0x3F & (val >> 6)) );
		str[3] = (char) ( 0x80 + (0x3F & val) );
	}
	return nSize;
}

//---------------------------------------------------------------------
// サロゲートペア判定
//---------------------------------------------------------------------
//--- サロゲートペア確認（UTF-8から） ---
bool LocalEtcCore::LocalWbCnv::isSurrogatesPairFromUtf8(const string str, int n){
	int nSize;
	char32_t qch = getU32charFromUtf8(nSize, str, n);
	if ( ((unsigned long) qch) > 0xFFFF ){
		return true;
	}
	return false;
}
//--- サロゲートペア確認（wstringから） ---
bool LocalEtcCore::LocalWbCnv::isSurrogatesPair(const wstring wstr, int n){
	int wlen = (int)wstr.length();
	if ( n+1 < wlen && n >= 0){
		if ( isSurrogatesHigher(wstr[n]) ){
			if ( isSurrogatesLower(wstr[n+1]) ){
				return true;
			}
		}
	}
	return false;
}
//--- 上位サロゲート確認 ---
bool LocalEtcCore::LocalWbCnv::isSurrogatesHigher(const wchar_t wch){
	unsigned long val = (unsigned long) wch;
	if ( val <= 0xFFFF ){
		if ( 0xD800 <= val && val < 0xDBFF ){
			return true;
		}
	}
	return false;
}
//--- 下位サロゲート確認 ---
bool LocalEtcCore::LocalWbCnv::isSurrogatesLower(const wchar_t wch){
	unsigned long val = (unsigned long) wch;
	if ( val <= 0xFFFF ){
		if ( 0xDC00 <= val && val < 0xDFFF ){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// 文字コード自動判定用
//---------------------------------------------------------------------
//--- Shift-JISとして正しい文字列か（半角カナ系統は含めない） ---
bool LocalEtcCore::LocalWbCnv::isCodeOkAsSjisCommon(const string& str, bool flagLimit){
	int lenSrc = (int)str.length();
	int i = 0;
	while( i < lenSrc ){
		int lenNeed = 1;
		unsigned char code = (unsigned char)str[i];
		if ((code >= 0x81 && code <= 0x9F) ||
			(code >= 0xE0 && code <= 0xFC)){		// Shift-JIS 1st-byte
			code = (unsigned char)str[i+1];
			if ((code >= 0x40 && code <= 0x7E) ||
				(code >= 0x80 && code <= 0xFC)){	// Shift-JIS 2nd-byte
				lenNeed = 2;
			}else{
				return false;
			}
		}
		else if ( flagLimit ){
			if ( code >= 0x80 ){	// 半角カナ系統はない前提とする場合
				return false;
			}
		}
		i += lenNeed;
	}
	return true;
}
//--- Shift-JISとして正しい文字列か（半角カナ系統は含めない） ---
bool LocalEtcCore::LocalWbCnv::isCodeOkAsSjisLimit(const string& str){
	bool flagLimit = true;
	return isCodeOkAsSjisCommon(str, flagLimit);
}
//--- Shift-JISとして正しい文字列か ---
bool LocalEtcCore::LocalWbCnv::isCodeOkAsSjis(const string& str){
	bool flagLimit = false;
	return isCodeOkAsSjisCommon(str, flagLimit);
}
//--- ASCIIコードのみの文字列か ---
bool LocalEtcCore::LocalWbCnv::isCodeOkAsAscii(const string& str){
	int lenSrc = (int)str.length();
	for(int i=0; i<lenSrc; i++){
		char ch = str[i];
		if ( (ch & 0x80) != 0 ) return false;
	}
	return true;
}
//--- UTF-8として正しい文字列か ---
bool LocalEtcCore::LocalWbCnv::isCodeOkAsUtf8(const string& str){
	int lenSrc = (int)str.length();
	int i = 0;
	while( i < lenSrc ){
		int lenNeed = getNeedByteFromUtf8Head(str[i]);
		if ( lenNeed <= 0 ) return false;
		for(int j=1; j<lenNeed; j++){
			char ch = str[i+j];
			if ( (ch & 0x80) == 0 ) return false;
			if ( (ch & 0x40) != 0 ) return false;
		}
		i += lenNeed;
	}
	return true;
}

//---------------------------------------------------------------------
// 文字コードを対応する形式で出力
//---------------------------------------------------------------------
LcParam::UtfType LocalEtcCore::LocalWbCnv::varUtfCodeFromNum(int num){
	LcParam::UtfType utfcode;
	switch( num ){
		case 1:
			utfcode = UtfType::standard;
			break;
		case 2:
			utfcode = UtfType::UTF8;
			break;
		case 3:
			utfcode = UtfType::UTF16LE;
			break;
		case 4:
			utfcode = UtfType::UTF16BE;
			break;
		case 11:
			utfcode = UtfType::SJIS;
			break;
		case 12:
			utfcode = UtfType::UTF8N;
			break;
		default:
			utfcode = UtfType::none;
			break;
	}
	return utfcode;
}
int LocalEtcCore::LocalWbCnv::varUtfNumFromCode(LcParam::UtfType utfcode){
	int num;
	switch( utfcode ){
		case UtfType::standard :
			num = 1;
			break;
		case UtfType::UTF8 :
			num = 2;
			break;
		case UtfType::UTF16LE :
			num = 3;
			break;
		case UtfType::UTF16BE :
			num = 4;
			break;
		case UtfType::SJIS :
			num = 11;
			break;
		case UtfType::UTF8N :
			num = 12;
			break;
		default:
			num = -1;
			break;
	}
	return num;
}
string LocalEtcCore::LocalWbCnv::varUtfStrFromNum(int num){
	string strUtf;
	switch( num ){
		case 1 :
			strUtf = "STD";
			break;
		case 2 :
			strUtf = "UTF8";
			break;
		case 3 :
			strUtf = "UTF16";
			break;
		case 4 :
			strUtf = "UTF16BE";
			break;
		case 11 :
			strUtf = "SJIS";
			break;
		case 12 :
			strUtf = "UTF8N";
			break;
		default :
			strUtf = "illegal";
			break;
	}
	return strUtf;
}
int LocalEtcCore::LocalWbCnv::varUtfNumFromStr(const string& strUtf){
	int num;
	if ( strUtf == "1" || strUtf == "STD" ){
		num = 1;
	}
	else if ( strUtf == "2" || strUtf == "UTF8"  || strUtf == "UTF-8"){
		num = 2;
	}
	else if ( strUtf == "3" || strUtf == "UTF16" || strUtf == "UTF-16" || strUtf == "UTF16LE" ){
		num = 3;
	}
	else if ( strUtf == "4" || strUtf == "UTF16BE" ){
		num = 4;
	}
	else if ( strUtf == "11" || strUtf == "SJIS" || strUtf == "S-JIS" ){
		num = 11;
	}
	else if ( strUtf == "12" || strUtf == "UTF8N" || strUtf == "UTF-8N" ){
		num = 12;
	}
	else{
		num = -1;
	}
	return num;
}
bool LocalEtcCore::LocalWbCnv::isUtfNumValid(int num){
	if (  1 <= num && num <=  4 ) return true;
	if ( 11 <= num && num <= 12 ) return true;
	return false;
}

//---------------------------------------------------------------------
// 一般関数処理
//---------------------------------------------------------------------
//--- 一般関数の形でifstream処理 ---
bool LocalEtc::getline(LocalIfs& ifs, string& str){
	return ifs.getline(str);
};

//--- stringで大文字小文字無視の文字列一致確認 ---
bool LocalEtc::isStrCaseSame(const string& str1, const string& str2){
	int n = (int)str1.length();
	if ( n != (int)str2.length() ) return false;
	for(int i=0; i<n; i++){
		char ch1 = ( str1[i] >= 'a' && str1[i] <= 'z' )? str1[i]-'a'+'A' : str1[i];
		char ch2 = ( str2[i] >= 'a' && str2[i] <= 'z' )? str2[i]-'a'+'A' : str2[i];
		if ( ch1 != ch2 ) return false;
	}
	return true;
}
