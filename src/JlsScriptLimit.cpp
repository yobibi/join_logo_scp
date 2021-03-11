//
// 実行スクリプトコマンドの引数条件からターゲットを絞る
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptLimit.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"

///////////////////////////////////////////////////////////////////////
//
// 制約条件によるターゲット選定クラス
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------
JlsScriptLimit::JlsScriptLimit(JlsDataset *pdata){
	this->pdata  = pdata;
}

//=====================================================================
// コマンド共通の範囲限定
//=====================================================================

//---------------------------------------------------------------------
// コマンド共通の範囲限定
//---------------------------------------------------------------------
void JlsScriptLimit::limitCommonRange(JlsCmdSet& cmdset){
	limitHeadTail(cmdset);						// 全体範囲設定
	limitWindow(cmdset);						// -F系オプション設定
	limitListForTarget(cmdset);					// -TLholdによる範囲設定
}

//---------------------------------------------------------------------
// 範囲の再設定
//---------------------------------------------------------------------
void JlsScriptLimit::resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec){
	//--- 範囲を設定 ---
	limitHeadTailImm(cmdset, rmsec);			// 全体範囲設定
	limitWindow(cmdset);						// -F系オプションと合わせた範囲を再度検索
}

//---------------------------------------------------------------------
// HEADTIME/TAILTIME定義によるフレーム位置限定
// 出力：
//    cmdset.limit.setHeadTail()
//---------------------------------------------------------------------
void JlsScriptLimit::limitHeadTail(JlsCmdSet& cmdset){
	RangeMsec rmsec;
	rmsec.st = pdata->recHold.rmsecHeadTail.st;
	if (rmsec.st == -1){
		rmsec.st = 0;
	}
	rmsec.ed = pdata->recHold.rmsecHeadTail.ed;
	if (rmsec.ed == -1){
		rmsec.ed = pdata->getMsecTotalMax();
	}
	cmdset.limit.setHeadTail(rmsec);
}

//--- 直接数値設定 ---
void JlsScriptLimit::limitHeadTailImm(JlsCmdSet& cmdset, RangeMsec rmsec){
	cmdset.limit.setHeadTail(rmsec);
}

//---------------------------------------------------------------------
// -F系オプションによるフレーム位置限定
// 出力：
//    cmdset.limit.setFrameRange()
//---------------------------------------------------------------------
void JlsScriptLimit::limitWindow(JlsCmdSet& cmdset){
	//--- フレーム制限値を設定 ---
	Msec msec_opt_left  = cmdset.arg.getOpt(OptType::MsecFrameL);
	Msec msec_opt_right = cmdset.arg.getOpt(OptType::MsecFrameR);
	Msec msec_limit_left  = msec_opt_left;
	Msec msec_limit_right = msec_opt_right;
	//--- -FRオプションのフレームを検索し、フレーム制限値を取得 ---
	OptType type_frame = (OptType) cmdset.arg.getOpt(OptType::TypeFrame);
	if (type_frame == OptType::FrFR){
		int nrf_1st_rise = pdata->getNrfNextLogo(-1, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		if (nrf_1st_rise >= 0){
			int msec_tmp = pdata->getMsecLogoNrf(nrf_1st_rise);
			if (msec_limit_left != -1){
				msec_limit_left += msec_tmp;
			}
			if (msec_limit_right != -1){
				msec_limit_right += msec_tmp;
			}
		}
	}
	//--- -F系定義ない場合で、HEADTIME/TAILTIMEがある場合 ---
	if ( cmdset.arg.isSetOpt(OptType::TypeFrame) == false ){
		if ( cmdset.arg.tack.fullFrame == false ){		// 常に全体の場合は除く
			RangeMsec rmsec = pdata->recHold.rmsecHeadTail;
			if ( rmsec.st >= 0 ){	// -HEADTIME定義ある場合
				msec_limit_left = rmsec.st;
			}
			if ( rmsec.ed >= 0 ){	// -TAILTIME定義ある場合
				msec_limit_right = rmsec.ed;
			}
		}
	}
	//--- 中間値制限情報の取得 ---
	bool flag_midext = ( (cmdset.arg.getOpt(OptType::TypeFrameSub) & 0x1) != 0)? true : false;
	// -Fhead,-Ftail,-Fmidでフレーム指定時のフレーム計算
	if (type_frame == OptType::FrFhead ||
		type_frame == OptType::FrFtail ||
		type_frame == OptType::FrFmid){
		//--- head/tail取得 ---
		RangeMsec wmsec_headtail = cmdset.limit.getHeadTail();
		Msec msec_head = wmsec_headtail.st;
		Msec msec_tail = wmsec_headtail.ed;
		//--- 中間地点の取得 ---
		// 最初のロゴ開始から最後のロゴ終了の中間地点を取得
		Nrf nrf_1st_rise = pdata->getNrfNextLogo(-1, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		Nrf nrf_end_fall = pdata->getNrfPrevLogo(pdata->sizeDataLogo()*2, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
		//--- 開始地点検索 ---
		Msec msec_window_start = 0;
		Msec msec_window_midst = 0;
		if (nrf_1st_rise >= 0)  msec_window_midst = pdata->getMsecLogoNrf(nrf_1st_rise);
		if (msec_window_midst < msec_head)  msec_window_midst = msec_head;
		if (msec_window_start < msec_head)  msec_window_start = msec_head;
		//--- 終了地点検索 ---
		Msec msec_window_mided = pdata->getMsecTotalMax();
		Msec msec_window_end   = pdata->getMsecTotalMax();
		if (nrf_end_fall >= 0) msec_window_mided = pdata->getMsecLogoNrf(nrf_end_fall);
		if (msec_window_mided > msec_tail) msec_window_mided = msec_tail;
		if (msec_window_end > msec_tail) msec_window_end = msec_tail;
		//--- 中間地点検索 ---
		Msec msec_window_md = (msec_window_midst +msec_window_mided) / 2;
		//--- フレーム制限範囲を設定 ---
		if (type_frame == OptType::FrFhead){
			msec_limit_left  = msec_window_start + msec_opt_left;
			msec_limit_right = msec_window_start + msec_opt_right;
			if ( msec_opt_left  == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_right == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_right = min(msec_limit_right, msec_window_md);
			}
		}
		else if (type_frame == OptType::FrFtail){
			msec_limit_left  = msec_window_end - msec_opt_right;
			msec_limit_right = msec_window_end - msec_opt_left;
			if ( msec_opt_right == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_left  == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_left = max(msec_limit_left, msec_window_md);
			}
		}
		else if (type_frame == OptType::FrFmid){
			msec_limit_left  = msec_window_start + msec_opt_left;
			msec_limit_right = msec_window_end   - msec_opt_right;
			if ( msec_opt_left  == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_right == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_left  = min(msec_limit_left,  msec_window_md);
				msec_limit_right = max(msec_limit_right, msec_window_md);
			}
		}
	}
	//--- 結果格納 ---
	RangeMsec rmsecLimit = {msec_limit_left, msec_limit_right};
	cmdset.limit.setFrameRange(rmsecLimit);
}


//---------------------------------------------------------------------
// -TgtLimitオプションで対象位置を限定する場合の位置リスト作成
// 出力：
//    cmdset.limit.addTargetList
//---------------------------------------------------------------------
void JlsScriptLimit::limitListForTarget(JlsCmdSet& cmdset){
	//--- オプションなければ何もしない ---
	if (cmdset.arg.isSetOpt(OptType::MsecTgtLimL) == false) return;

	string strList = cmdset.arg.getStrOpt(OptType::StrValListR);	// $LISTHOLD
	if ( strList.empty() == false ){
		cmdset.limit.clearTargetList();		// リスト初期化
		int pos = 0;
		string dstr;
		while ( (pos = pdata->cnv.getStrWord(dstr, strList, pos)) > 0 ){
			int val;
			if (pdata->cnv.getStrValMsecM1(val, dstr, 0) > 0){
				if (val != -1){
					Msec arg_l = (Msec) cmdset.arg.getOpt(OptType::MsecTgtLimL);
					Msec arg_r = (Msec) cmdset.arg.getOpt(OptType::MsecTgtLimR);
					RangeMsec rmsec = {arg_l, arg_r};
					if (arg_l != -1) rmsec.st += val;
					if (arg_r != -1) rmsec.ed += val;
					if (arg_l != -1 || arg_r != -1){
						cmdset.limit.addTargetList(rmsec);
					}
				}
			}
			while (strList[pos] == ',') pos++;
		}
	}
}


//=====================================================================
// ロゴ位置情報リストを取得
//=====================================================================

//---------------------------------------------------------------------
// -N -NR -LGオプションに対応する有効ロゴリストを作成
// 出力：
//   返り値： リスト数
//    cmdset.limit.addLogoList()
//---------------------------------------------------------------------
int JlsScriptLimit::limitLogoList(JlsCmdSet& cmdset){
	//--- ロゴ種類選択 ---
	LogoSelectType typeLogoSel = LOGO_SELECT_ALL;
	bool flagLimitRange = false;
	switch( (OptType)cmdset.arg.getOpt(OptType::TypeNumLogo) ){
		case OptType::LgN:						// -Nオプション
			typeLogoSel = LOGO_SELECT_ALL;
			flagLimitRange = false;
			break;
		case OptType::LgNR:						// -NRオプション
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = false;
			break;
		case OptType::LgNlogo:					// -Nlogoオプション
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = true;
			break;
		case OptType::LgNauto:					// -Nautoオプション
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = true;
			break;
		default:
			break;
	}

	//--- ロゴ位置リストを取得 ---
	vector<WideMsec> listWmsecLogo;
	int locStart;
	int locEnd;
	bool validList = false;
	{
		//--- リスト取得 ---
		ScrLogoInfoCmdRecord infoCmd = {};
		infoCmd.typeLogo = typeLogoSel;
		getLogoInfoList(listWmsecLogo, cmdset, infoCmd);

		//--- リストの有効範囲を限定 ---
		RangeMsec rmsecHeadTail = {-1, -1};
		if ( flagLimitRange ){
			rmsecHeadTail = cmdset.limit.getHeadTail();
		}
		validList = limitLogoListRange(locStart, locEnd, listWmsecLogo, rmsecHeadTail);
		//--- 開始と終了を必ずセットにする場合 ---
		if ( cmdset.arg.getOpt(OptType::FlagPair) > 0 ){
			if ( locStart > 0 && (locStart % 2 != 0) ){
				locStart -= 1;
			}
			if ( locEnd > 0 && (locEnd % 2 == 0) ){
				locEnd += 1;
			}
		}
	}

	//--- -N系オプションの条件に合うロゴ位置を限定・格納 ---
	if ( validList ){
		//--- 設定情報 ---
		LogoEdgeType edgeSel = cmdset.arg.selectEdge;			// コマンドのS/E/B選択
		int maxRise = (locEnd / 2) - ((locStart+1) / 2) + 1;	// rise（偶数）のリスト数
		int maxFall = ((locEnd + 1) / 2) - (locStart / 2);		// fall（奇数）のリスト数
		int curRise = 0;
		int curFall = 0;
		//--- ロゴ番号を順番に確認 ---
		for(int i = locStart; i <= locEnd; i++){
			bool flagLocRise = ( i % 2 == 0 )? true : false;
			if ( flagLocRise && isLogoEdgeRise(edgeSel) ){		// riseエッジ確認
				curRise ++;
				bool result = limitLogoListSub(cmdset.arg, curRise, maxRise);
				if ( result ){
					cmdset.limit.addLogoList(listWmsecLogo[i].just, LOGO_EDGE_RISE);
				}
			}
			bool flagLocFall = ( i % 2 == 1 )? true : false;
			if ( flagLocFall && isLogoEdgeFall(edgeSel) ){		// fallエッジ確認
				curFall ++;
				bool result = limitLogoListSub(cmdset.arg, curFall, maxFall);
				if ( result ){
					cmdset.limit.addLogoList(listWmsecLogo[i].just, LOGO_EDGE_FALL);
				}
			}
		}
	}
	return cmdset.limit.sizeLogoList();
}
// 現在ロゴ番号がオプション指定ロゴ番号かチェック
bool JlsScriptLimit::limitLogoListSub(JlsCmdArg& cmdarg, int curNum, int maxNum){
	bool result = false;
	int numlist = (int) cmdarg.sizeLgOpt();
	if (numlist == 0){				// 指定なければ条件判断は全部有効扱い
		result = true;
	}
	for(int m=0; m<numlist; m++){
		string strVal = cmdarg.getLgOpt(m);
		int rloc = (int)strVal.find("..");
		if ( rloc == (int)string::npos ){		// 通常の数値
			int val = stoi(strVal);
			if ( (val == 0) || (val == curNum) || (maxNum + val + 1 == curNum) ){
				result = true;
			}
		}else{								// 範囲指定
			string strSt = strVal.substr(0, rloc);
			string strEd = strVal.substr(rloc+2);
			int valSt = stoi(strSt);
			int valEd = stoi(strEd);
			if ( valSt < 0 ){
				valSt = maxNum + valSt + 1;
			}
			if ( valEd < 0 ){
				valEd = maxNum + valEd + 1;
			}
			if ( (valSt <= curNum) && (curNum <= valEd) ){
				result = true;
			}
		}
	}
	return result;
}
// リストの有効範囲を限定
bool JlsScriptLimit::limitLogoListRange(int& st, int& ed, vector<WideMsec>& listWmsec, RangeMsec rmsec){
	st = -1;
	ed = -1;
	if ( listWmsec.empty() ){
		return false;
	}
	Msec msecSpc = pdata->msecValSpc;
	bool st1st = true;
	for(int i=0; i < (int)listWmsec.size(); i++){
		Msec msecSel = listWmsec[i].just;
		if ( i % 2 == 0 ){		// ロゴ開始側
			msecSel += msecSpc;
		}else{					// ロゴ終了側
			msecSel -= msecSpc;
		}
		if ( msecSel >= rmsec.st || rmsec.st < 0 ){
			if ( st1st ){
				st = i;
				st1st = false;
			}
		}
		if ( msecSel <= rmsec.ed || rmsec.ed < 0 ){
			ed = i;
		}
	}
	if ( st > ed || st < 0 || ed < 0){		// 範囲存在しない場合
		st = -1;
		ed = -1;
		return false;
	}
	return true;
}


//=====================================================================
// 指定ロゴの制約を適用
//=====================================================================

//---------------------------------------------------------------------
// 対象ロゴについて制約条件を加味して対象位置取得
//---------------------------------------------------------------------
bool JlsScriptLimit::selectTargetByLogo(JlsCmdSet& cmdset, int nlist){
	//--- ロゴ条件を満たす基準ロゴを選択 ---
	bool exeflag = limitTargetLogo(cmdset, nlist);
	//--- 検索対象範囲を設定（基準ロゴ位置をベース） ---
	if (exeflag){
		exeflag = limitTargetRangeByLogo(cmdset);
	}
	//--- ターゲットに一番近いシーンチェンジ位置を取得 ---
	if (exeflag){
		getTargetPoint(cmdset);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// 対象範囲を限定、制約条件を加味して対象位置取得
//---------------------------------------------------------------------
void JlsScriptLimit::selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec, bool force){
	//--- 検索対象範囲を直接数値で設定 ---
	limitTargetRangeByImm(cmdset, wmsec, force);
	//--- 一番近いシーンチェンジ位置を取得 ---
	getTargetPoint(cmdset);
}


//---------------------------------------------------------------------
// 基準ロゴを選択
// 入力：
//    nlist: 有効ロゴリストから選択する番号
// 出力：
//   返り値： 制約満たすロゴ情報判定（false=制約満たさない true=制約満たしロゴ情報取得）
//    cmdset.limit.setLogoBaseNrf()
//    cmdset.limit.setLogoBaseNsc()
//---------------------------------------------------------------------
bool JlsScriptLimit::limitTargetLogo(JlsCmdSet& cmdset, int nlist){
	bool exeflag = limitTargetLogoGet(cmdset, nlist);
	if (exeflag){
		exeflag = limitTargetLogoCheck(cmdset);
	}
	return exeflag;
}

// 基準ロゴ位置を取得
bool JlsScriptLimit::limitTargetLogoGet(JlsCmdSet& cmdset, int nlist){
	//--- コマンド設定情報取得 ---
	Msec msecTarget  = cmdset.limit.getLogoListMsec(nlist);
	LogoEdgeType edgeSel = cmdset.limit.getLogoListEdge(nlist);
	//--- リストがなければ終了 ---
	if ( msecTarget < 0 ) return false;
	//--- 基準位置を取得して設定（取得には無効を含めた全ロゴで検索） ---
	ScrLogoInfoCmdRecord infoCmd = {};
	infoCmd.typeLogo = LOGO_SELECT_ALL;	// 全ロゴ
	infoCmd.typeSetBase = ScrLogoSetBase::BaseLoc;		// 基準位置を設定する動作
	infoCmd.edgeSel = edgeSel;			// 基準位置のエッジ（立上り／立下り）選択
	infoCmd.msecSel = msecTarget;		// 基準位置のミリ秒情報（対応するロゴ番号を探す）
	vector<WideMsec> listTmp;			// 不使用仮設定
	bool det = getLogoInfoList(listTmp, cmdset, infoCmd);	// 基準位置を取得して設定
	//--- 有効ロゴのみでロゴリストを作成する ---
	if ( det ){
		infoCmd.typeLogo = LOGO_SELECT_VALID;	// 有効ロゴ
		infoCmd.typeSetBase = ScrLogoSetBase::ValidList;	// ロゴリストの設定
		getLogoInfoList(listTmp, cmdset, infoCmd);	// 有効ロゴリストの設定
	}
	return det;
}

// 基準ロゴ位置に対応する条件設定を確認
bool JlsScriptLimit::limitTargetLogoCheck(JlsCmdSet& cmdset){
	//--- コマンド設定情報取得 ---
	Nrf nrf_base = cmdset.limit.getLogoBaseNrf();
	Nsc nsc_base = cmdset.limit.getLogoBaseNsc();
	LogoEdgeType edge_base = cmdset.limit.getLogoBaseEdge();
	bool exeflag = false;
	//--- ロゴ位置を直接設定するコマンドに必要なチェック ---
	if (nrf_base >= 0){
		exeflag = true;
		//--- 確定検出済みロゴか確認 ---
		Msec msec_tmp;
		LogoResultType outtype_rf;
		pdata->getResultLogoAtNrf(msec_tmp, outtype_rf, nrf_base);
		//--- 確定ロゴ位置も検出するコマンドか ---
		bool igncomp = cmdset.arg.tack.ignoreComp;
		if (outtype_rf == LOGO_RESULT_NONE || (outtype_rf == LOGO_RESULT_DECIDE && igncomp)){
		}
		else{
			exeflag = false;
		}
		//--- select用確定候補存在時は除く ---
		if (cmdset.arg.cmdsel == CmdType::Select &&
			cmdset.arg.getOpt(OptType::FlagReset) == 0 &&
			pdata->getPriorLogo(nrf_base) >= LOGO_PRIOR_DECIDE){
			exeflag = false;
		}
	}
	else if (nsc_base >= 0){
		exeflag = true;
	}
	//--- 条件に合うか判別 ---
	if (exeflag){
		//--- フレーム範囲チェック ---
		if ( cmdset.arg.isSetOpt(OptType::TypeFrameSub) &&	// フレーム範囲オプションあり
			(cmdset.arg.getOpt(OptType::TypeFrameSub) & 0x2) == 0 ){	// -FT系ではない
			WideMsec wmsecLg;
			bool flagWide = false;		// 各点中心位置で設定
			cmdset.limit.getLogoWmsecBase(wmsecLg, 0, flagWide);	// 前後は同じロゴ(0)
			RangeMsec rmsecFrame = cmdset.limit.getFrameRange();	// フレーム範囲
			//--- ロゴが範囲内か確認 ---
			if ((rmsecFrame.st > wmsecLg.just && rmsecFrame.st >= 0) ||
				(rmsecFrame.ed < wmsecLg.just && rmsecFrame.ed >= 0)){
				exeflag = false;
			}
		}
		//--- オプションと比較(-LenP, -LenN) ---
		if (exeflag){
			bool flagWide = false;		// 各点中心位置で設定
			WideMsec wmsecLg;
			cmdset.limit.getLogoWmsecBase(wmsecLg, 1, flagWide);	// 前後は隣接(1)
			RangeMsec lenP;
			RangeMsec lenN;
			lenP.st = cmdset.arg.getOpt(OptType::MsecLenPMin);
			lenP.ed = cmdset.arg.getOpt(OptType::MsecLenPMax);
			lenN.st = cmdset.arg.getOpt(OptType::MsecLenNMin);
			lenN.ed = cmdset.arg.getOpt(OptType::MsecLenNMax);
			exeflag = limitTargetLogoCheckLength(wmsecLg, lenP, lenN);
		}
		//--- オプションと比較(-LenPE, -LenNE) ---
		if (exeflag){
			bool flagWide = false;		// 各点中心位置で設定
			WideMsec wmsecLgE;
			cmdset.limit.getLogoWmsecBase(wmsecLgE, 2, flagWide);	// 前後は同エッジ隣接(2)
			RangeMsec lenPE;
			RangeMsec lenNE;
			lenPE.st = cmdset.arg.getOpt(OptType::MsecLenPEMin);
			lenPE.ed = cmdset.arg.getOpt(OptType::MsecLenPEMax);
			lenNE.st = cmdset.arg.getOpt(OptType::MsecLenNEMin);
			lenNE.ed = cmdset.arg.getOpt(OptType::MsecLenNEMax);
			exeflag = limitTargetLogoCheckLength(wmsecLgE, lenPE, lenNE);
		}
	}
	//--- ロゴ位置から-SC系オプションを見る場合の確認 ---
	if (cmdset.arg.tack.floatBase == false && exeflag){
		WideMsec wmsecLg;
		bool flagWide = false;		// 各点中心位置で設定
		cmdset.limit.getLogoWmsecBase(wmsecLg, 0, flagWide);	// 前後は同じロゴ(0)
		bool chk_base = true;
		bool chk_rel  = false;
		exeflag = checkOptScpFromMsec(cmdset.arg, wmsecLg.just, edge_base, chk_base, chk_rel);
	}
	return exeflag;
}
//--- 前後ロゴ間の長さによる制約 ---
bool JlsScriptLimit::limitTargetLogoCheckLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN){
	//--- 前後ロゴまでの長さ ---
	Msec msecDifPrev = wmsecLg.just - wmsecLg.early;
	Msec msecDifNext = wmsecLg.late - wmsecLg.just;
	//--- 端部分の処理 ---
	if ( wmsecLg.early < 0 && wmsecLg.just >= 0 ){
		msecDifPrev = wmsecLg.just;
	}
	if ( wmsecLg.late < 0 && wmsecLg.just >= 0 ){
		msecDifNext = pdata->getMsecTotalMax() - wmsecLg.just;
	}
	//--- -LenP/-LenPE 比較 ---
	bool exeflag = true;
	if ( lenP.st >= 0 ){
		if ( msecDifPrev < lenP.st || msecDifPrev < 0 ){
			exeflag = false;
		}
	}
	if ( lenP.ed >= 0 ){
		if ( msecDifPrev > lenP.ed || msecDifPrev < 0 ){
			exeflag = false;
		}
	}
	//--- -LenN/-LenNE 比較 ---
	if ( lenN.st >= 0 ){
		if ( msecDifNext < lenN.st || msecDifNext < 0 ){
			exeflag = false;
		}
	}
	if ( lenN.ed >= 0 ){
		if ( msecDifNext > lenN.ed || msecDifNext < 0 ){
			exeflag = false;
		}
	}
	return exeflag;
}
//---------------------------------------------------------------------
// 検索対象範囲を設定（基準ロゴ位置をベース）
// 出力：
//   返り値：制約満たす範囲確認（0:該当なし  1:対象範囲取得）
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
bool JlsScriptLimit::limitTargetRangeByLogo(JlsCmdSet& cmdset){
	bool exeflag = true;
	//--- 基準とするロゴデータの位置範囲を読み込み ---
	WideMsec wmsec_lg_org;
	{
		Nrf nrf_base = cmdset.limit.getLogoBaseNrf();
		if (nrf_base >= 0){			// 実際のロゴ基準の場合
			int numSft = 0;
			if (cmdset.arg.getOpt(OptType::FlagFromLast) > 0){	// １つ前のロゴを取る時は逆エッジ
				numSft = -1;
			}
			bool flagWide = false;
			if ( cmdset.arg.getOpt(OptType::FlagWide) > 0 ){	// 可能性範囲で検索
				flagWide = true;
			}
			cmdset.limit.getLogoWmsecBaseShift(wmsec_lg_org, 0, flagWide, numSft);
			if ( wmsec_lg_org.just < 0 ){
				wmsec_lg_org = {};
			}
		}
		else{				// 推測構成をロゴ扱いする場合
			bool flagWide = true;		// 立上り／立下りの違いを吸収
			cmdset.limit.getLogoWmsecBase(wmsec_lg_org, 0, flagWide);
		}
	}
	//--- オプションによる位置選択 ---
	WideMsec wmsec_base;
	{
		if ( cmdset.arg.isSetOpt(OptType::MsecFromAbs) ){
			Msec msec_tmp = cmdset.arg.getOpt(OptType::MsecFromAbs);
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_tmp < 0) exeflag = false;				// 負の値の時はコマンドを無効とする
		}
		else if ( cmdset.arg.isSetOpt(OptType::MsecFromHead) ){
			Msec msec_opt = cmdset.arg.getOpt(OptType::MsecFromHead);
			Msec msec_tmp = cmdset.limit.getHead() + msec_opt;
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_opt < 0) exeflag = false;				// 負の値の時はコマンドを無効とする
		}
		else if ( cmdset.arg.isSetOpt(OptType::MsecFromTail) ){
			Msec msec_opt = cmdset.arg.getOpt(OptType::MsecFromTail);
			Msec msec_tmp = cmdset.limit.getTail() - msec_opt;
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_opt < 0) exeflag = false;				// 負の値の時はコマンドを無効とする
		}
		else{
			wmsec_base = wmsec_lg_org;
		}
	}
	//--- コマンド指定の範囲をフレーム範囲に追加 ---
	WideMsec wmsec_find;
	{
		//--- コマンド指定の範囲を追加 ---
		wmsec_find.just  = wmsec_base.just  + cmdset.arg.wmsecDst.just;	// set point to find
		wmsec_find.early = wmsec_base.early + cmdset.arg.wmsecDst.early;
		wmsec_find.late  = wmsec_base.late  + cmdset.arg.wmsecDst.late;
		//--- shiftコマンド位置反映 ---
		if (cmdset.arg.isSetOpt(OptType::MsecSftC) != 0){
			wmsec_find.just  += cmdset.arg.getOpt(OptType::MsecSftC);
			wmsec_find.early += cmdset.arg.getOpt(OptType::MsecSftL);
			wmsec_find.late  += cmdset.arg.getOpt(OptType::MsecSftR);
		}
		//--- コマンド範囲に-1設定時の変換 ---
		if ( cmdset.arg.wmsecDst.early == -1 ){
			wmsec_find.early = 0;
		}
		if ( cmdset.arg.wmsecDst.late == -1 ){
			wmsec_find.late = pdata->getMsecTotalMax();
		}
		//--- ロゴ候補内に限定するSelectコマンド用の範囲 ---
		if (cmdset.arg.cmdsel == CmdType::Select){
			Msec msec_ext_l = wmsec_lg_org.early + cmdset.arg.getOpt(OptType::MsecLogoExtL);
			Msec msec_ext_r = wmsec_lg_org.late  + cmdset.arg.getOpt(OptType::MsecLogoExtR);
			if (wmsec_find.early > msec_ext_r || wmsec_find.late < msec_ext_l){
				exeflag = false;
			}
			else{
				if (wmsec_find.early < msec_ext_l)  wmsec_find.early = msec_ext_l;
				if (wmsec_find.just  < msec_ext_l)  wmsec_find.just  = msec_ext_l;
				if (wmsec_find.late  > msec_ext_r)  wmsec_find.late  = msec_ext_r;
				if (wmsec_find.just  > msec_ext_r)  wmsec_find.just  = msec_ext_r;
			}
		}
		//--- 前後のロゴ位置以内に範囲限定する場合 ---
		if (cmdset.arg.tack.limitByLogo){
			bool flagWide = true;		// 可能性ある範囲で検索
			WideMsec wmsec_lgpn;
			cmdset.limit.getLogoWmsecBase(wmsec_lgpn, 1, flagWide);	// 前後は隣接(1)
			if (wmsec_lgpn.early >= 0){
				if (wmsec_find.early < wmsec_lgpn.early) wmsec_find.early = wmsec_lgpn.early;
				if (wmsec_find.just  < wmsec_lgpn.early) wmsec_find.just  = wmsec_lgpn.early;
			}
			if (wmsec_lgpn.late >= 0){
				if (wmsec_find.late > wmsec_lgpn.late) wmsec_find.late = wmsec_lgpn.late;
				if (wmsec_find.just > wmsec_lgpn.late) wmsec_find.just = wmsec_lgpn.late;
			}
		}
	}
	//--- フレーム指定範囲内に限定 ---
	if (exeflag){
		RangeMsec rmsec_window = cmdset.limit.getFrameRange();
		exeflag = pdata->limitWideMsecFromRange(wmsec_find, rmsec_window);
	}
	//--- 範囲が存在しなければ無効化 ---
	if (wmsec_find.early > wmsec_find.late){
		exeflag = false;
	}
	//--- 結果を格納 ---
	Msec msec_force = -1;
	if ((cmdset.arg.getOpt(OptType::FlagForce) > 0) ||
		(cmdset.arg.getOpt(OptType::FlagNoForce) > 0)){
		msec_force = wmsec_find.just;
	}
	bool from_logo = true;
	cmdset.limit.setTargetRange(wmsec_find, msec_force, from_logo);

	return exeflag;
}

//--- 直接数値設定 ---
void JlsScriptLimit::limitTargetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec, bool force){
	Msec msec_force = -1;
	if ( force ){
		if ( wmsec.just >= 0 && wmsec.just <= pdata->getMsecTotalMax() ){
			msec_force = wmsec.just;
		}
	}
	bool from_logo = false;
	cmdset.limit.setTargetRange(wmsec, msec_force, from_logo);
}



//=====================================================================
// ターゲット位置を取得
//=====================================================================

//---------------------------------------------------------------------
// ターゲットに一番近いシーンチェンジ位置を取得
// 出力：
//   cmdset.list.setResultTarget() : 選択シーンチェンジ位置
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPoint(JlsCmdSet& cmdset){
	//--- 範囲を取得 ---
	WideMsec wmsec_target = cmdset.limit.getTargetRangeWide();
	LogoEdgeType edge_sel = cmdset.limit.getLogoBaseEdge();

	Nsc nsc_scpos_tag = -1;
	Nsc nsc_scpos_end = -1;
	int flag_noedge = cmdset.arg.getOpt(OptType::FlagNoEdge);

	//--- -SC, -NoSC等オプションに対応するシーンチェンジ有無判定 ---
	getTargetPointSetScpEnable(cmdset);

	//--- NextTailコマンド用 ---
	bool flag_nexttail = false;
	bool flag_logorise = false;
	if (cmdset.arg.cmdsel == CmdType::NextTail){
		flag_nexttail = true;
		flag_noedge = 0;
		edge_sel = LOGO_EDGE_RISE;
		if (cmdset.arg.selectEdge == LOGO_EDGE_RISE){
			flag_logorise = true;
		}
	}
	//--- 一番近いシーンチェンジ位置を取得 ---
	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + flag_noedge;
	int jsize = size_scp - flag_noedge;

	int val_difmin = 0;
	ScpPriorType stat_scpos = SCP_PRIOR_DUPE;
	//--- ロゴからの情報取得用(Nextコマンド用） ---
	Nrf nrf_logo = 0;
	Msec msec_logo = 0;
	bool flag_logo = false;
	//--- 一番近い箇所の探索 ---
	for(int j=jfrom; j<jsize; j++){
		Msec         msec_now = pdata->getMsecScp(j);
		ScpPriorType stat_now = pdata->getPriorScp(j);
		Msec         msec_now_edge = pdata->getMsecScpEdge(j, edge_sel);
		Msec         val_dif = abs(msec_now_edge - wmsec_target.just);
		//--- 対象箇所のオプション制約確認 ---
		if ( (cmdset.limit.getScpEnable(j) && cmdset.limit.isTargetListed(msec_now)) || 
			 (j == size_scp-1 && flag_nexttail) ){
			//--- ロゴからの情報使用時(NextTailコマンド用） ---
			bool flag_now_logo = false;
			if (flag_logorise){
				while (msec_logo + pdata->msecValSpc < msec_now && nrf_logo >= 0){
					nrf_logo = pdata->getNrfNextLogo(nrf_logo, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
					if (nrf_logo >= 0){
						msec_logo = pdata->getMsecLogoNrf(nrf_logo);
					}
				}
				if (abs(msec_logo - msec_now) <= pdata->msecValSpc){
					if ( pdata->isAutoModeUse() == false ||
						 pdata->isScpChapTypeDecideFromNsc(j) ){	// 確定区切り時のみ優先
						flag_now_logo = true;
					}
				}
				if (j == size_scp-1) flag_now_logo = true;			// 最終位置はロゴ扱い
				if (flag_now_logo == false && flag_logo == true){	// 候補と現位置の優先状態判断
					stat_now = SCP_PRIOR_DUPE;
				}
			}
			//--- 最小差分の位置を探索 ---
			if (val_difmin > val_dif || nsc_scpos_tag < 0){
				if (msec_now >= wmsec_target.early && msec_now <= wmsec_target.late){
					//--- 候補状態の確認 ---
					bool chk_stat = false;
					if (stat_now >= stat_scpos || cmdset.arg.getOpt(OptType::FlagFlat) > 0){
						chk_stat = true;
					}
					else if (cmdset.arg.cmdsel == CmdType::Select){
						chk_stat = true;
					}
					else if (j == size_scp-1){						// 最終位置は確定扱い
						chk_stat = true;
					}
					if (chk_stat){
						//--- -EndLenオプション確認 ---
						bool chk_end = getTargetPointEndResult(nsc_scpos_end, cmdset.arg, msec_now);
						if (chk_end){			// End位置を検索して未発見の場合のみ除く
							val_difmin = val_dif;
							nsc_scpos_tag = j;
							stat_scpos = stat_now;
							flag_logo = flag_now_logo;
						}
					}
				}
			}
		}
	}
	//--- 検出できなかった場合でforce適用あればforce位置からEnd地点確認 ---
	if (nsc_scpos_tag < 0){
		Msec msec_force   = cmdset.limit.getTargetRangeForce();
		if (msec_force >= 0){
			getTargetPointEndResult(nsc_scpos_end, cmdset.arg, msec_force);
		}
	}
	//--- 位置出力時用のロゴエッジ選択 ---
	getTargetPointOutEdge(cmdset);
	//--- 結果を格納 ---
	cmdset.limit.setResultTarget(nsc_scpos_tag, nsc_scpos_end);
}

//---------------------------------------------------------------------
// 位置情報出力時のロゴエッジ選択
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPointOutEdge(JlsCmdSet& cmdset){
	LogoEdgeType outEdge = cmdset.limit.getLogoBaseEdge();
	if ( cmdset.arg.getOpt(OptType::FlagEdgeS) ){
		outEdge = LOGO_EDGE_RISE;
	}
	if ( cmdset.arg.getOpt(OptType::FlagEdgeE) ){
		outEdge = LOGO_EDGE_FALL;
	}
	cmdset.limit.setTargetOutEdge(outEdge);
}

//---------------------------------------------------------------------
// -End系オプションに対応するシーンチェンジ位置取得
// 入力：
//   msec_base  : 基準となるフレーム
// 出力：
//   返り値：制約条件適用結果（0=失敗  1=成功）
//   nsc_scpos_end : （制約条件を満たした時のみ更新）対応するシーンチェンジ位置
//---------------------------------------------------------------------
bool JlsScriptLimit::getTargetPointEndResult(int& nsc_scpos_end, JlsCmdArg& cmdarg, int msec_base){
	//--- -EndLenオプション確認 ---
	int nend = -2;
	if ( cmdarg.isSetOpt(OptType::MsecEndlenC) &&
	     cmdarg.getOpt(OptType::MsecEndlenC) != 0){
		nend = getTargetPointEndlen(cmdarg, msec_base);
	}
	else{
		nend = getTargetPointEndArg(cmdarg, msec_base);
	}
	if ( nend >= 0 ){
		nsc_scpos_end = nend;
	}
	if ( nend == -1 ) return false;		// -End系オプション存在かつ検出失敗
	return true;
}

//---------------------------------------------------------------------
// -EndLenオプションに対応するシーンチェンジ位置取得
// 入力：
//   msec_base  : 基準となるフレーム
// 返り値：
//   -1    : 該当なし
//   0以上 : 一致するシーンチェンジ番号
//---------------------------------------------------------------------
Nsc JlsScriptLimit::getTargetPointEndlen(JlsCmdArg& cmdarg, int msec_base){
	Msec msec_endlen_c = msec_base + cmdarg.getOpt(OptType::MsecEndlenC);
	Msec msec_endlen_l = msec_base + cmdarg.getOpt(OptType::MsecEndlenL);
	Msec msec_endlen_r = msec_base + cmdarg.getOpt(OptType::MsecEndlenR);

	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + cmdarg.getOpt(OptType::FlagNoEdge);
	int jsize = size_scp - cmdarg.getOpt(OptType::FlagNoEdge);

	Nsc  nsc_scpos_end = -1;
	Msec val_difmin = 0;
	ScpPriorType stat_scpos = SCP_PRIOR_NONE;
	for(int j=jfrom; j<jsize; j++){
		int msec_now = pdata->getMsecScp(j);
		ScpPriorType stat_now = pdata->getPriorScp(j);
		int val_dif = abs(msec_now - msec_endlen_c);
		if (val_difmin > val_dif || nsc_scpos_end < 0){
			if (msec_now >= msec_endlen_l && msec_now <= msec_endlen_r){
				if (stat_now >= stat_scpos || cmdarg.getOpt(OptType::FlagFlat) > 0){
					val_difmin = val_dif;
					nsc_scpos_end = j;
					stat_scpos = stat_now;
				}
			}
		}
	}
	return nsc_scpos_end;
}

//---------------------------------------------------------------------
// End系オプションからの位置取得（-EndLen以外）
// 入力：
//   msec_base  : 基準となるフレーム
// 返り値：
//   -2    : コマンドなし
//   -1    : 指定に対応する位置は該当なし
//   0以上 : 一致するシーンチェンジ番号
//---------------------------------------------------------------------
Nsc JlsScriptLimit::getTargetPointEndArg(JlsCmdArg& cmdarg, int msec_base){
	int  errnum = -2;
	Msec msec_target = 0;
	Msec msec_th = pdata->msecValExact;
	string cstr;

	//--- オプション方法取得 ---
	if ( cmdarg.isSetOpt(OptType::MsecEndAbs) ){			// -EndAbs
		msec_target = cmdarg.getOpt(OptType::MsecEndAbs);
		if (msec_target >= 0){
			errnum = 0;
		}
		else{
			errnum = -1;
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndHead) ){	// -EndHead
		Msec val = pdata->recHold.rmsecHeadTail.st;
		errnum = 0;
		if (val != -1){
			msec_target = val;
		}
		else{
			msec_target = 0;
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndTail) ){	// -EndTail
		Msec val = pdata->recHold.rmsecHeadTail.ed;
		errnum = 0;
		if (val != -1){
			msec_target = val;
		}
		else{
			msec_target = pdata->getMsecTotalMax();
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndHold) ){	// -EndHold
		string strVal = cmdarg.getStrOpt(OptType::StrValPosR);	// $POSHOLD
		int pos = pdata->cnv.getStrValMsecM1(msec_target, strVal, 0);
		if ( pos >= 0 && msec_target >= 0 ){
			errnum = 0;
		}
		else{
			errnum = -1;
		}
	}
	//--- オプション設定 ---
	Nsc nsc_scpos_end;
	if (errnum == 0){
		nsc_scpos_end = pdata->getNscFromMsecFull(
							msec_target, msec_th, SCP_CHAP_NONE, SCP_END_EDGEIN);
	}
	else{
		nsc_scpos_end = errnum;
	}
	return nsc_scpos_end;
}

//---------------------------------------------------------------------
// -SC, -NoSC等オプションに対応するシーンチェンジ有無判定（全無音シーンチェンジ位置で確認）
// 出力：
//   cmdset.limit.setScpEnable()
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPointSetScpEnable(JlsCmdSet& cmdset){
	//--- 更新判断 ---
	int size_scp = pdata->sizeDataScp();
	int size_enable = cmdset.limit.sizeScpEnable();
	if (size_scp == size_enable) return;	// 増減なければ前回から変更なし

	//--- 相対コマンドは常にチェック。通常コマンドは設定によりチェック ---
	bool chk_base = false;
	bool chk_rel  = true;
	if (cmdset.arg.tack.floatBase){
		chk_base = true;
	}
	//--- 全無音シーンチェンジ位置でチェック ---
	vector <bool> list_enable;
	for(int m=0; m<size_scp; m++){
		int msec_base = pdata->getMsecScp(m);
		bool result = checkOptScpFromMsec(cmdset.arg, msec_base, LOGO_EDGE_RISE, chk_base, chk_rel);
		list_enable.push_back(result);
	}
	cmdset.limit.setScpEnable(list_enable);
}



//=====================================================================
// 複数処理で使用
//=====================================================================

//---------------------------------------------------------------------
// ロゴの立上り／立下り位置情報のリストを取得
// 入力：
//   infoCmd  : 取得するデータの設定
// 出力：
//   返り値：取得したいデータ存在（false=存在しない、ture=存在）
//   listWmsecLogo : 各ロゴの位置リスト
//---------------------------------------------------------------------
bool JlsScriptLimit::getLogoInfoList(vector<WideMsec>& listWmsecLogo, JlsCmdSet& cmdset, ScrLogoInfoCmdRecord infoCmd){
	int loc;
	bool det = false;
	if ( cmdset.arg.tack.virtualLogo == false ){
		vector<Nrf>  listNrfLogo;
		//--- 実際のロゴ位置をロゴ番号とする場合のロゴ位置リスト取得 ---
		getLogoInfoListLg(listWmsecLogo, listNrfLogo, infoCmd.typeLogo);
		//--- リスト格納位置に対応するロゴ番号を取得 ---
		loc = getLogoInfoListFind(listWmsecLogo, infoCmd.msecSel, infoCmd.edgeSel);
		//--- ターゲットをBase位置として設定する場合の処理 ---
		if ( infoCmd.typeSetBase == ScrLogoSetBase::BaseLoc ){
			if ( loc >= 0 && loc < (int)listWmsecLogo.size() ){
				if ( listWmsecLogo[loc].just == infoCmd.msecSel ){
					det = true;
					cmdset.limit.setLogoBaseNrf(listNrfLogo[loc], infoCmd.edgeSel);
				}
			}
		}else{
			det = ( listWmsecLogo.empty() )? false : true;
		}
	}else{
		vector<Nsc>  listNscLogo;
		//--- 推測構成変化点をロゴ番号とする場合のロゴ位置リスト取得 ---
		bool flagOut = ( cmdset.arg.getOpt(OptType::FlagFinal) != 0 )? true : false;	// -finalオプション
		getLogoInfoListElg(listWmsecLogo, listNscLogo, flagOut);
		//--- リスト格納位置に対応するロゴ番号を取得 ---
		loc = getLogoInfoListFind(listWmsecLogo, infoCmd.msecSel, infoCmd.edgeSel);
		//--- ターゲットをBase位置として設定する場合の処理 ---
		if ( infoCmd.typeSetBase == ScrLogoSetBase::BaseLoc ){
			if ( loc >= 0 && loc < (int)listWmsecLogo.size() ){
				if ( listWmsecLogo[loc].just == infoCmd.msecSel ){
					det = true;
					cmdset.limit.setLogoBaseNsc(listNscLogo[loc], infoCmd.edgeSel);
				}
			}
		}else{
			det = ( listWmsecLogo.empty() )? false : true;
		}
	}
	//--- ロゴリストを保管 ---
	if ( infoCmd.typeSetBase == ScrLogoSetBase::ValidList ){
		cmdset.limit.setLogoWmsecList(listWmsecLogo, loc);
	}
	return det;
}

//---------------------------------------------------------------------
// 現在位置がロゴリストの何番目か取得
// 返り値：
//   -2    : データなし
//   -1    : 最初の立ち上がりより手前
//   0以上 : ロゴリスト番号（偶数：立上り、奇数：立下り）
//---------------------------------------------------------------------
int JlsScriptLimit::getLogoInfoListFind(vector<WideMsec>& listWmsec, Msec msecLogo, LogoEdgeType edge){
	if ( listWmsec.empty() ){
		return -2;
	}
	int loc = isLogoEdgeRise(edge)? 0 : 1;		// 最初の対応エッジロゴ番号
	int nmax = (int)listWmsec.size();
	bool flagCont = true;
	while( flagCont && (loc+2 < nmax) ){
		if ( listWmsec[loc+2].just <= msecLogo && listWmsec[loc].just < msecLogo ){
			loc += 2;
		}else{
			flagCont = false;
			bool flagInC = ( listWmsec[loc].late >= msecLogo )? true : false;
			bool flagInN = ( listWmsec[loc+2].early <= msecLogo )? true : false;
			if ( flagInC == true && flagInN == false ){
				// 現在領域で次の領域ではない
			}else if ( flagInC == false && flagInN == true ){
				loc += 2;
			}else if ( listWmsec[loc+1].just < msecLogo ){
				loc += 2;
			}else if ( listWmsec[loc+1].just == msecLogo ){
				if ( listWmsec[loc+2].just - msecLogo < msecLogo - listWmsec[loc].just ){
					loc += 2;
				}
			}
		}
	}
	if ( loc == nmax-2 ){		// 最後のfall後のrise判別
		if ( listWmsec[loc+1].just <= msecLogo && listWmsec[loc].just < msecLogo ){
			loc += 2;
		}
	}
	if ( loc == 1 ){			// 最初のrise前のfall判別
		if ( listWmsec[loc-1].just >= msecLogo && listWmsec[loc].just > msecLogo ){
			loc = -1;
		}
	}
	return loc;
}
//---------------------------------------------------------------------
// 実際のロゴ位置のリストを取得
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoInfoListLg(vector<WideMsec>& listWmsec, vector<Nrf>& listNrf, LogoSelectType type){
	listWmsec.clear();		// リスト初期化
	listNrf.clear();		// リスト初期化

	//--- 実際のロゴ位置をロゴ番号とする場合のロゴ位置 ---
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getNrfptNext(logopt, type);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			pdata->getWideMsecLogoNrf(wmsecRise, logopt.nrfRise);
			pdata->getWideMsecLogoNrf(wmsecFall, logopt.nrfFall);
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
			listNrf.push_back(logopt.nrfRise);
			listNrf.push_back(logopt.nrfFall);
		}
	}
}
//---------------------------------------------------------------------
// ロゴ扱い推測構成位置のリストを取得
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoInfoListElg(vector<WideMsec>& listWmsec, vector<Nsc>& listNsc, bool outflag){
	listWmsec.clear();		// リスト初期化
	listNsc.clear();		// リスト初期化

	//--- 推測構成変化点をロゴ番号とする場合のロゴ位置 ---
	ElgCurrent elg = {};
	elg.outflag = outflag;
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getElgptNext(elg);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			Msec msecRiseT  = pdata->getMsecScp(elg.nscRise);
			Msec msecRiseBk = pdata->getMsecScpBk(elg.nscRise);
			Msec msecFallT  = pdata->getMsecScp(elg.nscFall);
			Msec msecFallBk = pdata->getMsecScpBk(elg.nscFall);
			wmsecRise.just  = msecRiseT;	// 中心は立上り基準位置で検索
			wmsecRise.early = msecRiseBk;	// 立下りエッジは少し手前の可能性
			wmsecRise.late  = msecRiseT;
			wmsecFall.just  = msecFallT;	// 中心は立下り基準位置で検索
			wmsecFall.early = msecFallBk;
			wmsecFall.late  = msecFallT;
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
			listNsc.push_back(elg.nscRise);
			listNsc.push_back(elg.nscFall);
		}
	}
}

//---------------------------------------------------------------------
// -SC, -NoSC系オプションに対応するシーンチェンジ有無判定
// 入力：
//   msec_base  : 基準となるフレーム
//   edge      : 0:start edge  1:end edge
//   chk_base  : 通常コマンドの判定実施(false=しない true=する)
//   chk_rel   : 相対位置コマンドの判定実施(false=しない true=する)
// 返り値：
//   false : 一致せず
//   true  : 一致確認
//---------------------------------------------------------------------
bool JlsScriptLimit::checkOptScpFromMsec(JlsCmdArg& cmdarg, int msec_base, LogoEdgeType edge, bool chk_base, bool chk_rel){
	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + cmdarg.getOpt(OptType::FlagNoEdge);
	int jsize = size_scp - cmdarg.getOpt(OptType::FlagNoEdge);
	bool result = true;
	int numlist = cmdarg.sizeScOpt();
	for(int k=0; k<numlist; k++){
		//--- 相対位置コマンド判定処理 ---
		OptType sctype = cmdarg.getScOptType(k);
		if (cmdarg.isScOptRelative(k)){			// 相対位置明示コマンド
			if (chk_rel == false){				// 相対位置チェックしない時は中止
				sctype = OptType::ScNone;
			}
		}
		else{									// 通常設定
			if (chk_base == false){				// 通常設定のチェックでない時は中止
				sctype = OptType::ScNone;
			}
		}
		//--- 対象であればチェック ---
		if (sctype != OptType::ScNone){
			DataScpRecord dt;
			Nsc nsc_scpos_sc   = -1;
			Nsc nsc_smute_all  = -1;
			Nsc nsc_smute_part = -1;
			Nsc nsc_chap_auto  = -1;
			Msec lens_min = cmdarg.getScOptMin(k);
			Msec lens_max = cmdarg.getScOptMax(k);
			for(int j=jfrom; j<jsize; j++){
				pdata->getRecordScp(dt, j);
				int msec_now;
				if ( isLogoEdgeRise(edge) ){
					msec_now = dt.msec;
				}
				else{
					msec_now = dt.msbk;
				}
				if ((msec_now - msec_base >= lens_min || lens_min == -1) &&
					(msec_now - msec_base <= lens_max || lens_min == -1)){
					nsc_scpos_sc = j;
					// for -AC option
					ScpChapType chap_now = dt.chap;
					if (chap_now >= SCP_CHAP_DECIDE || chap_now == SCP_CHAP_CDET){
						nsc_chap_auto = j;
					}
				}
				// 無音系
				int msec_smute_s = dt.msmute_s;
				int msec_smute_e = dt.msmute_e;
				if (msec_smute_s < 0 || msec_smute_e < 0){
					msec_smute_s = msec_now;
					msec_smute_e = msec_now;
				}
				// for -SMA option （無音情報がある場合のみ検出）
				if ((msec_smute_s - msec_base <= lens_min) &&
					(msec_smute_e - msec_base >= lens_max)){
					nsc_smute_all = j;
				}
				//for -SM option
				if ((msec_smute_s - msec_base <= lens_max || lens_max == -1) &&
					(msec_smute_e - msec_base >= lens_min || lens_min == -1)){
					nsc_smute_part = j;
				}
			}
			if (nsc_scpos_sc < 0 && sctype == OptType::ScSC){	// -SC
				result = false;
			}
			else if (nsc_scpos_sc >= 0 && sctype == OptType::ScNoSC){	// -NoSC
				result = false;
			}
			else if (nsc_smute_part < 0 && sctype == OptType::ScSM){	// -SM
				result = false;
			}
			else if (nsc_smute_part >= 0 && sctype == OptType::ScNoSM){	// -NoSM
				result = false;
			}
			else if (nsc_smute_all < 0 && sctype == OptType::ScSMA){	// -SMA
				result = false;
			}
			else if (nsc_smute_all >= 0 && sctype == OptType::ScNoSMA){	// -NoSMA
				result = false;
			}
			else if (nsc_chap_auto < 0 && sctype == OptType::ScAC){	// -AC
				result = false;
			}
			else if (nsc_chap_auto >= 0 && sctype == OptType::ScNoAC){	// -NoAC
				result = false;
			}
		}
		if (result == false){
			break;
		}
	}
	return result;
}

