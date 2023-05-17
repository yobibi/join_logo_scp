//
// �ϐ��̊i�[
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrReg.hpp"


///////////////////////////////////////////////////////////////////////
//
// �ϐ��N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// �ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsRegFile::setRegVar(const string& strName, const string& strVal, bool overwrite){
	int n;
	int nloc   = -1;
	int nlenvar = (int) strName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;
	string strPair;

	//--- �����ϐ��̏����������`�F�b�N ---
	for(int i=0; i<nMaxList; i++){
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		if (nlenvar == n){
			if ( isSameInLen(strName, strOrgName, n) ){
				nloc = i;
			}
		}
	}
	//--- �ݒ蕶����쐬 ---
	strPair = strName + ":" + strVal;
	//--- �����ϐ��̏������� ---
	if (nloc >= 0){
		if (overwrite){
			m_strListVar[nloc] = strPair;
		}
	}
	//--- �V�K�ϐ��̒ǉ� ---
	else{
		if (nMaxList < SIZE_VARNUM_MAX){		// �O�̂��ߕϐ��ő吔�܂�
			m_strListVar.push_back(strPair);
		}
		else{
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------
// �ϐ����폜
// ���́F
//   strName   : �ϐ���
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsRegFile::unsetRegVar(const string& strName){
	int nloc   = -1;
	int nlenvar = (int) strName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;

	//--- �ʒu�擾 ---
	for(int i=0; i<nMaxList; i++){
		int n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		if (nlenvar == n){
			if ( isSameInLen(strName, strOrgName, n) ){
				nloc = i;
			}
		}
	}
	if ( nloc < 0 ) return false;
	//--- �폜 ---
	m_strListVar.erase(m_strListVar.begin() + nloc);
	return true;
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo��
// ���́F
//   strCandName : �ǂݏo���ϐ����i���j
//   excact      : 0=���͕����ɍő�}�b�`����ϐ�  1=���͕����Ɗ��S��v����ϐ�
// �o�́F
//   �Ԃ�l  : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsRegFile::getRegVar(string& strVal, const string& strCandName, bool exact){
	int n;
	int nmatch = 0;
	int nloc   = -1;
	int nlencand = (int) strCandName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;

	//--- ���O�ƃ}�b�`����ʒu������ ---
	for(int i=0; i<nMaxList; i++){
		//--- �ϐ����ƒl������e�[�u������擾 ---
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		//--- �����e�[�u���ϐ����������܂ł̍ő��v��蒷����Ό��� ---
		if (nmatch < n){
			if (isSameInLen(strCandName, strOrgName, n) &&		// �擪�ʒu����}�b�`
				(n == nlencand || exact == false)){								// ���ꕶ����exact=false
				nloc   = i;
				nmatch = n;
			}
		}
	}
	//--- �}�b�`�����ꍇ�̒l�̓ǂݏo�� ---
	if (nloc >= 0){
		n = getRegNameVal(strOrgName, strVal, m_strListVar[nloc]);			// �ϐ��l���o��
		if ( strOrgName != strCandName.substr(0, n) ){
			msgErr += "warning : mismatch capital letter of register name(";
			msgErr += strCandName.substr(0, n) + " " + strOrgName + ")\n";
		}
	}
	return nmatch;
}

//---------------------------------------------------------------------
// �i�[�ϐ��𖼑O�ƒl�ɕ����i�ϐ��ǂݏ����֐�����̃T�u���[�`���j
//---------------------------------------------------------------------
int JlsRegFile::getRegNameVal(string& strName, string& strVal, const string& strPair){
	//--- �ŏ��̃f���~�^���� ---
	int n = (int) strPair.find(":");
	//--- �f���~�^�𕪉����ďo�͂ɐݒ� ---
	if (n > 0){
		strName = strPair.substr(0, n);
		int nLenPair = (int) strPair.length();
		if (n < nLenPair-1){
			strVal = strPair.substr(n+1);
		}
		else{
			strVal = "";
		}
	}
	return n;
}
//---------------------------------------------------------------------
// �啶���������֘A
//---------------------------------------------------------------------
void JlsRegFile::setIgnoreCase(bool valid){
	m_ignoreCase = valid;
}
bool JlsRegFile::isSameInLen(const string& s1, const string& s2, int nLen){
	if ( m_ignoreCase ){
		return ( _strnicmp(s1.c_str(), s2.c_str(), nLen) == 0 );	// �擪�ʒu����}�b�`
	}
	return ( s1.substr(0, nLen) == s2.substr(0, nLen) );
}
//---------------------------------------------------------------------
// �Q�Ɠn���ϐ��Ƃ��Ă̐ݒ�
//---------------------------------------------------------------------
void JlsRegFile::setFlagAsRef(const string& strName){
	m_flagListRef[strName] = true;
}
bool JlsRegFile::isRegNameRef(const string& strName){
	return ( m_flagListRef.find(strName) != m_flagListRef.end() );
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�����݂�������o��
// �o�́F
//   �Ԃ�l   : �G���[���b�Z�[�W�L���i0=�Ȃ��A1=����j
//   msg      : �擾�����G���[���b�Z�[�W
//---------------------------------------------------------------------
bool JlsRegFile::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}


///////////////////////////////////////////////////////////////////////
//
// �K�w�\���ϐ��N���X
//
///////////////////////////////////////////////////////////////////////

JlsScrReg::JlsScrReg(){
	onlyLocal = false;
	ignoreCase = true;		// ���݂̏����ݒ�l
	globalReg.setIgnoreCase(ignoreCase);
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w���쐬
// �o�́F
//   �Ԃ�l    : �쐬�K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrReg::createLocalCall(){
	return createLocalCommon(RegOwner::Call);
}
int JlsScrReg::createLocalFunc(){
	return createLocalCommon(RegOwner::Func);
}
int JlsScrReg::createLocalOne(){
	return createLocalCommon(RegOwner::One);
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�̏I��
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrReg::releaseLocalAny(){
	return releaseLocalCommon(RegOwner::Any);
}
int JlsScrReg::releaseLocalCall(){
	return releaseLocalCommon(RegOwner::Call);
}
int JlsScrReg::releaseLocalFunc(){
	return releaseLocalCommon(RegOwner::Func);
}
int JlsScrReg::releaseLocalOne(){
	return releaseLocalCommon(RegOwner::One);
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�̎擾
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrReg::getLocalLayer(){
	return (int) layerReg.size();
}
//---------------------------------------------------------------------
// ���ʏ���
//---------------------------------------------------------------------
int JlsScrReg::createLocalCommon(RegOwner owner){
	if ( layerReg.size() >= INT_MAX/4 ){		// �O�̂��߃T�C�Y����
		msgErr += "error:too many create local-register\n";
		return 0;
	}
	bool flagBase = ( owner != RegOwner::One );	// �����K�w�i0=��ʊK�w��������  1=�ŏ�ʊK�w�����j
	RegLayer layer;
	layer.owner  = owner;
	layer.base   = flagBase;
	layer.regfile.setIgnoreCase(ignoreCase);
	//--- ���[�J���ϐ��K�w���쐬 ---
	layerReg.push_back(layer);
	//--- �ŏ�ʊK�w������Call�ł���Έ��������[�J���ϐ��Ɋi�[ ---
	if ( flagBase ){
		setRegFromArg();
	}else{
		clearArgReg();			// �g��Ȃ����������폜
	}

	return (int) layerReg.size();
}
int JlsScrReg::releaseLocalCommon(RegOwner owner){
	int numLayer = 0;
	if ( layerReg.empty() == false ){
		numLayer = (int) (layerReg.size() - 1);
		bool allow = ( layerReg[numLayer].owner == owner || owner == RegOwner::Any );
		if ( allow ){	// �I������
			layerReg.pop_back();
			clearArgReg();			// �g��Ȃ����������폜
			numLayer = (int) (layerReg.size() + 1);	
		}else{
			msgErr += "error:not match release local-register layer\n";
			return 0;
		}
	}
	if ( numLayer == 0 ){
		msgErr += "error:too many release local-register layer\n";
		return 0;
	}
	return numLayer;
}
//---------------------------------------------------------------------
// �ϐ�������
// ���́F
//   strName   : �ϐ���
//   flagLocal : 0=���ׂĂ̕ϐ�  1=���[�J���ϐ��݂̂P�ӏ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsScrReg::unsetRegVar(const string& strName, bool flagLocal){
	bool success = false;
	int numLayer = -1;
	bool cont = true;
	while( cont ){
		int numLast = numLayer;
		cont = findRegForUnset(numLayer, strName, flagLocal);
		if ( cont ){
			cont = unsetRegCore(strName, numLayer);		// �ϐ�����
		}
		if ( cont ){
			success = true;
			if ( numLayer <= 0 || flagLocal ){
				cont = false;
			}else if ( numLast <= numLayer && numLast >= 0 ){	// �O�̂���
				cont = false;
			}
		}
	}
	return success;
}
//---------------------------------------------------------------------
// ���[�J���ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsScrReg::setLocalRegVar(const string& strName, const string& strVal, bool overwrite){
	if ( layerReg.empty() ){	// ���[�J���ϐ��K�w�̑��݂�O�̂��߃`�F�b�N
		msgErr += "error:internal setting(empty local-register layer)\n";
		return false;
	}
	//--- ���݂̃��[�J���ϐ��K�w�ɏ������� ---
	int numLayer = (int) layerReg.size();
	return setRegCore(strName, strVal, overwrite, numLayer);
}
//---------------------------------------------------------------------
// �ϐ���ݒ�i���[�J���ϐ��ɑ��݂�����D��A�Ȃ���΃O���[�o���ϐ��Ɂj
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsScrReg::setRegVar(const string& strName, const string& strVal, bool overwrite){
	//--- �K�w�͎��������ŏ������� ---
	int  numLayer = -1;
	return setRegCore(strName, strVal, overwrite, numLayer);
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo���i���[�J���ϐ��D��A�Ȃ���΃O���[�o���ϐ��j
// ���́F
//   strCandName : �ǂݏo���ϐ����i���j
//   excact      : 0=���͕����ɍő�}�b�`����ϐ�  1=���͕����Ɗ��S��v����ϐ�
// �o�́F
//   �Ԃ�l  : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsScrReg::getRegVar(string& strVal, const string& strCandName, bool exact){
	return findRegForRead(strCandName, strVal, exact);
}
//---------------------------------------------------------------------
// Call�ň����Ƃ��Ďg����ϐ���ݒ�
// ���́F
//   strName : �����Ɏg����ϐ���
//   strVal  : �����Ɏg����ϐ��l
//---------------------------------------------------------------------
bool JlsScrReg::setArgReg(const string& strName, const string& strVal){
	//--- �������X�g�ɒǉ� ---
	if ( listValArg.size() >= INT_MAX/4 ){		// �O�̂��߃T�C�Y����
		msgErr += "error:too many create arg-registers\n";
		return false;
	}
	listValArg.push_back(strName);
	listValArg.push_back(strVal);
	return true;
}
//--- �Q�Ɠn���p ---
bool JlsScrReg::setArgRefReg(const string& strName, const string& strVal){
	//--- �������X�g�ɒǉ� ---
	if ( listRefArg.size() >= INT_MAX/4 ){		// �O�̂��߃T�C�Y����
		msgErr += "error:too many create arg-registers\n";
		return false;
	}
	listRefArg.push_back(strName);
	listRefArg.push_back(strVal);
	return true;
}
//--- �Ԃ�l�ϐ��ƂȂ�֐�����ݒ� ---
void JlsScrReg::setArgFuncName(const string& strName){
	nameFuncReg = strName;
}
//---------------------------------------------------------------------
// �ǂݏo���ŃO���[�o���ϐ������Ȃ��ݒ�
// ���́F
//   flag : ���[�J���ϐ��ɂȂ����̃O���[�o���ϐ��Q�Ɓifalse=����  true=�֎~�j
//---------------------------------------------------------------------
void JlsScrReg::setLocalOnly(bool flag){
	onlyLocal = flag;
}
void JlsScrReg::setIgnoreCase(bool valid){
	ignoreCase = valid;
	int numLayer = (int) layerReg.size();
	for(int i=0; i<numLayer; i++){
		layerReg[numLayer-1].regfile.setIgnoreCase(ignoreCase);
	}
	globalReg.setIgnoreCase(ignoreCase);
}
void JlsScrReg::setGlobalLock(const string& strName, bool flag){
	if ( flag ){
		m_mapGlobalLock[strName] = true;
	}else{
		if ( isGlobalLocked(strName) ){
			m_mapGlobalLock.erase(strName);
		}
	}
}
bool JlsScrReg::isGlobalLocked(const string& strName){
	return ( m_mapGlobalLock.count(strName) != 0 );
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�����݂�������o��
// �o�́F
//   �Ԃ�l   : �G���[���b�Z�[�W�L���i0=�Ȃ��A1=����j
//   msg      : �擾�����G���[���b�Z�[�W
//---------------------------------------------------------------------
bool JlsScrReg::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}

//---------------------------------------------------------------------
// �ϐ����K�w�w��ŏ�������
//---------------------------------------------------------------------
//--- �ϐ��̏��� ---
bool JlsScrReg::unsetRegCore(const string& strName, int numLayer){
	if ( checkErrRegName(strName) ){	// �ϐ����ُ펞�̏I��
		return false;
	}
	bool success;
	if ( numLayer > 0 ){
		success = layerReg[numLayer-1].regfile.unsetRegVar(strName);
	}else{
		success = globalReg.unsetRegVar(strName);
	}
	return success;
}
//--- �ʏ�̕ϐ��������� ---
// numLayer=-1 �̎��͊K�w��������������
bool JlsScrReg::setRegCore(const string& strName, const string& strVal, bool overwrite, int numLayer){
	if ( checkErrRegName(strName) ){	// �ϐ����ُ펞�̏I��
		return false;
	}
	//--- �����ϐ����m�F ---
	string strWriteName = strName;
	string strWriteVal = strVal;
	bool flagOvw = overwrite;
	int  numWriteLayer = numLayer;
	if ( !findRegForWrite(strWriteName, strWriteVal, flagOvw, numWriteLayer) ){
		return false;		// �A�N�Z�X�ł��Ȃ����͖���
	}
	if ( numWriteLayer < 0 ){
		numWriteLayer = 0;		// ������������Ȃ���΃O���[�o���ϐ��ɏ�������
	}
	if ( numWriteLayer > 0 ){
		if ( isRegNameRef(strWriteName, numWriteLayer) && overwrite ){
			msgErr += "error: overwrite ref register " + strWriteName + "\n";
			return false;		// �Q�Ɠn���ϐ����̂̃��[�J�����������͋֎~
		}
	}
	//--- ���ۂ̏������� ---
	bool success;
	if ( numWriteLayer > 0 ){
		success = layerReg[numWriteLayer-1].regfile.setRegVar(strWriteName, strWriteVal, flagOvw);
	}else{
		success = globalReg.setRegVar(strWriteName, strWriteVal, flagOvw);
		if ( isGlobalLocked(strWriteName) && success ){
			msgErr += "warning: global fixed register is changed(" + strWriteName + ")\n";
		}
	}
	return success;
}
//--- �Q�Ɠn���Ƃ��ĕϐ��������� ---
bool JlsScrReg::setRegCoreAsRef(const string& strName, const string& strVal, int numLayer){
	if ( checkErrRegName(strName) ){	// �ϐ����ُ펞�̏I��
		return false;
	}
	bool success;
	if ( numLayer > 0 ){
		bool overwrite = true;
		success = layerReg[numLayer-1].regfile.setRegVar(strName, strVal, overwrite);
		layerReg[numLayer-1].regfile.setFlagAsRef(strName);
	}else{
		success = false;		// �O���[�o���ϐ��͎Q�Ɠn���ɂł��Ȃ�
	}
	return success;
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo��
//---------------------------------------------------------------------
//--- ��������ϐ��K�w������ ---
bool JlsScrReg::findRegForUnset(int& numLayer, const string& strName, bool flagLocal){
	RegSearch data(strName);
	data.numLayer = numLayer;
	data.stopRef = true;	// �Q�Ɠn���͎~�߂�
	bool success = findRegData(data);
	if ( success ){
		numLayer = data.numLayer;
		if ( numLayer < 0 || data.flagRef ){	// �Q�Ɠn���͑ΏۊO
			success = false;
		}else if ( numLayer == 0 && flagLocal){
			success = false;
		}
	}
	return success;
}
//--- �������ݑO�Ɋ����ϐ������� ---
// �Q�ƕϐ��⃊�X�g���������͍ŏI�I�ɏ������ޕϐ����ɕύX����
bool JlsScrReg::findRegForWrite(string& strName, string& strVal, bool& overwrite, int& numLayer){
	RegSearch data(strName);
	if ( numLayer >= 0 ){		// �K�w����̏ꍇ
		data.numLayer = numLayer;
		data.onlyOneLayer = true;
	}
	if ( !findRegData(data) ){	// �f�[�^�擾
		//--- �����f�[�^���Ȃ��ꍇ�̏��� ---
		auto nList = data.regOrg.listElem.size();
		if ( nList >= 2 ){	// 2���z��ȏ�͖���
			return false;
		}else if ( nList == 1 ){	// �z��͍ŏ��̗v�f�̂ݏ������݉\�ł���
			if ( data.regOrg.listElem[0] != 1 ) return false;
			strName = data.regOrg.nameBase;
			string listTmp;
			funcList.setListStrIns(listTmp, strVal, 1);
			strVal = listTmp;
		}
		return true;
	}
	//--- �����f�[�^������ꍇ�̏��� ---
	strName = data.regSel.nameBase;
	numLayer = data.numLayer;
	return findRegListForWrite(data.regSel, strVal, overwrite, data.strVal);
}
//--- �ǂݏo���f�[�^���擾 ---
int JlsScrReg::findRegForRead(const string& strName, string& strVal, bool exact){
	int retMatch = 0;
	//--- �ϐ��ɑ��݂��邩���� ---
	RegSearch data(strName);
	data.exact = exact;
	if ( findRegData(data) ){
		strVal = data.strVal;
		retMatch = data.regOrg.nMatch;		// ���f�[�^�̃}�b�`��
		//--- ���X�g�v�f�Ή� ---
		findRegListForRead(data.regSel, strVal);
	}
	return retMatch;
}
//--- ���X�g�v�f�������ꍇ�͑S�̂̃f�[�^����Ώە����̂ݍ����ւ��� ---
bool JlsScrReg::findRegListForWrite(RegDivList& regName, string& strVal, bool& overwrite, const string& strRead){
	if ( regName.listElem.empty() ){	// ���X�g�̂Ȃ��ʏ�f�[�^�͉��������I��
		return true;
	}
	bool success = true;
	string strParts = strRead;
	//--- �e�v�f���擾 ---
	int nElem = (int)regName.listElem.size();
	vector<string> listHold;
	for(int i=nElem-1; i>=1; i--){
		listHold.push_back(strParts);
		string strBak = strParts;
		if ( !funcList.getListStrElement(strParts, strBak, regName.listElem[i]) ){
			success = false;
		}
	}
	//--- �������݃f�[�^�ݒ� ---
	if ( success ){
		int sizeParts = funcList.getListStrSize(strParts);
		int selElem = regName.listElem[0];
		if ( selElem > sizeParts + 1 ){		// �v�f�������݂͊���+1�v�f�܂�
			success = false;
		}else if ( selElem == sizeParts + 1 ){		// �v�f�Ō�ɒǉ�
			overwrite = true;		// �ǉ��ł��S�̂Ƃ��Ă͊����ϐ��Ȃ̂Őݒ�ύX
			success = funcList.setListStrIns(strParts, strVal, selElem);
		}else{
			success = funcList.setListStrRep(strParts, strVal, selElem);
		}
	}
	//--- �e�v�f�ɐݒ��f�[�^��߂� ---
	if ( success ){
		for(int i=1; i<=nElem-1; i++){
			string strItem = strParts;
			strParts = listHold[nElem-i-1];
			funcList.setListStrRep(strParts, strItem, regName.listElem[i]);
		}
		strVal = strParts;
	}
	if ( !success ){
		msgErr += "error: not exist the list element in the register " + regName.nameBase + "\n";
	}
	return success;
}
//--- ���X�g�v�f�������ꍇ�͊Y�����X�g�̂ݔ����o�� ---
bool JlsScrReg::findRegListForRead(RegDivList& regName, string& strVal){
	if ( regName.listElem.empty() ){	// ���X�g�̂Ȃ��ʏ�f�[�^�͉��������I��
		return true;
	}
	int nElem = (int)regName.listElem.size();
	for(int i=nElem-1; i>=0; i--){
		string strBak = strVal;
		if ( !funcList.getListStrElement(strVal, strBak, regName.listElem[i]) ){
			msgErr += "error: out of range at register read\n";
			strVal.clear();
			return false;
		}
	}
	return true;
}
//
// �ϐ����K�w�������ēǂݏo��
// �o�́F
//   �Ԃ�l   : �ϐ��L��
//   data     : �ǂݏo�����ϐ���� �K�w�i-1:�Y���Ȃ��A0:�O���[�o���K�w�A1-:���[�J���K�w�j
//
bool JlsScrReg::findRegData(RegSearch& data){
	//--- �ϐ��̊K�w�ƒl���擾 ---
	bool success = findRegDataFromLayer(data);
	data.decide();		// �ݒ���s
	if ( !success ){	// �ϐ���������Ȃ���Ύ��s
		return false;
	}
	//--- ���[�J���ϐ��̎��͎Q�Ɠn�����m�F ---
	if ( data.numLayer > 0 ){
		data.flagRef = isRegNameRef(data.strName, data.numLayer);
	}
	if ( !data.flagRef ){		// �Q�Ɠn���łȂ���Ί���
		return true;
	}
	if ( data.stopRef || data.onlyOneLayer ){		// �Q�Ɠn����܂Ń`�F�b�N�łȂ���Ί���
		return true;
	}
	//--- �Q�Ɠn���̎Q�Ɛ�ϐ����擾 ---
	bool cont = true;
	while( success && cont ){
		//--- �P�K�w���̎Q�Ɛ�ϐ����擾 ---
		success = data.updateRef(data.strVal);		// �Q�Ɛ��V�����ϐ����ɂ���
		if ( success ){
			success = findRegDataFromLayer(data);
		}
		if ( success ){
			data.flagRef = isRegNameRef(data.strName, data.numLayer);
			if ( !data.flagRef ){	// �Q�Ɠn���ł͂Ȃ����ϐ�������������I��
				cont = false;
			}
		}
	}
	return success;
}
// ����: data(strName, numLayer)
// �o��: data(numLayer, numMatch, [strName, strVal]) []�͏󋵂ɂ��o��
bool JlsScrReg::findRegDataFromLayer(RegSearch& data){
	//--- �����K�w�̐ݒ� ---
	int n = (int) layerReg.size();
	if ( n > data.numLayer && data.numLayer >= 0 ){		// �ő僌�C���[�w�肪���鎞
		n = data.numLayer;
	}
	bool skipGlobal = ( data.numLayer != 0 && data.onlyOneLayer );
	data.numLayer = -1;
	data.numMatch = 0;
	bool scope = true;
	//--- ���ʊK�w���猟�����K�w�܂ŕϐ����� ---
	while( scope && n > 0 ){
		n --;
		string str;
		int nmatch = layerReg[n].regfile.getRegVar(str, data.strName, data.exact);
		if ( nmatch > 0 ){				// �ϐ�����
			data.numLayer = n+1;
			data.numMatch = nmatch;
			data.strVal   = str;
			scope         = false;
			popErrLower(layerReg[n].regfile);
		}else if ( layerReg[n].base ){	// ��ʊK�w���������Ȃ��K�w
			scope    = false;
		}else if ( data.onlyOneLayer ){	// 1�K�w�����������Ȃ�
			scope    = false;
		}
	}
	//--- �Ȃ���΃O���[�o���ϐ������� ---
	if ( data.numMatch == 0 && !onlyLocal && !skipGlobal ){
		string str;
		int nMatch = globalReg.getRegVar(str, data.strName, data.exact);
		if ( nMatch > 0 ){
			data.numLayer = 0;
			data.numMatch = nMatch;
			data.strVal   = str;
			popErrLower(globalReg);
		}
	}
	bool success = ( data.numMatch > 0 );
	if ( success && !data.exact ){
		data.strName = data.strName.substr(0, data.numMatch);
	}
	return success;
}
//--- �Q�Ɠn���t���O�擾 ---
bool JlsScrReg::isRegNameRef(const string& strName, int numLayer){
	if ( numLayer <= 0 ){		// �O���[�o���ϐ��͎Q�ƂȂ�
		return false;
	}
	return layerReg[numLayer-1].regfile.isRegNameRef(strName);
}

//---------------------------------------------------------------------
// �����i�[�f�[�^���폜
//---------------------------------------------------------------------
void JlsScrReg::clearArgReg(){
	//--- �������X�g���폜 ---
	listValArg.clear();
	listRefArg.clear();
	nameFuncReg.clear();
}
//---------------------------------------------------------------------
// ���������[�J���ϐ��ɐݒ�
//---------------------------------------------------------------------
void JlsScrReg::setRegFromArg(){
	//--- �ݒ� ---
	setRegFromArgSub( listValArg, false );		// ref=false
	setRegFromArgSub( listRefArg, true );		// ref=true
	//--- �Ԃ�l�̎󂯎��悪�Ȃ����̃_�~�[���[�J���ϐ��ݒ� ---
	if ( !nameFuncReg.empty() ){
		bool overwrite = false;		// ���ݒ莞�̂݃_�~�[���W�X�^�Ƃ��ĕԂ�l�ϐ����쐬
		setLocalRegVar(nameFuncReg, "", overwrite);
	}
	//--- �������X�g���폜 ---
	clearArgReg();
}
void JlsScrReg::setRegFromArgSub(vector<string>& listArg, bool ref){
	int sizeList = (int) listArg.size();
	if ( sizeList > 0 ){
		//--- �������X�g�����[�J���ϐ��ɐݒ� ---
		if ( ref ){
			int numLayer = (int) layerReg.size();
			for(int i=0; i<sizeList-1; i+=2){
				setRegCoreAsRef(listArg[i], listArg[i+1], numLayer);
			}
		}else{
			bool overwrite = true;
			for(int i=0; i<sizeList-1; i+=2){
				setLocalRegVar(listArg[i], listArg[i+1], overwrite);
			}
		}
	}
}
//---------------------------------------------------------------------
// �ϐ����̍Œ���̈ᔽ�����m�F
// �o�́F
//   �Ԃ�l   : �G���[�L���i0=����A1=�G���[����j
//---------------------------------------------------------------------
bool JlsScrReg::checkErrRegName(const string& strName, bool silent){
	//--- �Œ���̈ᔽ�����m�F ---
	string strCheckFull  = "!#$%&'()*+,-./:;<=>?\"";		// �ϐ�������Ƃ��Ďg�p�֎~
	string strCheckFirst = strCheckFull + "0123456789";		// �ϐ��擪�����Ƃ��Ďg�p�֎~
	string strFirst = strName.substr(0, 1);
	if ( strCheckFirst.find(strFirst) != string::npos ){
		if ( !silent ){
			msgErr += "error: register setting, invalid first char(" + strName + ")\n";
		}
		return true;
	}
	for(int i=0; i < (int)strCheckFull.length(); i++){
		string strNow = strCheckFull.substr(i, 1);
		if ( strName.find(strNow) != string::npos ){
			if ( !silent ){
				msgErr += "error: register setting, bad char exist(" + strName + ")\n";
			}
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------
// ���ʊK�w�̃G���[�擾
//---------------------------------------------------------------------
bool JlsScrReg::popErrLower(JlsRegFile& regfile){
	string msgTmp;
	if ( regfile.popMsgError(msgTmp) ){
		msgErr += msgTmp;
		return true;
	}
	return false;
}
