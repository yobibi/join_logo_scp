//
// Auto系JLコマンド実行用
//
#pragma once

class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsAutoReform;
class JlsDataset;


///////////////////////////////////////////////////////////////////////
//
// Auto系パラメータ保持クラス
//
///////////////////////////////////////////////////////////////////////
class JlsAutoArg
{
public:
	void	setParam(JlsCmdArg &cmdarg, CmdAutoType cmdtype);
	int		getParam(ParamAuto type);

private:
	void	clearAll();
	void	setVal(ParamAuto type, int val);
	void	setParamCutTR(JlsCmdArg &cmdarg);
	void	setParamCutEC(JlsCmdArg &cmdarg);
	void	setParamAdd(JlsCmdArg &cmdarg);
	void	setParamEdge(JlsCmdArg &cmdarg);
	void	setParamInsDel(JlsCmdArg &cmdarg);
private:
	// コマンド種類
	CmdAutoType m_cmdtype;
	// 格納
	static const int SIZE_PARAM_AUTO = static_cast<int>(ParamAuto::MAXSIZE);

	int m_enable_prm[SIZE_PARAM_AUTO];
	int m_val_prm[SIZE_PARAM_AUTO];
};


///////////////////////////////////////////////////////////////////////
//
// JLスクリプトの自動推測コマンド
//
///////////////////////////////////////////////////////////////////////
class JlsAutoScript
{
private:
// AutoAddで判断する指定位置の前後状態から優先順位算出元データ取得
struct AddLocInfo {
	int typeLogo;			// ロゴからの状態（0-9:ロゴなし 10-19:ロゴあり +100:ロゴ開始側 +1000:前側）
	int typeTr;				// 予告からの状態（0:該当なし 1:後側 2:前側 3:最初の予告位置 4:予告間隙間）
	int typeSp;				// 番組提供からの状態（0:該当なし 1:後側 2:前側 3:内部）
	int typeEc;				// エンドカードからの状態（0:該当なし 1:後側 2:前側）
	Sec secDifLogo;			// ロゴからの時間
	Sec secDifTr;			// 予告からの時間
	Sec secDifSp;			// 番組提供からの時間
	Sec secDifEc;			// エンドカードからの時間（前側の時のみ計測）
	int typeNolast;			// 予告ではなく後に内容がある場合=1
	int typeEndlogo;		// 範囲内に完全に含まれる最後のロゴからの状態（0:該当ロゴなし 1:ロゴ以降 2:ロゴ手前側）
	Sec secDifSc;			// １つ前の番組構成からの時間
};
// AutoAddで構成取得時の情報
struct AddExistInfo {
	bool trailer;			// 予告が存在するか
	bool sponsor;			// 番組提供が存在するか
	Sec  sec_tr;			// 予告時間合計
	bool divideCm;			// CM構成内分割を行う
};

public:
	JlsAutoScript(JlsDataset *pdata);
	virtual ~JlsAutoScript();
	bool startCmd(JlsCmdSet &cmdset, bool setup_only);

private:
	void checkFirstAct(JlsCmdArg &cmdarg);
	bool exeCmdMain(JlsCmdSet &cmdset);
	jlscmd::CmdAutoType exeCmdParam(JlsCmdArg &cmdarg);
	bool startAutoUp();
	bool startAutoBorder(RangeMsec autoscope);
	bool startAutoIClear(RangeMsec autoscope);
	bool startAutoChg(JlsCmdLimit &cmdlimit);
	Nsc  getNscElgFromNrf(Nrf nrf_base);
	bool startAutoIns(JlsCmdLimit &cmdlimit);
	bool startAutoDel(JlsCmdLimit &cmdlimit);
	bool subInsDelGetRange(RangeNscMsec& rangeData, JlsCmdLimit &cmdlimit);
	Nsc  subInsDelGetBase(JlsCmdLimit &cmdlimit);
	void subInsDelAddUnit(RangeMsec rmsec, bool flagAdd);
	void subInsDelChangeArExt(RangeMsec rmsec, bool flagAdd);
	bool startAutoCutTR(RangeMsec autoscope);
	Nsc  subCutTRGetLocSt(RangeMsec autoscope);
	void subCutTRGetLocStSub(Nsc *r_nsc_cand, bool *r_flag_cand, RangeMsec autoscope, ElgCurrent elg);
	void subCutTRSetCut(Term &term);
	bool startAutoCutEC(RangeMsec autoscope);
	int  subCutECGetLocSt(vector<int> &local_cntcut, int *r_ovw_force, RangeMsec autoscope);
	bool subCutECCheckScope(Sec sec_dif, int prm_c_sel, int prm_period, int prm_maxprd);
	bool startAutoAddSP(RangeMsec autoscope);
	bool startAutoAddEC(RangeMsec autoscope);
	bool startAutoAddTR(RangeMsec autoscope);
	int  subAddGetLimit(AddExistInfo &exist_info, CmdAutoType cmdtype, RangeMsec autoscope);
	Nsc  subAddSearch(CmdAutoType cmdtype, AddExistInfo exist_info, RangeMsec autoscope);
	void subAddGetLocInfo(AddLocInfo &locinfo, Term target, RangeMsec autoscope);
	int  subAddGetPriority(AddLocInfo &locinfo, CmdAutoType cmdtype, ScpArType arstat_cur, ScpArExtType arext_cur, AddExistInfo exist_info);
	bool subAddCheckSec(Nsc nsc_cur, Sec difsec);
	void subAddCancelCut(Nsc nsc_prior, RangeMsec autoscope);
	void subAddReviseUnitCm(Nsc nsc_target);
	bool startAutoEdge(JlsCmdLimit &cmdlimit);
	bool startAutoEdgeMain(Nsc nsc_elg, LogoEdgeType edge_elg);
	bool subEdgeExec(Nsc nsc_logo, LogoEdgeType edge_logo, SearchDirType dr);
	bool getElgNextKeep(ElgCurrent &elg);
	int  getConfig(ConfigVarType tp);
	int  getAutoParam(ParamAuto tp);
	void setTermEndtype(Term &term, ScpEndType endtype);
	bool getTermNext(Term &term);
	bool getTermPrev(Term &term);
	ScpArType getScpArstat(Term term);
	ScpArExtType getScpArext(Term term);
	void setScpArstat(Term term, ScpArType arstat);
	void setScpArext(Term term, ScpArExtType arext);
	bool isScpArstatCmUnit(Term term);
	bool checkScopeTerm(Term term, RangeMsec scope);
	bool checkScopeRange(RangeMsec bounds, RangeMsec scope);
	Msec calcDifGap(Msec msec1, Msec msec2);

private:
	//--- データ関数ポインタ ---
	JlsDataset 		*pdata;

	//--- 保持データ ---
	JlsAutoArg		m_autoArg;				// JLコマンド（Auto系）現在行の内容データ
};
