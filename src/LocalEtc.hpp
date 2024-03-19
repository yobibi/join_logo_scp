//
// Copyright (c) 2024 Yobi
// Released under the MIT License
// http://opensource.org/licenses/mit-license.php
//
// 環境依存で面倒そうな処理をまとめたもの
//
// Unicode関連の処理は、入出力の違いを全部ここで吸収し、内部は全部UTF-8で動作させる
// （型はstring,charのまま。Shift-JISみたいな制御コード重なりもなく簡易化できる）
// Shift-JISの処理は全部OS(MS-windows)またはライブラリ(iconv)にまかせる
// C++17を使用（ファイルシステム関連）
// Windows / Linux 共通でアクセス可能
//
#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>


#if defined(_MSC_VER)
	// _sが付加された安全にアクセスする関数を使う場合は定義
	#define LOCALETC_USE_SAFETY_CALL
#endif

//#if defined(_MSC_VER)			// MinGWでiconvのShift-JIS変換する場合はこちら側を定義
#if defined(_WIN32)				// MinGWでWindowsのOS処理Shift-JIS変換する場合はこちら側を定義
  #define LOCALETC_USE_WINOS_SJIS		// OS処理を使ってShift-JISをワイドバイト経由で変換
#else
  #define LOCALETC_USE_ICONV_SJIS		// MinGWまたはLinux用のShift-JIS変換処理
#endif

#if defined(_WIN32)
    #define LOCALETC_WIDE_PATH		// ファイルパス（OS処理）でワイドバイト文字列を使う場合
#endif

//---------------------------------------------------------------------
// 内部処理
//---------------------------------------------------------------------
namespace LocalEtcCore
{
	namespace LcParam
	{
		enum class UtfType {		// 文字コード種類
			none,
			standard,
			UTF8,
			UTF8N,
			SJIS,
			UTF16LE,
			UTF16BE,
		};
	
	#if defined(_WIN32)
		static const UtfType UtfDefault = UtfType::SJIS;	// WindowsはShift-JISが標準
		static const char delimPath[]  = "\\";
		static const char delimXPath[] = "/";               // 変更するパス区切り。""の時は変換しない
	#else
		static const UtfType UtfDefault = UtfType::UTF8N;	// Windows以外はUTF-8が標準 BOMなし=出力結果に付けたくない
		static const char delimPath[]  = "/";
		static const char delimXPath[] = "\\";              // 変更するパス区切り。""の時は変換しない
	#endif

	#if defined(LOCALETC_WIDE_PATH)
		using PathString   = std::wstring;					// OSに渡すパスはwstring(WindowsはUTF-16)
	#else
		using PathString   = std::string;					// OSに渡すパスはstring(UTF-8)
	#endif
	}

	//---------------------------------------------------------------------
	// クラス前方宣言
	//---------------------------------------------------------------------
	class LocalOutStream;
	class LocalErrStream;
	class LocalSys;
	class LocalStr;
	class LocalWbCnv;
	class LocalOfs;
	//---------------------------------------------------------------------
	// 共通利用の標準出力／エラー変数／システム制御
	// cout/cerr 記載箇所を lcout/lcerr に変更することでUnicode対応
	//---------------------------------------------------------------------
	extern LocalOutStream  lcout;	// cout代わり
	extern LocalErrStream  lcerr;	// cerr代わり
	extern LocalSys        LSys; 	// システム制御
	extern LocalStr        LStr;	// UTF-8文字列処理
	extern LocalWbCnv      LWbCnv;	// 各クラス内で使用の文字コード変換処理
	//---------------------------------------------------------------------
	// システム制御
	//---------------------------------------------------------------------
	class LocalSys {
		LcParam::UtfType m_utfSys;	// システム文字コード保持
		int  m_nMemoHold;			// メモに標準出力等を出力する設定保持
		int  m_nMsgUtf; 			// 文字コード関連表示を標準出力に出すか保持
		std::string m_strMsgUtf;	// 文字コード関連表示で連続して同じメッセージを出さないため保持
		std::vector<std::string> m_listMemo;	// 内部メモデータ
		LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
	public:
		LocalSys();
		//--- 文字列出力 ---
		void bufcout(const std::string& buf);
		void bufcerr(const std::string& buf);
		void bufMemoIns(const std::string& buf);
	private:
		void bufMemoInsSel(const std::string& buf, bool chkStd, bool chkErr);
		bool isMemoHoldStd();
		bool isMemoHoldErr();
		bool isMsgUtfUndecided();
	public:
		void bufMemoFlush(LocalOfs& ofs);
		void bufOutCodeMsg(const std::string& msg);
		//--- OSコマンド ---
		bool cmdMkdir(const std::string& strName);
		bool cmdCopy(const std::string& strFrom, const std::string& strTo);
		std::string getCurrentPath();
		bool getEnvString(std::string& strVal, const std::string& strEnvName);
		std::vector<std::string> getMainArg(int argc, char *argv[]);
		std::string getPathDelimiter();

		//--- 内部メモ情報設定 ---
		void setMemoSel(int n);
		void setMsgUtf(int n);
		//--- 文字コードの番号-文字列間を変換 ---
		std::string getUtfStrFromNum(int num);
		int  getUtfNumFromStr(const std::string& strUtf);
		bool isUtfNumValid(int num);
		//--- 標準コードの設定を変更 ---
		void setUtfDefaultNum(int num);
		int  getUtfDefaultNum();
		//--- 標準出力／標準エラーの文字コード設定 ---
		void setSysUtfNum(int num);
		int  getSysUtfNum();
		//--- ファイル出力のデフォルト文字コード設定 ---
		void setFileUtfNum(int num);
		int  getFileUtfNum();

		//--- デバッグ用 ---
		void echoCodeWB(std::wstring str, int len = -1);
		void echoCodeByte(std::string str, int len = -1);
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
	class LocalOfsStreamBuf : public LocalUtf8StreamBuf {	// fileoutput用バッファ出力
		LocalOfs* m_ofs;
	protected:
		virtual std::streamsize xsputn(const char_type* s, std::streamsize count);
	public:
		void setStreamBufOfs(LocalOfs *ofs);
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
	//--- LocalOfsストリーム ---
	class LocalOfsStream : public std::ostream {		// fileoutput用クラス
		LocalOfsStreamBuf *m_streambuf;
	public:
		~LocalOfsStream() { delete m_streambuf; }
		LocalOfsStream() : std::ostream(m_streambuf = new LocalOfsStreamBuf) {}
		void setStreamOfs(LocalOfs *ofs){ m_streambuf->setStreamBufOfs(ofs); };
	};
	//---------------------------------------------------------------------
	// ファイルタイプ情報
	//---------------------------------------------------------------------
	class LocalFileType {
		bool m_set = false;			// データが設定されたらtrue
		bool m_unfix = false;		// 自動判断で文字コードが不確実な場合true
		LcParam::UtfType m_utf;		// 文字コード種類
		int  m_unit;				// 最低単位バイト数
		int  m_bom; 				// BOMのバイト数（0=BOM付加なし）
		std::string m_strbom;		// BOMの文字列（BOM付加なしの時も文字列は残す）
		LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
	private:
		bool  setFromFileBom(std::ifstream& ifs, const std::string& strName);
	public:
		bool  setFromFile(std::ifstream& ifs, const std::string& strName);
		void  setDirect(LcParam::UtfType utfcode);
		bool  isSet();

		LcParam::UtfType utf() { return m_utf; };	// 文字コード情報
		int              unit(){ return m_unit; };	// 最小単位バイト数
		int              bom() { return m_bom; };	// BOM文字数（0=BOM付加なし）
		std::string      strbom(){ return m_strbom; };	// BOM文字列
		bool             isUnfix(){ return m_unfix; };	// 自動判断で文字コード不確実
	};
	//---------------------------------------------------------------------
	// ifstream処理（作成コマンドに対応）
	//---------------------------------------------------------------------
	class LocalIfs
	{
		LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
		std::ifstream ifs;			// ifstream本体
		LocalFileType attr;			// ファイル情報保持
	protected:
		void iniSet(){};
		void iniOpen(const std::string& strName){ iniSet(); open(strName); };
		void iniOpenA(const std::string& strName, std::ios::openmode mode){ iniSet(); open(strName, mode); };
	public:
		LocalIfs(){ iniSet(); };
		LocalIfs(const std::string& strName){ iniOpen(strName); };
		LocalIfs(const std::string& strName, std::ios::openmode mode){ iniOpenA(strName, mode); };
		explicit operator bool() const { return (bool)ifs; };
		void open(const std::string& strName, std::ios::openmode mode);
		void open(const std::string& strName);
		bool getline(std::string& buf);
	private:
		bool getlineCore(std::string& buf);
	public:
		void close();
		bool is_open(){ return ifs.is_open(); };
		bool fail(){ return ifs.fail(); };
		bool eof(){ return ifs.eof(); };
		int  getUtfNum();
	};
	//---------------------------------------------------------------------
	// ofstream処理（作成コマンドに対応）
	//---------------------------------------------------------------------
	class LocalOfs : public LocalOfsStream
	{
		LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
		std::ofstream ofs;			// ofstream本体
		LocalFileType attr;			// ファイル情報保持
	protected:
		void iniSet(){ setStreamOfs(this); };
		void iniOpen(const std::string& strName){ iniSet(); open(strName); };
		void iniOpenA(const std::string& strName, std::ios::openmode mode){ iniSet(); open(strName, mode); };
	public:
		LocalOfs(){ iniSet(); };
		LocalOfs(const std::string& strName){ iniOpen(strName); };
		LocalOfs(const std::string& strName, std::ios::openmode mode){ iniOpenA(strName, mode); };
		explicit operator bool() const { return (bool)ofs; };
		void open(const std::string& strName, std::ios::openmode mode);
		void open(const std::string& strName);
		void append(const std::string& strName);
		bool write(const std::string& buf);
	private:
		std::string writeRevStr(const std::string& buf);
	public:
		void close();
		bool is_open(){ return ofs.is_open(); };
		bool fail(){ return ofs.fail(); };
		void setUtfNum(int num);
	};
	//---------------------------------------------------------------------
	// UTF-8の文字列操作（正規表現含む）
	//---------------------------------------------------------------------
	class LocalStr
	{
		LocalWbCnv& wbc = LWbCnv;	// 文字コード変換関数
		std::string getSubStrCommon(const std::string& str, int st, int len, bool validLen);
	public:
		int         getStrLen(const std::string& str);
		std::string getSubStr(const std::string& str, int st);
		std::string getSubStrLen(const std::string& str, int st, int len);
		int         countInStr(const std::string& strSrc, const std::string& strEach);
		bool        replaceInStr(std::string& strSrc, const std::string& strEach, const std::string& strSub);
		int         countRegExMatch(const std::string& strSrc, const std::string& strRe);
		std::string getRegMatch(const std::string& strSrc, const std::string& strRe);
	};
	//---------------------------------------------------------------------
	// UTF-8 - 他形式 文字列変換処理
	//---------------------------------------------------------------------
	class LocalWbCnv {
		LcParam::UtfType m_utfDefault;			// 標準コードに使用する文字コード
		LcParam::UtfType m_utfFileDefault;		// ファイル出力のデフォルト文字コード

		//--- Shift-JIS変換処理 ---
	#if defined(LOCALETC_USE_WINOS_SJIS)
		std::wstring getWstrFromSjis(const std::string& str);
		std::string getSjisFromWstr(const std::wstring& wstr);
	#endif
	#if defined(LOCALETC_USE_ICONV_SJIS)
		std::string getIconvStr(const std::string& strSrc, const std::string& encDst, const std::string& encSrc);
	#endif
		std::string getUtf8FromSjis(const std::string& str);
		std::string getSjisFromUtf8(const std::string& str);

	public:
		//--- 標準設定コード、ファイルのデフォルト文字コード処理 ---
		LocalWbCnv();
		void             setDefaultUtfStdCode(LcParam::UtfType utype);
		LcParam::UtfType getUtfTypeRevised(LcParam::UtfType utype);
		void             setFileDefaultUtfCode(LcParam::UtfType utype);
		LcParam::UtfType getFileDefaultUtfCode();

		//--- OSパス - UTF-8 文字列変換処理 ---
		std::string replacePathDelimiter(const std::string& str);
		LcParam::PathString cnvToPathString(const std::string& str);
		std::string cnvUtf8FromPathString(const LcParam::PathString& str);
		std::string cnvUtf8FromFileSystemPath(std::filesystem::path& fpath);

		//--- ファイルIO - UTF-8 文字コード変換処理 ---
		std::string cnvFromFileString(const std::string& lstr, LcParam::UtfType utype);
		std::string cnvToFileString(const std::string& ustr, LcParam::UtfType utype);

		//--- wstring/u32 - UTF-8 文字列変換処理 ---
		int  getWlenFromUtf8(const std::string& str, bool force16 = false);
		int  getU32lenFromUtf8(const std::string& str);
		int  getLenToUtf8(const std::wstring& wstr);
		int  getLenToUtf8(const std::u32string& ustr);
		std::wstring   getWstrFromUtf8(const std::string& str, bool force16 = false);
		std::u32string getU32strFromUtf8(const std::string& str);
		std::string    getUtf8FromWstr(const std::wstring& wstr);
		std::string    getUtf8FromU32str(const std::u32string& ustr);
		int  getNeedByteFromUtf8Head(const char ch);
		int  getWordByteFromUtf8(const std::string& str, int n);
	private:
		wchar_t  getWcharFromUtf8(int& nSize, const std::string& str, int n, bool& sgpair, wchar_t& wchex, bool force16);
		wchar_t  getWcharFromU32str(const std::u32string& qstr, int n, bool& sgpair, wchar_t& wchex, bool force16);
		wchar_t  getWcharFromU32char(const char32_t qch, bool& sgpair, wchar_t& wchex, bool force16);
		char32_t getU32charFromUtf8(int& nSize, const std::string& str, int n);
		char32_t getU32charFromWstr(int& nSize, const std::wstring& wstr, int n);
		int      getWordUtf8FromWstr(char* str, const std::wstring& wstr, int n, bool& sgpair);
		int      getWordUtf8FromU32str(char* str, const std::u32string& qstr, int n);
		bool isSurrogatesPairFromUtf8(const std::string str, int n);
		bool isSurrogatesPair(const std::wstring wstr, int n);
		bool isSurrogatesHigher(const wchar_t wch);
		bool isSurrogatesLower(const wchar_t wch);
		//--- 文字コード自動判定用 ---
		bool isCodeOkAsSjisCommon(const std::string& str, bool flagLimit);
	public:
		bool isCodeOkAsSjisLimit(const std::string& str);
		bool isCodeOkAsSjis(const std::string& str);
		bool isCodeOkAsAscii(const std::string& str);
		bool isCodeOkAsUtf8(const std::string& str);
		// 文字コードを対応する形式で出力
		LcParam::UtfType varUtfCodeFromNum(int num);
		int              varUtfNumFromCode(LcParam::UtfType utfcode);
		std::string      varUtfStrFromNum(int num);
		int              varUtfNumFromStr(const std::string& strUtf);
		bool             isUtfNumValid(int num);
	};
}

//---------------------------------------------------------------------
// 外部アクセス用
//---------------------------------------------------------------------
namespace LocalEtc
{
	//---------------------------------------------------------------------
	// 文字コード・システム関連
	//---------------------------------------------------------------------
	extern LocalEtcCore::LocalOutStream&  lcout;		// cout代わり
	extern LocalEtcCore::LocalErrStream&  lcerr;		// cerr代わり
	extern LocalEtcCore::LocalSys&        LSys; 		// システム制御
	extern LocalEtcCore::LocalStr&        LStr; 		// UTF-8文字列処理

	//--- ifstream代わり ---
	class LocalIfs : public LocalEtcCore::LocalIfs
	{
	public:
		LocalIfs(){ iniSet(); };
		LocalIfs(const std::string& strName){ iniOpen(strName); };
		LocalIfs(const std::string& strName, std::ios::openmode mode){ iniOpenA(strName, mode); };
	};
	//--- ofstream代わり ---
	class LocalOfs : public LocalEtcCore::LocalOfs
	{
	public:
		LocalOfs(){ iniSet(); };
		LocalOfs(const std::string& strName){ iniOpen(strName); };
		LocalOfs(const std::string& strName, std::ios::openmode mode){ iniOpenA(strName, mode); };
	};

	//---------------------------------------------------------------------
	// 一般関数
	//---------------------------------------------------------------------
	//--- 一般関数の形でifstream処理 ---
	bool getline(LocalIfs& ifs, std::string& str);

	//--- 書式整形（snprintfの領域確保を代行してstringで出力） ---
	template<typename ... Args>
	std::string sformat(const std::string& fmt, Args ... args){
		size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ... );
		std::vector<char> buf(len+1);
		std::snprintf(&buf[0], len+1, fmt.c_str(), args ... );
		return std::string(&buf[0], &buf[0]+len);
	}
	//--- stringで大文字小文字無視の文字列一致確認 ---
	bool isStrCaseSame(const std::string& str1, const std::string& str2);

}
