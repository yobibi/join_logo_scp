//
// 実行スクリプトコマンドの引数条件からターゲットを絞る
//  出力：
//    JlsCmdSet& cmdset.limit
//
#pragma once

class JlsCmdArg;
class JlsCmdLimit;
class JlsCmdSet;
class JlsDataset;

///////////////////////////////////////////////////////////////////////
//
// 制約条件によるターゲット選定クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScriptLimit
{
private:
	enum class ScrLogoSetBase {
		None,			// 設定なし
		BaseLoc,		// 基準位置を設定する動作
		ValidList		// 有効ロゴリストを設定する動作
	};
	struct ScrLogoInfoCmdRecord {	// ロゴリスト取得用に使用
		LogoSelectType typeLogo;	// LOGO_SELECT_ALL or LOGO_SELECT_VALID
		ScrLogoSetBase typeSetBase;	// 0=設定なし 1=基準位置の設定 2=有効ロゴリストの設定
		LogoEdgeType edgeSel;		// 検索する場合の立上り／立下り
		Msec msecSel;				// 検索する場合の位置
	};

public:
	JlsScriptLimit(JlsDataset *pdata);
	void limitCommonRange(JlsCmdSet& cmdset);
	void resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec);
	int  limitLogoList(JlsCmdSet& cmdset);
	bool selectTargetByLogo(JlsCmdSet& cmdset, int nlist);
	void selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec, bool force);

private:
	//--- コマンド共通の範囲限定 ---
	void limitHeadTail(JlsCmdSet& cmdset);
	void limitHeadTailImm(JlsCmdSet& cmdset, RangeMsec rmsec);
	void limitWindow(JlsCmdSet& cmdset);
	void limitListForTarget(JlsCmdSet& cmdset);
	//--- ロゴ位置情報リストを取得 ---
	bool limitLogoListSub(JlsCmdArg& cmdarg, int curNum, int maxNum);
	bool limitLogoListRange(int& st, int& ed, vector<WideMsec>& listWmsec, RangeMsec rmsec);
	//--- 指定ロゴの制約を適用 ---
	bool limitTargetLogo(JlsCmdSet& cmdset, int nlist);
	bool limitTargetLogoGet(JlsCmdSet& cmdset, int nlist);
	bool limitTargetLogoCheck(JlsCmdSet& cmdset);
	bool limitTargetLogoCheckLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN);
	bool limitTargetRangeByLogo(JlsCmdSet& cmdset);
	void limitTargetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec, bool force);
	//--- ターゲット位置を取得 ---
	void getTargetPoint(JlsCmdSet& cmdset);
	void getTargetPointOutEdge(JlsCmdSet& cmdset);
	bool getTargetPointEndResult(int& nsc_scpos_end, JlsCmdArg& cmdarg, int msec_base);
	Nsc  getTargetPointEndlen(JlsCmdArg& cmdarg, int msec_base);
	Nsc  getTargetPointEndArg(JlsCmdArg& cmdarg, int msec_base);
	void getTargetPointSetScpEnable(JlsCmdSet& cmdset);
	//--- 複数処理で使用 ---
	bool getLogoInfoList(vector<WideMsec>& listWmsecLogo, JlsCmdSet& cmdset, ScrLogoInfoCmdRecord infoCmd);
	int  getLogoInfoListFind(vector<WideMsec>& listWmsec, Msec msecLogo, LogoEdgeType edge);
	void getLogoInfoListLg(vector<WideMsec>& listWmsec, vector<Nrf>& listNrf, LogoSelectType type);
	void getLogoInfoListElg(vector<WideMsec>& listWmsec, vector<Nsc>& listNsc, bool outflag);
	bool checkOptScpFromMsec(JlsCmdArg &cmdarg, int msec_base, LogoEdgeType edge, bool chk_base, bool chk_rel);

private:
	JlsDataset *pdata;									// 入力データアクセス
};
