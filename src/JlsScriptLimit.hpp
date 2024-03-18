//
// 実行スクリプトコマンドの引数条件からターゲットを絞る
//  出力：
//    JlsCmdSet& cmdset.limit
//
#pragma once

#include "JlsScriptLimVar.hpp"

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
	struct ScrTargetRecord {	// ターゲット位置検索時のデータ
		// 設定
		bool flagNoEdge;		// 全体の先頭と最後のフレームは含めない
		bool flagNextTail;		// NextTailコマンド用
		bool selectLogoRise;	// NextTailコマンドでロゴ立上り優先
		// 結果
		TargetLocInfo tgDst;	// 結果位置
		TargetLocInfo tgEnd;	// 終了位置
		int  numListDst;		// 複数候補の中から選択された番号
		int  numListEnd;		// 複数候補の中から選択された番号
		int  numListTryDst;		// 複数候補の中から選択された番号（候補検索用）
		ScpPriorType statDst;	// 構成の優先順位
		ScpPriorType statEnd;	// 構成の優先順位
		Msec gapDst;			// 中心からの距離
		Msec gapEnd;			// 中心からの距離
		bool flagOnLogo;		// NextTailコマンドのロゴ立上り検出用
	};
	struct ScrOptCRecord {		// 推測構成オプション
		bool exist;
		bool C;
		bool Tra;
		bool Trr;
		bool Trc;
		bool Sp;
		bool Ec;
		bool Bd;
		bool Mx;
		bool Aea;
		bool Aec;
		bool Cm;
		bool Nl;
		bool L;
	};

public:
	JlsScriptLimit(JlsDataset *pdata);
	void limitCommonRange(JlsCmdSet& cmdset);
	void resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec);
	int  limitLogoList(JlsCmdSet& cmdset);
	bool selectTargetByLogo(JlsCmdSet& cmdset, int nlist);
	void selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec);

private:
	//--- コマンド共通の範囲限定 ---
	void limitCustomLogo();
	void limitHeadTail();
	void limitHeadTailImm(RangeMsec rmsec);
	void limitWindow();
	void updateCommonRange(JlsCmdSet& cmdset);
	//--- 有効なロゴ位置リストを取得 ---
	void getLogoListStd(JlsCmdSet& cmdset);
	bool isLogoListStdNumUse(int curNum, int maxNum);
	bool getLogoListStdData(vector<Msec>& listMsecLogoIn, int& locStart, int& locEnd);
	bool getLogoListStdDataRange(int& st, int& ed, vector<Msec>& listMsec, RangeMsec rmsec);
	void getLogoListDirect(JlsCmdSet& cmdset);
	void getLogoListDirectCom(JlsCmdSet& cmdset);
	void getLogoListDirectComOpt(ScrOptCRecord& optC);
	bool getLogoListDirectComOptSub(bool& data, int n);
	bool isLogoListDirectComValid(Nsc nscCur, ScrOptCRecord optC);
	int  getLogoListNearest(JlsCmdSet& cmdset, vector<Msec> listMsec, Msec msecFrom);
	//--- ロゴ位置リスト内の指定ロゴで基準ロゴデータを作成 ---
	bool baseLogo(JlsCmdSet& cmdset, int nlist);
	bool getBaseLogo(JlsCmdSet& cmdset, int nlist);
	void getBaseLogoForTg(WideMsec& wmsecTg, LogoEdgeType& edgeTg, JlsCmdSet& cmdset, bool flagBase);
	bool checkBaseLogo(JlsCmdSet& cmdset);
	bool checkBaseLogoLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN);
	//--- ターゲット範囲を取得 ---
	bool targetRangeByLogo(JlsCmdSet& cmdset);
	void targetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec);
	void updateTargetRange(JlsCmdSet& cmdset, bool fromLogo);
	void addTargetRangeByLogoShift(WideMsec wmsecBase);
	void addTargetRangeData(WideMsec wmsecBase);
	bool findTargetRange(WideMsec& wmsecFind, WideMsec wmsecBase, Msec msecFrom);
	bool findTargetRangeSetBase(WideMsec& wmsecFind, WideMsec& wmsecAnd, WideMsec wmsecBase, Msec msecFrom);
	bool findTargetRangeLimit(WideMsec& wmsecFind, WideMsec& wmsecAnd);
	//--- ターゲット位置を取得 ---
	void targetPoint(JlsCmdSet& cmdset);
	void setTargetPointOutEdge(JlsCmdSet& cmdset);
	void seekTargetPoint(JlsCmdSet& cmdset);
	void seekTargetPointFromScp(JlsCmdSet& cmdset, Nsc nscNow, bool lastNsc);
	bool seekTargetPointEnd(JlsCmdSet& cmdset, Msec msecRef, bool force);
	void seekTargetPointEndRefer(TargetLocInfo& tgEnd, JlsCmdSet& cmdset, Msec msecIn);
	bool seekTargetPointEndScp(JlsCmdSet& cmdset, Msec msecIn, Msec msecDst, Nsc nscAnd);
	void prepTargetPoint(JlsCmdSet& cmdset);
	void prepTargetPointEnd(JlsCmdSet& cmdset);
	bool prepTargetPointEndAbs(TargetLocInfo& tgEnd, bool& multiBase, JlsCmdSet& cmdset);
	//--- 複数処理で使用 ---
	// 基準位置からのロゴ前後幅
	void getWidthLogoFromBase(WideMsec& wmsec, JlsCmdSet& cmdset, int step, bool flagWide);
	void getWidthLogoFromBaseForTarget(WideMsec& wmsec, JlsCmdSet& cmdset, int step, bool flagWide);
	void getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide);

private:
	JlsScriptLimVar var;

private:
	JlsDataset *pdata;									// 入力データアクセス
};
