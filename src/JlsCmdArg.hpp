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
		bool      comFrom;					// 0:�ʏ�  1:�����\��from�w��
		bool      useScC;					// 0:�ʏ�  1:-C�I�v�V�����t��
		bool      floatBase;				// 0:���S�ʒu�  1:���ʈʒu�
		bool      shiftBase;				// 0:�ʏ�  1:�V�t�g��ʒu
		bool      virtualLogo;				// 0:���ۂ̃��S  1:�������S�������S
		bool      ignoreComp;				// 0:�ʏ�  1:���S�m���Ԃł����s
		bool      limitByLogo;				// 0:�ʏ�  1:�אڃ��S�܂łɐ���
		bool      needAuto;					// 0:�ʏ�  1:Auto�\���K�v
		bool      fullFrameA;				// 0:�ʏ�  1:-F�n����`���͏�ɑS��(RANGETYPE=0�̎�)
		bool      fullFrameB;				// 0:�ʏ�  1:-F�n����`���͏�ɑS��(RANGETYPE=1�̎�)
		LazyType  typeLazy;					// �x�����s�ݒ���
		bool      ignoreAbort;				// 0:�ʏ�  1:���SAbort��Ԃł����s
		bool      immFrom;					// 0:�ʏ�  1:���ڃt���[��from�w��
		bool      existDstOpt;				// 0:Dst�w��Ȃ�  1:Dst�w�肠��
		bool      forcePos;					// 0:�ʏ�  1:�����ʒu�ݒ�
		bool      pickIn;					// 0:�ʏ�  1:�I�ʂ��ē���
		bool      pickOut;					// 0:�ʏ�  1:�I�ʂ��ďo��
	};
	struct CmdArgCond {						// ��͎��̏��
		int       numCheckCond;				// �������̊m�F�ʒu�i0=�s�v�A1-=�m�F��������ʒu�j
		bool      flagCond;					// IF���p�̏������f
	};
	struct CmdArgSc {					// -SC�n�̃I�v�V�����f�[�^
		OptType   type;					// �I�v�V�������
		TargetCatType  category;		// �K�p�Ώۈʒu�̑I��
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
	bool   getOptFlag(OptType tp);
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
	void   signalInternalRegError(string msg, OptType tp);
public:
// -SC�n�R�}���h�p
	void    addScOpt(OptType tp, TargetCatType tgcat, int tmin, int tmax);
	OptType getScOptType(int num);
	TargetCatType getScOptCategory(int num);
	Msec	getScOptMin(int num);
	Msec	getScOptMax(int num);
	int		sizeScOpt();
// -LG�n�R�}���h�p
	void	addLgOpt(string strNlg);
	string	getLgOpt(int num);
	string	getLgOptAll();
	int		sizeLgOpt();
// �����擾
	void   addArgString(const string& strArg);
	bool   replaceArgString(int n, const string& strArg);
	string getStrArg(int n);
	int    getValStrArg(int n);
	int    getListStrArgs(vector<string>& listStr);
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
};
