//
// JL�X�N���v�g�p�R�}���h���e�i�[�f�[�^
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsCmdSet.hpp"

///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g�R�}���h�ݒ蔽�f�p
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// �����ݒ�
//---------------------------------------------------------------------
JlsCmdLimit::JlsCmdLimit(){
	clear();
}

void JlsCmdLimit::clear(){
	process = 0;
	rmsecHeadTail = {-1, -1};
	rmsecFrameLimit = {-1, -1};

	clearLogoList();	// �L���ȃ��S�ԍ����X�g
	clearLogoBase();	// �ΏۂƂ������S�I��
	clearPickList();	// -pick�I�v�V�����p�ێ����X�g
}

//---------------------------------------------------------------------
// �擪�ƍŌ�̈ʒu
//---------------------------------------------------------------------
RangeMsec JlsCmdLimit::getHeadTail(){
	return rmsecHeadTail;
}

Msec JlsCmdLimit::getHead(){
	return rmsecHeadTail.st;
}

Msec JlsCmdLimit::getTail(){
	return rmsecHeadTail.ed;
}

bool JlsCmdLimit::setHeadTail(RangeMsec rmsec){
	process |= ARG_PROCESS_HEADTAIL;
	rmsecHeadTail = rmsec;
	return true;
}

//---------------------------------------------------------------------
// �t���[���͈�(-F�I�v�V����)
//---------------------------------------------------------------------
RangeMsec JlsCmdLimit::getFrameRange(){
	return rmsecFrameLimit;
}

bool JlsCmdLimit::setFrameRange(RangeMsec rmsec){
	if ((process & ARG_PROCESS_HEADTAIL) == 0){
		signalInternalError(ARG_PROCESS_FRAMELIMIT);
	}
	process |= ARG_PROCESS_FRAMELIMIT;
	rmsecFrameLimit = rmsec;
	return true;
}

//---------------------------------------------------------------------
// �L���ȃ��S�ԍ����X�g
//---------------------------------------------------------------------
void JlsCmdLimit::clearLogoList(){
	listLogoStd.clear();
	listLogoDir.clear();
	listLogoOrg.clear();
	forceLogoStdFix = false;
	existLogoDirDmy = false;
	process &= ~ARG_PROCESS_VALIDLOGO;
}

//--- �������݁i���S�ԍ��w��j ---
bool JlsCmdLimit::addLogoListStd(Msec msec, LogoEdgeType edge){
	if ((process & ARG_PROCESS_HEADTAIL) == 0){
		signalInternalError(ARG_PROCESS_VALIDLOGO);
	}
	process |= ARG_PROCESS_VALIDLOGO;
	ArgLogoList argset = {msec, edge};
	listLogoStd.push_back(argset);
	return true;
}
//--- ���ڃt���[���w��i�����������ʒu�j�̑��ݐݒ� ---
void JlsCmdLimit::addLogoListDirectDummy(bool flag){
	existLogoDirDmy = flag;
}
//--- �������݁i���ڃt���[���w��j ---
void JlsCmdLimit::addLogoListDirect(Msec msec, LogoEdgeType edge){
	ArgLogoList argset = {msec, edge};
	ArgLogoList argset2 = {-1, edge};
	listLogoDir.push_back(argset);
	listLogoOrg.push_back(argset2);
}
//--- ��ʒu�t���i���ڃt���[���w��j ---
void JlsCmdLimit::attachLogoListOrg(int num, Msec msec, LogoEdgeType edge){
	if ( num < 0 || num >= (int)listLogoOrg.size() ) return;
	listLogoOrg[num].msec = msec;
	listLogoOrg[num].edge = edge;
}
//--- ���ʌĂяo�� ---
Msec JlsCmdLimit::getLogoListMsec(int nlist){	// �ݒ���ʒu(msec)�擾
	if ( isErrorLogoList(nlist) ) return -1;
	if ( isLogoListDirect() ){
		return listLogoDir[nlist].msec;
	}
	return listLogoStd[nlist].msec;
}
Msec JlsCmdLimit::getLogoListOrgMsec(int nlist){	// �{���̊�ʒu(msec)�擾
	if ( isErrorLogoList(nlist) ) return -1;
	if ( isLogoListDirect() ){
		return listLogoOrg[nlist].msec;
	}
	return listLogoStd[nlist].msec;
}
LogoEdgeType JlsCmdLimit::getLogoListEdge(int nlist){	// �ݒ���ʒu�i�G�b�W���j�擾
	if ( isErrorLogoList(nlist) ) return LogoEdgeType::LOGO_EDGE_RISE;
	if ( isLogoListDirect() ){
		return listLogoDir[nlist].edge;
	}
	return listLogoStd[nlist].edge;
}
LogoEdgeType JlsCmdLimit::getLogoListOrgEdge(int nlist){	// �{���̊�ʒu�i�G�b�W���j�擾
	if ( isErrorLogoList(nlist) ) return LogoEdgeType::LOGO_EDGE_RISE;
	if ( isLogoListDirect() ){
		return listLogoOrg[nlist].edge;
	}
	return listLogoStd[nlist].edge;
}
int JlsCmdLimit::sizeLogoList(){		// ���X�g���擾
	if ( isLogoListDirect() ){
		return (int)listLogoDir.size();
	}
	return (int)listLogoStd.size();
}
//--- �t���[�����ڎw��p�̊g�� ---
bool JlsCmdLimit::isLogoListDirect(){		// �g���I�����
	if ( forceLogoStdFix ) return false;
	return ( listLogoDir.size() > 0 || existLogoDirDmy );
}
void JlsCmdLimit::forceLogoListStd(bool flag){		// �ꎞ�I�Ɋg��������
	forceLogoStdFix = flag;
}
//--- �������� ---
bool JlsCmdLimit::isErrorLogoList(int nlist){
	if ( isLogoListDirect() ){
		return ( nlist < 0 || nlist >= (int) listLogoDir.size() );
	}
	return ( nlist < 0 || nlist >= (int) listLogoStd.size() );;
}


//---------------------------------------------------------------------
// �ΏۂƂ������S�I��
//---------------------------------------------------------------------
void JlsCmdLimit::clearLogoBase(){
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = -1;
	edgeBase = LOGO_EDGE_RISE;

	process &= ~ARG_PROCESS_BASELOGO;
}
//--- �{���̊�ʒu��ݒ�i�����S�^�������S�ǂ��炩�P�̂ݐݒ�j ---
bool JlsCmdLimit::setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_VALIDLOGO) == 0){
		signalInternalError(ARG_PROCESS_BASELOGO);
	}
	process |= ARG_PROCESS_BASELOGO;
	flagBaseNrf = true;
	nrfBase = nrf;
	nscBase = -1;
	edgeBase = edge;
	return true;
}
bool JlsCmdLimit::setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_VALIDLOGO) == 0){
		signalInternalError(ARG_PROCESS_BASELOGO);
	}
	process |= ARG_PROCESS_BASELOGO;
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = nsc;
	edgeBase = edge;
	return true;
}
//--- �ݒ肳�ꂽ����S�ʒu�����擾 ---
bool JlsCmdLimit::isLogoBaseExist(){	// �{���̊���S�ʒu�����݂��邩�H
	return ( nrfBase >= 0 || nscBase >= 0 );
}
bool JlsCmdLimit::isLogoBaseNrf(){		// �{���̊���S�ʒu�������S���H
	return flagBaseNrf;
}
Nrf JlsCmdLimit::getLogoBaseNrf(){		// �{���̊���S�ʒu�i�����S�̃��S�ԍ��j
	return nrfBase;
}
Nsc JlsCmdLimit::getLogoBaseNsc(){		// �{���̊���S�ʒu�i�������S�̃V�[���`�F���W�ԍ��j
	return nscBase;
}
LogoEdgeType JlsCmdLimit::getLogoBaseEdge(){	// �{���̊���S�����^��������
	return edgeBase;
}
//---------------------------------------------------------------------
// �^�[�Q�b�g�I���\�͈�
//---------------------------------------------------------------------
//--- �^�[�Q�b�g�f�[�^�����i�L���b�V���͏����j ---
void JlsCmdLimit::clearTargetData(){
	//--- �^�[�Q�b�g�I���\�͈� ---
	wmsecTarget   = {-1, -1, -1};
	fromLogo      = false;
	//--- �^�[�Q�b�g�����X�g ---
	targetLocDst = {TargetScpType::None, LOGO_EDGE_RISE, false, false, -1, -1, -1};
	targetLocEnd = {TargetScpType::None, LOGO_EDGE_RISE, false, false, -1, -1, -1};

	process &= ~ARG_PROCESS_TARGETRANGE;
	process &= ~ARG_PROCESS_RESULT;
}
//--- �^�[�Q�b�g�ʒu�ݒ� ---
bool JlsCmdLimit::setTargetRange(WideMsec wmsec, bool fromLogo){
	if ((process & ARG_PROCESS_BASELOGO) == 0 && fromLogo){
		signalInternalError(ARG_PROCESS_TARGETRANGE);
	}
	process |= ARG_PROCESS_TARGETRANGE;
	wmsecTarget  = wmsec;
	fromLogo     = fromLogo;
	return true;
}
//--- �^�[�Q�b�g�ʒu�擾 ---
WideMsec JlsCmdLimit::getTargetRangeWide(){
	return wmsecTarget;
}
bool JlsCmdLimit::isTargetRangeFromLogo(){
	return fromLogo;
}

//---------------------------------------------------------------------
// �^�[�Q�b�g�Ɉ�ԋ߂��ʒu
//---------------------------------------------------------------------
//--- ���ʈʒu�^�I���ʒu�̏������� ---
void JlsCmdLimit::setResultDst(TargetLocInfo tgIn){
	if ((process & ARG_PROCESS_TARGETRANGE) == 0 ||
		(process & ARG_PROCESS_SCPENABLE  ) == 0){
//		signalInternalError(ARG_PROCESS_RESULT);
	}
	setResultSubMake(tgIn);
	targetLocDst = tgIn;
}
void JlsCmdLimit::setResultEnd(TargetLocInfo tgIn){
	setResultSubMake(tgIn);
	targetLocEnd = tgIn;
}
void JlsCmdLimit::setResultSubMake(TargetLocInfo& tgIn){
	if ( tgIn.tp == TargetScpType::ScpNum || 
	     tgIn.tp == TargetScpType::Force ){
		tgIn.valid = true;
	}else{
		tgIn.valid = false;
	}
	if ( tgIn.tp == TargetScpType::Invalid ){
		tgIn.nsc = -1;
		tgIn.msout = -1;
	}else if ( tgIn.edge == LogoEdgeType::LOGO_EDGE_FALL ){
		tgIn.msout = tgIn.msbk;
	}else{
		tgIn.msout = tgIn.msec;
	}
}
//--- ���ʈʒu�^�I���ʒu�̓ǂݏo�� ---
TargetLocInfo JlsCmdLimit::getResultDst(){
	if ( isPickListValid() ){		// -pick�Ή�
		return getPickListDst();
	}
	return targetLocDst;
}
TargetLocInfo JlsCmdLimit::getResultEnd(){
	if ( isPickListValid() ){		// -pick�Ή�
		return getPickListEnd();
	}
	return targetLocEnd;
}
Nsc JlsCmdLimit::getResultDstNsc(){
	TargetLocInfo tgDst = getResultDst();
	if ( tgDst.tp == TargetScpType::ScpNum ){
		return tgDst.nsc;
	}
	return -1;
}
LogoEdgeType JlsCmdLimit::getResultDstEdge(){
	return getResultDst().edge;
}
TargetLocInfo JlsCmdLimit::getResultDstCurrent(){
	return targetLocDst;
}
TargetLocInfo JlsCmdLimit::getResultEndCurrent(){
	return targetLocEnd;
}

//---------------------------------------------------------------------
// -pick����
//---------------------------------------------------------------------
void JlsCmdLimit::clearPickList(){
	listPickResult.clear();
	numPickList = 0;
}
void JlsCmdLimit::addPickListCurrent(){
	TargetLocInfoSet dat;
	dat.d = targetLocDst;
	dat.e = targetLocEnd;
	listPickResult.push_back(dat);
}
int JlsCmdLimit::sizePickList(){
	return (int) listPickResult.size();
}
void JlsCmdLimit::selectPickList(int num){
	numPickList = num;
}
bool JlsCmdLimit::isPickListValid(){
	int nsize = sizePickList();
	if ( nsize > 0 && numPickList > 0 && numPickList <= nsize ){
		return true;
	}
	return false;
}
TargetLocInfo JlsCmdLimit::getPickListDst(){
	return listPickResult[numPickList-1].d;
}
TargetLocInfo JlsCmdLimit::getPickListEnd(){
	return listPickResult[numPickList-1].e;
}

//---------------------------------------------------------------------
// �G���[�m�F
//---------------------------------------------------------------------
void JlsCmdLimit::signalInternalError(CmdProcessFlag flags){
	string mes = "error:internal flow at ArgCmdLimit flag=" + to_string(flags) + ",process=" + to_string(process);
	lcerr << mes << endl;
}

