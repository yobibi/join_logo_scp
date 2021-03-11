//
// join_logo_scp データ格納クラス
//
//
// 変数名共通命名則
//   nlg      : ロゴ番号（立ち上がり／立ち下がり別）
//   nrf      : ロゴ番号（立ち上がり／立ち下がり通算）
//   nsc      : シーンチェンジ番号
//   msec,ms* : 時間ミリ秒単位
//   sec      : 時間秒単位
//   frm      : 時間フレーム単位
//
//
/////////////////////////////////////////////////////////////////////////
#pragma once

#include "CnvStrTime.hpp"


//--- シーンチェンジと構成区切り用 ---
struct DataScpRecord {
// 読込データ
	Msec			msec;				// ミリ秒
	Msec			msbk;				// 終了位置ミリ秒
	Msec			msmute_s;			// シーンチェンジ無音検出開始位置
	Msec			msmute_e;			// シーンチェンジ無音検出終了位置
	bool			still;				// 静止画
// 従来設定用
	ScpPriorType	statpos;			// -1:重複等 0:初期 1:候補 2:確定
// 推測設定用
	int				score;				// 構成区切り候補の推測用
	ScpChapType		chap;				// 構成区切り状態
	ScpArType		arstat;				// 区切りの構成内容
	ScpArExtType	arext;				// 区切りの構成内容の拡張設定
};

//--- ロゴデータ保持用 ---
struct DataLogoRecord {
// 読込データ
	Msec			rise;				// ロゴ各開始フレーム（ミリ秒）
	Msec			fall;				// ロゴ各終了フレーム（ミリ秒）
	Msec			rise_l;				// ロゴ各開始フレーム候補開始（ミリ秒）
	Msec			rise_r;				// ロゴ各開始フレーム候補終了（ミリ秒）
	Msec			fall_l;				// ロゴ各終了フレーム候補開始（ミリ秒）
	Msec			fall_r;				// ロゴ各終了フレーム候補終了（ミリ秒）
	int				fade_rise;			// 各開始フェードイン状態(0 or fadein)
	int				fade_fall;			// 各終了フェードアウト状態(0 or fadeout)
	int				intl_rise;			// インターレース状態(0:ALL 1:TOP 2:BTM)
	int				intl_fall;			// インターレース状態(0:ALL 1:TOP 2:BTM)
// 読み込み元データ保持
	Msec			org_rise;			// ロゴ各開始フレーム（ミリ秒）
	Msec			org_fall;			// ロゴ各終了フレーム（ミリ秒）
	Msec			org_rise_l;			// ロゴ各開始フレーム候補開始（ミリ秒）
	Msec			org_rise_r;			// ロゴ各開始フレーム候補終了（ミリ秒）
	Msec			org_fall_l;			// ロゴ各終了フレーム候補開始（ミリ秒）
	Msec			org_fall_r;			// ロゴ各終了フレーム候補終了（ミリ秒）
// 従来設定調整用
	LogoPriorType	stat_rise;			// -1:はずれ確定 0:初期 1:候補 2:確定
	LogoPriorType	stat_fall;			// -1:はずれ確定 0:初期 1:候補 2:確定
	LogoUnitType	unit;				// 1:独立フレーム
// 従来結果格納用
	LogoResultType	outtype_rise;		// 0:出力未確定  1:出力確定  2:abort破棄確定
	LogoResultType	outtype_fall;		// 0:出力未確定  1:出力確定  2:abort破棄確定
	Msec			result_rise;		// ロゴ各開始確定位置（ミリ秒）
	Msec			result_fall;		// ロゴ各終了確定位置（ミリ秒）
};

/////////////////////////////////////////////////////////////////////////

//--- データ格納用 ---
class JlsDataset
{
private:
	static const int MAXSIZE_LOGO  = 1024;
	static const int MAXSIZE_SCP   = 4096;

	//--- 初期外部入力オプション ---
	struct DtExtOptRecord {
		// オプション
		int		verbose;				// 詳細表示用
		int		frmLastcut;				// オプション -lastcut
		int		msecCutIn;				// オプション -CutMrgIn
		int		msecCutOut;				// オプション -CutMrgOut
		int		wideCutIn;				// CutMrgInの幅選択  0:固定 1:0地点と2点
		int		wideCutOut;				// CutMrgOutの幅選択 0:固定 1:0地点と2点
		// 書き込み有無
		int		fixCutIn;				// 0:CutMrgIn指定なし 1:CutMrgIn指定あり
		int		fixCutOut;				// 0:CutMrgOut指定なし 1:CutMrgOut指定あり
		int		fixWidCutI;				// 0:widecut指定なし 1:widecut指定あり
		int		fixWidCutO;				// 0:widecut指定なし 1:widecut指定あり
		// 内部保持パラメータ
		int		flagNoLogo;				// 0:通常 1:ロゴを読み込まない場合
		int		oldAdjust;				// 0:旧方式調整なし 1:旧方式調整あり
		// 追加
		int     fixVLine;				// 0:vLine指定なし  1:vLine指定あり
		int     fixSubList;				// 0:subDir指定なし 1:subDir指定あり
		int     fixSubPath;				// 0:subPath指定なし 1:subPath指定あり
		int     vLine;					// 読み込み行表示用
		string  subList;				// サブフォルダリスト指定
		string  subPath;				// サブフォルダパス指定
	};
	//--- 各行実行時の保持パラメータ ---
	struct RecordHoldFromCmd {			// コマンドで設定される値で持ち続ける値
		Msec	msecSelect1st;			// 最初の開始位置候補（Select使用時のみ）
		Msec	msecTrPoint;			// CutTRコマンドの設定位置（CM構成内部分割の位置判断用）
		RangeMsec	rmsecHeadTail;		// $HEADTIME,$TAILTIME制約
	};

public:
// 初期設定
	JlsDataset();
	void initData();
// 動作設定の保存・読み出し
	void setConfig(ConfigVarType tp, int val);
	int  getConfig(ConfigVarType tp);
	int  getConfigAction(ConfigActType acttp);
// データサイズ取得
	int  sizeDataLogo();
	int  sizeDataScp();
	bool emptyDataLogo();
// １データセット単位の処理
	void clearRecordLogo(DataLogoRecord &dt);
	void clearRecordScp(DataScpRecord &dt);
	void pushRecordLogo(DataLogoRecord &dt);
	void pushRecordScp(DataScpRecord &dt);
	void popRecordLogo();
	void insertRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void insertRecordScp(DataScpRecord &dt, Nsc nsc);
	void getRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void setRecordLogo(DataLogoRecord &dt, Nlg nlg);
	void getRecordScp(DataScpRecord &dt, Nsc nsc);
	void setRecordScp(DataScpRecord &dt, Nsc nsc);
// １要素単位の処理
	Msec getMsecLogoRise(Nlg nlg);
	Msec getMsecLogoFall(Nlg nlg);
	Msec getMsecLogoNrf(Nrf nrf);
	void getMsecLogoNrfWide(int &msec_c, int &msec_l, int &msec_r, Nrf nrf);
	void getWideMsecLogoNrf(WideMsec &wmsec, Nrf nrf);
	void setMsecLogoNrf(Nrf nrf, Msec val);
	Msec getMsecScp(Nsc nsc);
	Msec getMsecScpBk(Nsc nsc);
	Msec getMsecScpEdge(Nsc nsc, LogoEdgeType edge);
	RangeMsec getRangeMsecFromRangeNsc(RangeNsc rnsc);
	bool				getScpStill(Nsc nsc);
	jlsd::ScpPriorType	getScpStatpos(Nsc nsc);
	int					getScpScore(Nsc nsc);
	jlsd::ScpChapType	getScpChap(Nsc nsc);
	jlsd::ScpArType		getScpArstat(Nsc nsc);
	jlsd::ScpArExtType	getScpArext(Nsc nsc);
	bool				getResultLogoAtNrf(Msec &msec, LogoResultType &outtype, Nrf nrf);
	void				setScpStatpos(Nsc nsc, ScpPriorType val);
	void				setScpScore(Nsc nsc, int val);
	void				setScpChap(Nsc nsc, ScpChapType val);
	void				setScpArstat(Nsc nsc, ScpArType val);
	void				setScpArext(Nsc nsc, ScpArExtType val);
	void				setResultLogoAtNrf(Msec msec, LogoResultType outtype, Nrf nrf);
// 優先度取得
	jlsd::LogoPriorType	getPriorLogo(Nrf nrf);
	jlsd::ScpPriorType	getPriorScp(Nsc nsc);
// 前後データ取得処理（ロゴ）
	Nrf  getNrfDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type);
	Nrf  getNrfPrevLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type);
	Nrf  getNrfNextLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type);
	bool getNrfNextLogoSet(Nrf &nrf_rise, Nrf &nrf_fall, LogoSelectType type);
	Nsc  getNscDirElgForAll(Nsc nsc, SearchDirType dr, LogoEdgeType edge, bool flag_border, bool flag_out);
	Nsc  getNscPrevElg(Nsc nsc, LogoEdgeType edge);
	Nsc  getNscNextElg(Nsc nsc, LogoEdgeType edge);
	bool getNrfptNext(NrfCurrent &logopt, LogoSelectType type);
	bool getElgptNext(ElgCurrent &elg);
	Nlg  getResultLogoNext(Msec &msec_rise, Msec &msec_fall, bool &cont_next, Nlg nlg);
// 前後データ取得処理（無音シーンチェンジ）
	Nsc  getNscDirScpChap(Nsc nsc, SearchDirType dr, ScpChapType chap_th);
	Nsc  getNscPrevScpChap(Nsc nsc, ScpChapType chap_th);
	Nsc  getNscNextScpChap(Nsc nsc, ScpChapType chap_th);
	Nsc  getNscNextScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscNextScpCheckCmUnit(Nsc nsc, ScpEndType noedge);
	Nsc  getNscDirScpDecide(Nsc nsc, SearchDirType dr, ScpEndType noedge);
	Nsc  getNscPrevScpDecide(Nsc nsc, ScpEndType noedge);
	Nsc  getNscNextScpDecide(Nsc nsc, ScpEndType noedge);
	Nsc  getNscNextScpOutput(Nsc nsc, ScpEndType noedge);
// 位置に対応するデータ取得処理
	Nrf  getNrfLogoFromMsec(Msec msec_target, LogoEdgeType edge);
	Nsc  getNscFromNrf(Nrf nrf_target, Msec msec_th, ScpChapType chap_th, bool flat=false);
	Nsc  getNscFromMsecFull(Msec msec_target, Msec msec_th, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscFromMsecChap(Msec msec_target, Msec msec_th, ScpChapType chap_th);
	Nsc  getNscFromMsecAll(Msec msec_target);
	Nsc  getNscFromMsecAllEdgein(Msec msec_target);
	Nsc  getNscFromWideMsecFull(WideMsec wmsec_target, ScpChapType chap_th, ScpEndType noedge);
	Nsc  getNscFromWideMsecByChap(WideMsec wmsec_target, ScpChapType chap_th);
	bool getRangeNscFromRangeMsec(RangeNsc &rnsc, RangeMsec rmsec);
// 状態設定
	void setLevelUseLogo(int level);
	int  getLevelUseLogo();
	void setFlagSetupAdj(bool flag);
	void setFlagAutoMode(bool flag);
// 状態判定
	bool isExistLogo();
	bool isUnuseLogo();
	bool isUnuseLevelLogo();
	bool isSetupAdjInitial();
	bool isAutoModeInitial();
	bool isAutoModeUse();
	bool isAutoLogoOnly();
	bool isValidLogoRise(Nlg nlg);
	bool isValidLogoFall(Nlg nlg);
	bool isValidLogoNrf(Nrf nrf);
	bool isSameLocDirElg(Nsc nsc, SearchDirType dr, LogoEdgeType edge);
	bool isSameLocPrevElg(Nsc nsc, LogoEdgeType edge);
	bool isSameLocNextElg(Nsc nsc, LogoEdgeType edge);
	bool isElgInScp(int nsc);
	bool isElgInScpForAll(Nsc nsc, bool flag_border, bool flag_out);
	bool isScpChapTypeDecideFromNsc(Nsc nsc);
	bool isStillFromMsec(Msec msec_target);
	bool isSmuteFromMsec(Msec msec_target);
	bool isSmuteSameArea(Nsc nsc1, Nsc nsc2);
	bool limitWideMsecFromRange(WideMsec& wmsec, RangeMsec rmsec);
// Term構成処理
	void setTermEndtype(Term &term, ScpEndType endtype);
	bool getTermNext(Term &term);
	bool getTermPrev(Term &term);
	jlsd::ScpArType		getScpArstat(Term term);
	jlsd::ScpArExtType	getScpArext(Term term);
	void setScpArstat(Term term, ScpArType arstat);
	void setScpArext(Term term, ScpArExtType arext);
	bool isScpArstatCmUnit(Term term);
	bool checkScopeTerm(Term term, RangeMsec scope);
	bool checkScopeRange(RangeMsec bounds, RangeMsec scope);
// データ挿入
	Nsc  insertLogo(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit);
	Nsc  insertScpos(Msec msec_dst_s, Msec msec_dst_bk, Nsc nsc_mute, ScpPriorType stat_scpos_dst);
	Nsc  getNscForceMsec(Msec msec_in, LogoEdgeType edge);
// 構成内のロゴ表示期間の取得
	Sec  getSecLogoComponent(Msec msec_s, Msec msec_e);
	Sec  getSecLogoComponentFromLogo(Msec msec_s, Msec msec_e);
// 出力作成
	void outputResultTrimGen();
	void outputResultDetailReset();
	bool outputResultDetailGetLine(string &strBuf);
	void displayLogo();
	void displayScp();
	void setMsecTotalMax(int msec){ m_msecTotalMax = msec; };
	int  getMsecTotalMax(){ return m_msecTotalMax; };

	CnvStrTime		cnv;				// 変換処理
	JlsDataset		*pdata;				// 自分自身へのポインタ

private:
	int getSecLogoComponentFromElg(int msec_s, int msec_e);
	void outputResultTrimGenManual();
	void outputResultTrimGenAuto();
	void outputResultDetailGetLineLabel(string &strBuf, ScpArType arstat, ScpArExtType arext);

public:
	// 固定期間設定
	int		msecValExact;		// 100ms
	int		msecValNear1;		// 180ms
	int		msecValNear2;		// 350ms
	int		msecValNear3;		// 1200ms
	int		msecValLap1;		// 700ms   重複扱い期間（無音SC間隔）
	int		msecValLap2;		// 2500ms  重複扱い期間（構成）
	int		msecValSpc;			// 1200ms  ロゴ検索用

	// 設定値
	DtExtOptRecord				extOpt;			// 外部設定オプション
	RecordHoldFromCmd 			recHold;		// コマンドから決定される設定値
	vector<int>					resultTrim;		// 出力結果（カット位置）
	vector<int>         divFile;    // ファイル分割位置

private:
	// 格納データ
	vector<DataScpRecord>		m_scp;
	vector<DataLogoRecord>		m_logo;
	int							m_config[SIZE_CONFIG_VAR];
	int							m_msecTotalMax;			// 最大フレーム期間
	// セットアップ状態格納
	int		m_levelUseLogo;
	bool	m_flagSetupAdj;
	int		m_flagSetupAuto;
	// 結果出力用
	int		m_nscOutDetail;
};
