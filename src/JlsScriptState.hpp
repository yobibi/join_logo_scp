//
// JL�X�N���v�g�����Ԃ̕ێ��iCall�̐������쐬�j
//
#pragma once

class JlsScrGlobal;

///////////////////////////////////////////////////////////////////////
//
// �X�N���v�g����
//
///////////////////////////////////////////////////////////////////////
class JlsScriptState
{
private:
	enum class CondIfState {	// If���
		FINISHED,				// ���s�ς�
		PREPARE,				// �����s
		RUNNING					// ���s��
	};
	struct RepDepthHold {		// Repeat�e�l�X�g�̏��
		int  lineStart;			// �J�n�s
		int  countLoop;			// �J��Ԃ��c���
		int  extLineEnd;		// �x�����s�̃L���b�V���I���s
		int  extLineRet;		// �x�����s��Repeat�I����ɖ߂�s
		CacheExeType exeType;	// �x�����s�̎��
	};

public:
	JlsScriptState(JlsScrGlobal* globalState);
	void clear();
	// IF����
	int  ifBegin(bool flag_cond);
	int  ifEnd();
	int  ifElse(bool flag_cond);
	// Repeat����
	int  repeatBegin(int num);
private:
	int  repeatBeginNormal(RepDepthHold& holdval, const string& strCmdRepeat);
	int  repeatBeginExtend(RepDepthHold& holdval, const string& strCmdRepeat);
public:
	int  repeatEnd();
private:
	void repeatEndExtend(RepDepthHold& holdval);
	int  repeatExtMoveQueue(vector <string>& listCache, queue <string>& queSrc);
	void repeatExtBackQueue(queue <string>& queDst, vector <string>& listCache, int nFrom, int nTo);
public:
	// �x�����s�ۊǗ̈�A�N�Z�X�i�ǂݏo�����s�j
	bool setLazyExe(LazyType typeLazy, const string& strBuf);
private:
	bool setLazyExeProcS(queue <string>& queStr, const string& strBuf);
public:
	void setLazyFlush();
	bool setMemCall(const string& strName);
	// �x�����s�ۊǗ̈�A�N�Z�X�iglobal state�ɏ������܂�����j
	bool setLazyStore(LazyType typeLazy, const string& strBuf);
	void setLazyStateIniAuto(bool flag);
	bool isLazyStateIniAuto();
	bool setMemStore(const string& strName, const string& strBuf);
	bool setMemErase(const string& strName);
	bool setMemCopy(const string& strSrc, const string& strDst);
	bool setMemMove(const string& strSrc, const string& strDst);
	bool setMemAppend(const string& strSrc, const string& strDst);
	void setMemEcho(const string& strName);
	void setMemGetMapForDebug();
	// �x�����s�L���[����
private:
	bool popCacheExeLazyMem(string& strBuf);
	bool readRepeatExtCache(string& strBuf);
	void addCacheExeLazy(queue <string>& queStr, LazyType typeLazy);
	void addCacheExeMem(queue <string>& queStr);
	void addCacheExeCommon(queue <string>& queStr, CacheExeType typeCache, bool flagHead);
	bool popQueue(string& strBuf, queue <string>& queSrc);
	void addQueue(queue <string>& queDst, queue <string>& queSrc, bool flagHead);
public:
	// �L���b�V���f�[�^�ǂݏo��
	bool   readCmdCache(string& strBufOrg);
	bool   addCmdCache(string& strBufOrg);
	bool   readLazyMemNext(string& strBufOrg);
	// ��Ԏ擾
	int   isRemainNest();
	void  setCmdReturn(bool flag);
	bool  isCmdReturnExit();
	bool  isNeedRaw(CmdCat category);
	bool  isNeedFullDecode(CmdType cmdsel, CmdCat category);
	bool  isSkipCmd();
	bool  isInvalidCmdLine(CmdCat category);
public:
	bool  isLazyExe();
	LazyType getLazyExeType();
private:
	bool   isRepeatExtType();
	CacheExeType getRepeatExtType();
	CacheExeType getCacheExeType();
	bool  isMemExe();
public:
	void   setLazyStartType(LazyType typeLazy);
	bool   isLazyArea();
	LazyType getLazyStartType();
	void   setLazyAuto(bool flag);
	bool   isLazyAuto();
	void   startMemArea(const string& strName);
	void   endMemArea();
	bool   isMemArea();
	string getMemName();
	void   setMemDupe(bool flag);

private:
	//--- IF������ ---
	bool					m_ifSkip;			// IF�����O�i0=�ʏ�  1=�����O�Ŏ��s���Ȃ��j
	vector <CondIfState>	m_listIfState;		// �eIF�l�X�g�̏�ԁi���s�ς� �����s ���s���j
	//--- Repeat������ ---
	bool					m_repSkip;			// Repeat���s�i0=�ʏ�  1=�J��Ԃ��O��Ŏ��s�Ȃ��j
	int						m_repLineReadCache;	// �ǂݏo���L���b�V���s
	vector <string>			m_listRepCmdCache;	// repeat���̃R�}���h������L���b�V��
	vector <RepDepthHold>	m_listRepDepth;		// �J��Ԃ���ԕێ�
	int                     m_repLineExtRCache;	// �x�����s��repeat���̓ǂݏo���L���b�V���s
	vector <string>         m_listRepExtCache;	// �x�����s��repeat���̃R�}���h������L���b�V��
	//--- return�� ---
	bool					m_flagReturn;		// Return�R�}���h�ɂ��I��
	//--- �x������ ---
	CacheExeType            m_typeCacheExe;		// ���s�L���b�V���̑I��
	//--- lazy������ ---
	bool                    m_lazyAuto;			// LazyAuto�ݒ��ԁi0=��ݒ� 1=�ݒ�j
	LazyType                m_lazyStartType;	// LazyStart - EndLazy ���ԓ���lazy�ݒ�
	//--- mem������ ---
	bool                    m_memArea;			// Memory - EndMemory ���ԓ��ł�true
	string                  m_memName;			// Memory�R�}���h�Őݒ肳��Ă��鎯�ʎq
	bool                    m_memDupe;			// MemOnce�R�}���h��2��ڈȏ�̎�
	bool                    m_memSkip;			// Memory�R�}���h�d���ɂ��ȗ�
	//--- lazy/mem ���s�L���[�f�[�^ ---
	queue <string>  m_cacheExeLazyS;	// ���Ɏ��s����lazy���������ꂽ�R�}���h������(LAZY_S)
	queue <string>  m_cacheExeLazyA;	// ���Ɏ��s����lazy���������ꂽ�R�}���h������(LAZY_A)
	queue <string>  m_cacheExeLazyE;	// ���Ɏ��s����lazy���������ꂽ�R�}���h������(LAZY_E)
	queue <string>  m_cacheExeLazyO;	// ���Ɏ��s����lazy���������ꂽ�R�}���h������(LAZY_O)
	queue <string>  m_cacheExeMem;		// ���Ɏ��s����MemCall�ŌĂяo���ꂽ�R�}���h������

private:
	JlsScrGlobal  *pGlobalState;	// �O���[�o����ԎQ��
};


