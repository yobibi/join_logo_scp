//
// JLスクリプト制御状態の保持（Callの数だけ作成）
//
#pragma once

class JlsScrGlobal;

///////////////////////////////////////////////////////////////////////
//
// スクリプト制御
//
///////////////////////////////////////////////////////////////////////
class JlsScriptState
{
private:
	enum class CondIfState {	// If状態
		FINISHED,				// 実行済み
		PREPARE,				// 未実行
		RUNNING					// 実行中
	};
	struct RepDepthHold {		// Repeat各ネストの状態
		int  lineStart;			// 開始行
		int  countLoop;			// 繰り返し残り回数
		int  extLineEnd;		// 遅延実行のキャッシュ終了行
		int  extLineRet;		// 遅延実行のRepeat終了後に戻る行
		CacheExeType exeType;	// 遅延実行の種類
	};

public:
	JlsScriptState(JlsScrGlobal* globalState);
	void clear();
	// IF処理
	int  ifBegin(bool flag_cond);
	int  ifEnd();
	int  ifElse(bool flag_cond);
	// Repeat処理
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
	// 遅延実行保管領域アクセス（読み出し実行）
	bool setLazyExe(LazyType typeLazy, const string& strBuf);
private:
	bool setLazyExeProcS(queue <string>& queStr, const string& strBuf);
public:
	void setLazyFlush();
	bool setMemCall(const string& strName);
	// 遅延実行保管領域アクセス（global stateに処理をまかせる）
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
	// 遅延実行キュー処理
private:
	bool popCacheExeLazyMem(string& strBuf);
	bool readRepeatExtCache(string& strBuf);
	void addCacheExeLazy(queue <string>& queStr, LazyType typeLazy);
	void addCacheExeMem(queue <string>& queStr);
	void addCacheExeCommon(queue <string>& queStr, CacheExeType typeCache, bool flagHead);
	bool popQueue(string& strBuf, queue <string>& queSrc);
	void addQueue(queue <string>& queDst, queue <string>& queSrc, bool flagHead);
public:
	// キャッシュデータ読み出し
	bool   readCmdCache(string& strBufOrg);
	bool   addCmdCache(string& strBufOrg);
	bool   readLazyMemNext(string& strBufOrg);
	// 状態取得
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
	//--- IF文制御 ---
	bool					m_ifSkip;			// IF条件外（0=通常  1=条件外で実行しない）
	vector <CondIfState>	m_listIfState;		// 各IFネストの状態（実行済み 未実行 実行中）
	//--- Repeat文制御 ---
	bool					m_repSkip;			// Repeat実行（0=通常  1=繰り返し０回で実行なし）
	int						m_repLineReadCache;	// 読み出しキャッシュ行
	vector <string>			m_listRepCmdCache;	// repeat中のコマンド文字列キャッシュ
	vector <RepDepthHold>	m_listRepDepth;		// 繰り返し状態保持
	int                     m_repLineExtRCache;	// 遅延実行内repeat中の読み出しキャッシュ行
	vector <string>         m_listRepExtCache;	// 遅延実行内repeat中のコマンド文字列キャッシュ
	//--- return文 ---
	bool					m_flagReturn;		// Returnコマンドによる終了
	//--- 遅延制御 ---
	CacheExeType            m_typeCacheExe;		// 実行キャッシュの選択
	//--- lazy文制御 ---
	bool                    m_lazyAuto;			// LazyAuto設定状態（0=非設定 1=設定）
	LazyType                m_lazyStartType;	// LazyStart - EndLazy 期間内のlazy設定
	//--- mem文制御 ---
	bool                    m_memArea;			// Memory - EndMemory 期間内ではtrue
	string                  m_memName;			// Memoryコマンドで設定されている識別子
	bool                    m_memDupe;			// MemOnceコマンドで2回目以上の時
	bool                    m_memSkip;			// Memoryコマンド重複による省略
	//--- lazy/mem 実行キューデータ ---
	queue <string>  m_cacheExeLazyS;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_S)
	queue <string>  m_cacheExeLazyA;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_A)
	queue <string>  m_cacheExeLazyE;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_E)
	queue <string>  m_cacheExeLazyO;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_O)
	queue <string>  m_cacheExeMem;		// 次に実行するMemCallで呼び出されたコマンド文字列

private:
	JlsScrGlobal  *pGlobalState;	// グローバル状態参照
};


