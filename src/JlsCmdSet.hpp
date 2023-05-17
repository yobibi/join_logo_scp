//
// JL�X�N���v�g�p�R�}���h���e�i�[�f�[�^
//
#pragma once

#include "JlsCmdArg.hpp"

///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g�R�}���h�ݒ蔽�f�p
//
///////////////////////////////////////////////////////////////////////
class JlsCmdLimit
{
private:
	enum CmdProcessFlag {					// �ݒ��ԋL���p
		ARG_PROCESS_HEADTAIL    = 0x01,
		ARG_PROCESS_FRAMELIMIT  = 0x02,
		ARG_PROCESS_VALIDLOGO   = 0x04,
		ARG_PROCESS_BASELOGO    = 0x08,
		ARG_PROCESS_TARGETRANGE = 0x10,
		ARG_PROCESS_SCPENABLE   = 0x20,
		ARG_PROCESS_RESULT      = 0x40,
	};
	struct ArgLogoList {					// �L�����S���X�g�擾�p
		Msec            msec;
		LogoEdgeType    edge;
	};
	struct TargetLocInfoSet {				// ���ʃ��X�g�ۊǗp
		TargetLocInfo d;
		TargetLocInfo e;
	};

public:

	JlsCmdLimit();
	void			clear();
	RangeMsec		getHeadTail();
	Msec			getHead();
	Msec			getTail();
	bool			setHeadTail(RangeMsec rmsec);
	bool			setFrameRange(RangeMsec rmsec);
	RangeMsec		getFrameRange();
	// �L���ȃ��S�ԍ����X�g
	void            clearLogoList();
	bool            addLogoListStd(Msec msec, LogoEdgeType edge);
	void            addLogoListDirectDummy(bool flag);
	void            addLogoListDirect(Msec msec, LogoEdgeType edgeBase);
	void            attachLogoListOrg(int num, Msec msec, LogoEdgeType edge);
	Msec            getLogoListMsec(int nlist);
	Msec            getLogoListOrgMsec(int nlist);
	LogoEdgeType	getLogoListEdge(int nlist);
	LogoEdgeType	getLogoListOrgEdge(int nlist);
	int				sizeLogoList();
	bool            isLogoListDirect();
	void            forceLogoListStd(bool flag);
private:
	bool            isErrorLogoList(int nlist);
public:
	// �ΏۂƂ������S�I��
	void            clearLogoBase();
	bool			setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge);
	bool			setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge);
	bool            isLogoBaseExist();
	bool            isLogoBaseNrf();
	Nrf				getLogoBaseNrf();
	Nsc				getLogoBaseNsc();
	LogoEdgeType	getLogoBaseEdge();
	// �^�[�Q�b�g�I���\�͈�
	void            clearTargetData();
	bool            setTargetRange(WideMsec wmsece, bool fromLogo);
	WideMsec        getTargetRangeWide();
	bool            isTargetRangeFromLogo();
	// �^�[�Q�b�g�Ɉ�ԋ߂��ʒu
	void            setResultDst(TargetLocInfo tgIn);
	void            setResultEnd(TargetLocInfo tgIn);
private:
	void            setResultSubMake(TargetLocInfo& tgIn);
public:
	TargetLocInfo   getResultDst();
	TargetLocInfo   getResultEnd();
	Nsc             getResultDstNsc();
	LogoEdgeType	getResultDstEdge();
	TargetLocInfo   getResultDstCurrent();
	TargetLocInfo   getResultEndCurrent();
    void            clearPickList();
    void            addPickListCurrent();
    int             sizePickList();
    void            selectPickList(int num);
    bool            isPickListValid();
private:
    TargetLocInfo   getPickListDst();
    TargetLocInfo   getPickListEnd();

private:
	void			signalInternalError(CmdProcessFlag flags);
private:
	RangeMsec		rmsecHeadTail;			// $HEADTIME/$TAILTIME����
	RangeMsec		rmsecFrameLimit;		// -F�I�v�V��������
	// �L���ȃ��S�ԍ����X�g
	vector<ArgLogoList>  listLogoStd;		// �L�����S�ꗗ�i���S�ԍ��ɂ��ʒu�j
	vector<ArgLogoList>  listLogoDir;		// �L�����S�ꗗ�i���ڃt���[���w��ʒu�j
	vector<ArgLogoList>  listLogoOrg;		// �L�����S�ꗗ�i���ڃt���[���w��̖{�����S�ʒu�j
	bool            forceLogoStdFix;		// �L�����S�����S�ԍ��ɂ��ʒu�ɋ����I��
	bool            existLogoDirDmy;		// ���ڃt���[���w��i�����������ʒu�j���ݗL��
	// ����S
	bool            flagBaseNrf;			// ��ʒu�͎����S�g�p
	Nrf				nrfBase;				// ��ʒu�̎����S�ԍ�
	Nsc				nscBase;				// ��ʒu�̐����\�����S���������V�[���`�F���W�ԍ�
	LogoEdgeType	edgeBase;				// ��ʒu�̃G�b�W�I��
	// �^�[�Q�b�g�I���\�͈�
	WideMsec		wmsecTarget;			// �Ώۈʒu�͈�
	bool			fromLogo;				// ���S��񂩂�̑Ώۈʒu�͈�
	// �^�[�Q�b�g�Ɉ�ԋ߂��ʒu
	TargetLocInfo   targetLocDst;			// �Ώۈʒu���
	TargetLocInfo   targetLocEnd;			// End�ʒu���
	// -pick����
	int             numPickList;			// ���ʕۊǃ��X�g�̌��ݑI��ԍ�
	vector<TargetLocInfoSet>  listPickResult;	// ���ʕۊǃ��X�g

	int				process;				// �ݒ��ԕێ�
};



///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g�R�}���h�S��
//
///////////////////////////////////////////////////////////////////////
class JlsCmdSet
{
public:
	JlsCmdArg		arg;			// �ݒ�l
	JlsCmdLimit		limit;			// �ݒ蔽�f
};
