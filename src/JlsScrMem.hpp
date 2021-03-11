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
		MAXSIZE
	};
	static const int SIZE_MEM_SPECIAL_ID = static_cast<int>(MemSpecialID::MAXSIZE);

	//--- MemSpecialIDに対応する文字列 ---
	const char MemSpecialString[SIZE_MEM_SPECIAL_ID][8] = {
		"DUMMY",
		"LAZY",
		"LAZY_S",
		"LAZY_A",
		"LAZY_E",
		""
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
	void getBaseName(string& strName);
	void getNameList(vector <string>& listName);

private:
	void setMapNameToBase(const string strName);
	void setMapNameToExt(const string strName);
	bool findSpecialName(MemSpecialID& idName, const string& strName);
	string getStringSpecialID(MemSpecialID idName);

private:
	bool m_flagDummy;
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

public:
	bool isLazyExist(LazyType typeLazy);
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
	bool exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf);
	bool exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags);
	bool exeCmdEraseMem(JlsScrMemArg& argDst);
	bool exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags);
	// 記憶領域の直接操作
	bool memPushStr(const string& strName, const string& strBuf);
	bool memGetList(queue <string>& queStr, const string& strName, CopyFlagRecord flags);
	bool memErase(const string& strName);
	bool memCopy(const string& strSrc, const string& strDst, CopyFlagRecord flags);
	bool memIsExist(const string& strName);
	bool memIsNameExist(const string& strName);
	void setQueue(queue <string>& queDst, queue <string>& queSrc, CopyFlagRecord flags);

private:
	unordered_map <string, queue<string>> m_mapVar;
};
