//
// ���s�X�N���v�g�R�}���h�̃^�[�Q�b�g���i�邽�߂̕ϐ��ێ�
//
#pragma once

#include "JlsCmdArg.hpp"

//class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsDataset;

///////////////////////////////////////////////////////////////////////
//
// ��������ɂ��^�[�Q�b�g�I��N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScriptLimVar
{
private:
	struct ArgRange {					// �^�[�Q�b�g�ʒu�p
		WideMsec        wmsecFind;		// Dst�����͈�
		WideMsec        wmsecFrom;		// ��ʒu�i�J�n�^�I���ʒu�␳�܂ށj
	};

public:
	struct SeekVarRecord {	// �^�[�Q�b�g�ʒu�������̃f�[�^
		// �ݒ�
		RangeNsc rnscScp;		// ����SC�̌����͈�
		bool flagNoEdge;		// �S�̂̐擪�ƍŌ�̃t���[���͊܂߂Ȃ�
		bool flagNextTail;		// NextTail�R�}���h�p
		bool selectLogoRise;	// NextTail�R�}���h�Ń��S�����D��
		// ����
		TargetLocInfo tgDst;	// ���ʈʒu
		TargetLocInfo tgEnd;	// �I���ʒu
		int  numListDst;		// �������̒�����I�����ꂽ�ԍ�
		int  numListEnd;		// �������̒�����I�����ꂽ�ԍ�
		ScpPriorType statDst;	// �\���̗D�揇��
		ScpPriorType statEnd;	// �\���̗D�揇��
		Msec gapDst;			// ���S����̋���
		Msec gapEnd;			// ���S����̋���
		bool flagOnLogo;		// NextTail�R�}���h�̃��S����茟�o�p
	} seek;

public:
	void setPdata(JlsDataset *pdata);
	void clear();
	// �R�}���h���ʂ̐ݒ�
	void initVar(JlsCmdSet& cmdset);
	void setHeadTail(RangeMsec rmsec);
	void setFrameRange(RangeMsec rmsec);
	RangeMsec getHeadTail();
	RangeMsec getFrameRange();
public:
	// ���S�ʒu���X�g���̎w�胍�S�Ŋ���S�f�[�^���쐬
	void clearLogoBase();
	void setLogoBaseListNum(int n);
	void setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge);
	void setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge);
	void setLogoBsrcMsec(Msec msec);
	void setLogoBorgMsec(Msec msec);
	void setLogoBsrcEdge(LogoEdgeType edge);
	void setLogoBtgWmsecEdge(WideMsec wmsec, jlsd::LogoEdgeType edge);
	int  getLogoBaseListNum();
	Msec getLogoBsrcMsec();
	Msec getLogoBorgMsec();
	bool isLogoBaseExist();
	bool isLogoBaseNrf();
	Nrf  getLogoBaseNrf();
	Nsc  getLogoBaseNsc();
	LogoEdgeType    getLogoBaseEdge();
	LogoEdgeType    getLogoBsrcEdge();
	WideMsec        getLogoBtgWmsec();
	LogoEdgeType	getLogoBtgEdge();
	void getWidthLogoFromBase(WideMsec& wmsec, int step, bool flagWide);
	void getWidthLogoFromBaseForTarget(WideMsec& wmsec, int step, bool flagWide);
private:
	void getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide);
public:
	// Dst�͈͐ݒ�
	void clearRangeDst();
	void addRangeDst(WideMsec wmsecFind, WideMsec wmsecFrom);
	void selRangeDstNum(int num);
	int  sizeRangeDst();
	bool isRangeDstMultiFrom();
	WideMsec getRangeDstWide();
	Msec getRangeDstJust();
	Msec getRangeDstFrom();
	void getRangeDstFromForScp(Msec& msec, Msec& msbk, Nsc& nsc);
private:
	WideMsec getRangeDstItemWide(int num);
	WideMsec getRangeDstItemFromWide(int num);
	bool isErrorRangeDst(int num);
public:
	// �ʒu�����p�̐ݒ�
	void initSeekVar(JlsCmdSet& cmdset);
	bool isRangeToDst(Msec msecBsrc, Msec msecDst);
	bool isRangeToEnd(Msec msecDst, Msec msecEnd, WideMsec wmsecRange);
	bool isRangeToEndZone(Msec msecDst, Msec msecEnd);
	// �I���ʒu�̎��O����
	void clearPrepEnd();
	void addPrepEndRange(WideMsec wmsec);
	void setPrepEndRefer(bool flag);
	void setPrepEndAbs(bool fromAbs, bool multiBase, TargetLocInfo tgEnd);
	int  sizePrepEndRange();
	bool isPrepEndRangeExist();
	bool isPrepEndReferExist();
	bool isPrepEndFromAbs();
	bool isPrepEndMultiBase();
	TargetLocInfo getPrepEndAbs();
	WideMsec getPrepEndRangeWithOffset(int num, Msec msecOfs);
	Msec getPrepEndRangeForceLen();
	// Zone�͈�
private:
	void clearZone();
	bool isZoneAtDst(Msec msecBsrc, Msec msecDst);
	bool isZoneAtEnd(Msec msecDst, Msec msecEnd);
	bool isZoneBothEnds(Msec msecA, Msec msecB);
	bool isZoneInRange(Msec msecNow, RangeMsec rmsecZone);
	bool isZoneInForbid(Msec msecNow, RangeMsec rmsecZone);
	bool calcZoneRange(RangeMsec& rmsecZone, Msec msecB);
	bool calcZoneRangeSub(RangeMsec& rmsecZone, Msec msecTg, vector<Msec>& listMsec, bool flagL);
	bool calcZoneUnder(RangeMsec& rmsecZone, Msec msecB);
	bool calcZoneOver(RangeMsec& rmsecForbid, Msec msecB);
	void setZoneCache(Msec msecSrc, bool validRange, RangeMsec rmsecRange, RangeMsec rmsecForbid);
	bool getZoneCache(bool& validRange, RangeMsec& rmsecRange, RangeMsec& rmsecForbid, Msec msecSrc);
	bool isTgtLimitAllow(Msec msecTarget);
	int  setTgtLimit();
	// ������������
public:
	bool isScpEnableAtMsec(int msecBase, LogoEdgeType edge, TargetCatType tgcat);
private:
	bool isScpEnableAtMsecCheck(int msecBase, LogoEdgeType edge, int numOpt);
public:
	bool isScpEnableAtNsc(TargetCatType tgcat, Nsc nsc);
private:
	void clearScpEnable();
	void setScpEnableEveryNsc();
	int  sizeScpEnable(TargetCatType tgcat);
	bool isErrorScpEnable(TargetCatType tgcat, Nsc nsc);

public:
	JlsCmdArg           opt;		// �R�}���h�I�v�V�����ێ�
private:
	// �R�}���h���ʂ̐ݒ�
	RangeMsec           rmsecHeadTail;			// $HEADTIME/$TAILTIME����
	RangeMsec           rmsecFrameLimit;		// -F�I�v�V��������
	// ���S�ʒu���X�g���̎w�胍�S�Ŋ���S�f�[�^���쐬
	int             nBaseListNum;			// ��ʒu�̓��X�g���ŉ��Ԗڂ�
	bool            flagBaseNrf;			// ��ʒu�͎����S�g�p
	Nrf             nrfBase;				// ��ʒu�̎����S�ԍ�
	Nsc             nscBase;				// ��ʒu�̐����\�����S���������V�[���`�F���W�ԍ�
	LogoEdgeType	edgeBase;				// ��ʒu�̃G�b�W�I��
	Msec            msecBaseBsrc;			// ��ʒu�i�ύX���ʒu�j
	Msec            msecBaseBorg;			// ��ʒu�i�{���̊�ʒu�A�Ȃ���Ε␳�j
	LogoEdgeType	edgeBaseBsrc;				// ��ʒu�̃G�b�W�I��
	WideMsec        wmsecBaseBtg;			// �^�[�Q�b�g�͈͍쐬�p�̊�ʒu
	LogoEdgeType	edgeBaseBtg;			// �^�[�Q�b�g�͈͍쐬�p�̊�ʒu�̃G�b�W�I��
	// Dst�͈͐ݒ�
	vector<ArgRange>    listRangeDst;
	int                 numRangeDst;
	// END�ʒu���O�ݒ�
	vector<WideMsec>    listPrepEndRange;
	bool                existPrepEndRefer;
	bool                fromPrepEndAbs;
	bool                multiPrepEndBase;
	TargetLocInfo       tgPrepEndAbs;
	vector<Msec>        listPrepEndBaseMsec;
	vector<Msec>        listPrepEndBaseMsbk;
	// zone�p
	Msec                msecZoneSrc;		// Zone�L���b�V���ʒu
	bool                validZoneRange;		// Zone�L���b�V���͈͗L��
	RangeMsec           rmsecZoneRange;		// Zone�L���b�V���L���͈�
	RangeMsec           rmsecZoneForbid;	// Zone�L���b�V�������͈�
	vector<RangeMsec>   listTLRange;		// �Ώۈʒu�Ƃ��ċ�����͈̓��X�g(-TgtLimit)
	// ������������
	vector<bool>    listScpEnableDst;		// �����V�[���`�F���W�I��
	vector<bool>    listScpEnableEnd;		// �����V�[���`�F���W�I��

private:
	JlsDataset *pdata;									// ���̓f�[�^�A�N�Z�X
};
