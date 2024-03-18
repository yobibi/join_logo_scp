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
		bool extFlagNest;		// 遅延実行のキャッシュ内のRepeatネスト
		int  varStep;			// 回数連動変数ステップ数
		string varName;			// 回数連動変数名
		int  layerReg;			// 開始時のローカル変数階層
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
	int  repeatBeginExtNest(RepDepthHold& holdval);
public:
	int  repeatEnd();
private:
	int  repeatEndForce();
	int  repeatEndMain(bool force);
	void repeatEndExtend(RepDepthHold& holdval);
	int  repeatExtMoveQueue(vector <string>& listCache, queue <string>& queSrc);
	void repeatExtBackQueue(queue <string>& queDst, vector <string>& listCache, int nFrom, int nTo);
	void releaseBreak();
public:
	bool setBreak();
	void repeatVarSet(const string& name, int step);
	bool repeatVarGet(string& name, int& step);
	// 遅延実行保管領域アクセス（読み出し実行）
	bool setLazyExe(LazyType typeLazy, const string& strBuf);
private:
	bool setLazyExeProcS(queue <string>& queStr, const string& strBuf);
public:
	void setLazyFlush();
	bool setMemCall(const string& strName);
	void setMemOrder(int order);
	void releaseMemOrder();
	bool setMemDefArg(vector<string>& argDef);
	bool getMemDefArg(vector<string>& argDef, const string& strName);
	void setMemUnusedFlag(const string& strName);
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
	bool  isFlowLazy(CmdCat category);
	bool  isFlowMem(CmdCat category);
	bool  isMemDeepArea();
	bool  isNeedRaw(CmdCat category);
	bool  isNeedFullDecode(CmdType cmdsel, CmdCat category);
	bool  isSkipCmd();
	bool  isInvalidCmdLine(CmdCat category);
	void  addNestInfoForEnd(CmdType& cmdsel, CmdCat& category);
private:
	bool  addNestInfoForEndRemove(CmdType& cmdtarget, CmdCat& cattarget, CmdType cmdsel);
	void  addNestInfoForBreak(CmdType cmdtarget, bool opAdd, bool opRemove);
public:
	bool  isLazyExe();
	LazyType getLazyExeType();
private:
	bool   isRepeatExtType();
	CacheExeType getRepeatExtType();
	CacheExeType getCacheExeType();
public:
	bool  isMemExe();
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
	void   setMemExpand(bool flag);
	// Call引数用処理
	void   setArgAreaEnter(bool flag);
	bool   isArgAreaEnter();
	void   addArgAreaName(const string& strName);
	int    sizeArgAreaNameList();
	bool   getArgAreaName(string& strName, int num);
	// 保管型メモリ引数の格納
	void   clearArgMstoreBuf();
	void   addArgMstoreBuf(const string& strBuf);
	void   exeArgMstoreInsert(CmdType cmdsel);
private:
	void   useArgMstoreBuf();
	bool   isArgMstoreBufEmpty();
public:
	void pushBufDivCmd(const string& str);
	bool popBufDivCmd(string& str);

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
	bool                    m_flagBreak;		// Break期間中
	//--- return文 ---
	bool					m_flagReturn;		// Returnコマンドによる終了
	//--- 遅延制御 ---
	CacheExeType            m_typeCacheExe;		// 実行キャッシュの選択
	bool                    m_flagCacheRepExt;	// 遅延実行用Repeatキャッシュから読み出し
	//--- lazy文制御 ---
	bool                    m_lazyAuto;			// LazyAuto設定状態（0=非設定 1=設定）
	LazyType                m_lazyStartType;	// LazyStart - EndLazy 期間内のlazy設定
	//--- mem文制御 ---
	bool                    m_memArea;			// Memory - EndMemory 期間内ではtrue
	string                  m_memName;			// Memoryコマンドで設定されている識別子
	bool                    m_memDupe;			// MemOnceコマンドで2回目以上の時
	bool                    m_memSkip;			// Memoryコマンド重複による省略
	int                     m_memOrderVal;		// 遅延保管の実行順位（現在設定）
	//--- mem/lazy文制御 ---
	int                     m_memExpand;		// Memory/LazyStart内の変数展開
	//--- Endコマンド制御 ---
	vector <CmdType>        m_listNestECmd;		// 待機するEnd種類を順番に格納（コマンド）
	vector <CmdCat>         m_listNestECat;		// 待機するEnd種類を順番に格納（カテゴリ）
	int                     m_nestMemNow;		// Memoryネスト数（現在）
	int                     m_nestMemLast;		// Memoryネスト数（前回）
	int                     m_nestBreakIf;		// Break中のIf数
	int                     m_nestBreakRep;		// Break中のRepeat数
	//--- 引数名前制御 ---
	bool                    m_flagArgArEnter;	// ArgBegin - ArgEnd 期間内ではtrue
	vector <string>         m_listArgArName;	// 引数ローカル変数の名前
	//---メモリ引数 ---
	queue <string>          m_queArgMsBuf;		// 挿入予定のローカル変数設定コマンド文字列
	//--- コマンド分割対応の一時格納用 ---
	string m_bufCmdDivHold;
	//--- lazy/mem 実行キューデータ ---
	queue <string>  m_cacheExeLazyS;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_S)
	queue <string>  m_cacheExeLazyA;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_A)
	queue <string>  m_cacheExeLazyE;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_E)
	queue <string>  m_cacheExeLazyO;	// 次に実行するlazyから解放されたコマンド文字列(LAZY_O)
	queue <string>  m_cacheExeMem;		// 次に実行するMemCallで呼び出されたコマンド文字列

private:
	JlsScrGlobal  *pGlobalState;	// グローバル状態参照
};


