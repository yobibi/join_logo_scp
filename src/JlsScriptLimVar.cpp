//
// ���s�X�N���v�g�R�}���h�̈�����������^�[�Q�b�g���i��
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptLimit.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"

///////////////////////////////////////////////////////////////////////
//
// ��������ɂ��^�[�Q�b�g�I��N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ������
//---------------------------------------------------------------------
void JlsScriptLimVar::setPdata(JlsDataset *pdata){
	this->pdata  = pdata;
}
void JlsScriptLimVar::clear(){
	rmsecHeadTail = {-1, -1};
	rmsecFrameLimit = {-1, -1};

	clearRangeDst();
	clearPrepEnd();
	clearZone();
	clearScpEnable();
}
//=====================================================================
// �R�}���h���ʂ̐ݒ�
//=====================================================================
//--- ���s�R�}���h�������i�[ ---
void JlsScriptLimVar::initVar(JlsCmdSet& cmdset){
	clear();
	opt = cmdset.arg;
}
//--- ���ʈʒu���̐ݒ� ---
void JlsScriptLimVar::setHeadTail(RangeMsec rmsec){
	rmsecHeadTail = rmsec;
}
void JlsScriptLimVar::setFrameRange(RangeMsec rmsec){
	rmsecFrameLimit = rmsec;
}
//--- ���ʈʒu���̓ǂݏo�� ---
RangeMsec JlsScriptLimVar::getHeadTail(){
	return rmsecHeadTail;
}
RangeMsec JlsScriptLimVar::getFrameRange(){
	return rmsecFrameLimit;
}

//=====================================================================
// ���S�ʒu���X�g���̎w�胍�S�Ŋ���S�f�[�^���쐬
//=====================================================================

void JlsScriptLimVar::clearLogoBase(){
	nBaseListNum = -1;
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = -1;
	edgeBase = LOGO_EDGE_RISE;
	msecBaseBsrc = -1;
	msecBaseBorg = -1;
	edgeBaseBsrc = LOGO_EDGE_RISE;
	wmsecBaseBtg = {-1, -1, -1};
	edgeBaseBtg = LOGO_EDGE_RISE;
}
void JlsScriptLimVar::setLogoBaseListNum(int n){
	nBaseListNum = n;
}
//--- �{���̊�ʒu��ݒ�i�����S�^�������S�ǂ��炩�P�̂ݐݒ�j ---
void JlsScriptLimVar::setLogoBaseNrf(Nrf nrf, LogoEdgeType edge){
	flagBaseNrf = true;
	nrfBase = nrf;
	nscBase = -1;
	edgeBase = edge;
}
void JlsScriptLimVar::setLogoBaseNsc(Nsc nsc, LogoEdgeType edge){
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = nsc;
	edgeBase = edge;
}
//--- �i-fromabs�������f�����j�ύX��̊���S�ʒu��ݒ� ---
void JlsScriptLimVar::setLogoBsrcMsec(Msec msec){
	msecBaseBsrc = msec;
}
//--- �i���S�ԍ��Ō��肷��j�{���̊���S�ʒu��ݒ� ---
void JlsScriptLimVar::setLogoBorgMsec(Msec msec){
	msecBaseBorg = msec;
}
void JlsScriptLimVar::setLogoBsrcEdge(LogoEdgeType edge){
	edgeBaseBsrc = edge;
}
//--- �^�[�Q�b�g�I��̈�̌v�Z�Ɏg�p�����ʒu��ݒ� ---
void JlsScriptLimVar::setLogoBtgWmsecEdge(WideMsec wmsec, jlsd::LogoEdgeType edge){
	wmsecBaseBtg = wmsec;
	edgeBaseBtg  = edge;
}

//--- �ݒ肳�ꂽ����S�ʒu�����擾 ---
int JlsScriptLimVar::getLogoBaseListNum(){	// ��ʒu�̓��X�g���ŉ��Ԗڂ�
	return nBaseListNum;
}
Msec JlsScriptLimVar::getLogoBsrcMsec(){	// �ύX��̊���S�ʒu
	return msecBaseBsrc;
}
Msec JlsScriptLimVar::getLogoBorgMsec(){	// �{���̊���S�ʒu�i���݂��Ȃ����͕ύX��Ɉ�ԋ߂��ʒu�j
	return msecBaseBorg;
}
bool JlsScriptLimVar::isLogoBaseExist(){	// �{���̊���S�ʒu�����݂��邩�H
	return ( nrfBase >= 0 || nscBase >= 0 );
}
bool JlsScriptLimVar::isLogoBaseNrf(){		// �{���̊���S�ʒu�������S���H
	return flagBaseNrf;
}
Nrf JlsScriptLimVar::getLogoBaseNrf(){		// �{���̊���S�ʒu�i�����S�̃��S�ԍ��j
	return nrfBase;
}
Nsc JlsScriptLimVar::getLogoBaseNsc(){		// �{���̊���S�ʒu�i�������S�̃V�[���`�F���W�ԍ��j
	return nscBase;
}
LogoEdgeType JlsScriptLimVar::getLogoBaseEdge(){	// �{���̊���S�����^��������
	return edgeBase;
}
LogoEdgeType JlsScriptLimVar::getLogoBsrcEdge(){	// �{���̊���S�����^��������
	return edgeBaseBsrc;
}
WideMsec JlsScriptLimVar::getLogoBtgWmsec(){	// �^�[�Q�b�g�p����S�ʒu
	return wmsecBaseBtg;
}
LogoEdgeType JlsScriptLimVar::getLogoBtgEdge(){	// �^�[�Q�b�g�p����S�����^��������
	return edgeBaseBtg;
}
//---------------------------------------------------------------------
// ��ʒu����̃��S�O�㕝���擾
// ���́F
//   step     : �w�萔��̑O�ネ�S�ʒu�܂ł̕�
//   flagWide : false=�O������S�ʒu�܂�  true=�O��͉\���͈͂܂Ŋg��
// �o�́F
//   wmsec    : �O��܂ރ��S�ʒu�i.just=��ʒu���S  .early=�O���̃��S�ʒu  .late=�㑤�̃��S�ʒu�j
//---------------------------------------------------------------------
//--- ����S�ʒu����̑O�ネ�S�� ---
void JlsScriptLimVar::getWidthLogoFromBase(WideMsec& wmsec, int step, bool flagWide){
	Msec         msecLogo = getLogoBorgMsec();
	LogoEdgeType edgeLogo = getLogoBaseEdge();
	getWidthLogoCommon(wmsec, msecLogo, edgeLogo, step, flagWide);
}
//--- (-fromlast�␳���܂�)�^�[�Q�b�g�ʒu�v�Z�p����S�ʒu����̑O�ネ�S�� ---
void JlsScriptLimVar::getWidthLogoFromBaseForTarget(WideMsec& wmsec, int step, bool flagWide){
	Msec         msecLogo = getLogoBtgWmsec().just;
	LogoEdgeType edgeLogo = getLogoBtgEdge();
	getWidthLogoCommon(wmsec, msecLogo, edgeLogo, step, flagWide);
}
void JlsScriptLimVar::getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide){
	int locLogo = pdata->getClogoNumNear(msecLogo, edgeLogo);
	WideMsec wTmpC, wTmpP, wTmpN;
	wTmpC = pdata->getClogoWmsecFromNum(locLogo);
	wTmpP = pdata->getClogoWmsecFromNum(locLogo-step);
	wTmpN = pdata->getClogoWmsecFromNum(locLogo+step);
	if ( flagWide ){
		wmsec.just  = wTmpC.just;
		wmsec.early = wTmpP.early;
		wmsec.late  = wTmpN.late;
	}else{
		wmsec.just  = wTmpC.just;
		wmsec.early = wTmpP.just;
		wmsec.late  = wTmpN.just;
	}
}

//=====================================================================
// Dst�͈͐ݒ�
//=====================================================================
void JlsScriptLimVar::clearRangeDst(){
	listRangeDst.clear();
	numRangeDst = 0;
}
void JlsScriptLimVar::addRangeDst(WideMsec wmsecFind, WideMsec wmsecFrom){
	ArgRange argTmp;
	argTmp.wmsecFind = wmsecFind;
	argTmp.wmsecFrom = wmsecFrom;
	listRangeDst.push_back(argTmp);
}
void JlsScriptLimVar::selRangeDstNum(int num){
	numRangeDst = num;
}

//--- �^�[�Q�b�g�ʒu�擾 ---
int JlsScriptLimVar::sizeRangeDst(){
	return (int) listRangeDst.size();
}
bool JlsScriptLimVar::isRangeDstMultiFrom(){
	if ( sizeRangeDst() <= 1 ) return false;
	Msec msecZ = listRangeDst[0].wmsecFrom.just;
	for(int i=1; i<sizeRangeDst(); i++){
		if ( listRangeDst[i].wmsecFrom.just != msecZ ){
			return true;
		}
	}
	return false;
}
WideMsec JlsScriptLimVar::getRangeDstWide(){
	return getRangeDstItemWide(numRangeDst);
}
Msec JlsScriptLimVar::getRangeDstJust(){
	WideMsec wmsec = getRangeDstItemWide(numRangeDst);
	return wmsec.just;
}
Msec JlsScriptLimVar::getRangeDstFrom(){
	return getRangeDstItemFromWide(numRangeDst).just;
}
void JlsScriptLimVar::getRangeDstFromForScp(Msec& msec, Msec& msbk, Nsc& nsc){
	WideMsec wmsec = getRangeDstItemFromWide(numRangeDst);
	msec = wmsec.late;
	msbk = wmsec.early;
	if ( msec != msbk ){
		nsc = pdata->getNscFromMsecMgn(wmsec.just, pdata->msecValExact, SCP_END_EDGEIN);
	}else{
		nsc = -1;
	}
}
WideMsec JlsScriptLimVar::getRangeDstItemWide(int num){
	if ( isErrorRangeDst(num) ) return {-1, -1, -1};
	return listRangeDst[num].wmsecFind;
}
WideMsec JlsScriptLimVar::getRangeDstItemFromWide(int num){
	if ( isErrorRangeDst(num) ) return {-1, -1, -1};
	return listRangeDst[num].wmsecFrom;
}
bool JlsScriptLimVar::isErrorRangeDst(int num){
	return ( num < 0 || num >= sizeRangeDst() );
}

//=====================================================================
// �ʒu�����p�̐ݒ�
//=====================================================================
//---------------------------------------------------------------------
// �����p�̐ݒ菉����
//---------------------------------------------------------------------
void JlsScriptLimVar::initSeekVar(JlsCmdSet& cmdset){
	seek = {};
	bool seekNoEdge = opt.getOptFlag(OptType::FlagNoEdge);
	//--- �����ݒ�iNextTail�R�}���h�Ή��j ---
	if ( cmdset.arg.cmdsel == CmdType::NextTail ){
		seek.flagNextTail = true;
		seekNoEdge        = false;
		if ( cmdset.arg.selectEdge == LogoEdgeType::LOGO_EDGE_RISE ){
			seek.selectLogoRise = true;	// ���S�����ʒu�D��
		}
	}
	seek.tgDst.exact = opt.getOptFlag(OptType::FlagFixPos);
	seek.rnscScp = pdata->getRangeNscTotal(seekNoEdge);
}
//---------------------------------------------------------------------
// Dst�ʒu���͈͏����𖞂���������
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToDst(Msec msecBsrc, Msec msecDst){
	WideMsec wmsecTmp = getRangeDstWide();		// Dst�͈͂͐ݒ�ǂݍ���
	if ( msecDst < wmsecTmp.early || wmsecTmp.late < msecDst ){
		return false;
	}
	return isZoneAtDst(msecBsrc, msecDst);
}
//---------------------------------------------------------------------
// End�ʒu���͈͏����𖞂���������iEnd�͈͈͂����ŗ^����j
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToEnd(Msec msecDst, Msec msecEnd, WideMsec wmsecRange){
	if ( msecEnd < wmsecRange.early || wmsecRange.late < msecEnd ){
		return false;
	}
	return isZoneAtEnd(msecDst, msecEnd);
}
//---------------------------------------------------------------------
// End�ʒu���͈͏����𖞂���������iZone�����̂݊m�F�j
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToEndZone(Msec msecDst, Msec msecEnd){
	return isZoneAtEnd(msecDst, msecEnd);
}

//=====================================================================
// �I���ʒu�̎��O����
//=====================================================================

void JlsScriptLimVar::clearPrepEnd(){
	listPrepEndRange.clear();
	existPrepEndRefer = false;
	fromPrepEndAbs = false;
	multiPrepEndBase = false;
	tgPrepEndAbs = {};
	listPrepEndBaseMsec.clear();
	listPrepEndBaseMsbk.clear();
}
void JlsScriptLimVar::addPrepEndRange(WideMsec wmsec){
	listPrepEndRange.push_back(wmsec);
}
void JlsScriptLimVar::setPrepEndRefer(bool flag){
	existPrepEndRefer = flag;
}
void JlsScriptLimVar::setPrepEndAbs(bool fromAbs, bool multiBase, TargetLocInfo tgEnd){
	fromPrepEndAbs  = fromAbs;
	multiPrepEndBase = multiBase;
	tgPrepEndAbs     = tgEnd;
}
int JlsScriptLimVar::sizePrepEndRange(){
	return (int)listPrepEndRange.size();
}
bool JlsScriptLimVar::isPrepEndRangeExist(){
	return ( listPrepEndRange.size() > 0 );
}
bool JlsScriptLimVar::isPrepEndReferExist(){
	return existPrepEndRefer;
}
bool JlsScriptLimVar::isPrepEndFromAbs(){
	return fromPrepEndAbs;
}
bool JlsScriptLimVar::isPrepEndMultiBase(){
	return ( multiPrepEndBase && fromPrepEndAbs );
}
TargetLocInfo JlsScriptLimVar::getPrepEndAbs(){
	return tgPrepEndAbs;
}
WideMsec JlsScriptLimVar::getPrepEndRangeWithOffset(int num, Msec msecOfs){
	if ( num >= sizePrepEndRange() || num < 0 ) return {-1, -1, -1};
	WideMsec wmsecTmp = listPrepEndRange[num];
	wmsecTmp.just  += msecOfs;
	wmsecTmp.early += msecOfs;
	wmsecTmp.late  += msecOfs;
	return wmsecTmp;
}
Msec JlsScriptLimVar::getPrepEndRangeForceLen(){
	if ( sizePrepEndRange() <= 0 ) return 0;
	return listPrepEndRange[0].just;
}

//=====================================================================
// Zone�ݒ�
//=====================================================================

void JlsScriptLimVar::clearZone(){
	msecZoneSrc = -1;
	validZoneRange = true;
	rmsecZoneRange = {-1, -1};
	rmsecZoneForbid = {-1, -1};
	listTLRange.clear();
}
//---------------------------------------------------------------------
// ���ʈʒu����ʒu����̗L���̈悩����
//---------------------------------------------------------------------
bool JlsScriptLimVar::isZoneAtDst(Msec msecBsrc, Msec msecDst){
	//--- -TgtLimit�m�F ---
	if ( !isTgtLimitAllow(msecDst) ){
		return false;
	}
	//--- ��ʒu����łȂ����Dst��Zone����Ȃ� ---
	if ( opt.getOptFlag(OptType::FlagZEnd) ) return true;
	//--- �͈͊m�F ---
	return isZoneBothEnds(msecBsrc, msecDst);
}
//---------------------------------------------------------------------
// �I���ʒu�����ʈʒu����̗L���̈悩����
//---------------------------------------------------------------------
bool JlsScriptLimVar::isZoneAtEnd(Msec msecDst, Msec msecEnd){
	//--- ��ʒu���猋�ʈʒu�̎��͖����� ---
	if ( !opt.getOptFlag(OptType::FlagZEnd) ) return true;
	//--- �͈͊m�F ---
	return isZoneBothEnds(msecDst, msecEnd);
}

// �Ώ�2�_��Zone�͈͓��m�F
bool JlsScriptLimVar::isZoneBothEnds(Msec msecP, Msec msecQ){
	//--- �L���b�V������f�[�^�擾 ---
	bool validRange;
	RangeMsec rmsecRange;
	RangeMsec rmsecForbid;
	bool flagCache = getZoneCache(validRange, rmsecRange, rmsecForbid, msecP);
	//--- �L���b�V���ɂȂ���Όv�Z���Ď擾 ---
	if ( !flagCache ){
		RangeMsec rmsecTmpR = rmsecRange;
		RangeMsec rmsecTmpF = rmsecForbid;
		validRange = false;
		if ( calcZoneRange(rmsecTmpR, msecP) ){	// Zone�ݒ�
			if ( calcZoneUnder(rmsecTmpR, msecP) ){	// under�ݒ�
				validRange = true;
				rmsecRange = rmsecTmpR;
			}
		}
		if ( calcZoneOver(rmsecTmpF, msecP) ){
			rmsecForbid = rmsecTmpF;
		}
		setZoneCache(msecP, validRange, rmsecRange, rmsecForbid);	// �v�Z���ʂ��L���b�V���Ɋi�[
	}
	//--- �͈͊m�F ---
	if ( !validRange ) return false;
	if ( isZoneInForbid(msecQ, rmsecForbid) ) return false;
	return isZoneInRange(msecQ, rmsecRange);
}
// �w��ʒu���w��Zone���ɂ��邩����
bool JlsScriptLimVar::isZoneInRange(Msec msecNow, RangeMsec rmsecZone){
	if ( (msecNow < rmsecZone.st && rmsecZone.st >= 0) ||
		 (msecNow > rmsecZone.ed && rmsecZone.ed >= 0) ){
		return false;
	}
	return true;
}
// �w��ʒu���w��֎~�̈���ɂ��邩����
bool JlsScriptLimVar::isZoneInForbid(Msec msecNow, RangeMsec rmsecZone){
	if ( (msecNow >= rmsecZone.st && rmsecZone.st >= 0) &&
		 (msecNow <= rmsecZone.ed && rmsecZone.ed >= 0) ){
		return true;
	}
	return false;
}
// Zone�ݒ�
bool JlsScriptLimVar::calcZoneRange(RangeMsec& rmsecZone, Msec msecB){
	bool useZoneL = opt.getOptFlag(OptType::FlagZoneL);
	bool useZoneN = opt.getOptFlag(OptType::FlagZoneN);
	if ( useZoneL || useZoneN ){
		int nsize = pdata->sizeClogoList();
		vector<Msec> listMsec;
		for(int i=0; i<nsize; i++){
			Msec msecTmp = pdata->getClogoMsecFromNum(i);
			listMsec.push_back(msecTmp);
		}
		if ( !calcZoneRangeSub(rmsecZone, msecB, listMsec, useZoneL) ){
			return false;
		}
	}
	//--- zone�ݒ�(���ڎw��ʒu) ---
	bool useZoneImmL = opt.isSetStrOpt(OptType::ListZoneImmL);
	bool useZoneImmN = opt.isSetStrOpt(OptType::ListZoneImmN);
	if ( useZoneImmL || useZoneImmN ){
		string strList;
		if ( useZoneImmN ){
			strList = opt.getStrOpt(OptType::ListZoneImmN);
		}else{
			strList = opt.getStrOpt(OptType::ListZoneImmL);
		}
		vector<Msec> listMsec;
		pdata->cnv.getListValMsecM1(listMsec, strList);
		if ( !calcZoneRangeSub(rmsecZone, msecB, listMsec, useZoneImmL) ){
			return false;
		}
	}
	return true;
}
// ���X�g����Zone�ݒ�͈̔͂�ǉ�
bool JlsScriptLimVar::calcZoneRangeSub(RangeMsec& rmsecZone, Msec msecTg, vector<Msec>& listMsec, bool flagL){
	Msec msecMgn = pdata->getClogoMsecMgn();
	//--- �e���S�ʒu�ŗ̈���擾 ---
	RangeMsec rmsecL = {-1, -1};
	int nsize = (int)listMsec.size();
	for(int i=0; i<nsize-1; i=i+2){
		RangeMsec rms = {listMsec[i], listMsec[i+1]};
		if ( rms.ed < 0 || rms.st >= rms.ed ){
			continue;
		}
		if ( flagL ){	// ���S����̈�͈������S��Ԃ݂̂�OR�irmsecL�Ɋi�[�j
			if ( rms.ed >= msecTg - msecMgn ){
				Msec msecSt = rms.st - msecMgn;
				if ( rmsecL.st > msecSt || rmsecL.st < 0 ){
					if ( msecSt < 0 ) msecSt = 0;
					rmsecL.st = msecSt;
				}
			}
			if ( rms.st <= msecTg + msecMgn ){
				Msec msecEd = rms.ed + msecMgn;
				if ( rmsecL.ed < msecEd || rmsecL.ed < 0 ){
					rmsecL.ed = msecEd;
				}
			}
		}else{	// ���S�Ȃ��̈�͌���Zone����������S��Ԃ��J�b�g
			if ( rms.st < msecTg - msecMgn ){
				Msec msecEd = rms.ed - msecMgn;
				if ( rmsecZone.st < msecEd || rmsecZone.st < 0 ){
					rmsecZone.st = msecEd;
				}
			}
			if ( rms.ed > msecTg + msecMgn ){
				Msec msecSt = rms.st + msecMgn;
				if ( rmsecZone.ed > msecSt || rmsecZone.ed < 0 ){
					if ( msecSt < 0 ) msecSt = 0;
					rmsecZone.ed = msecSt;
				}
			}
		}
	}
	//--- ���S����̈�͎擾�����������S��Ԃƌ���Zone��AND ---
	if ( flagL ){
		if ( rmsecL.st < 0 || rmsecL.ed < 0 ){	// �������S��Ԍ��o�Ȃ�
			return false;
		}else if ( rmsecL.st >= rmsecL.ed ){	// �������S��Ԍ��o�Ȃ�
			return false;
		}else{
			if ( rmsecZone.st < rmsecL.st || rmsecZone.st < 0 ) rmsecZone.st = rmsecL.st;
			if ( rmsecZone.ed > rmsecL.ed || rmsecZone.ed < 0 ) rmsecZone.ed = rmsecL.ed;
		}
	}
	//--- �̈摶�݊m�F ---
	if ( rmsecZone.st >= 0 && rmsecZone.ed >= 0 ){
		if ( rmsecZone.st >= rmsecZone.ed ) return false;
	}
	return true;
}
// �����\����؂�w��͈͂�Zone�͈͂ɒǉ�
bool JlsScriptLimVar::calcZoneUnder(RangeMsec& rmsecZone, Msec msecB){
	bool useZUnderC = opt.isSetOpt(OptType::NumZUnderC);
	if ( useZUnderC ){
		int numUnder = opt.getOpt(OptType::NumZUnderC);
		Nsc nscP = pdata->getNscPrevScpDispFromMsecCount(msecB, numUnder, true);
		Nsc nscN = pdata->getNscNextScpDispFromMsecCount(msecB, numUnder, true);
		Msec msecMgn = pdata->getClogoMsecMgn();
		RangeMsec rmsecU;
		rmsecU.st = pdata->getMsecScpBk(nscP);
		rmsecU.ed = pdata->getMsecScp(nscN);
		if ( rmsecU.st >= msecMgn ){
			rmsecU.st -= msecMgn;
		}else if ( rmsecU.st >= 0 ){
			rmsecU.st = 0;
		}
		if ( rmsecU.ed >= 0 ){
			rmsecU.ed += msecMgn;
		}
		if ( rmsecU.st >= 0 ){
			if ( rmsecZone.st < rmsecU.st || rmsecZone.st < 0 ){
				 rmsecZone.st = rmsecU.st;
			}
		}
		if ( rmsecU.ed >= 0 ){
			if ( rmsecZone.ed > rmsecU.ed || rmsecZone.ed < 0 ){
				 rmsecZone.ed = rmsecU.ed;
			}
		}
	}
	//--- �̈摶�݊m�F ---
	if ( rmsecZone.st >= 0 && rmsecZone.ed >= 0 ){
		if ( rmsecZone.st >= rmsecZone.ed ) return false;
	}
	return true;
}
// �����\����؂�w��͈͂��֎~�͈͂ɒǉ�
bool JlsScriptLimVar::calcZoneOver(RangeMsec& rmsecForbid, Msec msecB){
	bool useZOverC = opt.isSetOpt(OptType::NumZOverC);
	if ( useZOverC ){
		int numOver = opt.getOpt(OptType::NumZOverC);
		Nsc nscP = pdata->getNscPrevScpDispFromMsecCount(msecB, numOver, true);
		Nsc nscN = pdata->getNscNextScpDispFromMsecCount(msecB, numOver, true);
		Msec msecMgn = pdata->getClogoMsecMgn();
		RangeMsec rmsecO;
		rmsecO.st = pdata->getMsecScp(nscP);
		rmsecO.ed = pdata->getMsecScpBk(nscN);
		if ( rmsecO.st >= msecMgn ){
			rmsecO.st -= msecMgn;
		}else if ( rmsecO.st >= 0 ){
			rmsecO.st = 0;
		}
		if ( rmsecO.ed >= 0 ){
			rmsecO.ed += msecMgn;
		}
		if ( rmsecForbid.st > rmsecO.st || rmsecForbid.st < 0 ){
			 rmsecForbid.st = rmsecO.st;
		}
		if ( rmsecForbid.ed < rmsecO.ed || rmsecForbid.ed < 0 ){
			 rmsecForbid.ed = rmsecO.ed;
		}
	}
	//--- �̈摶�݊m�F ---
	if ( rmsecForbid.st >= 0 && rmsecForbid.ed >= 0 ){
		if ( rmsecForbid.st < rmsecForbid.ed ) return true;
	}
	return false;
}
// �L���b�V���Ɏw��ʒu��zone�����i�[
void JlsScriptLimVar::setZoneCache(Msec msecSrc, bool validRange, RangeMsec rmsecRange, RangeMsec rmsecForbid){
	msecZoneSrc = msecSrc;
	validZoneRange = validRange;
	rmsecZoneRange = rmsecRange;
	rmsecZoneForbid = rmsecForbid;
}
// �L���b�V���Ɋi�[���ꂽ�w��ʒu��zone�����擾
bool JlsScriptLimVar::getZoneCache(bool& validRange, RangeMsec& rmsecRange, RangeMsec& rmsecForbid, Msec msecSrc){
	if ( msecSrc == msecZoneSrc ){
		validRange = validZoneRange;
		rmsecRange = rmsecZoneRange;
		rmsecForbid = rmsecZoneForbid;
		return true;
	}
	validRange = true;
	rmsecRange = {-1, -1};
	rmsecForbid = {-1, -1};
	return false;
}
//---------------------------------------------------------------------
// -TgtLimit�ݒ�
//---------------------------------------------------------------------
bool JlsScriptLimVar::isTgtLimitAllow(Msec msecTarget){
	//--- �I�v�V�����Ȃ���Ή������Ȃ� ---
	if ( !opt.isSetOpt(OptType::MsecTgtLimL) ) return true;

	int nsize = (int) listTLRange.size();
	if ( nsize == 0 ){
		nsize = setTgtLimit();
		if ( nsize == 0 ) return true;	// ���X�g���Ȃ���Ζ������ŋ���
	}
	//--- �e���X�g���� ---
	bool det = false;
	for(int i=0; i<nsize; i++){
		if ( listTLRange[i].st <= msecTarget && msecTarget <= listTLRange[i].ed){
			det = true;
		}
	}
	return det;
}
int JlsScriptLimVar::setTgtLimit(){
	string strList = opt.getStrOpt(OptType::StrValListR);	// $LISTHOLD
	if ( strList.empty() ) return 0;
	listTLRange.clear();
	vector<Msec> listMsec;
	if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
		Msec msecL = (Msec) opt.getOpt(OptType::MsecTgtLimL);
		Msec msecR = (Msec) opt.getOpt(OptType::MsecTgtLimR);
		if ( msecL != -1 || msecR != -1 ){
			for(int i=0; i<(int)listMsec.size(); i++){
				RangeMsec rmsec = {msecL, msecR};
				if ( msecL != -1 ) rmsec.st += listMsec[i];
				if ( msecR != -1 ) rmsec.ed += listMsec[i];
				listTLRange.push_back(rmsec);
			}
		}
	}
	return (int) listTLRange.size();
}

//=====================================================================
// ������������
//=====================================================================

//---------------------------------------------------------------------
// -SC, -NoSC�n�I�v�V�����ɑΉ�����V�[���`�F���W�L������
// ���́F
//   msecBase : ��ƂȂ�t���[��
//   edge     : 0:start edge  1:end edge
//   tgcat    : ����Ώێ��
// �Ԃ�l�F
//   false : ��v����
//   true  : ��v�m�F
//---------------------------------------------------------------------
bool JlsScriptLimVar::isScpEnableAtMsec(int msecBase, LogoEdgeType edge, TargetCatType tgcat){
	//--- ���Έʒu���o���鎞�́AFrom�ʒu����̌����͉������Ȃ� ---
	if ( opt.tack.floatBase && (tgcat == TargetCatType::From) ){
		return true;
	}

	bool result = true;
	for(int k=0; k<opt.sizeScOpt(); k++){
		//--- ���Έʒu�R�}���h���菈�� ---
		bool needCheck = false;
		TargetCatType category = opt.getScOptCategory(k);
		switch( tgcat ){
			case TargetCatType::From :
				if ( category == TargetCatType::From ){
					needCheck = true;
				}
				break;
			case TargetCatType::Shift :
				if ( (category == TargetCatType::From) ||
					 (category == TargetCatType::RX  ) ){
					needCheck = true;
				}
				break;
			case TargetCatType::Dst :
				if ( (category == TargetCatType::Dst) ||
					 (category == TargetCatType::RX ) ){
					needCheck = true;
				}else if ( opt.tack.floatBase && category == TargetCatType::From ){
					needCheck = true;
				}
				break;
			case TargetCatType::End :
				if ( (category == TargetCatType::End) ||
					 (category == TargetCatType::RX ) ){
					needCheck = true;
				}
				break;
			default :
				break;
		}
		if ( !needCheck ){
			continue;
		}
		//--- �Ώۂł���΃`�F�b�N ---
		result = isScpEnableAtMsecCheck(msecBase, edge, k);
		if ( !result ){
			break;
		}
	}
	return result;
}
//--- �ʃI�v�V�����`�F�b�N ---
bool JlsScriptLimVar::isScpEnableAtMsecCheck(int msecBase, LogoEdgeType edge, int numOpt){
	//--- �֘A�I�v�V�������ݎ��̂݃`�F�b�N ---
	OptType sctype = opt.getScOptType(numOpt);
	if ( sctype == OptType::ScNone ){
		return true;
	}
	bool result = true;
	DataScpRecord dt;
	Nsc nscScposSc   = -1;
	Nsc nscSmuteAll  = -1;
	Nsc nscSmutePart = -1;
	Nsc nscChapAuto  = -1;
	Nsc nscChapAtc   = -1;
	Msec lensMin = opt.getScOptMin(numOpt);
	Msec lensMax = opt.getScOptMax(numOpt);
	//--- -AC�p�\���擾 ---
	Term term = {};
	pdata->setTermEndtype(term, SCP_END_EDGEIN);	// �[���܂߂Ď��{
	pdata->setTermForDisp(term, true);		// �\���p�̍\��
	bool contTerm = pdata->getTermNext(term);
	//--- �e�ʒu�`�F�b�N ---
	RangeNsc rnscScp = pdata->getRangeNscTotal( opt.getOptFlag(OptType::FlagNoEdge) );
	for(int j=rnscScp.st; j<=rnscScp.ed; j++){
		if ( contTerm ){	// -AC�p�X�V
			if ( j > term.nsc.ed ) contTerm = pdata->getTermNext(term);
		}
		pdata->getRecordScp(dt, j);
		int msecNow;
		if ( jlsd::isLogoEdgeRise(edge) ){
			msecNow = dt.msec;
		}
		else{
			msecNow = dt.msbk;
		}
		if ((msecNow - msecBase >= lensMin || lensMin == -1) &&
			(msecNow - msecBase <= lensMax || lensMax == -1)){
			nscScposSc = j;
			// for -AC option
			if ( contTerm ){
				if ( j == term.nsc.st || j == term.nsc.ed ){
					nscChapAuto = j;
				}
			}
			ScpChapType chapNow = dt.chap;
			if (chapNow >= SCP_CHAP_DECIDE || chapNow == SCP_CHAP_CDET ){
				nscChapAtc = j;
			}
		}
		// �����n
		int msecSmuteS = dt.msmute_s;
		int msecSmuteE = dt.msmute_e;
		if (msecSmuteS < 0 || msecSmuteE < 0){
			msecSmuteS = msecNow;
			msecSmuteE = msecNow;
		}
		// for -SMA option �i������񂪂���ꍇ�̂݌��o�j
		if ((msecSmuteS - msecBase <= lensMin) &&
			(msecSmuteE - msecBase >= lensMax)){
			nscSmuteAll = j;
		}
		//for -SM option
		if ((msecSmuteS - msecBase <= lensMax || lensMax == -1) &&
			(msecSmuteE - msecBase >= lensMin || lensMin == -1)){
			nscSmutePart = j;
		}
	}
	if (nscScposSc < 0 && sctype == OptType::ScSC){	// -SC
		result = false;
	}
	else if (nscScposSc >= 0 && sctype == OptType::ScNoSC){	// -NoSC
		result = false;
	}
	else if (nscSmutePart < 0 && sctype == OptType::ScSM){	// -SM
		result = false;
	}
	else if (nscSmutePart >= 0 && sctype == OptType::ScNoSM){	// -NoSM
		result = false;
	}
	else if (nscSmuteAll < 0 && sctype == OptType::ScSMA){	// -SMA
		result = false;
	}
	else if (nscSmuteAll >= 0 && sctype == OptType::ScNoSMA){	// -NoSMA
		result = false;
	}
	else if (nscChapAuto < 0 && sctype == OptType::ScAC){	// -AC
		result = false;
	}
	else if (nscChapAuto >= 0 && sctype == OptType::ScNoAC){	// -NoAC
		result = false;
	}
	else if (nscChapAtc < 0 && sctype == OptType::ScACC){	// -ACC
		result = false;
	}
	else if (nscChapAtc >= 0 && sctype == OptType::ScNoACC){	// -NoACC
		result = false;
	}
	return result;
}
//---------------------------------------------------------------------
// �e����SC�ʒu�̔���ǂݏo��
//---------------------------------------------------------------------
bool JlsScriptLimVar::isScpEnableAtNsc(TargetCatType tgcat, Nsc nsc){
	//--- �X�V���f ---
	int sizeScp    = pdata->sizeDataScp();
	int sizeEnable = sizeScpEnable(tgcat);
	if ( sizeScp != sizeEnable ){	// ��������Ίe����SC�ʒu�ŏ��ۊ�
		setScpEnableEveryNsc();
	}
	//--- �擾 ---
	if ( isErrorScpEnable(tgcat, nsc) ) return false;
	if ( tgcat == TargetCatType::Dst ) return (int) listScpEnableDst[nsc];
	if ( tgcat == TargetCatType::End ) return (int) listScpEnableEnd[nsc];
	return false;
}

//---------------------------------------------------------------------
// �e����SC�ʒu�ɂ����锻��L��������
//---------------------------------------------------------------------
void JlsScriptLimVar::clearScpEnable(){
	listScpEnableDst.clear();
	listScpEnableEnd.clear();
}
//---------------------------------------------------------------------
// �e����SC�ʒu�ɂ����锻��L���i�����V�[���`�F���W�ǉ��Ȃ���΃R�}���h���͍X�V�����g�p�\�j
//---------------------------------------------------------------------
void JlsScriptLimVar::setScpEnableEveryNsc(){
	//--- �S�����V�[���`�F���W�ʒu�Ń`�F�b�N ---
	int sizeScp    = pdata->sizeDataScp();
	clearScpEnable();
	for(int m=0; m<sizeScp; m++){
		int msecBase = pdata->getMsecScp(m);
		bool resultDst = isScpEnableAtMsec(msecBase, LOGO_EDGE_RISE, TargetCatType::Dst);
		bool resultEnd = isScpEnableAtMsec(msecBase, LOGO_EDGE_RISE, TargetCatType::End);
		listScpEnableDst.push_back(resultDst);
		listScpEnableEnd.push_back(resultEnd);
	}
}
//--- �V�[���`�F���W�ɑΉ�����f�[�^�� ---
int JlsScriptLimVar::sizeScpEnable(TargetCatType tgcat){
	if ( tgcat == TargetCatType::Dst ) return (int) listScpEnableDst.size();
	if ( tgcat == TargetCatType::End ) return (int) listScpEnableEnd.size();
	return 0;
}
//--- �G���[�`�F�b�N ---
bool JlsScriptLimVar::isErrorScpEnable(TargetCatType tgcat, Nsc nsc){
	if ( nsc < 0 || nsc >= sizeScpEnable(tgcat) ){
		string mes = "error:internal ScpEnable access nsc=" + to_string(nsc);
		mes += " size=" + to_string(sizeScpEnable(tgcat));
		lcerr << mes << endl;
		return true;
	}
	return false;
}

