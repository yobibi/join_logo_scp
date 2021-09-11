//
// JL�X�N���v�g�p�R�}���h���e�i�[�f�[�^
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g�R�}���h�ݒ�l
//
///////////////////////////////////////////////////////////////////////
class JlsCmdArg
{
private:
	struct CmdArgTack {						// �ݒ���e�g�ݍ��킹���猈�肳�����s�p�ݒ�
		bool      floatBase;				// 0:���S�ʒu�  1:���ʈʒu�
		bool      virtualLogo;				// 0:���ۂ̃��S  1:�������S�������S
		bool      ignoreComp;				// 0:�ʏ�  1:���S�m���Ԃł����s
		bool      limitByLogo;				// 0:�ʏ�  1:�אڃ��S�܂łɐ���
		bool      onePoint;					// 0:�ʏ�  1:���S�P�ӏ��ɐ���
		bool      needAuto;					// 0:�ʏ�  1:Auto�\���K�v
		bool      fullFrame;				// 0:�ʏ�  1:-F�n����`���͏�ɑS��
		LazyType  typeLazy;					// �x�����s�ݒ���
		bool      ignoreAbort;				// 0:�ʏ�  1:���SAbort��Ԃł����s
	};
	struct CmdArgCond {						// ��͎��̏��
		int       numCheckCond;				// �������̊m�F�ʒu�i0=�s�v�A1-=�m�F��������ʒu�j
		bool      flagCond;					// IF���p�̏������f
	};
	struct CmdArgSc {					// -SC�n�̃I�v�V�����f�[�^
		OptType   type;
		bool      relative;		// false=�ʏ�Atrue=R�t��������I�v�V����
		Msec      min;
		Msec      max;
	};

	//--- �f�[�^�ۊǗp�T�C�Y ---
	static const int SIZE_JLOPT_OPTNUM = static_cast<int>(OptType::ArrayMAX) - static_cast<int>(OptType::ArrayMIN) - 1;
	static const int SIZE_JLOPT_OPTSTR = static_cast<int>(OptType::StrMAX) - static_cast<int>(OptType::StrMIN) - 1;

public:
	JlsCmdArg();
	void	clear();
// ��ʃI�v�V�����p
	void   setOpt(OptType tp, int val);
	void   setOptDefault(OptType tp, int val);
	int    getOpt(OptType tp);
	bool   isSetOpt(OptType tp);
// ������I�v�V�����p
	void   setStrOpt(OptType tp, const string& str);
	void   setStrOptDefault(OptType tp, const string& str);
	string getStrOpt(OptType tp);
	bool   isSetStrOpt(OptType tp);
	void   clearStrOptUpdate(OptType tp);
	bool   isUpdateStrOpt(OptType tp);
	bool   getOptCategory(OptCat& category, OptType tp);
private:
	bool   getRangeOptArray(int& num, OptType tp);
	bool   getRangeStrOpt(int& num, OptType tp);
public:
// -SC�n�R�}���h�p
	void    addScOpt(OptType tp, bool relative, int tmin, int tmax);
	OptType getScOptType(int num);
	bool	isScOptRelative(int num);
	Msec	getScOptMin(int num);
	Msec	getScOptMax(int num);
	int		sizeScOpt();
// -LG�n�R�}���h�p
	void	addLgOpt(string strNlg);
	string	getLgOpt(int num);
	int		sizeLgOpt();
// �����擾
	void   addArgString(const string& strArg);
	bool   replaceArgString(int n, const string& strArg);
	string getStrArg(int n);
	int    getValStrArg(int n);
// IF�������p
	void setNumCheckCond(int num);
	int  getNumCheckCond();
	void setCondFlag(bool flag);
	bool getCondFlag();

public:
// �R�}���h
	CmdType             cmdsel;				// �R�}���h�I��
	CmdCat              category;			// ���s���̃R�}���h���
	WideMsec			wmsecDst;			// �ΏۑI��͈�
	LogoEdgeType		selectEdge;			// S/E/B
	CmdTrSpEcID         selectAutoSub;			// TR/SP/EC
// �������
	CmdArgTack			tack;				// �ݒ���e�g�ݍ��킹���猈�肳�����s�p�ݒ�
private:
	CmdArgCond			cond;				// ��͎��̏��

private:
// ��ʃI�v�V�����ۑ�
	int					optdata[SIZE_JLOPT_OPTNUM];
	bool				flagset[SIZE_JLOPT_OPTNUM];
	string              optStrData[SIZE_JLOPT_OPTSTR];
	bool                flagStrSet[SIZE_JLOPT_OPTSTR];
	bool                flagStrUpdate[SIZE_JLOPT_OPTSTR];
// ���X�g�ۑ�
	vector<string>		listStrArg;	// ����������
	vector<CmdArgSc>	listScOpt;	// -SC�n�I�v�V�����ێ�
	vector<string>		listLgVal;	// ���S�ԍ����ۑ�
	vector<Msec>		listTLOpt;	// -TL�I�v�V�����ێ�
};



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
	struct ArgValidLogo {					// �L�����S���X�g�擾�p
		Msec			msec;
		LogoEdgeType	edge;
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
	Msec			getLogoListMsec(int nlist);
	LogoEdgeType	getLogoListEdge(int nlist);
	bool			addLogoList(Msec &rmsec, jlsd::LogoEdgeType edge);
	int				sizeLogoList();
	Nrf				getLogoBaseNrf();
	Nsc				getLogoBaseNsc();
	LogoEdgeType	getLogoBaseEdge();
	bool			setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge);
	bool			setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge);
	void			setLogoWmsecList(vector<WideMsec>& listWmsec, int locBase);
	void			getLogoWmsecBase(WideMsec& wmsec, int step, bool flagWide);
	void			getLogoWmsecBaseShift(WideMsec& wmsec, int step, bool flagWide, int sft);
	WideMsec		getTargetRangeWide();
	Msec			getTargetRangeForce();
	bool			isTargetRangeLogo();
	bool			setTargetRange(WideMsec wmsec, Msec msec_force, bool from_logo);
	void			setTargetOutEdge(LogoEdgeType edge);
	LogoEdgeType	getTargetOutEdge();
	bool			isTargetListed(Msec msec_target);
	void			clearTargetList();
	void			addTargetList(RangeMsec rmsec);
	bool			getScpEnable(Nsc nsc);
	bool			setScpEnable(vector<bool> &listEnable);
	int 			sizeScpEnable();
	Nsc				getResultTargetSel();
	Nsc				getResultTargetEnd();
	bool			setResultTarget(Nsc nscSelIn, Nsc nscEndIn);

private:
	void			signalInternalError(CmdProcessFlag flags);

private:
	RangeMsec		rmsecHeadTail;			// $HEADTIME/$TAILTIME����
	RangeMsec		rmsecFrameLimit;		// -F�I�v�V��������
	vector<ArgValidLogo>	listValidLogo;	// �L�����S�ʒu�ꗗ
	Nrf				nrfBase;				// ��ʒu�̎����S�ԍ�
	Nsc				nscBase;				// ��ʒu�̐����\�����S���������V�[���`�F���W�ԍ�
	LogoEdgeType	edgeBase;				// ��ʒu�̃G�b�W�I��
	vector<WideMsec>	listWmsecLogoBase;	// ���S�ʒu��񃊃X�g
	int				locMsecLogoBase;		// ���S�ʒu��񃊃X�g�̊�ʒu�Ή��ԍ�
	WideMsec		wmsecTarget;			// �Ώۈʒu�͈�
	Msec			msecTargetFc;			// �����ݒ�p�Ώۈʒu
	bool			fromLogo;				// ���S��񂩂�̑Ώۈʒu�͈�
	vector<RangeMsec>	listTLRange;		// �Ώۈʒu�Ƃ��ċ�����͈̓��X�g
	vector<bool>	listScpEnable;			// �����V�[���`�F���W�I��
	Nsc				nscSel;					// �Ώۈʒu�����V�[���`�F���W�ԍ�
	Nsc				nscEnd;					// -endlen�ɑΉ����閳���V�[���`�F���W�ԍ�
	LogoEdgeType    edgeOutPos;				// �ʒu�o�͎��̃G�b�W�I��

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
