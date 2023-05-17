//
// ���s�X�N���v�g�R�}���h�̈�����������^�[�Q�b�g���i��
//  �o�́F
//    JlsCmdSet& cmdset.limit
//
#pragma once

#include "JlsScriptLimVar.hpp"

class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsDataset;

///////////////////////////////////////////////////////////////////////
//
// ��������ɂ��^�[�Q�b�g�I��N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScriptLimit
{
private:
	struct ScrTargetRecord {	// �^�[�Q�b�g�ʒu�������̃f�[�^
		// �ݒ�
		bool flagNoEdge;		// �S�̂̐擪�ƍŌ�̃t���[���͊܂߂Ȃ�
		bool flagNextTail;		// NextTail�R�}���h�p
		bool selectLogoRise;	// NextTail�R�}���h�Ń��S�����D��
		// ����
		TargetLocInfo tgDst;	// ���ʈʒu
		TargetLocInfo tgEnd;	// �I���ʒu
		int  numListDst;		// �������̒�����I�����ꂽ�ԍ�
		int  numListEnd;		// �������̒�����I�����ꂽ�ԍ�
		int  numListTryDst;		// �������̒�����I�����ꂽ�ԍ��i��⌟���p�j
		ScpPriorType statDst;	// �\���̗D�揇��
		ScpPriorType statEnd;	// �\���̗D�揇��
		Msec gapDst;			// ���S����̋���
		Msec gapEnd;			// ���S����̋���
		bool flagOnLogo;		// NextTail�R�}���h�̃��S����茟�o�p
	};
	struct ScrOptCRecord {		// �����\���I�v�V����
		bool exist;
		bool C;
		bool Tra;
		bool Trr;
		bool Trc;
		bool Sp;
		bool Ec;
		bool Bd;
		bool Mx;
		bool Aea;
		bool Aec;
		bool Cm;
		bool Nl;
		bool L;
	};

public:
	JlsScriptLimit(JlsDataset *pdata);
	void limitCommonRange(JlsCmdSet& cmdset);
	void resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec);
	int  limitLogoList(JlsCmdSet& cmdset);
	bool selectTargetByLogo(JlsCmdSet& cmdset, int nlist);
	void selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec);

private:
	//--- �R�}���h���ʂ͈̔͌��� ---
	void limitCustomLogo();
	void limitHeadTail();
	void limitHeadTailImm(RangeMsec rmsec);
	void limitWindow();
	void updateCommonRange(JlsCmdSet& cmdset);
	//--- �L���ȃ��S�ʒu���X�g���擾 ---
	void getLogoListStd(JlsCmdSet& cmdset);
	bool isLogoListStdNumUse(int curNum, int maxNum);
	bool getLogoListStdData(vector<Msec>& listMsecLogoIn, int& locStart, int& locEnd);
	bool getLogoListStdDataRange(int& st, int& ed, vector<Msec>& listMsec, RangeMsec rmsec);
	void getLogoListDirect(JlsCmdSet& cmdset);
	void getLogoListDirectCom(JlsCmdSet& cmdset);
	void getLogoListDirectComOpt(ScrOptCRecord& optC);
	bool getLogoListDirectComOptSub(bool& data, int n);
	bool isLogoListDirectComValid(Nsc nscCur, ScrOptCRecord optC);
	int  getLogoListNearest(JlsCmdSet& cmdset, vector<Msec> listMsec, Msec msecFrom);
	//--- ���S�ʒu���X�g���̎w�胍�S�Ŋ���S�f�[�^���쐬 ---
	bool baseLogo(JlsCmdSet& cmdset, int nlist);
	bool getBaseLogo(JlsCmdSet& cmdset, int nlist);
	void getBaseLogoForTg(WideMsec& wmsecTg, LogoEdgeType& edgeTg, JlsCmdSet& cmdset, bool flagBase);
	bool checkBaseLogo(JlsCmdSet& cmdset);
	bool checkBaseLogoLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN);
	//--- �^�[�Q�b�g�͈͂��擾 ---
	bool targetRangeByLogo(JlsCmdSet& cmdset);
	void targetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec);
	void updateTargetRange(JlsCmdSet& cmdset, bool fromLogo);
	void addTargetRangeByLogoShift(WideMsec wmsecBase);
	void addTargetRangeData(WideMsec wmsecBase);
	bool findTargetRange(WideMsec& wmsecFind, WideMsec wmsecBase, Msec msecFrom);
	bool findTargetRangeSetBase(WideMsec& wmsecFind, WideMsec& wmsecAnd, WideMsec wmsecBase, Msec msecFrom);
	bool findTargetRangeLimit(WideMsec& wmsecFind, WideMsec& wmsecAnd);
	//--- �^�[�Q�b�g�ʒu���擾 ---
	void targetPoint(JlsCmdSet& cmdset);
	void setTargetPointOutEdge(JlsCmdSet& cmdset);
	void seekTargetPoint(JlsCmdSet& cmdset);
	void seekTargetPointFromScp(JlsCmdSet& cmdset, Nsc nscNow, bool lastNsc);
	bool seekTargetPointEnd(JlsCmdSet& cmdset, Msec msecRef, bool force);
	void seekTargetPointEndRefer(TargetLocInfo& tgEnd, JlsCmdSet& cmdset, Msec msecIn);
	bool seekTargetPointEndScp(JlsCmdSet& cmdset, Msec msecIn, Msec msecDst, Nsc nscAnd);
	void prepTargetPoint(JlsCmdSet& cmdset);
	void prepTargetPointEnd(JlsCmdSet& cmdset);
	bool prepTargetPointEndAbs(TargetLocInfo& tgEnd, bool& multiBase, JlsCmdSet& cmdset);
	//--- ���������Ŏg�p ---
	// ��ʒu����̃��S�O�㕝
	void getWidthLogoFromBase(WideMsec& wmsec, JlsCmdSet& cmdset, int step, bool flagWide);
	void getWidthLogoFromBaseForTarget(WideMsec& wmsec, JlsCmdSet& cmdset, int step, bool flagWide);
	void getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide);

private:
	JlsScriptLimVar var;

private:
	JlsDataset *pdata;									// ���̓f�[�^�A�N�Z�X
};
