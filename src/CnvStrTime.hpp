//
// 文字列と時間とフレーム位置の相互変換クラス
//

class CnvStrTime
{
private:
	// 極端に長い文字列の保険破棄用
	static const int SIZE_BUF_MAX   = 4096;
	// 演算分類（演算子定義の分類に対応）
	static const int D_CALCCAT_IMM  = 0;			// 数値
	static const int D_CALCCAT_OP2  = 1;			// ２項演算
	static const int D_CALCCAT_OP1  = 2;			// 単項演算
	static const int D_CALCCAT_PAR  = 3;			// 括弧
	static const int D_CALCCAT_OPE  = 4;			// 単項後演算
	// 演算子定義 0xF000ビット:分類  0x0F00ビット：優先順位
	static const int D_CALCOP_PERD  = 0x0021;		// .（小数点）
	static const int D_CALCOP_COLON = 0x0022;		// :（時分秒）
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
	static const int D_CALCOP_SIGNP = 0x2202;		// +（符号）
	static const int D_CALCOP_SIGNM = 0x2203;		// -（符号）
	static const int D_CALCOP_P_INC = 0x2201;		// ++（前側）非実装
	static const int D_CALCOP_P_DEC = 0x2202;		// --（前側）非実装
	static const int D_CALCOP_SEC   = 0x2204;		// S
	static const int D_CALCOP_FRM   = 0x2205;		// F
	static const int D_CALCOP_PARS  = 0x3101;		// (
	static const int D_CALCOP_PARE  = 0x3102;		// )
	static const int D_CALCOP_N_INC = 0x4201;		// ++（後側）非実装
	static const int D_CALCOP_N_DEC = 0x4202;		// --（後側）非実装
	static const int D_CALCOP_ERROR = 0xFFFF;		// エラー
	// 文字列から取得する時の区切り
	enum DelimtStrType {
		DELIMIT_SPACE_QUOTE,	// 空白区切りQUOTE可
		DELIMIT_SPACE_ONLY,		// 空白のみ区切り
		DELIMIT_SPACE_COMMA,	// 空白＋コンマも区切り
		DELIMIT_SPACE_EXNUM		// 最初の数字部分のみ
	};
	// 文字の制御用種類
	enum CharCtrType {
		CHAR_CTR_NULL,			// 文字列終了
		CHAR_CTR_CTRL,			// 制御コード
		CHAR_CTR_SPACE,			// 空白
		CHAR_CTR_OTHER			// 通常文字
	};

public:
	CnvStrTime();
	//--- ファイル名解析 ---
	int getStrFilePathName(string &pathname, string &fname, const string &fullname);
	int getStrFilePath(string &pathname, const string &fullname);
	string getStrFileDelimiter();
	//--- 時間とフレーム位置の変換 ---
	int getFrmFromMsec(int msec);
	int getMsecFromFrm(int frm);
	int getMsecAlignFromMsec(int msec);
	int getMsecAdjustFrmFromMsec(int msec, int frm);
	int getSecFromMsec(int msec);
	int changeFrameRate(int n, int d);
	int changeUnitSec(int n);
	//--- 文字列から値取得 ---
	int getStrValNumHead(int &val, const string &cstr, int pos);
	int getStrValNum(int &val, const string &cstr, int pos);
	int getStrValMsec(int &val, const string &cstr, int pos);
	int getStrValMsecFromFrm(int &val, const string &cstr, int pos);
	int getStrValMsecM1(int &val, const string &cstr, int pos);
	int getStrValSec(int &val, const string &cstr, int pos);
	int getStrValSecFromSec(int &val, const string &cstr, int pos);
	//--- 文字列から単語取得 ---
	int getStrItem(string &dst, const string &cstr, int pos);
	int getStrWord(string &dst, const string &cstr, int pos);
	int getStrItemWithQuote(string &dst, const string &cstr, int pos);
	//--- 時間を文字列（フレームまたはミリ秒）に変換 ---
	string getStringMsecM1(int msec_val);
	string getStringFrameMsecM1(int msec_val);
	string getStringTimeMsecM1(int msec_val);
	string getStringMsecM1All(int msec_val, bool type_frm);
	string getStringZeroRight(int val, int len);

private:
	bool isStrSjisSecond(const string &str, int n);
	bool isStrSjisMultiByte(const string &str, int n);
	int getStrValSub(int &val, const string &cstr, int pos, int unitsec);
	int getStrValSubDelimit(int &val, const string &cstr, int pos, int unitsec, DelimtStrType type);
	int getStrItemRange(int &st, int &ed, const string &cstr, int pos, DelimtStrType type);
	CnvStrTime::CharCtrType getCharTypeSub(char ch);
	bool isCharTypeSpace(char ch);

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
	int m_frate_n;				// フレームレート用(初期値=30000)
	int m_frate_d;				// フレームレート用(初期値=1001)
	int m_unitsec;				// 整数単位（0:フレーム 1:ミリ秒）
	string m_delimiter;			// ファイルパスの区切り文字

};
