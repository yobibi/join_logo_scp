//
// JLスクリプト制御状態の保持（Callの数だけ作成）
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptState.hpp"
#include "JlsScrGlobal.hpp"

///////////////////////////////////////////////////////////////////////
//
// スクリプト制御
//
///////////////////////////////////////////////////////////////////////
JlsScriptState::JlsScriptState(JlsScrGlobal* globalState){
	this->pGlobalState = globalState;
	clear();
}

//---------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------
void JlsScriptState::clear(){
	m_ifSkip = false;
	m_listIfState.clear();
	m_repSkip = false;
	m_repLineReadCache = -1;
	m_listRepCmdCache.clear();
	m_listRepDepth.clear();
	m_repLineExtRCache = -1;
	m_listRepExtCache.clear();
	m_flagReturn = false;
	m_typeCacheExe = CacheExeType::None;
	m_lazyAuto = false;
	m_lazyStartType = LazyType::None;
	m_memArea = false;
	m_memName = "";
	m_memDupe = false;
	m_memSkip = false;
	queue<string>().swap(m_cacheExeLazyS);
	queue<string>().swap(m_cacheExeLazyA);
	queue<string>().swap(m_cacheExeLazyE);
	queue<string>().swap(m_cacheExeLazyO);
	queue<string>().swap(m_cacheExeMem);
}


//=====================================================================
// If処理
//=====================================================================

//---------------------------------------------------------------------
// If文設定
//   入力:   flag_cond=条件式
//   返り値: エラー番号（0=正常終了、1=エラー）
//---------------------------------------------------------------------
int JlsScriptState::ifBegin(bool flag_cond){
	CondIfState stat;
	if (m_ifSkip){
		m_ifSkip = true;
		stat = CondIfState::FINISHED;		// 条件：終了後
	}
	else if (flag_cond == false){
		m_ifSkip = true;
		stat = CondIfState::PREPARE;		// 条件：未実行
	}
	else{
		m_ifSkip = false;
		stat = CondIfState::RUNNING;		// 条件：実行
	}
	m_listIfState.push_back(stat);			// リストに保存
	return 0;
}

//---------------------------------------------------------------------
// EndIf文設定
//   返り値: エラー番号（0=正常終了、1=エラー）
//---------------------------------------------------------------------
int JlsScriptState::ifEnd(){
	int depth = (int) m_listIfState.size();
	if (depth <= 0){
		return 1;
	}
	m_listIfState.pop_back();				// リストから削除
	depth --;
	if (depth <= 0){
		m_ifSkip = false;
	}
	else{
		if (m_listIfState[depth-1] == CondIfState::RUNNING){
			m_ifSkip = false;
		}
		else{
			m_ifSkip = true;
		}
	}
	return 0;
}

//---------------------------------------------------------------------
// ElsIf文設定
//   入力:   flag_cond=条件式
//   返り値: エラー番号（0=正常終了、1=エラー）
//---------------------------------------------------------------------
int JlsScriptState::ifElse(bool flag_cond){
	int depth = (int) m_listIfState.size();
	if (depth <= 0){
		return 1;
	}
	CondIfState stat_cur = m_listIfState[depth-1];
	CondIfState stat_nxt = stat_cur;
	switch(stat_cur){
		case CondIfState::FINISHED:
		case CondIfState::RUNNING:
			m_ifSkip = true;
			stat_nxt = CondIfState::FINISHED;
			break;
		case CondIfState::PREPARE:
			if (flag_cond){
				m_ifSkip = false;
				stat_nxt = CondIfState::RUNNING;
			}
			else{
				m_ifSkip = true;
				stat_nxt = CondIfState::PREPARE;
			}
			break;
	}
	m_listIfState[depth-1] = stat_nxt;
	return 0;
}


//=====================================================================
// Repeat処理
//=====================================================================

//---------------------------------------------------------------------
// Repeat文設定
//   入力:   繰り返し回数
//   返り値: エラー番号（0=正常終了、1=エラー）
//---------------------------------------------------------------------
int JlsScriptState::repeatBegin(int num){
	//--- 最初のリピート処理 ---
	int depth = (int) m_listRepDepth.size();
	if (depth == 0){
		m_repSkip = false;					// 飛ばさない
		m_repLineReadCache = -1;			// キャッシュ読み出し無効
	}
	//--- リピートコマンドキャッシュ用 ---
	string strCmdRepeat = "Repeat " + to_string(num);
	//--- リピート情報を設定 ---
	if (num <= 0 || m_repSkip){				// 最初から実行なしの場合
		num = -1;							// 実行なし時の回数は-1にする
		m_repSkip = true;					// コマンドを飛ばす
	}
	//--- 設定保存 ---
	RepDepthHold holdval;
	holdval.countLoop = num;				// 繰り返し回数を設定
	//--- 遅延実行処理の拡張 ---
	int errval = 0;
	if ( isLazyExe() || isMemExe() ){
		errval = repeatBeginExtend(holdval, strCmdRepeat);
	}else{
		errval = repeatBeginNormal(holdval, strCmdRepeat);
	}
	if ( errval == 0 ){
		m_listRepDepth.push_back(holdval);		// リストに保存
	}
	return errval;
}
//--- 通常のRepeat設定 ---
int JlsScriptState::repeatBeginNormal(RepDepthHold& holdval, const string& strCmdRepeat){
	//--- キャッシュ行数チェック ---
	int size_line = (int) m_listRepCmdCache.size();
	if (size_line >= SIZE_REPLINE){			// 最大行数チェック
		return 1;
	}
	//--- キャッシュしてない場合は現在コマンドをキャッシュ ---
	if (size_line == 0){
		m_listRepCmdCache.push_back(strCmdRepeat);
		size_line ++;
	}
	int line_start = m_repLineReadCache;
	if (line_start < 0) line_start = size_line;
	//--- 設定保存 ---
	holdval.lineStart = line_start;			// 開始行を設定
	//--- 拡張無効設定 ---
	holdval.extLineEnd = 0;
	holdval.extLineRet = 0;
	holdval.exeType = CacheExeType::None;
	return 0;
}
//--- 遅延実行中のRepeat設定 ---
int JlsScriptState::repeatBeginExtend(RepDepthHold& holdval, const string& strCmdRepeat){
	//--- キャッシュ行数チェック ---
	int size_line = (int)m_listRepExtCache.size();
	if (size_line >= SIZE_REPLINE){			// 最大行数チェック
		return 1;
	}
	//--- 拡張リピートキャッシュにコマンド行を格納 ---
	m_listRepExtCache.push_back(strCmdRepeat);
	holdval.lineStart  = (int)m_listRepExtCache.size();
	holdval.exeType    = getCacheExeType();
	int errval = 1;
	switch( holdval.exeType ){
		case CacheExeType::LazyS :
			errval = repeatExtMoveQueue(m_listRepExtCache, m_cacheExeLazyS);
			break;
		case CacheExeType::LazyA :
			errval = repeatExtMoveQueue(m_listRepExtCache, m_cacheExeLazyA);
			break;
		case CacheExeType::LazyE :
			errval = repeatExtMoveQueue(m_listRepExtCache, m_cacheExeLazyE);
			break;
		case CacheExeType::Mem :
			errval = repeatExtMoveQueue(m_listRepExtCache, m_cacheExeMem);
			break;
		default :
			break;
	}
	
	holdval.extLineEnd = (int)m_listRepExtCache.size() - 1;
	if ( holdval.lineStart > holdval.extLineEnd ){
		return 1;		// error
	}
	if ( errval == 0 ){
		holdval.extLineRet = m_repLineExtRCache;
		m_repLineExtRCache = holdval.lineStart;
	}
	return errval;
}
//---------------------------------------------------------------------
// EndRepeat文設定
//   返り値: エラー番号（0=正常終了、1=エラー）
//---------------------------------------------------------------------
int JlsScriptState::repeatEnd(){
	int depth = (int) m_listRepDepth.size();
	if (depth <= 0){
		return 1;
	}
	if (m_listRepDepth[depth-1].countLoop > 0){		// カウントダウン
		m_listRepDepth[depth-1].countLoop --;
	}
	bool extMode = isRepeatExtType();
	if (m_listRepDepth[depth-1].countLoop > 0){		// 繰り返し継続の場合
		if ( extMode ){
			m_repLineExtRCache = m_listRepDepth[depth-1].lineStart;	// 読み出し行設定
		}else{
			m_repLineReadCache = m_listRepDepth[depth-1].lineStart;	// 読み出し行設定
		}
	}
	else{											// 繰り返し終了の場合
		if ( extMode ){								// 遅延実行モード用
			repeatEndExtend(m_listRepDepth[depth-1]);
		}
		m_listRepDepth.pop_back();					// リストから削除
		depth --;
		//--- 全リピート終了時の処理 ---
		if (depth == 0){
			m_listRepCmdCache.clear();				// キャッシュ文字列の消去
			m_listRepExtCache.clear();				// 遅延実行用拡張キャッシュ
			m_repSkip = false;
		//--- 飛ばし状態を更新 ---
		}else if (m_repSkip){
			if (m_listRepDepth[depth-1].countLoop >= 0){
				m_repSkip = false;
			}
		}
	}
	return 0;
}
//--- 遅延実行中のRepeat設定 ---
void JlsScriptState::repeatEndExtend(RepDepthHold& holdval){
	int nNext = m_repLineExtRCache;
	int nEnd  = holdval.extLineEnd;
	//--- 未実行部分をqueueに戻す ---
	if ( nNext <= nEnd ){
		switch( holdval.exeType ){
			case CacheExeType::LazyS :
				repeatExtBackQueue(m_cacheExeLazyS, m_listRepExtCache, nNext, nEnd);
				break;
			case CacheExeType::LazyA :
				repeatExtBackQueue(m_cacheExeLazyA, m_listRepExtCache, nNext, nEnd);
				break;
			case CacheExeType::LazyE :
				repeatExtBackQueue(m_cacheExeLazyE, m_listRepExtCache, nNext, nEnd);
				break;
			case CacheExeType::Mem :
				repeatExtBackQueue(m_cacheExeMem, m_listRepExtCache, nNext, nEnd);
				break;
			default:
				break;
		}
	}
	//--- リピートCacheの不要になる部分を削除 ---
	if ( nEnd + 1 == (int)m_listRepExtCache.size() ){
		for(int i = holdval.lineStart - 1; i <= nEnd; i++){
			if ( m_listRepExtCache.empty() == false ) m_listRepExtCache.pop_back();
		}
	}
	//--- キャッシュ実行行をRepeat前に戻す ---
	m_repLineExtRCache = holdval.extLineRet;
}
//--- queueデータを遅延実行内のリピート用キャッシュに移動 ---
int JlsScriptState::repeatExtMoveQueue(vector <string>& listCache, queue <string>& queSrc){
	if ( listCache.size() + queSrc.size() >= SIZE_MEMLINE){		// 最大行数チェック
		pGlobalState->addMsgError("error: memory cache for repeat overflow\n");
		return 1;
	}
	while( queSrc.empty() == false ){
		listCache.push_back( queSrc.front() );
		queSrc.pop();
	}
	return 0;
}
//--- リピート用キャッシュからqueueにデータを戻す ---
void JlsScriptState::repeatExtBackQueue(queue <string>& queDst, vector <string>& listCache, int nFrom, int nTo){
	if ( queDst.size() + nTo - nFrom + 1 >= SIZE_MEMLINE ){		// 最大行数チェック
		pGlobalState->addMsgError("error: memory cache for repeat overflow\n");
		return;
	}
	queue <string> q;
	queDst.swap(q);			// queDstクリア + qに一時保持
	for(int i = nFrom; i <= nTo; i++){
		queDst.push( listCache[i] );
	}
	while( q.empty() == false ){
		queDst.push( q.front() );
		q.pop();
	}
	return;
}


//=====================================================================
// 遅延実行保管領域へのアクセス
//=====================================================================

//---------------------------------------------------------------------
// lazy処理で保管されたコマンドを実行に移す
// 入力：
//   typeLazy  : LazyS, LazyA, LazyE, FULL
//   strBuf   : 現在行の文字列（保管コマンドを先に実行する時にキャッシュへ移す）
// 入出力：
//   state    : lazy実行処理追加
// 出力：
//   返り値   ：現在行のコマンド実行有無（実行キャッシュに移した時は実行しない）
//---------------------------------------------------------------------
bool JlsScriptState::setLazyExe(LazyType typeLazy, const string& strBuf){
	//--- 実行処理 ---
	bool enableExe = true;
	if ( pGlobalState->isLazyExist(LazyType::FULL) ){		// lazyコマンド使用実績があれば確認する
		queue <string> queStrS;
		queue <string> queStrA;
		queue <string> queStrE;
		//--- Lazy保管行の取り出し ---
		switch( typeLazy ){
			case LazyType::LazyS:
				enableExe = setLazyExeProcS(queStrS, strBuf);	// （検出直前lazy + 現在行）
				break;
			case LazyType::LazyA:
				//--- 検出直前lazyが残っていたら先に実行 ---
				enableExe = setLazyExeProcS(queStrS, strBuf);	// （検出直前lazy + 現在行）
				pGlobalState->popListByLazy(queStrA, LazyType::LazyA);	// （auto直後lazy）
				break;
			case LazyType::LazyE:
				pGlobalState->popListByLazy(queStrE, LazyType::LazyE);	// （終了後lazy）
				break;
			case LazyType::FULL:
				//--- lazy_eまでの全部をFlush動作 ---
				enableExe = setLazyExeProcS(queStrS, strBuf);	// （検出直前lazy + 現在行）
				pGlobalState->popListByLazy(queStrA, LazyType::LazyA);	// （auto直後lazy）
				pGlobalState->popListByLazy(queStrE, LazyType::LazyE);	// （終了後lazy）
				break;
			default:
				break;
		}
		if ( queStrS.empty() == false ){
			addCacheExeLazy(queStrS, LazyType::LazyS);	// Lazy実行キャッシュに移動
		}
		if ( queStrA.empty() == false ){
			addCacheExeLazy(queStrA, LazyType::LazyA);	// Lazy実行キャッシュに移動
		}
		if ( queStrE.empty() == false ){
			addCacheExeLazy(queStrE, LazyType::LazyE);	// Lazy実行キャッシュに移動
		}
	}
	return enableExe;
}
//---------------------------------------------------------------------
// （サブルーチン）保管されたデータの後に現在行を実行するための処理
// 返り値   ：現在行のコマンド実行有無（現在行をリスト内に入れたら実行しない）
//---------------------------------------------------------------------
bool JlsScriptState::setLazyExeProcS(queue <string>& queStr, const string& strBuf){
	bool enableExe = true;
	//--- 検出直前lazyを取り込み、成功したら現在行をその後に追加 ---
	bool success = pGlobalState->popListByLazy(queStr, LazyType::LazyS);	
	if ( success ){
		enableExe = false;
		if ( strBuf.empty() == false ){
			queStr.push(strBuf);
		}
	}
	return enableExe;
}
//---------------------------------------------------------------------
// lazy処理で保管されたコマンドを全部実行に移す
// 入出力：
//   state    : lazy実行処理追加
//---------------------------------------------------------------------
void JlsScriptState::setLazyFlush(){
	setLazyExe(LazyType::FULL, "");
}
//---------------------------------------------------------------------
// memory処理で保管されたコマンドを実行に移す(MemCall)
// 入力：
//   strName  : 保管領域の識別子
// 入出力：
//   state    : MemCall実行処理追加
// 出力：
//   返り値   ：true=成功、false=失敗
//---------------------------------------------------------------------
bool JlsScriptState::setMemCall(const string& strName){
	queue <string> queStr;
	bool enable_exe = pGlobalState->getListByName(queStr, strName);
	addCacheExeMem(queStr);							// Mem実行キャッシュに移動
	return enable_exe;
}
//--- global state側で処理 ---
bool JlsScriptState::setLazyStore(LazyType typeLazy, const string& strBuf){
	return pGlobalState->setLazyStore(typeLazy, strBuf);
}
void JlsScriptState::setLazyStateIniAuto(bool flag){
	pGlobalState->setLazyStateIniAuto(flag);
}
bool JlsScriptState::isLazyStateIniAuto(){
	return pGlobalState->isLazyStateIniAuto();
}
bool JlsScriptState::setMemStore(const string& strName, const string& strBuf){
	return pGlobalState->setMemStore(strName, strBuf);
}
bool JlsScriptState::setMemErase(const string& strName){
	return pGlobalState->setMemErase(strName);
}
bool JlsScriptState::setMemCopy(const string& strSrc, const string& strDst){
	return pGlobalState->setMemCopy(strSrc, strDst);
}
bool JlsScriptState::setMemMove(const string& strSrc, const string& strDst){
	return pGlobalState->setMemMove(strSrc, strDst);
}
bool JlsScriptState::setMemAppend(const string& strSrc, const string& strDst){
	return pGlobalState->setMemAppend(strSrc, strDst);
}
void JlsScriptState::setMemEcho(const string& strName){
	pGlobalState->setMemEcho(strName);
}
void JlsScriptState::setMemGetMapForDebug(){
	pGlobalState->setMemGetMapForDebug();
}


//=====================================================================
// 遅延実行キュー処理
//=====================================================================

//---------------------------------------------------------------------
// 実行キャッシュを１行読み出し
//---------------------------------------------------------------------
//--- Lazy/Memコマンド実行キャッシュ ---
bool JlsScriptState::popCacheExeLazyMem(string& strBuf){
	//--- 遅延実行用リピートCache確認 ---
	CacheExeType typeRepeat = getRepeatExtType();

	//--- read先の選択 ---
	bool flagExt = false;
	CacheExeType typeRead = CacheExeType::None;

	if ( m_cacheExeLazyS.empty() == false ){	// LazyS 実行キャッシュ存在
		flagExt   = false;
		typeRead  = CacheExeType::LazyS;
	}
	else if ( typeRepeat == CacheExeType::LazyS ){	// LazyS Repeatキャッシュ存在
		flagExt   = true;
		typeRead  = CacheExeType::LazyS;
	}
	else if ( m_cacheExeLazyA.empty() == false ){	// LazyA 実行キャッシュ存在
		flagExt   = false;
		typeRead  = CacheExeType::LazyA;
	}
	else if ( typeRepeat == CacheExeType::LazyA ){	// LazyA Repeatキャッシュ存在
		flagExt   = true;
		typeRead  = CacheExeType::LazyA;
	}
	else if ( m_cacheExeLazyE.empty() == false ){	// LazyE 実行キャッシュ存在
		flagExt   = false;
		typeRead  = CacheExeType::LazyE;
	}
	else if ( typeRepeat == CacheExeType::LazyE ){	// LazyE Repeatキャッシュ存在
		flagExt   = true;
		typeRead  = CacheExeType::LazyE;
	}
	else if ( m_cacheExeMem.empty() == false ){	// Mem 実行キャッシュ存在
		flagExt   = false;
		typeRead  = CacheExeType::Mem;
	}
	else if ( typeRepeat == CacheExeType::Mem ){	// Mem Repeatキャッシュ存在
		flagExt   = true;
		typeRead  = CacheExeType::Mem;
	}
	//--- Cache読み出し ---
	bool flagRead = false;
	if ( typeRead != CacheExeType::None ){
		if ( flagExt ){
			flagRead = readRepeatExtCache(strBuf);	// 遅延実行用Repeatキャッシュから読み出し
		}else{
			switch( typeRead ){
				case CacheExeType::LazyS :
					flagRead = popQueue(strBuf, m_cacheExeLazyS);
					break;
				case CacheExeType::LazyA :
					flagRead = popQueue(strBuf, m_cacheExeLazyA);
					break;
				case CacheExeType::LazyE :
					flagRead = popQueue(strBuf, m_cacheExeLazyE);
					break;
				case CacheExeType::Mem :
					flagRead = popQueue(strBuf, m_cacheExeMem);
					break;
				default :
					flagRead = false;
					break;
			}
		}
	}
	m_typeCacheExe = typeRead;		// 読み出し先キャッシュ設定
	return flagRead;
}
//---------------------------------------------------------------------
// 遅延実行内リピート中の次のコマンド取得
//   返り値: 格納実行（false=格納不要、true=格納済み）
//---------------------------------------------------------------------
bool JlsScriptState::readRepeatExtCache(string& strBuf){
	int nSize = (int)m_listRepExtCache.size();
	if ( m_repLineExtRCache >= 0 && m_repLineExtRCache < nSize ){
		strBuf = m_listRepExtCache[m_repLineExtRCache];
		m_repLineExtRCache ++;
	}else{
		string msgErr = "error: not found EndRepeat at Lazy/Mem cache Line:";
		msgErr += to_string(m_repLineExtRCache) + "\n";
		pGlobalState->addMsgError(msgErr);
	}
	return true;
}
//---------------------------------------------------------------------
// 実行キャッシュを設定
//---------------------------------------------------------------------
//--- Lazy実行で呼び出された行を設定 ---
void JlsScriptState::addCacheExeLazy(queue <string>& queStr, LazyType typeLazy){
	bool flagHead = false;				// バッファに追加挿入
	CacheExeType typeCache = CacheExeType::None;
	switch( typeLazy ){
		case LazyType::LazyS :
			typeCache = CacheExeType::LazyS;
			break;
		case LazyType::LazyA :
			typeCache = CacheExeType::LazyA;
			break;
		case LazyType::LazyE :
			typeCache = CacheExeType::LazyE;
			break;
		default:
			pGlobalState->addMsgError("error: internal at JlsScriptState::addCacheExeLazy\n");
			break;
	}
	addCacheExeCommon(queStr, typeCache, flagHead);
}
//--- MemCallで呼び出された行を設定 ---
void JlsScriptState::addCacheExeMem(queue <string>& queStr){
	if ( isLazyExe() ){
		//--- Lazy実行中のMemCallは残りのLazy実行より先に即実行 ---
		bool flagHead = true;			// 現バッファより先に挿入
		addCacheExeCommon(queStr, m_typeCacheExe, flagHead);
	}
	else{
		//--- 通常はLazy処理より優先度が低いMemCall実行待ちバッファに挿入 ---
		bool flagHead = true;			// 現バッファより先に挿入
		addCacheExeCommon(queStr, CacheExeType::Mem, flagHead);
	}
}
void JlsScriptState::addCacheExeCommon(queue <string>& queStr, CacheExeType typeCache, bool flagHead){
	switch( typeCache ){
		case CacheExeType::LazyS :
			addQueue(m_cacheExeLazyS, queStr, flagHead);
			break;
		case CacheExeType::LazyA :
			addQueue(m_cacheExeLazyA, queStr, flagHead);
			break;
		case CacheExeType::LazyE :
			addQueue(m_cacheExeLazyE, queStr, flagHead);
			break;
		case CacheExeType::Mem :
			addQueue(m_cacheExeMem, queStr, flagHead);
			break;
		default :
			pGlobalState->addMsgError("error: internal at JlsScriptState::addCacheExeCommon\n");
			break;
	}
}
//---------------------------------------------------------------------
// queue処理
//---------------------------------------------------------------------
//--- queueの先頭データを取り出し ---
bool JlsScriptState::popQueue(string& strBuf, queue <string>& queSrc){
	if ( queSrc.empty() ) return false;		// queueが空ならfalseを返す
	strBuf = queSrc.front();
	queSrc.pop();
	return true;
}
//--- queueに別のqueueを追加 ---
void JlsScriptState::addQueue(queue <string>& queDst, queue <string>& queSrc, bool flagHead){
	queue <string> q;
	//--- キャッシュ保持最大行数を超えないか確認 ---
	if ( queDst.size() + queSrc.size() > SIZE_MEMLINE){
		pGlobalState->addMsgError("error: memory cache overflow\n");
		queDst = q;			// キャッシュクリア
		return;
	}
	if ( flagHead ){		// 手前にSrcを追加する場合
		q      = queDst;
		queDst = queSrc;
	}
	else{					// 後にSrcを追加する場合
		q = queSrc;
	}
	while( q.empty() == false ){
		queDst.push( q.front() );
		q.pop();
	}
}


//=====================================================================
// キャッシュデータ読み出し
//=====================================================================

//---------------------------------------------------------------------
// Cacheからの読み出し（Repeat用）
//   返り値: 読み出し結果（false=読み出しなし、true=cacheからの読み出し）
//   strBufOrg: 読み出された文字列
//---------------------------------------------------------------------
bool JlsScriptState::readCmdCache(string& strBufOrg){
	//--- 読み出し可能かチェック ---
	if ( (int)m_listRepDepth.size() <= 0 ){		// Repeat中ではない
		return false;
	}
	if ( m_repLineReadCache >= (int)m_listRepCmdCache.size() ){
		m_repLineReadCache = -1;
	}
	if (m_repLineReadCache < 0) return false;

	//--- 読み出し実行 ---
	strBufOrg = m_listRepCmdCache[m_repLineReadCache];
	m_repLineReadCache ++;
	return true;
}
//---------------------------------------------------------------------
// Cacheに文字列格納（Repeat用）
//   入力:   strBufOrg=格納文字列
//   返り値: 格納実行（false=格納不要、true=格納済み）
//---------------------------------------------------------------------
bool JlsScriptState::addCmdCache(string& strBufOrg){
	if ( (int)m_listRepDepth.size() <= 0 ){		// Repeat中ではない
		return false;
	}
	//--- キャッシュ行数チェック ---
	int size_line = (int) m_listRepCmdCache.size();
	if (size_line >= SIZE_REPLINE){			// 最大行数チェック
		return false;
	}
	m_listRepCmdCache.push_back(strBufOrg);
	return true;
}
//---------------------------------------------------------------------
// Lazy/Mem実行行を読み出し
// 出力：
//   返り値   ：文字列取得結果（0=取得なし  1=取得あり）
//   strBufOrg : 取得文字列
//---------------------------------------------------------------------
bool JlsScriptState::readLazyMemNext(string& strBufOrg){
	//--- Lazy/Mem実行行があれば先頭を読み出し ---
	if ( popCacheExeLazyMem(strBufOrg) ){
		return true;
	}
	//--- Lazy/Mem実行行がなければfalseを返す ---
	return false;
}


//=====================================================================
// 状態取得
//=====================================================================

//---------------------------------------------------------------------
// ネスト状態が残っているか確認
//   返り値: エラー番号（0=正常終了、bit0=If文ネスト中、bit1=Repeat文ネスト中）
//                     （bit2=Lazy実行中）
//---------------------------------------------------------------------
int  JlsScriptState::isRemainNest(){
	int ret = 0;
	if ((int)m_listIfState.size() != 0) ret += 1;
	if ((int)m_listRepDepth.size() != 0) ret += 2;
	if ( isLazyArea() ) ret += 4;
	return ret;
}
//---------------------------------------------------------------------
// Returnコマンドによる終了を設定・確認
//---------------------------------------------------------------------
//--- returnコマンド発行状態の記憶 ---
void JlsScriptState::setCmdReturn(bool flag){
	m_flagReturn = flag;
}
//--- returnコマンド発行状態 ---
bool JlsScriptState::isCmdReturnExit(){
	return m_flagReturn || pGlobalState->isCmdExit();
}
//---------------------------------------------------------------------
// 変数展開しない区間中判定
//---------------------------------------------------------------------
bool JlsScriptState::isNeedRaw(CmdCat category){
	bool flagNeed = false;
	if ( isLazyArea() ){
		if ( category != CmdCat::LAZYF ){
			flagNeed = true;
		}
	}
	if ( isMemArea() ){
		if ( category != CmdCat::MEMF ){
			flagNeed = true;
		}
	}
	return flagNeed;
}
//---------------------------------------------------------------------
// 文字列全体をデコード必要か判別
//---------------------------------------------------------------------
bool JlsScriptState::isNeedFullDecode(CmdType cmdsel, CmdCat category){
	bool flagNeed = true;
	//--- 変数展開しない区間 ---
	if ( isNeedRaw(category) ){
		flagNeed = false;
	}
	//--- IF条件によるskip中に判定必要なケースかチェック ---
	if ( m_ifSkip ){
		if ( category != CmdCat::COND ){
			flagNeed = false;
		}else{
			switch( cmdsel ){
				case CmdType::If:
					flagNeed = false;
					break;
				case CmdType::ElsIf:
				case CmdType::Else:
					{
						int depth = (int) m_listIfState.size();
						if ( depth <= 1 ){
							flagNeed = true;
						}else{
							if ( m_listIfState[depth-1] == CondIfState::RUNNING ){
								flagNeed = true;
							}else{
								flagNeed = false;
							}
						}
					}
					break;
				default:
					break;
			}
		}
	}
	//--- Repeat 0回による不要区間 ---
	if ( m_repSkip ){
		if ( category != CmdCat::REP ){
			flagNeed = false;
		}
	}
	return flagNeed;
}
//---------------------------------------------------------------------
// 無効行の判定
//---------------------------------------------------------------------
bool JlsScriptState::isSkipCmd(){
	return m_ifSkip || m_repSkip || m_memSkip;
}
//---------------------------------------------------------------------
// 現在行の制御状態からのコマンド実行有効性
//   入力：  現在行のコマンド分類
//   返り値: 有効性（false=有効行、true=無効行）
//---------------------------------------------------------------------
bool JlsScriptState::isInvalidCmdLine(CmdCat category){
	bool flagInvalid = false;

	if ( m_ifSkip ){		// If条件 skip中
		if ( category != CmdCat::COND ){
			flagInvalid = true;
		}
	}
	if ( m_repSkip ){		// repeat0回 skip中
		if ( category != CmdCat::REP ){
			flagInvalid = true;
		}
	}
	if ( isLazyArea() ){	// LazyStart-End skip中
		if ( category != CmdCat::LAZYF ){
			flagInvalid = true;
		}
	}
	if ( isMemArea() ){		// Memory-End skip中
		if ( category != CmdCat::MEMF ){
			flagInvalid = true;
		}
	}
	return flagInvalid;
}
//---------------------------------------------------------------------
// Lazy処理の実行中判別（0=lazy以外の処理  1=lazy動作中）
//---------------------------------------------------------------------
bool JlsScriptState::isLazyExe(){
	bool flag;
	switch( m_typeCacheExe ){
		case CacheExeType::LazyS :
		case CacheExeType::LazyA :
		case CacheExeType::LazyE :
			flag = true;
			break;
		default :
			flag = false;
			break;
	}
	return flag;
}
//--- Lazy処理の実行中のLazy種類 ---
LazyType JlsScriptState::getLazyExeType(){
	LazyType typeLazy;
	switch( m_typeCacheExe ){
		case CacheExeType::LazyS :
			typeLazy = LazyType::LazyS;
			break;
		case CacheExeType::LazyA :
			typeLazy = LazyType::LazyA;
			break;
		case CacheExeType::LazyE :
			typeLazy = LazyType::LazyE;
			break;
		default :
			typeLazy = LazyType::None;
			break;
	}
	return typeLazy;
}
//---------------------------------------------------------------------
// 遅延実行内リピート中のキャッシュ種類
//---------------------------------------------------------------------
//--- 遅延実行内リピート中判定 ---
bool JlsScriptState::isRepeatExtType(){
	return ( getRepeatExtType() != CacheExeType::None )? true : false;
}
//--- 遅延実行内リピートの種類取得 ---
CacheExeType JlsScriptState::getRepeatExtType(){
	int depth = (int) m_listRepDepth.size();
	if (depth <= 0){
		return CacheExeType::None;
	}
	return m_listRepDepth[depth-1].exeType;
}
//--- 読み出し中の実行キャッシュ種類 ---
CacheExeType JlsScriptState::getCacheExeType(){
	return m_typeCacheExe;
}
//--- MemCall内容の実行中 ---
bool JlsScriptState::isMemExe(){
	bool flag;
	switch( m_typeCacheExe ){
		case CacheExeType::Mem :
			flag = true;
			break;
		default :
			flag = false;
			break;
	}
	return flag;
}
//---------------------------------------------------------------------
// LazyStart - EndLazy 期間内のlazy設定
//---------------------------------------------------------------------
void JlsScriptState::setLazyStartType(LazyType typeLazy){
	m_lazyStartType = typeLazy;
}
//---------------------------------------------------------------------
// LazyStart - EndLazy 期間内のlazy設定の取得
//---------------------------------------------------------------------
LazyType JlsScriptState::getLazyStartType(){
	return m_lazyStartType;
}
//---------------------------------------------------------------------
// LazyStart - EndLazy 期間内のlazy設定中判別
//---------------------------------------------------------------------
bool JlsScriptState::isLazyArea(){
	return (m_lazyStartType != LazyType::None)? true : false;
}
//---------------------------------------------------------------------
// LazyAuto設定
//---------------------------------------------------------------------
void JlsScriptState::setLazyAuto(bool flag){
	m_lazyAuto = flag;
}
//---------------------------------------------------------------------
// LazyAuto状態読み出し
//---------------------------------------------------------------------
bool JlsScriptState::isLazyAuto(){
	return m_lazyAuto;
}
//---------------------------------------------------------------------
// Memory - EndMemory 期間の設定
//---------------------------------------------------------------------
void JlsScriptState::startMemArea(const string& strName){
	m_memArea = true;
	m_memName = strName;
	m_memSkip = m_memDupe;
}
//---------------------------------------------------------------------
// Memory - EndMemory 期間の終了
//---------------------------------------------------------------------
void JlsScriptState::endMemArea(){
	m_memArea = false;
	m_memSkip = false;
}
//---------------------------------------------------------------------
// Memory - EndMemory 期間の設定中判定
//---------------------------------------------------------------------
bool JlsScriptState::isMemArea(){
	return m_memArea;
}
//---------------------------------------------------------------------
// Memory - EndMemory 期間中の設定識別子を取得
//---------------------------------------------------------------------
string JlsScriptState::getMemName(){
	return m_memName;
}
//---------------------------------------------------------------------
// MemOnceコマンドによる重複状態の設定
//---------------------------------------------------------------------
void JlsScriptState::setMemDupe(bool flag){
	m_memDupe = flag;
}
