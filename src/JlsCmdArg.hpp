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
		bool      comFrom;					// 0:通常  1:推測構成from指定
		bool      useScC;					// 0:通常  1:-Cオプション付加
		bool      floatBase;				// 0:ロゴ位置基準  1:結果位置基準
		bool      shiftBase;				// 0:通常  1:シフト基準位置
		bool      virtualLogo;				// 0:実際のロゴ  1:推測ロゴ扱いロゴ
		bool      ignoreComp;				// 0:通常  1:ロゴ確定状態でも実行
		bool      limitByLogo;				// 0:通常  1:隣接ロゴまでに制限
		bool      needAuto;					// 0:通常  1:Auto構成必要
		bool      fullFrameA;				// 0:通常  1:-F系未定義時は常に全体(RANGETYPE=0の時)
		bool      fullFrameB;				// 0:通常  1:-F系未定義時は常に全体(RANGETYPE=1の時)
		LazyType  typeLazy;					// 遅延実行設定種類
		bool      ignoreAbort;				// 0:通常  1:ロゴAbort状態でも実行
		bool      immFrom;					// 0:通常  1:直接フレームfrom指定
		bool      existDstOpt;				// 0:Dst指定なし  1:Dst指定あり
		bool      forcePos;					// 0:通常  1:強制位置設定
		bool      pickIn;					// 0:通常  1:選別して入力
		bool      pickOut;					// 0:通常  1:選別して出力
	};
	struct CmdArgCond {						// 解析時の状態
		int       numCheckCond;				// 条件式の確認位置（0=不要、1-=確認する引数位置）
		bool      flagCond;					// IF文用の条件判断
	};
	struct CmdArgSc {					// -SC系のオプションデータ
		OptType   type;					// オプション種類
		TargetCatType  category;		// 適用対象位置の選択
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
	bool   getOptFlag(OptType tp);
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
	void   signalInternalRegError(string msg, OptType tp);
public:
// -SC系コマンド用
	void    addScOpt(OptType tp, TargetCatType tgcat, int tmin, int tmax);
	OptType getScOptType(int num);
	TargetCatType getScOptCategory(int num);
	Msec	getScOptMin(int num);
	Msec	getScOptMax(int num);
	int		sizeScOpt();
// -LG系コマンド用
	void	addLgOpt(string strNlg);
	string	getLgOpt(int num);
	string	getLgOptAll();
	int		sizeLgOpt();
// 引数取得
	void   addArgString(const string& strArg);
	bool   replaceArgString(int n, const string& strArg);
	string getStrArg(int n);
	int    getValStrArg(int n);
	int    getListStrArgs(vector<string>& listStr);
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
};
