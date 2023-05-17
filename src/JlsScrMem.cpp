//
// �x�����s�R�}���h�̕ۊ�
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrMem.hpp"

///////////////////////////////////////////////////////////////////////
//
// �x�����s�ۊǗp�̎��ʎq�ێ��N���X
//
///////////////////////////////////////////////////////////////////////
JlsScrMemArg::JlsScrMemArg(){
	clearArg();
}

//---------------------------------------------------------------------
// �����ݒ��������
//---------------------------------------------------------------------
void JlsScrMemArg::clearArg(){
	m_flagDummy  = false;
	m_flagSpecial = false;
	m_listName.clear();
	m_listName.push_back("");		// ���X�g1�ڂ͏����ݒ�
}

//---------------------------------------------------------------------
// �ݒ�F���ʎq������ɂ��ۊǗp���ʎq���
//  ToBase �ŕۊǂ��鎯�ʎq�̖��O��ݒ�
//  ToExt  ��Lazy_S/A/E �R��ނ��ꂼ��̏�Ԃ�ۊǂ��邽�߂̎��ʎq��ݒ�
//---------------------------------------------------------------------
void JlsScrMemArg::setNameByStr(const string strName){
	//--- ������ ---
	clearArg();
	bool base = true;		// �P�̂̕�����ݒ�
	string strRegName = strName;
	//--- ���ꎯ�ʎq�m�F ---
	MemSpecialID idName;
	if ( findSpecialName(idName, strName) ){	// ���ꎯ�ʎq
		m_flagSpecial = true;
		switch( idName ){
			case MemSpecialID::DUMMY:
				m_flagDummy = true;
				base = false;
				break;
			case MemSpecialID::LAZY_FULL:
				setMapNameToExt("");			// Lazy�{�̂ɂ�Lazy�p�g�����ʎq�����Ȃ�
				base = false;
				break;
			case MemSpecialID::NoData:
				base = false;
				break;
			default:
				strRegName = getStringSpecialID(idName);	// ����͑Ή����鎯�ʎq�ɂ���
				break;
		}
	}
	else{
		setMapNameToExt(strRegName);				// �ʏ펯�ʎq�ɕۊǂ���Lazy�p�g�����ʎq�ݒ�
	}
	//--- ������ݒ� ---
	if ( base ){
		setMapNameToBase(strRegName);
	}
}
//---------------------------------------------------------------------
// �ݒ�FLazy��ނɂ��ۊǗp���ʎq���
//---------------------------------------------------------------------
void JlsScrMemArg::setNameByLazy(LazyType typeLazy){
	//--- ������ ---
	clearArg();
	string strName = "";
	//--- �ݒ� ---
	switch( typeLazy ){
		case LazyType::LazyS:
			strName = getStringSpecialID(MemSpecialID::LAZY_S);
			break;
		case LazyType::LazyA:
			strName = getStringSpecialID(MemSpecialID::LAZY_A);
			break;
		case LazyType::LazyE:
			strName = getStringSpecialID(MemSpecialID::LAZY_E);
			break;
		case LazyType::FULL:
			strName = getStringSpecialID(MemSpecialID::LAZY_FULL);
			break;
		default:
			break;
	}
	if ( strName.empty() == false ){
		setNameByStr(strName);
	}
}
//---------------------------------------------------------------------
// �擾�F�ۊǗp���ʎq�̑���
//---------------------------------------------------------------------
bool JlsScrMemArg::isExistBaseName(){
	return !( m_listName[0].empty() );
}
//---------------------------------------------------------------------
// �擾�FLazy�p�ۊǗp���ʎq�̑���
//---------------------------------------------------------------------
bool JlsScrMemArg::isExistExtName(){
	int sizeName = (int)m_listName.size();
	if ( sizeName > 1 && sizeName <= SIZE_MEM_SPECIAL_ID ){
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
// �擾�FDUMMY�����񔻒�
//---------------------------------------------------------------------
bool JlsScrMemArg::isNameDummy(){
	return m_flagDummy;
}
//---------------------------------------------------------------------
// �擾�F���ꕶ���񔻒�
//---------------------------------------------------------------------
bool JlsScrMemArg::isNameSpecial(){
	return m_flagSpecial;
}
//---------------------------------------------------------------------
// �擾�F�ۊǗp���ʎq������
//---------------------------------------------------------------------
void JlsScrMemArg::getBaseName(string& strName){
	strName = m_listName[0];
}
//---------------------------------------------------------------------
// �擾�FLazy�p���܂߂��ۊǗp���ʎq�����񃊃X�g
//---------------------------------------------------------------------
void JlsScrMemArg::getNameList(vector <string>& listName){
	listName = m_listName;
}

//---------------------------------------------------------------------
// ���������F�ʏ�̕ۊǗp���ʎq��ݒ�
//---------------------------------------------------------------------
void JlsScrMemArg::setMapNameToBase(const string strName){
	m_listName[0] = strName;
}
//---------------------------------------------------------------------
// ���������FLazy�p�̊g���ۊǎ��ʎq�Z�b�g��ݒ�
//---------------------------------------------------------------------
void JlsScrMemArg::setMapNameToExt(const string strName){
	//--- ���ʎqstrName�ɑΉ�����Lazy�p�ۊǕ������ݒ�iFULL�w�莞�Ɏc��Lazy�̎�ޕ��j ---
	for(int i=0; i < SIZE_MEM_SPECIAL_ID; i++){
		MemSpecialID id = (MemSpecialID) i;
		switch( id ){
			case MemSpecialID::LAZY_S:
			case MemSpecialID::LAZY_A:
			case MemSpecialID::LAZY_E:
				{
					string str_var = getStringSpecialID(id);	// Lazy�p�ۊǎ��ʎq
					if ( !strName.empty() ){				// �ʏ펯�ʎq��Lazy�p�ۊǎ��ʎq
						str_var = strName + ScrMemStrLazy + str_var;
					}
					m_listName.push_back(str_var);
				}
				break;
			default:
				break;
		}
	}
}
//---------------------------------------------------------------------
// ���������F���ꎯ�ʎq�̊m�F�E�擾
//---------------------------------------------------------------------
bool JlsScrMemArg::findSpecialName(MemSpecialID& idName, const string& strName){
	bool result = false;
	for(int i=0; i < SIZE_MEM_SPECIAL_ID; i++){
		if ( strName == MemSpecialData[i].str ){
			idName = MemSpecialData[i].id;		// ���ʎq��(strName)�ɑΉ�����ԍ����擾
			result = true;
			break;
		}
	}
	return result;
}
//---------------------------------------------------------------------
// ���������F���ꎯ�ʎq�ɑΉ����镶������擾
//---------------------------------------------------------------------
string JlsScrMemArg::getStringSpecialID(MemSpecialID idName){
	int num = static_cast<int>(idName);
	if ( num >= 0 && num < SIZE_MEM_SPECIAL_ID ){
		int n2 = static_cast<int>(MemSpecialData[num].id);
		return MemSpecialData[n2].str;
	}
	return "";
}


///////////////////////////////////////////////////////////////////////
//
// �X�N���v�g�f�[�^�ۊǎ��s�{�̃N���X
//
///////////////////////////////////////////////////////////////////////
JlsScrMem::JlsScrMem(){
	m_orderHold = orderInitial;
}

//---------------------------------------------------------------------
// Lazy�ۊǂ���Ă��邩�m�F
//---------------------------------------------------------------------
bool JlsScrMem::isLazyExist(LazyType typeLazy){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- ���݊m�F ---
	vector <string> slist;
	marg.getNameList(slist);
	bool exist = false;
	for(int i=0; i < (int)slist.size(); i++){
		if ( memIsExist(slist[i]) ){
			exist = true;
		}
	}
	return exist;
}

//=====================================================================
// �R�}���h���s
// �Ԃ�l   �F���s�L��
//=====================================================================

//---------------------------------------------------------------------
// �i�[���̎��s����
//---------------------------------------------------------------------
//--- �i�[���̎��s���ʂ�ݒ� ---
void JlsScrMem::setOrderForPush(int order){
	m_orderHold = order;
}
//--- �i�[���̎��s���ʂ�W���l�Őݒ� ---
void JlsScrMem::resetOrderForPush(){
	m_orderHold = orderInitial;
}
//--- ���s���ʂ̕W���l��ǂݏo�� ---
int JlsScrMem::getOrderForPush(){
	return m_orderHold;
}
//---------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------
//--- �����ݒ� ---
bool JlsScrMem::setDefArg(vector<string>& argDef){
	string strName = argDef[0];
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���ꕶ���񏈗� ---
	if ( marg.isNameSpecial() ){
		if ( argDef.size() > 1 ) return false;		// �����̓_��
		return true;
	}
	//--- �ʏ�ϐ����� ---
	string strNameBase;
	marg.getBaseName(strNameBase);
	if ( strNameBase.empty() ){		// �ʏ�ϐ����Ƃ��Đݒ�ł��Ȃ����̂̓_��
		return false;
	}
	if ( memIsNameExistArg(strNameBase) ){
		if ( m_mapArg[strName].size() >= 2 ){	// �������ݎ��̓G���[�ɂ���
			return false;
		}
		return ( argDef == m_mapArg[strName] );		// �������͓�����true
	}
	m_mapArg[strNameBase] = argDef;
	return true;
}
//--- �����擾 ---
bool JlsScrMem::getDefArg(vector<string>& argDef, const string& strName){
	argDef.clear();
	argDef.push_back(strName);
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���ꕶ���񏈗� ---
	if ( marg.isNameSpecial() ){
		return true;
	}
	//--- �ʏ�ϐ����� ---
	string strNameBase;
	marg.getBaseName(strNameBase);
	if ( strNameBase.empty() ){
		return false;
	}
	if ( memIsNameExistArg(strNameBase) ){
		argDef = m_mapArg[strNameBase];
		return true;
	}
	return false;	// �Ȃ���Ύ��s
}
//---------------------------------------------------------------------
// MemSet�ɂ�関�g�p�t���O��ݒ�
//---------------------------------------------------------------------
void JlsScrMem::setUnusedFlag(const string& strName){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���g�p�ɒǉ� ---
	setUnused(marg);
}
//---------------------------------------------------------------------
// �P�s��������i�[�i���ʎq�Ŋi�[����w��j
//---------------------------------------------------------------------
bool JlsScrMem::pushStrByName(const string& strName, const string& strBuf){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���s ---
	return exeCmdPushStr(marg, strBuf, m_orderHold);
}
//---------------------------------------------------------------------
// �P�s��������i�[�iLazy��ނŊi�[����w��j
//---------------------------------------------------------------------
bool JlsScrMem::pushStrByLazy(LazyType typeLazy, const string& strBuf){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- ���s ---
	return exeCmdPushStr(marg, strBuf, m_orderHold);
}
//---------------------------------------------------------------------
// �ۊǕ����񃊃X�g���擾�i���ʎq�Ŋi�[�����w��j
//---------------------------------------------------------------------
bool JlsScrMem::getListByName(queue <string>& queStr, const string& strName){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���g�p��Ԃ����� ---
	clearUnused(marg);
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// �L���[�ɒǉ�
	flags.move = false;		// ���f�[�^�͎c��
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ����񃊃X�g�����o���i���ʎq�Ŋi�[�����w��j
//---------------------------------------------------------------------
bool JlsScrMem::popListByName(queue <string>& queStr, const string& strName){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���g�p��Ԃ����� ---
	clearUnused(marg);
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// �L���[�ɒǉ�
	flags.move = true;		// ���f�[�^������
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ����񃊃X�g���擾�iLazy��ނŊi�[�����w��j
//---------------------------------------------------------------------
bool JlsScrMem::getListByLazy(queue <string>& queStr, LazyType typeLazy){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- ���g�p��Ԃ����� ---
	clearUnused(marg);
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// �L���[�ɒǉ�
	flags.move = false;		// ���f�[�^�͎c��
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ����񃊃X�g�����o���iLazy��ނŊi�[�����w��j
//---------------------------------------------------------------------
bool JlsScrMem::popListByLazy(queue <string>& queStr, LazyType typeLazy){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- ���g�p��Ԃ����� ---
	clearUnused(marg);
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// �L���[�ɒǉ�
	flags.move = true;		// ���f�[�^������
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ�����̈������
//---------------------------------------------------------------------
bool JlsScrMem::eraseMemByName(const string& strName){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- ���g�p��Ԃ����� ---
	clearUnused(marg);
	//--- ���s ---
	return exeCmdEraseMem(marg);
}
//---------------------------------------------------------------------
// �ۊǕ�����̈�𕡎�
//---------------------------------------------------------------------
bool JlsScrMem::copyMemByName(const string& strSrc, const string& strDst){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- ���g�p��Ԃ͌p�� ---
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = false;		// �L���̈�ɐV�K
	flags.move = false;		// ���f�[�^�͎c��
	return exeCmdCopyMem(sarg, darg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ�����̈���ړ�
//---------------------------------------------------------------------
bool JlsScrMem::moveMemByName(const string& strSrc, const string& strDst){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- ���g�p��Ԃ�ύX ---
	clearUnused(sarg);
	setUnused(darg);
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = false;		// �L���̈�ɐV�K
	flags.move = true;		// ���f�[�^������
	return exeCmdCopyMem(sarg, darg, flags);
}
//---------------------------------------------------------------------
// �ۊǕ�����̈��ǉ�
//---------------------------------------------------------------------
bool JlsScrMem::appendMemByName(const string& strSrc, const string& strDst){
	//--- ���ʎq�ݒ� ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- ���g�p��Ԃ͌p�� ---
	//--- ���s ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// �L���̈�ɒǉ�
	flags.move = false;		// ���f�[�^�͎c��
	return exeCmdCopyMem(sarg, darg, flags);
}

//=====================================================================
// ���ʂ̈�������R�}���h���s
// �Ԃ�l   �F���s�L��
//=====================================================================

//---------------------------------------------------------------------
// �P�s��������i�[
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf, int order){
	//--- ���ʎq�m�F ---
	bool success = false;
	if ( argDst.isExistBaseName() ){
		//--- �������� ---
		string str_name;
		argDst.getBaseName(str_name);
		success = memPushStr(str_name, strBuf, order);
	}
	else{
		success = argDst.isNameDummy();		// �_�~�[���͐�������
	}
	return success;
}
//---------------------------------------------------------------------
// �ۊǕ����񃊃X�g���擾�E���o��
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags){
	bool success = false;
	//--- ���݂���ꍇ�擾 ---
	if ( argSrc.isExistBaseName() ){
		string strName;
		argSrc.getBaseName(strName);
		success = memGetList(queStr, strName, flags);
	}else{
		//--- �o�̓N���A ---
		if ( !flags.add ){
			queue<string>().swap(queStr);
		}
		success = argSrc.isNameDummy();		// �_�~�[���͐�������
	}
	//--- �ۊǂ��ꂽLazy�p�������Lazy�p�{�̂� ---
	if ( argSrc.isExistExtName() ){
		//--- Lazy�p���ʎq��� ---
		JlsScrMemArg argDst;
		argDst.setNameByLazy(LazyType::FULL);	// Lazy�p�{��
		//--- �R�s�[���{ ---
		flags.add = true;			// Lazy�{�̂̋L�^�͎c�����܂ܒǉ�
		bool s2 = exeCmdCopyMem(argSrc, argDst, flags);
		if ( s2 ){					// Lazy�p���ʎq�ɕۊǂ����݂��Ă�true��Ԃ�
			success = true;
		}
	}
	return success;
}
//---------------------------------------------------------------------
// �ۊǕ�����̈������
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdEraseMem(JlsScrMemArg& argDst){
	bool success = false;
	vector <string> listName;
	argDst.getNameList(listName);
	for(int i=0; i < (int)listName.size(); i++){
		success |= memErase(listName[i]);
	}
	if ( argDst.isNameDummy() ){		// �_�~�[���͐�������
		success = true;
	}
	return success;
}
//---------------------------------------------------------------------
// �ۊǕ�����̈�𕡎ʁE�ړ�
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags){
	vector <string> slist;
	vector <string> dlist;
	argSrc.getNameList(slist);
	argDst.getNameList(dlist);
	int smax = (int) slist.size();
	int dmax = (int) dlist.size();

	bool success  = false;
	int loopmax = ( smax >= dmax )? smax : dmax;
	for(int i=0; i<loopmax; i++){
		bool si = false;
		//--- ���ڑ��݊m�F ---
		bool existSrc = ( i < smax )? true : false;
		bool existDst = ( i < dmax )? true : false;
		if ( existSrc ){
			existSrc = ! slist[i].empty();
		}
		if ( existDst ){
			existDst = ! dlist[i].empty();
		}
		//--- ���ڂ��������݂���ꍇ�̂݃R�s�[���� ---
		if ( existSrc && existDst ){
			si = memCopy(slist[i], dlist[i], flags);
		}else{
			si = existSrc & argDst.isNameDummy();	// ���͑��݂ŕ��ʐ悪�_�~�[�͐�������
			//--- Src���폜 ---
			if ( existSrc  && flags.move ){
				memErase(slist[i]);
			}
			//--- Dst���폜 ---
			if ( existDst  && !flags.add ){
				memErase(dlist[i]);
			}
		}
		success |= si;
	}
	return success;
}

//=====================================================================
// �L���̈�̒��ڑ���
// �Ԃ�l   �F���s�L��
//=====================================================================

//---------------------------------------------------------------------
// �L���̈�ɕ�����P�s��ǉ��i�[
//---------------------------------------------------------------------
bool JlsScrMem::memPushStr(const string& strName, const string& strBuf, int order){
	if (m_mapVar.size() >= SIZE_MEMVARNUM_MAX){	// �O�̂��ߓ��ꂷ���m�F
		return false;
	}
	if ( strName.empty() ){
		return false;
	}
	addQueueLine(m_mapVar[strName], strBuf, order);
	return true;
}
//---------------------------------------------------------------------
// �L���̈悩�當����S�̂�ǂݏo��
//---------------------------------------------------------------------
bool JlsScrMem::memGetList(queue <string>& queStr, const string& strName, CopyFlagRecord flags){
	if ( memIsExist(strName) == false ){		// �i�[����ĂȂ������ꍇ
		queue <MemDataRecord> q;
		setQueueStr(queStr, q, flags);				// ������
		return false;
	}
	setQueueStr(queStr, m_mapVar[strName], flags);
	return true;
}
//---------------------------------------------------------------------
// �L���̈����
//---------------------------------------------------------------------
bool JlsScrMem::memErase(const string& strName){
	if ( memIsNameExist(strName) == false ) return false;	// ���ʎq���̂��Ȃ������ꍇ
	m_mapVar.erase(strName);
	m_mapArg.erase(strName);	// �����ݒ�
	return true;
}
//---------------------------------------------------------------------
// �L���̈��ʂ̋L���̈�փR�s�[
//---------------------------------------------------------------------
bool JlsScrMem::memCopy(const string& strSrc, const string& strDst, CopyFlagRecord flags){
	if ( memIsExist(strSrc) == false ){			// �i�[����ĂȂ������ꍇ
		if ( flags.add == false ){
			memErase(strDst);
		}
		return false;
	}
	setQueueFull(m_mapVar[strDst], m_mapVar[strSrc], flags);
	m_mapArg[strDst] = m_mapArg[strSrc];	// �����ݒ�
	return true;
}
//---------------------------------------------------------------------
// �L���̈�m�F�i���ɑ��݂��Ă�����true��Ԃ��j
//---------------------------------------------------------------------
//--- �f�[�^�������܂� ---
bool JlsScrMem::memIsExist(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	bool flag = ( m_mapVar.find(strName) != m_mapVar.end() )? true : false;
	if ( flag ){
		if ( m_mapVar[strName].empty() ){
			flag = false;	// �f�[�^�����݂��Ȃ����false
		}
	}
	return flag;
}
//--- ���ʎq���̂̑��݊m�F ---
bool JlsScrMem::memIsNameExist(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	return ( m_mapVar.find(strName) != m_mapVar.end() );
}
//--- �����̑��� ---
bool JlsScrMem::memIsNameExistArg(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	return ( m_mapArg.find(strName) != m_mapArg.end() );
}
//---------------------------------------------------------------------
// queue�ɂP�s�ǉ��iorder�ɂ��}���ʒu�m�F���s���j
//---------------------------------------------------------------------
void JlsScrMem::addQueueLine(queue <MemDataRecord>& queDst, const string& strBuf, int order){
	MemDataRecord din = { order, strBuf };
	//--- �Ō�ɒǉ��̔��� ---
	bool flagTail = false;
	if ( queDst.empty() ){
		flagTail = true;
	}else if ( queDst.back().order <= din.order ){
		flagTail = true;
	}
	//--- �ǉ����� ---
	bool remain = true;
	if ( flagTail ){	// �Ō�ɒǉ�
		queDst.push(din);
	}else{
		queue <MemDataRecord> q;
		q.swap(queDst);
		while( q.empty() == false ){
			MemDataRecord dtmp = q.front();
			q.pop();
			if ( remain && (din.order < dtmp.order) ){
				queDst.push(din);
				remain = false;
			}
			queDst.push(dtmp);
		}
		if ( remain ){
			queDst.push(din);
		}
	}
}
//---------------------------------------------------------------------
// queue�ɕʂ�queue���i�[
//---------------------------------------------------------------------
void JlsScrMem::setQueueStr(queue <string>& queDstStr, queue <MemDataRecord>& queSrc, CopyFlagRecord flags){
	if ( flags.add == false ){
		queue<string>().swap(queDstStr);		// ������
	}
	queue <MemDataRecord> q = queSrc;
	while( q.empty() == false ){
		queDstStr.push( q.front().buffer );
		q.pop();
	}
	if ( flags.move ){
		queue<MemDataRecord>().swap(queSrc);		// ������
	}
}
//---------------------------------------------------------------------
// queue�ɕʂ�queue���i�[
//---------------------------------------------------------------------
void JlsScrMem::setQueueFull(queue <MemDataRecord>& queDst, queue <MemDataRecord>& queSrc, CopyFlagRecord flags){
	if ( flags.add ){
		queue <MemDataRecord> q = queSrc;
		while( q.empty() == false ){
			queDst.push( q.front() );
			q.pop();
		}
	}else{
		queDst = queSrc;
	}
	if ( flags.move ){
		queue<MemDataRecord>().swap(queSrc);		// ������
	}
}
//---------------------------------------------------------------------
// �A�N�Z�X�`�F�b�N����
//---------------------------------------------------------------------
void JlsScrMem::setUnused(JlsScrMemArg& marg){
	if ( marg.isExistBaseName() ){
		string str;
		marg.getBaseName(str);
		m_mapUnused[str] = true;
	}
}
void JlsScrMem::clearUnused(JlsScrMemArg& marg){
	if ( marg.isExistBaseName() ){
		string str;
		marg.getBaseName(str);
		if ( m_mapUnused.count(str) != 0 ){
			m_mapUnused.erase(str);
		}
	}
}
bool JlsScrMem::getUnusedStr(string& strBuf){
	strBuf.clear();
	for(auto itr = m_mapUnused.begin(); itr != m_mapUnused.end(); itr++){
		strBuf += itr->first;
		strBuf += " ";
	}
	if ( strBuf.empty() ) return false;
	strBuf = "warning: unused MemSet: " + strBuf + "\n";
	return true;
}
//---------------------------------------------------------------------
// ���ׂĂ̕ۊǓ��e���o���i�f�o�b�O�p�j
//---------------------------------------------------------------------
void JlsScrMem::getMapForDebug(string& strBuf){
	for( auto itr = m_mapVar.begin(); itr != m_mapVar.end(); ++itr ){
		strBuf += "[" + itr->first + "]" + "\n";
		queue <MemDataRecord> q = itr->second;
		while( q.empty() == false ){
			strBuf += q.front().buffer + "\n";
			q.pop();
		}
		strBuf += "\n";
	}
}
