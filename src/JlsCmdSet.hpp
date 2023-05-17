//
// JLスクリプト用コマンド内容格納データ
//
#pragma once

#include "JlsCmdArg.hpp"

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
	struct ArgLogoList {					// 有効ロゴリスト取得用
		Msec            msec;
		LogoEdgeType    edge;
	};
	struct TargetLocInfoSet {				// 結果リスト保管用
		TargetLocInfo d;
		TargetLocInfo e;
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
	// 有効なロゴ番号リスト
	void            clearLogoList();
	bool            addLogoListStd(Msec msec, LogoEdgeType edge);
	void            addLogoListDirectDummy(bool flag);
	void            addLogoListDirect(Msec msec, LogoEdgeType edgeBase);
	void            attachLogoListOrg(int num, Msec msec, LogoEdgeType edge);
	Msec            getLogoListMsec(int nlist);
	Msec            getLogoListOrgMsec(int nlist);
	LogoEdgeType	getLogoListEdge(int nlist);
	LogoEdgeType	getLogoListOrgEdge(int nlist);
	int				sizeLogoList();
	bool            isLogoListDirect();
	void            forceLogoListStd(bool flag);
private:
	bool            isErrorLogoList(int nlist);
public:
	// 対象とする基準ロゴ選択
	void            clearLogoBase();
	bool			setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge);
	bool			setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge);
	bool            isLogoBaseExist();
	bool            isLogoBaseNrf();
	Nrf				getLogoBaseNrf();
	Nsc				getLogoBaseNsc();
	LogoEdgeType	getLogoBaseEdge();
	// ターゲット選択可能範囲
	void            clearTargetData();
	bool            setTargetRange(WideMsec wmsece, bool fromLogo);
	WideMsec        getTargetRangeWide();
	bool            isTargetRangeFromLogo();
	// ターゲットに一番近い位置
	void            setResultDst(TargetLocInfo tgIn);
	void            setResultEnd(TargetLocInfo tgIn);
private:
	void            setResultSubMake(TargetLocInfo& tgIn);
public:
	TargetLocInfo   getResultDst();
	TargetLocInfo   getResultEnd();
	Nsc             getResultDstNsc();
	LogoEdgeType	getResultDstEdge();
	TargetLocInfo   getResultDstCurrent();
	TargetLocInfo   getResultEndCurrent();
    void            clearPickList();
    void            addPickListCurrent();
    int             sizePickList();
    void            selectPickList(int num);
    bool            isPickListValid();
private:
    TargetLocInfo   getPickListDst();
    TargetLocInfo   getPickListEnd();

private:
	void			signalInternalError(CmdProcessFlag flags);
private:
	RangeMsec		rmsecHeadTail;			// $HEADTIME/$TAILTIME制約
	RangeMsec		rmsecFrameLimit;		// -Fオプション制約
	// 有効なロゴ番号リスト
	vector<ArgLogoList>  listLogoStd;		// 有効ロゴ一覧（ロゴ番号による位置）
	vector<ArgLogoList>  listLogoDir;		// 有効ロゴ一覧（直接フレーム指定位置）
	vector<ArgLogoList>  listLogoOrg;		// 有効ロゴ一覧（直接フレーム指定の本来ロゴ位置）
	bool            forceLogoStdFix;		// 有効ロゴをロゴ番号による位置に強制選択
	bool            existLogoDirDmy;		// 直接フレーム指定（ただし無効位置）存在有無
	// 基準ロゴ
	bool            flagBaseNrf;			// 基準位置は実ロゴ使用
	Nrf				nrfBase;				// 基準位置の実ロゴ番号
	Nsc				nscBase;				// 基準位置の推測構成ロゴ扱い無音シーンチェンジ番号
	LogoEdgeType	edgeBase;				// 基準位置のエッジ選択
	// ターゲット選択可能範囲
	WideMsec		wmsecTarget;			// 対象位置範囲
	bool			fromLogo;				// ロゴ情報からの対象位置範囲
	// ターゲットに一番近い位置
	TargetLocInfo   targetLocDst;			// 対象位置情報
	TargetLocInfo   targetLocEnd;			// End位置情報
	// -pick処理
	int             numPickList;			// 結果保管リストの現在選択番号
	vector<TargetLocInfoSet>  listPickResult;	// 結果保管リスト

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
