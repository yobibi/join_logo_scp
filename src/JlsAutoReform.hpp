//
// join_logo_scp : Auto系CM構成推測処理
//
#pragma once

class JlsDataset;
class JlsCmdArg;


class JlsAutoReform
{
private:
	//--- 構成構築用 ---
	static const int SIZE_SETFORM_KEEP = 6;		// CM構成検出時の履歴保持数
	//--- ロゴなしCM構成推測用 ---
	enum AutoCMStateType {			// （ロゴなしCM構成推測用）状態
		AUTOCM_ST_S0N_FIRST,		// 先頭CM
		AUTOCM_ST_S1L_LOGO,			// 通常ロゴ
		AUTOCM_ST_S2N_CM,			// 通常CM
		AUTOCM_ST_S3L_DEFER,		// 微妙判断先送りロゴ
		AUTOCM_ST_S4N_JUDGE,		// 不確定先送り地点のCM判断
		AUTOCM_NUM_SMAX				// 状態数最大
	};
	enum AutoCMCommandType {		// （ロゴなしCM構成推測用）コマンド種類
		AUTOCM_T_INIT,				// コマンド：初期化
		AUTOCM_T_SFT,				// コマンド：平行移動
		AUTOCM_T_MRG				// コマンド：合併
	};
	enum AutoFirstType {			// 先頭ロゴのカット条件
		AUTOFIRST_LAX,				// 緩め
		AUTOFIRST_STRICT,			// 厳しめ
		AUTOFIRST_NOCUT				// カットなし
	};

	struct FormAutoCMArg {			// AutoCM実行に使用する入力情報
		int  levelCmDet;			// CM検出レベル
		Msec msecTailareaKeep;		// 短時間はCM扱い継続する最後からの位置
		Msec msecTailareaUnit;		// 60/90/120秒構成をCM扱いする最後からの位置
		Sec  secMaxCm;				// 指定秒を超える構成は15秒単位でもCM扱いにしない
		RangeMsec rmsecHeadTail;	// 先頭末尾位置
	};
	struct AutoCMCount {			// （ロゴなしCM構成推測用）カウンタ
		RangeNsc rnsc;				// 開始終了位置シーンチェンジ番号
		int det;					// 構成数（合計）カウント
		Sec sec;					// 秒数（合計）カウント
		int d15;					// 構成数（15秒）カウント
		int d30;					// 構成数（30秒）カウント
		int dov15;					// 構成数（15秒以上）カウント
	};
	struct FormFirstInfo {			// 先頭カット結果情報
		AutoFirstType limitHead;	// 先頭直後カット方針（0:緩め 1:厳しめ 2:なし）
		Nsc nscTreat;				// 先頭の確定位置番号
	};
	struct FormFirstLoc {			// 先頭カット処理中の位置情報
		Nsc  nscDecide;				// 構成確定位置のシーンチェンジ番号
		Nsc  nscLogo1st;			// ロゴに対応するシーンチェンジ番号
		Msec msecDecide;			// 構成確定位置
		Msec msecLogo1st;			// ロゴ開始位置
		RangeNsc rnscLogo1st;		// ロゴとして検索する範囲シーンチェンジ番号
		int  lvPos1stSel;			// （設定）開始マージン設定
		Msec msec1stSel;			// （設定）開始マージン位置
		Msec msecEnd1st;			// （設定）調査終了位置
		Msec msec1stZone;			// （設定）先頭から指定期間に全体が含まれる構成はCM
		Msec msecWcomp1st;			// （設定）一番最初の構成CM化期間
		Msec msecLgCut1st;			// （設定）一番最初にロゴあり時の構成CM化期間
	};
	struct FormCMByLogo {			// CM構成検出で使用するデータ
		Msec msecTarget;			// 検索ロゴ位置
		Msec msecLimit;				// 補正可能限界地点
		Msec msecCmSide;			// 検索範囲ロゴ位置（CM側）
		Msec msecLogoSide;			// 検索範囲ロゴ位置（ロゴ側）
		int  revDelEdge;			// ロゴ端CM削除（0=可能性ある位置 1=中心位置 2=中心位置で最小限 3=削除なし）
		SearchDirType dr;			// 方向（SEARCH_DIR_PREV/SEARCH_DIR_NEXT）
	};
	struct FormCMEdgeSide {
		bool endSide;				// 端情報（false=開始側, true=終了側）
		Nsc  nscDetect;				// 候補位置
		Nsc  nscFixed;				// 確定位置
		Nsc  nscOther;				// 候補の確定位置とは反対側の確定位置
		bool logoModePrev;			// 前側のロゴ有無
		bool logoModeNext;			// 後側のロゴ有無
	};
	struct FormLogoLevelExt {		// ロゴレベルに対応する認識期間の判定用
		bool en_long;				// 長期間の30秒単位構成を含む
		bool longmod;				// 長期間の5秒/15秒単位を含む
		bool mid5s;					// ロゴレベルMIDLも5秒単位で検出
	};
	struct TraceInterpolar {		// ２点間構成分割時の一時保持データ
		int keepType;				// 保持構成の種類（0=保持なし 1=候補あり 2=ずれの大きい候補あり）
		int keepGap;				// 保持構成場所の誤差ミリ秒
		Nsc nscKeep;				// 保持構成番号
		Nsc nscBase;				// 確定構成番号
		bool keep15s;				// 複数15秒単位構成
	};
	struct TraceChap {					// chap生成時の一時保持データ
		int numDa;						// 入出力：保持データ数（継続候補）
		int numDb;						// 入出力：保持データ数（新候補）
		Nsc nscDa[SIZE_SETFORM_KEEP];	// 入出力：保持データの継続候補位置（サイズ=SIZE_SETFORM_KEEP）
		Nsc nscDb[SIZE_SETFORM_KEEP];	// 入出力：保持データの新候補位置（サイズ=SIZE_SETFORM_KEEP）
	};

public:
	JlsAutoReform(JlsDataset *pdata);
	void mkReformAll(JlsCmdArg &cmdarg);
	bool startAutoCM(JlsCmdArg &cmdarg);
	void mkReformTarget(Nsc nsc_from, Nsc nsc_to, bool logoon, int restruct);

private:
	void mkReformAllLogo();
	bool mkReformAllLogoUnit(Msec &msec_lg_remain_st, NrfCurrent &logopt);
	void mkReformAllLogoUpdate(RangeMsec rmsec_new);
	void mkReformAllNologo(JlsCmdArg &cmdarg);
	void setReformParam(FormAutoCMArg &param_autocm, JlsCmdArg &cmdarg, bool reform_all);
// 構成認識後の追加処理
	void addLogoComponent(int lvlogo);
	void addLogoEdge();
	void addLogoEdgeUpdate(RangeWideMsec area_nologo, int msec_recmin);
	void addLogoMix();
	void addLogoBorder();
	void addLogoBorderSet();
	Msec addLogoBorderSub(RangeMsec component, RangeNrf rnrf_logo, bool enable_st);
	Msec addLogoBorderSubUpdate(RangeMsec component, RangeMsec logo_targets, bool rise_logost, bool enable_st);
// ロゴなし時の構成認識
	bool detectCM(FormAutoCMArg param_autocm);
	void detectCMCount(AutoCMCount *cntset, AutoCMCommandType type, AutoCMStateType st, AutoCMStateType ed);
	bool detectCMSetLogo(RangeNsc rnsc);
	bool detectCMCancelShort(RangeNsc rnsc);
	void detectCMAssistLogo(int nsc_det1st, int nsc_detend);
// 指定区間を推測実行して構成認識
	bool mkRangeInterLogo(RangeNsc nscbounds, bool cm_inter);
	bool mkRangeCM(RangeMsec &rmsec_result, RangeFixMsec scope_org, bool logo1st, int interfill);
	bool mkRangeCMGetLogoEdge(WideMsec &wmsec, int msec_target, bool flag_fix, LogoEdgeType edge);
// 全体における先頭と最後の構成認識処理
	bool setFirstArea(FormFirstInfo &info_first);
	bool setFirstAreaUpdate(FormFirstInfo &info_first, FormFirstLoc locinfo);
	bool setFinalArea();
// 2点間の構成確定処理
	bool setInterLogo(RangeNsc nscbounds, bool cm_inter);
	bool setInterLogoUpdate(Nsc nsc_fin, RangeNsc cmterm, bool shortcm);
	bool setInterMultiCM(RangeNsc nscbounds, int interfill);
	bool setInterpolar(RangeNsc rnsc_target, bool logomode);
	void setInterpolarClearChap(Nsc nsc_n);
	bool setInterpolarDetect(TraceInterpolar &trace, Nsc nsc_cur, RangeNscMsec target, bool logomode, int nside);
	bool setInterpolarExtra(RangeNscMsec target, bool logomode);
// 指定期間内に含まれるCM構成を検出
	bool setCMForm(RangeMsec &bounds, RangeWideMsec cmscope, bool logo1st, bool logointer);
	bool setCMFormDetect(Msec &msec_stpoint, Nsc nsc_base, RangeWideMsec findscope, bool logoon_st);
	void setCMFormDetRevise(RangeMsec &bounds, bool fix_mode);
	bool setCMFormByLogo(RangeMsec &bounds, RangeWideMsec cmscope);
	int  setCMFormByLogoLimit(Msec msec_point, SearchDirType dr);
	bool setCMFormByLogoAdd(Msec &msec_result, FormCMByLogo form);
	bool setCMFormEdge(RangeMsec &rmsec_result, RangeWideMsec cmscope, bool logo1st);
	bool setCMFormEdgeSideInfo(FormCMEdgeSide &sidesel, Msec msec_bounds, WideMsec wmsec_scope, ScpChapType chap_cmp, bool endside);
	bool setCMFormEdgeSetSide(FormCMEdgeSide &sidesel, int level);
	bool setCMFormEdgeSetBoth(RangeNsc rnsc_detect, RangeNsc rnsc_scope, int level, bool logo1st);
// 各区切りのスコアおよび構成を検出
	void setScore(RangeMsec scope);
	void setScore(RangeFixMsec fixscope);
	void setChap(RangeFixMsec fixscope);
	void setChapUpdate(TraceChap &trace, Nsc nsc_cur);
	void setChapGuess(TraceChap &trace, Nsc nsc_cur);
	int  setChapMaxscore(Nsc nsc_target);
	int  setChapGetDistanceScore(Nsc nsc, Msec msec_dif);
	bool setChapFixed(TraceChap &trace);
	bool setChapFixedLimit(TraceChap &trace);
	void setChapThinOverlap(RangeFixMsec fixscope);
	int  getScore(Msec msec_target, RangeMsec scope);
	int  getScore(Msec msec_target, RangeWideMsec range_wide_scope);
	int  getScoreDist(Nsc &nsc_cur, Msec msec_cmp);
	int  getScoreTarget(Msec msec_dist, int sec_width, bool smute_det, bool still_det);
// 共通処理
	int  checkMuteMany(RangeNsc rnsc);
	void addCommonComponentOne(Nsc nsc_target, Nsc nsc_side, bool enable_short);
	int  calcDifSelect(CalcDifInfo &calcdif, Msec msec_src, Msec msec_dst);
	int  calcDifMod0515(CalcModInfo &calcmod, Msec msec_dif);
	RangeMsec getRangeMsec(RangeNsc range_nsc);
	bool isCmLengthMsec(Msec msec_target);
	bool isLengthLogoLevel(Msec msec_target, FormLogoLevelExt extype);

private:
	//--- データ関数ポインタ ---
	JlsDataset *pdata;

};

