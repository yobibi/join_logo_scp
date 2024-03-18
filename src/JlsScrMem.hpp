//
// 遅延実行コマンドの保管
//
// クラス構成
//   JlsScrMem         : 遅延実行コマンド保管
//     |- JlsScrMemArg : 特殊文字列の解析・設定
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// 遅延実行保管用の識別子保持クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScrMemArg
{
private:
	enum class MemSpecialID {
		DUMMY,
		LAZY_FULL,
		LAZY_S,
		LAZY_A,
		LAZY_E,
		NoData,
		START,
		AUTO,
		END,
		MAXSIZE
	};
	static const int SIZE_MEM_SPECIAL_ID = static_cast<int>(MemSpecialID::MAXSIZE);

	struct StrIDRecord{
		char          str[8];	// 識別子文字列
		MemSpecialID  id;		// 実際に使用する文字列の識別子ID
	};
	//--- MemSpecialIDに対応する文字列 ---
	const StrIDRecord MemSpecialData[SIZE_MEM_SPECIAL_ID] = {
		{ "DUMMY",  MemSpecialID::DUMMY },
		{ "LAZY",   MemSpecialID::LAZY_FULL },
		{ "LAZY_S", MemSpecialID::LAZY_S },
		{ "LAZY_A", MemSpecialID::LAZY_A },
		{ "LAZY_E", MemSpecialID::LAZY_E },
		{ "",       MemSpecialID::NoData },
		{ "START",  MemSpecialID::LAZY_S },
		{ "AUTO",   MemSpecialID::LAZY_A },
		{ "END",    MemSpecialID::LAZY_E },
	};
	const string ScrMemStrLazy = "__LAZY__";	// 通常識別子に追加するLazy用保管文字列

public:
	JlsScrMemArg();
	void clearArg();
	void setNameByStr(string strName);
	void setNameByLazy(LazyType typeLazy);
	bool isExistBaseName();
	bool isExistExtName();
	bool isNameDummy();
	bool isNameSpecial();
	void getBaseName(string& strName);
	void getNameList(vector <string>& listName);

private:
	void setMapNameToBase(const string strName);
	void setMapNameToExt(const string strName);
	bool findSpecialName(MemSpecialID& idName, const string& strName);
	string getStringSpecialID(MemSpecialID idName);

private:
	bool m_flagDummy;
	bool m_flagSpecial;
	vector <string> m_listName;
};


///////////////////////////////////////////////////////////////////////
//
// スクリプトデータ保管クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScrMem
{
private:
	struct CopyFlagRecord {
		bool	add;
		bool	move;
	};
	struct MemDataRecord {
		int     order;
		string  buffer;
	};
	const int orderInitial = 50;	// 初期実行順位

public:
	JlsScrMem();
	bool isLazyExist(LazyType typeLazy);
	// 格納時の実行順位
	void setOrderForPush(int order);
	void resetOrderForPush();
	int  getOrderForPush();
	// 引数処理
	bool setDefArg(vector<string>& argDef);
	bool getDefArg(vector<string>& argDef, const string& strName);
	// バッファ処理
	void setUnusedFlag(const string& strName);
	bool pushStrByName(const string& strName, const string& strBuf);
	bool pushStrByLazy(LazyType typeLazy, const string& strBuf);
	bool getListByName(queue <string>& queStr, const string& strName);
	bool popListByName(queue <string>& queStr, const string& strName);
	bool getListByLazy(queue <string>& queStr, LazyType typeLazy);
	bool popListByLazy(queue <string>& queStr, LazyType typeLazy);
	bool eraseMemByName(const string& strName);
	bool copyMemByName(const string& strSrc, const string& strDst);
	bool moveMemByName(const string& strSrc, const string& strDst);
	bool appendMemByName(const string& strSrc, const string& strDst);
	void getMapForDebug(string& strBuf);

private:
	// 共通の引数からコマンド実行
	bool exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf, int order);
	bool exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags);
	bool exeCmdEraseMem(JlsScrMemArg& argDst);
	bool exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags);
	// 記憶領域の直接操作
	bool memPushStr(const string& strName, const string& strBuf, int order);
	bool memGetList(queue <string>& queStr, const string& strName, CopyFlagRecord flags);
	bool memErase(const string& strName);
	bool memCopy(const string& strSrc, const string& strDst, CopyFlagRecord flags);
	bool memIsExist(const string& strName);
	bool memIsNameExist(const string& strName);
	bool memIsNameExistArg(const string& strName);
	void addQueueLine(queue <MemDataRecord>& queDst, const string& strBuf, int order);
	void setQueueStr(queue <string>& queDstStr, queue <MemDataRecord>& queSrc, CopyFlagRecord flags);
	void setQueueFull(queue <MemDataRecord>& queDst, queue <MemDataRecord>& queSrc, CopyFlagRecord flags);
	// 未使用チェック
	void setUnused(JlsScrMemArg& marg);
	void clearUnused(JlsScrMemArg& marg);
public:
	bool getUnusedStr(string& strBuf);

private:
	int  m_orderHold;
	unordered_map <string, queue<MemDataRecord>> m_mapVar;
	unordered_map <string, vector<string>> m_mapArg;
	unordered_map <string, bool> m_mapUnused;
};
