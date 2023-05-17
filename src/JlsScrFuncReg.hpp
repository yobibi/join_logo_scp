//
// 変数アクセス関連処理
//

#pragma once

class JlsDataset;
class JlsCmdArg;
class JlsScrGlobal;
class JlsScrFuncList;

class JlsScrFuncReg
{
private:
	const string DefRegExpTrim  = R"(Trim\s*\(\s*(\d+)\s*,\s*(\d+)\s*\))";	// Trim取得正規表現
	const string DefRegExtChar = "EXTCHAR";		// 拡張子文字列処理用の変数名
	const string DefRegExtCsub = "EXTCHSUB";	// 拡張子文字列処理用の変数名
	const string DefRegDQuote  = "DQUOTE";		// ダブルクォート置換退避用の変数名
	const string DefRegSQuote  = "SQUOTE";		// シングルクォート置換退避用の変数名
	const string DefStrRepDQ   = "__!(DQ)!__";	// ダブルクォートを制御無関係文字に置換退避
	const string DefStrRepSQ   = "__!(SQ)!__";	// シングルクォートを制御無関係文字に置換退避

	enum class VarProcType {	// 変数の文字列処理
		none,
		path,		// パス部分取得
		divext,		// 拡張子部分取得
		substr,		// 部分文字列
		exchg,		// 文字列置換
		blank,		// 空白除去全体
		trim,		// 空白除去前後
		chpath,		// パス文字追加
		frame,		// フレーム数取得
		match,		// 正規表現検索
		count,		// 拡張子出現数
		len,		// 文字列長
	};
	struct VarProcRecord {
		VarProcType typeProc;
		bool selHead;
		bool selTail;
		bool selPath;
		bool withDelim;
		bool selRegEx;
		bool selInStr;
		bool selQuote;
		bool selBackup;
	};
	enum class ReadFileType {
		Check,
		Trim,
		List,
		String,
	};

public:
	void setDataPointer(JlsDataset *pdata, JlsScrGlobal *pglobal, JlsScrFuncList *plist);
public:
	// 起動オプション処理
	int  setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite);
private:
	Msec setOptionsCnvCutMrg(const char* str);
	bool setInputReg(const char *name, const char *val, bool overwrite);
	bool setInputFlags(const char *flags, bool overwrite);
public:
	// 関数引数アクセス
	void setArgFuncName(const string& strName);
	bool setArgRefReg(const string& strName, const string& strVal);
	bool setArgRegByVal(const string& strName, const string& strVal);
	bool setArgRegByName(const string& strName, const string& strValName);
	bool setArgRegByBoth(const string& strName, const string& strVal, bool quote);
private:
	bool isValidAsRegName(const string& strName);
	bool setArgRegCheckName(const string& strName);
public:
	// レジスタアクセス(write)
	bool unsetJlsRegVar(const string& strName, bool flagLocal);
	bool setJlsRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal);
	bool setJlsRegVarCountUp(const string& strName, int step, bool flagLocal);
private:
	void setJlsRegVarCouple(const string& strName, const string& strVal);
public:
	// レジスタアクセス(read)
	bool getJlsRegVarNormal(string& strVal, const string& strName);
	int  getJlsRegVarPartName(string& strVal, const string& strCandName, bool exact);
	// （private設定予定）
	int  getJlsRegVar(string& strVal, const string& strCandName, bool exact);
private:
	bool checkJlsRegDivide(string& strNamePart, string& strDivPart, int& lenFullVar);
	bool divideJlsRegVar(string& strVal, const string& strDivPart);
	bool divideJlsRegVarDecode(string& strVal, const string& strCmd);
	bool divideJlsRegVarDecodeIn(VarProcRecord& var, const string& strCmd);
	bool fixJlsRegNameAtList(string& strNamePart, int& lenFullVar, bool exact);
	void replaceStrAllFind(string& strVal, const string& strFrom, const string& strTo);
public:
	void backupStrQuote(string& strVal);
	void restoreStrQuote(string& strVal);
	// 変数名部分の変数値に置換
	bool replaceBufVar(string& dstBuf, const string& srcBuf);
private:
	int  replaceRegVarInBuf(string& strVal, const string& strBuf, int pos);
public:
	// システム変数設定
	void setSystemRegInit();
	void setSystemRegUpdate();
	void setSystemRegFilePath();
	void setSystemRegFileOpen();
	void setSystemRegHeadtail(int headframe, int tailframe);
	void setSystemRegNologo(bool need_check);
	void setSystemRegReadValid(bool valid);
	void setSystemRegLastexe(bool exe_command);
	bool isSystemRegLastexe();
	void setOutDirect();
	bool setSystemRegOptions(const string& strBuf, int pos, bool overwrite);
	void getSystemData(JlsCmdArg& cmdarg, const string& strIdent);
	// コマンド結果による変数更新
	void updateResultRegWrite(JlsCmdArg& cmdarg);
	void setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList);
	void setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListIns(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListDel(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListJoin(JlsCmdArg& cmdarg);
	void setResultRegListRemove(JlsCmdArg& cmdarg);
private:
	bool setResultRegSubGetRegVal(JlsCmdArg& cmdarg, string& strListSub, bool must);
public:
	// リスト操作コマンド
	void setResultRegListSel(JlsCmdArg& cmdarg, string strListNum);
	void setResultRegListRep(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListClear(JlsCmdArg& cmdarg);
	void setResultRegListDim(JlsCmdArg& cmdarg, int num);
	void setResultRegListSort(JlsCmdArg& cmdarg);
private:
	void sortResultRegList(JlsCmdArg& cmdarg, string& strList);
	void writeResultRegListW(JlsCmdArg& cmdarg, const string& strList);
public:
	// 文字列リスト化コマンド
	string getStrRegListByCsvStr(const string& strBuf);
	void setStrRegListByCsv(JlsCmdArg& cmdarg);
	void setStrRegListBySpc(JlsCmdArg& cmdarg);
private:
	void setStrRegListCommon(JlsCmdArg& cmdarg, const string& strBuf, int readtype);
public:
	// データ用ファイル読み込み
	bool readDataCheck(JlsCmdArg& cmdarg, const string& fname);
	bool readDataList(JlsCmdArg& cmdarg, const string& fname);
	bool readDataTrim(JlsCmdArg& cmdarg, const string& fname);
	bool readDataString(JlsCmdArg& cmdarg, const string& fname);
private:
	bool readDataCommon(JlsCmdArg& cmdarg, const string& fname, ReadFileType rtype);
	bool readDataCommonIns(JlsCmdArg& cmdarg, int& nCur, const string& strLine, int nMax, ReadFileType rtype);
	void readDataStrAdd(JlsCmdArg& cmdarg, const string& sdata);
	bool readDataStrTrimGet(string& strLocSt, string& strLocEd, string& strTarget);
	bool readDataStrTrimDetect(const string& strLine);
	bool readDataFileLine(string& strLine, LocalIfs& ifs);
	bool readDataFileTrim(string& strCmd, LocalIfs& ifs);
public:
	// グローバル領域作成のファイル読み込み
	bool readGlobalOpen(JlsCmdArg& cmdarg, const string& fname);
	void readGlobalClose(JlsCmdArg& cmdarg);
	bool readGlobalLine(JlsCmdArg& cmdarg);
private:
	bool getRegArg(JlsCmdArg& cmdarg, string& strArg);
	bool setRegOutSingle(JlsCmdArg& cmdarg, const string& strVal, bool certain);
public:
	// 環境設定読み込み
	bool readDataEnvGet(JlsCmdArg& cmdarg, const string& strEnvName);
private:
	void outputMesErr(const std::string& mes);

private:
	JlsDataset *pdata;
	JlsScrGlobal *pGlobalState;
	JlsScrFuncList *pFuncList;
};
