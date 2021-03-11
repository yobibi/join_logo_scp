//
// ���s�X�N���v�g�R�}���h�̈�����������^�[�Q�b�g���i��
//  �o�́F
//    JlsCmdSet& cmdset.limit
//
#pragma once

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
	enum class ScrLogoSetBase {
		None,			// �ݒ�Ȃ�
		BaseLoc,		// ��ʒu��ݒ肷�铮��
		ValidList		// �L�����S���X�g��ݒ肷�铮��
	};
	struct ScrLogoInfoCmdRecord {	// ���S���X�g�擾�p�Ɏg�p
		LogoSelectType typeLogo;	// LOGO_SELECT_ALL or LOGO_SELECT_VALID
		ScrLogoSetBase typeSetBase;	// 0=�ݒ�Ȃ� 1=��ʒu�̐ݒ� 2=�L�����S���X�g�̐ݒ�
		LogoEdgeType edgeSel;		// ��������ꍇ�̗����^������
		Msec msecSel;				// ��������ꍇ�̈ʒu
	};

public:
	JlsScriptLimit(JlsDataset *pdata);
	void limitCommonRange(JlsCmdSet& cmdset);
	void resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec);
	int  limitLogoList(JlsCmdSet& cmdset);
	bool selectTargetByLogo(JlsCmdSet& cmdset, int nlist);
	void selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec, bool force);

private:
	//--- �R�}���h���ʂ͈̔͌��� ---
	void limitHeadTail(JlsCmdSet& cmdset);
	void limitHeadTailImm(JlsCmdSet& cmdset, RangeMsec rmsec);
	void limitWindow(JlsCmdSet& cmdset);
	void limitListForTarget(JlsCmdSet& cmdset);
	//--- ���S�ʒu��񃊃X�g���擾 ---
	bool limitLogoListSub(JlsCmdArg& cmdarg, int curNum, int maxNum);
	bool limitLogoListRange(int& st, int& ed, vector<WideMsec>& listWmsec, RangeMsec rmsec);
	//--- �w�胍�S�̐����K�p ---
	bool limitTargetLogo(JlsCmdSet& cmdset, int nlist);
	bool limitTargetLogoGet(JlsCmdSet& cmdset, int nlist);
	bool limitTargetLogoCheck(JlsCmdSet& cmdset);
	bool limitTargetLogoCheckLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN);
	bool limitTargetRangeByLogo(JlsCmdSet& cmdset);
	void limitTargetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec, bool force);
	//--- �^�[�Q�b�g�ʒu���擾 ---
	void getTargetPoint(JlsCmdSet& cmdset);
	void getTargetPointOutEdge(JlsCmdSet& cmdset);
	bool getTargetPointEndResult(int& nsc_scpos_end, JlsCmdArg& cmdarg, int msec_base);
	Nsc  getTargetPointEndlen(JlsCmdArg& cmdarg, int msec_base);
	Nsc  getTargetPointEndArg(JlsCmdArg& cmdarg, int msec_base);
	void getTargetPointSetScpEnable(JlsCmdSet& cmdset);
	//--- ���������Ŏg�p ---
	bool getLogoInfoList(vector<WideMsec>& listWmsecLogo, JlsCmdSet& cmdset, ScrLogoInfoCmdRecord infoCmd);
	int  getLogoInfoListFind(vector<WideMsec>& listWmsec, Msec msecLogo, LogoEdgeType edge);
	void getLogoInfoListLg(vector<WideMsec>& listWmsec, vector<Nrf>& listNrf, LogoSelectType type);
	void getLogoInfoListElg(vector<WideMsec>& listWmsec, vector<Nsc>& listNsc, bool outflag);
	bool checkOptScpFromMsec(JlsCmdArg &cmdarg, int msec_base, LogoEdgeType edge, bool chk_base, bool chk_rel);

private:
	JlsDataset *pdata;									// ���̓f�[�^�A�N�Z�X
};
