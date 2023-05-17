//
// 実行スクリプトコマンドのターゲットを絞るための変数保持
//
#pragma once

#include "JlsCmdArg.hpp"

//class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsDataset;

///////////////////////////////////////////////////////////////////////
//
// 制約条件によるターゲット選定クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScriptLimVar
{
private:
	struct ArgRange {					// ターゲット位置用
		WideMsec        wmsecFind;		// Dst検索範囲
		WideMsec        wmsecFrom;		// 基準位置（開始／終了位置補正含む）
	};

public:
	struct SeekVarRecord {	// ターゲット位置検索時のデータ
		// 設定
		RangeNsc rnscScp;		// 無音SCの検索範囲
		bool flagNoEdge;		// 全体の先頭と最後のフレームは含めない
		bool flagNextTail;		// NextTailコマンド用
		bool selectLogoRise;	// NextTailコマンドでロゴ立上り優先
		// 結果
		TargetLocInfo tgDst;	// 結果位置
		TargetLocInfo tgEnd;	// 終了位置
		int  numListDst;		// 複数候補の中から選択された番号
		int  numListEnd;		// 複数候補の中から選択された番号
		ScpPriorType statDst;	// 構成の優先順位
		ScpPriorType statEnd;	// 構成の優先順位
		Msec gapDst;			// 中心からの距離
		Msec gapEnd;			// 中心からの距離
		bool flagOnLogo;		// NextTailコマンドのロゴ立上り検出用
	} seek;

public:
	void setPdata(JlsDataset *pdata);
	void clear();
	// コマンド共通の設定
	void initVar(JlsCmdSet& cmdset);
	void setHeadTail(RangeMsec rmsec);
	void setFrameRange(RangeMsec rmsec);
	RangeMsec getHeadTail();
	RangeMsec getFrameRange();
public:
	// ロゴ位置リスト内の指定ロゴで基準ロゴデータを作成
	void clearLogoBase();
	void setLogoBaseListNum(int n);
	void setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge);
	void setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge);
	void setLogoBsrcMsec(Msec msec);
	void setLogoBorgMsec(Msec msec);
	void setLogoBsrcEdge(LogoEdgeType edge);
	void setLogoBtgWmsecEdge(WideMsec wmsec, jlsd::LogoEdgeType edge);
	int  getLogoBaseListNum();
	Msec getLogoBsrcMsec();
	Msec getLogoBorgMsec();
	bool isLogoBaseExist();
	bool isLogoBaseNrf();
	Nrf  getLogoBaseNrf();
	Nsc  getLogoBaseNsc();
	LogoEdgeType    getLogoBaseEdge();
	LogoEdgeType    getLogoBsrcEdge();
	WideMsec        getLogoBtgWmsec();
	LogoEdgeType	getLogoBtgEdge();
	void getWidthLogoFromBase(WideMsec& wmsec, int step, bool flagWide);
	void getWidthLogoFromBaseForTarget(WideMsec& wmsec, int step, bool flagWide);
private:
	void getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide);
public:
	// Dst範囲設定
	void clearRangeDst();
	void addRangeDst(WideMsec wmsecFind, WideMsec wmsecFrom);
	void selRangeDstNum(int num);
	int  sizeRangeDst();
	bool isRangeDstMultiFrom();
	WideMsec getRangeDstWide();
	Msec getRangeDstJust();
	Msec getRangeDstFrom();
	void getRangeDstFromForScp(Msec& msec, Msec& msbk, Nsc& nsc);
private:
	WideMsec getRangeDstItemWide(int num);
	WideMsec getRangeDstItemFromWide(int num);
	bool isErrorRangeDst(int num);
public:
	// 位置検索用の設定
	void initSeekVar(JlsCmdSet& cmdset);
	bool isRangeToDst(Msec msecBsrc, Msec msecDst);
	bool isRangeToEnd(Msec msecDst, Msec msecEnd, WideMsec wmsecRange);
	bool isRangeToEndZone(Msec msecDst, Msec msecEnd);
	// 終了位置の事前準備
	void clearPrepEnd();
	void addPrepEndRange(WideMsec wmsec);
	void setPrepEndRefer(bool flag);
	void setPrepEndAbs(bool fromAbs, bool multiBase, TargetLocInfo tgEnd);
	int  sizePrepEndRange();
	bool isPrepEndRangeExist();
	bool isPrepEndReferExist();
	bool isPrepEndFromAbs();
	bool isPrepEndMultiBase();
	TargetLocInfo getPrepEndAbs();
	WideMsec getPrepEndRangeWithOffset(int num, Msec msecOfs);
	Msec getPrepEndRangeForceLen();
	// Zone範囲
private:
	void clearZone();
	bool isZoneAtDst(Msec msecBsrc, Msec msecDst);
	bool isZoneAtEnd(Msec msecDst, Msec msecEnd);
	bool isZoneBothEnds(Msec msecA, Msec msecB);
	bool isZoneInRange(Msec msecNow, RangeMsec rmsecZone);
	bool isZoneInForbid(Msec msecNow, RangeMsec rmsecZone);
	bool calcZoneRange(RangeMsec& rmsecZone, Msec msecB);
	bool calcZoneRangeSub(RangeMsec& rmsecZone, Msec msecTg, vector<Msec>& listMsec, bool flagL);
	bool calcZoneUnder(RangeMsec& rmsecZone, Msec msecB);
	bool calcZoneOver(RangeMsec& rmsecForbid, Msec msecB);
	void setZoneCache(Msec msecSrc, bool validRange, RangeMsec rmsecRange, RangeMsec rmsecForbid);
	bool getZoneCache(bool& validRange, RangeMsec& rmsecRange, RangeMsec& rmsecForbid, Msec msecSrc);
	bool isTgtLimitAllow(Msec msecTarget);
	int  setTgtLimit();
	// 無音条件判定
public:
	bool isScpEnableAtMsec(int msecBase, LogoEdgeType edge, TargetCatType tgcat);
private:
	bool isScpEnableAtMsecCheck(int msecBase, LogoEdgeType edge, int numOpt);
public:
	bool isScpEnableAtNsc(TargetCatType tgcat, Nsc nsc);
private:
	void clearScpEnable();
	void setScpEnableEveryNsc();
	int  sizeScpEnable(TargetCatType tgcat);
	bool isErrorScpEnable(TargetCatType tgcat, Nsc nsc);

public:
	JlsCmdArg           opt;		// コマンドオプション保持
private:
	// コマンド共通の設定
	RangeMsec           rmsecHeadTail;			// $HEADTIME/$TAILTIME制約
	RangeMsec           rmsecFrameLimit;		// -Fオプション制約
	// ロゴ位置リスト内の指定ロゴで基準ロゴデータを作成
	int             nBaseListNum;			// 基準位置はリスト内で何番目か
	bool            flagBaseNrf;			// 基準位置は実ロゴ使用
	Nrf             nrfBase;				// 基準位置の実ロゴ番号
	Nsc             nscBase;				// 基準位置の推測構成ロゴ扱い無音シーンチェンジ番号
	LogoEdgeType	edgeBase;				// 基準位置のエッジ選択
	Msec            msecBaseBsrc;			// 基準位置（変更後基準位置）
	Msec            msecBaseBorg;			// 基準位置（本来の基準位置、なければ補正）
	LogoEdgeType	edgeBaseBsrc;				// 基準位置のエッジ選択
	WideMsec        wmsecBaseBtg;			// ターゲット範囲作成用の基準位置
	LogoEdgeType	edgeBaseBtg;			// ターゲット範囲作成用の基準位置のエッジ選択
	// Dst範囲設定
	vector<ArgRange>    listRangeDst;
	int                 numRangeDst;
	// END位置事前設定
	vector<WideMsec>    listPrepEndRange;
	bool                existPrepEndRefer;
	bool                fromPrepEndAbs;
	bool                multiPrepEndBase;
	TargetLocInfo       tgPrepEndAbs;
	vector<Msec>        listPrepEndBaseMsec;
	vector<Msec>        listPrepEndBaseMsbk;
	// zone用
	Msec                msecZoneSrc;		// Zoneキャッシュ位置
	bool                validZoneRange;		// Zoneキャッシュ範囲有効
	RangeMsec           rmsecZoneRange;		// Zoneキャッシュ有効範囲
	RangeMsec           rmsecZoneForbid;	// Zoneキャッシュ無効範囲
	vector<RangeMsec>   listTLRange;		// 対象位置として許可する範囲リスト(-TgtLimit)
	// 無音条件判定
	vector<bool>    listScpEnableDst;		// 無音シーンチェンジ選択
	vector<bool>    listScpEnableEnd;		// 無音シーンチェンジ選択

private:
	JlsDataset *pdata;									// 入力データアクセス
};
