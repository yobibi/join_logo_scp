//
// JL�X�N���v�g�����Ԃ̕ێ��iCall�̐������쐬�j
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptState.hpp"
#include "JlsScrGlobal.hpp"

///////////////////////////////////////////////////////////////////////
//
// �X�N���v�g����
//
///////////////////////////////////////////////////////////////////////
JlsScriptState::JlsScriptState(JlsScrGlobal* globalState){
	this->pGlobalState = globalState;
	clear();
}

//---------------------------------------------------------------------
// ������
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
// If����
//=====================================================================

//---------------------------------------------------------------------
// If���ݒ�
//   ����:   flag_cond=������
//   �Ԃ�l: �G���[�ԍ��i0=����I���A1=�G���[�j
//---------------------------------------------------------------------
int JlsScriptState::ifBegin(bool flag_cond){
	CondIfState stat;
	if (m_ifSkip){
		m_ifSkip = true;
		stat = CondIfState::FINISHED;		// �����F�I����
	}
	else if (flag_cond == false){
		m_ifSkip = true;
		stat = CondIfState::PREPARE;		// �����F�����s
	}
	else{
		m_ifSkip = false;
		stat = CondIfState::RUNNING;		// �����F���s
	}
	m_listIfState.push_back(stat);			// ���X�g�ɕۑ�
	return 0;
}

//---------------------------------------------------------------------
// EndIf���ݒ�
//   �Ԃ�l: �G���[�ԍ��i0=����I���A1=�G���[�j
//---------------------------------------------------------------------
int JlsScriptState::ifEnd(){
	int depth = (int) m_listIfState.size();
	if (depth <= 0){
		return 1;
	}
	m_listIfState.pop_back();				// ���X�g����폜
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
// ElsIf���ݒ�
//   ����:   flag_cond=������
//   �Ԃ�l: �G���[�ԍ��i0=����I���A1=�G���[�j
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
// Repeat����
//=====================================================================

//---------------------------------------------------------------------
// Repeat���ݒ�
//   ����:   �J��Ԃ���
//   �Ԃ�l: �G���[�ԍ��i0=����I���A1=�G���[�j
//---------------------------------------------------------------------
int JlsScriptState::repeatBegin(int num){
	//--- �ŏ��̃��s�[�g���� ---
	int depth = (int) m_listRepDepth.size();
	if (depth == 0){
		m_repSkip = false;					// ��΂��Ȃ�
		m_repLineReadCache = -1;			// �L���b�V���ǂݏo������
	}
	//--- ���s�[�g�R�}���h�L���b�V���p ---
	string strCmdRepeat = "Repeat " + to_string(num);
	//--- ���s�[�g����ݒ� ---
	if (num <= 0 || m_repSkip){				// �ŏ�������s�Ȃ��̏ꍇ
		num = -1;							// ���s�Ȃ����̉񐔂�-1�ɂ���
		m_repSkip = true;					// �R�}���h���΂�
	}
	//--- �ݒ�ۑ� ---
	RepDepthHold holdval;
	holdval.countLoop = num;				// �J��Ԃ��񐔂�ݒ�
	//--- �x�����s�����̊g�� ---
	int errval = 0;
	if ( isLazyExe() || isMemExe() ){
		errval = repeatBeginExtend(holdval, strCmdRepeat);
	}else{
		errval = repeatBeginNormal(holdval, strCmdRepeat);
	}
	if ( errval == 0 ){
		m_listRepDepth.push_back(holdval);		// ���X�g�ɕۑ�
	}
	return errval;
}
//--- �ʏ��Repeat�ݒ� ---
int JlsScriptState::repeatBeginNormal(RepDepthHold& holdval, const string& strCmdRepeat){
	//--- �L���b�V���s���`�F�b�N ---
	int size_line = (int) m_listRepCmdCache.size();
	if (size_line >= SIZE_REPLINE){			// �ő�s���`�F�b�N
		return 1;
	}
	//--- �L���b�V�����ĂȂ��ꍇ�͌��݃R�}���h���L���b�V�� ---
	if (size_line == 0){
		m_listRepCmdCache.push_back(strCmdRepeat);
		size_line ++;
	}
	int line_start = m_repLineReadCache;
	if (line_start < 0) line_start = size_line;
	//--- �ݒ�ۑ� ---
	holdval.lineStart = line_start;			// �J�n�s��ݒ�
	//--- �g�������ݒ� ---
	holdval.extLineEnd = 0;
	holdval.extLineRet = 0;
	holdval.exeType = CacheExeType::None;
	return 0;
}
//--- �x�����s����Repeat�ݒ� ---
int JlsScriptState::repeatBeginExtend(RepDepthHold& holdval, const string& strCmdRepeat){
	//--- �L���b�V���s���`�F�b�N ---
	int size_line = (int)m_listRepExtCache.size();
	if (size_line >= SIZE_REPLINE){			// �ő�s���`�F�b�N
		return 1;
	}
	//--- �g�����s�[�g�L���b�V���ɃR�}���h�s���i�[ ---
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
// EndRepeat���ݒ�
//   �Ԃ�l: �G���[�ԍ��i0=����I���A1=�G���[�j
//---------------------------------------------------------------------
int JlsScriptState::repeatEnd(){
	int depth = (int) m_listRepDepth.size();
	if (depth <= 0){
		return 1;
	}
	if (m_listRepDepth[depth-1].countLoop > 0){		// �J�E���g�_�E��
		m_listRepDepth[depth-1].countLoop --;
	}
	bool extMode = isRepeatExtType();
	if (m_listRepDepth[depth-1].countLoop > 0){		// �J��Ԃ��p���̏ꍇ
		if ( extMode ){
			m_repLineExtRCache = m_listRepDepth[depth-1].lineStart;	// �ǂݏo���s�ݒ�
		}else{
			m_repLineReadCache = m_listRepDepth[depth-1].lineStart;	// �ǂݏo���s�ݒ�
		}
	}
	else{											// �J��Ԃ��I���̏ꍇ
		if ( extMode ){								// �x�����s���[�h�p
			repeatEndExtend(m_listRepDepth[depth-1]);
		}
		m_listRepDepth.pop_back();					// ���X�g����폜
		depth --;
		//--- �S���s�[�g�I�����̏��� ---
		if (depth == 0){
			m_listRepCmdCache.clear();				// �L���b�V��������̏���
			m_listRepExtCache.clear();				// �x�����s�p�g���L���b�V��
			m_repSkip = false;
		//--- ��΂���Ԃ��X�V ---
		}else if (m_repSkip){
			if (m_listRepDepth[depth-1].countLoop >= 0){
				m_repSkip = false;
			}
		}
	}
	return 0;
}
//--- �x�����s����Repeat�ݒ� ---
void JlsScriptState::repeatEndExtend(RepDepthHold& holdval){
	int nNext = m_repLineExtRCache;
	int nEnd  = holdval.extLineEnd;
	//--- �����s������queue�ɖ߂� ---
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
	//--- ���s�[�gCache�̕s�v�ɂȂ镔�����폜 ---
	if ( nEnd + 1 == (int)m_listRepExtCache.size() ){
		for(int i = holdval.lineStart - 1; i <= nEnd; i++){
			if ( m_listRepExtCache.empty() == false ) m_listRepExtCache.pop_back();
		}
	}
	//--- �L���b�V�����s�s��Repeat�O�ɖ߂� ---
	m_repLineExtRCache = holdval.extLineRet;
}
//--- queue�f�[�^��x�����s���̃��s�[�g�p�L���b�V���Ɉړ� ---
int JlsScriptState::repeatExtMoveQueue(vector <string>& listCache, queue <string>& queSrc){
	if ( listCache.size() + queSrc.size() >= SIZE_MEMLINE){		// �ő�s���`�F�b�N
		pGlobalState->addMsgError("error: memory cache for repeat overflow\n");
		return 1;
	}
	while( queSrc.empty() == false ){
		listCache.push_back( queSrc.front() );
		queSrc.pop();
	}
	return 0;
}
//--- ���s�[�g�p�L���b�V������queue�Ƀf�[�^��߂� ---
void JlsScriptState::repeatExtBackQueue(queue <string>& queDst, vector <string>& listCache, int nFrom, int nTo){
	if ( queDst.size() + nTo - nFrom + 1 >= SIZE_MEMLINE ){		// �ő�s���`�F�b�N
		pGlobalState->addMsgError("error: memory cache for repeat overflow\n");
		return;
	}
	queue <string> q;
	queDst.swap(q);			// queDst�N���A + q�Ɉꎞ�ێ�
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
// �x�����s�ۊǗ̈�ւ̃A�N�Z�X
//=====================================================================

//---------------------------------------------------------------------
// lazy�����ŕۊǂ��ꂽ�R�}���h�����s�Ɉڂ�
// ���́F
//   typeLazy  : LazyS, LazyA, LazyE, FULL
//   strBuf   : ���ݍs�̕�����i�ۊǃR�}���h���Ɏ��s���鎞�ɃL���b�V���ֈڂ��j
// ���o�́F
//   state    : lazy���s�����ǉ�
// �o�́F
//   �Ԃ�l   �F���ݍs�̃R�}���h���s�L���i���s�L���b�V���Ɉڂ������͎��s���Ȃ��j
//---------------------------------------------------------------------
bool JlsScriptState::setLazyExe(LazyType typeLazy, const string& strBuf){
	//--- ���s���� ---
	bool enableExe = true;
	if ( pGlobalState->isLazyExist(LazyType::FULL) ){		// lazy�R�}���h�g�p���т�����Ίm�F����
		queue <string> queStrS;
		queue <string> queStrA;
		queue <string> queStrE;
		//--- Lazy�ۊǍs�̎��o�� ---
		switch( typeLazy ){
			case LazyType::LazyS:
				enableExe = setLazyExeProcS(queStrS, strBuf);	// �i���o���Olazy + ���ݍs�j
				break;
			case LazyType::LazyA:
				//--- ���o���Olazy���c���Ă������Ɏ��s ---
				enableExe = setLazyExeProcS(queStrS, strBuf);	// �i���o���Olazy + ���ݍs�j
				pGlobalState->popListByLazy(queStrA, LazyType::LazyA);	// �iauto����lazy�j
				break;
			case LazyType::LazyE:
				pGlobalState->popListByLazy(queStrE, LazyType::LazyE);	// �i�I����lazy�j
				break;
			case LazyType::FULL:
				//--- lazy_e�܂ł̑S����Flush���� ---
				enableExe = setLazyExeProcS(queStrS, strBuf);	// �i���o���Olazy + ���ݍs�j
				pGlobalState->popListByLazy(queStrA, LazyType::LazyA);	// �iauto����lazy�j
				pGlobalState->popListByLazy(queStrE, LazyType::LazyE);	// �i�I����lazy�j
				break;
			default:
				break;
		}
		if ( queStrS.empty() == false ){
			addCacheExeLazy(queStrS, LazyType::LazyS);	// Lazy���s�L���b�V���Ɉړ�
		}
		if ( queStrA.empty() == false ){
			addCacheExeLazy(queStrA, LazyType::LazyA);	// Lazy���s�L���b�V���Ɉړ�
		}
		if ( queStrE.empty() == false ){
			addCacheExeLazy(queStrE, LazyType::LazyE);	// Lazy���s�L���b�V���Ɉړ�
		}
	}
	return enableExe;
}
//---------------------------------------------------------------------
// �i�T�u���[�`���j�ۊǂ��ꂽ�f�[�^�̌�Ɍ��ݍs�����s���邽�߂̏���
// �Ԃ�l   �F���ݍs�̃R�}���h���s�L���i���ݍs�����X�g���ɓ��ꂽ����s���Ȃ��j
//---------------------------------------------------------------------
bool JlsScriptState::setLazyExeProcS(queue <string>& queStr, const string& strBuf){
	bool enableExe = true;
	//--- ���o���Olazy����荞�݁A���������猻�ݍs�����̌�ɒǉ� ---
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
// lazy�����ŕۊǂ��ꂽ�R�}���h��S�����s�Ɉڂ�
// ���o�́F
//   state    : lazy���s�����ǉ�
//---------------------------------------------------------------------
void JlsScriptState::setLazyFlush(){
	setLazyExe(LazyType::FULL, "");
}
//---------------------------------------------------------------------
// memory�����ŕۊǂ��ꂽ�R�}���h�����s�Ɉڂ�(MemCall)
// ���́F
//   strName  : �ۊǗ̈�̎��ʎq
// ���o�́F
//   state    : MemCall���s�����ǉ�
// �o�́F
//   �Ԃ�l   �Ftrue=�����Afalse=���s
//---------------------------------------------------------------------
bool JlsScriptState::setMemCall(const string& strName){
	queue <string> queStr;
	bool enable_exe = pGlobalState->getListByName(queStr, strName);
	addCacheExeMem(queStr);							// Mem���s�L���b�V���Ɉړ�
	return enable_exe;
}
//--- global state���ŏ��� ---
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
// �x�����s�L���[����
//=====================================================================

//---------------------------------------------------------------------
// ���s�L���b�V�����P�s�ǂݏo��
//---------------------------------------------------------------------
//--- Lazy/Mem�R�}���h���s�L���b�V�� ---
bool JlsScriptState::popCacheExeLazyMem(string& strBuf){
	//--- �x�����s�p���s�[�gCache�m�F ---
	CacheExeType typeRepeat = getRepeatExtType();

	//--- read��̑I�� ---
	bool flagExt = false;
	CacheExeType typeRead = CacheExeType::None;

	if ( m_cacheExeLazyS.empty() == false ){	// LazyS ���s�L���b�V������
		flagExt   = false;
		typeRead  = CacheExeType::LazyS;
	}
	else if ( typeRepeat == CacheExeType::LazyS ){	// LazyS Repeat�L���b�V������
		flagExt   = true;
		typeRead  = CacheExeType::LazyS;
	}
	else if ( m_cacheExeLazyA.empty() == false ){	// LazyA ���s�L���b�V������
		flagExt   = false;
		typeRead  = CacheExeType::LazyA;
	}
	else if ( typeRepeat == CacheExeType::LazyA ){	// LazyA Repeat�L���b�V������
		flagExt   = true;
		typeRead  = CacheExeType::LazyA;
	}
	else if ( m_cacheExeLazyE.empty() == false ){	// LazyE ���s�L���b�V������
		flagExt   = false;
		typeRead  = CacheExeType::LazyE;
	}
	else if ( typeRepeat == CacheExeType::LazyE ){	// LazyE Repeat�L���b�V������
		flagExt   = true;
		typeRead  = CacheExeType::LazyE;
	}
	else if ( m_cacheExeMem.empty() == false ){	// Mem ���s�L���b�V������
		flagExt   = false;
		typeRead  = CacheExeType::Mem;
	}
	else if ( typeRepeat == CacheExeType::Mem ){	// Mem Repeat�L���b�V������
		flagExt   = true;
		typeRead  = CacheExeType::Mem;
	}
	//--- Cache�ǂݏo�� ---
	bool flagRead = false;
	if ( typeRead != CacheExeType::None ){
		if ( flagExt ){
			flagRead = readRepeatExtCache(strBuf);	// �x�����s�pRepeat�L���b�V������ǂݏo��
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
	m_typeCacheExe = typeRead;		// �ǂݏo����L���b�V���ݒ�
	return flagRead;
}
//---------------------------------------------------------------------
// �x�����s�����s�[�g���̎��̃R�}���h�擾
//   �Ԃ�l: �i�[���s�ifalse=�i�[�s�v�Atrue=�i�[�ς݁j
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
// ���s�L���b�V����ݒ�
//---------------------------------------------------------------------
//--- Lazy���s�ŌĂяo���ꂽ�s��ݒ� ---
void JlsScriptState::addCacheExeLazy(queue <string>& queStr, LazyType typeLazy){
	bool flagHead = false;				// �o�b�t�@�ɒǉ��}��
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
//--- MemCall�ŌĂяo���ꂽ�s��ݒ� ---
void JlsScriptState::addCacheExeMem(queue <string>& queStr){
	if ( isLazyExe() ){
		//--- Lazy���s����MemCall�͎c���Lazy���s����ɑ����s ---
		bool flagHead = true;			// ���o�b�t�@����ɑ}��
		addCacheExeCommon(queStr, m_typeCacheExe, flagHead);
	}
	else{
		//--- �ʏ��Lazy�������D��x���ႢMemCall���s�҂��o�b�t�@�ɑ}�� ---
		bool flagHead = true;			// ���o�b�t�@����ɑ}��
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
// queue����
//---------------------------------------------------------------------
//--- queue�̐擪�f�[�^�����o�� ---
bool JlsScriptState::popQueue(string& strBuf, queue <string>& queSrc){
	if ( queSrc.empty() ) return false;		// queue����Ȃ�false��Ԃ�
	strBuf = queSrc.front();
	queSrc.pop();
	return true;
}
//--- queue�ɕʂ�queue��ǉ� ---
void JlsScriptState::addQueue(queue <string>& queDst, queue <string>& queSrc, bool flagHead){
	queue <string> q;
	//--- �L���b�V���ێ��ő�s���𒴂��Ȃ����m�F ---
	if ( queDst.size() + queSrc.size() > SIZE_MEMLINE){
		pGlobalState->addMsgError("error: memory cache overflow\n");
		queDst = q;			// �L���b�V���N���A
		return;
	}
	if ( flagHead ){		// ��O��Src��ǉ�����ꍇ
		q      = queDst;
		queDst = queSrc;
	}
	else{					// ���Src��ǉ�����ꍇ
		q = queSrc;
	}
	while( q.empty() == false ){
		queDst.push( q.front() );
		q.pop();
	}
}


//=====================================================================
// �L���b�V���f�[�^�ǂݏo��
//=====================================================================

//---------------------------------------------------------------------
// Cache����̓ǂݏo���iRepeat�p�j
//   �Ԃ�l: �ǂݏo�����ʁifalse=�ǂݏo���Ȃ��Atrue=cache����̓ǂݏo���j
//   strBufOrg: �ǂݏo���ꂽ������
//---------------------------------------------------------------------
bool JlsScriptState::readCmdCache(string& strBufOrg){
	//--- �ǂݏo���\���`�F�b�N ---
	if ( (int)m_listRepDepth.size() <= 0 ){		// Repeat���ł͂Ȃ�
		return false;
	}
	if ( m_repLineReadCache >= (int)m_listRepCmdCache.size() ){
		m_repLineReadCache = -1;
	}
	if (m_repLineReadCache < 0) return false;

	//--- �ǂݏo�����s ---
	strBufOrg = m_listRepCmdCache[m_repLineReadCache];
	m_repLineReadCache ++;
	return true;
}
//---------------------------------------------------------------------
// Cache�ɕ�����i�[�iRepeat�p�j
//   ����:   strBufOrg=�i�[������
//   �Ԃ�l: �i�[���s�ifalse=�i�[�s�v�Atrue=�i�[�ς݁j
//---------------------------------------------------------------------
bool JlsScriptState::addCmdCache(string& strBufOrg){
	if ( (int)m_listRepDepth.size() <= 0 ){		// Repeat���ł͂Ȃ�
		return false;
	}
	//--- �L���b�V���s���`�F�b�N ---
	int size_line = (int) m_listRepCmdCache.size();
	if (size_line >= SIZE_REPLINE){			// �ő�s���`�F�b�N
		return false;
	}
	m_listRepCmdCache.push_back(strBufOrg);
	return true;
}
//---------------------------------------------------------------------
// Lazy/Mem���s�s��ǂݏo��
// �o�́F
//   �Ԃ�l   �F������擾���ʁi0=�擾�Ȃ�  1=�擾����j
//   strBufOrg : �擾������
//---------------------------------------------------------------------
bool JlsScriptState::readLazyMemNext(string& strBufOrg){
	//--- Lazy/Mem���s�s������ΐ擪��ǂݏo�� ---
	if ( popCacheExeLazyMem(strBufOrg) ){
		return true;
	}
	//--- Lazy/Mem���s�s���Ȃ����false��Ԃ� ---
	return false;
}


//=====================================================================
// ��Ԏ擾
//=====================================================================

//---------------------------------------------------------------------
// �l�X�g��Ԃ��c���Ă��邩�m�F
//   �Ԃ�l: �G���[�ԍ��i0=����I���Abit0=If���l�X�g���Abit1=Repeat���l�X�g���j
//                     �ibit2=Lazy���s���j
//---------------------------------------------------------------------
int  JlsScriptState::isRemainNest(){
	int ret = 0;
	if ((int)m_listIfState.size() != 0) ret += 1;
	if ((int)m_listRepDepth.size() != 0) ret += 2;
	if ( isLazyArea() ) ret += 4;
	return ret;
}
//---------------------------------------------------------------------
// Return�R�}���h�ɂ��I����ݒ�E�m�F
//---------------------------------------------------------------------
//--- return�R�}���h���s��Ԃ̋L�� ---
void JlsScriptState::setCmdReturn(bool flag){
	m_flagReturn = flag;
}
//--- return�R�}���h���s��� ---
bool JlsScriptState::isCmdReturnExit(){
	return m_flagReturn || pGlobalState->isCmdExit();
}
//---------------------------------------------------------------------
// �ϐ��W�J���Ȃ���Ԓ�����
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
// ������S�̂��f�R�[�h�K�v������
//---------------------------------------------------------------------
bool JlsScriptState::isNeedFullDecode(CmdType cmdsel, CmdCat category){
	bool flagNeed = true;
	//--- �ϐ��W�J���Ȃ���� ---
	if ( isNeedRaw(category) ){
		flagNeed = false;
	}
	//--- IF�����ɂ��skip���ɔ���K�v�ȃP�[�X���`�F�b�N ---
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
	//--- Repeat 0��ɂ��s�v��� ---
	if ( m_repSkip ){
		if ( category != CmdCat::REP ){
			flagNeed = false;
		}
	}
	return flagNeed;
}
//---------------------------------------------------------------------
// �����s�̔���
//---------------------------------------------------------------------
bool JlsScriptState::isSkipCmd(){
	return m_ifSkip || m_repSkip || m_memSkip;
}
//---------------------------------------------------------------------
// ���ݍs�̐����Ԃ���̃R�}���h���s�L����
//   ���́F  ���ݍs�̃R�}���h����
//   �Ԃ�l: �L�����ifalse=�L���s�Atrue=�����s�j
//---------------------------------------------------------------------
bool JlsScriptState::isInvalidCmdLine(CmdCat category){
	bool flagInvalid = false;

	if ( m_ifSkip ){		// If���� skip��
		if ( category != CmdCat::COND ){
			flagInvalid = true;
		}
	}
	if ( m_repSkip ){		// repeat0�� skip��
		if ( category != CmdCat::REP ){
			flagInvalid = true;
		}
	}
	if ( isLazyArea() ){	// LazyStart-End skip��
		if ( category != CmdCat::LAZYF ){
			flagInvalid = true;
		}
	}
	if ( isMemArea() ){		// Memory-End skip��
		if ( category != CmdCat::MEMF ){
			flagInvalid = true;
		}
	}
	return flagInvalid;
}
//---------------------------------------------------------------------
// Lazy�����̎��s�����ʁi0=lazy�ȊO�̏���  1=lazy���쒆�j
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
//--- Lazy�����̎��s����Lazy��� ---
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
// �x�����s�����s�[�g���̃L���b�V�����
//---------------------------------------------------------------------
//--- �x�����s�����s�[�g������ ---
bool JlsScriptState::isRepeatExtType(){
	return ( getRepeatExtType() != CacheExeType::None )? true : false;
}
//--- �x�����s�����s�[�g�̎�ގ擾 ---
CacheExeType JlsScriptState::getRepeatExtType(){
	int depth = (int) m_listRepDepth.size();
	if (depth <= 0){
		return CacheExeType::None;
	}
	return m_listRepDepth[depth-1].exeType;
}
//--- �ǂݏo�����̎��s�L���b�V����� ---
CacheExeType JlsScriptState::getCacheExeType(){
	return m_typeCacheExe;
}
//--- MemCall���e�̎��s�� ---
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
// LazyStart - EndLazy ���ԓ���lazy�ݒ�
//---------------------------------------------------------------------
void JlsScriptState::setLazyStartType(LazyType typeLazy){
	m_lazyStartType = typeLazy;
}
//---------------------------------------------------------------------
// LazyStart - EndLazy ���ԓ���lazy�ݒ�̎擾
//---------------------------------------------------------------------
LazyType JlsScriptState::getLazyStartType(){
	return m_lazyStartType;
}
//---------------------------------------------------------------------
// LazyStart - EndLazy ���ԓ���lazy�ݒ蒆����
//---------------------------------------------------------------------
bool JlsScriptState::isLazyArea(){
	return (m_lazyStartType != LazyType::None)? true : false;
}
//---------------------------------------------------------------------
// LazyAuto�ݒ�
//---------------------------------------------------------------------
void JlsScriptState::setLazyAuto(bool flag){
	m_lazyAuto = flag;
}
//---------------------------------------------------------------------
// LazyAuto��ԓǂݏo��
//---------------------------------------------------------------------
bool JlsScriptState::isLazyAuto(){
	return m_lazyAuto;
}
//---------------------------------------------------------------------
// Memory - EndMemory ���Ԃ̐ݒ�
//---------------------------------------------------------------------
void JlsScriptState::startMemArea(const string& strName){
	m_memArea = true;
	m_memName = strName;
	m_memSkip = m_memDupe;
}
//---------------------------------------------------------------------
// Memory - EndMemory ���Ԃ̏I��
//---------------------------------------------------------------------
void JlsScriptState::endMemArea(){
	m_memArea = false;
	m_memSkip = false;
}
//---------------------------------------------------------------------
// Memory - EndMemory ���Ԃ̐ݒ蒆����
//---------------------------------------------------------------------
bool JlsScriptState::isMemArea(){
	return m_memArea;
}
//---------------------------------------------------------------------
// Memory - EndMemory ���Ԓ��̐ݒ莯�ʎq���擾
//---------------------------------------------------------------------
string JlsScriptState::getMemName(){
	return m_memName;
}
//---------------------------------------------------------------------
// MemOnce�R�}���h�ɂ��d����Ԃ̐ݒ�
//---------------------------------------------------------------------
void JlsScriptState::setMemDupe(bool flag){
	m_memDupe = flag;
}
