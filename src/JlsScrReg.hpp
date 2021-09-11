//
// 変数の格納
//
// クラス構成
//   JlsScrReg       : ローカル変数（階層別）とグローバル変数それぞれJlsRegFileを保持
//     |- JlsRegFile : 変数格納
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// 変数格納クラス
//
///////////////////////////////////////////////////////////////////////
class JlsRegFile
{
public:
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	bool popMsgError(string& msg);

private:
	int  getRegNameVal(string& strName, string& strVal, const string& strPair);

private:
	vector<string>   m_strListVar;	// 変数格納
	string           msgErr;		// エラーメッセージ格納

};

///////////////////////////////////////////////////////////////////////
//
// 階層構造変数クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScrReg
{
private:
	struct RegLayer {
		bool base;				// 上位階層を検索しない階層
		JlsRegFile regfile;
	};

public:
	int  createLocal(bool flagBase);
	int  releaseLocal(bool flagBase);
	bool setLocalRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	bool setArgReg(const string& strName, const string& strVal);
	void setLocalOnly(bool flag);
	bool popMsgError(string& msg);

private:
	int  getLayerRegVar(int& numLayer, string& strVal, const string& strCandName, bool exact);
	void clearArgReg();
	void setRegFromArg();
	bool checkErrRegName(const string& strName);
	bool popErrLower(JlsRegFile& regfile);

private:
	vector<RegLayer> layerReg;		// 階層別ローカル変数
	JlsRegFile       globalReg;		// グローバル変数
	vector<string>   listArg;		// Call用引数格納
	bool             onlyLocal = false;	// グローバル変数を読み出さない設定=true
	string           msgErr;		// エラーメッセージ格納
};

