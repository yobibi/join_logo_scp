//
// join_logo_scp �f�[�^�i�[�N���X
//
//
// �ϐ������ʖ�����
//   nlg      : ���S�ԍ��i�����オ��^����������ʁj
//   nrf      : ���S�ԍ��i�����オ��^����������ʎZ�j
//   nsc      : �V�[���`�F���W�ԍ�
//   msec,ms* : ���ԃ~���b�P��
//   sec      : ���ԕb�P��
//   frm      : ���ԃt���[���P��
//
//
/////////////////////////////////////////////////////////////////////////
#pragma once

#include "CnvStrTime.hpp"


//--- �V�[���`�F���W�ƍ\����؂�p ---
struct DataScpRecord {
// �Ǎ��f�[�^
	Msec			msec;				// �~���b
	Msec			msbk;				// �I���ʒu�~���b
	Msec			msmute_s;			// �V�[���`�F���W�������o�J�n�ʒu
	Msec			msmute_e;			// �V�[���`�F���W�������o�I���ʒu
	bool			still;				// �Î~��
// �]���ݒ�p
	ScpPriorType	statpos;			// -1:�d���� 0:���� 1:��� 2:�m��
// �����ݒ�p
	int				score;				// �\����؂���̐����p
	ScpChapType		chap;				// �\����؂���
	ScpArType		arstat;				// ��؂�̍\�����e
	ScpArExtType	arext;				// ��؂�̍\�����e�̊g���ݒ�
};

//--- ���S�f�[�^�ێ��p ---
struct DataLogoRecord {
// �Ǎ��f�[�^
	Msec			rise;				// ���S�e�J�n�t���[���i�~���b�j
	Msec			fall;				// ���S�e�I���t���[���i�~���b�j
	Msec			rise_l;				// ���S�e�J�n�t���[�����J�n�i�~���b�j
	Msec			rise_r;				// ���S�e�J�n�t���[�����I���i�~���b�j
	Msec			fall_l;				// ���S�e�I���t���[�����J�n�i�~���b�j
	Msec			fall_r;				// ���S�e�I���t���[�����I���i�~���b�j
	int				fade_rise;			// �e�J�n�t�F�[�h�C�����(0 or fadein)
	int				fade_fall;			// �e�I���t�F�[�h�A�E�g���(0 or fadeout)
	int				intl_rise;			// �C���^�[���[�X���(0:ALL 1:TOP 2:BTM)
	int				intl_fall;			// �C���^�[���[�X���(0:ALL 1:TOP 2:BTM)
// �ǂݍ��݌��f�[�^�ێ�
	Msec			org_rise;			// ���S�e�J�n�t���[���i�~���b�j
	Msec			org_fall;			// ���S�e�I���t���[���i�~���b�j
	Msec			org_rise_l;			// ���S�e�J�n�t���[�����J�n�i�~���b�j
	Msec			org_rise_r;			// ���S�e�J�n�t���[�����I���i�~���b�j
	Msec			org_fall_l;			// ���S�e�I���t���[�����J�n�i�~���b�j
	Msec			org_fall_r;			// ���S�e�I���t���[�����I���i�~���b�j
// �]���ݒ蒲���p
	LogoPriorType	stat_rise;			// -1:�͂���m�� 0:���� 1:��� 2:�m��
	LogoPriorType	stat_fall;			// -1:�͂���m�� 0:���� 1:��� 2:�m��
	LogoUnitType	unit_rise;			// 1:�Ɨ��t���[��
	LogoUnitType	unit_fall;			// 1:�Ɨ��t���[��
// �]�����ʊi�[�p
	LogoResultType	outtype_rise;		// 0:�o�͖��m��  1:�o�͊m��  2:abort�j���m��
	LogoResultType	outtype_fall;		// 0:�o�͖��m��  1:�o�͊m��  2:abort�j���m��
	Msec			result_rise;		// ���S�e�J�n�m��ʒu�i�~���b�j
	Msec			result_fall;		// ���S�e�I���m��ʒu�i�~���b�j
};

/////////////////////////////////////////////////////////////////////////

//--- �f�[�^�i�[�p ---
class JlsDataset
{
private:
	static const int MAXSIZE_LOGO  = 1024;
	static const int MAXSIZE_SCP   = 4096;

	//--- �����O�����̓I�v�V���� ---
	struct DtExtOptRecord {
		// �I�v�V����
		int		verbose;				// �ڍו\���p
		int		frmLastcut;				// �I�v�V���� -lastcut
		int		msecCutIn;				// �I�v�V���� -CutMrgIn
		int		msecCutOut;				// �I�v�V���� -CutMrgOut
		int		wideCutIn;				// CutMrgIn�̕��I��  0:�Œ� 1:0�n�_��2�_
		int		wideCutOut;				// CutMrgOut�̕��I�� 0:�Œ� 1:0�n�_��2�_
		// �������ݗL��
		int		fixCutIn;				// 0:CutMrgIn�w��Ȃ� 1:CutMrgIn�w�肠��
		int		fixCutOut;				// 0:CutMrgOut�w��Ȃ� 1:CutMrgOut�w�肠��
		int		fixWidCutI;				// 0:widecut�w��Ȃ� 1:widecut�w�肠��
		int		fixWidCutO;				// 0:widecut�w��Ȃ� 1:widecut�w�肠��
		// �����ێ��p�����[�^
		int		flagNoLogo;				// 0:�ʏ� 1:���S��ǂݍ��܂Ȃ��ꍇ
		int		oldAdjust;				// 0:�����������Ȃ� 1:��������������
		// �ǉ�
		int     fixVLine;				// 0:vLine�w��Ȃ�  1:vLine�w�肠��
		int     fixFDirect;				// 0:�w��Ȃ�  1:�w�肠��  flagDirect
		int     fixNLgExact;			// 0:�w��Ȃ�  1:�w�肠��  nLgExact
		int     fixNSysCode;			// 0:�w��Ȃ�  1:�w�肠��  nSysCode
		int     fixSubList;				// 0:subDir�w��Ȃ� 1:subDir�w�肠��
		int     fixSubPath;				// 0:subPath�w��Ȃ� 1:subPath�w�肠��
		int     fixSetup;				// 0:setup�w��Ȃ� 1:setup�w�肠��
		int     vLine;					// �ǂݍ��ݍs�\���p
		int     flagDirect;				// 0:�ʏ�̌��o���S 1:���S�ʒu�𒼐ڎw��
		int     nLgExact;				// 0:�ʏ� 1:���S�␳�ŏ���
		int     nSysCode;				// �W���o��/�G���[�̕����R�[�h�ԍ� 1:STD 2:UTF8 3:UTF16
		int     dispSysMes;				// ���O�\�� 0:�Ȃ� 1:CurMrgIn/Out 2:OUTDIRECT 4:CallFile
		string  subList;				// �T�u�t�H���_���X�g�w��
		string  subPath;				// �T�u�t�H���_�p�X�w��
		string  setup;					// ���ʐ擪���s�t�@�C��
		// ���O�ۊǂ̂�
		string  logofile;
		string  scpfile;
		string  cmdfile;
		string  outfile;
		string  outscpfile;
		string  outdivfile;
	};
	//--- �e�s���s���̕ێ��p�����[�^ ---
	struct RecordHoldFromCmd {			// �R�}���h�Őݒ肳���l�Ŏ���������l
		Msec	msecSelect1st;			// �ŏ��̊J�n�ʒu���iSelect�g�p���̂݁j
		Msec	msecTrPoint;			// CutTR�R�}���h�̐ݒ�ʒu�iCM�\�����������̈ʒu���f�p�j
		RangeMsec	rmsecHeadTail;		// $HEADTIME,$TAILTIME����
		int     typeRange;				// GetPos/GetList��HEADTIME/TAILTIME����(1=�t��)
	};
	//--- ���S���Z�b�g�p�o�b�N�A�b�v�ŕۊǂ���f�[�^ ---
	struct RecordBackupLogo {
		vector<DataScpRecord>   bak_scp;
		vector<DataLogoRecord>  bak_logo;
		int                     bak_msecTotalMax;	// �ő�t���[������
		DtExtOptRecord          bak_extOpt;
	};

public:
	//--- �\����� ---
	enum class SysMesType{
		CutMrg,
		OutDirect,
		CallFile,
	};

public:
// �����ݒ�
	JlsDataset();
	void initData();
	void clearDataLogoAll();
// ���S�E�V�[���`�F���W�f�[�^�̕ۑ��E�ǂݏo��
	void backupLogosetSave();
	void backupLogosetLoad();
// ����ݒ�̕ۑ��E�ǂݏo��
	void setConfig(ConfigVarType tp, int val);
	int  getConfig(ConfigVarType tp);
	int  getConfigAction(ConfigActType acttp);
// �f�[�^�T�C�Y�擾
	int  sizeDataLogo();
	int  sizeDataScp();
	bool emptyDataLogo();
	RangeNsc getRangeNscTotal(bool flagNoEdge);
// �P�f�[�^�Z�b�g�P�ʂ̏���
	void clearRecordLogo(DataLogoRecord &dt);
	void clearRecordScp(DataScpRecord &dt);
	void pushRecordLogo(DataLogoRecord &dt);
	void pushRecordScp(DataScpRecord &dt);
	void popRecordLogo();
	void insertRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void insertRecordScp(DataScpRecord &dt, Nsc nsc);
	void deleteRecordScp(Nsc nsc);
	void getRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void setRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void getRecordScp(DataScpRecord &dt, Nsc nsc);
	void setRecordScp(DataScpRecord &dt, Nsc nsc);
// �P�v�f�P�ʂ̏���
	Msec getMsecLogoRise(Nlg nlg);
	Msec getMsecLogoFall(Nlg nlg);
	Msec getMsecLogoNrf(Nrf nrf);
	void getMsecLogoNrfWide(int &msec_c, int &msec_l, int &msec_r, Nrf nrf);
	void getWideMsecLogoNrf(WideMsec &wmsec, Nrf nrf);
	void setMsecLogoNrf(Nrf nrf, Msec val);
	Msec getMsecScp(Nsc nsc);
	Msec getMsecScpBk(Nsc nsc);
	Msec getMsecScpEdge(Nsc nsc, LogoEdgeType edge);
	WideMsec  getWideMsecScp(Nsc nsc);
	RangeMsec getRangeMsecFromRangeNsc(RangeNsc rnsc);
	bool				getScpStill(Nsc nsc);
	jlsd::ScpPriorType	getScpStatpos(Nsc nsc);
	int					getScpScore(Nsc nsc);
	jlsd::ScpChapType	getScpChap(Nsc nsc);
	jlsd::ScpArType		getScpArstat(Nsc nsc);
	jlsd::ScpArExtType	getScpArext(Nsc nsc);
	bool				getResultLogoAtNrf(Msec &msec, LogoResultType &outtype, Nrf nrf);
	void				setScpStatpos(Nsc nsc, ScpPriorType val);
	void				setScpScore(Nsc nsc, int val);
	void				setScpChap(Nsc nsc, ScpChapType val);
	void				setScpArstat(Nsc nsc, ScpArType val);
	void				setScpArext(Nsc nsc, ScpArExtType val);
	void				setResultLogoAtNrf(Msec msec, LogoResultType outtype, Nrf nrf);
// �D��x�擾
	jlsd::LogoPriorType	getPriorLogo(Nrf nrf);
	jlsd::ScpPriorType	getPriorScp(Nsc nsc);
// �O��f�[�^�擾�����i���S�j
	Nrf  getNrfDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type);
	Nrf  getNrfPrevLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type);
	Nrf  getNrfNextLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type);
	bool getNrfNextLogoSet(Nrf &nrf_rise, Nrf &nrf_fall, LogoSelectType type);
	Nsc  getNscDirElgForAll(Nsc nsc, SearchDirType dr, LogoEdgeType edge, bool flag_border, bool flag_out);
	Nsc  getNscPrevElg(Nsc nsc, LogoEdgeType edge);
	Nsc  getNscNextElg(Nsc nsc, LogoEdgeType edge);
	bool getNrfptNext(NrfCurrent &logopt, LogoSelectType type);
	bool getElgptNext(ElgCurrent &elg);
	Nlg  getResultLogoNext(Msec &msec_rise, Msec &msec_fall, bool &cont_next, Nlg nlg);
	bool getNrfptOutNext(NrfCurrent &logopt, LogoSelectType type, bool final);
private:
	Nrf  getNrfMsecOutNextLogo(Msec& msecOut, Nrf nrf, LogoEdgeType edge, LogoSelectType type, bool final);
	Nrf  getNrfOutDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type, bool final);
public:
// �O��f�[�^�擾�����i�����V�[���`�F���W�j
	Nsc  getNscDirScpChap(Nsc nsc, SearchDirType dr, ScpChapType chap_th);
	Nsc  getNscPrevScpChap(Nsc nsc, ScpChapType chap_th);
	Nsc  getNscPrevScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscNextScpChap(Nsc nsc, ScpChapType chap_th);
	Nsc  getNscNextScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscNextScpCheckCmUnit(Nsc nsc, ScpEndType noedge);
	Nsc  getNscPrevScpCheckCmUnit(Nsc nsc, ScpEndType noedge);
	Nsc  getNscDirScpDecide(Nsc nsc, SearchDirType dr, ScpEndType noedge);
	Nsc  getNscPrevScpDecide(Nsc nsc, ScpEndType noedge);
	Nsc  getNscNextScpDecide(Nsc nsc, ScpEndType noedge);
	Nsc  getNscNextScpDisp(Nsc nsc, ScpEndType noedge);
	Nsc  getNscPrevScpDisp(Nsc nsc, ScpEndType noedge);
	Nsc  getNscPrevScpDispFromMsecCount(Msec msec, int nCount, bool clip);
	Nsc  getNscNextScpDispFromMsecCount(Msec msec, int nCount, bool clip);
// �ʒu�ɑΉ�����f�[�^�擾����
	Nrf  getNrfLogoFromMsec(Msec msec_target, LogoEdgeType edge);
	Nrf  getNrfLogoFromMsecResult(Msec msec_target, LogoEdgeType edge, bool result);
	Nsc  getNscFromNrf(Nrf nrf_target, Msec msec_th, ScpChapType chap_th, bool flat=false);
	Nsc  getNscFromMsecFull(Msec msec_target, Msec msec_th, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscFromMsecChap(Msec msec_target, Msec msec_th, ScpChapType chap_th);
	Nsc  getNscFromMsecMgn(Msec msec_target, Msec msec_th, ScpEndType noedge);
	Nsc  getNscFromMsecAll(Msec msec_target);
	Nsc  getNscFromMsecAllEdgein(Msec msec_target);
	Nsc  getNscFromWideMsecFull(WideMsec wmsec_target, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscFromWideMsecByChap(WideMsec wmsec_target, ScpChapType chap_th);
	bool getRangeNscFromRangeMsec(RangeNsc &rnsc, RangeMsec rmsec);
	Nsc  getNscFromMsecDisp(Msec msec_target, Msec msec_th, ScpEndType noedge);
	Nsc  getNscFromWideMsecDisp(WideMsec wmsec_target, ScpEndType noedge);
// ��Ԑݒ�
	void setLevelUseLogo(int level);
	int  getLevelUseLogo();
	void setFlagSetupAdj(bool flag);
	void setFlagAutoMode(bool flag);
// ��Ԕ���
	bool isExistLogo();
	bool isUnuseLogo();
	bool isUnuseLevelLogo();
	bool isSetupAdjInitial();
	bool isAutoModeInitial();
	bool isAutoModeUse();
	bool isAutoLogoOnly();
	bool isValidLogoRise(Nlg nlg);
	bool isValidLogoFall(Nlg nlg);
	bool isValidLogoNrf(Nrf nrf);
	bool isElgDivScpForAll(Nsc nsc, bool flag_border, bool flag_out);
	bool isElgInScp(int nsc);
	bool isElgInScpForAll(Nsc nsc, bool flag_border, bool flag_out);
	bool isScpChapTypeDecideFromNsc(Nsc nsc);
	bool isStillFromMsec(Msec msec_target);
	bool isSmuteFromMsec(Msec msec_target);
	bool isSmuteSameArea(Nsc nsc1, Nsc nsc2);
	bool limitWideMsecFromRange(WideMsec& wmsec, RangeMsec rmsec);
// Term�\������
	void setTermEndtype(Term &term, ScpEndType endtype);
	void setTermForDisp(Term &term, bool flag);
	bool getTermNext(Term &term);
	bool getTermPrev(Term &term);
	jlsd::ScpArType		getScpArstat(Term term);
	jlsd::ScpArExtType	getScpArext(Term term);
	void setScpArstat(Term term, ScpArType arstat);
	void setScpArext(Term term, ScpArExtType arext);
	bool isScpArstatCmUnit(Term term);
	bool checkScopeTerm(Term term, RangeMsec scope);
	bool checkScopeRange(RangeMsec bounds, RangeMsec scope);
// �J�X�^�����S�쐬
	void trialClogo(vector<WideMsec>& listWmsec, LogoCustomType custom);
	void makeClogo(LogoCustomType custom);
private:
	void makeClogoMain(vector<WideMsec>& listWmsec, LogoCustomType& custom);
	void makeClogoFromReal(vector<WideMsec>& listWmsec, LogoCustomType custom);
	void makeClogoFromVirtual(vector<WideMsec>& listWmsec, LogoCustomType custom);
public:
	LogoCustomType getClogoCustom();
	vector<WideMsec> getClogoList();
	int  sizeClogoList();
	int  getClogoNumNear(Msec msecLogo, LogoEdgeType edge);
	int  getClogoNumPrev(Msec msecLogo, LogoEdgeType edge);
	int  getClogoNumNext(Msec msecLogo, LogoEdgeType edge);
	int  getClogoNumPrevCount(Msec msecLogo, int nCount);
	int  getClogoNumNextCount(Msec msecLogo, int nCount);
	WideMsec getClogoWmsecFromNum(int num);
	Msec getClogoMsecFromNum(int num);
	Msec getClogoMsecNear(Msec msecLogo, LogoEdgeType edge);
	Msec getClogoMsecPrev(Msec msecLogo, LogoEdgeType edge);
	Msec getClogoMsecNext(Msec msecLogo, LogoEdgeType edge);
	bool isClogoMsecExist(Msec msecLogo, LogoEdgeType edge);
	bool isClogoReal();
	Nrf  getClogoRealNrf(Msec msecLogo, LogoEdgeType edge);
	Nsc  getClogoNsc(Msec msecLogo);
	Msec getClogoMsecMgn();
// �f�[�^�}��
	Nsc  insertDivLogo(Msec msec_target, bool confirm, bool unit, LogoEdgeType edge);
	Nsc  insertLogo(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit);
	Nsc  insertLogoEdge(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit, LogoEdgeType edge);
	Nsc  insertScpos(Msec msec_dst_s, Msec msec_dst_bk, Nsc nsc_mute, ScpPriorType stat_scpos_dst);
	bool restructScp();
	Nsc  getNscForceMsecExact(Msec msec_in, LogoEdgeType edge, bool exact);
	Nsc  getNscForceMsec(Msec msec_in, LogoEdgeType edge);
	Nsc  getNscForceMsecOrg(Msec msec_in, LogoEdgeType edge);
	Nsc  getNscForceExactFixMsec(Msec msec_in, LogoEdgeType edge);
// �����\�����̕ύX
	void changeLogoOnOff(Nsc nsc, bool logoOn);
	void changeChapDispUnit(Nsc nscFrom, Nsc nscTo, bool cutDivUnit);
	void changeChapDispUnitWithSide(Nsc nscFrom, Nsc nscTo);
	void changeChapDispUnitSub(Nsc nscFrom, Nsc nscTo, bool side);
	bool isCmLenNscToNsc(Nsc nscFrom, Nsc nscTo);
// �����\�����x������
	bool isLabelLogoFromNsc(Nsc nsc, bool flagBorder, bool flagOut);
	ComLabelType getLabelTypeFromNsc(Nsc nsc, bool flagOut);
private:
	bool isLabelLogo(ComLabelType label, bool flagBorder, bool flagOut);
	ComLabelType getLabelTypeFromStat(ScpArType arstat, ScpArExtType arext, bool flagOut);
	string getLabelStr(ComLabelType label);
public:
// �\�����̃��S�\�����Ԃ̎擾
	Sec  getSecLogoComponent(Msec msec_s, Msec msec_e);
	Sec  getSecLogoComponentFromLogo(Msec msec_s, Msec msec_e);
// Trim�ʒu���ڐݒ�
	void setOutDirect(vector<Msec>& listMsec);
// �o�͍쐬
	void outputResultTrimGen();
	void outputResultDetailReset();
	bool outputResultDetailGetLine(string &strBuf);
	void dispSysMesN(const string& msg, SysMesType typeMsg);
	void displayLogo();
	void displayScp();
	void setMsecTotalMax(int msec){ m_msecTotalMax = msec; };
	int  getMsecTotalMax(){ return m_msecTotalMax; };
	bool isRangeInTotalMax(Msec ms){ return (ms >= 0 && ms <= getMsecTotalMax()); };

	CnvStrTime		cnv;				// �ϊ�����
	JlsDataset		*pdata;				// �������g�ւ̃|�C���^

private:
	int getSecLogoComponentFromElg(int msec_s, int msec_e);
	void outputResultTrimGenManual();
	void outputResultTrimGenAuto();
	void outputResultDetailGetLineLabel(string &strBuf, ScpArType arstat, ScpArExtType arext);

public:
	// �Œ���Ԑݒ�
	int		msecValExact;		// 100ms
	int		msecValNear1;		// 180ms
	int		msecValNear2;		// 350ms
	int		msecValNear3;		// 1200ms
	int		msecValLap1;		// 700ms   �d���������ԁi����SC�Ԋu�j
	int		msecValLap2;		// 2500ms  �d���������ԁi�\���j
	int		msecValSpc;			// 1200ms  ���S�����p

	// �ݒ�l
	DtExtOptRecord				extOpt;			// �O���ݒ�I�v�V����
	RecordHoldFromCmd 			recHold;		// �R�}���h���猈�肳���ݒ�l
	vector<int>					resultTrim;		// �o�͌��ʁi�J�b�g�ʒu�j
	vector<int>         divFile;    // �t�@�C�������ʒu

private:
	// �i�[�f�[�^
	vector<DataScpRecord>		m_scp;
	vector<DataLogoRecord>		m_logo;
	int							m_config[SIZE_CONFIG_VAR];
	int							m_msecTotalMax;			// �ő�t���[������
	LogoCustomType              m_customLogo;		// �J�X�^�����S�ݒ�
	vector<WideMsec>            m_listClogo;		// �J�X�^�����S�ʒu�i�[
	// �Z�b�g�A�b�v��Ԋi�[
	int		m_levelUseLogo;
	bool	m_flagSetupAdj;
	int		m_flagSetupAuto;
	// Trim�ʒu���ڐݒ�
	vector<Msec>  m_listOutDirect;
	// ���ʏo�͗p
	int		m_nscOutDetail;
	// �o�b�N�A�b�v�ۊǃf�[�^
	RecordBackupLogo m_backupData;
};
