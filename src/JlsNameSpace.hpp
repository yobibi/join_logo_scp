//
// join_logo_scp �f�[�^�p��`
//
#pragma once

//---------------------------------------------------------------------
// �f�[�^�i�[�p
//---------------------------------------------------------------------
namespace jlsd
{
	using Msec = int;
	using Sec = int;
	using Nsc = int;
	using Nrf = int;
	using Nlg = int;

	//--- Auto�\����؂�(scp.chap) ---
	enum ScpChapType {
		SCP_CHAP_DUPE = -1,		// �d���ӏ�
		SCP_CHAP_NONE,			// �������
		SCP_CHAP_CPOSIT,		// 5�b�P�ʉ\���ێ�
		SCP_CHAP_CPOSQ,			// 15�b�P�ʉ\���ێ�
		SCP_CHAP_CDET,			// 15�b�P�ʕێ�
		SCP_CHAP_DINT,			// �\�����������b�P�ʋ�؂�
		SCP_CHAP_DBORDER,		// �����V�[���`�F���W�Ȃ���؂�
		SCP_CHAP_DFIX,			// �\����؂�
		SCP_CHAP_DFORCE,		// ������؂�ݒ�ӏ�
		SCP_CHAP_DUNIT			// Trim��������؂�
	};
	static const ScpChapType SCP_CHAP_DECIDE = SCP_CHAP_DINT;		// �m��ӏ�臒l
	inline bool isScpChapTypeDecide(ScpChapType type){
		return (type >= SCP_CHAP_DECIDE)? true : false;
	}

	//--- Auto�\��������{���e(scp.arstat) ---
	enum ScpArType {
		SCP_AR_UNKNOWN,			// �s��
		SCP_AR_L_UNIT,			// ���S�L �P�T�b�P��
		SCP_AR_L_OTHER,			// ���S�L ���̑�
		SCP_AR_L_MIXED,			// ���S�L ���S��������
		SCP_AR_N_UNIT,			// ���S�� �P�T�b�P��
		SCP_AR_N_OTHER,			// ���S�� ���̑�
		SCP_AR_N_AUNIT,			// ���S�� �����łP�T�b�̒��Ԓn�_
		SCP_AR_N_BUNIT,			// ���S�� �����łP�T�b�̒[
		SCP_AR_B_UNIT,			// ���S���E �P�T�b�P��
		SCP_AR_B_OTHER			// ���S���E ���̑�
	};
	static const int SCP_ARR_L_LOW  = SCP_AR_L_UNIT;		// ���S�L�̉����l
	static const int SCP_ARR_L_HIGH = SCP_AR_L_MIXED;		// ���S�L�̏���l
	inline bool isScpArTypeLogo(ScpArType type){
		return (type >= SCP_ARR_L_LOW && type <= SCP_ARR_L_HIGH)? true : false;
	}
	inline bool isScpArTypeBorder(ScpArType type){
		return (type >= SCP_AR_B_UNIT && type <= SCP_AR_B_OTHER)? true : false;
	}
	inline bool isScpArTypeLogoBorder(ScpArType type){
		return ((type >= SCP_ARR_L_LOW && type <= SCP_ARR_L_HIGH) ||
				(type >= SCP_AR_B_UNIT && type <= SCP_AR_B_OTHER))? true : false;
	}
	//--- Auto�\�������g�����e(scp.arext) ---
	enum ScpArExtType {
		SCP_AREXT_NONE,			// �ǉ��\���Ȃ�
		SCP_AREXT_L_TRKEEP,		// ���S�L �c���\��
		SCP_AREXT_L_TRCUT,		// ���S�L �J�b�g�Ԑ�
		SCP_AREXT_L_TRRAW,		// ���S�L �G���h�J�[�h���f�O
		SCP_AREXT_L_ECCUT,		// ���S�L �J�b�g�Ԑ�
		SCP_AREXT_L_EC,			// ���S�L �G���h�J�[�h
		SCP_AREXT_L_SP,			// ���S�L �ԑg��
		SCP_AREXT_L_LGCUT,		// ���S�L ���S�[�����J�b�g
		SCP_AREXT_L_LGADD,		// ���S�L ���S�[�����c��
		SCP_AREXT_N_TRCUT,		// ���S�� �J�b�g�Ԑ�
		SCP_AREXT_N_LGCUT,		// ���S�� ���S�[�����J�b�g
		SCP_AREXT_N_LGADD		// ���S�� ���S�[�����c��
	};
	

	//--- �ݒ�l�ێ� ---
	enum class ConfigVarType {
		msecWLogoTRMax,			// AutoCut�R�}���h�ŃJ�b�g�ΏۂƂ��郍�S���ԍő�t���[������
		msecWCompTRMax,			// AutoCut�R�}���hTR�ŗ\���ƔF������\���ő�t���[������
		msecWLogoSftMrg,		// Auto�R�}���h�O�����Ń��S�؂�ւ��̂���������t���[������
		msecWCompFirst,			// �擪�\���J�b�g�����ɂ���\���ő�t���[������
		msecWCompLast,			// �Ō�\���J�b�g�����ɂ���\���ő�t���[������
		msecWLogoSumMin,		// ���S���v���Ԃ��w��t���[�������̎��̓��S�Ȃ��Ƃ��Ĉ���
		msecWLogoLgMin,			// CM�������Ƀ��S�L���m��ƔF������ŏ����S����
		msecWLogoCmMin,			// CM�������Ƀ��S�����m��ƔF������ŏ����S����
		msecWLogoRevMin,		// ���S���␳���鎞�ɖ{�҂ƔF������ŏ�����
		msecMgnCmDetect,		// CM�\����15�b�P�ʂł͂Ȃ��\���ƔF������덷�t���[������
		msecMgnCmDivide,		// CM�\���������������P�b�P�ʂ���̌덷�t���[������
		secWCompSPMin,			// Auto�R�}���h�ԑg�񋟂ŕW���ŏ��b��
		secWCompSPMax,			// Auto�R�}���h�ԑg�񋟂ŕW���ő�b��
		flagCutTR,				// 15�b�ȏ�Ԑ���J�b�g����ꍇ��1���Z�b�g
		flagCutSP,				// �ԑg�񋟂��J�b�g����ꍇ��1���Z�b�g
		flagAddLogo,			// ���S����ʏ�\�����c���ꍇ��1���Z�b�g�i���݂͖��g�p�j
		flagAddUC,				// ���S�Ȃ��s���\�����c���ꍇ��1���Z�b�g
		typeNoSc,				// �V�[���`�F���W�Ȃ������ʒu��CM���f�i0:���� 1:�Ȃ� 2:����j
		cancelCntSc,			// �����������\���𕪗����Ȃ�������1�̎��͎g�p���Ȃ�
		LogoLevel,				// ���S�g�p���x��
		LogoRevise,				// ���S����̕␳
		AutoCmSub,				// ���S�Ȃ����̕⏕�ݒ�
		msecPosFirst,			// ���S�J�n�ʒu���o�ݒ����
		msecLgCutFirst,			// ���S���ŏ����炠�鎞�ɃJ�b�g�����ɂ���\���ő�t���[������
		msecZoneFirst,			// ���S�����Ƃ���J�n�ʒu���o�ݒ����
		msecZoneLast,			// ���S�����Ƃ���I���ʒu���o�ݒ����
		priorityPosFirst,		// �ŏ��̈ʒu�ݒ�D��x�i0:����Ȃ� 1:���S���� 2:�ʒu�D�� 3:Select�D��j
		MAXSIZE
	};
    static const int SIZE_CONFIG_VAR = static_cast<int>(ConfigVarType::MAXSIZE);

	enum class ConfigActType {	// �ݒ�l�𓮍�ʂɎ擾�p
		LogoDelEdge,			// ���S�[��CM���f
		LogoDelMid,				// ���S����15�b�P��CM��
		LogoDelWide,			// �L�惍�S�Ȃ��폜
		LogoUCRemain,			// ���S�Ȃ��s���������c��
		LogoUCGapCm,			// CM�P�ʂ���덷���傫���\�����c��
		MuteNoSc				// �V�[���`�F���W�Ȃ������ʒu��CM���f�i1:�g�p���Ȃ� 2:�g�p����j
	};

	//--- �\�����D�揇��(scp.stat) ---
	enum ScpPriorType {
		SCP_PRIOR_DUPE = -1,	// �Ԉ���
		SCP_PRIOR_NONE,			// �������
		SCP_PRIOR_LV1,			// ���
		SCP_PRIOR_DECIDE		// ����
	};

	//--- �\�����D�揇��(logo.stat_*) ---
	enum LogoPriorType {
		LOGO_PRIOR_DUPE = -1,	// �Ԉ���
		LOGO_PRIOR_NONE,		// �������
		LOGO_PRIOR_LV1,			// ���
		LOGO_PRIOR_DECIDE		// ����
	};
	//--- ���S�����\�����(logo.unit_*) ---
	enum LogoUnitType {
		LOGO_UNIT_NORMAL,		// �ʏ�
		LOGO_UNIT_DIVIDE		// ���S����
	};
	//--- ���S���ʊm����(logo.flag_*) ---
	enum LogoResultType {
		LOGO_RESULT_NONE,		// �������
		LOGO_RESULT_DECIDE,		// �m��
		LOGO_RESULT_ABORT		// abort�j���m��
	};

	//--- �I����� ---
	enum SearchDirType {
		SEARCH_DIR_PREV,
		SEARCH_DIR_NEXT
	};

	//--- ���S�̑I���G�b�W ---
	enum LogoEdgeType {
		LOGO_EDGE_RISE,			// ���S�̗����オ��G�b�W
		LOGO_EDGE_FALL,			// ���S�̗���������G�b�W
		LOGO_EDGE_BOTH			// ���S�̗��G�b�W
	};
	//--- ���S�̑I�� ---
	enum LogoSelectType {
		LOGO_SELECT_ALL,		// �S�I��
		LOGO_SELECT_VALID		// �L���̂ݑI��
	};
	//--- �V�[���`�F���W�őS�̂̐擪�Ō�������I�� ---
	enum ScpEndType {
		SCP_END_EDGEIN,			// �V�[���`�F���W�ԍ��̐擪�Ō�܂�
		SCP_END_NOEDGE			// �V�[���`�F���W�ԍ��̐擪�Ō㏜��
	};

	//--- ���S�̃G�b�W�����F�� ---
	inline bool isLogoEdgeRise(LogoEdgeType type){
		return (type == LOGO_EDGE_RISE || type == LOGO_EDGE_BOTH)? true : false;
	}
	inline bool isLogoEdgeFall(LogoEdgeType type){
		return (type == LOGO_EDGE_FALL || type == LOGO_EDGE_BOTH)? true : false;
	}
	inline LogoEdgeType edgeInvert(LogoEdgeType type){
		return (type == LOGO_EDGE_FALL)? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
	}
	inline bool isLogoEdgeRiseFromNrf(int nrf){
		return (nrf % 2 == 0)? true : false;
	}
	//--- ���S�ԍ��ϊ��inrf - nlg�j ---
	inline LogoEdgeType edgeFromNrf(int nrf){
		return (nrf % 2 == 0)? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
	}
	inline LogoEdgeType edgeInvertFromNrf(int nrf){
		return (nrf % 2 == 0)? LOGO_EDGE_FALL : LOGO_EDGE_RISE;
	}
	inline int nlgFromNrf(int nrf){
		return nrf / 2;
	}
	inline int nrfFromNlg(int nlg, LogoEdgeType edge){
		return (edge == LOGO_EDGE_FALL)? nlg*2+1 : nlg*2;
	}
	inline int nrfFromNlgRise(int nlg){
		return nlg*2;
	}
	inline int nrfFromNlgFall(int nlg){
		return nlg*2+1;
	}
	//--- �D�揇�ʂ̕ϊ� ---
	inline LogoPriorType priorLogoFromScp(ScpPriorType n){
		return (LogoPriorType) n;
	}

	//--- �ێ��ݒ�l�̃t���O�l��` ---
	enum ConfigBitType {
		CONFIG_LOGO_LEVEL_DEFAULT   = 0,
		CONFIG_LOGO_LEVEL_UNUSE_ALL = 1,
		CONFIG_LOGO_LEVEL_UNUSE_EX1 = 2,
		CONFIG_LOGO_LEVEL_UNUSE_EX2 = 3,
		CONFIG_LOGO_LEVEL_USE_LOW   = 4,
		CONFIG_LOGO_LEVEL_USE_MIDL  = 5,
		CONFIG_LOGO_LEVEL_USE_MIDH  = 6,
		CONFIG_LOGO_LEVEL_USE_HIGH  = 7,
		CONFIG_LOGO_LEVEL_USE_MAX   = 8
	};

	//--- �\���� ---
	struct RangeNsc {
		Nsc st;
		Nsc ed;
	};
	struct RangeNrf {
		Nrf st;
		Nrf ed;
	};
	struct RangeMsec {
		Msec st;
		Msec ed;
	};
	struct WideMsec {
		Msec just;
		Msec early;
		Msec late;
	};
	struct RangeFixMsec {
		Msec st;
		Msec ed;
		bool fixSt;				// true=�m��J�n�n�_
		bool fixEd;				// true=�m��I���n�_
	};
	struct RangeWideMsec {
		WideMsec st;
		WideMsec ed;
		bool fixSt;				// true=�m��J�n�n�_
		bool fixEd;				// true=�m��I���n�_
		bool logomode;			// false=CM����  true=���S����
	};
	struct RangeNscMsec {
		RangeNsc  nsc;
		RangeMsec msec;
	};
	struct Term {
		bool valid;
		bool endfix;
		ScpEndType endtype;
		Nsc ini;
		RangeNsc nsc;
		RangeMsec msec;
	};
	struct NrfCurrent {
		bool valid;				// 0=�f�[�^�i�[�Ȃ�  1=�f�[�^�i�[����
		Nrf nrfRise;
		Nrf nrfFall;
		Nrf nrfLastRise;
		Nrf nrfLastFall;
		Msec msecRise;
		Msec msecFall;
		Msec msecLastRise;
		Msec msecLastFall;
	};
	struct ElgCurrent {
		bool valid;				// 0=�f�[�^�i�[�Ȃ�  1=�f�[�^�i�[����
		bool border;			// 0=border�܂߂Ȃ�  1=border�܂�
		bool outflag;			// 0=�������� 1=�ŏI�o�͓���
		Nsc nscRise;
		Nsc nscFall;
		Nsc nscLastRise;
		Nsc nscLastFall;
		Msec msecRise;
		Msec msecFall;
		Msec msecLastRise;
		Msec msecLastFall;
	};
	struct CalcDifInfo {
		int  sgn;
		Sec  sec;
		Msec gap;
	};
	struct CalcModInfo {
		Msec mod15;
		Msec mod05;
	};

}

//---------------------------------------------------------------------
// JL�X�N���v�g�̈����֘A�ێ�
//---------------------------------------------------------------------
namespace jlscmd
{
	const int SIZE_VARNUM_MAX = 2048;	// �ϐ��̍ő吔��O�̂��ߐݒ�(JlsRegFile)
	const int SIZE_MEMVARNUM_MAX = 2048;	// �x���ۊǕ�����̎��ʖ��ő吔(JlsScrMemBody)
	const int SIZE_MEMVARLINE_MAX = 4096;	// �x���ۊǕ�����̊e���ʖ��̍ő�s��(JlsScrMemBody)
	const int SIZE_REPLINE  = 4096;		// �L���b�V���ێ��ő�s���iRepeat�p�j(JlsScriptState)
	const int SIZE_MEMLINE  = 8192;		// �L���b�V���ێ��ő�s���iMem/Lazy�p�j(JlsScrGlobal)
	const int SIZE_CALL_LOOP = 10;		// Call�R�}���h�̍ċA�ő��(JlsScript)

	//--- JL�X�N���v�g���� ---
	enum class CmdType {
		Nop,
		If,
		EndIf,
		Else,
		ElsIf,
		Call,
		Repeat,
		EndRepeat,
		LocalSt,
		LocalEd,
		ArgBegin,
		ArgEnd,
		Exit,
		Return,
		FileOpen,
		FileAppend,
		FileClose,
		Echo,
		LogoOff,
		OldAdjust,
		LogoDirect,
		LogoExact,
		LogoReset,
		ReadData,
		ReadTrim,
		ReadString,
		EnvGet,
		Set,
		Default,
		EvalFrame,
		EvalTime,
		EvalNum,
		CountUp,
		SetParam,
		OptSet,
		OptDefault,
		UnitSec,
		LocalSet,
		ArgSet,
		ListGetAt,
		ListIns,
		ListDel,
		ListSetAt,
		ListClear,
		ListSort,
		AutoCut,
		AutoAdd,
		AutoEdge,
		AutoCM,
		AutoUp,
		AutoBorder,
		AutoIClear,
		AutoIns,
		AutoDel,
		Find,
		MkLogo,
		DivLogo,
		Select,
		Force,
		Abort,
		GetPos,
		GetList,
		NextTail,
		DivFile,
		LazyStart,
		EndLazy,
		Memory,
		EndMemory,
		MemCall,
		MemErase,
		MemCopy,
		MemMove,
		MemAppend,
		MemOnce,
		LazyFlush,
		LazyAuto,
		LazyStInit,
		MemEcho,
		MemDump,
		ExpandOn,
		ExpandOff,
		MAXSIZE
	};
	//--- JL�X�N���v�g���ߎ�� ---
	enum class CmdCat {
		NONE,
		COND,
		CALL,
		REP,
		FLOW,
		SYS,
		REG,
		NEXT,
		LOGO,
		AUTOLOGO,
		AUTOEACH,
		AUTO,
		LAZYF,
		MEMF,
		MEMEXE,
		MEMLAZYF,
	};
	//--- JL�X�N���v�g�x�����s�pCache��� ---
	enum class CacheExeType {
		None,
		LazyS,
		LazyA,
		LazyE,
		Mem,
	};
	//--- JL�X�N���v�gLazy������ ---
	enum class LazyType {
		None,
		FULL,
		LazyS,
		LazyA,
		LazyE,
	};
	//--- JL�X�N���v�g�I�v�V�������� ---
	enum class OptCat {
		None,
		PosSC,
		NumLG,
		FRAME,
		STR,
		NUM,
	};
	//--- JL�X�N���v�g�I�v�V�������� ---
	enum class OptType {
		None,			// ����`�F���p
		ArrayMIN,		// �J�n���ʗp
		TypeNumLogo,
		TypeFrame,
		TypeFrameSub,
		MsecFrameL,
		MsecFrameR,
		MsecEndlenC,
		MsecEndlenL,
		MsecEndlenR,
		MsecSftC,
		MsecSftL,
		MsecSftR,
		MsecTgtLimL,
		MsecTgtLimR,
		MsecLenPMin,
		MsecLenPMax,
		MsecLenNMin,
		MsecLenNMax,
		MsecLenPEMin,
		MsecLenPEMax,
		MsecLenNEMin,
		MsecLenNEMax,
		MsecFromAbs,
		MsecFromHead,
		MsecFromTail,
		MsecLogoExtL,
		MsecLogoExtR,
		MsecEndAbs,
		MsecDcenter,
		MsecDrangeL,
		MsecDrangeR,
		MsecDmargin,
		MsecEmargin,
		AutopCode,
		AutopLimit,
		AutopScope,
		AutopScopeN,
		AutopScopeX,
		AutopPeriod,
		AutopMaxPrd,
		AutopSecNext,
		AutopSecPrev,
		AutopTrScope,
		AutopTrSumPrd,
		AutopTr1stPrd,
		AutopTrInfo,
		FlagWide,
		FlagFromLast,
		FlagWithP,
		FlagWithN,
		FlagNoEdge,
		FlagOverlap,
		FlagConfirm,
		FlagUnit,
		FlagElse,
		FlagCont,
		FlagReset,
		FlagFlat,
		FlagForce,
		FlagNoForce,
		FlagAutoChg,
		FlagAutoEach,
		FlagEndHead,
		FlagEndTail,
		FlagEndHold,
		FlagRelative,
		FlagLazyS,
		FlagLazyA,
		FlagLazyE,
		FlagNow,
		FlagNoLap,
		FlagEdgeS,
		FlagEdgeE,
		FlagClear,
		FlagPair,
		FlagFinal,
		FlagLocal,
		FlagDefault,
		FlagUnique,
		FlagDummy,
		AbbrEndlen,
		AbbrSft,
		AbbrFromHead,
		AbbrFromTail,
		ArrayMAX,		// JlsCmdSet�Ńf�[�^�i�[����z��͂����܂�

		ScMIN,
		ScNone,
		ScSC,
		ScNoSC,
		ScSM,
		ScNoSM,
		ScSMA,
		ScNoSMA,
		ScAC,
		ScNoAC,
		ScMAX,

		LgMIN,
		LgNone,
		LgN,
		LgNR,
		LgNlogo,
		LgNauto,
		LgNFlogo,
		LgNFauto,
		LgNFXlogo,
		LgMAX,

		FrMIN,
		FrF,
		FrFR,
		FrFhead,
		FrFtail,
		FrFmid,
		FrFheadX,
		FrFtailX,
		FrFmidX,
		FrMAX,

		StrMIN,			// JlsCmdSet�Ńf�[�^�i�[���镶����I�v�V�����J�n
		StrRegPos,
		StrValPosR,
		StrValPosW,
		StrRegList,
		StrValListR,
		StrValListW,
		StrRegSize,
		StrRegEnv,
		StrArgVal,
		StrMAX,			// JlsCmdSet�Ńf�[�^�i�[���镶����I�v�V�����I��
	};

	//--- JL�X�N���v�g���߃T�u�I�� ---
	enum CmdTrSpEcID {
		None,
		Off,
		TR,
		SP,
		EC,
		LG,
	};
	//--- JL�X�N���v�g�f�R�[�h���ʃG���[ ---
	enum class CmdErrType {
		None,
		ErrOpt,				// �R�}���h�ُ�i�I�v�V�����j
		ErrRange,			// �R�}���h�ُ�i�͈́j
		ErrSEB,				// �R�}���h�ُ�iS/E/B�I���j
		ErrVar,				// �R�}���h�ُ�i�ϐ��֘A�j
		ErrTR,				// �R�}���h�ُ�iTR/SP/ED�I���j
		ErrCmd,				// �R�}���h�ُ�i�R�}���h�j
	};
	//--- JL�X�N���v�gAuto�n�R�}���h ---
	enum class CmdAutoType {
		None,
		CutTR,
		CutEC,
		AddTR,
		AddSP,
		AddEC,
		Edge,
		AtCM,
		AtUP,
		AtBorder,
		AtIClear,
		AtChg,
		Ins,
		Del,
	};
	//--- Auto�R�}���h�p�����[�^ ---
	enum class ParamAuto {
		// code�p�����[�^
		c_exe,			// 0:�R�}���h���s�Ȃ� 1:�R�}���h���s
		c_search,		// ��������͈͂�I��
		c_wmin,			// �\�����Ԃ̍ŏ��l�b��
		c_wmax,			// �\�����Ԃ̍ő�l�b��
		c_w15,			// 1:�ԑg�\����15�b������
		c_lgprev,		// 0:���S�E�\���̑O����ΏۊO
		c_lgpost,		// 0:���S�E�\���̌㑤��ΏۊO
		c_lgintr,		// 1:�\���Ɣԑg�񋟂̊Ԃ̂ݑΏۂƂ���
		c_lgsp,			// 1:�ԑg�񋟂�����ɂ���ꍇ�̂ݑΏ�
		c_cutskip,		// 1:�\���J�b�g�ȍ~���ΏۂƂ���
		c_in1,			// 1:�\���ʒu�ɔԑg�񋟂�����
		c_chklast,		// 1:�{�̍\������ɂ���ΑΏۊO�Ƃ���
		c_lgy,			// 1:���S����ΏۂƂ���
		c_lgn,			// 1:���S�O��ΏۂƂ���
		c_lgbn,			// 1:���ׂ��܂߃��S�O�̏ꍇ��ΏۂƂ���
		c_limloc,		// 1:�W�����Ԃ̌��ʒu�݂̂Ɍ���
		c_limtrsum,		// 1:�\�����Ԃɂ�薳��������
		c_unitcmoff,		// 1:CM���������\���̌��o����������
		c_unitcmon,		// 1:CM���������\���̌��o�������ݒ�
		c_wdefmin,		// �W���̍\�����Ԃ̍ŏ��l�b��
		c_wdefmax,		// �W���̍\�����Ԃ̍ő�l�b��
		// autocut�p
		c_from,			// cuttr
		c_cutst,			// cuttr
		c_lgpre,			// cuttr
		c_sel,			// cutec
		c_cutla,			// cutec
		c_cutlp,			// cutec
		c_cut30,			// cutec
		c_cutsp,			// cutec
		// edge�p
		c_cmpart,
		c_add,
		c_allcom,
		c_noedge,
		// autoins,autodel�p
		c_restruct,
		c_unit,
		// ���l�p�����[�^
		v_limit,
		v_scope,
		v_scopen,
		v_period,
		v_maxprd,
		v_trsumprd,
		v_secprev,
		v_secnext,
		// autocut�p
		v_trscope,
		v_tr1stprd,
		// autoins,autodel�p
		v_info,
		// ���v��
		MAXSIZE
	};
}

