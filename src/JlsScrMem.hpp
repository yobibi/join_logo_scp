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
		START,
		AUTO,
		END,
		MAXSIZE
	};
	static const int SIZE_MEM_SPECIAL_ID = static_cast<int>(MemSpecialID::MAXSIZE);

	struct StrIDRecord{
		char          str[8];	// ���ʎq������
		MemSpecialID  id;		// ���ۂɎg�p���镶����̎��ʎqID
	};
	//--- MemSpecialID�ɑΉ����镶���� ---
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
	const string ScrMemStrLazy = "__LAZY__";	// �ʏ펯�ʎq�ɒǉ�����Lazy�p�ۊǕ�����

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
	struct MemDataRecord {
		int     order;
		string  buffer;
	};
	const int orderInitial = 50;	// �������s����

public:
	JlsScrMem();
	bool isLazyExist(LazyType typeLazy);
	// �i�[���̎��s����
	void setOrderForPush(int order);
	void resetOrderForPush();
	int  getOrderForPush();
	// ��������
	bool setDefArg(vector<string>& argDef);
	bool getDefArg(vector<string>& argDef, const string& strName);
	// �o�b�t�@����
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
	// ���ʂ̈�������R�}���h���s
	bool exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf, int order);
	bool exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags);
	bool exeCmdEraseMem(JlsScrMemArg& argDst);
	bool exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags);
	// �L���̈�̒��ڑ���
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
	// ���g�p�`�F�b�N
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
