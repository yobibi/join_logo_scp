//
// JLスクリプト用コマンド内容格納データ
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// JLスクリプトコマンド設定値
//
///////////////////////////////////////////////////////////////////////
class JlsCmdArg
{
private:
	struct CmdArgTack {						// 設定内容組み合わせから決定される実行用設定
		bool      floatBase;				// 0:ロゴ位置基準  1:結果位置基準
		bool      virtualLogo;				// 0:実際のロゴ  1:推測ロゴ扱いロゴ
		bool      ignoreComp;				// 0:通常  1:ロゴ確定状態でも実行
		bool      limitByLogo;				// 0:通常  1:隣接ロゴまでに制限
		bool      onePoint;					// 0:通常  1:ロゴ１箇所に制限
		bool      needAuto;					// 0:通常  1:Auto構成必要
		bool      fullFrame;				// 0:通常  1:-F系未定義時は常に全体
		LazyType  typeLazy;					// 遅延実行設定種類
		bool      ignoreAbort;				// 0:通常  1:ロゴAbort状態でも実行
	};
	struct CmdArgCond {						// 解析時の状態
		int       numCheckCond;				// 条件式の確認位置（0=不要、1-=確認する引数位置）
		bool      flagCond;					// IF文用の条件判断
	};
	struct CmdArgSc {					// -SC系のオプションデータ
		OptType   type;
		bool      relative;		// false=通常、true=R付き文字列オプション
		Msec      min;
		Msec      max;
	};

	//--- データ保管用サイズ ---
	static const int SIZE_JLOPT_OPTNUM = static_cast<int>(OptType::ArrayMAX) - static_cast<int>(OptType::ArrayMIN) - 1;
	static const int SIZE_JLOPT_OPTSTR = static_cast<int>(OptType::StrMAX) - static_cast<int>(OptType::StrMIN) - 1;

public:
	JlsCmdArg();
	void	clear();
// 一般オプション用
	void   setOpt(OptType tp, int val);
	void   setOptDefault(OptType tp, int val);
	int    getOpt(OptType tp);
	bool   isSetOpt(OptType tp);
// 文字列オプション用
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
// -SC系コマンド用
	void    addScOpt(OptType tp, bool relative, int tmin, int tmax);
	OptType getScOptType(int num);
	bool	isScOptRelative(int num);
	Msec	getScOptMin(int num);
	Msec	getScOptMax(int num);
	int		sizeScOpt();
// -LG系コマンド用
	void	addLgOpt(string strNlg);
	string	getLgOpt(int num);
	int		sizeLgOpt();
// 引数取得
	void   addArgString(const string& strArg);
	bool   replaceArgString(int n, const string& strArg);
	string getStrArg(int n);
	int    getValStrArg(int n);
// IF条件式用
	void setNumCheckCond(int num);
	int  getNumCheckCond();
	void setCondFlag(bool flag);
	bool getCondFlag();

public:
// コマンド
	CmdType             cmdsel;				// コマンド選択
	CmdCat              category;			// 実行時のコマンド種類
	WideMsec			wmsecDst;			// 対象選択範囲
	LogoEdgeType		selectEdge;			// S/E/B
	CmdTrSpEcID         selectAutoSub;			// TR/SP/EC
// 内部状態
	CmdArgTack			tack;				// 設定内容組み合わせから決定される実行用設定
private:
	CmdArgCond			cond;				// 解析時の状態

private:
// 一般オプション保存
	int					optdata[SIZE_JLOPT_OPTNUM];
	bool				flagset[SIZE_JLOPT_OPTNUM];
	string              optStrData[SIZE_JLOPT_OPTSTR];
	bool                flagStrSet[SIZE_JLOPT_OPTSTR];
	bool                flagStrUpdate[SIZE_JLOPT_OPTSTR];
// リスト保存
	vector<string>		listStrArg;	// 引数文字列
	vector<CmdArgSc>	listScOpt;	// -SC系オプション保持
	vector<string>		listLgVal;	// ロゴ番号情報保存
	vector<Msec>		listTLOpt;	// -TLオプション保持
};



///////////////////////////////////////////////////////////////////////
//
// JLスクリプトコマンド設定反映用
//
///////////////////////////////////////////////////////////////////////
class JlsCmdLimit
{
private:
	enum CmdProcessFlag {					// 設定状態記憶用
		ARG_PROCESS_HEADTAIL    = 0x01,
		ARG_PROCESS_FRAMELIMIT  = 0x02,
		ARG_PROCESS_VALIDLOGO   = 0x04,
		ARG_PROCESS_BASELOGO    = 0x08,
		ARG_PROCESS_TARGETRANGE = 0x10,
		ARG_PROCESS_SCPENABLE   = 0x20,
		ARG_PROCESS_RESULT      = 0x40,
	};
	struct ArgValidLogo {					// 有効ロゴリスト取得用
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
	RangeMsec		rmsecHeadTail;			// $HEADTIME/$TAILTIME制約
	RangeMsec		rmsecFrameLimit;		// -Fオプション制約
	vector<ArgValidLogo>	listValidLogo;	// 有効ロゴ位置一覧
	Nrf				nrfBase;				// 基準位置の実ロゴ番号
	Nsc				nscBase;				// 基準位置の推測構成ロゴ扱い無音シーンチェンジ番号
	LogoEdgeType	edgeBase;				// 基準位置のエッジ選択
	vector<WideMsec>	listWmsecLogoBase;	// ロゴ位置情報リスト
	int				locMsecLogoBase;		// ロゴ位置情報リストの基準位置対応番号
	WideMsec		wmsecTarget;			// 対象位置範囲
	Msec			msecTargetFc;			// 強制設定用対象位置
	bool			fromLogo;				// ロゴ情報からの対象位置範囲
	vector<RangeMsec>	listTLRange;		// 対象位置として許可する範囲リスト
	vector<bool>	listScpEnable;			// 無音シーンチェンジ選択
	Nsc				nscSel;					// 対象位置無音シーンチェンジ番号
	Nsc				nscEnd;					// -endlenに対応する無音シーンチェンジ番号
	LogoEdgeType    edgeOutPos;				// 位置出力時のエッジ選択

	int				process;				// 設定状態保持
};



///////////////////////////////////////////////////////////////////////
//
// JLスクリプトコマンド全体
//
///////////////////////////////////////////////////////////////////////
class JlsCmdSet
{
public:
	JlsCmdArg		arg;			// 設定値
	JlsCmdLimit		limit;			// 設定反映
};
