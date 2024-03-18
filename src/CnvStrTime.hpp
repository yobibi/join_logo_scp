//
// 文字列と時間とフレーム位置の相互変換クラス
//

class CnvStrTime
{
private:
	// 極端に長い文字列の保険破棄用
	static const int SIZE_BUF_MAX   = 16384;
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
		DELIMIT_SPACE_EXNUM,	// 最初の数字部分のみ
		DELIMIT_CSV,			// CSV形式
		DELIMIT_FUNC_NAME,		// 関数の名前部分
		DELIMIT_FUNC_ARGS,		// 関数の引数部分（空白区切り）
		DELIMIT_FUNC_CALC,		// 関数の演算部分（コンマ区切り）
	};
	// 文字の制御用種類
	enum CharCtrType {
		CHAR_CTR_NULL,			// 文字列終了
		CHAR_CTR_CTRL,			// 制御コード
		CHAR_CTR_SPACE,			// 空白
		CHAR_CTR_OTHER			// 通常文字
	};
	// 文字列区切り種類
	struct ArgItemType {
		DelimtStrType dstype;	// 文字の制御用種類
		bool concat;			// 連続quoteの結合
		bool separate;			// quote途切れあれば区切り文字なくても区切り
		bool remain;			// quoteあれば両端に入れる
		bool defstr;			// 定義用の文字列（quote内空リストを残す）
		bool qdisp;				// 途中のquote文字はquote認識している時でも残す
		bool emptyok;			// データなしも許可
	};
	// 文字列内クォート種類
	struct QuoteType {
		bool flagQw;			// "引用中
		bool flagQs;			// '引用中
		bool existQ;			// 端がQUOTE
		bool edgeQw;			// 端のQUOTEに"使用
		int  numPar;			// 括弧の数
	};
	// 文字列内クォート状態
	struct QuoteState {
		bool end;				// 終了予定
		bool add;				// 追加あり
		bool pos;				// 読み込み位置移動あり
	};

public:
	CnvStrTime();
	//--- ファイル名解析 ---
	bool getStrFileAllPath(string &pathname);
	int getStrFilePath(string &pathname, const string &fullname);
	int getStrFilePathName(string &pathname, string &fname, const string &fullname);
	string getStrFileDelimiter();
	//--- 文字列分割 ---
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
	//--- 時間とフレーム位置の変換 ---
	int getFrmFromMsec(Msec msec);
	int getMsecFromFrm(int frm);
	int getMsecAlignFromMsec(Msec msec);
	int getMsecAdjustFrmFromMsec(Msec msec, int frm);
	int getSecFromMsec(Msec msec);
	int changeFrameRate(int n, int d);
	int changeUnitSec(int n);
	//--- 文字列から値取得 ---
	int getStrValNumHead(int &val, const string &cstr, int pos);
	int getStrValNum(int &val, const string &cstr, int pos);
	int getStrValMsec(Msec &val, const string &cstr, int pos);
	int getStrValMsecFromFrm(Msec &val, const string &cstr, int pos);
	int getStrValMsecM1(Msec &val, const string &cstr, int pos);
	int getStrValSec(int &val, const string &cstr, int pos);
	int getStrValSecFromSec(int &val, const string &cstr, int pos);
	int getStrValFuncNum(int &val, const string &cstr, int pos);
	//--- リストデータ取得 ---
	bool getListValMsec(vector<Msec>& listMsec, const string& strList);
	bool getListValMsecM1(vector<Msec>& listMsec, const string& strList);
	//--- 文字列から単語取得 ---
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
	//--- 時間を文字列（フレームまたはミリ秒）に変換 ---
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
	int m_frate_n;				// フレームレート用(初期値=30000)
	int m_frate_d;				// フレームレート用(初期値=1001)
	int m_unitsec;				// 整数単位（0:フレーム 1:ミリ秒）
	string m_delimiter;			// ファイルパスの区切り文字

};
