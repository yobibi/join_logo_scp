//
// ������Ǝ��Ԃƃt���[���ʒu�̑��ݕϊ��N���X
//

class CnvStrTime
{
private:
	// �ɒ[�ɒ���������̕ی��j���p
	static const int SIZE_BUF_MAX   = 16384;
	// ���Z���ށi���Z�q��`�̕��ނɑΉ��j
	static const int D_CALCCAT_IMM  = 0;			// ���l
	static const int D_CALCCAT_OP2  = 1;			// �Q�����Z
	static const int D_CALCCAT_OP1  = 2;			// �P�����Z
	static const int D_CALCCAT_PAR  = 3;			// ����
	static const int D_CALCCAT_OPE  = 4;			// �P���㉉�Z
	// ���Z�q��` 0xF000�r�b�g:����  0x0F00�r�b�g�F�D�揇��
	static const int D_CALCOP_PERD  = 0x0021;		// .�i�����_�j
	static const int D_CALCOP_COLON = 0x0022;		// :�i�����b�j
	static const int D_CALCOP_PLUS  = 0x1501;		// +
	static const int D_CALCOP_MINUS = 0x1502;		// -
	static const int D_CALCOP_MUL   = 0x1401;		// *
	static const int D_CALCOP_DIV   = 0x1402;		// /
	static const int D_CALCOP_MOD   = 0x1403;		// %
	static const int D_CALCOP_CMPLT = 0x1701;		// <
	static const int D_CALCOP_CMPLE = 0x1702;		// <=
	static const int D_CALCOP_CMPGT = 0x1703;		// >
	static const int D_CALCOP_CMPGE = 0x1704;		// >=
	static const int D_CALCOP_CMPEQ = 0x1801;		// ==
	static const int D_CALCOP_CMPNE = 0x1802;		// !=
	static const int D_CALCOP_B_AND = 0x1901;		// &
	static const int D_CALCOP_B_XOR = 0x1A01;		// ^
	static const int D_CALCOP_B_OR  = 0x1B01;		// |
	static const int D_CALCOP_L_AND = 0x1C01;		// &&
	static const int D_CALCOP_L_OR  = 0x1D01;		// ||
	static const int D_CALCOP_NOT   = 0x2201;		// !
	static const int D_CALCOP_SIGNP = 0x2202;		// +�i�����j
	static const int D_CALCOP_SIGNM = 0x2203;		// -�i�����j
	static const int D_CALCOP_P_INC = 0x2201;		// ++�i�O���j�����
	static const int D_CALCOP_P_DEC = 0x2202;		// --�i�O���j�����
	static const int D_CALCOP_SEC   = 0x2204;		// S
	static const int D_CALCOP_FRM   = 0x2205;		// F
	static const int D_CALCOP_PARS  = 0x3101;		// (
	static const int D_CALCOP_PARE  = 0x3102;		// )
	static const int D_CALCOP_N_INC = 0x4201;		// ++�i�㑤�j�����
	static const int D_CALCOP_N_DEC = 0x4202;		// --�i�㑤�j�����
	static const int D_CALCOP_ERROR = 0xFFFF;		// �G���[
	// �����񂩂�擾���鎞�̋�؂�
	enum DelimtStrType {
		DELIMIT_SPACE_QUOTE,	// �󔒋�؂�QUOTE��
		DELIMIT_SPACE_ONLY,		// �󔒂̂݋�؂�
		DELIMIT_SPACE_COMMA,	// �󔒁{�R���}����؂�
		DELIMIT_SPACE_EXNUM,	// �ŏ��̐��������̂�
		DELIMIT_CSV,			// CSV�`��
		DELIMIT_FUNC_NAME,		// �֐��̖��O����
		DELIMIT_FUNC_ARGS,		// �֐��̈��������i�󔒋�؂�j
		DELIMIT_FUNC_CALC,		// �֐��̉��Z�����i�R���}��؂�j
	};
	// �����̐���p���
	enum CharCtrType {
		CHAR_CTR_NULL,			// ������I��
		CHAR_CTR_CTRL,			// ����R�[�h
		CHAR_CTR_SPACE,			// ��
		CHAR_CTR_OTHER			// �ʏ핶��
	};
	// �������؂���
	struct ArgItemType {
		DelimtStrType dstype;	// �����̐���p���
		bool concat;			// �A��quote�̌���
		bool separate;			// quote�r�؂ꂠ��΋�؂蕶���Ȃ��Ă���؂�
		bool remain;			// quote����Η��[�ɓ����
		bool defstr;			// ��`�p�̕�����iquote���󃊃X�g���c���j
		bool qdisp;				// �r����quote������quote�F�����Ă��鎞�ł��c��
		bool emptyok;			// �f�[�^�Ȃ�������
	};
	// ��������N�H�[�g���
	struct QuoteType {
		bool flagQw;			// "���p��
		bool flagQs;			// '���p��
		bool existQ;			// �[��QUOTE
		bool edgeQw;			// �[��QUOTE��"�g�p
		int  numPar;			// ���ʂ̐�
	};
	// ��������N�H�[�g���
	struct QuoteState {
		bool end;				// �I���\��
		bool add;				// �ǉ�����
		bool pos;				// �ǂݍ��݈ʒu�ړ�����
	};

public:
	CnvStrTime();
	//--- �t�@�C������� ---
	bool getStrFileAllPath(string &pathname);
	int getStrFilePath(string &pathname, const string &fullname);
	int getStrFilePathName(string &pathname, string &fname, const string &fullname);
	string getStrFileDelimiter();
	//--- �����񕪊� ---
	int  getBufLineSize();
	bool isStrFuncModule(const string &cstr, int pos);
	int  getListModuleArg(vector<string>& listMod, const string &cstr, int pos);
	bool getStrDivPath(string& strDiv, bool selHead, bool withDelim);
	bool getStrDivide(string& strDiv, const string& strDelim, bool selHead, bool typePath);
private:
	int  getStrPosPath(const string& fullname);
	int  getStrPosDivide(const string& fullname, const string& strDelim, bool typePath);
	int  getStrPosDivideCore(const string& fullname, const string& strDelim, bool reverse);
public:
	//--- ���Ԃƃt���[���ʒu�̕ϊ� ---
	int getFrmFromMsec(Msec msec);
	int getMsecFromFrm(int frm);
	int getMsecAlignFromMsec(Msec msec);
	int getMsecAdjustFrmFromMsec(Msec msec, int frm);
	int getSecFromMsec(Msec msec);
	int changeFrameRate(int n, int d);
	int changeUnitSec(int n);
	//--- �����񂩂�l�擾 ---
	int getStrValNumHead(int &val, const string &cstr, int pos);
	int getStrValNum(int &val, const string &cstr, int pos);
	int getStrValMsec(Msec &val, const string &cstr, int pos);
	int getStrValMsecFromFrm(Msec &val, const string &cstr, int pos);
	int getStrValMsecM1(Msec &val, const string &cstr, int pos);
	int getStrValSec(int &val, const string &cstr, int pos);
	int getStrValSecFromSec(int &val, const string &cstr, int pos);
	int getStrValFuncNum(int &val, const string &cstr, int pos);
	//--- ���X�g�f�[�^�擾 ---
	bool getListValMsec(vector<Msec>& listMsec, const string& strList);
	bool getListValMsecM1(vector<Msec>& listMsec, const string& strList);
	//--- �����񂩂�P��擾 ---
	int getStrItem(string &dst, const string &cstr, int pos);
	int getStrWord(string &dst, const string &cstr, int pos);
	int getStrCsv(string &dst, const string &cstr, int pos);
	int getStrItemCmd(string &dst, const string &cstr, int pos);
	int getStrItemArg(string &dst, const string &cstr, int pos);
	int getStrItemMonitor(string &dst, const string &cstr, int pos);
	int getStrItemWithQuote(string &dst, const string &cstr, int pos);
	int getStrWithoutComment(string &dst, const string &cstr);
	int getStrPosComment(const string &cstr, int pos);
	int getStrPosReplaceVar(const string &cstr, int pos);
	int getStrPosChar(const string &cstr, char chsel, bool expand, int pos);
	int getStrMultiNum(string &dst, const string &cstr, int pos);
	bool isStrMultiNumIn(const string &cstr, int numCur, int numMax);
	//--- ���Ԃ𕶎���i�t���[���܂��̓~���b�j�ɕϊ� ---
	string getStringMsecM1(Msec msec_val);
	string getStringFrameMsecM1(Msec msec_val);
	string getStringTimeMsecM1(Msec msec_val);
	string getStringMsecM1All(Msec msec_val, bool type_frm);
	string getStringZeroRight(int val, int len);

private:
	int  getMbStrSize(const string& str, int n);
	int  getMbStrSizeSjis(const string& str, int n);
	int  getMbStrSizeUtf8(const string& str, int n);
	bool isStrMbSecond(const string& str, int n);
	int  getStrValSub(int &val, const string &cstr, int pos, int unitsec);
	int  getStrValSubDelimit(int &val, const string &cstr, int pos, int unitsec, DelimtStrType type);
	int  getStrItemHubRange(int &st, int &ed, const string &cstr, int pos, DelimtStrType type);
	int  getStrItemHubFunc(string& dstr, const string &cstr, int pos, DelimtStrType dstype);
	int  getStrItemHubStr(string& dstr, const string &cstr, int pos, ArgItemType itype);
	int  getStrItemCommon(string& dstr, int &st, int &ed, const string &cstr, int pos, ArgItemType itype);
	bool getStrItemCommonCh(QuoteState& qstate, QuoteType& qtype, char ch, bool yet, ArgItemType itype);
	CnvStrTime::CharCtrType getCharTypeSub(char ch);
	int  skipCharSpace(const string &cstr, int pos);
	bool isCharTypeSpace(char ch);
	bool isCharTypeSpaceEnd(char ch);
	bool isCharTypeDelim(char ch, DelimtStrType dstype);
	bool isCharValidDquote(DelimtStrType dstype);
	bool isCharValidSquote(DelimtStrType dstype);

	int getStrCalc(const string &cstr, int st, int ed, int unitsec);
	int getStrCalcDecode(const string &cstr, int st, int ed, int dsec, int draw);
	int getMarkCategory(int code);
	int getMarkPrior(int code);
	int getStrCalcCodeChar(char ch, int head);
	int getStrCalcCodeTwoChar(char ch1, char ch2, int head);
	int getStrCalcTime(const string &cstr, int st, int ed, int dsec, int draw);
	int getStrCalcOp1(int din, int nMark);
	int getStrCalcOp2(int din1, int din2, int nMark);

private:
	int m_frate_n;				// �t���[�����[�g�p(�����l=30000)
	int m_frate_d;				// �t���[�����[�g�p(�����l=1001)
	int m_unitsec;				// �����P�ʁi0:�t���[�� 1:�~���b�j
	string m_delimiter;			// �t�@�C���p�X�̋�؂蕶��

};
