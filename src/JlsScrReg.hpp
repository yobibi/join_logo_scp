//
// 変数の格納
//
// クラス構成
//   JlsScrReg       : ローカル変数（階層別）とグローバル変数それぞれJlsRegFileを保持
//     |- JlsRegFile : 変数格納
//
#pragma once

#include "JlsScrFuncList.hpp"

///////////////////////////////////////////////////////////////////////
//
// 変数格納クラス
//
///////////////////////////////////////////////////////////////////////
class JlsRegFile
{
public:
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	bool unsetRegVar(const string& strName);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	void setIgnoreCase(bool valid);
	void setFlagAsRef(const string& strName);
	bool isRegNameRef(const string& strName);
	bool popMsgError(string& msg);

private:
	bool isSameInLen(const string& s1, const string& s2, int nLen);
	int  getRegNameVal(string& strName, string& strVal, const string& strPair);

private:
	vector<string>   m_strListVar;	// 変数格納
	string           msgErr;		// エラーメッセージ格納
	bool             m_ignoreCase;	// 大文字小文字区別
	unordered_map<string, bool>  m_flagListRef;		// 参照渡し変数
};

///////////////////////////////////////////////////////////////////////
//
// 階層構造変数クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScrReg
{
private:
	enum class RegOwner {		// 階層作成元
		Any,					// （削除時）全部許可
		Call,					// Call
		Func,					// Function
		One,					// 通常コマンド
	};
	struct RegLayer {
		RegOwner owner;			// 階層作成主
		bool base;				// 上位階層を検索しない階層
		JlsRegFile regfile;
	};
	class RegDivList {			// 変数名を名前とリスト要素に分解
		public:
			string nameBase;		// 変数名
			vector<int>  listElem;	// 要素番号
			int    nMatch;			// 変数長
		public:
			RegDivList(){};
			RegDivList(const string& str){ set(str); };
			void set(const string& str){	// 設定
				listElem.clear();
				ref(str);
			};
			void ref(const string& str){	// 参照渡し更新
				bool flagDim = false;
				auto ns = str.find("[");
				if ( ns != string::npos ){
					nameBase = str.substr(0, ns);
					vector<int> listTmp;
					bool cont = true;
					while( cont ){
						cont = false;
						auto ne = str.find("]", ns);
						if ( ne != string::npos && ns+1 < ne ){
							flagDim = true;
							listTmp.push_back( atoi(str.substr(ns+1, ne-ns-1).c_str()) );
							nMatch = (int)ne + 1;
							if ( str[ne+1] == '[' ){	// 多次元配列
								ns = ne+1;
								cont = true;
							}
						}
					}
					for(int i=(int)listTmp.size()-1; i>=0; i--){
						listElem.push_back( listTmp[i] );	// 一番深い要素を先頭に
					}
				}
				if ( !flagDim ){	// リスト要素ではない通常の更新
					nameBase = str;
					nMatch = (int)nameBase.length();
				}
			};
	};
	class RegSearch {
		public:
			string strName;			// 変数名
			string strVal;			// 変数値
			bool   exact;			// true=変数名は全文字一致  false=変数名は先頭から部分一致
			int    numLayer;		// 階層（-1=検出なし  0=グローバル変数  1-=ローカル変数階層）
			bool   stopRef;			// 参照渡しは対象外
			bool   onlyOneLayer;	// 検索は1階層のみ
			bool   flagRef;			// 参照渡し変数=true
			int    numMatch;		// マッチした変数名の長さ
			RegDivList regOrg;		// 要素分割した変数名（元の変数）
			RegDivList regSel;		// 要素分割した変数名（選択後変数）
		public:
			RegSearch(){};
			RegSearch(const string& str){ set(str); };
			void set(const string& str){		// 初期設定
				regOrg.set(str);
				strName = regOrg.nameBase;
				exact = true;
				numLayer = -1;
				stopRef = false;
				onlyOneLayer = false;
				flagRef  = false;
				numMatch = 0;
			}
			void decide(){		// 検索結果を格納
				// 変数名を部分一致で取得なら[]は含めず一致部分のみ
				if ( 0 < numMatch && numMatch < (int)regOrg.nameBase.length() ){
					regOrg.set( regOrg.nameBase.substr(0, numMatch) );
				}
				regSel = regOrg;
			}
			bool updateRef(const string& str){	// 参照先を新しい変数名にする
				regSel.ref(str);
				strName = regSel.nameBase;
				numLayer -= 1;
				exact = true;		// 参照は正確な変数名
				return ( numLayer>=0 );
			}
	};

public:
	JlsScrReg();
	// 階層制御
	int  createLocalCall();
	int  createLocalFunc();
	int  createLocalOne();
	int  releaseLocalAny();
	int  releaseLocalCall();
	int  releaseLocalFunc();
	int  releaseLocalOne();
	int  getLocalLayer();
private:
	int  createLocalCommon(RegOwner owner);
	int  releaseLocalCommon(RegOwner owner);
public:
	// 変数アクセス
	bool unsetRegVar(const string& strName, bool flagLocal);
	bool setLocalRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	// 引数設定
	bool setArgReg(const string& strName, const string& strVal);
	bool setArgRefReg(const string& strName, const string& strVal);
	void setArgFuncName(const string& strName);
	// その他制御
	void setLocalOnly(bool flag);
	void setIgnoreCase(bool valid);
	void setGlobalLock(const string& strName, bool flag);
	bool isGlobalLocked(const string& strName);
	bool checkErrRegName(const string& strName, bool silent = false);
	bool popMsgError(string& msg);

private:
	// 変数を階層指定で書き込み
	bool unsetRegCore(const string& strName, int numLayer);
	bool setRegCore(const string& strName, const string& strVal, bool overwrite, int numLayer);
	bool setRegCoreAsRef(const string& strName, const string& strVal, int numLayer);
	// 変数を検索して読み出し
	bool findRegForUnset(int& numLayer, const string& strName, bool flagLocal);
	bool findRegForWrite(string& strName, string& strVal, bool& overwrite, int& numLayer);
	int  findRegForRead(const string& strName, string& strVal, bool exact);
	bool findRegListForWrite(RegDivList& regName, string& strVal, bool& overwrite, const string& strRead);
	bool findRegListForRead(RegDivList& regName, string& strVal);
	bool findRegData(RegSearch& data);
	bool findRegDataFromLayer(RegSearch& data);
	bool isRegNameRef(const string& strName, int numLayer);
	// 引数の設定
	void clearArgReg();
	void setRegFromArg();
	void setRegFromArgSub(vector<string>& listArg, bool ref);
	// エラー処理
	bool popErrLower(JlsRegFile& regfile);
	// 内部処理
	string makeDummyReg(const string& str){ return "[]"+str; };
	string backDummyReg(const string& str){ return str.substr(2); };

private:
	vector<RegLayer> layerReg;		// 階層別ローカル変数
	JlsRegFile       globalReg;		// グローバル変数
	vector<string>   listValArg;	// Call用引数格納
	vector<string>   listRefArg;	// 参照渡し引数格納
	string           nameFuncReg;	// レジスタ名として使用される次の関数名
	bool             onlyLocal;		// グローバル変数を読み出さない設定=true
	bool             ignoreCase;	// 大文字小文字の無視
	string           msgErr;		// エラーメッセージ格納
	unordered_map <string, bool> m_mapGlobalLock;

	JlsScrFuncList   funcList;		// リスト処理
};

