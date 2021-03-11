//
// �x�����s�R�}���h�̕ۊ�
//
// �N���X�\��
//   JlsScrMem         : �x�����s�R�}���h�ۊ�
//     |- JlsScrMemArg : ���ꕶ����̉�́E�ݒ�
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// �x�����s�ۊǗp�̎��ʎq�ێ��N���X
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

	//--- MemSpecialID�ɑΉ����镶���� ---
	const char MemSpecialString[SIZE_MEM_SPECIAL_ID][8] = {
		"DUMMY",
		"LAZY",
		"LAZY_S",
		"LAZY_A",
		"LAZY_E",
		""
	};
	const string ScrMemStrLazy = "__LAZY__";	// �ʏ펯�ʎq�ɒǉ�����Lazy�p�ۊǕ�����

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
// �X�N���v�g�f�[�^�ۊǃN���X
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
	// ���ʂ̈�������R�}���h���s
	bool exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf);
	bool exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags);
	bool exeCmdEraseMem(JlsScrMemArg& argDst);
	bool exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags);
	// �L���̈�̒��ڑ���
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
