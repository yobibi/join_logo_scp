//
// join_logo_scp : Auto系CM構成推測処理
//  出力：
//    pdata(chap,arstat)
//  作業領域：
//    pdata(score)
//  データ挿入によるシーンチェンジ番号(nsc)変更あり
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsAutoReform.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"


//=====================================================================
// 推測構成に必要な事前設定
//=====================================================================

//---------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------
JlsAutoReform::JlsAutoReform(JlsDataset *pdata){
	//--- 関数ポインタ作成 ---
	this->pdata = pdata;
}

//=====================================================================
// 推測構成を作成
//=====================================================================

//---------------------------------------------------------------------
// 構成推測を実行（auto系コマンドの一番最初に必ず実行）
// 入力：
//    cmdarg：引数データ（AutoCMだった場合のみロゴなし推測で使用）
//---------------------------------------------------------------------
void JlsAutoReform::mkReformAll(JlsCmdArg &cmdarg){
	//--- ロゴ優先度から構成を推測 ---
	if (pdata->isUnuseLevelLogo()){			// ロゴなし構成推測する場合
		mkReformAllNologo(cmdarg);
	}
	else{
		mkReformAllLogo();
	}
}

//---------------------------------------------------------------------
// CM構成検出コマンド開始（ロゴなし時のみ実行される）
// 入力：
//    cmdarg：引数データ（ロゴなし推測で使用）
//---------------------------------------------------------------------
bool JlsAutoReform::startAutoCM(JlsCmdArg &cmdarg){
	bool exeflag = false;
	if (pdata->isUnuseLevelLogo()){			// ロゴなし構成推測する場合
		FormAutoCMArg param_autocm;
		bool reform_all = false;
		setReformParam(param_autocm, cmdarg, reform_all);
		exeflag = detectCM(param_autocm);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// 対象位置（１点または２点範囲）の構成を確定
// 入力：
//    nsc_from：範囲開始位置
//    nsc_to：  範囲終了位置
//    logoon：  ロゴ設定（0:ロゴなし、1:ロゴあり）
//    resutuct：構成再構築（0:範囲内は単一構成、1:範囲内の構成を再構築）
// 出力：
//   pdata(chap,arstat)
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
void JlsAutoReform::mkReformTarget(Nsc nsc_from, Nsc nsc_to, bool logoon, int restruct){
	//--- fromを前側、toを後側 ---
	if (nsc_from > nsc_to){
		Nsc tmp = nsc_from;
		nsc_from = nsc_to;
		nsc_to   = tmp;
	}
	//--- 両方マイナスの時は設定なし ---
	if (nsc_to < 0){
		return;
	}
	//--- 処理開始 ---
	Msec msec_to = -1;
	Msec msec_from = -1;
	Sec  sec_dif = 0;
	//--- 前側位置の確定処理 ---
	if (nsc_from >= 0){
		msec_from = pdata->getMsecScp(nsc_from);
		//--- 手前位置に設定する構成を取得 ---
		Nsc nsc_chapfrom = pdata->getNscNextScpDecide(nsc_from-1, SCP_END_EDGEIN);
		ScpArType arstat_from = pdata->getScpArstat(nsc_chapfrom);
		ScpArExtType arext_from = pdata->getScpArext(nsc_chapfrom);
		//--- 手前位置の設定 ---
		pdata->setScpChap(nsc_from, SCP_CHAP_DFORCE);
		pdata->setScpArstat(nsc_from, arstat_from);
		pdata->setScpArext(nsc_from, arext_from);
		//--- 対象間の確定位置を解除 ---
		while (nsc_chapfrom >= 0 && nsc_chapfrom < nsc_to){
			if (nsc_chapfrom > nsc_from){
				pdata->setScpChap(nsc_chapfrom, SCP_CHAP_NONE);
			}
			nsc_chapfrom = pdata->getNscNextScpDecide(nsc_chapfrom, SCP_END_EDGEIN);
		}
	}
	//--- 後側位置の確定処理 ---
	if (nsc_to >= 0){
		msec_to = pdata->getMsecScp(nsc_to);
		//--- 対象間の距離取得 ---
		ScpArType arstat_to = SCP_AR_L_OTHER;
		if (nsc_from >= 0){
			sec_dif = pdata->cnv.getSecFromMsec(msec_to - msec_from);
		}
		//--- 後側位置に設定する構成を取得 ---
		if (logoon == 0){
			if (sec_dif > 0 && sec_dif <= 120 && (sec_dif % 15 == 0)){
				arstat_to = SCP_AR_N_UNIT;
			}
			else{
				arstat_to = SCP_AR_N_OTHER;
			}
		}
		//--- 後側位置の設定 ---
		pdata->setScpChap(nsc_to, SCP_CHAP_DFORCE);
		pdata->setScpArstat(nsc_to, arstat_to);
	}
	//--- ２点間の構成再構築 ---
	if (nsc_from >= 0 && nsc_to >= 0 && restruct > 0){
		//--- ロゴなし再構築 ---
		if (logoon == 0){
			RangeFixMsec scope;
			scope.st = msec_from;
			scope.ed = msec_to;
			scope.fixSt = true;
			scope.fixEd = true;
			if (nsc_from == 0){
				scope.fixSt = false;
				scope.st     = -1;
			}
			if (nsc_to == pdata->sizeDataScp()-1){
				scope.fixEd = false;
				scope.ed     = -1;
			}
			int  interfill = 2;			// CM構成内部は強制的にCM化
			RangeMsec bounds;
			mkRangeCM(bounds, scope, logoon, interfill);
		}
		//--- ロゴあり再構築 ---
		else{
			RangeNsc rnsc_target = {nsc_from, nsc_to};
			setInterpolar(rnsc_target, logoon);
		}
	}
}




//=====================================================================
// 全体の構成推測実行
//=====================================================================

//---------------------------------------------------------------------
// ロゴ情報を使って構成推測を実行
//---------------------------------------------------------------------
void JlsAutoReform::mkReformAllLogo(){
	bool lastlogo  = true;			// 最終位置のロゴ有フラグ
	//--- ロゴのない期間をCM化する処理 ---
	{
		NrfCurrent logopt = {};			// 現在のロゴ位置格納
		RangeFixMsec scope = {};		// 検索範囲
		scope.st = -1;
		Msec msec_lg_remain_st = -1;	// 次の残す箇所の先頭位置
		bool logoon_st = false;			// 先頭部分のロゴ状態
		bool flagend   = false;			// 終了フラグ
		while(flagend == false){
			//--- ロゴ位置取得 ---
			if ( pdata->getNrfptNext(logopt, LOGO_SELECT_VALID) ){
				scope.ed = logopt.msecRise;
			}
			else{
				scope.ed = -1;
				flagend = true;			// 最後の端処理をして終了
			}
			//--- CM構成を作成 ---
			int  interfill = 1;			// 複数のCM構成では間もCM
			RangeMsec bounds;
			bool det = mkRangeCM(bounds, scope, logoon_st, interfill);
			//--- 最終位置のロゴ有無判断 ---
			if (flagend == true && det){
				if (logopt.msecFall < bounds.ed + pdata->msecValLap2){
					lastlogo = false;
				}
			}
			//--- 残す箇所（CM手前）の構成内部を構成分割 ---
			if (flagend == true || det){				// 最後の位置またはCM検出した場合
				if (logoon_st){							// 手前にロゴがあった場合
					int msec_lg_remain_ed;				// 残す箇所の終了位置
					if (det){
						msec_lg_remain_ed = bounds.st;
					}
					else{
						msec_lg_remain_ed = -1;
					}
					//--- 残す箇所の内部分割 ---
					RangeMsec rmsec_new = {msec_lg_remain_st, msec_lg_remain_ed};
					mkReformAllLogoUpdate(rmsec_new);
				}
				msec_lg_remain_st = bounds.ed;		// 次の残す箇所の先頭位置
			}
			//--- CMが見つからない時はロゴ期間自体が１構成にならないか確認 ---
			else if (det == false){
				det = mkReformAllLogoUnit(msec_lg_remain_st, logopt);
			}
			//--- CM認識またはロゴ期間が長い場合は次の開始をロゴ終了位置に設定 ---
			Msec msec_recmin = pdata->getConfig(ConfigVarType::msecWLogoLgMin);
			if (flagend == false && (det || (logopt.msecFall - logopt.msecRise) >= msec_recmin)){
				scope.st = logopt.msecFall;
			}
			//--- 次の位置設定 ---
			logoon_st = true;
		}
	}

	//--- 先頭位置の追加処理 ---
	{
		FormFirstInfo tmpinfo;
		setFirstArea(tmpinfo);
	}
	//--- 最後位置の追加処理 ---
	{
		Nsc nsc_final = pdata->sizeDataScp() - 1;
		if (lastlogo == false){
			pdata->setScpArstat(nsc_final, SCP_AR_N_OTHER);
		}
		else{
			pdata->setScpArstat(nsc_final, SCP_AR_L_OTHER);
		}
		setFinalArea();
	}
	//--- 構成要素のロゴ部分分離（MIX/Border追加） ---
	addLogoComponent( pdata->getLevelUseLogo() );
}


//---------------------------------------------------------------------
// ロゴ部分が１構成となる場合は構成追加
// 入力：
//   logopt: 検出するロゴ情報
// 入出力:
//   msec_lg_remain_st: 未設定ロゴ開始位置
// 出力:
//   返り値: 更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::mkReformAllLogoUnit(Msec &msec_lg_remain_st, NrfCurrent &logopt){
	//--- 対応するシーンチェンジ番号を取得 ---
	RangeNsc  rnsc_lg_cur = {-1, -1};
	{
		RangeMsec rmsec_decide = {logopt.msecRise, logopt.msecFall};
		Nsc nsc_decide_st = pdata->getNscFromMsecChap(rmsec_decide.st, pdata->msecValLap2, SCP_CHAP_DECIDE);
		Nsc nsc_decide_ed = pdata->getNscFromMsecChap(rmsec_decide.ed, pdata->msecValLap2, SCP_CHAP_DECIDE);
		Nsc nsc_lg_any_st = pdata->getNscFromMsecChap(rmsec_decide.st, pdata->msecValLap2, SCP_CHAP_NONE);
		Nsc nsc_lg_any_ed = pdata->getNscFromMsecChap(rmsec_decide.ed, pdata->msecValLap2, SCP_CHAP_NONE);
		if (nsc_decide_st >= 0){
			rnsc_lg_cur.st = nsc_decide_st;
		}
		else if (nsc_lg_any_st >= 0){
			rnsc_lg_cur.st = nsc_lg_any_st;
		}
		if (nsc_decide_ed >= 0){
			rnsc_lg_cur.ed = nsc_decide_ed;
		}
		else if (nsc_lg_any_ed >= 0){
			rnsc_lg_cur.ed = nsc_lg_any_ed;
		}
	}
	//--- 対応する無音シーンチェンジがあれば１構成期間の判別 ---
	bool det = false;
	if (rnsc_lg_cur.st >= 0 && rnsc_lg_cur.ed >= 0 && rnsc_lg_cur.st < rnsc_lg_cur.ed){
		RangeMsec rmsec_lg_cur = getRangeMsec(rnsc_lg_cur);
		//--- 現在のロゴ長が認識期間であるか確認 ---
		FormLogoLevelExt extype = {};
		extype.en_long = true;						// 長時間構成を認識
		bool match_len = isLengthLogoLevel(rmsec_lg_cur.ed - rmsec_lg_cur.st, extype);
		if (match_len){
			det = true;
			if (rnsc_lg_cur.st > 0){
				if (pdata->getScpChap(rnsc_lg_cur.st) < SCP_CHAP_DFIX){
					pdata->setScpChap(rnsc_lg_cur.st, SCP_CHAP_DFIX);
				}
			}
			if (rnsc_lg_cur.ed > 0){
				if (pdata->getScpChap(rnsc_lg_cur.ed) < SCP_CHAP_DFIX){
					pdata->setScpChap(rnsc_lg_cur.ed, SCP_CHAP_DFIX);
				}
			}
			if (msec_lg_remain_st < rmsec_lg_cur.st){		// 直前までのロゴ設定
				RangeMsec rmsec_new = {msec_lg_remain_st, rmsec_lg_cur.st};
				mkReformAllLogoUpdate(rmsec_new);
			}
			{
				RangeMsec rmsec_new = {rmsec_lg_cur.st, rmsec_lg_cur.ed};
				mkReformAllLogoUpdate(rmsec_new);
			}
			msec_lg_remain_st = rmsec_lg_cur.ed;		// ロゴ先頭位置を更新
		}
	}
	return det;
}


//---------------------------------------------------------------------
// ロゴ期間の更新
//---------------------------------------------------------------------
void JlsAutoReform::mkReformAllLogoUpdate(RangeMsec rmsec_new){
	RangeNsc rnsc_new;
	pdata->getRangeNscFromRangeMsec(rnsc_new, rmsec_new);
	if (rnsc_new.ed > 0){
		pdata->setScpArstat(rnsc_new.ed, SCP_AR_L_UNIT);	// 念のため設定
	}
	bool cm_inter = true;
	mkRangeInterLogo(rnsc_new, cm_inter);
}


//---------------------------------------------------------------------
// ロゴを使わず構成推測を実行
//---------------------------------------------------------------------
void JlsAutoReform::mkReformAllNologo(JlsCmdArg &cmdarg){
	//--- CM構成を作成 ---
	RangeMsec bounds;
	RangeFixMsec scope_org;
	scope_org.st = -1;				// 検索開始位置を指定しない
	scope_org.ed = -1;				// 検索終了位置を指定しない
	scope_org.fixSt = false;		// 固定設定ではない
	scope_org.fixEd = false;		// 固定設定ではない			
	bool logo1st = true;			// 先頭からロゴありとして検索
	int  interfill = 0;				// CMとCMの間はCMで埋めない
	mkRangeCM(bounds, scope_org, logo1st, interfill);
	//--- メイン推測実行 ---
	FormAutoCMArg param_autocm;
	bool reform_all = true;
	setReformParam(param_autocm, cmdarg, reform_all);
	detectCM(param_autocm);
}

//---------------------------------------------------------------------
// 推測構成作成用のパラメータ設定
//---------------------------------------------------------------------
void JlsAutoReform::setReformParam(FormAutoCMArg &param_autocm, JlsCmdArg &cmdarg, bool reform_all){
	//--- パラメータ取得 ---
	int  level_cmdet = 6;				// 全体設定時のロゴ検出レベルデフォルト値
	Msec msec_tailarea_keep = 0;
	Msec msec_tailarea_unit = 0;
	Sec  sec_maxcm = 60;
	RangeMsec rmsec_headtail = {-1, -1};
	if (!reform_all) rmsec_headtail = pdata->recHold.rmsecHeadTail;
	if (rmsec_headtail.st < 0) rmsec_headtail.st = 0;
	if (rmsec_headtail.ed < 0) rmsec_headtail.ed = pdata->getMsecTotalMax();
	//--- コマンド有効時 ---
	if (cmdarg.cmdsel == CmdType::AutoCM){
		//--- 全体設定時は有効設定のみ更新 ---
		int tmp_level_cmdet = cmdarg.getOpt(OptType::AutopCode) % 100;
		if (reform_all == false || tmp_level_cmdet > 0){
			level_cmdet = tmp_level_cmdet;
		}
		//--- Autoコマンド有効時はAutoMode設定 ---
		if (level_cmdet > 0){
			pdata->setFlagAutoMode(true);
		}
		//--- 最後の位置 ---
		Sec sectmp_keep = cmdarg.getOpt(OptType::AutopScope);
		msec_tailarea_keep = 0;
		if (sectmp_keep > 0){
			msec_tailarea_keep = rmsec_headtail.ed - sectmp_keep * 1000;
		}
		//--- 最後の60/90/120秒判断位置 ---
		Sec sectmp_unit = cmdarg.getOpt(OptType::AutopScopeX);
		msec_tailarea_unit = 0;
		if (sectmp_unit > 0){
			msec_tailarea_unit = rmsec_headtail.ed - sectmp_unit * 1000;
		}
		//--- 期間設定 ---
		sec_maxcm   = cmdarg.getOpt(OptType::AutopMaxPrd);
		if (sec_maxcm <= 0){
			sec_maxcm = 60;			// 未設定時の初期値
		}
	}

	//--- 結果反映 ---
	param_autocm.levelCmDet = level_cmdet;
	param_autocm.msecTailareaKeep = msec_tailarea_keep;
	param_autocm.msecTailareaUnit = msec_tailarea_unit;
	param_autocm.secMaxCm = sec_maxcm;
	param_autocm.rmsecHeadTail = rmsec_headtail;
}



//=====================================================================
// ロゴ情報を使って最後に構成情報追加（ロゴを使った推測後）
//=====================================================================

//---------------------------------------------------------------------
// 構成要素のロゴ部分分離（MIX/Border追加）
// 出力：
//   pdata(chap,arstat)
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
void JlsAutoReform::addLogoComponent(int lvlogo){
		addLogoEdge();
		addLogoBorder();
		addLogoMix();
		return;
}

//---------------------------------------------------------------------
// ロゴ扱い部分でロゴなし部分が多い所の補正
// 出力：
//   pdata(chap,arstat)
//---------------------------------------------------------------------
void JlsAutoReform::addLogoEdge(){
	//--- 認識最小期間 ---
	Msec msec_recmin = pdata->getConfig(ConfigVarType::msecWLogoCmMin);
	//--- ロゴ構成区間内の端構成をCM化する処理設定 ---
	if (pdata->getConfigAction(ConfigActType::LogoDelWide) == 0){
		return;								// カットしない設定では終了
	}
	int rev_del_edge = pdata->getConfigAction(ConfigActType::LogoDelEdge);
	//--- ロゴ位置を順番に検索 ---
	NrfCurrent logopt = {};
	bool remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
	//--- 最初と最後は対象外として検索 ---
	while(remain_logo){
		remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
		//--- 最後のロゴは別途処理（ロゴ扱い終了箇所を次のロゴ開始扱いにする） ---
		bool valid_last = false;
		if (remain_logo == false && logopt.msecLastFall > 0){
			bool over = false;
			ElgCurrent elg = {};
			while( pdata->getElgptNext(elg) && over == false){
				//--- ロゴ扱い構成終了がロゴ立ち下がり以降なら候補とする ---
				if (elg.msecFall > logopt.msecLastFall){
					over = true;
					valid_last = true;
					logopt.msecRise = elg.msecFall;
				}
			}
		}
		if (remain_logo || valid_last){
			if (logopt.msecRise - logopt.msecLastFall >= msec_recmin){
				//--- ロゴ位置取得（前のfallから現在のriseまでの区間） ---
				RangeWideMsec area_nologo;
				pdata->getWideMsecLogoNrf(area_nologo.st, logopt.nrfLastFall);
				pdata->getWideMsecLogoNrf(area_nologo.ed, logopt.nrfRise);
				if (rev_del_edge == 0){			// 不確定部分は使わない設定時
					area_nologo.st.early = area_nologo.st.late  - pdata->msecValSpc;
					area_nologo.ed.late  = area_nologo.ed.early + pdata->msecValSpc;
					if (area_nologo.st.just < area_nologo.st.early){
						area_nologo.st.just = area_nologo.st.early;
					}
					if (area_nologo.ed.just > area_nologo.ed.late){
						area_nologo.ed.just = area_nologo.ed.late;
					}
				}
				else if (rev_del_edge <= 2){	// 中心付近までに制限
					area_nologo.st.early = area_nologo.st.just - pdata->msecValSpc;
					area_nologo.ed.late  = area_nologo.ed.just + pdata->msecValSpc;
				}
				else{
					area_nologo.st.early -= pdata->msecValSpc;
					area_nologo.ed.late  += pdata->msecValSpc;
				}
				if (valid_last){		// 最終ロゴの終了地点を固定
					area_nologo.ed.just  = logopt.msecRise;
					area_nologo.ed.early = logopt.msecRise;
					area_nologo.ed.late  = logopt.msecRise;
				}
				//--- 設定 ---
				addLogoEdgeUpdate(area_nologo, msec_recmin);
			}
		}
	}
}

//---------------------------------------------------------------------
// ロゴ扱い部分でロゴなし部分が多い所の補正-更新処理
// 入力：
//   area_nologo: ロゴなし期間
//   msec_recmin: 更新を実行する最低期間
// 出力：
//   pdata(chap,arstat)
//---------------------------------------------------------------------
void JlsAutoReform::addLogoEdgeUpdate(RangeWideMsec area_nologo, int msec_recmin){
	//--- 検索範囲設定 ---
	RangeNsc rnsc_target;
	RangeNsc rnsc_cmp;
	{
		RangeWideMsec area_search = area_nologo;
		area_search.st.late  = area_nologo.ed.early;
		area_search.ed.early = area_nologo.st.late;
		rnsc_target.st = pdata->getNscFromWideMsecFull(area_search.st, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
		rnsc_target.ed = pdata->getNscFromWideMsecFull(area_search.ed, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
		rnsc_cmp.st    = pdata->getNscFromWideMsecFull(area_search.st, SCP_CHAP_NONE, SCP_END_EDGEIN);
		rnsc_cmp.ed    = pdata->getNscFromWideMsecFull(area_search.ed, SCP_CHAP_NONE, SCP_END_EDGEIN);
	}
	//--- 検索対象がなければ終了 ---
	if (rnsc_cmp.st <= 0 || rnsc_cmp.ed <= 0){
		return;
	}
	//--- 対象位置の選択 ---
	RangeNsc rnsc_select = rnsc_target;			// 使用するロゴなし区間
	//--- 確定以外の箇所にするかチェック ---
	if (rnsc_target.st != rnsc_cmp.st){
		if (rnsc_target.st < 0){
			rnsc_select.st = rnsc_cmp.st;
		}
		else{
			//--- ロゴ認識期間の位置検索 ---
			bool cont = true;
			Nsc nsc_cur = rnsc_cmp.st;
			while(cont && nsc_cur > 0 && nsc_cur < rnsc_target.st){
				Msec msec_dif = pdata->getMsecScp(rnsc_target.st) - pdata->getMsecScp(nsc_cur);
				FormLogoLevelExt extype = {};
				extype.en_long = true;						// 長時間構成を認識
				extype.longmod = true;						// 長時間の5/15秒単位を認識
				if (isLengthLogoLevel(msec_dif, extype)){	// 期間がロゴ認識期間
					rnsc_select.st = nsc_cur;
					cont = false;
				}
				nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_NONE);
			}
			//--- 最終地点まで離れていたら認識期間に関係なく設定 ---
			if (rnsc_select.st != rnsc_cmp.st){
				Msec msec_dif = pdata->getMsecScp(rnsc_select.st) - pdata->getMsecScp(rnsc_cmp.st);
				if (msec_dif >= msec_recmin){
					rnsc_select.st = rnsc_cmp.st;
				}
			}
		}
	}
	if (rnsc_target.ed != rnsc_cmp.ed){
		if (rnsc_target.ed < 0){
			rnsc_select.ed = rnsc_cmp.ed;
		}
		else{
			//--- ロゴ認識期間の位置検索 ---
			bool cont = true;
			Nsc nsc_cur = rnsc_cmp.ed;
			while(cont && nsc_cur > 0 && nsc_cur > rnsc_target.ed){
				Msec msec_dif = pdata->getMsecScp(nsc_cur) - pdata->getMsecScp(rnsc_target.ed);
				FormLogoLevelExt extype = {};
				extype.en_long = true;						// 長時間構成を認識
				extype.longmod = true;						// 長時間の5/15秒単位を認識
				if (isLengthLogoLevel(msec_dif, extype)){	// 期間がロゴ認識期間
					rnsc_select.ed = nsc_cur;
					cont = false;
				}
				nsc_cur = pdata->getNscPrevScpChap(nsc_cur, SCP_CHAP_NONE);
			}
			//--- 最終地点まで離れていたら認識期間に関係なく設定 ---
			if (rnsc_select.ed != rnsc_cmp.ed){
				Msec msec_dif = pdata->getMsecScp(rnsc_cmp.ed) - pdata->getMsecScp(rnsc_select.ed);
				if (msec_dif >= msec_recmin){
					rnsc_select.ed = rnsc_cmp.ed;
				}
			}
		}
	}
	//--- 位置取得 ---
	RangeMsec rmsec_select;			// 使用するロゴなし区間（ミリ秒）
	rmsec_select.st = pdata->getMsecScp(rnsc_select.st);
	rmsec_select.ed = pdata->getMsecScp(rnsc_select.ed);
	//--- 期間チェック ---
	if (rmsec_select.ed - rmsec_select.st >= msec_recmin){
		ElgCurrent elg = {};
		while( pdata->getElgptNext(elg) ){
			//--- ロゴなし区間内でロゴ扱いの範囲を取得 ---
			RangeNsc rnsc_del = rnsc_select;	// 削除するロゴなし区間
			if (rnsc_del.st < elg.nscRise){
				rnsc_del.st = elg.nscRise;
			}
			if (rnsc_del.ed > elg.nscFall){
				rnsc_del.ed = elg.nscFall;
			}
			if (rnsc_del.st < rnsc_del.ed){
				RangeMsec rmsec_del;
				rmsec_del.st = pdata->getMsecScp(rnsc_del.st);
				rmsec_del.ed = pdata->getMsecScp(rnsc_del.ed);
				//--- ロゴなしのロゴ扱い期間が設定値以上、または構成全体が削除範囲内なら更新 ---
				if ((rmsec_del.ed - rmsec_del.st >= msec_recmin) ||
					(rnsc_del.st <= elg.nscRise && rnsc_del.ed >= elg.nscFall)){
					ScpChapType chap_st = pdata->getScpChap(rnsc_del.st);
					ScpChapType chap_ed = pdata->getScpChap(rnsc_del.ed);
					if (chap_st < SCP_CHAP_DFIX){
						pdata->setScpChap(rnsc_del.st, SCP_CHAP_DFIX);	// 更新
						pdata->setScpArstat(rnsc_del.st, SCP_AR_L_OTHER);
					}
					if (chap_ed < SCP_CHAP_DFIX){
						pdata->setScpChap(rnsc_del.ed, SCP_CHAP_DFIX);	// 更新
						pdata->setScpArstat(rnsc_del.ed, SCP_AR_N_OTHER);
					}
					Nsc nsc_tmp = rnsc_del.st;
					while(nsc_tmp > 0 && nsc_tmp < rnsc_del.ed){
						nsc_tmp = pdata->getNscNextScpChap(nsc_tmp, SCP_CHAP_DECIDE);
						if (nsc_tmp > 0 && nsc_tmp <= rnsc_del.ed){
							//--- ロゴなし扱いに変更 ---
							ScpArType arstat_tmp = pdata->getScpArstat(nsc_tmp);
							ScpArType arstat_new = arstat_tmp;
							if (arstat_tmp == SCP_AR_L_UNIT){
								arstat_new = SCP_AR_N_UNIT;
							}
							else if (arstat_tmp == SCP_AR_L_OTHER){
								arstat_new = SCP_AR_N_OTHER;
							}
							if (arstat_tmp != arstat_new){
								pdata->setScpArstat(nsc_tmp, arstat_new);	// 更新
							}
						}
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------
// 構成要素にMixを追加（Border分離できなかった構成）
// 出力：
//   pdata(arstat)
//---------------------------------------------------------------------
void JlsAutoReform::addLogoMix(){
	NrfCurrent logopt = {};
	bool remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
	if (remain_logo == false){
		return;
	}
	Nsc nsc_last     = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
	while(nsc_last > 0){
		Nsc  nsc_cur   = pdata->getNscNextScpChap(nsc_last, SCP_CHAP_DECIDE);		// 次の構成
		Msec msec_cur  = pdata->getMsecScp(nsc_cur);
		Msec msec_last = pdata->getMsecScp(nsc_last);
		int sumlogo   = 0;
		bool cmpnext = true;
		while(nsc_cur > 0 && remain_logo && cmpnext && logopt.msecRise < msec_cur - pdata->msecValLap2){
			if (remain_logo){
				if (logopt.msecFall > msec_last + pdata->msecValLap2){
					sumlogo ++;			// ロゴ内をカウント
				}
				//--- 次のロゴ位置移行処理 ---
				if (logopt.msecFall > msec_cur - pdata->msecValLap2){
					cmpnext = false;
				}
				else{
					remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
				}
			}
		}
		//--- ロゴが内部に２つ以上あればMix ---
		if (sumlogo > 1){
			ScpArType arstat_cur = pdata->getScpArstat(nsc_cur);
			if (arstat_cur == SCP_AR_L_OTHER){
				pdata->setScpArstat(nsc_cur, SCP_AR_L_MIXED);
			}
		}
		nsc_last = nsc_cur;
	}
}

//---------------------------------------------------------------------
// 構成要素のロゴ部分分離（Border追加）
// 出力：
//   pdata(chap,arstat)
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
void JlsAutoReform::addLogoBorder(){
	NrfCurrent logopt = {};
	bool remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
	if (remain_logo == false){
		return;
	}
	Nsc nsc_last = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
	//--- 各構成位置のロゴを順番に確認 ---
	while(nsc_last > 0){
		Nsc  nsc_cur   = pdata->getNscNextScpChap(nsc_last, SCP_CHAP_DECIDE);		// 次の構成
		Msec msec_cur  = pdata->getMsecScp(nsc_cur);
		Msec msec_last = pdata->getMsecScp(nsc_last);
		Msec msec_res  = msec_last;
		bool enable_st = true;		// 先頭位置の確定状態
		bool cmpnext = true;		// 次のロゴ位置も比較するか
		//--- 現構成位置までのロゴを順番に確認 ---
		while(nsc_cur > 0 && remain_logo && cmpnext && logopt.msecRise < msec_cur - pdata->msecValLap2){
			if (remain_logo){
				//--- ロゴ立ち上がりの構成判別 ---
				RangeMsec component = {msec_last, msec_cur};
				RangeNrf  rnrf_logo = {logopt.nrfRise, logopt.nrfFall};
				msec_res = addLogoBorderSub(component, rnrf_logo, enable_st);
				enable_st = (msec_res >= 0)? true : false;
				if (msec_res > msec_last){
					msec_last = msec_res;
				}
				//--- 次のロゴ位置移行処理 ---
				if (logopt.msecFall > msec_cur - pdata->msecValLap2){
					cmpnext = false;			// 現在の構成内はロゴ終了
				}
				else{
					remain_logo = pdata->getNrfptNext(logopt, LOGO_SELECT_VALID);
				}
			}
			//--- ロゴ立ち下がりの構成判別 ---
			if (logopt.nrfLastFall > 0 && cmpnext){
				RangeMsec component = {msec_last, msec_cur};
				RangeNrf  rnrf_logo = {logopt.nrfLastFall, logopt.nrfRise};
				msec_res = addLogoBorderSub(component, rnrf_logo, enable_st);
				enable_st = (msec_res >= 0) ? true : false;
				if (msec_res > msec_last){
					msec_last = msec_res;
				}
			}
		}
		//--- 区切り挿入で番号が変わることあるため位置を再取得 ---
		if (nsc_cur > 0){
			nsc_last = pdata->getNscFromMsecAll(msec_cur);
			if (nsc_last > 0 && nsc_last < nsc_cur){	// 念のため戻ることないように保険
				nsc_last = nsc_cur;
			}
		}
		else{
			nsc_last = nsc_cur;
		}
	}
	//--- 構成情報にBorder付加 ---
	addLogoBorderSet();
}

//---------------------------------------------------------------------
// 構成区切り情報からBorder追加
// 出力：
//   pdata(arstat)
//---------------------------------------------------------------------
void JlsAutoReform::addLogoBorderSet(){
	Nsc nsc_last = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
	Nsc nsc_cur  = pdata->getNscNextScpChap(nsc_last, SCP_CHAP_DECIDE);		// 次の構成
	while(nsc_cur > 0){
		Nsc nsc_next  = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);	// 次の構成
		//--- border区切り検出 ---
		if (pdata->getScpChap(nsc_cur) == SCP_CHAP_DBORDER){
			//--- 前側の判別 ---
			ScpArType arstat_cur = pdata->getScpArstat(nsc_cur);
			if (arstat_cur == SCP_AR_N_UNIT){
				pdata->setScpArstat(nsc_cur, SCP_AR_B_UNIT);
			}
			else if (arstat_cur == SCP_AR_N_OTHER){
				pdata->setScpArstat(nsc_cur, SCP_AR_B_OTHER);
			}
			else if (jlsd::isScpArTypeLogo(arstat_cur)){
				bool enable_short = false;
				addCommonComponentOne(nsc_cur, nsc_last, enable_short);
			}
			//--- 後側の判別 ---
			if (nsc_next > 0){
				ScpArType arstat_next = pdata->getScpArstat(nsc_next);
				if (arstat_next == SCP_AR_N_UNIT){
					pdata->setScpArstat(nsc_next, SCP_AR_B_UNIT);
				}
				else if (arstat_next == SCP_AR_N_OTHER){
					pdata->setScpArstat(nsc_next, SCP_AR_B_OTHER);
				}
				else if (jlsd::isScpArTypeLogo(arstat_next)){
					bool enable_short = false;
					addCommonComponentOne(nsc_cur, nsc_next, enable_short);
				}
			}
		}
		//--- 次の位置設定 ---
		nsc_last = nsc_cur;
		nsc_cur  = nsc_next;
	}
}

//---------------------------------------------------------------------
// 単位構成とロゴ情報２点から単位構成内を分割
// 入力：
//   component: 対象の現在構成
//   rnrf_logo: ロゴの開始位置と終了位置の番号
//   enable_st: 先頭位置の確定状態（false=未確定、true=確定）
// 出力：
//   返り値:構成確定した位置（ミリ秒）
//   pdata(chap,arstat)
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Msec JlsAutoReform::addLogoBorderSub(RangeMsec component, RangeNrf rnrf_logo, bool enable_st){
	Msec msec_ret = -1;

	if (rnrf_logo.st < 0) return -1;
	//--- ロゴ位置を取得し対象範囲か確認 ---
	RangeMsec logo_targets = component;
	//--- 前のロゴ位置が対象範囲か確認 ---
	{
		WideMsec wmsec_logost;
		pdata->getWideMsecLogoNrf(wmsec_logost, rnrf_logo.st);
		if (wmsec_logost.early < component.st + pdata->msecValLap2) {	// ロゴ開始と構成開始が同じ
			if (enable_st) {
				return component.st;
			}
			else {
				return -1;
			}
		}
		else if (wmsec_logost.late > component.ed - pdata->msecValLap2) {	// ロゴ開始が構成終了付近
			return component.ed;
		}
		logo_targets.st = wmsec_logost.just;
		//--- 対応箇所の無音シーンチェンジあるか確認 ---
		Nsc nsc_inspos = pdata->getNscFromMsecAll(wmsec_logost.just);
		if (nsc_inspos >= 0) {
			logo_targets.st = pdata->getMsecScp(nsc_inspos);	// ロゴに対応する無音シーンチェンジ
		}
	}
	//--- 後のロゴ位置が対象範囲か確認 ---
	if (rnrf_logo.ed >= 0) {
		WideMsec wmsec_logoed;
		pdata->getWideMsecLogoNrf(wmsec_logoed, rnrf_logo.ed);
		//--- ロゴ期間がほとんどなければ対象外 ---
		if (wmsec_logoed.early < logo_targets.st + pdata->msecValLap2) {
			return component.st;
		}
		//--- 次のロゴが構成途中位置にある場合 ---
		else if (wmsec_logoed.late <= component.ed - pdata->msecValLap2) {
			logo_targets.ed = wmsec_logoed.just;
		}
	}
	//--- ロゴ位置に対応した構成分割を実行 ---
	bool rise_logost = jlsd::isLogoEdgeRiseFromNrf(rnrf_logo.st);
	msec_ret = addLogoBorderSubUpdate(component, logo_targets, rise_logost, enable_st);
	return msec_ret;
}

//---------------------------------------------------------------------
// ロゴ切り替わり位置の構成情報について分割実行
// 入力：
//   component:     対象の現在構成
//   logo_targets:  Border判断するロゴ位置２箇所
//   rise_logost:   先頭のロゴ位置は立ち上がりか
//   enable_st:     先頭位置の確定状態（false=未確定、true=確定）
// 出力：
//   pdata(chap,arstat)
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Msec JlsAutoReform::addLogoBorderSubUpdate(RangeMsec component, RangeMsec logo_targets, bool rise_logost, bool enable_st){
	Msec msec_ret = -1;

	//--- 構成前後の位置から対象範囲か確認 ---
	if (logo_targets.st <= 0) return -1;

	//--- 構成前後の距離取得 ---
	bool unit15s_logost = false;		// ロゴ前半部分までの構成が15秒単位か
	bool unit15s_logoed = false;		// ロゴ後半部分までの構成が15秒単位か
	int  update_logost = 0;				// ロゴ前半部分で書き換えるか
	int  update_logoed = 0;				// ロゴ後半部分で書き換えるか
	{
		CalcDifInfo calc1;
		CalcDifInfo calc2;
		int type1 = calcDifSelect(calc1, component.st, logo_targets.st);
		int type2 = calcDifSelect(calc2, logo_targets.st, logo_targets.ed);
		//--- 構成更新判断 ---
		int chknum1 = 0;
		int chknum2 = 0;
		for(int i=0; i<2; i++){
			int types;
			CalcDifInfo calcs;
			if (i==0){
				types = type1;
				calcs = calc1;
			}
			else{
				types = type2;
				calcs = calc2;
			}
			int lvlogo = pdata->getLevelUseLogo();
			if ((types > 0 && calcs.sec % 15 == 0 && calcs.sec >= 15) ||
				(types > 0 && calcs.sec % 5 == 0 && lvlogo >= CONFIG_LOGO_LEVEL_USE_MIDH) ||
				(types > 0 && calcs.sec >= 3 && lvlogo >= CONFIG_LOGO_LEVEL_USE_HIGH)) {
				if (i==0){
					chknum1 = 5;
				}
				else{
					chknum2 = 5;
				}
			}
		}
		if (chknum1 < 5){
			if (calc1.sec > 30){
				chknum1 = 4;
			}
			else if (type1 > 0 && calc1.sec > calc2.sec + 5){
				chknum1 = 3;
			}
		}
		if (chknum2 < 5){
			if (calc2.sec > 30){
				chknum2 = 4;
			}
			else if (type2 > 0 && calc2.sec > calc1.sec + 5){
				chknum2 = 3;
			}
		}
		//--- 構成情報の取得 ---
		Nsc nsc_ed = pdata->getNscFromMsecAll(component.ed);
		ScpArType arstat_ed = pdata->getScpArstat(nsc_ed);
		bool flag_arstat_logo = jlsd::isScpArTypeLogo(arstat_ed);	// 対象構成がロゴ有か
		//--- 構成更新位置を取得 ---
		if (chknum1 + chknum2 >= 7){
			if ((flag_arstat_logo == true) && (rise_logost == true)){
				if (type1 > 0 && calc1.sec <= 60 && enable_st){
					update_logost = 1;				// 前半をロゴなしに設定
				}
			}
			else if ((flag_arstat_logo == false) && (rise_logost == false)){
				if (type1 > 0 && calc1.sec <= 60 && enable_st){
					update_logost = 2;				// 前半をロゴありに設定
				}
			}
			else if ((flag_arstat_logo == true) && (rise_logost == false)){
				if (type2 > 0 && calc2.sec <= 60){
					update_logoed = 1;				// ロゴ位置後をロゴなしに設定
				}
			}
			else if ((flag_arstat_logo == false) && (rise_logost == true)){
				if (type2 > 0 && calc2.sec <= 60){
					update_logoed = 2;				// ロゴ位置後をロゴありに設定
				}
			}
		}
		//--- 15秒構成判断 ---
		if (calc1.sec % 15 == 0) unit15s_logost = true;
		if (calc2.sec % 15 == 0) unit15s_logoed = true;
	}
	//--- 構成を更新 ---
	if (update_logost > 0 || update_logoed > 0){
		bool border_logost = false;
		bool border_logoed = false;
		Nsc  nsc_logost = pdata->getNscFromMsecAll(logo_targets.st);
		//--- 無音シーンチェンジがなければ強制的に作成してBorderにする ---
		if (nsc_logost < 0){
			nsc_logost = pdata->getNscForceMsec(logo_targets.st, LOGO_EDGE_RISE);
			border_logost = true;
		}
		//--- 最初のロゴ位置の構成変更 ---
		if (update_logost > 0 || update_logoed > 0){
			msec_ret = logo_targets.st;
			//--- chap設定 ---
			if (border_logost){
				pdata->setScpChap(nsc_logost, SCP_CHAP_DBORDER);
			}
			else if (pdata->getScpChap(nsc_logost) < SCP_CHAP_DINT){
				pdata->setScpChap(nsc_logost, SCP_CHAP_DINT);
			}
			//--- arstat設定 ---
			if (update_logost == 1){
				ScpArType arstat_st = (unit15s_logost)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
				pdata->setScpArstat(nsc_logost, arstat_st);
			}
			else if (update_logost == 2){
				ScpArType arstat_st = (unit15s_logost)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
				pdata->setScpArstat(nsc_logost, arstat_st);
			}
		}
		//--- 次の位置の構成変更 ---
		if (update_logoed > 0){
			msec_ret = logo_targets.ed;
			int nsc_logoed = pdata->getNscFromMsecAll(logo_targets.ed);
			//--- 無音シーンチェンジがなければ強制的に作成してBorderにする ---
			if (nsc_logoed < 0){
				nsc_logoed = pdata->getNscForceMsec(logo_targets.ed, LOGO_EDGE_RISE);
				border_logoed = true;
			}
			//--- 区切り情報を更新 ---
			if (border_logoed){
				pdata->setScpChap(nsc_logoed, SCP_CHAP_DBORDER);
			}
			else if (pdata->getScpChap(nsc_logoed) < SCP_CHAP_DINT){
				pdata->setScpChap(nsc_logoed, SCP_CHAP_DINT);
			}
			//--- 対象位置の構成情報を更新 ---
			if (update_logoed == 1){
				ScpArType arstat_st = (unit15s_logost)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
				ScpArType arstat_ed = (unit15s_logoed)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
				pdata->setScpArstat(nsc_logost, arstat_st);
				pdata->setScpArstat(nsc_logoed, arstat_ed);
			}
			else if (update_logoed == 2){
				ScpArType arstat_st = (unit15s_logost)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
				ScpArType arstat_ed = (unit15s_logoed)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
				pdata->setScpArstat(nsc_logost, arstat_st);
				pdata->setScpArstat(nsc_logoed, arstat_ed);
			}
		}
	}
	return msec_ret;
}



//=====================================================================
// ロゴを使わずCM構成を推測
//=====================================================================

//---------------------------------------------------------------------
// ロゴなしでCM位置を推測
//---------------------------------------------------------------------
bool JlsAutoReform::detectCM(FormAutoCMArg param_autocm){
	AutoCMCount cntset[AUTOCM_NUM_SMAX];

	//--- パラメータ取得 ---
	int  level_cmdet         = param_autocm.levelCmDet;
	Msec msec_tailarea_keep  = param_autocm.msecTailareaKeep;
	Msec msec_tailarea_unit  = param_autocm.msecTailareaUnit;
	Sec  sec_maxcm           = param_autocm.secMaxCm;
	RangeMsec rmsec_headtail = param_autocm.rmsecHeadTail;
	Msec msec_wlogo_trmax    = pdata->getConfig(ConfigVarType::msecWLogoTRMax);
	//--- ロゴ構成区間内の15秒構成をCM化の判断追加設定 ---
	int sub_autocm = pdata->getConfig(ConfigVarType::AutoCmSub);
	bool is_sub_cmhead = ((sub_autocm % 10) == 1)? true : false;
	bool is_sub_off45s = ((sub_autocm / 10 % 10) == 1)? true : false;

	//--- 無効確認 ---
	if (level_cmdet <= 0){
		if (level_cmdet < 0) pdata->setFlagAutoMode(false);		// 認識済みで無効の状態に戻す
		return false;
	}

	//--- 初期化 ---
	detectCMCount(cntset, AUTOCM_T_INIT, AUTOCM_NUM_SMAX, AUTOCM_NUM_SMAX);

	//--- 先頭位置処理 ---
	FormFirstInfo info_first;
	if (rmsec_headtail.st <= 0){
		setFirstArea(info_first);
	}
	else{							// 開始が全体の先頭ではない場合
		info_first.limitHead = AUTOFIRST_STRICT;	// 先頭CMは厳しめ
		info_first.nscTreat = pdata->getNscFromMsecChap(rmsec_headtail.st, -1, SCP_CHAP_DECIDE);
	}
	Nsc nsc_last = info_first.nscTreat;
	Nsc nsc_detend = -1;
	if (nsc_last < 0){
		nsc_last = 0;
	}
	//--- CM扱いを検索・追加 ---
	bool flag_final = false;
	AutoCMStateType  state = AUTOCM_ST_S0N_FIRST;
	cntset[AUTOCM_ST_S0N_FIRST].rnsc.st = nsc_last;
	Nsc i = nsc_last;
	while(i >= 0 && flag_final == false){
		//--- 次の位置設定（構成区切り、CM単位優先） ---
		i = pdata->getNscNextScpCheckCmUnit(i, SCP_END_EDGEIN);
		//--- 秒数取得と最終位置判断 ---
		if (i >= 0){
			Msec msec_i  = pdata->getMsecScp(i);
			if (msec_i > rmsec_headtail.ed){		// 終了フレームを超えたら最終にセット
				flag_final = true;
				nsc_detend = nsc_last;
			}
		}
		else if (flag_final == false){		// 次がなくても完了前なら最終にセット
			int num_scpos = pdata->sizeDataScp();
			flag_final = true;
			i = num_scpos - 1;
		}
		if (i >= 0){
			Msec msec_last = pdata->getMsecScp(nsc_last);
			Msec msec_i    = pdata->getMsecScp(i);
			Sec  secdif_cur = pdata->cnv.getSecFromMsec(msec_i - msec_last);
			bool change = false;
			int cutlevel = level_cmdet;
			bool is_state_logo = false;
			if (state == AUTOCM_ST_S1L_LOGO || state == AUTOCM_ST_S3L_DEFER){
				is_state_logo = true;
			}
			bool flag_cm = false;
			if (((secdif_cur % 15)==0 && secdif_cur <= sec_maxcm) ||
				((secdif_cur % 30)==0 && secdif_cur <= 120 && cutlevel >= 10)){
				if (is_sub_off45s == false || (secdif_cur % 30)==0 || secdif_cur <= 30){
					flag_cm = true;
				}
			}
			int type_mute = 0;
			{	// 無音が多く境界にシーンチェンジない時はCMから外す
				RangeNsc rnsc = {nsc_last, i};
				type_mute = checkMuteMany(rnsc);
				if (type_mute >= 2){
					flag_cm = false;
				}
			}
			bool flag_tailcm = false;
			if (msec_tailarea_keep > 0 && msec_tailarea_keep < msec_i &&
				(msec_i - msec_last <= msec_wlogo_trmax) &&
				(is_state_logo == false || cntset[state].dov15 == 0)){
				flag_tailcm = true;
			}
			if (msec_tailarea_unit > 0 && msec_tailarea_unit < msec_i &&
				(secdif_cur >= 60 && secdif_cur <= 120 && (secdif_cur % 30 == 0))){
				flag_tailcm = true;
			}
			if (is_state_logo == false){			// CM候補検索
				if (flag_cm || flag_tailcm){
					cntset[state].rnsc.ed = i;
				}
				else{
					change = true;
				}
			}
			else if (is_state_logo == true){		// ロゴ期間検索
				if (flag_cm || flag_tailcm){
					change = true;
				}
				else{
					cntset[state].rnsc.ed = i;
				}
			}
			if (flag_final){	// 最後は強制設定
				change = true;
			}
			//--- 変化があった時に判断 ---
			if (change){
				//--- CM扱い部分完了時 ---
				if (is_state_logo == false){
					//--- 先頭位置カット部分の補正 ---
					if (state == AUTOCM_ST_S0N_FIRST){
						switch (info_first.limitHead){
							case AUTOFIRST_STRICT:		// 先頭カット厳しめ
								if (cutlevel > 3){
									cutlevel -= 3;
								}
								else if (cutlevel > 1){
									cutlevel = 1;
								}
								break;
							case AUTOFIRST_NOCUT:		// 先頭カットなし
								cutlevel = 0;
								break;
							case AUTOFIRST_LAX:			// 先頭カット条件緩く
								if (cutlevel > 0) cutlevel += 3;
								break;
						}
						if (is_sub_cmhead){		// 先頭の15秒単位を積極的にカット
							if (cutlevel < 9){
								cutlevel = 9;
							}
						}
					}
					//--- 最終地点の補正 ---
					if (flag_final){
						cutlevel = 10;
					}
					//--- 不確定先送り地点の判断 ---
					if (state == AUTOCM_ST_S4N_JUDGE){
						if ((cntset[AUTOCM_ST_S1L_LOGO].det +
							 cntset[AUTOCM_ST_S3L_DEFER].det <= 4) &&
							(cntset[AUTOCM_ST_S1L_LOGO].sec +
							 cntset[AUTOCM_ST_S3L_DEFER].sec >= 120)){
							//--- 対象部分をCM扱いに変更 ---
							//--- 手前ロゴ部分がCM扱いでないか判断後シフト ---
							if (cntset[AUTOCM_ST_S1L_LOGO].dov15 > 0){
								detectCMSetLogo(cntset[AUTOCM_ST_S1L_LOGO].rnsc);
							}
							detectCMCount(cntset, AUTOCM_T_SFT, AUTOCM_ST_S0N_FIRST, AUTOCM_ST_S2N_CM);
						}
						else{
							//--- ロゴ扱い部分として継続 ---
							detectCMCount(cntset, AUTOCM_T_MRG, AUTOCM_ST_S1L_LOGO, AUTOCM_ST_S3L_DEFER);
						}
						state = AUTOCM_ST_S2N_CM;
					}
					//--- 対象地点をロゴなし扱いにするか判断 ---
					// cutlevel 0 : カットなし
					//          1 : 4構成以上CM
					//          2 : 4構成以上CM、3構成の一部（周囲構成少）
					//          3 : 4構成以上CM、3構成の一部
					//          4 : 3構成以上CM
					//          5 : 3構成以上CM、2構成の一部（周囲構成少）
					//          6 : 3構成以上CM、2構成の一部
					//          7 : 2構成以上CM
					//          8 : 2構成以上CM、1構成の一部（周囲構成少）
					//          9 : 1構成以上CM
					//          10 : 1構成以上CM、120秒構成まで許可
					int dettype = 0;
					if ((cntset[state].det > 3 && cutlevel > 0) ||
						(cntset[state].det > 2 && cutlevel > 3) ||
						(cntset[state].det > 1 && cutlevel > 6) ||
						(cntset[state].det > 0 && cutlevel > 8)){
						dettype = 1;		// 確定
					}
					else if ((cntset[state].det == 2 &&
							  (cutlevel == 5 || cutlevel == 6) &&
							  (cntset[state].d15 >= 2 || cntset[state].d30 >= 2)) ||
							 (cntset[state].det == 3 &&
							  (cutlevel == 2 || cutlevel == 3) &&
							  (cntset[state].d15 + cntset[state].d30 >= 3)) ||
							 (cntset[state].det == 1 &&
							  (cutlevel == 8) &&
							  (cntset[state].d15 + cntset[state].d30 >= 1))){
						if (state == AUTOCM_ST_S0N_FIRST || cutlevel == 3 || cutlevel == 6){
							dettype = 1;		// 先頭位置、周囲関係なくだったら確定
						}
						else{
							dettype = 2;		// 微妙先送り
						}
					}
					//--- ロゴなし扱いにする変更処理 ---
					if ((dettype == 1) || (dettype == 2 && state == AUTOCM_ST_S0N_FIRST)){
						//--- 対象部分をCM扱いに変更 ---
						//--- 手前ロゴ部分がCM扱いでないか判断後シフト ---
						if (state != AUTOCM_ST_S0N_FIRST){
							if (cntset[AUTOCM_ST_S1L_LOGO].dov15 > 0){
								detectCMSetLogo(cntset[AUTOCM_ST_S1L_LOGO].rnsc);
							}
							else{
								detectCMCancelShort(cntset[AUTOCM_ST_S1L_LOGO].rnsc);
							}
							detectCMCount(cntset, AUTOCM_T_SFT, AUTOCM_ST_S0N_FIRST, AUTOCM_ST_S2N_CM);
						}
						state = AUTOCM_ST_S1L_LOGO;
						cntset[state].rnsc.st = nsc_last;
						cntset[state].rnsc.ed = i;
					}
					else if (dettype == 2){
						//--- 判断は先送り ---
						state = AUTOCM_ST_S3L_DEFER;
						cntset[state].rnsc.st = nsc_last;
						cntset[state].rnsc.ed = i;
					}
					else if (state == AUTOCM_ST_S0N_FIRST){
						//--- ロゴ扱い部分として継続 ---
						state = AUTOCM_ST_S1L_LOGO;
						cntset[state].rnsc.st = cntset[AUTOCM_ST_S0N_FIRST].rnsc.st;
						cntset[state].rnsc.ed = i;
					}
					else{
						//--- ロゴ扱い部分として継続 ---
						if (cntset[AUTOCM_ST_S1L_LOGO].dov15 > 0){
							//--- 15秒以上のロゴあり部分あれば確定して置き換え ---
							detectCMSetLogo(cntset[AUTOCM_ST_S1L_LOGO].rnsc);
							detectCMCount(cntset, AUTOCM_T_SFT, AUTOCM_ST_S1L_LOGO, AUTOCM_ST_S2N_CM);
						}
						else{
							//--- 15秒以上のロゴあり部分なければ現在のロゴあり部分に追加 ---
							detectCMCount(cntset, AUTOCM_T_MRG, AUTOCM_ST_S1L_LOGO, AUTOCM_ST_S2N_CM);
						}
						state = AUTOCM_ST_S1L_LOGO;
//						cntset[state].rnsc.st = nsc_last;
						cntset[state].rnsc.ed = i;
						cntset[state].dov15 = 0;		// 15秒以上カウントは初期化
					}
				}
				//--- ロゴ扱い部分完了時 ---
				else{
					if (flag_final){		// 最後位置
						if (cntset[state].dov15 > 0){
							detectCMSetLogo(cntset[state].rnsc);
						}
						else{
							detectCMCount(cntset, AUTOCM_T_SFT, AUTOCM_ST_S0N_FIRST, state);
							flag_cm = true;
						}
					}
					// state ++
					if (state == AUTOCM_ST_S3L_DEFER){
						state = AUTOCM_ST_S4N_JUDGE;
					}
					else{
						state = AUTOCM_ST_S2N_CM;
					}
					cntset[state].rnsc.st = nsc_last;
					cntset[state].rnsc.ed = i;
				}
				if (flag_final){				// 最後位置
					if (flag_cm || flag_tailcm){
						//--- 最終構成未設定ならロゴなしにセット ---
						if (pdata->getScpArstat(i) == SCP_AR_UNKNOWN){
							pdata->setScpArstat(i, SCP_AR_N_OTHER);
						}
					}
				}
			}
			//--- カウント ---
			cntset[state].det ++;
			cntset[state].sec += secdif_cur;
			if (secdif_cur == 15){
				cntset[state].d15 ++;
			}
			else if (secdif_cur == 30){
				cntset[state].d30 ++;
			}
			else if (secdif_cur >= 15){
				cntset[state].dov15 ++;
			}
			nsc_last = i;
		}
	}
	//--- ロゴ情報からの補正 ---
	detectCMAssistLogo(info_first.nscTreat, nsc_detend);
	//--- 最終位置処理 ---
	if (rmsec_headtail.ed >= pdata->getMsecTotalMax()){
		setFinalArea();
	}

	return true;
}

//---------------------------------------------------------------------
// カウント状態を変更
// 出力：
//  cntset  : CM構成候補検索用のカウント状態
//---------------------------------------------------------------------
void JlsAutoReform::detectCMCount(AutoCMCount *cntset, AutoCMCommandType type, AutoCMStateType st, AutoCMStateType ed){

	if (type == AUTOCM_T_INIT){
		AutoCMStateType stnew = (st == AUTOCM_NUM_SMAX)? AUTOCM_ST_S0N_FIRST : st;
		AutoCMStateType ednew = (ed == AUTOCM_NUM_SMAX)? AUTOCM_ST_S4N_JUDGE : ed;
		//--- 初期化処理 ---
		for(int i=stnew; i<=ednew; i++){
			cntset[i] = {};
		}
	}
	else if (type == AUTOCM_T_SFT){
		//--- シフト処理 ---
		for(int i=st; i<AUTOCM_NUM_SMAX; i++){
			if (i < AUTOCM_NUM_SMAX+st-ed){
				cntset[i] = cntset[i+ed-st];
			}
			else{
				cntset[i] = {};
			}
		}
	}
	else if (type == AUTOCM_T_MRG){
		//--- 合併処理 ---
		for(int i=st+1; i<=ed; i++){
			cntset[st].det += cntset[i].det;
			cntset[st].sec += cntset[i].sec;
			cntset[st].d15 += cntset[i].d15;
			cntset[st].d30 += cntset[i].d30;
			cntset[st].dov15 += cntset[i].dov15;
		}
		cntset[st].rnsc.ed = cntset[ed].rnsc.ed;
		//--- 残りをシフト ---
		for(int i=st+1; i<AUTOCM_NUM_SMAX; i++){
			if (i < AUTOCM_NUM_SMAX+st-ed){
				cntset[i] = cntset[i+ed-st];
			}
			else{
				cntset[i] = {};
			}
		}
	}
}

//---------------------------------------------------------------------
// ２点間を残す領域に設定
//---------------------------------------------------------------------
bool JlsAutoReform::detectCMSetLogo(RangeNsc rnsc){
	if (pdata->getScpChap(rnsc.st) < SCP_CHAP_DFIX){
		pdata->setScpChap(rnsc.st, SCP_CHAP_DFIX);
	}
	if (pdata->getScpChap(rnsc.ed) < SCP_CHAP_DFIX){
		pdata->setScpChap(rnsc.ed, SCP_CHAP_DFIX);
	}
	pdata->setScpArstat(rnsc.ed, SCP_AR_L_OTHER);
	bool cm_inter = false;				// ロゴ区間内のCMチェックしない
	bool ret = mkRangeInterLogo(rnsc, cm_inter);
	return ret;
}

//---------------------------------------------------------------------
// ２点間が短い時はロゴあり属性を削除
//---------------------------------------------------------------------
bool JlsAutoReform::detectCMCancelShort(RangeNsc rnsc){
	if (pdata->getScpChap(rnsc.st) < SCP_CHAP_DFIX ||
		pdata->getScpChap(rnsc.ed) < SCP_CHAP_DFIX){
		return false;
	}
	bool ret = false;
	Msec msec_st = pdata->getMsecScp(rnsc.st);
	Msec msec_ed = pdata->getMsecScp(rnsc.ed);
	Sec sec_dif = pdata->cnv.getSecFromMsec(msec_ed - msec_st);
	//--- 全体で15秒以下の時はロゴを外す ---
	if (sec_dif >= 0 && sec_dif < 15){
		int i = rnsc.st;
		while(i > 0 && i < rnsc.ed){
			i = pdata->getNscNextScpChap(i, SCP_CHAP_DFIX);
			if (i > 0 && i <= rnsc.ed){
				ScpArType arstat_i = pdata->getScpArstat(i);
				if (arstat_i == SCP_AR_L_OTHER){
					pdata->setScpArstat(i, SCP_AR_N_OTHER);
					ret = true;
				}
			}
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// ロゴなし推測後のロゴ情報を使った推測追加
//---------------------------------------------------------------------
void JlsAutoReform::detectCMAssistLogo(int nsc_det1st, int nsc_detend) {
	int lvlogo = pdata->getLevelUseLogo();
	if (lvlogo <= CONFIG_LOGO_LEVEL_UNUSE_ALL) {		// ロゴを使用しない場合
		return;
	}
	Nsc nsc_cur = nsc_det1st;
	while (nsc_cur >= 0 && (nsc_detend < 0 || nsc_cur < nsc_detend)) {
		Nsc nsc_last = nsc_cur;
		nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		if (nsc_cur > 0 && (nsc_detend < 0 || nsc_cur < nsc_detend)) {
			//--- 時間情報取得 ---
			Msec msec_st = pdata->getMsecScp(nsc_last);
			Msec msec_ed = pdata->getMsecScp(nsc_cur);
			Sec  sec_full = pdata->cnv.getSecFromMsec(msec_ed - msec_st);
			Sec  sec_logo = pdata->getSecLogoComponentFromLogo(msec_st, msec_ed);
			ScpArType arstat_cur = pdata->getScpArstat(nsc_cur);
			bool flag_logoon_cur = jlsd::isScpArTypeLogo(arstat_cur);
			//--- ロゴ情報の追加 ---
			if (lvlogo >= CONFIG_LOGO_LEVEL_UNUSE_EX1 && flag_logoon_cur == false) {
				if ((sec_full - 1 <= sec_logo) ||
					(sec_full >= 10 && sec_full <= sec_logo * 2)) {
					if ((sec_full + 1) % 15 <= 2) {
						pdata->setScpArstat(nsc_cur, SCP_AR_L_UNIT);
					}
					else {
						pdata->setScpArstat(nsc_cur, SCP_AR_L_OTHER);
					}
				}
			}
			//--- ロゴ情報の削除 ---
			if (lvlogo >= CONFIG_LOGO_LEVEL_UNUSE_EX2 && flag_logoon_cur == true) {
				if ((sec_logo <= 1) ||
					(sec_full >= 15 && sec_full >= sec_logo * 4)) {
					if ((sec_full + 1) % 15 <= 2) {
						pdata->setScpArstat(nsc_cur, SCP_AR_N_UNIT);
					}
					else {
						pdata->setScpArstat(nsc_cur, SCP_AR_N_OTHER);
					}
				}
			}
		}
	}
}


//=====================================================================
// 対象範囲の構成確定処理
//=====================================================================

//---------------------------------------------------------------------
// ロゴ区間を内部で推測構成を取得しながら設定
// 入力：
//    nscbounds: ロゴ区間
//    cm_inter:  内部の15秒単位CM検出（false=しない  true=CM検出する）
//---------------------------------------------------------------------
bool JlsAutoReform::mkRangeInterLogo(RangeNsc nscbounds, bool cm_inter){
	//--- 位置取得 ---
	RangeMsec bounds = getRangeMsec(nscbounds);

	//--- 範囲内のスコアと区切りを取得 ---
	RangeFixMsec scope_logo;
	scope_logo.st = bounds.st;
	scope_logo.ed = bounds.ed;
	scope_logo.fixSt = true;
	scope_logo.fixEd = true;
	if (bounds.st < 0){
		scope_logo.fixSt = false;
	}
	if (bounds.ed < 0){
		scope_logo.fixEd = false;
	}
	//--- 両端固定時の保存 ---
	ScpChapType chap_bakst = SCP_CHAP_NONE;
	ScpChapType chap_baked = SCP_CHAP_NONE;
	RangeNsc rnsc_fix = {-1, -1};
	if (scope_logo.fixSt){
		rnsc_fix.st = pdata->getNscFromMsecAll(scope_logo.st);
		if (rnsc_fix.st > 0){
			chap_bakst = pdata->getScpChap(rnsc_fix.st);
		}
	}
	if (scope_logo.fixEd){
		rnsc_fix.ed = pdata->getNscFromMsecAll(scope_logo.ed);
		if (rnsc_fix.ed > 0){
			chap_baked = pdata->getScpChap(rnsc_fix.ed);
		}
	}
	//--- 実行 ---
	setScore(scope_logo);
	setChap(scope_logo);
	//--- 両端固定時のデータ回復 ---
	if (scope_logo.fixSt && jlsd::isScpChapTypeDecide(chap_bakst)){
		pdata->setScpChap(rnsc_fix.st, chap_bakst);
	}
	if (scope_logo.fixEd && jlsd::isScpChapTypeDecide(chap_baked)){
		pdata->setScpChap(rnsc_fix.ed, chap_baked);
	}

	//---ロゴ設定 ---
	Nsc nsc_to_ar = nscbounds.ed;
	if (nsc_to_ar < 0){
		nsc_to_ar = pdata->sizeDataScp()-1;
	}
	pdata->setScpArstat(nsc_to_ar, SCP_AR_L_OTHER);

	//--- ロゴ区間設定 ---
	bool ret = setInterLogo(nscbounds, cm_inter);
	return ret;
}


//---------------------------------------------------------------------
// 範囲のCM構成区切りを作成
// 入力：
//   fixscope  : CM検索の検索範囲
//   logo1st   : 先頭部分のロゴ状態（false=ロゴなし true=ロゴあり）
//   interfill:  1=CMとCMの間をCMで埋める 2=CMとCMの間をCMで埋める（強制設定）
// 出力：
//   返り値 : 構成追加（false=なし  true=あり）
//   bounds : CMと判断した範囲
//---------------------------------------------------------------------
bool JlsAutoReform::mkRangeCM(RangeMsec &bounds, RangeFixMsec fixscope, bool logo1st, int interfill){
	//--- 不明確部分を含めたロゴ位置(cmscope)取得 ---
	RangeWideMsec cmscope;
	mkRangeCMGetLogoEdge(cmscope.st, fixscope.st, fixscope.fixSt, LOGO_EDGE_FALL);	// ロゴ立ち下がりがCM開始位置
	mkRangeCMGetLogoEdge(cmscope.ed, fixscope.ed, fixscope.fixEd, LOGO_EDGE_RISE);	// ロゴ立ち上がりがCM終了位置
	cmscope.fixSt = fixscope.fixSt;
	cmscope.fixEd = fixscope.fixEd;
	cmscope.logomode = false;
	//--- 固定時のデータ保存 ---
	ScpChapType chap_bakst = SCP_CHAP_NONE;
	ScpChapType chap_baked = SCP_CHAP_NONE;
	{
		Nsc nsc_st = pdata->getNscFromMsecAll(fixscope.st);
		Nsc nsc_ed = pdata->getNscFromMsecAll(fixscope.ed);
		if (nsc_st > 0) chap_bakst = pdata->getScpChap(nsc_st);
		if (nsc_ed > 0) chap_baked = pdata->getScpChap(nsc_ed);
	}

	//--- 範囲内のスコアと区切りを取得 ---
	RangeFixMsec scope_chap;
	scope_chap.st = (cmscope.fixSt == false)? cmscope.st.early : cmscope.st.just;
	scope_chap.ed = (cmscope.fixEd == false)? cmscope.ed.late  : cmscope.ed.just;
	scope_chap.fixSt = cmscope.fixSt;
	scope_chap.fixEd = cmscope.fixEd;
	setScore(scope_chap);
	setChap(scope_chap);

	//--- CM構成区切り作成 ---
	bool logointer = true;								// CM間の中間地点はロゴあり状態
	bool det = setCMForm(bounds, cmscope, logo1st, logointer);

	//--- CM内の補完処理 ---
	if (det){
		RangeNsc nscbounds;
		pdata->getRangeNscFromRangeMsec(nscbounds, bounds);
		setInterMultiCM(nscbounds, interfill);
	}

	//--- 固定時のデータ回復 ---
	if (fixscope.fixSt && isScpChapTypeDecide(chap_bakst) ){
		Nsc nsc_st = pdata->getNscFromMsecAll(fixscope.st);
		pdata->setScpChap(nsc_st, chap_bakst);
	}
	if (fixscope.fixEd && isScpChapTypeDecide(chap_baked) ){
		Nsc nsc_ed = pdata->getNscFromMsecAll(fixscope.ed);
		pdata->setScpChap(nsc_ed, chap_baked);
	}

	return det;
}

//---------------------------------------------------------------------
// 対象位置のロゴ変化地点として可能性範囲を取得
// 入力：
//	 msec_target : 検索するロゴ位置（ミリ秒）
//   flag_fix    : 対象地点が確定位置
//	 edge        : 立ち上がり／立ち下がり
// 出力：
//   返り値: ロゴ存在（false=なし  true=あり）
//   wmsec : ロゴ位置
//---------------------------------------------------------------------
bool JlsAutoReform::mkRangeCMGetLogoEdge(WideMsec &wmsec, int msec_target, bool flag_fix, LogoEdgeType edge){
	bool detect = false;
	Msec msec_sftmrg = pdata->getConfig(ConfigVarType::msecWLogoSftMrg);

	if (msec_target < 0){
		wmsec.just  = -1;
		wmsec.early = -1;
		wmsec.late  = -1;
	}
	else{
		if (flag_fix){
			wmsec.just  = msec_target;
			wmsec.early = msec_target - pdata->msecValSpc;
			wmsec.late  = msec_target + pdata->msecValSpc;
		}
		else{
			//--- 対象位置のロゴ番号を検索 ---
			Nrf nrf = pdata->getNrfLogoFromMsec(msec_target, edge);
			if (nrf >= 0){
				detect = true;
				pdata->getWideMsecLogoNrf(wmsec, nrf);
				wmsec.early -= msec_sftmrg;
				wmsec.late  += msec_sftmrg;
			}
			else{				// 対象位置にロゴがなければ中心位置をセット
				wmsec.just  = msec_target;
				wmsec.early = msec_target;
				wmsec.late  = msec_target;
			}
		}
	}
	return detect;
}



//=====================================================================
// 全体における端部分の推測構成
//=====================================================================

//---------------------------------------------------------------------
// 先頭位置の推測構成を作成
// 出力：
//   返り値: 更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setFirstArea(FormFirstInfo &info_first){
	bool ret = false;
	//--- 結果初期値格納 ---
	info_first.limitHead = AUTOFIRST_LAX;
	info_first.nscTreat = -1;
	if (pdata->sizeDataScp() <= 2) return false;

	//--- 設定値格納 ---
	FormFirstLoc locinfo;
	{
		//--- 先頭優先位置設定を取得 ---
		locinfo.msec1stSel = pdata->getConfig(ConfigVarType::msecPosFirst);
		locinfo.lvPos1stSel = pdata->getConfig(ConfigVarType::priorityPosFirst);
		if (locinfo.lvPos1stSel == 3 && pdata->recHold.msecSelect1st >= 0) {
			locinfo.msec1stSel = pdata->recHold.msecSelect1st;
		}
		locinfo.msec1stZone = pdata->getConfig(ConfigVarType::msecZoneFirst);
		//--- 先頭構成カットする最大先頭位置 ---
		locinfo.msecWcomp1st = pdata->getConfig(ConfigVarType::msecWCompFirst);
		if (locinfo.msecWcomp1st <= 0){
			locinfo.msecWcomp1st = 30 * 1000;			// 初期設定
		}
		//--- 検索終了位置 ---
		locinfo.msecEnd1st = 30 * 1000 + pdata->msecValLap2;
		if (locinfo.msec1stSel > 0){
			locinfo.msecEnd1st += locinfo.msec1stSel;
		}
		//--- 先頭ロゴあり時のカットする最大先頭位置 ---
		locinfo.msecLgCut1st = pdata->getConfig(ConfigVarType::msecLgCutFirst);
	}
	//--- 先頭ロゴと構成確定位置を取得 ---
	locinfo.nscLogo1st = -1;
	locinfo.nscDecide = -1;
	locinfo.msecLogo1st = -1;
	locinfo.msecDecide = -1;
	locinfo.rnscLogo1st = {-1, -1};
	bool unuse_lvlogo = pdata->isUnuseLevelLogo();
	if (unuse_lvlogo == false){						// ロゴあり時
		//--- ロゴ位置取得 ---
		Nrf nrf_1st  = pdata->getNrfNextLogo(-1, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		//--- 立ち下がりが最初の無効期間内だった場合は次のエッジにする ---
		Nrf nrf_fall = pdata->getNrfNextLogo(nrf_1st, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
		Msec msec_tmp = pdata->getMsecLogoNrf(nrf_fall);
		if (msec_tmp < locinfo.msec1stZone && locinfo.msec1stZone > 0){
			nrf_1st  = pdata->getNrfNextLogo(nrf_fall, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		}
		//--- 検索用ロゴ範囲取得 ---
		if (nrf_1st >= 0){
			WideMsec wmsec_logo1st;
			pdata->getWideMsecLogoNrf(wmsec_logo1st, nrf_1st);
			if (wmsec_logo1st.early >= pdata->msecValNear1){	// 先頭からロゴ開始が離れていたら検索
				wmsec_logo1st.early -= pdata->msecValSpc;
				wmsec_logo1st.late  += pdata->msecValSpc;
				locinfo.nscLogo1st = pdata->getNscFromWideMsecByChap(wmsec_logo1st, SCP_CHAP_NONE);
				if (locinfo.nscLogo1st > 0){
					//--- 対象ロゴ立ち上がり可能性ある範囲を取得 ---
					for(int i=1; i<pdata->sizeDataScp()-1; i++){
						Msec msec_tmp = pdata->getMsecScp(i);
						if ((msec_tmp >= wmsec_logo1st.early && msec_tmp <= wmsec_logo1st.late) ||
							i == locinfo.nscLogo1st){
							locinfo.rnscLogo1st.ed = i;
							if (locinfo.rnscLogo1st.st < 0){
								locinfo.rnscLogo1st.st = i;
							}
						}
					}
				}
			}
		}
		Msec msec_logo1st_org = pdata->getMsecLogoNrf(nrf_1st);
		if (locinfo.nscLogo1st >= 0){
			locinfo.msecLogo1st = pdata->getMsecScp(locinfo.nscLogo1st);
			//--- 0地点の方が近い場合の処理 ---
			if (msec_logo1st_org < locinfo.msecLogo1st - msec_logo1st_org){
				if (locinfo.msecLgCut1st >= 0 && locinfo.msecLgCut1st < locinfo.msecLogo1st){
					locinfo.nscLogo1st = -1;
					locinfo.msecLogo1st = 0;
				}
			}
		}
		else{
			locinfo.msecLogo1st = msec_logo1st_org;
		}
		//--- 先頭からロゴだった場合はなかったことに ---
		if (locinfo.msecLogo1st < pdata->msecValNear2){	// 先頭からロゴ
			if (locinfo.msecLgCut1st < 0){				// 先頭からのロゴを特に意識しない場合
				if (locinfo.lvPos1stSel > 0){
					locinfo.nscLogo1st = -1;
					locinfo.msecLogo1st = -1;
				}
			}
		}
		//--- 構成確定位置取得。検索位置外の時はなかったことに ---
		locinfo.nscDecide = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
		if (locinfo.nscDecide > 0){
			locinfo.msecDecide  = pdata->getMsecScp(locinfo.nscDecide);
			if (locinfo.msecDecide > locinfo.msecEnd1st){
				locinfo.nscDecide = -1;
				locinfo.msecDecide = -1;
			}
		}
	}
	//--- 先頭付近の構成を確定 ---
	{
		//--- 優先位置0設定時の処理 ---
		if (locinfo.msec1stSel == 0){
			NrfCurrent logopt = {};
			if ( pdata->getNrfptNext(logopt, LOGO_SELECT_VALID) ){
				WideMsec wmsec_1stlogo;
				pdata->getWideMsecLogoNrf(wmsec_1stlogo, logopt.nrfRise);
				if (wmsec_1stlogo.early <= pdata->msecValNear2){
					info_first.nscTreat = -1;
					info_first.limitHead = AUTOFIRST_NOCUT;		// 先頭カットなし
				}
			}
		}
		//--- 先頭位置を検索および設定 ---
		if (info_first.limitHead != AUTOFIRST_NOCUT){		// 先頭カットなしは除く
			bool update = setFirstAreaUpdate(info_first, locinfo);
			if (update) ret = true;
			//--- 取得した先頭位置が未確定ロゴ位置であれば確定して再検索 ---
			if (update && locinfo.msecLogo1st > pdata->msecValLap2){
				int msec_head = pdata->getMsecScp(info_first.nscTreat);
				if (msec_head + pdata->msecValLap2 < locinfo.msecDecide || locinfo.nscDecide < 0){
					setFirstAreaUpdate(info_first, locinfo);
				}
			}
		}
	}
	//--- 先頭位置のCM化 ---
	if (info_first.limitHead != AUTOFIRST_NOCUT){		// 先頭カットなしは除く
		Nsc nsc_head = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
		Msec msec_head = 0;
		if (nsc_head > 0){
			msec_head = pdata->getMsecScp(nsc_head);
			if (msec_head <= locinfo.msecWcomp1st){
				//--- ロゴ位置による制限を追加 ---
				bool before_logo = false;
				if (unuse_lvlogo){			// ロゴなし時
					before_logo = true;
				}
				else{						// ロゴあり時
					if (msec_head <= locinfo.msecLogo1st && locinfo.msecLogo1st >= 0){
						before_logo = true;
					}
				}
				if (before_logo ||
					msec_head <= locinfo.msecLgCut1st || locinfo.msecLgCut1st < 0){
					ret = true;
					info_first.nscTreat = nsc_head;
					pdata->setScpArstat(nsc_head, SCP_AR_N_OTHER);
				}
			}
		}
		//--- 指定位置までCM化 ---
		Msec msec_nologo = locinfo.msecLogo1st;
		if (msec_nologo < locinfo.msec1stZone && locinfo.msec1stZone > 0){
			msec_nologo = locinfo.msec1stZone;
		}
		Msec msec_last = 0;
		while(nsc_head > 0 && msec_head <= msec_nologo){
			ScpArType arstat_head = pdata->getScpArstat(nsc_head);
			if (arstat_head != SCP_AR_N_UNIT && arstat_head != SCP_AR_N_OTHER){
				ret = true;
				info_first.nscTreat = nsc_head;
				int sec_dif = pdata->cnv.getSecFromMsec(msec_head - msec_last);
				if (sec_dif > 0 && sec_dif % 15 == 0){
					pdata->setScpArstat(nsc_head, SCP_AR_N_UNIT);
				}
				else{
					pdata->setScpArstat(nsc_head, SCP_AR_N_OTHER);
				}
			}
			msec_last = msec_head;
			nsc_head = pdata->getNscNextScpChap(nsc_head, SCP_CHAP_DECIDE);
			msec_head = pdata->getMsecScp(nsc_head);
		}
	}
	//--- ロゴありで変更した時、先頭位置から１構成だけ追加処理 ---
	if (ret && pdata->isUnuseLevelLogo() == false){
		Nsc nsc_head = info_first.nscTreat;
		Nsc nsc_next = pdata->getNscNextScpChap(nsc_head, SCP_CHAP_DECIDE);
		if (nsc_head > 0 && nsc_next > 0){
			bool enable_short = true;
			addCommonComponentOne(nsc_head, nsc_next, enable_short);
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// 先頭候補位置を取得・更新
// 出力：
//   返り値: 先頭位置の追加（false=なし、true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setFirstAreaUpdate(FormFirstInfo &info_first, FormFirstLoc locinfo){
	int num_scpos = pdata->sizeDataScp();
	AutoFirstType limit_cand = AUTOFIRST_LAX;
	Nsc  nsc_cand = -1;
	int  npoint_cand  = 0;
	Msec msec_hold1st = -1;
	int  lvchap_max = 1;			// 対象同一地点で設定する値
	for(int i=1; i < num_scpos-1; i++){
		Msec msec_i = pdata->getMsecScp(i);
		ScpChapType chap_i = pdata->getScpChap(i);
		//--- 検索位置を超えたら終了 ---
		if (msec_i > locinfo.msecEnd1st){
			break;
		}
		if ((i > locinfo.rnscLogo1st.ed || locinfo.rnscLogo1st.ed < 0) &&	// ロゴ範囲超え
			msec_i > locinfo.msecLogo1st  && locinfo.msecLogo1st  >= 0 &&	// ロゴ位置超え
			msec_i > locinfo.msecLgCut1st && locinfo.msecLgCut1st >= 0){	// 検索位置超え
			break;
		}
		//--- 確定位置からの距離取得 ---
		int lvchap = 0;
		if (locinfo.nscDecide > 0 || locinfo.nscLogo1st > 0){
			CalcDifInfo calc1;
			if (i == locinfo.nscDecide || (i >= locinfo.rnscLogo1st.st && i <= locinfo.rnscLogo1st.ed)){
				lvchap = lvchap_max;
			}
			else if (locinfo.nscDecide > 0){
				lvchap = calcDifSelect(calc1, msec_i, locinfo.msecDecide);
				//--- 確定構成から15秒未満は正確に秒数単位でなければ候補から外す ---
				if (calc1.sec < 15 && (calc1.gap > pdata->msecValExact)){
					lvchap = -1;		// 検索対象外
				}
				if (calc1.sec < 3){
					if (locinfo.msec1stSel < 0){	// 先頭優先位置設定なし
						lvchap = -1;				// 検索対象外
					}
					else if (abs(msec_i - locinfo.msec1stSel) > abs(locinfo.msecDecide - locinfo.msec1stSel)){
						lvchap = -1;				// 検索対象外
					}
				}
			}
			else if (locinfo.nscLogo1st > 0){
				if (i <= locinfo.nscLogo1st || (locinfo.msec1stSel >= 0 && locinfo.lvPos1stSel >= 1)){
					// ロゴ手前またはロゴ以降も許可時
					lvchap = 0;
					int lvchap_tmp;
					for(int k=locinfo.rnscLogo1st.st; k<=locinfo.rnscLogo1st.ed; k++){
						lvchap_tmp = calcDifSelect(calc1, msec_i, locinfo.msecLogo1st);
						//--- ロゴ地点から15秒未満は正確に秒数単位でなければ候補から外す ---
						if (calc1.sec < 15 && (calc1.gap > pdata->msecValExact)){
							//--- 先頭優先位置設定から離れている場合か設定ない場合のみ ---
							if (abs(msec_i - locinfo.msec1stSel) > 3000 || locinfo.msec1stSel < 0){
								lvchap_tmp = 0;
							}
						}
						if (lvchap_tmp > 0){
							lvchap = 1;
							
						}
					}
				}
			}
			if (lvchap > lvchap_max){
				if (i < locinfo.nscLogo1st || i < locinfo.nscDecide){		// 対象同一地点より前
					lvchap_max = lvchap;						// 対象同一地点の値を変更
				}
				else{
					lvchap = lvchap_max;					// 対象同一地点の値に制限
				}
			}
		}
		if (lvchap >= 0){
			//--- 先頭保持 ---
			if (msec_hold1st < 0){			// 最初の構成位置を基準
				msec_hold1st = msec_i;
			}
			//--- 位置による優先度 ---
			int npoint_i = 0;
			if (locinfo.msec1stSel >= 0){			// 先頭優先位置設定あり
				int msec_dif = abs(msec_i - locinfo.msec1stSel);
				if (msec_dif < 3000){				// 3秒以内なら近いほど優先
					npoint_i = 3000 - msec_dif;
				}
				if (npoint_i == 0 && msec_hold1st == msec_i){	// なければ先頭位置を候補に
					npoint_i = 1;
				}
			}
			else if (locinfo.msecLogo1st >= 0 && (locinfo.msecLogo1st + 13000 < locinfo.msecDecide || locinfo.nscDecide < 0)){
				// ロゴが確定位置より前にあればロゴに近いほど優先
				Msec msec_dif = abs(msec_i - locinfo.msecLogo1st);
				npoint_i = 3000 - (msec_dif/100);
				if (npoint_i <= 0 && msec_hold1st == msec_i){	// なければ先頭位置を候補に
					npoint_i = 1;
				}
			}
			else{								// 先頭優先位置設定なし
				Msec msec_dif = abs(msec_i - msec_hold1st);
				if (msec_dif < 3000){				// 3秒以内なら近いほど優先
					npoint_i = 3000 - msec_dif;
				}
			}
			AutoFirstType limit_i = AUTOFIRST_LAX;
			if (npoint_i > 0){
				limit_i = AUTOFIRST_STRICT;			// 位置で確定の場合は先頭カット厳しめ
			}
			if (locinfo.lvPos1stSel >= 2){			// 位置優先度高い場合
				npoint_i *= 10;
			}
			//--- ロゴ位置の優先度 ---
			if (i >= locinfo.rnscLogo1st.st && i <= locinfo.rnscLogo1st.ed &&
				(locinfo.nscLogo1st < locinfo.nscDecide || locinfo.nscDecide < 0) &&
				(abs(locinfo.msecLogo1st - locinfo.msecDecide) > pdata->msecValLap2)){
				if (locinfo.msec1stSel < 0){			// 先頭優先位置設定なし
					npoint_i = 3001;					// 位置優先に設定
				}
				if (i == locinfo.nscLogo1st){			// ロゴ最近似位置を少しだけ優先
					npoint_i += 2;
				}
			}
			//--- 区切りによる優先度追加 ---
			if (i >= locinfo.nscDecide && locinfo.nscDecide >= 0){
			}
			else if (chap_i >= SCP_CHAP_DECIDE || lvchap == 2){
				npoint_i += 15000;
			}
			else if (chap_i >= SCP_CHAP_CPOSIT || lvchap == 1){
				npoint_i += 10000;
			}
			//--- 最適位置の判断 ---
			if ((npoint_cand < npoint_i || nsc_cand < 0) && npoint_i > 0){
				npoint_cand = npoint_i;
				limit_cand = limit_i;
				nsc_cand = i;
			}
		}
	}
	//--- 結果を更新 ---
	bool update = false;
	if (nsc_cand > 0 && (nsc_cand < info_first.nscTreat || info_first.nscTreat < 0)){
		//--- 位置指定とロゴ位置の両方ある時、0地点を使うかどうか比較 ---
		if (locinfo.msec1stSel >= 0 && locinfo.msecLogo1st >= 0){
			Msec msec_cand = pdata->getMsecScp(nsc_cand);
			//--- 対象設定位置が候補より0地点が近く、候補がロゴより後で離れている場合 ---
			if (abs(locinfo.msec1stSel - msec_cand) > locinfo.msec1stSel &&
				msec_cand - locinfo.msecLogo1st > locinfo.msecLogo1st){
				info_first.nscTreat = -1;
				info_first.limitHead = AUTOFIRST_NOCUT;		// 先頭カットなし
				return false;
			}
		}
		//--- 構成追加の更新 ---
		update = true;
		pdata->setScpChap(nsc_cand, SCP_CHAP_DFIX);
		pdata->setScpArstat(nsc_cand, SCP_AR_L_OTHER);	// 念のため追加
		info_first.limitHead = limit_cand;
		info_first.nscTreat = nsc_cand;
	}

	return update;
}


//---------------------------------------------------------------------
// 最終位置の推測構成を作成
// 出力：
//   返り値： 更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setFinalArea(){
	int num_scpos = pdata->sizeDataScp();
	bool ret = false;
	//--- 最後優先位置設定を取得 ---
	Nsc  nsc_total_last = num_scpos - 1;
	Msec msec_total_last  = pdata->getMsecScp(nsc_total_last);
	Msec msec_var_lastzone = pdata->getConfig(ConfigVarType::msecZoneLast);
	Msec msec_lastzone = -1;
	if (msec_var_lastzone >= 0){
		msec_lastzone = msec_total_last - msec_var_lastzone;
	}
	//--- 優先位置にない時カットする最大最後秒数 ---
	Msec msec_wcomp_lastloc = msec_total_last - pdata->getConfig(ConfigVarType::msecWCompLast);
	if (msec_wcomp_lastloc >= msec_total_last){
		msec_wcomp_lastloc = msec_total_last - 30 * 1000;			// 初期設定
	}

	//--- 最終構成未設定ならロゴありにセット ---
	if (pdata->getScpArstat(nsc_total_last) == SCP_AR_UNKNOWN){
			pdata->setScpArstat(nsc_total_last, SCP_AR_L_OTHER);
	}

	//--- 最終ロゴ情報取得 ---
	Nsc  nsc_logolast = -1;
	Msec msec_logolast = -1;
	int  lvlogo = pdata->getLevelUseLogo();
	if (lvlogo >= CONFIG_LOGO_LEVEL_USE_LOW){
		//--- ロゴ位置取得 ---
		Nlg nlg_last = pdata->sizeDataLogo()-1;
		Nrf nrf_last = jlsd::nrfFromNlg(nlg_last, LOGO_EDGE_FALL);
		nrf_last = pdata->getNrfPrevLogo(nrf_last+1, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
		//--- 立ち上がりが最後の無効期間内だった場合は次のエッジにする ---
		Nrf nrf_rise = pdata->getNrfPrevLogo(nrf_last, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		Msec msec_tmp = pdata->getMsecScp(nrf_rise);
		if (msec_tmp > msec_lastzone && msec_lastzone > 0){
			nrf_last  = pdata->getNrfPrevLogo(nrf_rise, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
		}
		nsc_logolast = pdata->getNscFromNrf(nrf_last, 3000, SCP_CHAP_NONE);
		if (nsc_logolast > 0){
			msec_logolast = pdata->getMsecScp(nsc_logolast);
		}
		else{
			msec_logolast = pdata->getMsecLogoNrf(nrf_last);
		}
	}

	//--- 最後構成の内部分割処理 ---
	if (msec_var_lastzone >= 0){			// -1設定時は処理しない
		ScpArType arstat_last = pdata->getScpArstat(nsc_total_last);
		bool flag_last_logo = jlsd::isScpArTypeLogo(arstat_last);
		Nsc  nsc_last_decide = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_DECIDE);
		Msec msec_last_decide = pdata->getMsecScp(nsc_last_decide);
		if (nsc_last_decide > 0 && msec_last_decide < msec_lastzone){
			RangeNsc rnsc_last = {nsc_last_decide, -1};
			setInterpolar(rnsc_last, flag_last_logo);
		}
	}
	//--- 最後付近の15秒単位構成追加 ---
	{
		ScpArType arstat_last = pdata->getScpArstat(nsc_total_last);
		bool flag_last_logo = jlsd::isScpArTypeLogo(arstat_last);
		Nsc nsc_last_decide = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_DECIDE);
		Nsc nsc_last_cdet   = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_CDET);
		//--- 最後付近の構成を確認 ---
		if (nsc_last_cdet > 0){
			Msec msec_last_decide = pdata->getMsecScp(nsc_last_decide);
			Msec msec_last_cdet   = pdata->getMsecScp(nsc_last_cdet);
			//--- 確定位置の後に15秒単位位置がある場合 ---
			if ((nsc_last_decide < nsc_last_cdet) && (nsc_last_decide > 0)){
				Msec msec_dif = msec_last_cdet - msec_last_decide;
				CalcModInfo calcmod;
				int modtype = calcDifMod0515(calcmod, msec_dif);
				if (modtype >= 2){
					ret = true;
					pdata->setScpChap(nsc_last_cdet, SCP_CHAP_DFIX);
					if (flag_last_logo) {
						pdata->setScpArstat(nsc_last_cdet, SCP_AR_L_UNIT);
					}
					else{
						pdata->setScpArstat(nsc_last_cdet, SCP_AR_N_UNIT);
					}
					RangeNsc rnsc_last = {nsc_last_decide, nsc_last_cdet};
					setInterpolar(rnsc_last, flag_last_logo);
				}
				else{
					//--- 15秒単位位置の最後２つの構成 ---
					Nsc nsc_last2_cdet = pdata->getNscPrevScpChap(nsc_last_cdet, SCP_CHAP_CDET);
					if (nsc_last_decide < nsc_last2_cdet){
						int msec_last2_cdet = pdata->getMsecScp(nsc_last2_cdet);
						if (msec_last2_cdet - msec_last_decide > pdata->msecValLap2){
							msec_dif = msec_last_cdet - msec_last2_cdet;
							modtype = calcDifMod0515(calcmod, msec_dif);
							if (modtype >= 2) {
								ret = true;
								pdata->setScpChap(nsc_last2_cdet, SCP_CHAP_DFIX);
								pdata->setScpChap(nsc_last_cdet, SCP_CHAP_DFIX);
								if (flag_last_logo) {
									pdata->setScpArstat(nsc_last2_cdet, SCP_AR_L_OTHER);
									pdata->setScpArstat(nsc_last_cdet, SCP_AR_L_UNIT);
								}
								else {
									pdata->setScpArstat(nsc_last2_cdet, SCP_AR_N_OTHER);
									pdata->setScpArstat(nsc_last_cdet, SCP_AR_N_UNIT);
								}
							}
						}
					}
				}
			}
		}
	}
	//--- 最後の構成を追加確認 ---
	if (msec_var_lastzone >= 0){			// -1設定時は処理しない
		Nsc nsc_last_decide = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_DECIDE);
		Nsc nsc_last_cand   = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_NONE);
		if (nsc_last_decide < nsc_last_cand && nsc_last_decide > 0){
			Msec msec_last_decide = pdata->getMsecScp(nsc_last_decide);
			Msec msec_last_cand   = pdata->getMsecScp(nsc_last_cand);
			CalcDifInfo calcdif;
			int diftype = calcDifSelect(calcdif, msec_last_decide, msec_last_cand);
			if (diftype > 0 && calcdif.gap <= pdata->msecValExact){
			}
			else{		// 対象外なら１つ前を確認
				Nsc nsc_tmp   = pdata->getNscPrevScpChap(nsc_last_cand, SCP_CHAP_NONE);
				if (nsc_last_decide < nsc_tmp){
					nsc_last_cand  = nsc_tmp;
					msec_last_cand = pdata->getMsecScp(nsc_last_cand);
					diftype = calcDifSelect(calcdif, msec_last_decide, msec_last_cand);
				}
			}
			if (diftype > 0 && calcdif.gap <= pdata->msecValExact){
				if (msec_last_cand >= msec_wcomp_lastloc && msec_wcomp_lastloc > 0 &&
					msec_last_decide < msec_lastzone){
					ScpArType arstat_last = pdata->getScpArstat(nsc_total_last);
					bool flag_last_logo = jlsd::isScpArTypeLogo(arstat_last);
					ret = true;
					pdata->setScpChap(nsc_last_cand, SCP_CHAP_DFIX);
					if (flag_last_logo) {
						pdata->setScpArstat(nsc_last_cand, SCP_AR_L_OTHER);
					}
					else{
						pdata->setScpArstat(nsc_last_cand, SCP_AR_N_OTHER);
					}
					RangeNsc rnsc_last = {nsc_last_decide, nsc_last_cand};
					setInterpolar(rnsc_last, flag_last_logo);
				}
			}
		}
	}

	//--- 指定箇所以降はロゴなし ---
	if (msec_lastzone > 0 || msec_logolast > 0){
		Nsc  nsc_last_start = nsc_total_last;
		Msec msec_endtarget = msec_lastzone;
		if ((msec_lastzone > msec_logolast && msec_logolast > 0) || msec_lastzone <= 0){
			msec_endtarget = msec_logolast;
		}
		bool checkend = false;
		//--- 指定箇所に対応する位置の構成検出 ---
		while(checkend == false && nsc_last_start > 0){
			Nsc  nsc_tmp  = pdata->getNscPrevScpChap(nsc_last_start, SCP_CHAP_DECIDE);
			Msec msec_tmp = pdata->getMsecScp(nsc_tmp);
			if (msec_tmp < msec_endtarget || nsc_tmp < 0){
				checkend = true;
			}
			else{
				nsc_last_start = nsc_tmp;
			}
		}
		//--- 指定箇所以降はロゴなしにする ---
		if (nsc_last_start > 0 && nsc_last_start < nsc_total_last){
			//--- 直前のロゴ情報 ---
			Nsc nsc_cur = nsc_last_start;
			do{
				//--- 次の構成検出（終了時は最後の場所に設定）---
				nsc_cur = pdata->getNscNextScpChap(nsc_last_start, SCP_CHAP_DECIDE);
				if (nsc_cur < 0){
					nsc_cur = nsc_total_last;
				}
				ScpArType arstat_cur = pdata->getScpArstat(nsc_cur);
				bool flag_logo_cur = jlsd::isScpArTypeLogo(arstat_cur);
				if (flag_logo_cur){
					ret = true;
					if (arstat_cur == SCP_AR_L_OTHER){
						pdata->setScpArstat(nsc_cur, SCP_AR_N_OTHER);
					}
					else if (arstat_cur == SCP_AR_L_UNIT){
						pdata->setScpArstat(nsc_cur, SCP_AR_N_UNIT);
					}
				}
				nsc_last_start = nsc_cur;
			} while(nsc_cur > 0 && nsc_cur < nsc_total_last);
		}
	}

	//--- 最終構成のロゴなし変更判断 ---
	if (msec_wcomp_lastloc > 0){
		Nsc  nsc_prev = pdata->getNscPrevScpChap(nsc_total_last, SCP_CHAP_DECIDE);
		Msec msec_prev = pdata->getMsecScp(nsc_prev);
		if (msec_prev > msec_wcomp_lastloc){
			ret = true;
			pdata->setScpArstat(nsc_total_last, SCP_AR_N_OTHER);
		}
	}

	return ret;
}



//=====================================================================
// ２点間の構成設定
//=====================================================================

//---------------------------------------------------------------------
// 確定２点間をロゴ設定 - 内部のCM構成も検出
// 入力：
//    nscbounds: 確定２点
//    cm_inter:  内部の15秒単位CM検出（false=しない  true=CM検出する）
// 出力：
//   返り値: 構成追加（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterLogo(RangeNsc nscbounds, bool cm_inter){
	//--- ロゴ構成区間内の15秒構成をCM化する処理 ---
	bool rev_del_mid  = (pdata->getConfigAction(ConfigActType::LogoDelMid))? true : false;
	//--- 15秒単位の構成検出 ---
	bool det = false;
	if (rev_del_mid && cm_inter){		// 内部のCMを検出
		//--- 範囲設定 ---
		RangeMsec bounds = getRangeMsec(nscbounds);
		//--- CM構成位置検出 ---
		RangeMsec rmsec_dmy;
		RangeWideMsec cmscope;
		cmscope.st = {bounds.st, bounds.st, bounds.st};
		cmscope.ed = {bounds.ed, bounds.ed, bounds.ed};
		cmscope.fixSt = true;
		cmscope.fixEd = true;
		cmscope.logomode = true;
		bool logo1st = true;
		bool logointer = true;
		det = setCMForm(rmsec_dmy, cmscope, logo1st, logointer);
	}
	//--- ロゴ区間設定 ---
	if (det == false){		// 間にCMがなければ全体の構成推測
		bool logomode = true;
		det = setInterpolar(nscbounds, logomode);
	}
	else{					// 間にCMがあればCMを除いて構成推測
		//--- 範囲設定 ---
		int num_scpos = pdata->sizeDataScp();
		RangeNsc nscope = nscbounds;
		if (nscope.st < 0) nscope.st = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
		if (nscope.ed < 0) nscope.ed = pdata->getNscPrevScpChap(num_scpos-1, SCP_CHAP_DECIDE);
		//--- CM構成検出 ---
		RangeNsc cmterm = {-1, -1};	// CM化開始終了位置
		int count    = 0;			// CM化構成数カウント
		Nsc nsc_fin = nscope.st;	// 補完完了している位置
		Nsc nsc_cur = nscope.st;
		while(nsc_cur >= nscope.st && nsc_cur < nscope.ed && nsc_cur >= 0){
			Nsc nsc_last = nsc_cur;
			nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
			if (nsc_cur >= nscope.st && nsc_cur <= nscope.ed){
				bool det15s = (pdata->getScpArstat(nsc_cur) == SCP_AR_L_UNIT)? true : false;
				Msec msec_last = pdata->getMsecScp(nsc_last);
				Msec msec_cur  = pdata->getMsecScp(nsc_cur);
				CalcDifInfo calc1;
				int type1 = calcDifSelect(calc1, msec_last, msec_cur);
				//--- 30秒までに制限 ---
				if (calc1.sec > 31){
					det15s = false;
				}
				//--- 短期間ロゴ区間は（端以外は）CM扱いとする ---
				bool shortcm = (nsc_fin == nscope.st)? false : true;
				//--- CM構成を検出時 ---
				if (det15s && type1 == 2){
					if (count == 0) cmterm.st = nsc_last;
					cmterm.ed = nsc_cur;
					count ++;
				}
				//--- CM以外の構成を検出時 ---
				else{
					if (count > 0){
						//--- 途中位置だった場合 ---
						if (cmterm.st > nscope.st){
							if (rev_del_mid && count >= 2){
								int msec_st = pdata->getMsecScp(cmterm.st);
								int msec_ed = pdata->getMsecScp(cmterm.ed);
								CalcDifInfo calc2;
								calcDifSelect(calc2, msec_st, msec_ed);
								if (calc2.sec % 30 == 0){		// 30秒単位の時に認識
									setInterLogoUpdate(nsc_fin, cmterm, shortcm);
									det = true;
								}
							}
						}
						nsc_fin = cmterm.ed;
						count = 0;
					}
				}
				//--- 最終位置の処理 ---
				if (nsc_cur == nscope.ed){
					cmterm = {nsc_cur, nsc_cur};		// 期間なしで最終位置をセット
					bool det_tmp = setInterLogoUpdate(nsc_fin, cmterm, shortcm);
					if (det_tmp) det = true;
				}
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// 確定２点間をロゴ設定の更新
// 入力：
//   nsc_fin:   前回完了位置
//   cmterm:    CM化開始終了位置
//   shortcm:   true=CM前の短期間構成もCM化
// 出力：
//   返り値:    構成追加（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterLogoUpdate(Nsc nsc_fin, RangeNsc cmterm, bool shortcm){
	bool det = false;
	//--- CM化開始位置より前のロゴあり部分内部補完 ---
	if (nsc_fin >= 0 && nsc_fin < cmterm.st){
		//--- 内部の確定箇所は候補状態に戻す ---
		Nsc nsc_cur = nsc_fin;
		nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		while(nsc_cur > 0 && nsc_cur < cmterm.st){
			pdata->setScpChap(nsc_cur, SCP_CHAP_CDET);
			nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		}
		//--- 短期間ロゴ部分の処理 ---
		if (shortcm){
			Msec msec_fin = pdata->getMsecScp(nsc_fin);
			Msec msec_st  = pdata->getMsecScp(cmterm.st);
			if (abs(msec_st - msec_fin) < 15*1000 - pdata->pdata->msecValLap2){
				pdata->setScpArstat(cmterm.st, SCP_AR_N_OTHER);
			}
		}
		//--- 補完処理 ---
		bool logomode = true;
		RangeNsc rnsc_target = {nsc_fin, cmterm.st};
		det = setInterpolar(rnsc_target, logomode);			// 内部構成推測
	}
	//--- CM化処理 ---
	if (cmterm.st >= 0 && cmterm.st < cmterm.ed){
		det = true;
		Nsc nsc_cur = cmterm.st;
		nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		while(nsc_cur > 0 && nsc_cur <= cmterm.ed){
			pdata->setScpArstat(nsc_cur, SCP_AR_N_UNIT);
			nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		}
	}
	return det;
}


//---------------------------------------------------------------------
// カット期間内の確定２点間に秒数単位となる構成があれば追加（＋内部構成の強制CM化処理）
// 入力：
//   nscbounds:  開始終了構成番号
//   interfill:  1=CMとCMの間をCMで埋める 2=CMとCMの間をCMで埋める（強制設定）
// 出力：
//   返り値:     更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterMultiCM(RangeNsc nscbounds, int interfill){
	//--- ロゴなし区間のCM外構成について設定取得 ---
	Msec msec_rev_logo = pdata->getConfig(ConfigVarType::msecWLogoRevMin);
	int rev_logo = pdata->getConfigAction(ConfigActType::LogoUCRemain);
	bool rev_add = (rev_logo > 0)? true : false;

	int upcnt = 0;
	Nsc nsc_cur = nscbounds.st;
	while(nsc_cur >= nscbounds.st && nsc_cur <= nscbounds.ed && nsc_cur >= 0){
		Nsc nsc_last = nsc_cur;
		nsc_cur = pdata->getNscNextScpChap(nsc_cur, SCP_CHAP_DECIDE);
		if (nsc_cur >= nscbounds.st && nsc_cur <= nscbounds.ed){
			//--- ロゴ構成は強制的にロゴなしに変更する処理 ---
			bool logomode = false;						// ロゴ外とする設定
			bool check_inter = (interfill > 0)? true : false;	// CMで埋める場合は無条件で内部構成チェック
			ScpArType arstat_cur = pdata->getScpArstat(nsc_cur);
			if ( jlsd::isScpArTypeLogo(arstat_cur) ){
				logomode = true;
				if (interfill > 0){				// CM化で埋める場合
					Msec msec_last = pdata->getMsecScp(nsc_last);
					Msec msec_cur  = pdata->getMsecScp(nsc_cur);
					Msec msec_dif  = msec_cur - msec_last;
					if (rev_add == false || msec_dif <= msec_rev_logo || interfill == 2){
						pdata->setScpArstat(nsc_cur, SCP_AR_N_OTHER);
						logomode = false;
					}
				}
			}
			else{						// CMの時は無条件で内部構成チェック
				check_inter = true;
			}
			if (check_inter){
				RangeNsc rnsc_target = {nsc_last, nsc_cur};
				bool update = setInterpolar(rnsc_target, logomode);
				if (update) upcnt ++;
			}
		}
	}
	bool det = (upcnt > 0)? true : false;
	return det;
}

//---------------------------------------------------------------------
// 確定２点間を設定し秒数単位となる構成があれば追加
// 入力：
//   rnsc_target: 開始終了構成番号
//   logomode:    false:ロゴ外  true:ロゴ内
// 出力：
//   返り値:  更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterpolar(RangeNsc rnsc_target, bool logomode){
	RangeNscMsec target;
	target.nsc = rnsc_target;
	//--- 先頭と最終の指定なしは構成始点から除く ---
	int nside = 3;							// 1:開始側から確認  2:終了側から確認  3:両側から確認
	if (target.nsc.st < 0){
		target.nsc.st = 0;
		nside &= 0x2;
	}
	if (target.nsc.ed < 0){
		target.nsc.ed = pdata->sizeDataScp()-1;
		nside &= 0x1;
	}
	if (nside == 0){						// 確認なしなら終了
		return false;
	}
	//--- 初期化 ---
	for(Nsc i = target.nsc.st + 1; i < target.nsc.ed; i++){
		setInterpolarClearChap(i);					// 内部の構成初期化
	}
	target.msec = getRangeMsec(target.nsc);

	int upcnt = 0;
	Nsc nsc_fix = target.nsc.st;
	//--- 開始位置から終了位置まで検索 ---
	{
		TraceInterpolar trace;
		trace.keepType = 0;
		trace.keepGap = 0;
		trace.nscKeep = -1;
		trace.nscBase = target.nsc.st;
		trace.keep15s = false;
		for(Nsc i = target.nsc.st + 1; i <= target.nsc.ed; i++){
			if ((pdata->getScpStill(i) == false) ||		// 画像変化あること必須
				(i == target.nsc.ed)){					// 最終位置
				bool update = setInterpolarDetect(trace, i, target, logomode, nside);
				if (update) upcnt ++;
			}
		}
		nsc_fix = trace.nscBase;		// 確定位置を設定
	}
	//--- 終了位置から開始位置（確定箇所あれば確定位置）まで検索 ---
	{
		TraceInterpolar trace;
		trace.keepType = 0;
		trace.keepGap = 0;
		trace.nscKeep = -1;
		trace.nscBase = target.nsc.ed;
		trace.keep15s = false;
		for(Nsc i = target.nsc.ed - 1; i >= nsc_fix; i--){
			if ((pdata->getScpStill(i) == false) ||			// 画像変化あること必須
				(i == nsc_fix)){						// 最終位置
				bool update = setInterpolarDetect(trace, i, target, logomode, nside);
				if (update) upcnt ++;
			}
		}
	}
	//--- 特殊構成の追加処理 ---
	{
		bool update = setInterpolarExtra(target, logomode);
		if (update) upcnt ++;
	}
	bool det = (upcnt > 0)? true : false;
	return det;
}

//---------------------------------------------------------------------
// 区切りが確定していた時は消去
//---------------------------------------------------------------------
void JlsAutoReform::setInterpolarClearChap(Nsc nsc_n){
	int num_scpos = pdata->sizeDataScp();
	ScpChapType chap_n, chap_set;

	if (nsc_n <= 0 || nsc_n >= num_scpos - 1){
		return;
	}
	chap_n = pdata->getScpChap(nsc_n);
	if (chap_n >= SCP_CHAP_DECIDE){
		if (chap_n >= SCP_CHAP_DFIX){
			chap_set = SCP_CHAP_CDET;
		}
		else{
			chap_set = SCP_CHAP_NONE;
		}
		pdata->setScpChap(nsc_n, chap_set);
	}
}

//---------------------------------------------------------------------
// 確定２点間に秒数単位となる構成を検出
// 入力：
//   trace      
//     keepType : 保持構成の種類（0=保持なし 1=候補あり 2=ずれの大きい候補あり）
//     keepGap :  保持構成場所の誤差ミリ秒
//     nscKeep :  保持構成番号
//     nscBase :  確定構成番号
//   nsc_cur:     現在構成番号
//   target:      確定２点シーンチェンジ位置
//   logomode:    0:ロゴ外  1:ロゴ内
//   nside:       1:開始側から確認  2:終了側から確認  3:両側から確認
// 出力：
//   返り値:     更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterpolarDetect(TraceInterpolar &trace, Nsc nsc_cur, RangeNscMsec target, bool logomode, int nside){
	int num_scpos = pdata->sizeDataScp();
	bool update = false;
	Msec msec_val_lap2  = pdata->msecValLap2;
	Msec msec_val_near2 = pdata->msecValNear2;
	Msec msec_cur  = pdata->getMsecScp(nsc_cur);
	Msec msec_base = pdata->getMsecScp(trace.nscBase);
	Msec msec_mid  = (target.msec.st + target.msec.ed) / 2;
	bool flag_last = ((nsc_cur == target.nsc.st) || (nsc_cur == target.nsc.ed))? true : false;

	//--- 確定場所近くでは検出しない ---
	if (nsc_cur != trace.nscBase && nsc_cur != target.nsc.st && nsc_cur != target.nsc.ed){
		if (abs(msec_cur - msec_base) < pdata->msecValLap2 ||
			abs(msec_cur - target.msec.st) < pdata->msecValLap2 ||
			abs(msec_cur - target.msec.ed) < pdata->msecValLap2){
			return false;
		}
	}
	//--- 許容誤差設定 ---
	Msec mgn_cm_detect = (Msec) pdata->getConfig(ConfigVarType::msecMgnCmDetect);
	Msec mgn_cm_divide  = (Msec) pdata->getConfig(ConfigVarType::msecMgnCmDivide);
	Msec msec_mgn = mgn_cm_detect;
	if (mgn_cm_divide < mgn_cm_detect){
		if (abs(target.msec.ed - target.msec.st) <= 15*1000 + pdata->msecValNear2){
			msec_mgn = mgn_cm_divide;		// 15秒以内構成の分割は厳しいパラメータ設定使用
		}
	}
	//--- 許容誤差が広くする位置の確認 ---
	bool flag_curmgn = false;
	{
		CalcDifInfo calc_st;
		CalcDifInfo calc_ed;
		calcDifSelect(calc_st, target.msec.st, msec_cur);
		calcDifSelect(calc_ed, msec_cur, target.msec.ed);
		if ((calc_st.sec % 5) == 0 && (calc_st.gap <= pdata->msecValNear2) &&
			(calc_ed.sec % 5) == 0 && (calc_ed.gap <= pdata->msecValNear2)){
			flag_curmgn = true;		// 両端からともに5秒単位なら許容誤差を広くする
		}
	}
	Msec msec_curmgn = msec_mgn;
	if (flag_curmgn){
		if (msec_curmgn < pdata->msecValNear2){	// 許容範囲が狭い時は広く
			msec_curmgn = pdata->msecValNear2;
		}
	}

	CalcDifInfo calc1;
	int type1 = calcDifSelect(calc1, msec_base, msec_cur);
	if (calc1.gap > msec_curmgn) type1 = 0;
	//--- 前位置確定（番組構成追加） ---
	if (trace.nscKeep > 0){
		Msec msec_keep = pdata->getMsecScp(trace.nscKeep);
		Msec msec_dif_keepcur  = abs(msec_cur - msec_keep);
		Msec msec_dif_keepbase = abs(msec_base - msec_keep);
		if (msec_dif_keepcur > msec_val_lap2 || flag_last){
			CalcDifInfo calc2;
			int type2 = 0;
			if (trace.keepType == 2 || flag_last){
				type2 = calcDifSelect(calc2, msec_keep, msec_cur);
			}
			//--- 判定を緩くする判別 ---
			bool flag_keepsteady = false;
			{
				CalcDifInfo calc_st;
				CalcDifInfo calc_ed;
				calcDifSelect(calc_st, target.msec.st, msec_keep);
				calcDifSelect(calc_ed, msec_keep, target.msec.ed);
				if ((calc_st.sec % 5) == 0 && (calc_st.gap <= pdata->msecValNear2) &&
					(calc_ed.sec % 5) == 0 && (calc_ed.gap <= pdata->msecValNear2)){
					flag_keepsteady = true;		// 両端からともに5秒単位なら許容を広くする
				}
			}
			//--- 余分な無音が多い所は外す処理 ---
			bool flag_mute = false;
			if (abs(trace.nscBase - trace.nscKeep) > 1){
				RangeNsc rnsc;
				if (trace.nscBase > trace.nscKeep){
					rnsc = {trace.nscKeep, trace.nscBase};
				}
				else{
					rnsc = {trace.nscBase, trace.nscKeep};
				}
				int type_mute = checkMuteMany(rnsc);
				if (type_mute >= 2){		// 無音が異常に多い場合は無効化
					flag_mute = true;
				}
				else if (type_mute == 1){	// 無音がある程度存在する場合は判別
					if (msec_dif_keepbase >= 29500){	// 距離が離れていたら無効化
						flag_mute = true;
					}
					else if (trace.keep15s == false){		// 複数15秒単位構成でなければ無効化
						if (flag_keepsteady == false){
							flag_mute = true;
						}
					}
				}
				//--- 無音の判断を無効にする設定の場合 ---
				bool calcel_cntsc = ( pdata->getConfig(ConfigVarType::cancelCntSc) != 0 )? true : false;
				if (calcel_cntsc){
					flag_mute = false;
				}
			}
			if (flag_mute){								// 間に無音が多い場合は外す
			}
			else if (trace.keepType == 2 &&				// 候補のずれがある場合は今回のずれ確認
				(type1 == 0 || type2 == 0 || calc1.sec <= 30)){
			}
			else if (flag_last && type2 == 0 &&
				((msec_dif_keepcur + msec_val_near2 < msec_dif_keepbase) || msec_dif_keepcur < 14500)){
			}
			else{
				pdata->setScpChap(trace.nscKeep, SCP_CHAP_DINT);
				ScpArType arstat_new = (logomode)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
				pdata->setScpArstat(trace.nscKeep, arstat_new);
				update = true;
				trace.nscBase = trace.nscKeep;
				msec_base = pdata->getMsecScp(trace.nscBase);
				trace.nscKeep = -1;
				// baseが変化するので取直し
				type1 = calcDifSelect(calc1, msec_base, msec_cur);
				if (calc1.gap > msec_curmgn) type1 = 0;
			}
		}
	}

	//--- 候補追加に必要な情報を設定 ---
	bool flag_decr = (nsc_cur < trace.nscBase)? true : false;
	Msec msec_st, msec_ed;
	bool enable_nside, enable_mid;
	if (flag_decr == false){				// 前側から後側
		msec_st      = target.msec.st;
		msec_ed      = target.msec.ed;
		enable_nside = ((nside & 0x01)!=0)? true : false;
		enable_mid   = (msec_cur < msec_mid + msec_val_near2)? true : false;
	}
	else{								// 後側から前側
		msec_st      = target.msec.ed;
		msec_ed      = target.msec.st;
		enable_nside = ((nside & 0x02)!=0)? true : false;
		enable_mid   = (msec_cur > msec_mid - msec_val_near2)? true : false;
	}
	bool sec30_cur_st  = (abs(msec_cur - msec_st) <= 30*1000+msec_val_near2)? true : false;
	bool sec30_cur_ed  = (abs(msec_cur - msec_ed) <= 30*1000+msec_val_near2)? true : false;
	bool enable_from   = (abs(msec_cur - target.msec.st) > msec_val_lap2)? true : false;
	bool enable_to     = (abs(msec_cur - target.msec.ed) > msec_val_lap2)? true : false;
	bool enable_base   = (abs(msec_cur - msec_base     ) > msec_val_lap2)? true : false;
	if (target.nsc.st == 0){							// 先頭番号か確認
		nside &= 0x2;
		enable_from = 1;
		enable_base = 1;
	}
	else if (target.nsc.ed == num_scpos-1){	// 最終番号か確認
		nside &= 0x1;
		enable_to = 1;
		enable_base = 1;
	}
	//--- 候補追加 ---
	//--- 両側と確定位置から離れている場合が条件 ---
	if (enable_from && enable_to && enable_base){
		//--- 最初の位置から特定秒数の場合に確定位置からの値を変更 ---
		{
			CalcDifInfo calcsub;
			int typesub = calcDifSelect(calcsub, msec_cur, msec_st);
			if (typesub > 0){
				if (type1 == 0 || (type1 <= typesub && calc1.gap > calcsub.gap)){
					type1 = typesub;
					calc1 = calcsub;
				}
			}
		}
		//--- 候補にするか判別 ---
		if ((enable_mid || trace.nscKeep >= 0) &&		// 検索終了地点まで
			(sec30_cur_st || type1 > 1)){				// 開始側の端に近いか特定秒数の時
			int type_upkeep = 0;
			CalcDifInfo calc2;
			int type2 = calcDifSelect(calc2, msec_cur, msec_ed);
			if (calc2.gap > msec_curmgn) type2 = 0;
			// 終了側にも近く両側検索する場合は両側から確認
			if (sec30_cur_ed && nside == 3 && flag_decr == false){
				if ((trace.keepGap > calc1.gap || trace.nscKeep < 0) && type2 > 0 && type1 > 0){
					type_upkeep = 1;
				}
			}
			// 開始側に候補ある場合
			else if (enable_nside && type1 > 0){
				if (trace.keepGap > calc1.gap || trace.nscKeep < 0){
					type_upkeep = 1;
				}
			}
			// 開始側に候補ある場合（ずれが多少ある場合）
			else if (enable_nside && trace.nscKeep < 0 &&
					 calc1.sec <= 15 && calc1.gap <= msec_val_near2){
				type_upkeep = 2;
			}
			//--- 候補位置が複数の15秒単位構成になるかチェック ---
			bool flag_det15s = false;
			if (type_upkeep > 0){
				int step = (flag_decr)? -1 : +1;
				int cnt_15s = 0;
				for(int i=-4; i<=4; i++){		// 近くのCM単位が複数存在するかで判断
					Msec msec_tmp = msec_cur + (step * i * 15000);
					if (i != 0 &&
						msec_tmp >= target.msec.st - msec_val_near2 &&
						msec_tmp <= target.msec.ed + msec_val_near2){
						ScpChapType chap_cmp = (i < 0)? SCP_CHAP_DECIDE : SCP_CHAP_NONE;
						Nsc nsc_tmp = pdata->getNscFromMsecChap(msec_tmp, msec_val_near2, chap_cmp);
						if (nsc_tmp > 0){
							cnt_15s ++;
						}
					}
				}
				if (cnt_15s >= 2){
					flag_det15s = true;
				}
			}
			//--- 候補を更新 ---
			if (type_upkeep > 0){
				trace.nscKeep = nsc_cur;
				trace.keepGap  = calc1.gap;
				trace.keepType = type_upkeep;
				trace.keep15s  = flag_det15s;
			}
		}
	}
	//--- 出力設定 ---
	return update;
}


//---------------------------------------------------------------------
// 確定２点間の追加設定（CM分割、ロゴ90秒認識）
// 入力：
//   target:     確定２点シーンチェンジ位置
//   logomode:   false:ロゴ外  true:ロゴ内
// 出力：
//   返り値:  更新有無（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setInterpolarExtra(RangeNscMsec target, bool logomode){
	int num_scpos = pdata->sizeDataScp();
	//--- 追加検出するか確認（1=ロゴ90秒検出、2=CM分割、3=CM分割しないCM）
	int type = (logomode)? 1 : 0;
	if (logomode == false && target.nsc.st > 0 && target.nsc.ed < num_scpos-1){
		if (pdata->getScpArstat(target.nsc.ed) == SCP_AR_N_UNIT){
			Msec msec_dif = target.msec.ed - target.msec.st;
			int secdif    = pdata->cnv.getSecFromMsec(msec_dif);
			if (secdif == 15 || secdif == 30){
				type = 2;
			}
			else{
				type = 3;
			}
		}
	}
	bool update = false;
	//--- ロゴ90秒認識 ---
	if (type == 1){
		for(Nsc i=target.nsc.st+1; i<=target.nsc.ed; i++){
			bool flag_search = true;
			int msec_i = pdata->getMsecScp(i);
			//--- 端近くは認識しない ---
			if ((abs(msec_i - target.msec.st) <= 89 * 1000) ||
				(abs(msec_i - target.msec.ed) <= pdata->msecValLap2 && i != target.nsc.ed)){
				flag_search = false;
			}
			//--- 近くに確定箇所がある場合は除く処理 ---
			if (flag_search){
				Nsc nsc_tmp = pdata->getNscFromMsecChap(msec_i, pdata->msecValLap2, SCP_CHAP_DECIDE);
				if (nsc_tmp >= 0 && nsc_tmp != i){
					flag_search = false;
				}
			}
			//--- 90秒構成を取得する処理 ---
			if (flag_search){
				//--- 90秒前地点取得 ---
				Nsc nsc_pts = pdata->getNscFromMsecChap(
								msec_i - 90000, pdata->msecValLap2, SCP_CHAP_DECIDE);
				if (nsc_pts < 0){		// 確定地点がなければそれ以外の地点
					nsc_pts = pdata->getNscFromMsecChap(
								msec_i - 90000, pdata->msecValLap2, SCP_CHAP_NONE);
				}
				//--- 間に無音シーンチェンジが２箇所ある場合は候補にしない ---
				if (nsc_pts > 0 && (i - nsc_pts) > 2){
					nsc_pts = -1;
				}
				//--- 間に無音シーンチェンジが１箇所ある場合は候補とするか判断 ---
				else if (nsc_pts > 0 && (i - nsc_pts) == 2){
					Nsc nsc_tmp = i - 1;
					bool flag_dist = false;
					{
						static const int MSEC_CM90S_SAME = 4500;		// 同一とみなす時間
						Msec msec_pts = pdata->getMsecScp(nsc_pts);
						Msec msec_tmp = pdata->getMsecScp(nsc_tmp);
						if (abs(msec_tmp - msec_pts) < MSEC_CM90S_SAME ||
							abs(msec_tmp - msec_i  ) < MSEC_CM90S_SAME){
							flag_dist = true;
						}
					}
					//--- 間が確定位置だった場合は処理しない ---
					if (jlsd::isScpChapTypeDecide( pdata->getScpChap(nsc_tmp) )){
						nsc_pts = -1;
					}
					//--- 間が前後どちらの無音区間でもない場合処理しない ---
					else if ((pdata->isSmuteSameArea(nsc_tmp, nsc_pts) == false) &&
							 (pdata->isSmuteSameArea(nsc_tmp, i) == false) &&
							 (flag_dist == false)){
						nsc_pts = -1;
					}
				}
				if (nsc_pts > 0){
					Nsc  nsc_pte = i;
					Msec msec_pts = pdata->getMsecScp(nsc_pts);
					Msec msec_pte = msec_i;
					Sec  sec_dif = pdata->cnv.getSecFromMsec(abs(msec_pte - msec_pts));
					if (sec_dif == 90){
						//--- 90秒構成の位置確定処理 ---
						ScpChapType chap_pts = pdata->getScpChap(nsc_pts);		// 開始地点
						ScpChapType chap_pte = pdata->getScpChap(nsc_pte);		// 終了地点
						if (chap_pts < SCP_CHAP_DECIDE){
							pdata->setScpChap(nsc_pts, SCP_CHAP_DINT);
							pdata->setScpArstat(nsc_pts, SCP_AR_L_OTHER);
							update = true;
						}
						if (chap_pte < SCP_CHAP_DECIDE){
							pdata->setScpChap(nsc_pte, SCP_CHAP_DINT);
							pdata->setScpArstat(nsc_pte, SCP_AR_L_OTHER);
							update = true;
						}
						//--- 90秒から5秒または10秒離れた構成も追加 ---
						Nsc nsc_a05 = pdata->getNscFromMsecChap(
										msec_pte+5000,  pdata->msecValNear1, SCP_CHAP_NONE);
						Nsc nsc_a10 = pdata->getNscFromMsecChap(
										msec_pte+10000, pdata->msecValNear1, SCP_CHAP_NONE);
						Nsc nsc_d05 = pdata->getNscFromMsecChap(
										msec_pte+5000, 4500, SCP_CHAP_DECIDE);
						Nsc nsc_d10 = pdata->getNscFromMsecChap(
										msec_pte+10000, 4500, SCP_CHAP_DECIDE);
						if (nsc_d05 <= 0 && nsc_d10 <= 0){		// 近くに既存構成がない場合のみ実行
							if (nsc_a05 > 0 && msec_pte+5000 < target.msec.ed - pdata->msecValLap2){
								pdata->setScpChap(nsc_a05, SCP_CHAP_DINT);
								pdata->setScpArstat(nsc_a05, SCP_AR_L_OTHER);
								update = true;
							}
							else if (nsc_a10 > 0 && msec_pte+10000 < target.msec.ed - pdata->msecValLap2){
								pdata->setScpChap(nsc_a10, SCP_CHAP_DINT);
								pdata->setScpArstat(nsc_a10, SCP_AR_L_OTHER);
								update = true;
							}
						}
					}
				}
			}
		}
	}
	//--- CM分割 ---
	if (type == 2 || type == 3){
		//--- 分割最小値を設定 ---
		bool flag_1st = true;
		for(Nsc i=target.nsc.st+1; i<=target.nsc.ed; i++){
			Msec msec_i = pdata->getMsecScp(i);
			ScpChapType chap_i = pdata->getScpChap(i);
			if (chap_i >= SCP_CHAP_DECIDE){
				//--- 分割許可する範囲 ---
				Msec msec_dif_target = abs(target.msec.ed - target.msec.st);
				Msec msec_minst = (type == 3)? 60*1000 : msec_dif_target / 4;
				Msec msec_mined = (type == 3)? 60*1000 : msec_dif_target / 8;
				if (msec_minst < pdata->msecValLap2){
					msec_minst = pdata->msecValLap2;
				}
				if (msec_mined < pdata->msecValLap2){
					msec_mined = pdata->msecValLap2;
				}
				//--- 分割決定 ---
				bool flag_div = false;
				if ((abs(msec_i - target.msec.st) > msec_minst &&
					 abs(msec_i - target.msec.ed) > msec_mined)){
					flag_div = true;
				}
				if (flag_div){
					//--- 秒単位からの誤差が設定値以上だった場合はCM分割しない ---
					CalcDifInfo calc_st;
					CalcDifInfo calc_ed;
					calcDifSelect(calc_st, msec_i, target.msec.st);
					calcDifSelect(calc_ed, msec_i, target.msec.ed);
					Msec msec_prm_gap = pdata->getConfig(ConfigVarType::msecMgnCmDivide);
					if (calc_st.gap > msec_prm_gap || calc_ed.gap > msec_prm_gap){
						//--- 5秒単位だった場合は少し制限緩くする ---
						if ((calc_st.sec % 5)!=0 || (calc_ed.sec % 5)!=0 ||
							calc_st.gap >= pdata->msecValNear1 || calc_ed.gap >= pdata->msecValNear1){
							flag_div = false;
						}
					}
				}
				if (flag_div || i == target.nsc.ed){
					if (flag_1st && i < target.nsc.ed){
						pdata->setScpArstat(i, SCP_AR_N_BUNIT);
						flag_1st = false;
					}
					else if (flag_1st == false){
						pdata->setScpArstat(i, SCP_AR_N_AUNIT);
					}
				}
				else{
					pdata->setScpChap(i, SCP_CHAP_CPOSQ);
				}
			}
		}
	}
	return update;
}



//=====================================================================
// CM構成を作成
//=====================================================================

//---------------------------------------------------------------------
// CM構成検出
// 入力：
//   cmscope:   CM検索する範囲
//   logo1st:   先頭部分のロゴ状態（false=ロゴなし true=ロゴあり）
//   logointer: 中間部分のロゴ状態（false=ロゴなし true=ロゴあり）
// 出力：
//   返り値: 構成追加判定（0=なし  1=あり）
//   bounds: CM位置と判定した範囲
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoReform::setCMForm(RangeMsec &bounds, RangeWideMsec cmscope, bool logo1st, bool logointer){
	int num_scpos = pdata->sizeDataScp();
	bool logoon_st = logo1st;
	if (num_scpos-1 <= 1){				// 検索最後まで完了時は終了
		return false;
	}
	//--- ロゴ構成区間内の15秒構成をCM化する処理 ---
	int rev_del_edge = pdata->getConfigAction(ConfigActType::LogoDelEdge);

	//--- 15秒範囲設定 ---
	RangeWideMsec  findscope = cmscope;
	findscope.st.early = cmscope.st.just - pdata->msecValLap2;
	findscope.st.late  = cmscope.st.just + pdata->msecValLap2;
	findscope.ed.early = cmscope.ed.just - pdata->msecValLap2;
	findscope.ed.late  = cmscope.ed.just + pdata->msecValLap2;
	if (rev_del_edge == 0){
		if (findscope.st.late < cmscope.st.late){
			findscope.st.late = cmscope.st.late;
		}
		if (findscope.ed.early > cmscope.ed.early){
			findscope.ed.early = cmscope.ed.early;
		}
	}
	if (rev_del_edge == 4){
		if (findscope.st.early > cmscope.st.early){
			findscope.st.early = cmscope.st.early;
		}
		if (findscope.ed.late < cmscope.ed.late){
			findscope.ed.late = cmscope.ed.late;
		}
	}
	if (findscope.st.just < 0){
		findscope.st.early = -1;
		findscope.st.late  = -1;
	}
	if (findscope.ed.just < 0){
		findscope.ed.early = -1;
		findscope.ed.late  = -1;
	}
	//--- 対象候補が不明確ロゴ部分で現在の選択範囲外であれば補正 ---
	if (cmscope.st.just > 0){
		WideMsec wmsec_logo = cmscope.st;
		wmsec_logo.early -= pdata->msecValSpc;
		wmsec_logo.late  += pdata->msecValSpc;
		Nsc nsc_st = pdata->getNscFromWideMsecByChap(wmsec_logo, SCP_CHAP_CDET);
		if (nsc_st < 0){
			nsc_st = pdata->getNscFromWideMsecByChap(wmsec_logo, SCP_CHAP_NONE);
		}
		if (nsc_st > 0){
			Msec msec_st = pdata->getMsecScp(nsc_st);
			if (findscope.st.early > msec_st - pdata->msecValSpc){
				findscope.st.early = msec_st - pdata->msecValSpc;
			}
		}
	}
	if (cmscope.ed.just > 0){
		WideMsec wmsec_logo = cmscope.ed;
		wmsec_logo.early -= pdata->msecValSpc;
		wmsec_logo.late  += pdata->msecValSpc;
		Nsc nsc_ed = pdata->getNscFromWideMsecByChap(wmsec_logo, SCP_CHAP_CDET);
		if (nsc_ed < 0){
			nsc_ed = pdata->getNscFromWideMsecByChap(wmsec_logo, SCP_CHAP_NONE);
		}
		if (nsc_ed > 0){
			Msec msec_ed = pdata->getMsecScp(nsc_ed);
			if (findscope.ed.late < msec_ed + pdata->msecValSpc){
				findscope.ed.late = msec_ed + pdata->msecValSpc;
			}
		}
	}
	//--- 範囲内の候補構成を検索 ---
	bounds = {-1, -1};			// 15秒単位の開始終了位置
	bool det = false;
	{
		Nsc nsc_cur = 1;
		bool over = false;
		while( over == false ){
			int msec_cur = pdata->getMsecScp(nsc_cur);
			ScpChapType chap_cur = pdata->getScpChap(nsc_cur);
			//--- 範囲内の候補位置であれば処理 ---
			if ((msec_cur >= findscope.st.early || findscope.st.early < 0) &&
				(msec_cur <= findscope.ed.late  || findscope.ed.late  < 0) &&
				chap_cur >= SCP_CHAP_CDET){
				//--- 15秒単位を手前方向に探索 ---
				int msec_stpoint;
				bool det_tmp = setCMFormDetect(msec_stpoint, nsc_cur, findscope, logoon_st);
				if (msec_stpoint > 0 && (bounds.st > msec_stpoint || bounds.st < 0)){
					bounds.st = msec_stpoint;
				}
				if (det_tmp || chap_cur >= SCP_CHAP_DFIX){
					bounds.ed = msec_cur;
					if (bounds.st < 0){
						bounds.st = bounds.ed;
					}
				}
				if (det_tmp){
					det = true;
					logoon_st = (logointer)? true : false;
				}
			}
			//--- シーンチェンジ挿入時は番号もシフト ---
			int nsc_tmp = pdata->getNscFromMsecAll(msec_cur);
			if (nsc_tmp > nsc_cur && nsc_tmp > 0 && nsc_cur > 0){
				nsc_cur = nsc_tmp;
			}
			nsc_cur ++;
			if (nsc_cur >= num_scpos-1 ||
				(msec_cur > findscope.ed.late && findscope.ed.late >= 0)){
				over = true;
			}
		}
	}
	//--- CM構成で誤差が大きい所の補正処理 ---
	bool fix_mode = (cmscope.fixSt || cmscope.fixEd)? true : false;
	if (cmscope.logomode == false && det){
		setCMFormDetRevise(bounds, fix_mode);
	}
	//--- 端処理 ---
	if (cmscope.logomode == false && fix_mode == false){
		int lvlogo = pdata->getLevelUseLogo();
		//--- ロゴによるCM端の補正処理 ---
		if (det && lvlogo >= CONFIG_LOGO_LEVEL_USE_LOW){
			setCMFormByLogo(bounds, cmscope);
		}
		//--- 端の15秒未満処理 ---
		if (det || lvlogo > CONFIG_LOGO_LEVEL_USE_LOW){
			bool det_tmp = setCMFormEdge(bounds, cmscope, logo1st);
			if (det_tmp){
				det = true;
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// CM位置を手前方向に検出していく
// 入力：
//   nsc_base:  検索開始する位置番号
//   findscope: CM検索する範囲
//   logoon_st: 先頭部分のロゴ状態（false=ロゴなし true=ロゴあり）
// 出力：
//   返り値:    CM判定（false=なし  true=あり）
//   msec_stpoint: 検索対象の開始位置
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormDetect(Msec &msec_stpoint, Nsc nsc_base, RangeWideMsec findscope, bool logoon_st){
	msec_stpoint = -1;				// 一番最初の15秒単位位置（-1の時は存在なし）
	//--- 設定値読み込み ---
	int type_nosc = pdata->getConfigAction(ConfigActType::MuteNoSc);	// シーンチェンジなし無音の使用キャンセル
	//--- 15秒単位位置を後側から確認 ---
	bool det = false;				// 15秒単位位置の検出有無
	bool flag_force = false;		// 強制位置設定箇所の有無
	Nsc nsc_save = nsc_base;		// 次に設定する箇所
	{
		Msec msec_base = pdata->getMsecScp(nsc_base);
		Sec  sec_difend = 0;
		int  i = nsc_base;
		bool fin = false;
		while(fin == false){
			//--- 次の位置取得 ---
			i = pdata->getNscPrevScpChap(i, SCP_CHAP_CDET);		// CDET以上の次位置
			//--- 検索範囲を超えたら終了 ---
			if (i <= 0) break;
			Msec msec_i = pdata->getMsecScp(i);
			if ((msec_base <= findscope.st.late && findscope.st.late >= 0) ||
				(msec_i < findscope.st.early && findscope.st.early >= 0)){
				break;
			}
			//--- 強制位置設定の確認 ---
			ScpChapType chap_i = pdata->getScpChap(i);
			if (chap_i >= SCP_CHAP_DFORCE){
				flag_force = true;
				if (msec_stpoint > msec_i || msec_stpoint < 0){
					msec_stpoint = msec_i;
				}
			}
			//--- 中間地点情報は削除 ---
			if (chap_i == SCP_CHAP_DINT){
				pdata->setScpChap(i, SCP_CHAP_NONE);
				chap_i = SCP_CHAP_NONE;
			}
			//--- 15秒単位で検索 ---
			Sec sec_dif = pdata->cnv.getSecFromMsec(msec_base - msec_i);
			Sec sec_dif15 = ((sec_dif + 7) / 15) * 15;
			Msec msec_align = msec_base - (sec_dif15 * 1000);
			//--- 一番近い構成位置と確定位置を検索 ---
			Nsc nsc_near = pdata->getNscFromMsecChap(msec_align, pdata->msecValLap2, SCP_CHAP_CDET);
			Nsc nsc_chk  = pdata->getNscFromMsecChap(msec_align, pdata->msecValLap2, SCP_CHAP_DFORCE);
			//--- 終了条件判定 ---
			if (sec_dif15 > 120 || (sec_dif15 > sec_difend && sec_difend > 0)){
				fin = true;
			}
			else{
				bool flag_dif_near = (abs(msec_i - msec_align) < pdata->msecValLap2)? true : false;
				//--- 15秒構成区切りの既存位置があればその秒数で終了させるため保持 ---
				if (chap_i >= SCP_CHAP_CDET && sec_difend <= 0){
					int score_tmp = pdata->getScpScore(i);
					int score_base = pdata->getScpScore(nsc_base);
					//--- 15秒単位に近いまたはスコアが大きいものは終了秒数として確定 ---
					if (flag_dif_near || score_tmp + 30 >= score_base){
						sec_difend = sec_dif15;
					}
				}
				//--- 構成外の推測確定箇所がある場合は終了 ---
				if (chap_i >= SCP_CHAP_DFIX && nsc_near != i){
					if (flag_dif_near == false){
						fin = true;
					}
				}
			}
			//--- 15秒単位の構成 ---
			if (fin == false){
				if (nsc_near == i && nsc_chk < 0 && sec_dif15 > 0 && sec_dif15 <= 120){
					//--- 15秒単位の設定 ---
					det = true;
					fin = true;
					pdata->setScpChap(nsc_save, SCP_CHAP_DFIX);
					if (findscope.logomode == false){
						pdata->setScpArstat(nsc_save, SCP_AR_N_UNIT);
					}
					else{
						pdata->setScpArstat(nsc_save, SCP_AR_L_UNIT);
					}
					//--- 検索開始位置の更新 ---
					int msec_update = pdata->getMsecScp(nsc_save);
					if (msec_stpoint > msec_update || msec_stpoint < 0){
						msec_stpoint = msec_update;
					}
					//--- 強制設定がなければ次の位置設定 ---
					if (flag_force == 0){
						nsc_save = i;
						//--- 15秒単位箇所が無音で対象位置が少しだけ離れている場合 ---
						int nsc_tmp = pdata->getNscFromMsecChap(msec_align, pdata->msecValNear2, SCP_CHAP_CDET);
						if ( pdata->isSmuteFromMsec(msec_align) == true && nsc_tmp < 0 && type_nosc != 1){
							//--- 15秒単位箇所が対象位置よりスコアが高い場合差し替え ---
							int score_align = getScore(msec_align, findscope);
							//--- 60秒以上で30秒単位ではない位置はシーンチェンジなければ中止 ---
							if (sec_dif15 > 60 && sec_dif15 % 30 > 0){
								score_align = 0;
							}
							if (score_align > pdata->getScpScore(i)){
								//--- フレーム単位の場所に変換 ---
								int msec_newrev = pdata->cnv.getMsecAlignFromMsec(msec_align);
								int msec_newbk  = pdata->cnv.getMsecAdjustFrmFromMsec(msec_align, -1);
								//--- 更新前の位置を構成単位から外す ---
								pdata->setScpChap(i, SCP_CHAP_NONE);
								pdata->setScpStatpos(i, SCP_PRIOR_NONE);
								//--- データ更新 ---
								int nsc_tmp = pdata->insertScpos(msec_newrev, msec_newbk, i, SCP_PRIOR_DECIDE);
								pdata->setScpChap(nsc_tmp, SCP_CHAP_CDET);
								pdata->setScpScore(nsc_tmp, score_align);
								nsc_save = nsc_tmp;
								msec_update = pdata->getMsecScp(nsc_tmp);
								fin = false;					// 次の位置も検索
								sec_difend = 0;
								if (i > nsc_tmp){				// 挿入場所が手前なら現在位置も移動
									i = nsc_tmp;
								}
							}
						}
					}
				}
			}
		}
	}
	//--- 更新した場合 ---
	if (det){
		if (nsc_save > 0 && flag_force == false){
			//--- 最後の位置が非確定であれば更新 ---
			if (pdata->getScpChap(nsc_save) < SCP_CHAP_DECIDE){
				pdata->setScpChap(nsc_save, SCP_CHAP_DFIX);
				ScpArType arstat_save = (logoon_st)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
				pdata->setScpArstat(nsc_save, arstat_save);
			}
			Msec msec_update = pdata->getMsecScp(nsc_save);
			if (msec_stpoint > msec_update || msec_stpoint < 0){
				msec_stpoint = msec_update;
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// CM認識した構成で誤差が大きい所の修正
// 入力：
//   fix_mode: 位置固定モード(0:固定ではない 1:固定モード)
// 入出力：
//   bounds:   15秒単位CMの位置番号
// 出力：
//   pdata(chap,arstat)
//---------------------------------------------------------------------
void JlsAutoReform::setCMFormDetRevise(RangeMsec &bounds, bool fix_mode){
	//--- 設定値読み込み ---
	int mgn_cm_detect = pdata->getConfig(ConfigVarType::msecMgnCmDetect);
	//--- 無音位置番号を取得 ---
	RangeNsc nscbounds;
	if (pdata->getRangeNscFromRangeMsec(nscbounds, bounds) == false){
		return;
	}
	//--- 範囲内を順番に検出 ---
	{
		Nsc nsc_head = pdata->getNscNextScpChap(0, SCP_CHAP_DECIDE);
		Nsc nsc_loc0 = -1;
		Nsc nsc_loc1 = nscbounds.st;
		Msec msec_loc0 = 0;
		Msec msec_loc1 = pdata->getMsecScp(nsc_loc1);
		bool flag_cm0  = false;
		bool flag_cm1  = false;
		CalcModInfo calc1 = {};
		bool cont1 = true;
		while(cont1){
			//--- 次の位置取得 ---
			Nsc  nsc_loc2  = pdata->getNscNextScpChap(nsc_loc1, SCP_CHAP_DECIDE);
			Msec msec_loc2 = pdata->getMsecScp(nsc_loc2);
			bool cont2     = (nsc_loc2 <= nscbounds.ed && nsc_loc2 > 0)? true : false;
			ScpArType arstat_loc2 = pdata->getScpArstat(nsc_loc2);
			bool flag_cm2 = (arstat_loc2 == SCP_AR_N_UNIT)? true : false;
			CalcModInfo calc2;
			calcDifMod0515(calc2, msec_loc2 - msec_loc1);
			if (calc2.mod15 == -1) flag_cm2 = false;		// 15秒単位範囲外
			//--- 1つ前の位置がCMで15秒単位から誤差が大きい場合の処理 ---
			if (flag_cm1 && calc1.mod15 > mgn_cm_detect && nsc_loc0 >= 0){
				bool ignore1 = false;
				if (cont2 && flag_cm2){		// 次位置がCM有効な場合
					Msec msec_dif20 = msec_loc2-msec_loc0;
					if (msec_dif20 <= 120*1000+pdata->msecValLap2){
						//--- 削除でマージンが半分以下になるなら削除して無視する ---
						CalcModInfo calcsum;
						calcDifMod0515(calcsum, msec_loc2 - msec_loc0);
						if (calcsum.mod15 < mgn_cm_detect &&
							(calcsum.mod15 <= mgn_cm_detect/2 ||
							 calcsum.mod15 < (calc1.mod15 + calc2.mod15)/6)){
							//--- 更新（区切りを外して無視する） ---
							pdata->setScpChap(nsc_loc1, SCP_CHAP_NONE);
							ignore1 = true;
							nsc_loc1  = nsc_loc0;
							msec_loc1 = msec_loc0;
							calc2 = calcsum;
						}
					}
				}
				if (ignore1 == false && fix_mode == false){
					//--- CM側から5/10秒構成だった場合は区切りを作る ---
					Nsc nsc_subs = -1;
					Nsc nsc_sube = -1;
					SearchDirType dr = SEARCH_DIR_NEXT;
					if (flag_cm0 == false && flag_cm2 == true){		// 後側がCM
						nsc_subs = nsc_loc0;
						nsc_sube = nsc_loc1;
						dr = SEARCH_DIR_NEXT;
					}
					if (flag_cm0 == true && flag_cm2 == false){		// 前側がCM
						nsc_subs = nsc_loc1;
						nsc_sube = nsc_loc0;
						dr = SEARCH_DIR_PREV;
					}
					if (nsc_sube > 0){
						bool cont_tmp = true;
						Nsc  nsc_tmp  = nsc_subs;
						Msec msec_subs = pdata->getMsecScp(nsc_subs);
						Msec msec_sube = pdata->getMsecScp(nsc_sube);
						while(cont_tmp){
							nsc_tmp = pdata->getNscDirScpChap(nsc_tmp, dr, SCP_CHAP_NONE);
							if (nsc_tmp <= 0 || nsc_tmp <= nsc_loc0 || nsc_tmp >= nsc_loc1){
								cont_tmp = false;
							}
							else{
								FormLogoLevelExt extype = {};
								Msec msec_tmp = pdata->getMsecScp(nsc_tmp);
								if (abs(msec_tmp - msec_subs) >= pdata->msecValLap2){
									CalcModInfo calc_mod;
									Msec msec_diftmp = abs(msec_sube - msec_tmp);
									bool flag_len = isLengthLogoLevel(msec_diftmp, extype);
									int  flag_s05 = calcDifMod0515(calc_mod, msec_diftmp);
									if (flag_len && flag_s05 > 0){
										cont_tmp = false;
										pdata->setScpChap(nsc_tmp, SCP_CHAP_DINT);
										pdata->setScpArstat(nsc_tmp, SCP_AR_N_OTHER);
										pdata->setScpArstat(nsc_loc1, SCP_AR_N_OTHER);
										Sec sec_logos = pdata->getSecLogoComponentFromLogo(msec_loc0, msec_tmp);
										if (sec_logos > (abs(msec_tmp - msec_loc0) + 500)/2000){
											pdata->setScpArstat(nsc_tmp, SCP_AR_L_OTHER);
											if (bounds.st == msec_loc0){
												bounds.st = msec_tmp;
											}
										}
										Sec sec_logoe = pdata->getSecLogoComponentFromLogo(msec_tmp, msec_loc1);
										if (sec_logoe * 1000 + 500 >= abs(msec_loc1 - msec_tmp)/2){
											pdata->setScpArstat(nsc_loc1, SCP_AR_L_OTHER);
											if (bounds.ed == msec_loc1){
												bounds.ed = msec_tmp;
											}
										}
									}
								}
							}
						}
					}
				}
				if (ignore1 == false && fix_mode == false){
					Nsc nsc_delete = -1;
					//--- 更新（CM単位から誤差大きい時残すならロゴありとしてCMから外す） ---
					if (pdata->getConfigAction(ConfigActType::LogoUCGapCm) > 0){
						//--- 先頭か最後の区切りだった場合は外す ---
						if (nsc_loc0 == nsc_head){
							ScpArType arstat_loc0 = pdata->getScpArstat(nsc_loc0);
							if (jlsd::isScpArTypeLogo(arstat_loc0)){
								pdata->setScpArstat(nsc_loc1, SCP_AR_L_OTHER);
							}
							else{
								pdata->setScpArstat(nsc_loc1, SCP_AR_N_OTHER);
							}
							pdata->setScpChap(nsc_loc0, SCP_CHAP_NONE);
							nsc_delete = nsc_loc0;
						}
						else if (nsc_loc2 < 0){
							pdata->setScpChap(nsc_loc1, SCP_CHAP_NONE);
							nsc_delete = nsc_loc1;
						}
						else{
							pdata->setScpArstat(nsc_loc1, SCP_AR_L_UNIT);
						}
					}
					//--- ロゴありでは先頭最後が極端に端だった場合のみ外す ---
					else{
						if (nsc_loc0 == nsc_head && msec_loc0 <= pdata->msecValLap2){
							pdata->setScpChap(nsc_loc0, SCP_CHAP_NONE);
							nsc_delete = nsc_loc0;
						}
						else if (nsc_loc2 < 0 && msec_loc1 >= pdata->getMsecTotalMax() - pdata->msecValLap2){
							pdata->setScpChap(nsc_loc1, SCP_CHAP_NONE);
							nsc_delete = nsc_loc1;
						}
					}
					//--- CM境界を削除した場合の処理 ---
					if (nsc_delete == nscbounds.st){
						nscbounds.st = pdata->getNscNextScpChap(nsc_delete, SCP_CHAP_DECIDE);
						bounds.st = pdata->getMsecScp(nscbounds.st);
					}
					else if (nsc_delete == nscbounds.ed){
						nscbounds.ed = pdata->getNscPrevScpChap(nsc_delete, SCP_CHAP_DECIDE);
						bounds.ed = pdata->getMsecScp(nscbounds.ed);
					}
				}
			}
			//--- 位置のシフト ---
			nsc_loc0  = nsc_loc1;
			nsc_loc1  = nsc_loc2;
			msec_loc0 = msec_loc1;
			msec_loc1 = msec_loc2;
			flag_cm0  = flag_cm1;
			flag_cm1  = flag_cm2;
			calc1     = calc2;
			cont1     = cont2;
		}
	}
}

//---------------------------------------------------------------------
// CM位置端部分のロゴ情報による15秒単位補正
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormByLogo(RangeMsec &bounds, RangeWideMsec cmscope){
	bool det = false;
	//--- ロゴ構成区間内の15秒構成をCM化する処理 ---
	int rev_del_edge = pdata->getConfigAction(ConfigActType::LogoDelEdge);
	//--- 開始位置の補正 ---
	if (cmscope.fixSt == false && bounds.st > 0 && cmscope.st.just > 0){
		int msec_limit = setCMFormByLogoLimit(bounds.st, SEARCH_DIR_PREV);
		if (bounds.st > msec_limit || msec_limit < 0){
			if (bounds.st > cmscope.st.just){
				//--- CM情報の追加 ---
				FormCMByLogo form;
				form.msecTarget   = cmscope.st.just;
				form.msecLimit    = msec_limit;
				form.msecCmSide   = cmscope.st.late;
				form.msecLogoSide = cmscope.st.early;
				form.revDelEdge   = rev_del_edge;
				form.dr           = SEARCH_DIR_PREV;
				int det_tmp = setCMFormByLogoAdd(bounds.st, form);
				if (det_tmp) det = true;
			}
		}
	}
	//--- 終了位置の補正 ---
	if (cmscope.fixEd == 0 && bounds.ed > 0 && cmscope.ed.just > 0){
		Msec msec_limit = setCMFormByLogoLimit(bounds.ed, SEARCH_DIR_NEXT);
		if (bounds.ed < msec_limit || msec_limit < 0){
			if (bounds.ed < cmscope.ed.just){
				//--- CM情報の追加 ---
				FormCMByLogo form;
				form.msecTarget   = cmscope.ed.just;
				form.msecLimit    = msec_limit;
				form.msecCmSide   = cmscope.ed.early;
				form.msecLogoSide = cmscope.ed.late;
				form.revDelEdge   = rev_del_edge;
				form.dr           = SEARCH_DIR_NEXT;
				int det_tmp = setCMFormByLogoAdd(bounds.ed, form);
				if (det_tmp) det = true;
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// CM位置端部分のロゴ情報による15秒単位補正で拡張限界地点を取得
// 出力：
//   返り値: 拡張限界地点（-1の時は設定なし）
//---------------------------------------------------------------------
int JlsAutoReform::setCMFormByLogoLimit(Msec msec_point, SearchDirType dr){
	if (msec_point < 0){
		return -1;
	}
	//--- 方向 ---
	LogoEdgeType edge_logo;
	int nrf_target;
	if (dr == SEARCH_DIR_PREV){			// CM前側に拡張時は前のロゴ立ち上がりを限界地点
		edge_logo = LOGO_EDGE_RISE;
		nrf_target = -1;
	}
	else{								// CM後側に拡張時は後のロゴ立ち下がりを限界地点
		edge_logo = LOGO_EDGE_FALL;
		nrf_target = pdata->sizeDataLogo();
	}
	//--- 対応ロゴ位置取得 ---
	Msec msec_result = -1;
	Msec msec_last_r = -1;
	bool det = false;
	do{
		int msec_lg_c, msec_lg_l, msec_lg_r;
		nrf_target = pdata->getNrfDirLogo(nrf_target, dr, edge_logo, LOGO_SELECT_VALID);
		if (nrf_target >= 0){
			pdata->getMsecLogoNrfWide(msec_lg_c, msec_lg_l, msec_lg_r, nrf_target);
			if (dr == SEARCH_DIR_PREV){
				if (msec_lg_r > msec_point){
					det = true;
					msec_result = msec_last_r + pdata->msecValLap2;
				}
			}
			else{
				if (msec_lg_l > msec_point){
					det = true;
					msec_result = msec_lg_l - pdata->msecValLap2;
				}
			}
			msec_last_r = msec_lg_r;
		}
	}while(nrf_target >= 0 && det == false);
	//--- 構成位置による設定 ---
	Nsc nsc_point = pdata->getNscFromMsecChap(msec_point, pdata->msecValNear2, SCP_CHAP_DFIX);
	if (nsc_point <= 0){
		msec_result = msec_point;		// 構成がなかったらその場で終了
	}
	else{		// 確定位置までに制限
		nsc_point = pdata->getNscDirScpChap(nsc_point, dr, SCP_CHAP_DECIDE);
		if (nsc_point >= 0){
			int msec_point = pdata->getMsecScp(nsc_point);
			if (dr == SEARCH_DIR_PREV){
				if (msec_result < msec_point || msec_result < 0){
					msec_result = msec_point;
				}
			}
			else{
				if (msec_result > msec_point || msec_result < 0){
					msec_result = msec_point;
				}
			}
		}
	}
	return msec_result;
}

//---------------------------------------------------------------------
// CM位置端部分のロゴ情報による15秒単位補正で追加処理
// 入出力：
//   msec_result : 追加後の15秒単位の位置
// 出力：
//   返り値: CM追加判定（false=なし  true=あり）
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormByLogoAdd(Msec &msec_result, FormCMByLogo form){
	Nsc nsc_point = pdata->getNscFromMsecChap(
						msec_result, pdata->msecValNear2, SCP_CHAP_DFIX);
	if (nsc_point < 0 || form.revDelEdge <= 1){	// 追加しない条件
		return 0;
	}
	//--- 検索方向 ---
	ScpArType arstat_new = SCP_AR_N_OTHER;
	int step;
	Msec msec_cmdetect;
	Msec msec_limit_rev;
	if (form.dr == SEARCH_DIR_PREV){
		step = -1;
		msec_cmdetect = msec_result - form.msecTarget;
		msec_limit_rev = form.msecLimit + pdata->msecValLap2;
		ScpArType arstat_point = pdata->getScpArstat(nsc_point);
		if (jlsd::isScpArTypeLogo(arstat_point)){
			arstat_new = SCP_AR_L_OTHER;
		}
	}
	else{
		step = +1;
		msec_cmdetect = form.msecTarget - msec_result;
		msec_limit_rev = form.msecLimit - pdata->msecValLap2;
	}
	if (msec_cmdetect < 0){
		return 0;
	}
	//--- 設定値読み込み ---
	int type_nosc = pdata->getConfigAction(ConfigActType::MuteNoSc);	// シーンチェンジなし無音の使用キャンセル
	//--- 15/30/45/60秒地点の検索 ---
	bool det = false;
	Msec msec_set_point = msec_result;					// 確定設定位置
	for(int i=0; i<4; i++){
		int msec_difpt = 15*1000*(i+1);
		int msec_revpt = msec_result + (step * msec_difpt);
		//--- 無音区切りであれば候補 ---
		if (pdata->isSmuteFromMsec(msec_revpt)){
			bool flag_match = true;
			//--- limitによる条件 ---
			if (form.msecLimit >= 0 &&
				((msec_revpt <= msec_limit_rev && step < 0) ||
				 (msec_revpt >= msec_limit_rev && step > 0))){
					flag_match = false;
			}
			//--- limit以外の条件 ---
			if (flag_match == true){
				bool match_none = false;		// CM期間なし判定
				bool match_full = false;		// CM期間が全体判定
				bool match_half = false;		// CM期間が半分以上判定
				bool match_minlogo = false;		// ロゴ期間がロゴ有情報確定以上の判定
				bool match_later = false;		// CM期間が後で削除の対象となる期間か判定
				{
					Msec msec_difbase = abs(msec_result - msec_set_point);
					Msec msec_diftarget = msec_cmdetect - msec_difbase;
					Msec msec_difrev  = abs(msec_revpt - msec_set_point);
					if (msec_diftarget <= pdata->msecValLap2){
						match_none = true;
					}
					else if (msec_diftarget >= msec_difrev - pdata->msecValLap2){
						match_full = true;
					}
					else if (msec_diftarget >= msec_difrev / 2){
						match_half = true;
					}
					Msec msec_recmin = pdata->getConfig(ConfigVarType::msecWLogoLgMin);
					if (msec_difrev - msec_diftarget >= msec_recmin){
						Msec msec_difcm_max = abs(msec_result - form.msecLogoSide);
						if (msec_difrev - msec_difcm_max >= msec_recmin){
							match_minlogo = true;
						}
					}
					if (msec_diftarget < 15000 - pdata->msecValLap2 && msec_diftarget > 0){
						FormLogoLevelExt extype = {};
						extype.mid5s = true;			// ロゴレベルMIDLも5秒単位にする
						if (isLengthLogoLevel(msec_diftarget, extype)){
							match_later = true;
						}
					}
				}
				bool match_logo = false;
				if ((step > 0 && form.msecLogoSide < msec_revpt) ||
					(step < 0 && form.msecLogoSide > msec_revpt)){
					match_logo = true;			// ロゴ期間が確実に存在
				}
				bool match_cm = false;
				if ((step > 0 && form.msecCmSide > msec_set_point) ||
					(step < 0 && form.msecCmSide < msec_set_point)){
					match_cm = true;			// CM期間が確実に存在
				}
				//--- 設定別の条件 ---
				if (form.revDelEdge == 4){
					flag_match = true;
				}
				else if (match_none){
					flag_match = false;
				}
				else if (match_minlogo || match_later){
					flag_match = false;
				}
				else if (match_full){
					flag_match = true;
				}
				else if (form.revDelEdge <= 1){
					if (!match_half || match_logo){
						flag_match = false;
					}
				}
				else if (form.revDelEdge == 2){
					if (!match_half){
						flag_match = false;
					}
				}
				else if (form.revDelEdge == 3){
					if (!match_half && !match_cm){
						flag_match = false;
					}
				}
			}
			//--- CMとして追加 ---
			if (flag_match == true){
				int nsc_new = pdata->getNscFromMsecChap(
								msec_revpt, pdata->msecValNear2, SCP_CHAP_DFIX);
				if (nsc_new < 0){
					nsc_new = pdata->getNscFromMsecChap(
								msec_revpt, pdata->msecValNear2, SCP_CHAP_DUPE);
					if (nsc_new < 0 && type_nosc != 1){		// 無音シーンチェンジ挿入
						nsc_new = pdata->getNscForceMsec(msec_revpt, LOGO_EDGE_RISE);
					}
				}
				if (nsc_new > 0){
					det = true;
					//--- 挿入があるためnsc取り直し ---
					int nsc_set_point = pdata->getNscFromMsecChap(
										msec_set_point, pdata->msecValNear2, SCP_CHAP_DFIX);
					pdata->setScpChap(nsc_new, SCP_CHAP_DFIX);
					if (step < 0){
						pdata->setScpArstat(nsc_new, arstat_new);
						pdata->setScpArstat(nsc_set_point, SCP_AR_N_UNIT);
					}
					else{
						pdata->setScpArstat(nsc_new, SCP_AR_N_UNIT);
					}
					msec_set_point = msec_revpt;
				}
			}
		}
	}
	//--- 結果更新 ---
	msec_result = msec_set_point;
	return det;
}



//---------------------------------------------------------------------
// CM位置の端部分15秒未満処理
// 入力：
//   cmscope:     検索範囲
//   logo1st:     先頭位置のロゴ有無（0=なし  1=あり）
// 入出力：
//   bounds: CM確定位置
// 出力：
//   返り値: 構成追加判定（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormEdge(RangeMsec &bounds, RangeWideMsec cmscope, bool logo1st){
	//--- ロゴ情報の使用判断 ---
	int lvlogo = pdata->getLevelUseLogo();
	ScpChapType chap_cmp;
	if (lvlogo < CONFIG_LOGO_LEVEL_USE_HIGH){
		chap_cmp = SCP_CHAP_CPOSIT;			// 5秒単位構成
	}
	else{
		chap_cmp = SCP_CHAP_NONE;			// 重複以外すべて
	}
	//--- ロゴからの修正候補取得 ---
	FormCMEdgeSide sidest;
	FormCMEdgeSide sideed;
	bool valid_detect_st = setCMFormEdgeSideInfo(sidest, bounds.st, cmscope.st, chap_cmp, false);
	bool valid_detect_ed = setCMFormEdgeSideInfo(sideed, bounds.ed, cmscope.ed, chap_cmp, true);
	//--- 構成制限 ---
	int level;
	if (lvlogo <= CONFIG_LOGO_LEVEL_USE_LOW){
		level = 0;			// 15秒単位
	}
	else if (lvlogo < CONFIG_LOGO_LEVEL_USE_HIGH){
		level = 1;			// 5秒単位
	}
	else if (lvlogo == CONFIG_LOGO_LEVEL_USE_HIGH){
		level = 2;			// 秒単位
	}
	else{
		level = 3;			// 制限なし
	}
	//--- 端部分の追加またはカットの実行 ---
	bool change_st = false;
	bool change_ed = false;
	if (valid_detect_st && sidest.nscFixed >= 0){
		sidest.logoModePrev = logo1st;						// 先頭ロゴ状態を設定
		change_st = setCMFormEdgeSetSide(sidest, level);
	}
	if (valid_detect_ed && sideed.nscFixed >= 0){
		change_ed = setCMFormEdgeSetSide(sideed, level);
	}
	//--- 15秒構成がない場合のロゴ前後位置で構成作成 ---
	if (valid_detect_st && valid_detect_ed && sidest.nscFixed < 0 && sideed.nscFixed < 0){
		if (lvlogo == CONFIG_LOGO_LEVEL_USE_MIDL){
			level = 0;			// 15秒単位
		}
		RangeNsc rnsc_detect = {sidest.nscDetect, sideed.nscDetect};
		RangeNsc rnsc_scope;
		rnsc_scope.st = pdata->getNscPrevScpChap(sideed.nscDetect, SCP_CHAP_DECIDE);
		rnsc_scope.ed = pdata->getNscNextScpChap(sidest.nscDetect, SCP_CHAP_DECIDE);
		change_st = setCMFormEdgeSetBoth(rnsc_detect, rnsc_scope, level, logo1st);
		change_ed = change_st;
	}
	//--- 範囲更新 ---
	if (change_st){
		bounds.st = pdata->getMsecScp(sidest.nscDetect);
	}
	if (change_ed){
		bounds.ed = pdata->getMsecScp(sideed.nscDetect);
	}

	bool det = change_st | change_ed;
	return det;
}

//---------------------------------------------------------------------
// CM位置の端部分（片側）のロゴからの修正候補取得
// 出力：
//   返り値:  候補有無判定（false=なし  true=あり）
//   sidesel: ロゴからの修正候補
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormEdgeSideInfo(FormCMEdgeSide &sidesel, Msec msec_bounds, WideMsec wmsec_scope, ScpChapType chap_cmp, bool endside){
	//--- ロゴ構成区間内の端構成をCM化する処理 ---
	int rev_del_edge = pdata->getConfigAction(ConfigActType::LogoDelEdge);
	//--- 検索範囲を設定 ---
	bool valid_detect = true;
	WideMsec usescope = wmsec_scope;
	if (wmsec_scope.just < 0){
		valid_detect = false;
	}
	else{
		if (rev_del_edge <= 3){			// ロゴ中心付近に制限
			if (endside == false){
				if (wmsec_scope.early < wmsec_scope.just - pdata->msecValSpc){
					usescope.early = wmsec_scope.just - pdata->msecValSpc;
				}
			}
			else{
				if (wmsec_scope.late > wmsec_scope.just + pdata->msecValSpc){
					usescope.late = wmsec_scope.just + pdata->msecValSpc;
				}
			}
		}
		if (rev_del_edge == 0){			// CM最小限
			if (endside == false){
				if (msec_bounds <= wmsec_scope.late && msec_bounds >= 0){
					valid_detect = false;
				}
			}
			else{
				if (msec_bounds >= wmsec_scope.early && msec_bounds >= 0){
					valid_detect = false;
				}
			}
		}
	}
	if (valid_detect == false){
		sidesel.nscDetect = -1;
		return false;
	}

	//--- どちらの端情報か記憶 ---
	sidesel.endSide = endside;
	//--- ロゴ変化範囲内の無音シーンチェンジを検出 ---
	sidesel.nscDetect = pdata->getNscFromWideMsecByChap(usescope, chap_cmp);
	//--- CM構成確定している位置を取得 ---
	sidesel.nscFixed = pdata->getNscFromMsecChap(msec_bounds, pdata->msecValSpc, SCP_CHAP_DECIDE);
	if ((pdata->getNscFromWideMsecByChap(usescope, SCP_CHAP_DFORCE) >= 0) ||
		(pdata->getNscFromMsecChap(msec_bounds, pdata->msecValSpc, SCP_CHAP_DFORCE) >= 0)){
		sidesel.nscDetect = -1;			// 固定箇所があれば設定しない
	}
	//--- CM確定位置と反対側の確定位置を取得 ---
	sidesel.nscOther = -1;							// 開始位置
	if (sidesel.nscDetect >= 0 && sidesel.nscFixed >= 0){
		if (sidesel.nscDetect < sidesel.nscFixed){
			sidesel.nscOther = pdata->getNscPrevScpChap(sidesel.nscFixed, SCP_CHAP_DECIDE);
			if (sidesel.nscOther >= sidesel.nscDetect && sidesel.nscOther >= 0){	// 先に確定位置があったら中止
				sidesel.nscDetect = -1;
			}
		}
		else if (sidesel.nscDetect > sidesel.nscFixed){
			sidesel.nscOther = pdata->getNscNextScpChap(sidesel.nscFixed, SCP_CHAP_DECIDE);
			if (sidesel.nscOther <= sidesel.nscDetect && sidesel.nscOther >= 0){	// 先に確定位置があったら中止
				sidesel.nscDetect = -1;
			}
		}
		else{
			sidesel.nscDetect = -1;
		}
	}
	//--- 前後のロゴ情報基本設定 ---
	if (endside == false){
		sidesel.logoModePrev = true;
		sidesel.logoModeNext = false;
	}
	else{
		sidesel.logoModePrev = false;
		sidesel.logoModeNext = true;
	}
	if (sidesel.nscDetect < 0) valid_detect = false;
	return valid_detect;
}

//---------------------------------------------------------------------
// CM位置の端部分15秒未満処理 - 片側の実行
// 出力：
//   返り値: 構成追加判定（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormEdgeSetSide(FormCMEdgeSide &sidesel, int level){
	Msec msec_detect = pdata->getMsecScp(sidesel.nscDetect);
	Msec msec_fixed  = pdata->getMsecScp(sidesel.nscFixed);
	Msec msec_other  = pdata->getMsecScp(sidesel.nscOther);
	Msec msec_dif_fixed = abs(msec_detect - msec_fixed);
	Msec msec_dif_other = abs(msec_detect - msec_other);
	SearchDirType dr = (sidesel.nscDetect < sidesel.nscFixed)? SEARCH_DIR_PREV : SEARCH_DIR_NEXT;
	//--- 対象２点存在なし、確定位置と隣接時は実行なし ---
	if (sidesel.nscDetect < 0 ||
		sidesel.nscFixed  < 0 ||
		(msec_dif_fixed <= pdata->msecValLap2) ||
		(msec_dif_other <= pdata->msecValLap2 && sidesel.nscOther >= 0)){
		return false;
	}
	//--- 対象２点の距離から実行有無を検出 ---
	bool det = false;
	if (level <= 1){
		//--- 15秒単位(level=0)または5秒単位(levle=1)を検出 ---
		CalcModInfo calcmod;
		int tmpdet = calcDifMod0515(calcmod, msec_dif_fixed);
		if (tmpdet >= 2 || (level == 1 && tmpdet >= 1)){
			det = true;
		}
	}
	else{
		CalcDifInfo calc1;
		calcDifSelect(calc1, 0, msec_dif_fixed);
		//--- 秒単位の場合(level=2)か、単位関係なく処理する場合(level=3) ---
		if (calc1.gap <= pdata->msecValExact || level == 3){
			det = true;
		}
	}
	//--- 実行 ---
	if (det){
		bool flag_other_s15  = (isCmLengthMsec(msec_dif_other) && sidesel.nscOther >= 0)? true : false;
		bool flag_fixed_s15  = isCmLengthMsec(msec_dif_fixed);
		pdata->setScpChap(sidesel.nscDetect, SCP_CHAP_DFIX);			// 固定設定
		if (dr == SEARCH_DIR_PREV){			// 設定地点が前方向
			ScpArType arstat_fixed;
			if (sidesel.logoModeNext){
				arstat_fixed = (flag_fixed_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
			}
			else{
				arstat_fixed = (flag_fixed_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
			}
			ScpArType arstat_detect;
			if (sidesel.logoModePrev){
				arstat_detect = (flag_other_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
			}
			else{
				arstat_detect = (flag_other_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
			}
			pdata->setScpArstat(sidesel.nscFixed,  arstat_fixed);
			pdata->setScpArstat(sidesel.nscDetect, arstat_detect);
		}
		else{							// 設定地点が後方向
			ScpArType arstat_detect;
			if (sidesel.logoModePrev){
				arstat_detect = (flag_fixed_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
			}
			else{
				arstat_detect = (flag_fixed_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
			}
			ScpArType arstat_other;
			if (sidesel.logoModeNext){
				arstat_other = (flag_other_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
			}
			else{
				arstat_other = (flag_other_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
			}
			pdata->setScpArstat(sidesel.nscDetect, arstat_detect);
			pdata->setScpArstat(sidesel.nscOther,  arstat_other);
		}
	}
	return det;
}

//---------------------------------------------------------------------
// CM位置の端部分15秒未満処理 - 15秒単位構成確定位置がない場合の２点間追加
// 出力：
//   返り値: 構成追加判定（false=なし  true=あり）
//---------------------------------------------------------------------
bool JlsAutoReform::setCMFormEdgeSetBoth(RangeNsc rnsc_detect, RangeNsc rnsc_scope, int level, bool logo1st){
	RangeMsec rmsec_detect = getRangeMsec(rnsc_detect);
	RangeMsec rmsec_scope  = getRangeMsec(rnsc_scope);
	Msec msec_dif_st = abs(rmsec_detect.st - rmsec_scope.st);
	Msec msec_dif_ed = abs(rmsec_detect.ed - rmsec_scope.ed);
	Msec msec_dif_detect = abs(rmsec_detect.st - rmsec_detect.ed);

	//--- 対象２点存在なし、検出前後関係逆、確定位置と隣接時は実行なし ---
	if (rnsc_detect.st < 0 ||
		rnsc_detect.ed < 0 ||
		(rnsc_detect.st >= rnsc_detect.ed) ||
		(rnsc_detect.st <= rnsc_scope.st && rnsc_scope.st >= 0) ||
		(rnsc_detect.ed >= rnsc_scope.ed && rnsc_scope.ed >= 0) ||
		(msec_dif_st <= pdata->msecValLap2 && rnsc_scope.st >= 0) ||
		(msec_dif_ed <= pdata->msecValLap2 && rnsc_scope.ed >= 0)){
		return false;
	}
	//--- 対象２点の距離から実行有無を検出 ---
	bool det = false;
	if (level <= 2){
		//--- 15秒単位(level=0)または5秒単位(levle=1)を検出 ---
		CalcModInfo calcmod;
		int tmpdet = calcDifMod0515(calcmod, msec_dif_detect);
		if (tmpdet >= 2 || (level == 1 && tmpdet >= 1)){
			det = true;
		}
	}
	else{
		CalcDifInfo calc1;
		calcDifSelect(calc1, 0, msec_dif_detect);
		//--- 秒単位の場合(level=2)か、単位関係なく処理する場合(level=3) ---
		if (calc1.gap <= pdata->msecValExact || level == 3){
			det = true;
		}
	}
	//--- 実行 ---
	if (det){
		Nsc  nsc_end = rnsc_scope.ed;
		bool flag_st_s15 = isCmLengthMsec(msec_dif_st);
		bool flag_ed_s15 = isCmLengthMsec(msec_dif_ed);
		bool flag_detect_s15 = isCmLengthMsec(msec_dif_detect);
		bool logo_next = true;
		if (nsc_end >= 0){
			ScpArType arstat_end = pdata->getScpArstat(nsc_end);
			logo_next = jlsd::isScpArTypeLogo(arstat_end);
		}
		ScpArType arstat_c1;
		ScpArType arstat_c2;
		ScpArType arstat_c3;
		if (logo1st){
			arstat_c1  = (flag_st_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
		}
		else{
			arstat_c1  = (flag_st_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
		}
		arstat_c2 = (flag_detect_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
		if (logo_next){
			arstat_c3  = (flag_ed_s15)? SCP_AR_L_UNIT : SCP_AR_L_OTHER;
		}
		else{
			arstat_c3  = (flag_ed_s15)? SCP_AR_N_UNIT : SCP_AR_N_OTHER;
		}
		pdata->setScpChap(rnsc_detect.st, SCP_CHAP_DFIX);
		pdata->setScpChap(rnsc_detect.ed, SCP_CHAP_DFIX);
		pdata->setScpArstat(rnsc_detect.st, arstat_c1);
		pdata->setScpArstat(rnsc_detect.ed, arstat_c2);
		if (nsc_end >= 0){
			pdata->setScpArstat(nsc_end,        arstat_c3 );
		}
	}
	return det;
}



//=====================================================================
// 対象範囲のスコアを作成
//=====================================================================

//---------------------------------------------------------------------
// 対象範囲内のスコア設定
// 入力：
//   scope
//     st: スコア計算をする開始位置
//     ed: スコア計算をする終了位置
// 出力：
//   pdata(score) : 対象位置のスコア
//   pdata(chap)  : 対象位置を構成候補外に間引きする場合-1を設定
//---------------------------------------------------------------------
void JlsAutoReform::setScore(RangeMsec scope){
	int num_scpos = pdata->sizeDataScp();
	Nsc nsc_cur = 1;
	Nsc nsc_last = -1;
	bool over = false;
	if (nsc_cur >= num_scpos-1){				// 検索最後まで完了時は終了
		over = true;
	}
	while( over == 0 ){
		ScpChapType chap_cur = pdata->getScpChap(nsc_cur);
		int msec_cur = pdata->getMsecScp(nsc_cur);
		if ((msec_cur >= scope.st || scope.st < 0) &&
			(msec_cur <= scope.ed || scope.ed < 0)){
			//--- スコア取得 ---
			int score = getScore(msec_cur, scope);
			int score_org = pdata->getScpScore(nsc_cur);
			if (chap_cur < SCP_CHAP_DECIDE || score >= score_org){
				pdata->setScpScore(nsc_cur, score);
			}
			//--- chap設定 ---
			if (chap_cur < SCP_CHAP_DECIDE){
				if (score >= 30){
					chap_cur = SCP_CHAP_CPOSQ;
				}
				else if (score >= 10){
					chap_cur = SCP_CHAP_CPOSIT;
				}
				else{
					chap_cur = SCP_CHAP_NONE;
				}
				pdata->setScpChap(nsc_cur, chap_cur);
			}
			//--- 隣接候補あれば間引き ---
			if (nsc_last >= 0 && chap_cur > SCP_CHAP_NONE){
				ScpChapType chap_last = pdata->getScpChap(nsc_last);
				int msec_last = pdata->getMsecScp(nsc_last);
				int msec_diff = msec_cur - msec_last;
				if (msec_diff <= pdata->msecValLap1){
					if (pdata->getScpScore(nsc_last) < score &&
						chap_last < SCP_CHAP_DECIDE){
						pdata->setScpChap(nsc_last, SCP_CHAP_DUPE);		// 前側を間引き
					}
					else if (chap_cur <= 2){
						pdata->setScpChap(nsc_cur, SCP_CHAP_DUPE);		// 現位置を間引き
					}
				}
			}
		}
		//--- 次位置設定 ---
		if (chap_cur > SCP_CHAP_NONE){
			nsc_last = nsc_cur;
		}
		nsc_cur ++;
		if (nsc_cur >= num_scpos-1){
			over = true;
		}
	}
}

void JlsAutoReform::setScore(RangeFixMsec fixscope){
	RangeMsec scope = {fixscope.st, fixscope.ed};
	setScore(scope);
}


//=====================================================================
// 対象範囲の構成区切りを作成
//=====================================================================

//---------------------------------------------------------------------
// 対象範囲内の推測構成情報を作成
// 入力：
//   fixscope
//     st:     構成推測開始位置
//     ed:     構成推測終了位置
//     fixSt:  開始部分固定状況（0:候補 1:固定）
//     fixEd:  終了部分固定状況（0:候補 1:固定）
// 出力：
//   pdata(chap)  : 対象位置を構成候補として設定
//---------------------------------------------------------------------
void JlsAutoReform::setChap(RangeFixMsec fixscope){
	TraceChap trace;
	trace.numDa = 0;
	trace.numDb = 0;

	//--- 区切り検出開始 ---
	int num_scpos = pdata->sizeDataScp();
	Nsc nsc_cur = 1;
	bool over = false;
	if (nsc_cur >= num_scpos-1){					// 検索最後まで完了時は終了
		over = true;
	}
	//--- 範囲内の候補構成を検索 ---
	while( over == false ){
		ScpChapType chap_cur = pdata->getScpChap(nsc_cur);
		Msec msec_cur = pdata->getMsecScp(nsc_cur);
		//--- 範囲内の候補位置であれば処理 ---
		if ((msec_cur >= fixscope.st || fixscope.st < 0) &&
			(msec_cur <= fixscope.ed || fixscope.ed < 0) &&
			chap_cur > SCP_CHAP_NONE){
			setChapUpdate(trace, nsc_cur);
			setChapGuess( trace, nsc_cur);
		}
		//--- 次の位置設定 ---
		nsc_cur ++;
		if ((nsc_cur >= num_scpos-1) ||
			(msec_cur > fixscope.ed && fixscope.ed > 0)){
			over = true;
		}
		//--- 最終確定処理 ---
		if (over){
			setChapUpdate(trace, -1);
		}
	}
	//--- 重複削除 ---
	setChapThinOverlap(fixscope);
}

//---------------------------------------------------------------------
// 対象位置の推測構成情報を更新
// 入力：
//   trace
//     num_d:   入出力：保持データ数（サイズ=2、継続＋新候補）
//     nsc_da:  入出力：保持データの継続候補位置（サイズ=SIZE_SETFORM_KEEP）
//     nsc_db:  入出力：保持データの新候補位置（サイズ=SIZE_SETFORM_KEEP）
//   nsc_cur:   追加するデータの位置（-1の時は完了処理）
// 出力：
//   pdata(chap)  : 対象位置を構成候補として設定
//---------------------------------------------------------------------
void JlsAutoReform::setChapUpdate(TraceChap &trace, Nsc nsc_cur){
	Msec msec_cur = 0;
	if (nsc_cur >= 0){
		msec_cur = pdata->getMsecScp(nsc_cur);
	}
	//--- 新候補位置の確定処理 ---
	if (trace.numDb > 0){
		//--- 新候補の先頭位置を取得 ---
		Msec msec_chk = pdata->getMsecScp(trace.nscDb[0]);
		Msec msec_dif;
		if (nsc_cur >= 0){
			msec_dif = msec_cur - msec_chk;
		}
		else{						// 完了処理の場合
			msec_dif = 180 * 1000 + pdata->msecValLap2;
		}
		//--- 最初の新候補から3分以上経過した場合 ---
		if (msec_dif >= 180 * 1000 + pdata->msecValLap2){
			int msec_tmp;
			if (trace.numDa > 0){
				int nsc_tmp = trace.nscDa[trace.numDa-1];
				msec_tmp  = pdata->getMsecScp(nsc_tmp);
			}
			else{
				msec_tmp = msec_chk - 1;
			}
			//--- 新候補の後に継続候補がなければ新候補を確定する ---
			if (msec_chk > msec_tmp && trace.numDb > 1){		// 2か所以上の候補
				for(int j=0; j<trace.numDb; j++){
					int nsc_tmp = trace.nscDb[j];
					trace.nscDa[j] = nsc_tmp;
				}
				trace.numDa = trace.numDb;
				trace.numDb = 0;
				//--- 更新 ---
				setChapFixed(trace);
			}
			else{											// それ以外の時は削除
				trace.numDb = 0;
			}
		}
	}
	//--- 継続候補の確定処理 ---
	if (trace.numDa > 0){
		bool flag_loop = true;
		while(flag_loop){
			flag_loop = false;
			//--- 継続候補の先頭位置を取得 ---
			Nsc nsc_chk = trace.nscDa[0];
			Msec msec_chk = pdata->getMsecScp(nsc_chk);
			Msec msec_dif;
			if (nsc_cur >= 0){
				msec_dif = msec_cur - msec_chk;
			}
			else{						// 完了処理の場合
				msec_dif = 180 * 1000 + pdata->msecValLap2;
			}
			//--- 先頭の継続候補から3分以上経過した場合 ---
			if (msec_dif >= 180 * 1000 + pdata->msecValLap2){
				setChapFixed(trace);
				//--- 確定して候補から削除 ---
				for(int j=1; j<trace.numDa; j++){
					int nsc_tmp = trace.nscDa[j];
					trace.nscDa[j-1] = nsc_tmp;
				}
				trace.numDa -= 1;
				//--- 継続候補が残っている場合は次位置も検索 ---
				if (trace.numDa > 0){
					flag_loop = true;
				}
			}
		};
	}
}

//---------------------------------------------------------------------
// 対象位置の推測構成情報を作成
// 入出力：
//   trace
//     num_d:   入出力：保持データ数（サイズ=2、継続＋新候補）
//     nsc_da:  入出力：保持データの継続候補位置（サイズ=SIZE_SETFORM_KEEP）
//     nsc_db:  入出力：保持データの新候補位置（サイズ=SIZE_SETFORM_KEEP）
//   nsc_cur:   追加するデータの位置（-1の時は完了処理）
//---------------------------------------------------------------------
void JlsAutoReform::setChapGuess(TraceChap &trace, Nsc nsc_cur){
	//--- 有効確認 ---
	if (nsc_cur < 0) return;

	//--- 追加データ情報を取得 ---
	int score_cur = pdata->getScpScore(nsc_cur);
	int msec_cur  = pdata->getMsecScp(nsc_cur);
	//--- スコア情報の整理（継続候補） ---
	int scoremax_prev = 0;
	int scoremax_mid  = 0;
	int scoremax_post = 0;
	if (trace.numDa > 0){
		Nsc nsc_chks;
		Nsc nsc_chke;
		if (trace.numDb > 0){			// 新候補が存在したら
			nsc_chks = trace.nscDb[0];	// 新候補最初の位置
			nsc_chke = trace.nscDb[trace.numDb-1];
		}
		else{							// 新候補なければ継続候補すべて
			nsc_chks = trace.nscDa[trace.numDa-1]+1;
			nsc_chke = nsc_chks;
		}
		//--- 新候補最初の位置までの最大スコア取得 ---
		for(int j=0; j<trace.numDa; j++){
			Nsc  nsc_tmp   = trace.nscDa[j];
			Msec msec_tmp  = pdata->getMsecScp(nsc_tmp);
			Msec msec_dif  = abs(msec_cur - msec_tmp);
			int  score_tmp = setChapGetDistanceScore(nsc_tmp, msec_dif);
			if (nsc_tmp < nsc_chks){
				if (scoremax_prev < score_tmp){
					scoremax_prev = score_tmp;
				}
			}
			else if (nsc_tmp > nsc_chke){
				if (scoremax_mid < score_tmp){
					scoremax_mid = score_tmp;
				}
			}
		}
		scoremax_post = setChapMaxscore(trace.nscDa[trace.numDa-1]);
	}
	//--- スコア情報の整理（新候補） ---
	int scoremax_db = 0;
	if (trace.numDb > 0){
		for(int j=0; j<trace.numDb; j++){
			int  nsc_tmp   = trace.nscDb[j];
			Msec msec_tmp  = pdata->getMsecScp(nsc_tmp);
			Msec msec_dif  = abs(msec_cur - msec_tmp);
			int  score_tmp = setChapGetDistanceScore(nsc_tmp, msec_dif);
			if (scoremax_db < score_tmp){
				scoremax_db = score_tmp;
			}
		}
	}

	//--- 保持情報から継続候補(k=0)と新候補(k=1)の5/15秒単位距離を取得 ---
	int chkmod_d[2];
	int difmod_d[2];
	for(int k=0; k<2; k++){
		chkmod_d[k] = 0;
		int jmax = (k==0)? trace.numDa : trace.numDb;
		for(int j=0; j<jmax; j++){
			int nsc_chk;
			if (k == 0){
				nsc_chk = trace.nscDa[j];
			}
			else{
				nsc_chk = trace.nscDb[j];
			}
			int msec_dif = msec_cur - pdata->getMsecScp(nsc_chk);
			CalcModInfo calcmod;
			int modtype = calcDifMod0515(calcmod, msec_dif);
			//--- 前に15秒単位、直前は15秒単位ではない時の処理 ---
			if (chkmod_d[k] == 2 && modtype == 0){
				int score_tmp = pdata->getScpScore(nsc_chk);
				if (score_cur <= score_tmp){
					chkmod_d[k] = 0;
				}
				else if (msec_dif <= 31 * 1000){
					// 候補から削除
					if (pdata->getScpChap(nsc_chk) >= SCP_CHAP_CDET){
						pdata->setScpChap(nsc_chk, SCP_CHAP_NONE);
					}
					// 候補から削除して後のデータを詰める
					for(int m=j; m<jmax-1; m++){
						if (k==0) trace.nscDa[m] = trace.nscDa[m+1];
						else	  trace.nscDb[m] = trace.nscDb[m+1];
					}
					if (k==0) trace.numDa--;
					else	  trace.numDb--;
					j--;
					jmax--;
				}
			}
			//--- 15/5秒単位最適箇所取得 ---
			if (modtype >= 2){					// 15秒単位
				if (chkmod_d[k] == 2){
					if (difmod_d[k] > calcmod.mod15){
						difmod_d[k] = calcmod.mod15;
					}
				}
				else if (chkmod_d[k] < 2){
					chkmod_d[k] = 2;			// 15秒単位を設定
					difmod_d[k] = calcmod.mod15;
				}
			}
			else if (modtype > 0){				// 5秒単位
				if  (chkmod_d[k] == 1){
					if (difmod_d[k] > calcmod.mod05){
						difmod_d[k] = calcmod.mod05;
					}
				}
				else if (chkmod_d[k] < 1){
					chkmod_d[k] = 1;			// 5秒単位を設定
					difmod_d[k] = calcmod.mod05;
				}
			}
		}
	}
	//--- 5/15秒単位情報から候補として追加していく ---
	//--- 継続候補位置から5/15秒単位の場合 ---
	if (chkmod_d[0] > 0){
		//--- 新候補も存在する場合
		if (trace.numDb > 0){
			//--- 継続候補の両側スコアが高ければ新候補を消去 ---
			if (scoremax_prev >= scoremax_db && score_cur >= scoremax_db){
				trace.numDb = 0;
			}
			//--- 新候補より後の継続候補のスコアが高い場合新候補を消去 ---
			else if (scoremax_db < scoremax_mid){
				trace.numDb = 0;
			}
		}
		//--- 新候補が存在して距離も近ければ新候補側にセット ---
		if (chkmod_d[1] > 0 && trace.numDb > 0 && difmod_d[0] > difmod_d[1]){
			chkmod_d[0] = 0;
		}
		//--- 継続候補として処理 ---
		else{
			chkmod_d[1] = 0;
			//--- 一杯の場合は１つ消す ---
			if (trace.numDa >= SIZE_SETFORM_KEEP){
				setChapFixed(trace);
				for(int j=1; j<SIZE_SETFORM_KEEP; j++){
					trace.nscDa[j-1] = trace.nscDa[j];
				}
				trace.numDa = SIZE_SETFORM_KEEP - 1;
			}
			//--- 継続候補に追加 ---
			trace.nscDa[trace.numDa] = nsc_cur;
			trace.numDa += 1;
			//--- 15秒単位であれば新候補ない状態なら推測情報構成として追加 ---
			if (chkmod_d[0] > 1 && trace.numDa > 1 && trace.numDb == 0){
				//--- 構成数少ない場合は制限 ---
				setChapFixed(trace);
			}
		}
	}
	//--- 新候補位置から5/15秒単位の場合 ---
	if (chkmod_d[1] > 0){
		//--- 念のため一杯ではないこと確認して新候補に追加 ---
		if (trace.numDb < SIZE_SETFORM_KEEP){
			trace.nscDb[trace.numDb] = nsc_cur;
			trace.numDb += 1;
		}
		//--- 15秒単位であれば継続候補に移し推測情報構成として追加 ---
		if (chkmod_d[1] > 1){
			bool flag_change = true;
			//--- 新候補全体のスコアが継続候補最後のスコアより小さい時は更新中断 ---
			if (trace.numDa > 0){
				int nsc_k0 = trace.nscDa[trace.numDa-1];
				int score_k0 = pdata->getScpScore(nsc_k0);
				int score_k1 = 0;
				for(int j=0; j<trace.numDb; j++){
					score_k1 += pdata->getScpScore(trace.nscDb[j]);
				}
				if (score_k0 >= score_k1){
					int msec_dif = msec_cur - pdata->getMsecScp(nsc_k0);
					if (msec_dif < 60*1000){	// 継続候補最後からまだ１分離れてない
						flag_change = false;
					}
				}
			}
			if (flag_change == true){
				for(int j=0; j<trace.numDb; j++){
					int nsc_chk = trace.nscDb[j];
					trace.nscDa[j] = nsc_chk;
				}
				trace.numDa = trace.numDb;
				trace.numDb = 0;
				setChapFixed(trace);
			}
		}
	}
	//--- 継続候補と新候補位置どちらからも5/15秒単位でない場合 ---
	if (chkmod_d[0] == 0 && chkmod_d[1] == 0){
		//--- 継続候補がなければ無条件で追加 ---
		if (trace.numDa == 0){
			trace.nscDa[0] = nsc_cur;
			trace.numDa += 1;
		}
		//--- 継続候補の両側どちらかが追加データよりスコアが小さければ ---
		else if (scoremax_prev < score_cur || scoremax_post < score_cur){
			//--- 新候補がなければ無条件で追加 ---
			if (trace.numDb == 0){
				trace.nscDb[0] = nsc_cur;
				trace.numDb = 1;
			}
			//--- 新候補よりスコアが大きければ追加 ---
			else if (scoremax_db < score_cur){
				trace.nscDb[0] = nsc_cur;
				trace.numDb = 1;
			}
		}
	}
//printf("(d:%d,A:",nsc_cur);
//for(int t1=0; t1<trace.numDa; t1++){
//	printf("%d,", trace.nscDa[t1]);
//}
//printf("B:");
//for(int t1=0; t1<trace.numDb; t1++){
//	printf("%d,", trace.nscDb[t1]);
//}
}

//---------------------------------------------------------------------
// 対象位置以降のの最大スコア取得
// 入力：
//   nsc_target: 対象位置番号
// 出力：
//   返り値: 対象位置以降の最大スコア
//---------------------------------------------------------------------
int JlsAutoReform::setChapMaxscore(Nsc nsc_target){
	int num_scpos = pdata->sizeDataScp();
	int max_score = 0;
	int i = nsc_target + 1;					// 対象位置の次から検索開始
	bool over = false;
	if (i >= num_scpos-1 || i < 0){
		over = true;
	}
	while( over == false ){
		//--- 検索位置と対象位置の距離取得 ---
		Msec msec_target = pdata->getMsecScp(nsc_target);
		Msec msec_i = pdata->getMsecScp(i);
		Msec msec_dif = msec_i - msec_target;
		//--- 5/15秒単位の位置であればスコア読み込み ---
		CalcModInfo calcmod;
		int modtype = calcDifMod0515(calcmod, msec_dif);
		if (modtype > 0){
			int score = setChapGetDistanceScore(i, msec_dif);
			if (max_score < score){
				max_score = score;
			}
		}
		//--- 180秒以上離れている時は終了 ---
		if (calcmod.mod15 < 0 || i >= num_scpos-1){
			over = true;
		}
		i ++;
	}
	return max_score;
}

//---------------------------------------------------------------------
// 距離によるスコア修正
// 入力：
//   nsc:      対象位置番号
//   msec_dif: 対象位置までの距離ミリ秒
// 出力：
//   返り値: 対象位置の距離補正済みスコア
//---------------------------------------------------------------------
int JlsAutoReform::setChapGetDistanceScore(Nsc nsc, Msec msec_dif){
	int score = pdata->getScpScore(nsc);;
	if (msec_dif > 122 * 1000){			// 2分超えて離れているスコアは半減扱い
		score = score / 2;
	}
	else if (msec_dif > 62 * 1000){		// 1分超えて離れているスコアは減少扱い
		score = score * 3 / 4;
	}
	return score;
}

//---------------------------------------------------------------------
// 確定可能な推測構成を確定
// 入力：
//   trace
//     num_da:  保持データ数（継続候補）
//     num_db:  保持データ数（新候補）
//     nsc_da:  保持データの継続候補位置（サイズ=SIZE_SETFORM_KEEP）
//     nsc_db:  保持データの新候補位置（サイズ=SIZE_SETFORM_KEEP）
// 出力：
//   pdata(chap)  :  対象位置を構成候補として設定
//---------------------------------------------------------------------
bool JlsAutoReform::setChapFixed(TraceChap &trace){
	bool flag_strict = setChapFixedLimit(trace);
	if (flag_strict == false){
		//--- 未確定位置 ---
		Nsc nsc_wait = -1;
		if (trace.numDb > 0){
			nsc_wait = trace.nscDb[0];
		}
		for(int j=0; j<trace.numDa; j++){
			Nsc nsc_chap = trace.nscDa[j];
			if (nsc_chap < nsc_wait || nsc_wait < 0){
				ScpChapType chap_dst = pdata->getScpChap(nsc_chap);
				if (chap_dst < SCP_CHAP_CDET){
					pdata->setScpChap(nsc_chap, SCP_CHAP_CDET);
				}
			}
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
// 数が少ない構成では構成を制限する
// 入力：
//   trace
//     num_da: 保持データ数（継続候補）
//     nsc_da: 保持データの継続候補位置（サイズ=SIZE_SETFORM_KEEP）
// 出力：
//   返り値:  false=制限なし  true=構成は無効
//---------------------------------------------------------------------
bool JlsAutoReform::setChapFixedLimit(TraceChap &trace){
	if (trace.numDa <= 1){
		return true;
	}

	bool flag_remove = false;
	int sum_s15 = 0;
	int sum_s05 = 0;
	for(int i=0; i<trace.numDa-1; i++){
		Nsc nsc_from  = trace.nscDa[i];
		Nsc nsc_to    = trace.nscDa[i+1];
		Msec msec_from = pdata->getMsecScp(nsc_from);
		Msec msec_to   = pdata->getMsecScp(nsc_to);
		//--- 間隔 ---
		Sec sec_dif   = (msec_to - msec_from + 500) / 1000;
		Sec sec_dif05 = ((sec_dif + 2) / 5) * 5;
		Msec msec_new  = msec_from + (sec_dif05 * 1000);
		Msec msecdif   = abs(msec_to - msec_new);
		//--- 状態 ---
		bool flag_still = false;
		if ((pdata->getScpStill(nsc_from)) ||
			(pdata->getScpStill(nsc_to))){
			flag_still = true;
		}
		bool flag_both_still = false;
		if ((pdata->getScpStill(nsc_from)) &&
			(pdata->getScpStill(nsc_to))){
			flag_both_still = true;
		}
		bool flag_nomute      = false;
		bool flag_long_nomute = false;
		if (nsc_to - nsc_from == 1){
			flag_nomute      = true;
			flag_long_nomute = true;
		}
		else{
			bool flag_sub = true;
			for(int k=nsc_from+1; k<nsc_to; k++){
				if (pdata->isSmuteSameArea(nsc_from, k) == false &&
					pdata->isSmuteSameArea(nsc_to  , k) == false){
					flag_sub = false;
				}
				else{
					int msec_k = pdata->getMsecScp(k);
					if (abs(msec_k - msec_from) > pdata->msecValLap2 ||
						abs(msec_to - msec_k  ) > pdata->msecValLap2){
						flag_sub = false;
					}
				}
			}
			if (flag_sub == true){
				flag_long_nomute = true;
			}
		}
		//--- 設定で無音なし扱いにする場合 ---
		bool calcel_cntsc = ( pdata->getConfig(ConfigVarType::cancelCntSc) != 0 )? true : false;
		if (calcel_cntsc){
			flag_nomute = true;
		}
		//--- 30秒以下構成の制約 ---
		if (sec_dif05 <= 30 && sec_dif05 > 0){
			//--- 動きあり---
			if (flag_still == false){
				if (msecdif <= pdata->msecValExact){	// 正確な秒数構成
					sum_s05 ++;
					if (flag_nomute){					// 間に無音がない構成
						sum_s05 ++;
					}
				}
			}
		}
		//--- 15秒倍数構成の制約 ---
		if ((sec_dif05 % 15 == 0) && sec_dif05 > 0){
			//--- 60秒以上の15秒倍数構成 ---
			if ((sec_dif05 % 30 == 0) && (sec_dif05 >= 60)){
				if (flag_long_nomute){						// 間に無音がない構成
					sum_s15 += 6;
				}
				else if (msecdif <= pdata->msecValExact && flag_still == false){
					sum_s15 += 6;
				}
			}
			//--- 動きあり ---
			if (flag_still == false){
				sum_s15 ++;
			}
			else if (flag_nomute){		// 動きなくても隣接なら追加
				sum_s15 ++;
			}
			//--- 位置ずれ ---
			if (msecdif <= pdata->msecValNear3){
				sum_s15 ++;
			}
			else if (flag_nomute){		// 位置ずれあっても隣接なら追加
				sum_s15 ++;
			}
			if (flag_both_still == false){
				if (msecdif <= pdata->msecValNear1){
					sum_s15 ++;
				}
			}
		}
	}
	if (sum_s05 < 2 && sum_s15 < 6){
		flag_remove = true;
	}
	return flag_remove;
}


//---------------------------------------------------------------------
// 隣接した候補があれば片側を消す
//---------------------------------------------------------------------
void JlsAutoReform::setChapThinOverlap(RangeFixMsec fixscope){
	//--- 固定箇所取得 ---
	Nsc nsc_fix_st = -1;
	Nsc nsc_fix_ed = -1;
	if (fixscope.fixSt){
		nsc_fix_st = pdata->getNscFromMsecAll(fixscope.st);
	}
	if (fixscope.fixEd){
		nsc_fix_ed = pdata->getNscFromMsecAll(fixscope.ed);
	}
	//--- 範囲内の候補構成を検索 ---
	Nsc nsc_cur  = 0;
	Msec msec_cur = 0;
	bool over = false;
	while( over == false ){
		Nsc nsc_last  = nsc_cur;
		Msec msec_last = msec_cur;
		nsc_cur = pdata->getNscNextScpChap(nsc_last, SCP_CHAP_CDET);
		msec_cur = pdata->getMsecScp(nsc_cur);
		if (nsc_cur < 0){
			over = true;
		}
		else if (msec_cur > fixscope.ed && fixscope.ed >= 0){
			over = true;
		}
		else if (nsc_last <= 0){
		}
		//--- 範囲内の候補位置であれば処理 ---
		else if ((msec_cur  >= fixscope.st || fixscope.st < 0) &&
				 (msec_last <= fixscope.ed || fixscope.ed < 0)){
			int msecdif_cur = abs(msec_cur - msec_last);
			//--- 隣接している場合は片側を外す ---
			if (msecdif_cur < pdata->msecValLap2){
				int score_last = pdata->getScpScore(nsc_last);
				int score_cur  = pdata->getScpScore(nsc_cur);
				bool flag_fix_last = (nsc_last == nsc_fix_st || nsc_last == nsc_fix_ed)? true : false;
				bool flag_fix_cur  = (nsc_cur  == nsc_fix_st || nsc_cur  == nsc_fix_ed)? true : false;
				bool flag_score_last = (score_last >= score_cur)? true : false;
				if ((flag_fix_last || flag_score_last) && !flag_fix_cur){
					pdata->setScpChap(nsc_cur, SCP_CHAP_CPOSQ);
				}
				if ((flag_fix_cur || !flag_score_last) && !flag_fix_last){
					pdata->setScpChap(nsc_last, SCP_CHAP_CPOSQ);
				}
			}
		}
	}
}



//=====================================================================
// 対象位置のスコアを取得
//=====================================================================

//---------------------------------------------------------------------
// 対象位置の指定範囲内でのスコア取得
// 入力：
//   msec_target: 対象位置のファイル先頭からのミリ秒数
//   scope
//     st:        スコア計算をする開始位置
//     ed:        スコア計算をする終了位置
// 出力：
//   返り値: 対象位置の算出結果スコア
//---------------------------------------------------------------------
int JlsAutoReform::getScore(Msec msec_target, RangeMsec scope){
	int score_all = 0;
	int score_m3 = 0;
	int score_m2 = 0;
	int score_m1 = 0;
	int score_mt = 0;
	int score_sec5 = 0;
	Nsc nsc_cur = 0;
	//--- target位置から前後３分を検索 ---
	for(int i=-16; i<=16; i++){
		//--- 検索位置設定 ---
		Sec sec_width;
		if (i < -6){					// -180秒 <= sec_width < -30秒
			sec_width = i * 15 + 60;
		}
		else if (i <= 6){				// -30秒 <= sec_width <= 30秒
			sec_width = i * 5;
		}
		else{							// 30秒 < sec_width <= 180秒
			sec_width = i * 15 - 60;
		}
		Msec msec_cmp = msec_target + sec_width * 1000;		// 次に比較する位置

		// --- スコア追加 ---
		if ((scope.st <= msec_cmp + pdata->msecValLap2 || scope.st < 0) &&
			(scope.ed >= msec_cmp - pdata->msecValLap2 || scope.ed < 0) &&
			sec_width != 0){

			//--- 一番近くの構成位置検索 ---
			int msec_dist_min = getScoreDist(nsc_cur, msec_cmp);
			//--- 無音確認 ---
			bool smute_det;
			bool still_det;
			if ((scope.st < 0 && sec_width < 0) ||
				(scope.ed < 0 && sec_width > 0)){	// 範囲なしでは無音チェックなし
				smute_det = 0;
				still_det = 0;
			}
			else{
				smute_det = pdata->isSmuteFromMsec(msec_cmp);
				still_det = pdata->isStillFromMsec(msec_cmp);
			}
			//--- スコア追加 ---
			int score_tmp = getScoreTarget(msec_dist_min, sec_width, smute_det, still_det);
			if (abs(sec_width) > 120){			// 120秒超え
				score_m3 += score_tmp;
			}
			else if (abs(sec_width) > 60){		// 60秒超え - 120秒
				score_m2 += score_tmp;
			}
			else if (abs(sec_width) % 15 == 0){	// 60秒まで15秒単位
				score_m1 += score_tmp;
				if (abs(sec_width) <= 30){		// 30秒まで15秒単位
					score_mt += score_tmp;
				}
			}
			else{								// 5秒単位
				score_sec5 += score_tmp;
			}
		}
	}
	//--- 30秒以内のスコアが大きく1分以内の大部分を占める場合は１分超えの上限を制限 ---
	if (score_mt >= 50 && score_m1 <= score_mt + 50/2){
		if (score_m2 > (score_m1 - score_mt)){
			score_m2 = (score_m1 - score_mt);
		}
	}
	//--- 2分超えは、1分超えより1分前のスコアが大きければ上限を制限 ---
	if (score_m3 > score_m2 / 2 && score_m2 < score_m1){
		score_m3 = score_m2 / 2;
	}
	int score_ma = score_m1 + score_m2 + score_m3;
	//--- スコアが大きい時は15秒単位のみにして5秒単位は含めない ---
	if (score_ma + score_sec5 >= 50){
		if (score_ma >= 50){
			score_all = score_ma;
		}
		else{
			score_all = 50;
		}
	}
	else{
		score_all = score_ma + score_sec5;
	}
	return score_all;
}
int JlsAutoReform::getScore(Msec msec_target, RangeWideMsec range_wide_scope){
	RangeMsec scope = {range_wide_scope.st.early, range_wide_scope.ed.late};
	return getScore(msec_target, scope);
};

//---------------------------------------------------------------------
// 対象位置に一番近い無音SCの距離取得
// 入力：
//   msec_cmp:  比較対象位置のファイル先頭からのミリ秒数
// 入出力：
//   nsc_cur: 検索を開始する位置番号（次の検索短縮のため値更新）
// 出力：
//   返り値: -1=対象となる構成なし 0以上=距離msec
//---------------------------------------------------------------------
int JlsAutoReform::getScoreDist(Nsc &nsc_cur, Msec msec_cmp){
	int num_scpos = pdata->sizeDataScp();

	Nsc  nsc_cand = -1;
	Msec msec_dist_min = -1;
	Msec msec_val_valid = pdata->msecValNear3;
	bool over = 0;
	if (nsc_cur >= num_scpos-1){				// 検索最後まで完了時は終了
		over = true;
	}
	while( over == false ){
		Msec msec_cur = pdata->getMsecScp(nsc_cur);
		Msec msec_dist_cur = abs(msec_cur - msec_cmp);	// 比較位置と現在位置の差分
		if (msec_dist_cur <= msec_val_valid){			// 差分が有効範囲内
			//--- 一番近い位置のみ記憶 ---
			if (nsc_cand < 0 || msec_dist_cur < msec_dist_min){
				nsc_cand = nsc_cur;
				msec_dist_min = msec_dist_cur;
			}
		}
		//--- 次の位置検索 ---
		if (msec_cur > msec_cmp + msec_val_valid){	// 現在の有効範囲外まで終了
			over = true;
		}
		else{
			nsc_cur ++;								// 次の位置
			if (nsc_cur >= num_scpos-1){				// 最終位置以降
				over = true;
			}
		}
	}
	return msec_dist_min;
}


//---------------------------------------------------------------------
// 距離別の単体スコア取得
// 入力：
//	msec_dist				// 対象までの距離差（ミリ秒）
//	sec_width				// 検出している距離（秒）
//	smute_det				// 無音情報（1=無音）
//	still_det				// 動きなし情報（1=動きなし）
// 出力：
//   返り値: 対象位置の算出結果スコア
//---------------------------------------------------------------------
int JlsAutoReform::getScoreTarget(Msec msec_dist, int sec_width, bool smute_det, bool still_det){
	int score = 0;
	int flag_d15 = 0;
	//--- 判定箇所 ---
	int abs_sec = abs(sec_width);
	if (abs_sec % 15 == 0){
		flag_d15 = 1;
	}
	//--- 距離別に分類 ---
	int ndist = 0;
	if (msec_dist < 0){
		ndist = -1;
	}
	else if (msec_dist <= pdata->msecValExact){
		ndist = 3;
	}
	else if (msec_dist <= pdata->msecValNear1){
		ndist = 2;
	}
	else if (msec_dist <= pdata->msecValNear2){
		ndist = 1;
	}
	//--- スコア付け ---
	if (abs_sec == 0){							// 基点
		score = 0;
	}
	else if (abs_sec <= 60){					// １分以内
		if (flag_d15 > 0){					// １分以内15秒単位
			if (ndist >= 0 || smute_det){
				score = 21;
			}
			if (ndist > 0){
				score += ndist * 3;
				if (ndist >= 3) score += ndist;	// 位置正確な時はは追加
			}
		}
		else{									// １分以内5秒単位
			if (ndist > 0){
				if (still_det){					// 動きなしなら最小
					ndist = 1;
				}
				score = 7;
				score += ndist * 3;
			}
		}
	}
	else{
		if (ndist >= 0 || smute_det){
			if (abs_sec <= 120){					// ２分以内
				score = 15;
			}
			else{									// ２分超え
				score = 3;
			}
			score += ndist * 3;
		}
	}
	return score;
}



//=====================================================================
// 共通処理の構成変更
//=====================================================================

//---------------------------------------------------------------------
// 無音が多い場合や境界にシーンチェンジがない場合やめる候補
// 出力:
//   返り値: 0=通常  1=無音が多い  2=無音が多く境界にシーンチェンジもない +4:シーンチェンジなく無音多い扱い
//---------------------------------------------------------------------
int JlsAutoReform::checkMuteMany(RangeNsc rnsc){
	int type_mute = 0;
	if (rnsc.st < rnsc.ed && rnsc.st >= 0 && rnsc.ed >= 0){
		{
			int count_smute = 0;
			if (abs(rnsc.ed - rnsc.st) > 1){
				int lpcnt = abs(rnsc.ed - rnsc.st) - 1;
				Nsc nsc_tmp = rnsc.st;
				for(int i=0; i<lpcnt; i++){
					nsc_tmp ++;
					if ((pdata->isSmuteSameArea(nsc_tmp, rnsc.st) == false) &&
						(pdata->isSmuteSameArea(nsc_tmp, rnsc.ed) == false) &&
						(pdata->isSmuteSameArea(nsc_tmp, nsc_tmp-1) == false)){
						count_smute ++;
					}
				}
				//--- 無音の判断を無効にする設定の場合 ---
				bool calcel_cntsc = ( pdata->getConfig(ConfigVarType::cancelCntSc) != 0 )? true : false;
				if (calcel_cntsc){
					count_smute = 0;
				}
			}
			RangeMsec rmsec = getRangeMsec(rnsc);
			Msec msec_dif = rmsec.ed - rmsec.st;
			if ((count_smute > 3 && msec_dif < 14500) ||	// 間に無音が特に多い場合は無条件で外す
				(count_smute > 4 && msec_dif < 29500) ||
				(count_smute > 6 && msec_dif < 59500) ||
				(count_smute > 8)){
				type_mute = 2;
			}
			else if ((count_smute > 1 && msec_dif < 14500) ||	// 間に無音が多い場合は外す
				(count_smute > 2 && msec_dif < 29500) ||
				(count_smute > 4 && msec_dif < 59500) ||
				(count_smute > 6)){
				type_mute = 1;
			}
		}
		int type_nosc = pdata->getConfigAction(ConfigActType::MuteNoSc);	// シーンチェンジなし無音の使用キャンセル
		if (type_mute > 0 && type_nosc == 1){
			if (pdata->getScpStill(rnsc.st) || pdata->getScpStill(rnsc.ed)){
				type_mute += 4;
			}
		}
	}
	return type_mute;
}

//---------------------------------------------------------------------
// １構成だけ追加処理
//---------------------------------------------------------------------
void JlsAutoReform::addCommonComponentOne(Nsc nsc_target, Nsc nsc_side, bool enable_short){
	SearchDirType dr = (nsc_target < nsc_side)? SEARCH_DIR_NEXT : SEARCH_DIR_PREV;
	//--- 隣接構成なら実行せず終了 ---
	if (abs(nsc_target - nsc_side) <= 1) return;
	//--- 設定値取得 ---
	Msec msec_mgn = pdata->getConfig(ConfigVarType::msecMgnCmDivide);
	Msec msec_target = pdata->getMsecScp(nsc_target);
	Msec msec_side   = pdata->getMsecScp(nsc_side);
	bool logomode = true;
	{
		Nsc nsc_tmp = (dr == SEARCH_DIR_NEXT)? nsc_side : nsc_target;
		ScpArType arstat_tmp = pdata->getScpArstat(nsc_tmp);
		if (jlsd::isScpArTypeLogo(arstat_tmp) == false){
			logomode = false;
		}
	}
	{
		Nsc nsc_cur = nsc_target;
		bool cont = true;
		while(cont){
			nsc_cur = pdata->getNscDirScpChap(nsc_cur, dr, SCP_CHAP_NONE);
			if (nsc_cur < 0 || nsc_cur == nsc_side){
				cont = false;
			}
			else{
				Msec msec_cur = pdata->getMsecScp(nsc_cur);
				CalcDifInfo calc_target;
				CalcDifInfo calc_side;
				int type_target = calcDifSelect(calc_target, msec_target, msec_cur);
				int type_side   = calcDifSelect(calc_side  , msec_side  , msec_cur);
				//--- 距離による補正 ---
				if (calc_target.sec < 30 && enable_short == false){
					type_target = 0;
				}
				else if (calc_target.sec >= 122){		// 検索範囲を超えたらやめる
					cont = false;
				}
				//--- 秒単位区切りで誤差が小さければ構成追加 ---
				if (type_target > 0 && (type_side > 0 || calc_side.sec > 30)){
					if (calc_target.gap <= msec_mgn){
						//--- 構成区切り追加処理 ---
						ScpArType arstat_cur = (logomode)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
						pdata->setScpChap(nsc_cur, SCP_CHAP_DINT);
						pdata->setScpArstat(nsc_cur, arstat_cur);
						cont = false;		// 終了
					}
				}
			}
		}
	}
}



//=====================================================================
// ２点間の時間と判別情報を取得
//=====================================================================

//---------------------------------------------------------------------
// ２フレーム間の秒数を取得
// 入力：
//   msec_src  : 比較元フレーム番号
//   msec_dst  : 比較先フレーム番号
// 出力：
//   calcdif
//     sgn : 比較フレーム大小（src側が大きい=-1,同じ=0,dst側が大きい=1）
//     sec : フレーム間秒数
//     gap : フレーム間秒数の誤差（ミリ秒）
// 返り値：
//   2 : 10,15,30,45,60,90,120秒
//   1 : ３秒以上２５秒以下かつ秒単位で誤差が少ない場合
//   0 : それ以外
//---------------------------------------------------------------------
int JlsAutoReform::calcDifSelect(CalcDifInfo &calcdif, Msec msec_src, Msec msec_dst){
	Msec msec_val_near1 = pdata->msecValNear1;
	Msec msec_val_near2 = pdata->msecValNear2;

	Msec msec_dif = abs(msec_dst - msec_src);
	Sec  sec_dif = pdata->cnv.getSecFromMsec(msec_dif);
	Msec gap_dif = abs(sec_dif * 1000 - msec_dif);
	int type = 0;
	if (gap_dif <= msec_val_near2 &&
		(sec_dif == 10 || sec_dif == 15 || sec_dif == 30 || sec_dif == 45 ||
		 sec_dif == 60 || sec_dif == 90 || sec_dif == 120)){
		type = 2;
	}
	else if (gap_dif <= msec_val_near1 &&
			 (sec_dif >= 3 && sec_dif <= 25)){
		type = 1;
	}
	// 結果格納
	if (msec_src > msec_dst){
		calcdif.sgn = -1;
	}
	else if (msec_src < msec_dst){
		calcdif.sgn = 1;
	}
	else{
		calcdif.sgn = 0;
	}
	calcdif.sec = sec_dif;
	calcdif.gap = gap_dif;

	return type;
}

//---------------------------------------------------------------------
// 対象距離が5/15秒単位か取得
// 入力：
//   msec_dif:  対象距離
// 出力：
//   返り値: 0=5/15秒単位ではない 1=5秒単位  2=15秒単位  3=5/15秒単位
//   calcmod.mod15: 15秒単位からの誤差msec出力（-1の時は範囲外）
//   calcmod.mod05: 05秒単位からの誤差msec出力（-1の時は範囲外）
//---------------------------------------------------------------------
int JlsAutoReform::calcDifMod0515(CalcModInfo &calcmod, Msec msec_dif){
	int ret = 0;
	Msec calc_mod15 = -1;
	Msec calc_mod05 = -1;
	//--- 180秒以内は15秒単位を確認 ---
	if (msec_dif > pdata->msecValLap2 && 
		msec_dif <= 180 * 1000 + pdata->msecValLap2){
		calc_mod15 = msec_dif % (15 * 1000);
		if (calc_mod15 > (15 * 1000)/2){
			calc_mod15 = (15 * 1000) - calc_mod15;
		}
		if (calc_mod15 <= pdata->msecValNear3){
			ret += 2;
		}
	}
	//--- 30秒以内は5秒単位を確認 ---
	if (msec_dif > pdata->msecValLap2 && 
		msec_dif <= 30 * 1000 + pdata->msecValLap2){
		calc_mod05 = msec_dif % (5 * 1000);
		if (calc_mod05 > (5 * 1000)/2){
			calc_mod05 = (5 * 1000) - calc_mod05;
		}
		if (calc_mod05 <= pdata->msecValNear1){
			ret += 1;
		}
	}
	//--- 結果格納 ---
	calcmod.mod15 = calc_mod15;
	calcmod.mod05 = calc_mod05;
	return ret;
}

//---------------------------------------------------------------------
// 範囲位置を取得
//---------------------------------------------------------------------
RangeMsec JlsAutoReform::getRangeMsec(RangeNsc range_nsc){
	return pdata->getRangeMsecFromRangeNsc(range_nsc);
}

//---------------------------------------------------------------------
// ミリ秒入力が15秒単位か判断
//---------------------------------------------------------------------
bool JlsAutoReform::isCmLengthMsec(Msec msec_target){
	Sec sec_target = pdata->cnv.getSecFromMsec(msec_target);
	return (sec_target % 15 == 0)? true : false;
}

//---------------------------------------------------------------------
// 期間がロゴ認識期間になっているか確認
//---------------------------------------------------------------------
bool JlsAutoReform::isLengthLogoLevel(Msec msec_target, FormLogoLevelExt extype){
	//--- 極端に短い時は無効 ---
	if (msec_target < pdata->msecValLap2) return false;
	//--- ロゴレベルにより判別 ---
	bool match_len = false;
	int lvmid = (extype.mid5s)? CONFIG_LOGO_LEVEL_USE_MIDL : CONFIG_LOGO_LEVEL_USE_MIDH;
	int lvlogo = pdata->getLevelUseLogo();
	if (lvlogo == CONFIG_LOGO_LEVEL_USE_MAX){
		match_len = true;
	}
	else{
		CalcModInfo calcmod;
		int typemod = calcDifMod0515(calcmod, msec_target);
		CalcDifInfo calcdif;
		calcDifSelect(calcdif, 0, msec_target);
		if (lvlogo >= CONFIG_LOGO_LEVEL_USE_HIGH){		// 秒数単位確認
			if (calcdif.gap <= pdata->msecValNear1) match_len = true;
		}
		if (lvlogo >= lvmid){							// 5秒単位確認
			if (typemod >= 1) match_len = true;
		}
		if (typemod >= 2) match_len = true;				// 15秒単位確認
		//--- 長期間の構成も許可する場合 ---
		if (extype.en_long){
			if (calcdif.sec % 30 == 0 && calcdif.sec > 180 && calcdif.sec <= 600){
				match_len = true;						// 長期間の30秒単位構成
			}
		}
		//--- 長時間の単位構成も見る場合 ---
		if (extype.longmod){
			if (lvlogo >= lvmid){						// 5秒単位確認
				if (calcdif.sec % 5 == 0 && calcdif.gap <= pdata->msecValNear1) match_len = true;
			}
			if (calcdif.sec % 15 == 0 && calcdif.gap <= pdata->msecValNear1) match_len = true;
		}
	}
	
	return match_len;
}

