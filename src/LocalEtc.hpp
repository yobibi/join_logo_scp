//
// 環境依存で面倒そうな処理をまとめたもの
//
// Unicode関連の処理は、入出力の違いを全部ここで吸収し、内部は全部UTF-8で動作させる
// （型はstring,charのまま。Shift-JISみたいな制御コード重なりもなく簡易化できる）
// Shift-JISの処理は全部OSにまかせる（ワイドバイトUTF-16間をWindows機能で相互変換）
// Windowsのみ動作確認（Linux動作も考慮して作成）
//

#pragma once


#if defined(_MSC_VER)
	// _sが付加された安全にアクセスする関数を使う場合は定義
	#define USE_SAFETY_CALL
#endif


#if defined(_WIN32)
  // Shift-JISを使用する場合は定義（OS処理のためWindowsのみ定義）
  #define WIDE_BYTE_SJIS		// Shift-JISを使う場合はワイドバイトを使ってOSで変換
#endif


namespace LcParam
{
	enum class UtfType {		// 文字コード種類
		none,
		UTF8,
		SJIS,
		UTF16LE,
		UTF16BE,
	};

#if defined(_WIN32)
	static const UtfType UtfDefault = UtfType::SJIS;	// WindowsはShift-JISが標準
#else
	static const UtfType UtfDefault = UtfType::UTF8;	// Windows以外はUTF-8が標準
#endif
}

namespace LocalInc		// 一般的な共通コマンドを記載
{
	//--- 書式整形（snprintfの領域確保を代行してstringで出力） ---
	template<typename ... Args>
	std::string sformat(const std::string& fmt, Args ... args){
		size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ... );
		std::vector<char> buf(len+1);
		std::snprintf(&buf[0], len+1, fmt.c_str(), args ... );
		return std::string(&buf[0], &buf[0]+len);
	}
}

class LocalOutStream;
class LocalErrStream;
class LocalSys;
class LocalWbCnv;
class LocalOfs;

//---------------------------------------------------------------------
// 共通利用の標準出力／エラー変数／システム制御
// cout/cerr 記載箇所を lcout/lcerr に変更することでUnicode対応
//---------------------------------------------------------------------
extern LocalOutStream lcout;	// cout代わり
extern LocalErrStream lcerr;	// cerr代わり
extern LocalSys LSys;			// システム制御

extern LocalWbCnv LWbCnv;	// Local各クラス内で使用の文字コード変換処理
//---------------------------------------------------------------------
// システム制御
//---------------------------------------------------------------------
class LocalSys {
	LcParam::UtfType m_utfStd = LcParam::UtfDefault;
	LcParam::UtfType m_utfFile = LcParam::UtfDefault;
	int  m_nMemoHold = 0;
	vector<string> m_listMemo;
	LocalWbCnv& wbc = LWbCnv;
public:
	LocalSys(){ m_listMemo.clear(); };
	//--- 文字列出力 ---
	void bufcout(const string& buf);
	void bufcerr(const string& buf);
	void bufMemoIns(const string& buf);
private:
	void bufMemoInsSel(const string& buf, bool chkStd, bool chkErr);
public:
	void bufMemoFlush(LocalOfs& ofs);
	//--- OSコマンド ---
	bool cmdMkdir(const string& strName);
	bool cmdCopy(const string& strFrom, const string& strTo);
	string getCurrentPath();
	bool getEnvString(string& strVal, const string& strEnvName);
	vector<string> getMainArg(int argc, char *argv[]);

	// 内部メモ情報設定
	void setMemoSel(int n);
	// 文字コードの番号-コード間を変換
	LcParam::UtfType getUtfCodeFromNum(int num);
	int  getUtfNumFromCode(LcParam::UtfType utfcode);
	int  getUtfNumFromStr(const string& strUtf);
	// 標準出力／標準エラーの文字コード設定
	void setStdUtfCodeFromNum(int num);
	void setStdUtfCode(LcParam::UtfType code);
	int  getStdUtfNum();
	LcParam::UtfType getStdUtfCode();
	// ファイル出力のデフォルト文字コード設定
	void setFileUtfCodeFromNum(int num);
	void setFileUtfCode(LcParam::UtfType code);
	int  getFileUtfNum();
	LcParam::UtfType getFileUtfCode();

	// デバッグ用
	void echoCodeWB(wstring str, int len = -1);
	void echoCodeByte(string str, int len = -1);
};
//---------------------------------------------------------------------
// 標準ストリーム(lcout/lcerr用)
//---------------------------------------------------------------------
//--- 内部処理用 ---
class LocalUtf8StreamBuf : public std::streambuf {	// UTF-8を実際の文字単位で送信する
	int m_size = 0;
	int m_pos = 0;
	char m_buf[4];
protected:
	LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
	virtual int_type overflow(int_type ich = EOF);
};
class LocalOutStreamBuf : public LocalUtf8StreamBuf {	// lcout用バッファ出力
protected:
	virtual std::streamsize xsputn(const char_type* s, std::streamsize count);
};
class LocalErrStreamBuf : public LocalUtf8StreamBuf {	// lcerr用バッファ出力
protected:
	virtual std::streamsize xsputn(const char_type* s, std::streamsize count);
};
//--- lcoutストリーム ---
class LocalOutStream : public std::ostream {		// lcout用クラス
	LocalOutStreamBuf *m_streambuf;
public:
	~LocalOutStream() { delete m_streambuf; }
	LocalOutStream() : std::ostream(m_streambuf = new LocalOutStreamBuf) {}
};
//--- lcerrストリーム ---
class LocalErrStream : public std::ostream {		// lcerr用クラス
	LocalErrStreamBuf *m_streambuf;
public:
	~LocalErrStream() { delete m_streambuf; }
	LocalErrStream() : std::ostream(m_streambuf = new LocalErrStreamBuf) {}
};
//---------------------------------------------------------------------
// ファイルタイプ情報
//---------------------------------------------------------------------
class LocalFileType {
	bool m_set = false;			// データが設定されたらtrue
	LcParam::UtfType m_utf;		// 文字コード種類
	int m_unit;					// 最低単位バイト数
	int m_bom;					// BOMのバイト数（0=BOM付加なし）
	string m_strbom;			// BOMの文字列（BOM付加なしの時も文字列は残す）
	LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
public:
	bool  setFromFile(const string& strName);
	void  setDirect(LcParam::UtfType utfcode);
	bool  isSet();

	LcParam::UtfType utf() { return m_utf; };	// 文字コード情報
	int              unit(){ return m_unit; };	// 最小単位バイト数
	int              bom() { return m_bom; };	// BOM文字数（0=BOM付加なし）
	string           strbom(){ return m_strbom; };	// BOM文字列
};
//---------------------------------------------------------------------
// ifstream処理（作成コマンドのみ対応）
//---------------------------------------------------------------------
class LocalIfs
{
	LocalWbCnv& wbc = LWbCnv;
	ifstream ifs;
	LocalFileType attr;
	bool flagOpen = false;
public:
	LocalIfs(){};
	LocalIfs(const string& strName){ open(strName); };
	bool open(const string& strName);
	bool getline(string& buf);
private:
	bool getlineCore(string& buf);
public:
	void close();
	bool is_open();
	int  getCodeNum();
};
//---------------------------------------------------------------------
// ofstream処理（作成コマンドのみ対応）
//---------------------------------------------------------------------
class LocalOfs
{
	LocalWbCnv& wbc = LWbCnv;
	ofstream ofs;
	LocalFileType attr;
	bool flagOpen = false;
public:
	LocalOfs(){};
	LocalOfs(const string& strName){ open(strName); };
	bool open(const string& strName, bool append = false);
	bool append(const string& strName);
	bool write(const string& buf);
private:
	string writeRevStr(const string& buf);
public:
	void close();
	bool is_open();
	void setCodeTypeFromNum(int num);
	void setCodeType(LcParam::UtfType utf);
};
//---------------------------------------------------------------------
// UTF-8の文字列操作（正規表現含む）
//---------------------------------------------------------------------
class LocalStr
{
public:
	int getStrLen(const string& str);
	string getSubStr(const string& str, int st);
	string getSubStrLen(const string& str, int st, int len);
private:
	string getSubStrCommon(const string& str, int st, int len, bool validLen);
public:
	int countInStr(const string& strSrc, const string& strEach);
	bool replaceInStr(string& strSrc, const string& strEach, const string& strSub);
	int countRegExMatch(const string& strSrc, const string& strRe);
	string getRegMAtch(const string& strSrc, const string& strRe);
private:
	LocalWbCnv& wbc = LWbCnv;
};


//---------------------------------------------------------------------
// UTF-8 - 他形式 文字列変換処理
//---------------------------------------------------------------------
class LocalWbCnv {
public:
	// ファイルIOと文字コード変換処理
	string cnvFromFileString(const string& lstr, LcParam::UtfType utype);
	string cnvToFileString(const string& ustr, LcParam::UtfType utype);
#if defined(WIDE_BYTE_SJIS)
private:
	wstring getWstrFromSjis(const string& str);
	string getSjisFromWstr(const wstring& wstr);
#endif
public:
	// WideByte - UTF-8 文字列変換処理
	int getWlenFromUtf8(const string& str);
	int getLenToUtf8(const wstring& wstr);
	wstring getWstrFromUtf8(const string& str);
	string getUtf8FromWstr(const wstring& wstr);
	int getNeedByteFromUtf8Head(const char ch);
	int getWordByteFromUtf8(const string& str, int n);
private:
	wchar_t getWordStrFromUtf8(int& nSize, const string& str, int n);
	int getWordUtf8FromWstr(char* str, const wstring& wstr, int n);
};
