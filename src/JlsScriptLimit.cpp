//
// 実行スクリプトコマンドの引数条件からターゲットを絞る
//
//#include "stdafx.h"
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
	var.setPdata(pdata);
}

//=====================================================================
// コマンド共通の範囲限定
//=====================================================================

//---------------------------------------------------------------------
// コマンド共通の範囲限定
//---------------------------------------------------------------------
void JlsScriptLimit::limitCommonRange(JlsCmdSet& cmdset){
	var.initVar(cmdset);				// クラス内変数を初期設定
	limitCustomLogo();					// 設定に合わせたロゴ作成
	limitHeadTail();					// 全体範囲設定
	limitWindow();						// -F系オプション設定
	updateCommonRange(cmdset);			// cmdset.limitを更新
}

//---------------------------------------------------------------------
// 範囲の再設定
//---------------------------------------------------------------------
void JlsScriptLimit::resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec){
	//--- 範囲を設定 ---
	limitHeadTailImm(rmsec);			// 全体範囲設定
	limitWindow();						// -F系オプションと合わせた範囲を再度検索
	updateCommonRange(cmdset);			// cmdset.limitを更新
}

//--- cmdsetに共通設定内容を更新 ---
void JlsScriptLimit::updateCommonRange(JlsCmdSet& cmdset){
	cmdset.limit.setHeadTail( var.getHeadTail() );
	cmdset.limit.setFrameRange( var.getFrameRange() );
}

//---------------------------------------------------------------------
// 設定に合わせたロゴを作成
//---------------------------------------------------------------------
void JlsScriptLimit::limitCustomLogo(){
	//--- 設定値 ---
	LogoCustomType custom = {};
	custom.extLogo = var.opt.tack.virtualLogo;
	custom.selectAll = LOGO_SELECT_VALID;		// 出力リストは通常は有効のみ
	custom.final   = var.opt.getOptFlag(OptType::FlagFinal);
	custom.border  = false;
	if ( var.opt.tack.ignoreAbort ){		// ロゴAbort状態でも実行するコマンドの場合
		custom.selectAll = LOGO_SELECT_ALL;			// 全ロゴをリストに出力
	}
	//--- ロゴを作成・格納 ---
	pdata->makeClogo(custom);
}

//---------------------------------------------------------------------
// HEADTIME/TAILTIME定義によるフレーム位置限定
// 出力：
//    var.setHeadTail()
//---------------------------------------------------------------------
void JlsScriptLimit::limitHeadTail(){
	RangeMsec rmsec;
	rmsec.st = pdata->recHold.rmsecHeadTail.st;
	if (rmsec.st == -1){
		rmsec.st = 0;
	}
	rmsec.ed = pdata->recHold.rmsecHeadTail.ed;
	if (rmsec.ed == -1){
		rmsec.ed = pdata->getMsecTotalMax();
	}
	limitHeadTailImm(rmsec);
}

//--- 直接数値設定 ---
void JlsScriptLimit::limitHeadTailImm(RangeMsec rmsec){
	var.setHeadTail(rmsec);
}

//---------------------------------------------------------------------
// -F系オプションによるフレーム位置限定
// 出力：
//    var.setFrameRange()
//---------------------------------------------------------------------
void JlsScriptLimit::limitWindow(){
	//--- フレーム制限値を設定 ---
	Msec msec_opt_left  = var.opt.getOpt(OptType::MsecFrameL);
	Msec msec_opt_right = var.opt.getOpt(OptType::MsecFrameR);
	Msec msec_limit_left  = msec_opt_left;
	Msec msec_limit_right = msec_opt_right;
	//--- -FRオプションのフレームを検索し、フレーム制限値を取得 ---
	OptType type_frame = (OptType) var.opt.getOpt(OptType::TypeFrame);
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
	if ( var.opt.isSetOpt(OptType::TypeFrame) == false ){
		bool fullFrame = var.opt.tack.fullFrameA;
		if ( pdata->recHold.typeRange == 1 ){		// 変数RANGETYPE=1設定時
			fullFrame = var.opt.tack.fullFrameB;	// 最小限のタイプ
		}
		if ( !fullFrame ){		// 常に全体の場合は除く
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
	bool flag_midext = ( (var.opt.getOpt(OptType::TypeFrameSub) & 0x1) != 0)? true : false;
	// -Fhead,-Ftail,-Fmidでフレーム指定時のフレーム計算
	if (type_frame == OptType::FrFhead ||
		type_frame == OptType::FrFtail ||
		type_frame == OptType::FrFmid){
		//--- head/tail取得 ---
		RangeMsec wmsec_headtail = var.getHeadTail();
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
	var.setFrameRange(rmsecLimit);
}


//=====================================================================
// 有効なロゴ位置リストを取得
//=====================================================================

//---------------------------------------------------------------------
// -N系オプションに対応する有効ロゴリストを作成
// 出力：
//   返り値： リスト数
//    cmdset.limit.addLogoList*()
//---------------------------------------------------------------------
int JlsScriptLimit::limitLogoList(JlsCmdSet& cmdset){
	//--- 有効ロゴリストを初期化 ---
	cmdset.limit.clearLogoList();
	//--- -N系オプションで限定した有効ロゴリスト取得 ---
	getLogoListStd(cmdset);
	//--- 直接位置設定（-from系オプション等）がある場合の追加処理 ---
	getLogoListDirect(cmdset);
	//--- 有効ロゴ数を返す ---
	return cmdset.limit.sizeLogoList();
}
//--- -N系オプションで限定したロゴ位置リストを取得 ---
void JlsScriptLimit::getLogoListStd(JlsCmdSet& cmdset){
	//--- 入力ロゴ取得 ---
	vector<Msec> listMsecLogoIn;		// 入力番号に対応したロゴ格納用
	int locStart;	// 範囲内先頭のロゴリスト番号
	int locEnd;		// 範囲内最後のロゴリスト番号
	if ( getLogoListStdData(listMsecLogoIn, locStart, locEnd) == false ){
		return;
	}
	//--- 使用ロゴ取得 ---
	vector<bool> listUseLogoIn(listMsecLogoIn.size(), false);
	{
		//--- 設定情報 ---
		LogoEdgeType edgeSel = var.opt.selectEdge;			// コマンドのS/E/B選択
		int maxRise = (locEnd / 2) - ((locStart+1) / 2) + 1;	// rise（偶数）のリスト数
		int maxFall = ((locEnd + 1) / 2) - (locStart / 2);		// fall（奇数）のリスト数
		int curRise = 0;
		int curFall = 0;
		//--- ロゴ番号を順番に使用ロゴか確認 ---
		for(int i = locStart; i <= locEnd; i++){
			Msec msecNow = listMsecLogoIn[i];
			LogoEdgeType edgeNow;
			if ( i % 2 == 0 ){
				edgeNow = LOGO_EDGE_RISE;
				curRise ++;
			}else{
				edgeNow = LOGO_EDGE_FALL;
				curFall ++;
			}
			if ( pdata->isClogoMsecExist(msecNow, edgeNow) == false ){
				continue;		// 出力にないロゴは使用しない
			}
			if ( (edgeNow == LOGO_EDGE_RISE) && isLogoEdgeRise(edgeSel) ){	// riseエッジ確認
				if ( isLogoListStdNumUse(curRise, maxRise) ){
					listUseLogoIn[i] = true;		// 使用するロゴ
				}
			}
			if ( (edgeNow == LOGO_EDGE_FALL) && isLogoEdgeFall(edgeSel) ){	// fallエッジ確認
				if ( isLogoListStdNumUse(curFall, maxFall) ){
					listUseLogoIn[i] = true;		// 使用するロゴ
				}
			}
		}
	}
	//--- 使用ロゴを格納 ---
	for(int i=0; i<(int)listMsecLogoIn.size(); i++){
		if ( listUseLogoIn[i] ){
			LogoEdgeType edgeNow = ( i % 2 == 0 )? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
			cmdset.limit.addLogoListStd(listMsecLogoIn[i], edgeNow);
		}
	}
}
// 現在ロゴ番号がオプション指定ロゴ番号かチェック
bool JlsScriptLimit::isLogoListStdNumUse(int curNum, int maxNum){
	if ( var.opt.sizeLgOpt() == 0 ){		// 指定なければ条件判断は全部有効扱い
		return true;
	}
	string strLgNum = var.opt.getLgOptAll();
	return pdata->cnv.isStrMultiNumIn(strLgNum, curNum, maxNum);
}
//--- 設定値に合わせた入力位置を取得 ---
bool JlsScriptLimit::getLogoListStdData(vector<Msec>& listMsecLogoIn, int& locStart, int& locEnd){
	bool typeLogo  = LOGO_SELECT_ALL;
	bool flagLimit = false;
	//--- オプションによるロゴ設定（入力ロゴ番号と必要情報） ---
	switch( (OptType)var.opt.getOpt(OptType::TypeNumLogo) ){
		case OptType::LgN:						// -Nオプション
		case OptType::LgNFXlogo:				// -NFXlogoオプション
			typeLogo  = LOGO_SELECT_ALL;
			flagLimit = false;
			break;
		case OptType::LgNR:						// -NRオプション
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		case OptType::LgNlogo:					// -Nlogoオプション
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = true;
			break;
		case OptType::LgNauto:					// -Nautoオプション
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = true;
			break;
		case OptType::LgNFlogo:					// -NFlogoオプション
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		case OptType::LgNFauto:					// -NFautoオプション
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		default:
			break;
	}
	//--- 対応するロゴデータ取得 ---
	LogoCustomType custom = pdata->getClogoCustom();	// 出力ロゴ設定取得
	custom.selectAll = ( typeLogo == LOGO_SELECT_ALL );
	vector<WideMsec> listWmsec;
	pdata->trialClogo(listWmsec, custom);		// 対応するロゴ作成
	listMsecLogoIn.clear();
	for(int i=0; i<(int)listWmsec.size(); i++){
		listMsecLogoIn.push_back(listWmsec[i].just);
	}
	//--- リストの有効範囲を限定 ---
	RangeMsec rmsecHeadTail = {-1, -1};
	if ( flagLimit ){
		rmsecHeadTail = var.getHeadTail();
	}
	bool validList = getLogoListStdDataRange(locStart, locEnd, listMsecLogoIn, rmsecHeadTail);
	//--- 開始と終了を必ずセットにする場合 ---
	if ( var.opt.getOpt(OptType::FlagPair) > 0 ){
		if ( locStart > 0 && (locStart % 2 != 0) ){
			locStart -= 1;
		}
		if ( locEnd > 0 && (locEnd % 2 == 0) ){
			locEnd += 1;
		}
	}
	return validList;
}
// リストの有効範囲を限定
bool JlsScriptLimit::getLogoListStdDataRange(int& st, int& ed, vector<Msec>& listMsec, RangeMsec rmsec){
	st = -1;
	ed = -1;
	if ( listMsec.empty() ){
		return false;
	}
	Msec msecMgn = pdata->getClogoMsecMgn();
	bool st1st = true;
	for(int i=0; i < (int)listMsec.size(); i++){
		Msec msecSel = listMsec[i];
		if ( i % 2 == 0 ){		// ロゴ開始側
			msecSel += msecMgn;
		}else{					// ロゴ終了側
			msecSel -= msecMgn;
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
//---------------------------------------------------------------------
// 直接位置指定（-from系オプション）処理
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoListDirect(JlsCmdSet& cmdset){
	bool exist = false;
	//--- オプションから直接フレーム位置指定を取得 ---
	vector<Msec> listMsecDirect;
	if ( var.opt.isSetStrOpt(OptType::ListFromAbs) ){			// -fromabs
		exist = true;
		string strList = var.opt.getStrOpt(OptType::ListFromAbs);
		pdata->cnv.getListValMsecM1(listMsecDirect, strList);
	}
	else if ( var.opt.isSetStrOpt(OptType::ListFromHead) ){			// -fromhead
		exist = true;
		RangeMsec rmsecHeadTail = var.getHeadTail();
		string strList = var.opt.getStrOpt(OptType::ListFromHead);
		vector<Msec> listTmp;
		pdata->cnv.getListValMsecM1(listTmp, strList);
		for(int i=0; i<(int)listTmp.size(); i++){
			Msec msec = rmsecHeadTail.st + listTmp[i];
			if ( listTmp[i] < 0 ){		// 無効設定
				msec = listTmp[i];
			}
			listMsecDirect.push_back(msec);
		}
	}
	else if ( var.opt.isSetStrOpt(OptType::ListFromTail) ){			// -fromtail
		exist = true;
		RangeMsec rmsecHeadTail = var.getHeadTail();
		string strList = var.opt.getStrOpt(OptType::ListFromTail);
		vector<Msec> listTmp;
		pdata->cnv.getListValMsecM1(listTmp, strList);
		for(int i=0; i<(int)listTmp.size(); i++){
			Msec msec = rmsecHeadTail.ed - listTmp[i];
			if ( listTmp[i] < 0 ){		// 無効設定
				msec = listTmp[i];
			}
			listMsecDirect.push_back(msec);
		}
	}
	else{
		bool useAbsS = false;
		bool useAbsE = false;
		string strList;
		if ( var.opt.isSetStrOpt(OptType::ListAbsSetFD) ){			// -AbsSetFD
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFD);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFE) ){		// -AbsSetFE
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFE);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFX) ){		// -AbsSetFX
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFX);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetXF) ){		// -AbsSetXF
			useAbsE = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetXF);
		}
		if ( useAbsS || useAbsE ){
			vector<Msec> listTmp;
			pdata->cnv.getListValMsecM1(listTmp, strList);
			for(int i=0; i<(int)listTmp.size(); i++){
				if ( (useAbsS && (i % 2 == 0)) || (useAbsE && (i % 2 == 1)) ){
					listMsecDirect.push_back(listTmp[i]);
				}
			}
		}
	}

	//--- 指定リストが空の時は無効設定 ---
	if ( exist && listMsecDirect.empty() ){
		cmdset.limit.addLogoListDirectDummy(true);
	}
	//--- 直接フレーム位置指定が存在する時は元となる基準ロゴ位置とセットで保管 ---
	int nsizeDir = (int)listMsecDirect.size();
	if ( nsizeDir > 0 ){
		//--- 本来のロゴ位置情報取得 ---
		vector<Msec> listMsecLogo;		// ロゴ位置（全体）
		for(int i=0; i<pdata->sizeClogoList(); i++){
			listMsecLogo.push_back( pdata->getClogoMsecFromNum(i) );
		}
		//--- 各位置を保管 ---
		for(int i=0; i<nsizeDir; i++){
			//--- ロゴ位置を保管 ---
			Msec msecFrom = listMsecDirect[i];
			LogoEdgeType edgeFrom = var.opt.selectEdge;		// コマンドのS/E/B選択
			cmdset.limit.addLogoListDirect(msecFrom, edgeFrom);	// リストに追加
			//--- 各位置それぞれ一番近い基準ロゴ位置を取得 ---
			int locNearest = getLogoListNearest(cmdset, listMsecLogo, msecFrom);
			if ( locNearest >= 0 ){		// 一番近いロゴが存在したら基準ロゴに
				Msec msecLogo = listMsecLogo[locNearest];
				LogoEdgeType edgeLogo = ( locNearest % 2 == 0 )? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
				int locDir = cmdset.limit.sizeLogoList() - 1;	// 追加した位置
				cmdset.limit.attachLogoListOrg(locDir, msecLogo, edgeLogo);
			}
		}
	}
	//--- 推測構成別の開始位置 ---
	getLogoListDirectCom(cmdset);
}
// 推測構成別の開始位置
void JlsScriptLimit::getLogoListDirectCom(JlsCmdSet& cmdset){
	vector<Msec> listMsec;
	vector<LogoEdgeType> listEdge;
	//--- 推測構成区切りによる位置 ---
	ScrOptCRecord optC = {};
	getLogoListDirectComOpt(optC);

	if ( optC.exist ){
		Term term = {};
		pdata->setTermEndtype(term, SCP_END_EDGEIN);	// 端を含めて実施
		pdata->setTermForDisp(term, true);		// 表示用の構成
		RangeMsec rmsecHold;
		bool hold = false;
		bool cont = true;
		while( cont || hold ){
			//--- 次の位置読み込み ---
			if ( cont ){
				cont = pdata->getTermNext(term);
			}
			//--- 目的の構成か判定 ---
			bool valid = false;
			RangeMsec rmsecNow = {-1, -1};
			if ( cont ){
				valid = isLogoListDirectComValid(term.nsc.ed, optC);
				rmsecNow = {term.msec.st, term.msec.ed};
			}
			//--- 結合判定 ---
			if ( valid ){
				if ( !hold ){
					rmsecHold = rmsecNow;	// 新規格納
					hold = true;
					valid = false;
				}else if ( var.opt.getOptFlag(OptType::FlagMerge) &&
						   rmsecHold.ed == rmsecNow.st ){
					rmsecHold.ed = rmsecNow.ed;	// 結合
					valid = false;
				}
			}
			//--- 更新 ---
			if ( hold && (valid || !cont) ){	// データ追加または最後終了時にholdデータを更新
				LogoEdgeType edgeBase = var.opt.selectEdge;	// コマンドのS/E/B選択
				if ( jlsd::isLogoEdgeRise(edgeBase) ){
					listMsec.push_back(rmsecHold.st);
					listEdge.push_back(LOGO_EDGE_RISE);
				}
				if ( jlsd::isLogoEdgeFall(edgeBase) ){
					listMsec.push_back(rmsecHold.ed);
					listEdge.push_back(LOGO_EDGE_FALL);
				}
				if ( valid ){
					rmsecHold = rmsecNow;
				}else{
					hold = false;
				}
			}
		}
		//--- 指定リストが空の時は無効設定 ---
		if ( listMsec.empty() ){
			cmdset.limit.addLogoListDirectDummy(true);
		}
	}
	//--- 直接フレーム位置指定が存在する時はセットで保管 ---
	if ( listMsec.empty() == false ){
		for(int i=0; i<(int)listMsec.size(); i++){
			cmdset.limit.addLogoListDirect(listMsec[i], listEdge[i]);	// リスト追加
		}
	}
}
//--- オプション設定状態取得 ---
void JlsScriptLimit::getLogoListDirectComOpt(ScrOptCRecord& optC){
	int n;
	n = var.opt.getOpt(OptType::FnumFromAllC);		// -fromC
	if ( n == 1 ){
		optC.exist = true;
		optC.Tra = true;
		optC.Trr = true;
		optC.Trc = true;
		optC.Sp  = true;
		optC.Ec  = true;
		optC.Bd  = true;
		optC.Mx  = true;
		optC.Aea = true;
		optC.Aec = true;
		optC.Cm  = true;
		optC.Nl  = true;
		optC.L   = true;
	}
	else if ( n == 2 ){
		optC.exist = true;
	}

	n = var.opt.getOpt(OptType::FnumFromTr);			// -fromTR
	optC.exist |= getLogoListDirectComOptSub(optC.Tra, n);
	optC.exist |= getLogoListDirectComOptSub(optC.Trr, n);

	n = var.opt.getOpt(OptType::FnumFromSp);			// -fromSP
	optC.exist |= getLogoListDirectComOptSub(optC.Sp, n);

	n = var.opt.getOpt(OptType::FnumFromEc);			// -fromEC
	optC.exist |= getLogoListDirectComOptSub(optC.Ec, n);

	n = var.opt.getOpt(OptType::FnumFromBd);			// -fromBD
	optC.exist |= getLogoListDirectComOptSub(optC.Bd, n);

	n = var.opt.getOpt(OptType::FnumFromTra);			// -fromTRa
	optC.exist |= getLogoListDirectComOptSub(optC.Tra, n);

	n = var.opt.getOpt(OptType::FnumFromTrr);			// -fromTRr
	optC.exist |= getLogoListDirectComOptSub(optC.Trr, n);

	n = var.opt.getOpt(OptType::FnumFromCm);			// -fromCM
	optC.exist |= getLogoListDirectComOptSub(optC.Cm, n);

	n = var.opt.getOpt(OptType::FnumFromNl);			// -fromNL
	optC.exist |= getLogoListDirectComOptSub(optC.Nl, n);

	n = var.opt.getOpt(OptType::FnumFromL);				// -fromL
	optC.exist |= getLogoListDirectComOptSub(optC.L, n);
}
bool JlsScriptLimit::getLogoListDirectComOptSub(bool& data, int n){
	if ( n == 1 ){
		data = true;
		return true;
	}else if ( n == 2 ){
		data = false;
		return true;
	}
	return false;
}
// 推測構成がオプション指定の有効対象か判断
bool JlsScriptLimit::isLogoListDirectComValid(Nsc nscCur, ScrOptCRecord optC){
	if ( optC.C ){
		return true;
	}
	bool flagOut = true;
	ComLabelType label = pdata->getLabelTypeFromNsc(nscCur, flagOut);
	bool valid = false;
	if ( optC.Tra ){
		switch( label ){
			case ComLabelType::AddTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Trr ){
		switch( label ){
			case ComLabelType::RawTR :
			case ComLabelType::CanTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Trc ){
		switch( label ){
			case ComLabelType::CutTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Sp ){
		switch( label ){
			case ComLabelType::AddSP :
			case ComLabelType::CutSP :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Ec ){
		switch( label ){
			case ComLabelType::AddEC :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Bd ){
		switch( label ){
			case ComLabelType::Bd :
			case ComLabelType::Bd15s :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Mx ){
		switch( label ){
			case ComLabelType::Mix :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Aea ){
		switch( label ){
			case ComLabelType::AddNEdge :
			case ComLabelType::AddLEdge :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Aec ){
		switch( label ){
			case ComLabelType::CutNEdge :
			case ComLabelType::CutLEdge :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Cm ){
		switch( label ){
			case ComLabelType::CM :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Nl ){
		switch( label ){
			case ComLabelType::NoLogo :
			case ComLabelType::CutNoLogo :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.L ){
		switch( label ){
			case ComLabelType::Logo :
				valid = true;
				break;
			default:
				break;
		}
	}
	return valid;
}

//---------------------------------------------------------------------
// フレーム直接指定位置に一番近い基準ロゴ位置を取得（-from系オプション用）
// 入力：
//    listMsec: ロゴ位置リスト（全体）
//    msecFrom: フレーム直接指定位置
// 出力：
//   返り値： 入力リスト内で一番近い位置
//---------------------------------------------------------------------
int JlsScriptLimit::getLogoListNearest(JlsCmdSet& cmdset, vector<Msec> listMsec, Msec msecFrom){
	//--- 事前確認 ---
	int locNearest = -1;						// 入力リスト内で一番近い位置
	cmdset.limit.forceLogoListStd(true);		// 有効ロゴリスト取得に強制設定
	//--- 有効ロゴリストの各位置の中から一番近い位置を検索 ---
	Msec msecDif = 0;
	bool flagArea = false;
	int  nsize = cmdset.limit.sizeLogoList();		// 有効判定されたロゴ総数
	for(int i=0; i<nsize; i++){
		Msec         msecRef = cmdset.limit.getLogoListMsec(i);		// 有効ロゴリスト内のロゴ基準候補
		LogoEdgeType edgeRef = cmdset.limit.getLogoListEdge(i);
		//--- 各ロゴ区間を取得し前後を含めたロゴ位置を取得 ---
		for(int j=0; j<(int)listMsec.size(); j++){
			if ( msecRef == listMsec[j] ){
				//--- 設定後基準位置と距離測定 ---
				WideMsec wmsec;
				wmsec.early = ( j > 0 )? listMsec[j-1] : 0;
				wmsec.just  = msecRef;
				wmsec.late  = ( j < (int)listMsec.size()-1 )? listMsec[j+1] : pdata->getMsecTotalMax();
				Msec msecTmpDif = abs(msecFrom - wmsec.just);
				//--- ロゴ区間内か判別 ---
				bool flagTmpArea = false;
				if ( jlsd::isLogoEdgeRise(edgeRef) ){		// rise edge
					if ( wmsec.early <= msecFrom && msecFrom < wmsec.late ){
						flagTmpArea = true;
					}
				}else{		// fall edge
					if ( wmsec.early < msecFrom && msecFrom <= wmsec.late ){
						flagTmpArea = true;
					}
				}
				//--- 一番近い位置は更新 ---
				if ( msecDif > msecTmpDif || locNearest < 0 || (flagTmpArea && !flagArea) ){
					//--- ロゴ区間内 または ロゴ区間未発見 ---
					if ( flagTmpArea || !flagArea ){
						locNearest = j;					// リストの位置
						msecDif  = msecTmpDif;
						flagArea = flagTmpArea;
					}
				}
			}
		}
	}
	cmdset.limit.forceLogoListStd(false);		// 有効ロゴリスト取得の強制設定を解除
	return locNearest;
}


//=====================================================================
// ターゲット選定
//=====================================================================

//---------------------------------------------------------------------
// 対象ロゴについて制約条件を加味して対象位置取得
//---------------------------------------------------------------------
bool JlsScriptLimit::selectTargetByLogo(JlsCmdSet& cmdset, int nlist){
	//--- データ初期化 ---
	cmdset.limit.clearLogoBase();
	cmdset.limit.clearTargetData();
	var.clearRangeDst();
	//--- 基準ロゴを確定 ---
	bool exeflag = baseLogo(cmdset, nlist);
	//--- 検索対象範囲を設定（基準ロゴ位置をベース） ---
	if (exeflag){
		exeflag = targetRangeByLogo(cmdset);
	}
	//--- ターゲットに一番近いシーンチェンジ位置を取得 ---
	if (exeflag){
		targetPoint(cmdset);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// 対象範囲を限定、制約条件を加味して対象位置取得
//---------------------------------------------------------------------
void JlsScriptLimit::selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec){
	//--- データ初期化 ---
	cmdset.limit.clearTargetData();
	var.clearRangeDst();
	//--- 検索対象範囲を直接数値で設定 ---
	targetRangeByImm(cmdset, wmsec);
	//--- 一番近いシーンチェンジ位置を取得 ---
	targetPoint(cmdset);
}

//=====================================================================
// ロゴ位置リスト内の指定ロゴで基準ロゴデータを作成
//=====================================================================

//---------------------------------------------------------------------
// 基準ロゴデータを作成
// 入力：
//    nlist: 有効ロゴリストから選択する番号
// 出力：
//   返り値： 制約満たすロゴ情報判定（false=制約満たさない true=制約満たしロゴ情報取得）
//    cmdset.limit.setLogoB*
//---------------------------------------------------------------------
bool JlsScriptLimit::baseLogo(JlsCmdSet& cmdset, int nlist){
	//--- ロゴの基準位置関連を取得 ---
	bool exeflag = getBaseLogo(cmdset, nlist);

	//--- 基準位置が条件を満たすか確認 ---
	if (exeflag){
		exeflag = checkBaseLogo(cmdset);	
	}
	return exeflag;
}
// 基準ロゴを取得
bool JlsScriptLimit::getBaseLogo(JlsCmdSet& cmdset, int nlist){
	//--- コマンド設定情報取得 ---
	Msec         msecBsrc = cmdset.limit.getLogoListMsec(nlist);		// 変更後基準ロゴ位置
	Msec         msecBorg = cmdset.limit.getLogoListOrgMsec(nlist);		// 本来の基準ロゴ位置
	LogoEdgeType edgeBase = cmdset.limit.getLogoListEdge(nlist);
	//--- リスト番号設定 ---
	var.setLogoBaseListNum(nlist);

	if ( var.opt.getOptFlag(OptType::FlagSftLogo) ){	// 基準位置を本来の位置に(-SftLogo)
		msecBsrc = msecBorg;
	}
	if ( msecBsrc < 0 ) return false;	// 無効位置確認

	//--- 今回の基準としてロゴ情報を設定 ---
	if ( pdata->isClogoReal() ){
		Nrf nrfBase = pdata->getClogoRealNrf(msecBorg, edgeBase);
		var.setLogoBaseNrf(nrfBase, edgeBase);		// 実際ロゴで基準設定
		cmdset.limit.setLogoBaseNrf(nrfBase, edgeBase);	// 共通領域に設定
	}else{
		Nsc nscBase = pdata->getClogoNsc(msecBorg);
		var.setLogoBaseNsc(nscBase, edgeBase);		// 推測ロゴで基準設定
		cmdset.limit.setLogoBaseNsc(nscBase, edgeBase);	// 共通領域に設定
	}
	Msec msecBmod = msecBorg;
	if ( msecBmod < 0 ){
		msecBmod = pdata->getClogoMsecNear(msecBsrc, edgeBase);	// 存在しない時は変更後に一番近い位置
	}
	var.setLogoBsrcMsec(msecBsrc);		// 変更後基準ロゴ位置を設定
	var.setLogoBorgMsec(msecBmod);		// 本来の基準ロゴ位置を設定（存在しない時は近くに変更）

	//--- ターゲット選択領域の計算に使用するロゴ情報を設定 ---
	WideMsec wmsecTg;
	wmsecTg.just  = msecBsrc;
	wmsecTg.early = msecBsrc;
	wmsecTg.late  = msecBsrc;
	LogoEdgeType edgeTg = edgeBase;
	bool flagBase = ( msecBsrc == msecBorg );
	getBaseLogoForTg(wmsecTg, edgeTg, cmdset, flagBase);	// オプション等による補正
	if ( wmsecTg.late < 0 ){
		return false;	// 無効位置確認
	}
	var.setLogoBtgWmsecEdge(wmsecTg, edgeTg);	// ターゲット計算用の基準位置を設定
	return true;
}
// ターゲット選択領域の計算に使用するロゴ情報を補正
void JlsScriptLimit::getBaseLogoForTg(WideMsec& wmsecTg, LogoEdgeType& edgeTg, JlsCmdSet& cmdset, bool flagBase){
	//--- コマンド設定情報取得 ---
	Msec msecMgn  = pdata->getClogoMsecMgn();
	Msec msecBsrc = wmsecTg.just;
	if ( msecBsrc < 0 ){
		return;
	}
	//--- ロゴ単位移動オプション ---
	int numSft = 0;
	if ( var.opt.getOptFlag(OptType::FlagFromLast) ){	// １つ前のロゴを取る(-fromlast)
		numSft = -1;
	}
	if ( numSft == 0 && !flagBase ){	// ロゴ範囲チェックなければ何もしない
		return;
	}
	//--- ロゴ位置チェック ---
	int locLogo = pdata->getClogoNumNear(msecBsrc, edgeTg);	// 基準ロゴ位置
	if ( flagBase ){
		wmsecTg = pdata->getClogoWmsecFromNum(locLogo+numSft);
	}else{
		if ( numSft < 0 ){
			locLogo = pdata->getClogoNumPrev(msecBsrc-msecMgn, LOGO_EDGE_BOTH);
		}
		wmsecTg = pdata->getClogoWmsecFromNum(locLogo);
	}
	//--- ロゴ範囲チェック ---
	bool flagLogoWide = false;
	if ( var.opt.getOptFlag(OptType::FlagWide) ){	// 可能性範囲で検索(-wide)
		flagLogoWide = true;
	}else if ( pdata->isClogoReal() == false ){	// 実際のロゴではない時も立上り／立下り幅
		flagLogoWide = true;
	}
	if ( !flagLogoWide ){
		wmsecTg.early = wmsecTg.just;
		wmsecTg.late  = wmsecTg.just;
	}
}
// 基準ロゴ位置に対応する条件設定を確認
bool JlsScriptLimit::checkBaseLogo(JlsCmdSet& cmdset){
	//--- コマンド設定情報取得 ---
	bool flagRealLogo = pdata->isClogoReal();
	Msec msecLogoBsrc = var.getLogoBsrcMsec();		// 設定後基準位置
	Nrf nrf_base = var.getLogoBaseNrf();			// 基準ロゴ位置
//	Nsc nsc_base = var.getLogoBaseNsc();
	bool exeflag = true;
	//--- ロゴ位置を直接設定するコマンドに必要なチェック ---
	if ( flagRealLogo ){
		//--- 確定検出済みロゴか確認 ---
		Msec           msec_tmp   = -1;
		LogoResultType outtype_rf = LOGO_RESULT_DECIDE;	// 基準ロゴがない時は確定時の処理
		if ( nrf_base >= 0 ){
			pdata->getResultLogoAtNrf(msec_tmp, outtype_rf, nrf_base);
		}
		//--- 確定ロゴ位置も検出するコマンドか ---
		bool igncomp = cmdset.arg.tack.ignoreComp;
		bool ignabort = cmdset.arg.tack.ignoreAbort;
		if (outtype_rf == LOGO_RESULT_NONE || (outtype_rf == LOGO_RESULT_DECIDE && igncomp)){
		}
		else if ( outtype_rf == LOGO_RESULT_ABORT && ignabort ){	// Abort無視で実行する場合
		}
		else{
			exeflag = false;
		}
		//--- select用確定候補存在時は除く ---
		if (var.opt.cmdsel == CmdType::Select &&
			var.opt.getOptFlag(OptType::FlagReset) == false &&
			pdata->getPriorLogo(nrf_base) >= LOGO_PRIOR_DECIDE){
			exeflag = false;
		}
	}
	//--- 条件に合うか判別 ---
	if (exeflag){
		//--- フレーム範囲チェック（変更後基準ロゴ位置で確認） ---
		if ( var.opt.isSetOpt(OptType::TypeFrameSub) &&	// フレーム範囲オプションあり
			(var.opt.getOpt(OptType::TypeFrameSub) & 0x2) == 0 ){	// -FT系ではない
			RangeMsec rmsecFrame = var.getFrameRange();	// フレーム範囲
			//--- ロゴが範囲内か確認 ---
			if ((rmsecFrame.st > msecLogoBsrc && rmsecFrame.st >= 0) ||
				(rmsecFrame.ed < msecLogoBsrc && rmsecFrame.ed >= 0)){
				exeflag = false;
			}
		}
		//--- オプションと比較(-LenP, -LenN)（本来の基準ロゴで確認） ---
		if (exeflag){
			bool flagWide = false;		// 各点中心位置で設定
			WideMsec wmsecLg;
			var.getWidthLogoFromBase(wmsecLg, 1, flagWide);	// 前後は隣接(1)
			RangeMsec lenP;
			RangeMsec lenN;
			lenP.st = var.opt.getOpt(OptType::MsecLenPMin);
			lenP.ed = var.opt.getOpt(OptType::MsecLenPMax);
			lenN.st = var.opt.getOpt(OptType::MsecLenNMin);
			lenN.ed = var.opt.getOpt(OptType::MsecLenNMax);
			exeflag = checkBaseLogoLength(wmsecLg, lenP, lenN);
		}
		//--- オプションと比較(-LenPE, -LenNE)（本来の基準ロゴで確認） ---
		if (exeflag){
			bool flagWide = false;		// 各点中心位置で設定
			WideMsec wmsecLgE;
			var.getWidthLogoFromBase(wmsecLgE, 2, flagWide);	// 前後は同エッジ隣接(2)
			RangeMsec lenPE;
			RangeMsec lenNE;
			lenPE.st = var.opt.getOpt(OptType::MsecLenPEMin);
			lenPE.ed = var.opt.getOpt(OptType::MsecLenPEMax);
			lenNE.st = var.opt.getOpt(OptType::MsecLenNEMin);
			lenNE.ed = var.opt.getOpt(OptType::MsecLenNEMax);
			exeflag = checkBaseLogoLength(wmsecLgE, lenPE, lenNE);
		}
	}
	//--- ロゴ位置から-SC系オプションを見る場合の確認（変更後基準ロゴ位置で確認） ---
	if ( !var.opt.tack.floatBase && !var.opt.tack.shiftBase && exeflag){
		LogoEdgeType edgeBase = var.getLogoBsrcEdge();
		exeflag = var.isScpEnableAtMsec(msecLogoBsrc, edgeBase, TargetCatType::From);
	}
	return exeflag;
}
//--- 前後ロゴ間の長さによる制約 ---
bool JlsScriptLimit::checkBaseLogoLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN){
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

//=====================================================================
// ターゲット範囲を取得
//=====================================================================

//---------------------------------------------------------------------
// 検索対象範囲を設定（基準ロゴ位置をベース）
// 出力：
//   返り値：制約満たす範囲確認（0:該当なし  1:対象範囲取得）
//   var.addRangeDst()
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
bool JlsScriptLimit::targetRangeByLogo(JlsCmdSet& cmdset){
	//--- ターゲット範囲設定用の基準位置取得 ---
	WideMsec wmsecBase = var.getLogoBtgWmsec();	// ターゲット用ロゴ位置（検索幅含）取得

	//--- 基準位置からターゲット範囲作成 ---
	if ( var.opt.tack.shiftBase ){		// シフト基準位置（-shiftオプション）
		addTargetRangeByLogoShift(wmsecBase);
	}else{				// 通常の１つの基準位置
		addTargetRangeData(wmsecBase);
	}
	//--- ターゲット情報設定 ---
	bool fromLogo = true;			// ロゴ位置ベースのターゲット
	updateTargetRange(cmdset, fromLogo);

	return ( var.sizeRangeDst() > 0 );
}
//---------------------------------------------------------------------
// 検索対象範囲の基準位置を直接数値設定
// 出力：
//   var.addRangeDst()
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
void JlsScriptLimit::targetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec){
	//--- 基準位置からターゲット範囲作成 ---
	addTargetRangeData(wmsec);
	//--- ターゲット情報設定 ---
	bool fromLogo = false;			// ロゴ位置基準ではなく直接指定コマンド
	updateTargetRange(cmdset, fromLogo);
}

// ターゲット検索範囲の基本情報を設定
void JlsScriptLimit::updateTargetRange(JlsCmdSet& cmdset, bool fromLogo){
	WideMsec wmsec = var.getRangeDstWide();
	cmdset.limit.setTargetRange(wmsec, fromLogo);
}
// シフト複数候補に対応したロゴ基準位置を設定
void JlsScriptLimit::addTargetRangeByLogoShift(WideMsec wmsecBase){
	//--- ソート用 ---
	struct data_t {
		WideMsec wmsec;
		Msec gap;
		bool operator<( const data_t& right ) const {
			return gap < right.gap;
		}
		bool operator==( const data_t& right ) const {
			return gap == right.gap;
		}
	};

	//--- 検索範囲設定 ---
	WideMsec wmsecScope = wmsecBase;
	wmsecScope.just  += var.opt.getOpt(OptType::MsecSftC);
	wmsecScope.early += var.opt.getOpt(OptType::MsecSftL);
	wmsecScope.late  += var.opt.getOpt(OptType::MsecSftR);
	//--- 複数基準位置の検索開始 ---
	vector<data_t> listTarget;
	//--- 無音シ－ンチェンジの候補を順番にチェック ---
	bool flagNoEdge = var.opt.getOptFlag(OptType::FlagNoEdge);
	RangeNsc rnsc = pdata->getRangeNscTotal(flagNoEdge);
	for(int j=rnsc.st; j<=rnsc.ed; j++){
		Msec msecNow   = pdata->getMsecScp(j);
		Msec msecNowBk = pdata->getMsecScpBk(j);
		if ( wmsecScope.early <= msecNow && msecNowBk <= wmsecScope.late ){	// 検索範囲内
			//--- -shiftによるロゴ位置から-SC系オプションを確認 ---
			bool valid = var.isScpEnableAtMsec(msecNow, LOGO_EDGE_RISE, TargetCatType::Shift);
			if ( valid ){		// 基準ロゴ位置として有効
				Msec msecRel = msecNow - wmsecBase.just;
				WideMsec wmsecTmp;
				wmsecTmp.just  = wmsecBase.just  + msecRel;
				wmsecTmp.early = wmsecBase.early + msecRel;
				wmsecTmp.late  = wmsecBase.late  + msecRel;
				data_t dat;
				dat.wmsec = wmsecTmp;
				dat.gap   = abs(msecRel);
				listTarget.push_back(dat);		// 仮リストに追加
			}
		}
	}
	if ( listTarget.size() > 0 ){
		std::sort(listTarget.begin(), listTarget.end());	// 近い順にソート
	}else{
		if ( var.opt.tack.forcePos ){		// 基準位置を強制設定する場合
			data_t dat;
			dat.wmsec = wmsecBase;
			dat.gap   = 0;
			listTarget.push_back(dat);		// 仮リストに追加
		}
	}
	//--- 条件を満たした基準位置を設定 ---
	for(int i=0; i<(int)listTarget.size(); i++){
		addTargetRangeData(listTarget[i].wmsec);
	}
}
//---------------------------------------------------------------------
// ターゲット検索範囲を設定（複数追加していくこと可能）
//---------------------------------------------------------------------
void JlsScriptLimit::addTargetRangeData(WideMsec wmsecBase){
	//--- -DList取得 ---
	vector<Msec> listMsec;
	if ( var.opt.isSetStrOpt(OptType::ListTgDst) ){		// -DList
		string strList = var.opt.getStrOpt(OptType::ListTgDst);
		pdata->cnv.getListValMsecM1(listMsec, strList);
	}
	//--- End系として使われる基準位置を正確に設定 ---
	WideMsec wmsecFrom = {wmsecBase.just, wmsecBase.just, wmsecBase.just};
	if ( var.opt.getOptFlag(OptType::FlagFixPos) == false ||
		 var.opt.tack.immFrom == false ){	// -fromabs等の直接フレーム番号指定ではない
		Nsc nscTmp = pdata->getNscFromMsecMgn(wmsecBase.just, pdata->msecValExact, SCP_END_NOEDGE);
		if ( nscTmp >= 0 ){
			wmsecFrom.early = pdata->getMsecScpBk(nscTmp);
			wmsecFrom.late  = pdata->getMsecScp(nscTmp);
		}
	}
	//--- ターゲット検索範囲を設定 ---
	int nsize = (int)listMsec.size();
	if ( nsize > 0 ){		// 複数候補オプション指定（-DList）
		for(int i=0; i<nsize; i++){
			WideMsec wmsecOpt;
			wmsecOpt.just  = listMsec[i] + wmsecBase.just;
			wmsecOpt.early = listMsec[i] + wmsecBase.early;
			wmsecOpt.late  = listMsec[i] + wmsecBase.late;
			WideMsec wmsecFind;
			if ( findTargetRange(wmsecFind, wmsecOpt, wmsecFrom.just) ){
				var.addRangeDst(wmsecFind, wmsecFrom);
			}
		}
	}else{		// 通常の単一設定
		WideMsec wmsecFind;
		if ( findTargetRange(wmsecFind, wmsecBase, wmsecFrom.just) ){
			var.addRangeDst(wmsecFind, wmsecFrom);
		}
	}
}
// ターゲット選択領域の計算に使用するロゴ情報を補正
bool JlsScriptLimit::findTargetRange(WideMsec& wmsecFind, WideMsec wmsecBase, Msec msecFrom){
	WideMsec wmsecAnd = {-1, -1, -1};		// 通常は未使用
	bool exeflag = findTargetRangeSetBase(wmsecFind, wmsecAnd, wmsecBase, msecFrom);
	if ( exeflag ){
		exeflag = findTargetRangeLimit(wmsecFind, wmsecAnd);
	}
	return exeflag;
}
// ターゲット位置変更オプションを反映
bool JlsScriptLimit::findTargetRangeSetBase(WideMsec& wmsecFind, WideMsec& wmsecAnd, WideMsec wmsecBase, Msec msecFrom){
	//--- 基本設定 ---
	wmsecFind = wmsecBase;
	bool flagAnd = var.opt.getOptFlag(OptType::FlagDstAnd);	// -DstAnd
	if ( flagAnd ){
		wmsecAnd = {msecFrom, msecFrom, msecFrom};	// And設定時用
	}
	//--- 直接位置リスト指定 ---
	if ( var.opt.isSetStrOpt(OptType::ListDstAbs) ){	// -DstAbs
		vector<Msec> listMsec;
		string strList = var.opt.getStrOpt(OptType::ListDstAbs);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			int nlist = var.getLogoBaseListNum();
			if ( nlist < 0 ) nlist = 0;
			if ( nlist >= nsize ) nlist = nsize-1;
			wmsecFind.just  = listMsec[nlist];
			wmsecFind.early = listMsec[nlist];
			wmsecFind.late  = listMsec[nlist];
			if ( flagAnd ){
				wmsecAnd = wmsecFind;
			}
		}
	}
	else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFD) ){	// -AbsSetFD
		vector<Msec> listMsec;
		string strList = var.opt.getStrOpt(OptType::ListAbsSetFD);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			if ( nsize < 2 ) return false;		// 必ず2データセット
			int nlist = var.getLogoBaseListNum();
			int ndst = nlist * 2 + 1;
			if ( ndst <= 0 ) ndst = 1;
			if ( ndst >= nsize ) ndst = nsize-1;
			wmsecFind.just  = listMsec[nlist];
			wmsecFind.early = listMsec[nlist];
			wmsecFind.late  = listMsec[nlist];
			if ( flagAnd ){
				wmsecAnd = wmsecFind;
			}
		}
	}
	if ( wmsecFind.just < 0 ){
		return false;
	}
	//--- 参照位置設定 ---
	WideMsec wmsecNext = ( flagAnd )? wmsecAnd : wmsecFind;		// And設定分岐
	{
		Msec msecSrc = wmsecNext.just;
		bool existNextL = var.opt.isSetOpt(OptType::NumDstNextL);	// -DstNextL
		bool existPrevL = var.opt.isSetOpt(OptType::NumDstPrevL);	// -DstPrevL
		if ( existNextL || existPrevL ){
			int numArg = 0;
			if ( existNextL ) numArg = var.opt.getOpt(OptType::NumDstNextL);
			if ( existPrevL ) numArg = var.opt.getOpt(OptType::NumDstPrevL) * -1;
			int locLogo;
			if ( numArg > 0 ){
				locLogo = pdata->getClogoNumNextCount(msecSrc, abs(numArg));
			}else if ( numArg < 0 ){
				locLogo = pdata->getClogoNumPrevCount(msecSrc, abs(numArg));
			}else{
				locLogo = pdata->getClogoMsecNear(msecSrc, LOGO_EDGE_BOTH);
			}
			wmsecNext = pdata->getClogoWmsecFromNum(locLogo);
		}
	}
	{
		Msec msecSrc = wmsecNext.just;
		bool existNextC = var.opt.isSetOpt(OptType::NumDstNextC);	// -DstNextC
		bool existPrevC = var.opt.isSetOpt(OptType::NumDstPrevC);	// -DstPrevC
		bool selNextCom = var.opt.tack.comFrom && !var.opt.tack.existDstOpt && !var.opt.tack.immFrom;
		if ( existNextC || existPrevC || selNextCom ){
			int numArg = 0;
			if ( existNextC ){
				numArg = var.opt.getOpt(OptType::NumDstNextC);
			}else if ( existPrevC ){
				numArg = var.opt.getOpt(OptType::NumDstPrevC) * -1;
			}else if ( var.opt.selectEdge == LOGO_EDGE_RISE ){
				numArg = 1;
			}else if ( var.opt.selectEdge == LOGO_EDGE_FALL ){
				numArg = -1;
			}
			Nsc nscT;
			if ( numArg > 0 ){
				nscT = pdata->getNscNextScpDispFromMsecCount(msecSrc, abs(numArg), true);
			}else if ( numArg < 0 ){
				nscT = pdata->getNscPrevScpDispFromMsecCount(msecSrc, abs(numArg), true);
			}else{
				nscT = pdata->getNscFromMsecDisp(msecSrc, pdata->getMsecTotalMax(), SCP_END_EDGEIN);
			}
			wmsecNext = pdata->getWideMsecScp(nscT);
		}
	}
	//--- And設定分岐 ---
	if ( flagAnd ){
		wmsecAnd = wmsecNext;
		wmsecAnd.early -= pdata->msecValSpc;
		wmsecAnd.late  += pdata->msecValSpc;
	}else{
		wmsecFind = wmsecNext;
	}
	return true;
}
// コマンド指定のターゲット範囲を元の範囲に追加
bool JlsScriptLimit::findTargetRangeLimit(WideMsec& wmsecFind, WideMsec& wmsecAnd){
	bool exeflag = true;
	//--- コマンド指定の範囲をフレーム範囲に追加 ---
	wmsecFind.just  += var.opt.wmsecDst.just;
	wmsecFind.early += var.opt.wmsecDst.early;
	wmsecFind.late  += var.opt.wmsecDst.late;
	//--- コマンド範囲に-1設定時の変換 ---
	if ( var.opt.wmsecDst.early == -1 ){
		wmsecFind.early = 0;
	}
	if ( var.opt.wmsecDst.late == -1 ){
		wmsecFind.late = pdata->getMsecTotalMax();
	}
	//--- ロゴ候補内に限定するSelectコマンド用の範囲 ---
	if ( exeflag && var.opt.cmdsel == CmdType::Select ){
		if ( var.getLogoBsrcMsec() == var.getLogoBorgMsec() ){
			WideMsec wmsecBtg = var.getLogoBtgWmsec();	// ロゴ位置と検索幅を取得
			RangeMsec rmsecExt;
			rmsecExt.st = wmsecBtg.early + var.opt.getOpt(OptType::MsecLogoExtL);
			rmsecExt.ed = wmsecBtg.late  + var.opt.getOpt(OptType::MsecLogoExtR);
			exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecExt);
		}
	}
	//--- 前後のロゴ位置以内に範囲限定する場合 ---
	if ( exeflag && var.opt.tack.limitByLogo ){
		bool flagWide = true;		// 可能性ある範囲で検索
		WideMsec wmsecPN;
		var.getWidthLogoFromBaseForTarget(wmsecPN, 1, flagWide);	// ターゲット用ロゴで前後は隣接(1)
		RangeMsec rmsecPN = { wmsecPN.early, wmsecPN.late };
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecPN);
	}
	//--- フレーム指定範囲内に限定 ---
	if ( exeflag ){
		RangeMsec rmsecWindow = var.getFrameRange();
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecWindow);
	}
	//--- -DstAndによる限定 ---
	if ( exeflag && wmsecAnd.just >= 0 ){
		RangeMsec rmsecFind = {wmsecFind.early, wmsecFind.late};
		wmsecFind = wmsecAnd;
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecFind);
	}
	//--- 範囲が存在しなければ無効化 ---
	if ( wmsecFind.early > wmsecFind.late ){
		exeflag = false;
	}
	return exeflag;
}

//=====================================================================
// ターゲット位置を取得
//=====================================================================

//---------------------------------------------------------------------
// ターゲットに一番近いシーンチェンジ位置を取得
// 出力：
//   cmdset.list.setTargetOutEdge() : 出力時のロゴエッジ
//   cmdset.list.setResult*()       : 選択シーンチェンジ位置
//---------------------------------------------------------------------
void JlsScriptLimit::targetPoint(JlsCmdSet& cmdset){
	//--- 条件に合う位置を検索 ---
	seekTargetPoint(cmdset);

	//--- 位置出力時用のロゴエッジによる補正 ---
	setTargetPointOutEdge(cmdset);
}

//---------------------------------------------------------------------
// 位置情報出力時のロゴエッジによる補正
//---------------------------------------------------------------------
void JlsScriptLimit::setTargetPointOutEdge(JlsCmdSet& cmdset){
	//--- 標準エッジ設定 ---
	LogoEdgeType edgeBase = cmdset.limit.getLogoBaseEdge();
	LogoEdgeType edgeDst = edgeBase;
	LogoEdgeType edgeEnd = edgeBase;
	//--- データ取得 ---
	TargetLocInfo tgDst = cmdset.limit.getResultDstCurrent();
	TargetLocInfo tgEnd = cmdset.limit.getResultEndCurrent();
	//--- 位置による設定 ---
	if ( cmdset.arg.cmdsel == CmdType::MkLogo  ||	// MkLogoコマンドは従来通り開始－終了位置
		 cmdset.arg.getOptFlag(OptType::FlagHoldBoth) ||
		 cmdset.arg.getOptFlag(OptType::FlagEdgeB) ){
		if ( tgDst.msec < tgEnd.msec ){
			edgeDst = LOGO_EDGE_RISE;
			edgeEnd = LOGO_EDGE_FALL;
		}else{
			edgeDst = LOGO_EDGE_FALL;
			edgeEnd = LOGO_EDGE_RISE;
		}
	}
	//--- 固定エッジ設定 ---
	if ( cmdset.arg.getOptFlag(OptType::FlagEdgeS) ){
		edgeDst = LOGO_EDGE_RISE;
		edgeEnd = LOGO_EDGE_RISE;
	}
	if ( cmdset.arg.getOptFlag(OptType::FlagEdgeE) ){
		edgeDst = LOGO_EDGE_FALL;
		edgeEnd = LOGO_EDGE_FALL;
	}
	//--- 書き戻し ---
	tgDst.edge = edgeDst;
	tgEnd.edge = edgeEnd;
	cmdset.limit.setResultDst(tgDst);
	cmdset.limit.setResultEnd(tgEnd);
}
//---------------------------------------------------------------------
// 条件に合う位置を検索
//---------------------------------------------------------------------
void JlsScriptLimit::seekTargetPoint(JlsCmdSet& cmdset){
	//--- 検索開始前の準備 ---
	prepTargetPoint(cmdset);
	//--- 検索設定 ---
	var.initSeekVar(cmdset);
	//--- 無音シーンチェンジ位置を順番に検索 ---
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	if ( !fixpos ){
		RangeNsc rnsc = var.seek.rnscScp;
		for(int j=rnsc.st; j<=rnsc.ed; j++){
			bool lastNsc = ( j == rnsc.ed );
			seekTargetPointFromScp(cmdset, j, lastNsc);
		}
	}
	//--- 検出できなかった場合の処理 ---
	if ( var.seek.tgDst.tp == TargetScpType::None ){
		bool flagForce = var.opt.tack.forcePos || fixpos;
		var.selRangeDstNum(0);	// 先頭候補選択
		var.seek.tgDst.nsc  = -1;
		var.seek.tgDst.msec = var.getRangeDstJust();
		var.seek.tgDst.msbk = var.seek.tgDst.msec;
		bool zone = var.isRangeToDst(var.getRangeDstFrom(), var.seek.tgDst.msec);
		bool valid = zone;
		if ( valid ){
			valid = var.isScpEnableAtMsec(var.seek.tgDst.msec, LOGO_EDGE_RISE, TargetCatType::Dst);
		}
		if ( !valid ){
			var.seek.tgDst.tp = TargetScpType::Invalid;
		}else if ( flagForce ){
			if ( pdata->isRangeInTotalMax(var.seek.tgDst.msec) ){
				var.seek.tgDst.tp = TargetScpType::Force;
			}else{
				var.seek.tgDst.tp = TargetScpType::Invalid;
			}
		}else if ( fixpos ){
			var.seek.tgDst.tp = TargetScpType::Direct;
		}else{
			var.seek.tgDst.tp = TargetScpType::Invalid;
		}
		if ( var.seek.tgDst.tp != TargetScpType::Invalid ){
			seekTargetPointEnd(cmdset, var.seek.tgDst.msec, flagForce);	// End地点も検索
		}else{
			//var.seek.tgDst.tp   = TargetScpType::Invalid;
			var.seek.tgEnd.tp   = TargetScpType::Invalid;
		}
	}
	//--- 無音シーンチェンジチェックない場合の推測構成エッジ補正 ---
	if ( fixpos ){
		if ( pdata->isClogoReal() == false ){
			WideMsec wmsec = var.getLogoBtgWmsec();
			Msec msecRevBk = wmsec.just - wmsec.early;
			var.seek.tgDst.msbk = var.seek.tgDst.msec - msecRevBk;	// 終了位置用の補正
		}
	}
	//--- フレーム単位の結果 ---
	var.seek.tgDst.msec = pdata->cnv.getMsecAlignFromMsec(var.seek.tgDst.msec);
	var.seek.tgDst.msbk = pdata->cnv.getMsecAlignFromMsec(var.seek.tgDst.msbk);
	var.seek.tgEnd.msec = pdata->cnv.getMsecAlignFromMsec(var.seek.tgEnd.msec);
	var.seek.tgEnd.msbk = pdata->cnv.getMsecAlignFromMsec(var.seek.tgEnd.msbk);
	//--- 候補位置確定処理 ---
	cmdset.limit.setResultDst(var.seek.tgDst);
	cmdset.limit.setResultEnd(var.seek.tgEnd);
}
//--- 指定無音シーンチェンジ位置からの対象検索 ---
void JlsScriptLimit::seekTargetPointFromScp(JlsCmdSet& cmdset, Nsc nscNow, bool lastNsc){
	Msec         msecNow = pdata->getMsecScp(nscNow);
	ScpPriorType statNow = pdata->getPriorScp(nscNow);
	bool exist     = ( var.seek.tgDst.tp != TargetScpType::None );
	//--- 条件を満たさなければ次の位置へ ---
	if ( lastNsc && var.seek.flagNextTail ){	// NextTailで最終位置はelseの条件関係なし
	}else{
		if ( var.isScpEnableAtNsc(TargetCatType::Dst, nscNow) == false ){
			return;
		}
	}
	//--- 構成の優先順位による判定 ---
	bool condPrior = false;
	if ( exist && !var.opt.getOpt(OptType::FlagFlat) ){
		if ( cmdset.arg.cmdsel == CmdType::Select ){
		}
		else if ( statNow > var.seek.statDst ){
			condPrior = true;		// 他の候補よりも優先
		}
		else if ( statNow < var.seek.statDst && !lastNsc ){	// 最終位置は確定扱いで中断しない
			return;		// 優先順位が低い候補はここでフィルタ
		}
	}
	//--- ロゴからの情報使用時(NextTailコマンド用） ---
	bool flagLogoNow = false;
	if ( var.seek.selectLogoRise ){
		int nsize = pdata->sizeClogoList();
		for(int k=0; k<nsize; k+=2){		// 立上りエッジのみ
			Msec msecLogo = pdata->getClogoMsecFromNum(k);
			if ( abs(msecNow - msecLogo) <= pdata->msecValSpc ){
				if ( pdata->isAutoModeUse() == false ||
					 pdata->isScpChapTypeDecideFromNsc(nscNow) ){	// 確定区切り時のみ優先
					flagLogoNow = true;			// ロゴ立上り位置
				}
				else if ( pdata->isClogoReal() &&
					(pdata->getNscFromMsecMgn(msecNow, pdata->msecValSpc, ScpEndType::SCP_END_EDGEIN) < 0 ||
					 var.opt.getOptFlag(OptType::FlagFlat)) ){
					flagLogoNow = true;			// 実際ロゴ使用で近くに確定区切りがない時もロゴ立上り位置
				}
			}
		}
		if ( !flagLogoNow && var.seek.flagOnLogo && !lastNsc ){
			return;		// 優先順位が低い候補はここでフィルタ
		}
		if ( (flagLogoNow || lastNsc) && !var.seek.flagOnLogo ){
			condPrior = true;		// ロゴ優先時は他の候補よりも優先
		}
	}
	//--- 各Dstオプション条件と比較して最適位置を取得 ---
	bool det = false;
	int nsize = var.sizeRangeDst();
	for(int k=0; k<nsize; k++){
		var.selRangeDstNum(k);	// 候補選択
		if ( var.isRangeToDst(var.getRangeDstFrom(), msecNow) ){
			Msec msecGap = abs(msecNow - var.getRangeDstJust());
			if ( k < var.seek.numListDst || !exist ){	// 複数ターゲット候補は先優先
				det = true;
			}else if ( k > var.seek.numListDst ){	// 後のターゲット候補処理
				det = condPrior;
			}else if ( msecGap < var.seek.gapDst ){	// 他の条件が同じであれば中心に近い所優先
				det = true;
			}
			if ( det ){
				if ( seekTargetPointEnd(cmdset, msecNow, false) ){	// End位置判定
					var.seek.tgDst.tp   = TargetScpType::ScpNum;
					var.seek.tgDst.nsc  = nscNow;
					var.seek.tgDst.msec = msecNow;
					var.seek.tgDst.msbk = pdata->getMsecScpBk(nscNow);
					var.seek.numListDst = k;
					var.seek.statDst    = statNow;
					var.seek.gapDst     = msecGap;
					var.seek.flagOnLogo = flagLogoNow;
				}
			}
		}
	}
}
//--- -End系オプションに該当する位置を検索 ---
bool JlsScriptLimit::seekTargetPointEnd(JlsCmdSet& cmdset, Msec msecDst, bool force){
	//--- オプション設定状態 ---
	bool needRange = var.isPrepEndRangeExist();	// 範囲設定
	bool needRefer = var.isPrepEndReferExist();	// 前後END位置設定
	bool needMulti = var.isPrepEndMultiBase();	// 複数from位置
	bool fixEnd = ( !needRange && !needRefer && !needMulti );
	//--- -End固定位置確定時の設定 ---
	if ( fixEnd && !force ){
		if ( var.seek.tgEnd.tp != TargetScpType::None ){	// 設定済みなら毎回同じなので省略
			return ( var.seek.tgEnd.tp != TargetScpType::Invalid );
		}
	}
	//--- 固定位置部分の設定 ---
	TargetLocInfo tgTry = var.getPrepEndAbs();
	if ( var.isPrepEndFromAbs() ){
		if ( needMulti ){	// END複数時の基準位置再選択
			var.getRangeDstFromForScp(tgTry.msec, tgTry.msbk, tgTry.nsc);
		}
	}else{	// End固定位置指定ではない時は結果位置を参照位置とする
		tgTry.tp = TargetScpType::None;
		tgTry.nsc = -1;
		tgTry.msec = msecDst;
		tgTry.msbk = msecDst;
	}
	Msec msecBefore = tgTry.msec;	// -EndAnd使用時のために保管
	//--- 相対END位置の参照設定 ---
	if ( needRefer ){
		seekTargetPointEndRefer(tgTry, cmdset, tgTry.msec);
	}
	//--- 無音シーンチェンジ位置を範囲検索しない場合の確定処理 ---
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	if ( !needRange || tgTry.tp == TargetScpType::Invalid ){
		bool zone = var.isRangeToEndZone(msecDst, tgTry.msec);	// Zone確認
		bool valid = zone;
		if ( valid ){		// 無音シーンチェンジのEnd位置を検索する
			valid = var.isScpEnableAtMsec(tgTry.msec, LOGO_EDGE_RISE, TargetCatType::End);
		}
		if ( !valid ){
			tgTry.tp = TargetScpType::Invalid;
		}else if ( force ){
			if ( pdata->isRangeInTotalMax(tgTry.msec) ){
				if ( tgTry.tp != TargetScpType::ScpNum ){
					tgTry.tp = TargetScpType::Force;
				}
			}else{
				tgTry.tp = TargetScpType::Invalid;
			}
		}else if ( fixpos ){
			tgTry.tp = TargetScpType::Direct;
		}
		valid = ( tgTry.tp != TargetScpType::Invalid );
		if ( valid || var.seek.tgEnd.tp == TargetScpType::None ){
			var.seek.tgEnd = tgTry;		// End位置更新
		}
		return valid;
	}
	//--- 範囲から無音シーンチェンジ位置を選択 ---
	bool exist = false;
	if ( !fixpos ){
		Msec msecIn = tgTry.msec;	// End検索の起点
		Nsc  nscAnd = -1;			// -EndNext* + -EndAnd による限定
		if ( var.opt.getOptFlag(OptType::FlagEndAnd) ){		// -EndAnd
			msecIn = msecBefore;
			nscAnd = tgTry.nsc;
		}
		exist = seekTargetPointEndScp(cmdset, msecIn, msecDst, nscAnd);
	}
	//--- -End系位置に該当ない場合の処理 ---
	if ( !exist ){
		if ( force || var.seek.tgEnd.tp == TargetScpType::None ){
			Msec msecLen = var.getPrepEndRangeForceLen();
			var.seek.tgEnd.tp   = TargetScpType::Invalid;
			var.seek.tgEnd.nsc  = -1;
			var.seek.tgEnd.msec = tgTry.msec + msecLen;
			var.seek.tgEnd.msbk = tgTry.msbk + msecLen;
			var.seek.numListEnd = 0;
			var.seek.statEnd    = ScpPriorType::SCP_PRIOR_NONE;
			var.seek.gapEnd     = 0;
			bool zone = var.isRangeToEndZone(msecDst, var.seek.tgEnd.msec);	// Zone確認
			bool valid = zone;
			if ( valid ){		// 無音シーンチェンジのEnd位置を検索する
				valid = var.isScpEnableAtMsec(var.seek.tgEnd.msec, LOGO_EDGE_RISE, TargetCatType::End);
			}
			if ( valid ){
				if ( force && pdata->isRangeInTotalMax(var.seek.tgEnd.msec) ){
					var.seek.tgEnd.tp = TargetScpType::Force;
				}else if ( fixpos ){
					var.seek.tgEnd.tp = TargetScpType::Direct;
				}
			}
		}
	}
	return ( var.seek.tgEnd.tp != TargetScpType::Invalid );
}
//--- 相対END位置の参照設定 ---
void JlsScriptLimit::seekTargetPointEndRefer(TargetLocInfo& tgEnd, JlsCmdSet& cmdset, Msec msecIn){
	Nsc  nscEnd = -1;
	Msec msecEnd = msecIn;
	Msec msbkEnd = msecIn;
	bool exist = false;
	{
		bool existNextL = cmdset.arg.isSetOpt(OptType::NumEndNextL);	// -EndNextL
		bool existPrevL = cmdset.arg.isSetOpt(OptType::NumEndPrevL);	// -EndPrevL
		if ( existNextL || existPrevL ){
			exist = true;
			int numArg = 0;
			if ( existNextL ) numArg = cmdset.arg.getOpt(OptType::NumEndNextL);
			if ( existPrevL ) numArg = cmdset.arg.getOpt(OptType::NumEndPrevL) * -1;
			int locLogo;
			if ( numArg > 0 ){
				locLogo = pdata->getClogoNumNextCount(msecEnd, abs(numArg));
			}else if ( numArg < 0 ){
				locLogo = pdata->getClogoNumPrevCount(msecEnd, abs(numArg));
			}else{
				locLogo = pdata->getClogoMsecNear(msecEnd, LOGO_EDGE_BOTH);
			}
			WideMsec wmsecLogo = pdata->getClogoWmsecFromNum(locLogo);
			msecEnd = wmsecLogo.late;
			msbkEnd = wmsecLogo.early;
			nscEnd = pdata->getClogoNsc(msecEnd);
		}
	}
	{
		bool existNextC = cmdset.arg.isSetOpt(OptType::NumEndNextC); 	// -EndNextC
		bool existPrevC = cmdset.arg.isSetOpt(OptType::NumEndPrevC); 	// -EndPrevC
		if ( existNextC || existPrevC ){
			exist = true;
			int numArg = 0;
			if ( existNextC ) numArg = cmdset.arg.getOpt(OptType::NumEndNextC);
			if ( existPrevC ) numArg = cmdset.arg.getOpt(OptType::NumEndPrevC) * -1;
			if ( numArg > 0 ){
				nscEnd = pdata->getNscNextScpDispFromMsecCount(msecEnd, abs(numArg), true);
			}else if ( numArg < 0 ){
				nscEnd = pdata->getNscPrevScpDispFromMsecCount(msecEnd, abs(numArg), true);
			}else{
				nscEnd = pdata->getNscFromMsecDisp(msecEnd, pdata->getMsecTotalMax(), SCP_END_EDGEIN);
			}
		}
	}
	if ( !exist ) return;
	//--- 更新 ---
	if ( nscEnd >= 0 ){
		tgEnd.tp   = TargetScpType::ScpNum;
		tgEnd.nsc  = nscEnd;
		tgEnd.msec = pdata->getMsecScp(nscEnd);
		tgEnd.msbk = pdata->getMsecScpBk(nscEnd);
	}else{
		tgEnd.tp   = TargetScpType::Direct;
		tgEnd.nsc  = -1;
		tgEnd.msec = msecEnd;
		tgEnd.msbk = msbkEnd;
		if ( msecEnd < 0 ){
			tgEnd.tp   = TargetScpType::Invalid;
		}
	}
}
//--- END位置を範囲から無音シーンチェンジ位置を選択 ---
bool JlsScriptLimit::seekTargetPointEndScp(JlsCmdSet& cmdset, Msec msecIn, Msec msecDst, Nsc nscAnd){
	//--- 各無音シーンチェンジ位置で条件に合う一番近い所を取得 ---
	bool exist = false;
	int nsize = var.sizePrepEndRange();
	int nCheckMax = nsize - 1;		// 最初は全オプション検索
	RangeNsc rnsc = var.seek.rnscScp;
	for(int j=rnsc.st; j<=rnsc.ed; j++){
		//--- 条件を満たさなければ次の位置へ ---
		if ( var.isScpEnableAtNsc(TargetCatType::End, j) == false ){
			continue;
		}
		if ( nscAnd >= 0 && nscAnd != j ){		// -EndAndの時は対象位置(nscAnd)に限定
			continue;
		}
		ScpPriorType statNow = pdata->getPriorScp(j);
		bool condPrior = false;
		if ( exist && !var.opt.getOptFlag(OptType::FlagFlat) ){
			if ( statNow < var.seek.statEnd ){	// 構成の優先順位が候補より低い位置はチェック省略
				continue;
			}else if ( statNow > var.seek.statEnd ){	// 優先順位が高い時
				condPrior = true;
			}
		}
		//--- 各Endオプション条件と比較して最適位置を取得 ---
		Msec msecNow = pdata->getMsecScp(j);
		bool det = false;
		for(int k=0; k<=nCheckMax; k++){
			WideMsec wmsecEnd = var.getPrepEndRangeWithOffset(k, msecIn);
			if ( var.isRangeToEnd(msecDst, msecNow, wmsecEnd) ){
				Msec msecGap = abs(msecNow - wmsecEnd.just);
				if ( k < nCheckMax || !exist || condPrior ){
					det = true;
				}else if ( msecGap < var.seek.gapEnd ){
					det = true;
				}
				if ( det ){
					exist   = true;
					nCheckMax = k;		// 検出済オプションより後の候補は以降無視する
					var.seek.tgEnd.tp   = TargetScpType::ScpNum;
					var.seek.tgEnd.nsc  = j;
					var.seek.tgEnd.msec = msecNow;
					var.seek.tgEnd.msbk = pdata->getMsecScpBk(j);
					var.seek.numListEnd = k;
					var.seek.statEnd    = statNow;
					var.seek.gapEnd     = msecGap;
					break;
				}
			}
		}
	}
	return exist;
}
//---------------------------------------------------------------------
// 検索開始前の準備
//---------------------------------------------------------------------
void JlsScriptLimit::prepTargetPoint(JlsCmdSet& cmdset){
	//--- End位置準備 ---
	prepTargetPointEnd(cmdset);
}
//--- END位置設定 ---
void JlsScriptLimit::prepTargetPointEnd(JlsCmdSet& cmdset){
	//--- 初期化 ---
	var.clearPrepEnd();
	//--- マージン設定 ---
	Msec msecMgnSpc   = pdata->msecValSpc;		// 検索用デフォルトマージン
	if ( cmdset.arg.isSetOpt(OptType::MsecEmargin) ){	// マージン設定あれば変更
		msecMgnSpc   = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
	}
	//--- EndSft指定 ---
	bool existEndSft = false;
	WideMsec wmsecEndSft;
	if ( cmdset.arg.isSetOpt(OptType::MsecEndSftC) ){			// -EndSft
		existEndSft = true;
		wmsecEndSft.just  = cmdset.arg.getOpt(OptType::MsecEndSftC);
		wmsecEndSft.early = cmdset.arg.getOpt(OptType::MsecEndSftL);
		wmsecEndSft.late  = cmdset.arg.getOpt(OptType::MsecEndSftR);
	}
	//--- End範囲指定 ---
	bool existEndLen = false;
	if ( cmdset.arg.isSetOpt(OptType::MsecEndlenC) &&			// -EndLen設定あり
	    ( cmdset.arg.getOpt(OptType::MsecEndlenC) != 0 ||		// -EndLen 0以外
	      cmdset.arg.getOptFlag(OptType::FlagReset) == false )){		// -resetなし
		existEndLen = true;
		WideMsec wmsec;
		wmsec.just  = cmdset.arg.getOpt(OptType::MsecEndlenC);
		wmsec.early = cmdset.arg.getOpt(OptType::MsecEndlenL);
		wmsec.late  = cmdset.arg.getOpt(OptType::MsecEndlenR);
		if ( existEndSft ){
			wmsec.just  += wmsecEndSft.just;
			wmsec.early += wmsecEndSft.early;
			wmsec.late  += wmsecEndSft.late;
		}
		var.addPrepEndRange(wmsec);	// End設定リスト追加
	}
	//--- End範囲リスト指定 ---
	if ( cmdset.arg.isSetStrOpt(OptType::ListTgEnd) ){	// -EndList
		existEndLen = true;
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListTgEnd);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			for(int i=0; i<(int)listMsec.size(); i++){
				WideMsec wmsec;
				if ( existEndSft ){
					wmsec.just  = listMsec[i] + wmsecEndSft.just;
					wmsec.early = listMsec[i] + wmsecEndSft.early;
					wmsec.late  = listMsec[i] + wmsecEndSft.late;
				}else{
					wmsec.just  = listMsec[i];
					wmsec.early = listMsec[i] - msecMgnSpc;
					wmsec.late  = listMsec[i] + msecMgnSpc;
				}
				var.addPrepEndRange(wmsec);	// End設定リスト追加
			}
		}
	}
	//--- EndSft指定（保存） ---
	if ( existEndSft && !var.isPrepEndRangeExist() ){
		var.addPrepEndRange(wmsecEndSft);	// EndSft設定リスト追加
	}
	//--- End前後位置指定 ---
	bool existEndRefer = false;
	if ( cmdset.arg.isSetOpt(OptType::NumEndNextL) ||	// -EndNextL
		 cmdset.arg.isSetOpt(OptType::NumEndNextC) ){	// -EndNextC
		existEndRefer = true;
	}
	var.setPrepEndRefer(existEndRefer);
	//--- 固定のEND位置取得 ---
	TargetLocInfo tgEnd;
	bool multiBase;
	bool fromAbs = prepTargetPointEndAbs(tgEnd, multiBase, cmdset);
	if ( !existEndLen && !existEndRefer ){
		fromAbs = true;	// 相対-End系オプションなければ基準位置をEndにする
	}
	var.setPrepEndAbs(fromAbs, multiBase, tgEnd);
}
//--- 固定のEND位置取得 ---
bool JlsScriptLimit::prepTargetPointEndAbs(TargetLocInfo& tgEnd, bool& multiBase, JlsCmdSet& cmdset){
	//--- マージン設定 ---
	Msec msecMgnSpc   = pdata->msecValSpc;		// 検索用デフォルトマージン
	Msec msecMgnExact = pdata->msecValExact;	// 直接フレーム位置指定時は最小限の範囲
	if ( cmdset.arg.isSetOpt(OptType::MsecEmargin) ){	// マージン設定あれば変更
		msecMgnSpc   = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
		msecMgnExact = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
	}
	//--- 固定END位置 ---
	bool useBase = false;
	tgEnd.nsc  = -1;
	tgEnd.msec = -1;
	tgEnd.msbk = -1;
	bool fromAbs = false;
	Msec msecMgn = msecMgnSpc;
	if ( cmdset.arg.isSetStrOpt(OptType::ListEndAbs) ){	// -EndAbs
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListEndAbs);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			int nlist = var.getLogoBaseListNum();
			if ( nlist < 0 ) nlist = 0;
			if ( nlist >= nsize ) nlist = nsize-1;
			tgEnd.msec = listMsec[nlist];
		}
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetStrOpt(OptType::ListAbsSetFE) ){	// -AbsSetFE
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListAbsSetFE);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			if ( nsize < 2 ) return false;		// 必ず2データセット
			int nlist = var.getLogoBaseListNum();
			int ned = nlist * 2 + 1;
			if ( ned <= 0 ) ned = 1;
			if ( ned >= nsize ) ned = nsize-1;
			tgEnd.msec = listMsec[ned];
		}
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndHead) ){	// -EndHead
		tgEnd.msec = cmdset.limit.getHead();
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndTail) ){	// -EndTail
		tgEnd.msec = cmdset.limit.getTail();
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndHold) ){	// -EndHold
		string strVal = cmdset.arg.getStrOpt(OptType::StrValPosR);	// $POSHOLD
		pdata->cnv.getStrValMsecM1(tgEnd.msec, strVal, 0);
		tgEnd.msbk = tgEnd.msec;
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.getOptFlag(OptType::FlagEndBaseL) ){	// -EndBaseL
		fromAbs = true;
		Nrf nrfBase = cmdset.limit.getLogoBaseNrf();
		Nrf nscBase = cmdset.limit.getLogoBaseNsc();
		if ( nrfBase >= 0 ){
			tgEnd.msec = pdata->getMsecLogoNrf(nrfBase);
			tgEnd.msbk = tgEnd.msec;
			tgEnd.nsc  = pdata->getClogoNsc(tgEnd.msec);
		}else if ( nscBase >= 0 ){
			tgEnd.nsc  = cmdset.limit.getLogoBaseNsc();
			tgEnd.msec = pdata->getMsecScp(tgEnd.nsc);
			tgEnd.msbk = pdata->getMsecScpBk(tgEnd.nsc);
		}else{
			msecMgn    = msecMgnExact;
			useBase    = true;
			multiBase  = var.isRangeDstMultiFrom();
		}
	}
	else{
		if ( cmdset.arg.getOptFlag(OptType::FlagEndBase) ){	// -EndBase
			fromAbs = true;
		}
		msecMgn    = msecMgnExact;
		useBase    = true;
		multiBase  = var.isRangeDstMultiFrom();
	}
	//--- 基準位置(from)指定時 ---
	if ( useBase ){
		var.getRangeDstFromForScp(tgEnd.msec, tgEnd.msbk, tgEnd.nsc);
	}
	//--- 補正 ---
	if ( tgEnd.msbk < 0 ){
		tgEnd.msbk = tgEnd.msec;
	}
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	tgEnd.exact = fixpos;
	if ( tgEnd.nsc >= 0 ){
		if ( fixpos ){
			tgEnd.tp = TargetScpType::Force;
		}else{
			tgEnd.tp = TargetScpType::ScpNum;
		}
	}else if ( !pdata->isRangeInTotalMax(tgEnd.msec) ){
		if ( fixpos ){
			tgEnd.tp = TargetScpType::Direct;
		}else{
			tgEnd.tp = TargetScpType::Invalid;
		}
	}else if ( fixpos ){
		tgEnd.tp = TargetScpType::Force;
	}else{
		tgEnd.nsc = pdata->getNscFromMsecMgn(tgEnd.msec, msecMgn, SCP_END_EDGEIN);
		if ( tgEnd.nsc >= 0 ){
			tgEnd.tp = TargetScpType::ScpNum;
		}else if ( var.opt.tack.forcePos ){
			tgEnd.tp = TargetScpType::Force;
		}else{
			tgEnd.tp = TargetScpType::Direct;
		}
	}
	if ( tgEnd.nsc >= 0 ){
		tgEnd.msec = pdata->getMsecScp(tgEnd.nsc);
		tgEnd.msbk = pdata->getMsecScpBk(tgEnd.nsc);
	}
	return fromAbs;
}

