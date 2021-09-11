//
// join_logo_scp データ構成初期補正
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsReformData.hpp"
#include "JlsDataset.hpp"


//---------------------------------------------------------------------
// 構築
//---------------------------------------------------------------------
JlsReformData::JlsReformData(JlsDataset *pdata){
	//--- 関数ポインタ作成 ---
	this->pdata = pdata;
}

//---------------------------------------------------------------------
// 入力データからの微調整を行う処理
//---------------------------------------------------------------------
void JlsReformData::adjustData(){
	pdata->setFlagSetupAdj( true );
	adjustScpos();
	//--- ロゴマージン調整 ---
	if ( (pdata->extOpt.nLgExact & 0x01) == 0 ){
		detectCutmrg();
		updateCutmrg();
	}
	adjustLogo();
}



//---------------------------------------------------------------------
// CutMrgIn/CutMrgOut を設定
//---------------------------------------------------------------------
void JlsReformData::updateCutmrg(){
	int num_logo  = pdata->sizeDataLogo();

	for(int i=0; i<num_logo; i++){
		DataLogoRecord dtlogo;
		pdata->getRecordLogo(dtlogo, i);

		// rise設定
		{
			int intl = dtlogo.intl_rise;
			int msec_cutin = pdata->extOpt.msecCutIn;
			if (intl > 0){
				Msec msec_org = dtlogo.org_rise;
				Msec msec_nxt = pdata->cnv.getMsecAdjustFrmFromMsec(msec_org, 1);
				msec_cutin += (msec_nxt - msec_org) / 2;
			}
			int n1 = dtlogo.org_rise   - msec_cutin;
			int n2 = dtlogo.org_rise_l - msec_cutin;
			int n3 = dtlogo.org_rise_r - msec_cutin;
			if (n1 < 0){
				n1 = 0;
			}
			if (n2 < 0){
				n2 = 0;
			}
			if (n3 < 0){
				n3 = 0;
			}
			//--- シーンチェンジ位置により更新判断 ---
			int nsc_base = pdata->getNscFromMsecAll(dtlogo.org_rise);
			int msec_base = pdata->getMsecScp(nsc_base);
			if (nsc_base > 0){
				if (pdata->getScpStill(nsc_base)){		// 動きなし位置は除く
					nsc_base = -1;
				}
			}
			if (pdata->extOpt.wideCutIn == 0 ||
				(pdata->extOpt.wideCutIn == 2 && msec_cutin > pdata->msecValNear3) ||
				nsc_base < 0 ||
				dtlogo.org_rise_l - pdata->msecValExact > msec_base ||
				dtlogo.org_rise_r + pdata->msecValExact < msec_base ||
				(dtlogo.org_rise - msec_base >= (dtlogo.org_rise - n1) / 2)){
				//--- 更新 ---
				dtlogo.rise   = pdata->cnv.getMsecAlignFromMsec( n1 );
				dtlogo.rise_l = pdata->cnv.getMsecAlignFromMsec( n2 );
				dtlogo.rise_r = pdata->cnv.getMsecAlignFromMsec( n3 );
			}
			else{
				dtlogo.rise   = dtlogo.org_rise;
				dtlogo.rise_l = dtlogo.org_rise_l;
				dtlogo.rise_r = dtlogo.org_rise_r;
			}
		}

		// fall設定
		{
			int intl = dtlogo.intl_fall;
			int msec_cutout = pdata->extOpt.msecCutOut;
			if (intl > 0){
				Msec msec_org = dtlogo.org_fall;
				Msec msec_nxt = pdata->cnv.getMsecAdjustFrmFromMsec(msec_org, -1);
				msec_cutout += (msec_nxt - msec_org) / 2;
			}
			int n1 = dtlogo.org_fall   + msec_cutout;
			int n2 = dtlogo.org_fall_l + msec_cutout;
			int n3 = dtlogo.org_fall_r + msec_cutout;
			if (n1 < 0){
				n1 = 0;
			}
			if (n2 < 0){
				n2 = 0;
			}
			if (n3 < 0){
				n3 = 0;
			}
			int msec_max = pdata->getMsecTotalMax();
			if (n1 > msec_max){
				n1 = msec_max;
			}
			if (n2 > msec_max){
				n2 = msec_max;
			}
			if (n3 > msec_max){
				n3 = msec_max;
			}
			//--- シーンチェンジ位置により更新判断 ---
			int nsc_base  = pdata->getNscFromMsecAll(dtlogo.org_fall);
			int msec_base = pdata->getMsecScpBk(nsc_base);
			if (nsc_base > 0){
				if (pdata->getScpStill(nsc_base)){		// 動きなし位置は除く
					nsc_base = -1;
				}
			}
			if (pdata->extOpt.wideCutOut == 0 ||
				(pdata->extOpt.wideCutOut == 2 && msec_cutout > pdata->msecValNear3) ||
				nsc_base < 0 ||
				dtlogo.org_fall_l - pdata->msecValExact > msec_base ||
				dtlogo.org_fall_r + pdata->msecValExact < msec_base ||
				(msec_base - dtlogo.org_fall >= (n1 - dtlogo.org_fall) / 2)){
				//--- 更新 ---
				dtlogo.fall   = pdata->cnv.getMsecAlignFromMsec( n1 );
				dtlogo.fall_l = pdata->cnv.getMsecAlignFromMsec( n2 );
				dtlogo.fall_r = pdata->cnv.getMsecAlignFromMsec( n3 );
			}
			else{
				dtlogo.fall   = dtlogo.org_fall;
				dtlogo.fall_l = dtlogo.org_fall_l;
				dtlogo.fall_r = dtlogo.org_fall_r;
			}
		}
		pdata->setRecordLogo(dtlogo, i);
	}
}


//---------------------------------------------------------------------
// 無音シーンチェンジの候補選別
//---------------------------------------------------------------------
void JlsReformData::adjustScpos(){
	int num_scpos = pdata->sizeDataScp();
	vector<int> nd5_bsum(num_scpos, 0);
	vector<int> nd5_sum(num_scpos, 0);
	vector<int> nd5_nxt(num_scpos, -1);
	vector<int> nd15_flag(num_scpos, 0);

	// ５の倍数秒のシーンチェンジ間隔場所を探索
	for(int i=0; i<num_scpos; i++){
		int msec_src = pdata->getMsecScp(i);
		int sec_last = 0;
		if (i > 0 && i < num_scpos-1){			// 先頭と最後は除く
			for(int j=1; j<num_scpos-1; j++){		// 先頭と最後は除く
				int msec_dst = pdata->getMsecScp(j);
				CalcDifInfo calc;
				if (calcDifFunc(calc, msec_src, msec_dst) > 0){
					if (calc.sgn < 0){
						if (calc.sec % 5 == 0){
							nd5_bsum[i] ++;
						}
						if (calc.sec % 15 == 0){
							nd15_flag[i] = 1;
						}
					}
					else if (calc.sgn > 0){
						if (calc.sec % 5 == 0){
							if (nd5_sum[i] == 0){
								nd5_nxt[i] = j;
							}
							if (sec_last != calc.sec){
								nd5_sum[i] ++;
								sec_last = calc.sec;
							}
						}
						if (calc.sec % 15 == 0){
							nd15_flag[i] = 1;
						}
					}
				}
			}
		}
		if (nd5_bsum[i] > 0 || nd5_sum[i] > 0){
			pdata->setScpStatpos(i, SCP_PRIOR_LV1);
		}
		else{
			pdata->setScpStatpos(i, SCP_PRIOR_NONE);
		}
	}
	// 15の倍数のシーンチェンジ間隔がある場所に関係あるかチェック
	for(int i=0; i<num_scpos; i++){
		if (pdata->getScpStatpos(i) > SCP_PRIOR_NONE){
			int flag_nd15 = 0;
			// 関係性をチェック
			int j = i;
			while(j >= 1 && j < num_scpos-1 && flag_nd15 == 0){		// 先頭と最後は除く
				if (nd15_flag[j] > 0){		// 15の倍数だった場合フラグを立てる
					flag_nd15 = 1;
				}
				j = nd5_nxt[j];
			}
			// 関係性があった場合のセット
			if (flag_nd15 > 0){
				j = i;
				while(j >= 0 && j < num_scpos){
					nd15_flag[j] = 1;
					j = nd5_nxt[j];
				}
			}
		}
	}
	for(int i=0; i<num_scpos; i++){
		if (nd15_flag[i] == 0){			// 15の倍数に関連なければ同期ステータスを解除
			pdata->setScpStatpos(i, SCP_PRIOR_NONE);
		}
	}
//	for(i=0; i<num_scpos; i++){
//		printf("(%ld:%d,%d,%d)", plogo->frm_scpos_s[i], nd15_flag[i], nd5_sum[i], nd5_bsum[i]);
//	}

	// ２５フレーム以内に５の倍数秒間隔シーンチェンジが重なる場合、片方のみにする
	{
		int ncmp_i = -1;
		int ncmp_msec = 0;
		for(int i=1; i<num_scpos-1; i++){		// 先頭と最後は除く
			int msec_src = pdata->getMsecScp(i);
			if (msec_src - ncmp_msec >= 870 && ncmp_i >= 0){
				ncmp_i = -1;
			}
			if (pdata->getScpStatpos(i) > SCP_PRIOR_NONE){
				if (ncmp_i < 0){
					ncmp_i = i;
					ncmp_msec = msec_src;
				}
				else{
					int count_src = 0;
					int count_cmp = 0;
					for(int j=1; j<num_scpos-1; j++){		// 先頭と最後以外で比較
						if (pdata->getScpStatpos(j) > SCP_PRIOR_NONE){
							int msec_dst = pdata->getMsecScp(j);
							CalcDifInfo calc1;
							CalcDifInfo calc2;
							int flag1 = calcDifFunc(calc1, msec_src, msec_dst);
							int flag2 = calcDifFunc(calc2, ncmp_msec, msec_dst);
							if (flag1 > 0 && flag2 > 0 &&
								calc1.sec % 5 == 0 && calc1.sec == calc2.sec && calc1.sec <= 60){
								if (calc1.gap < calc2.gap){
									count_src += 3;
								}
								else if (calc1.gap > calc2.gap){
									count_cmp += 3;
								}
							}
							else if (flag1 > 0 && (flag2 == 0 || calc2.sec % 5 != 0) && calc1.sec % 5 == 0){
								if (calc1.sec <= 60){
									count_src ++;
									if (calc1.gap <= 3){
										count_src ++;
									}
								}
							}
							else if (flag2 > 0 && (flag1 == 0 || calc1.sec % 5 != 0) && calc2.sec % 5 == 0){
								if (calc2.sec <= 60){
									count_cmp ++;
									if (calc2.gap <= 3){
										count_cmp ++;
									}
								}
							}
						}
					}
//printf("(0:%d-%d,%d,%d)", frm_src,ncmp_frm,count_src,count_cmp);
					if (count_src <= 1 && count_cmp <= 1){
						// 関連フレームが何もない場合は残す
					}
					else if (count_src > count_cmp){
//printf("(1:%d,%d,%d)", ncmp_i,count_src,count_cmp);
						for(int j=0; j<ncmp_i; j++){		// 差し替え元参照を入れ替える
							if (nd5_nxt[j] == ncmp_i){
								nd5_nxt[j] = i;
							}
						}
						pdata->setScpStatpos(ncmp_i, SCP_PRIOR_DUPE);
						ncmp_i = i;
						ncmp_msec = msec_src;
					}
					else{
//printf("(2:%d,%d,%d)", i,count_src,count_cmp);
						for(int j=0; j<i; j++){
							if (nd5_nxt[j] == i){	// 差し替え元参照を入れ替える
								nd5_nxt[j] = ncmp_i;
							}
						}
						pdata->setScpStatpos(i, SCP_PRIOR_DUPE);
					}
				}
			}
		}
	}

	// 先頭から順番にシーンチェンジ位置をたどっていく
	{
		int nst = 0;
//	plogo->stat_scpos[nst] = 2;
		while( nst<num_scpos-1 ){
			// 非同期終了地点を検索
			int ned = nst+1;
			while((pdata->getScpStatpos(ned) <= SCP_PRIOR_NONE || nd5_sum[ned] == 0) && ned < num_scpos-1){
				ned ++;
			}
			// 少し先に正しい非同期終了地点がないか検索
			if (ned < num_scpos-1){
				int nc_nxt = nd5_nxt[ned];
				int i = ned + 1;
				while(i < nc_nxt){
					if (pdata->getScpStatpos(i) >= SCP_PRIOR_NONE && nd5_sum[i] > 0){
						if (nd5_sum[i] > nd5_sum[ned]){		// 5の倍数間隔が多ければ非同期終了地点を変更
							ned = i;
							nc_nxt = nd5_nxt[ned];
						}
					}
					i ++;
				}
			}

			// 非同期２地点間のシーンチェンジがないか検索
			// 間隔が5の倍数ではなくても非同期両端から3～10秒のシーンチェンジは入れる
			int msec_st = pdata->getMsecScp(nst);
			int msec_ed = pdata->getMsecScp(ned);
			pdata->setScpStatpos(ned, SCP_PRIOR_DECIDE);
			int ncmp_i = -1;
			int ncmp_msec = 0;
			for(int i=nst+1; i<ned; i++){
				int msec_dst = pdata->getMsecScp(i);
				// １つ前の候補地点から離れたら１つ前の候補は確定する
				if (msec_dst - ncmp_msec >= 940 && ncmp_i >= 0){
					ncmp_i = -1;
				}
				// 非同期２地点のどちらからも３秒以上離れていることが前提
				if (msec_dst - msec_st >= 2670 && msec_ed - msec_dst >= 2670){
					int dis_ncmp = 0;
					int dis_dst = 1;
					// 前方からの間隔チェック（フレーム０は除く）
					CalcDifInfo calc1;
					CalcDifInfo calc2;
					if (calcDifFunc(calc1, msec_st, msec_dst) > 0 && nst > 0){
						if (calc1.sec < 10){
							dis_dst = 0;
							if (ncmp_i >= 0 && nst != 0){
								if (calcDifFunc(calc2, msec_st, ncmp_msec) > 0){
									if (calc1.gap < calc2.gap){
										dis_ncmp = 1;
									}
									else{
										dis_dst = 1;
									}
								}
							}
						}
					}
					// 後方からの間隔チェック
					if (calcDifFunc(calc1, msec_dst, msec_ed) > 0){
						if (calc1.sec < 10){
							dis_dst = 0;
							if (ncmp_i >= 0 && ned < num_scpos-1){
								if (calcDifFunc(calc2, ncmp_msec, msec_ed) > 0){
									if (calc1.gap < calc2.gap){
										dis_ncmp = 1;
									}
									else{
										dis_dst = 1;
									}
								}
							}
						}
					}
					if (dis_ncmp > 0){			// １つ前の候補が無効と判定された場合
						pdata->setScpStatpos(ncmp_i, SCP_PRIOR_DUPE);
						ncmp_i = -1;
					}
					if (dis_dst == 0){			// 現在の候補が無効ではないと判定された場合
						pdata->setScpStatpos(i, SCP_PRIOR_DECIDE);
						ncmp_i = i;
						ncmp_msec = msec_dst;
					}
				}
			}
			// 同期しているシーンチェンジの最終位置を取得
			int nc_st = ned;
			int nc_nxt = nd5_nxt[nc_st];
			int i = nc_st + 1;
			while(nc_nxt > 0 && i <= nc_nxt && i <= num_scpos-1){
				if (i == nc_nxt){				// 同期フレーム到着時
					nc_st = i;
					nc_nxt = nd5_nxt[nc_st];
					pdata->setScpStatpos(nc_st, SCP_PRIOR_DECIDE);
				}
				else if (i == num_scpos-1){		// 最終フレーム時
					nc_st = i;
					nc_nxt = 0;
				}
				else{
					if (nd5_sum[i] > nd5_sum[nc_st] && pdata->getScpStatpos(i) >= SCP_PRIOR_NONE){	// 同期フレーム到着前に候補があった場合
						int ntmp_nxt = nd5_nxt[i];
						int msec_tmp_st = pdata->getMsecScp(i);				// 新規候補位置
						int msec_tmp_ed = pdata->getMsecScp(ntmp_nxt);
						int msec_tmp_st2 = pdata->getMsecScp(nc_st);		// 元の位置
						int msec_tmp_ed2 = pdata->getMsecScp(nc_nxt);
						CalcDifInfo calc1;
						CalcDifInfo calc2;
						int flag1 = calcDifFunc(calc1, msec_tmp_st, msec_tmp_ed);
						int flag2 = calcDifFunc(calc2, msec_tmp_st2, msec_tmp_ed2);
						if (flag1 > 0 && calc1.gap <= pdata->msecValNear1){	// 誤差が小さかった場合のみ非同期地点とする
							if (flag2 == 0 ||
								(calc2.sec % 15 != 0 && (calc1.sec <= 30 || calc1.sec <= calc2.sec)) || 
								(msec_tmp_ed <= msec_tmp_ed2 + 10*1000)){
								nc_nxt = 0;
							}
						}
					}
				}
				i ++;
			}
			nst = nc_st;
		}
	}

	// 開始直後のシーンチェンジのチェック
	{
//		int ncmp_i = -1;
		ScpPriorType ncmp_stat = SCP_PRIOR_DUPE;
		int i = 1;
		while(pdata->getMsecScp(i) <= 30400 && i < num_scpos){		// 30秒以内をチェック
			ScpPriorType ntmp_stat = pdata->getScpStatpos(i);
			if (ntmp_stat == SCP_PRIOR_DECIDE && ncmp_stat <= SCP_PRIOR_LV1){	// 最初の確定候補
				ntmp_stat = SCP_PRIOR_LV1;
				if (nd15_flag[i] > 0){		// 15の倍数が後で存在する場合のみ
					int flag1 = 0;
					int flag2 = 0;
					int j = 0;					// 念のためループ防止
					// １回目の間隔取得
					int nc_st  = i;
					int nc_nxt = nc_st;
					CalcDifInfo calc1;
					CalcDifInfo calc2;
					while(nc_nxt > 0 && flag1 <= 1 && j < 100){
						nc_nxt = nd5_nxt[nc_nxt];
						j ++;
						if (nc_nxt > 0){
							int msec_src = pdata->getMsecScp(nc_st);
							int msec_dst = pdata->getMsecScp(nc_nxt);
							flag1 = calcDifExact(calc1, msec_src, msec_dst);
						}
					}
					// ２回目の間隔取得
					nc_st  = nc_nxt;
					while(nc_nxt > 0 && flag2 <= 0 && j < 100){
						nc_nxt = nd5_nxt[nc_nxt];
						j ++;
						if (nc_nxt > 0){
							int msec_src = pdata->getMsecScp(nc_st);
							int msec_dst = pdata->getMsecScp(nc_nxt);
							flag2 = calcDifExact(calc2, msec_src, msec_dst);
						}
					}
					if (flag1 > 0 && flag2 > 0){
						ntmp_stat = SCP_PRIOR_DECIDE;
					}
				}
				pdata->setScpStatpos(i, ntmp_stat);
			}
			if (ntmp_stat > SCP_PRIOR_LV1){							// 同期が既にある場合は何もしない
//				ncmp_i = -1;
				ncmp_stat = ntmp_stat;
			}
			else if (pdata->getMsecScp(i) <= 30400){		// 開始30秒以内が候補
				if (ntmp_stat > ncmp_stat){				// 最初の同期なしシーンチェンジが候補
					ncmp_stat = ntmp_stat;
//					ncmp_i = i;
				}
			}
			i ++;
		}
//		if (ncmp_i > 0){				// 最初の同期なしシーンチェンジがあった場合
//			pdata->m_nchk_scpos_1st = ncmp_i;
//		}
//		else{
//			pdata->m_nchk_scpos_1st = 0;
//		}
	}

//	for(i=0; i<num_scpos; i++){
//		printf("(%ld:%d, %d,%d,%d)", plogo->frm_scpos_s[i], plogo->stat_scpos[i], nd15_flag[i], nd5_sum[i], nd5_bsum[i]);
//	}

	// 単なる候補は初期状態に戻す
	for(int i=1; i<num_scpos-1; i++){		// 先頭と最後は除く
		if (pdata->getScpStatpos(i) == SCP_PRIOR_LV1){
			pdata->setScpStatpos(i, SCP_PRIOR_NONE);
		}
	}

//	// 配置情報初期化
//	for(i=0; i<plogo->num_scpos; i++){
//		plogo->arstat_sc_e[i] = D_SCINT_UNKNOWN;
//	}
}


//---------------------------------------------------------------------
// CutMrgIn/CutMrgOut位置の自動検出
//---------------------------------------------------------------------
void JlsReformData::detectCutmrg(){
	//--- 自動検出結果 ---
	Msec cutin  = detectCutmrgSub(1);
	Msec cutout = detectCutmrgSub(0);
	//--- 未設定だった場合は更新 ---
	if (pdata->extOpt.fixCutIn == 0){
		pdata->extOpt.msecCutIn = cutin;
	}
	if (pdata->extOpt.fixCutOut == 0){
		pdata->extOpt.msecCutOut = cutout;
	}

	//--- display auto detect CurMrgIn/Out ---
	if (1){
		string str_cutin;
		string str_cutout;
		// 検出マージン
		detectCutmrgDispval(str_cutin,  cutin );
		detectCutmrgDispval(str_cutout, cutout);
		printf("auto detect CutMrgIn=%s CutMrgOut=%s\n", str_cutin.c_str(), str_cutout.c_str());

		// 設定マージン
		detectCutmrgDispval(str_cutin,  pdata->extOpt.msecCutIn );
		detectCutmrgDispval(str_cutout, pdata->extOpt.msecCutOut);
		printf("current set CutMrgIn=%s CutMrgOut=%s\n", str_cutin.c_str(), str_cutout.c_str());
	}
}

//--- 表示用のマージン値を取得 ---
void JlsReformData::detectCutmrgDispval(string &strdisp, Msec mrg){
	//--- 16倍した値のベース値を取得 ---
	int mrg_abs = abs(mrg);
	int valbase = getFrm30fpsFromMsec(mrg_abs * 16 + 8);
	//--- 結果 ---
	int valint = valbase / 16;
	int valmod = valbase % 16;
	//--- 表示文字列作成 ---
	strdisp = "";
	if (mrg < 0 && (valint != 0 || valmod != 0)){
		strdisp += "-";
	}
	strdisp += to_string(valint);
	if (valmod > 0){
		int valfrac = (valmod * 100 + 8) / 16;
		strdisp += ".";
		int valf1 = valfrac / 10;
		int valf2 = valfrac % 10;
		strdisp += to_string(valf1);
		if (valf2 > 0){
			strdisp += to_string(valf2);
		}
	}
}

//---------------------------------------------------------------------
// CutMrgIn/CutMrgOut位置の自動検出
// 入力
//   rise : 0=CutMrgOut  1=CutMrgIn
// 出力
// 返り値： ミリ秒
//---------------------------------------------------------------------
Msec JlsReformData::detectCutmrgSub(int rise){
	int num_logo  = pdata->sizeDataLogo();
	int num_scpos = pdata->sizeDataScp();

	short hist[200];
	short hist_h[200];
	int fade_cnt;
	int loc_max;
	const int MINUS = 10;

	int loop = 2;
	while(loop > 0){
		// 使用する確定度
		ScpPriorType stat_level = SCP_PRIOR_NONE;
		if (loop == 2){
			stat_level = SCP_PRIOR_DECIDE;
		}

		fade_cnt = 0;
		int sum_scale = 0;
		for(int i=0; i<200; i++){
			hist[i] = 0;
			hist_h[i] = 0;
		}
		for(int i=0; i<num_logo; i++){
			Msec msec_logo;
			int scale, intl;
			{	// msec_logo, scale, intl, fade_cnt
				Msec msec_dif;
				int fade;
				DataLogoRecord dtlogo;
				pdata->getRecordLogo(dtlogo, i);
				if (rise > 0){
					msec_logo = dtlogo.rise;
					msec_dif  = dtlogo.rise_r - dtlogo.rise_l;
					fade      = dtlogo.fade_rise;
					intl      = dtlogo.intl_rise;
				}
				else{
					msec_logo = dtlogo.fall;
					msec_dif  = dtlogo.fall_r - dtlogo.fall_l;
					fade      = dtlogo.fade_fall * -1;
					intl      = dtlogo.intl_fall;
				}
				Msec msec_fade = 0;
				if (fade != 0){
					Msec msec_tmp = pdata->cnv.getMsecAdjustFrmFromMsec(msec_logo, fade);
					msec_fade = abs(msec_tmp - msec_logo);
				}
				// ロゴの不確定度で重みづけ
				scale = 1;
				if (msec_logo <= 50 || msec_logo >= pdata->getMsecTotalMax() - 50){
					scale = 0;
				}
				else if (msec_dif <= 50 || msec_dif <= msec_fade + 50){
					scale = 3;
				}
				sum_scale += scale;
				// フェード設定ありの判別
				if (fade != 0){
					fade_cnt += scale;
				}
				else{
					fade_cnt -= scale;
				}
			}
			// 確定無音シーンチェンジをマージン位置候補として追加していく
			if (scale > 0){
				for(int j=1; j < num_scpos - 1; j++){
					Msec msec_dif;
					Msec msec_scpj = pdata->getMsecScp(j);
					if (rise > 0){
						msec_dif = msec_logo - msec_scpj;
					}
					else{
						msec_dif = msec_scpj - msec_logo;
					}
					int tmp_dif = getFrm30fpsFromMsec( msec_dif );		// 30fps換算で推測
					tmp_dif += MINUS;
//printf("[%d:%d,%d]", j, tmp_dif, frm_logo);
					if (pdata->getScpStatpos(j) >= stat_level){
						if (tmp_dif >= 0 && tmp_dif < 200){
//printf("[%d,%d]", tmp_dif, frm_logo);
							hist[tmp_dif] += scale;
							if (intl > 0){
								hist_h[tmp_dif] += scale;
							}
						}
					}
				}
			}
		}

		// 候補から位置を決める
		loc_max = 0;
		int val_max = 0;
		for(int i=2; i<200-2; i++){
			int val;
			if (fade_cnt > 0){
				val = hist[i-2] + hist[i-1] + hist[i] + hist[i+1] + hist[i+2];
			}
			else{
				val = hist[i-1] + hist[i] + hist[i+1];
			}
			if (i >= MINUS-2 && i <= 36+MINUS){	// 標準的なマージン範囲
				val *= 2;						// 大き目に設定
			}
			else if (val == sum_scale){			// 全位置で検出した場合
				val = val * 5 / 3;				// 少し大き目に設定
			}
			else if (i >= 60+MINUS){			// 2秒以上はほぼ対象外
				val /= 4;
			}
			if (val_max < val){
				val_max = val;
				loc_max = i;
			}
			else if (val_max == val && loc_max > 0){
				if (hist_h[loc_max-1] < (hist[i+1] - hist_h[i+1])){
					val_max = val;
					loc_max = i;
				}
			}
		}
		// 候補が存在するか確認
		if (num_logo == 1 && val_max > 0){		// ロゴ１つだけの場合
			loop = 0;
		}
		if (val_max <= 1 || val_max <= num_logo * 2/3){	// 候補不足
			loop --;
			loc_max = 0;
		}
		else{
			loop = 0;
		}
	}

	// 候補の中から確定させる
	int loc_det = MINUS;
	int loc_half = 0;
	if (loc_max >= 2){
		if (fade_cnt <= 0){				// フェードなし
			loc_det = loc_max-1;
			int val_max = 0;
			short tmp_hist1[3];
			short tmp_hist2[3];
			for(int i=loc_max-1; i<=loc_max+1; i++){
				int val = hist[i];
				tmp_hist1[i-loc_max+1] = hist[i] - hist_h[i];
				tmp_hist2[i-loc_max+1] = hist_h[i];
				if (val_max < val){
					val_max = val;
					loc_det = i;
				}
			}
//printf("(%d,%d:%d,%d,%d - %d,%d,%d)", loc_det,loc_max,
// tmp_hist1[0],tmp_hist1[1],tmp_hist1[2],tmp_hist2[0],tmp_hist2[1],tmp_hist2[2]);
			if (loc_det == loc_max-1){
				if (tmp_hist2[0] < 3){		// HALFデータなし
					loc_half = 0;
					if (loc_det <= MINUS){
						loc_half = 1;
					}
				}
				else if ((tmp_hist1[1] + tmp_hist1[2])*5 <= tmp_hist1[0]){ // 全部最小
					loc_half = 1;
				}
				else{				// ALL:中間 HALF:最小
					loc_det += 1;
					loc_half = 0;
				}
			}
			else if (loc_det == loc_max + 1){
				if (tmp_hist2[0] < 3 && tmp_hist2[1] < 3 && tmp_hist2[2] < 3){	// HALFデータなし
					loc_half = 0;
					if (tmp_hist1[1] >= 3 && tmp_hist1[1] > tmp_hist1[0] * 2){
						loc_det -= 1;
					}
					if (loc_det <= MINUS){
						loc_half = 1;
					}
				}
				else if (tmp_hist2[0] >= 3 && tmp_hist2[0] > tmp_hist2[2] * 2){
					loc_det -= 1;		// ALL:中間 HALF:最小
					loc_half = 0;
				}
				else if (tmp_hist2[1] >= 3 && tmp_hist2[1] > tmp_hist2[0] * 2){
					loc_half = 0;		// ALL:最大 HALF:中間
				}
				else{
					loc_half = 1;		// 全部中間
				}
			}
			else{
				if (tmp_hist2[0] < 3 && tmp_hist2[1] < 3){	// HALFデータなし
					loc_half = 0;
					if (tmp_hist1[0] >= 3 && tmp_hist1[0] > tmp_hist1[2] * 2){
						loc_det -= 1;
					}
					if (loc_det <= MINUS){
						loc_half = 1;
					}
				}
				else if (tmp_hist2[0] >= 3 && tmp_hist2[0] > tmp_hist2[2] * 2){
					loc_half = 0;		// ALL:中間 HALF:最小
				}
				else if (tmp_hist1[2] >= 3 && tmp_hist1[2] > tmp_hist1[0] * 2){
					loc_det += 1;		// ALL:最大 HALF:中間
					loc_half = 0;
				}
				else{
					loc_half = 1;		// 全部中間
				}
			}
		}
		else{					// フェード時
			loc_det = loc_max-2;
			int val_max  = hist[loc_max-2];
			if (loc_det == 0){
				loc_det = 1;
				val_max = hist[1];
			}
			for(int i=loc_max-1; i<=loc_max+2; i++){
				int val = hist[i];
				if (val_max * 2 <= val){
					val_max = val;
					loc_det = i;
				}
			}
//printf("(%d,%d:%d,%d,%d,%d,%d)", loc_det,loc_max,
// hist[loc_max-2],hist[loc_max-1],hist[loc_max],hist[loc_max+1],hist[loc_max+2]);
			if (hist[loc_det+1] >= 3 && hist[loc_det-1] * 2 < hist[loc_det+1] &&
				hist[loc_det+1] > hist[loc_det]){
				loc_det += 1;
			}
		}
	}
	//--- 結果をミリ秒換算で返す ---
	Msec msec_mrg = getMsec30fpsFromFrm(loc_det - MINUS);
	if (loc_half != 0){
		msec_mrg += getMsec30fpsFromFrm(loc_half) / 2;
	}

	return msec_mrg;
}



//---------------------------------------------------------------------
// ロゴ位置を無音シーンチェンジ位置から補正
// 場合によってはロゴ位置から無音シーンチェンジ位置を修正
//---------------------------------------------------------------------
void JlsReformData::adjustLogo(){
	// ロゴデータ指定がない場合、全体をロゴデータとする
	if (pdata->sizeDataLogo() == 0 || pdata->extOpt.flagNoLogo > 0){
		pdata->clearDataLogoAll();
		DataLogoRecord dtlogo;
		pdata->clearRecordLogo(dtlogo);
		dtlogo.fall   = pdata->getMsecTotalMax();
		dtlogo.fall_l = dtlogo.fall;
		dtlogo.fall_r = dtlogo.fall;
		pdata->pushRecordLogo(dtlogo);
		// ロゴ読み込みなしに変更
		pdata->extOpt.flagNoLogo = 1;
	}

	// 旧調整を行わない場合
	if (pdata->extOpt.oldAdjust == 0){
		return;
	}

	int num_logo = pdata->sizeDataLogo();
	for(int i=0; i<num_logo*2; i++){
		int n = jlsd::nlgFromNrf(i);				// logo number
		LogoEdgeType edge = jlsd::edgeFromNrf(i);	// 0:start edge  1:end edge

		DataLogoRecord dtlogo;
		pdata->getRecordLogo(dtlogo, n);

		int msec_lg_c, msec_lg_l, msec_lg_r;
		if (edge == LOGO_EDGE_RISE){
			msec_lg_c = dtlogo.rise;
			msec_lg_l = dtlogo.rise_l;
			msec_lg_r = dtlogo.rise_r;
			dtlogo.stat_rise = LOGO_PRIOR_NONE;			 	// 初期化
		}
		else{
			msec_lg_c = dtlogo.fall;
			msec_lg_l = dtlogo.fall_l;
			msec_lg_r = dtlogo.fall_r;
			dtlogo.stat_fall = LOGO_PRIOR_NONE;			 	// 初期化
		}
		msec_lg_l -= pdata->msecValNear1;
		msec_lg_r += pdata->msecValNear1;
		int msec_lgn_l = msec_lg_l - 2*1000;
		int msec_lgn_r = msec_lg_r + 2*1000;

		int lgr_num   = -1;
		LogoPriorType lgr_stat  = LOGO_PRIOR_DUPE;
		int lgr_dif   = -1;
		int lgr_msec  = -1;
		int lgm1_num  = -1;
		int lgm1_dif  = -1;
		int lgm1_msec = -1;
		int lgm2_num  = -1;
		int lgm2_dif  = -1;
		int lgm2_msec = -1;
		int lgn_num   = -1;
		int lgn_dif   = -1;
		int lgn2_num  = -1;
		int lgn2_dif  = -1;
		// ロゴ位置に近い無音シーンチェンジを検索
		int num_scp = pdata->sizeDataScp();
		for(int j=1; j<num_scp-1; j++){
			DataScpRecord dtscp;
			pdata->getRecordScp(dtscp, j);

			int msec_sc;
			if (edge == LOGO_EDGE_RISE){
				msec_sc = dtscp.msec;
			}
			else{
				msec_sc = dtscp.msbk;
			}
			int msec_smute_s = dtscp.msmute_s - pdata->msecValExact;
			int msec_smute_e = dtscp.msmute_e + pdata->msecValExact;
			LogoPriorType stat_sc = priorLogoFromScp( dtscp.statpos );
			int msec_dif          = abs(msec_sc - msec_lg_c);

//		printf("(%d %d)", stat_sc, frm_sc);
			// ロゴ区間内にある無音シーンチェンジ検出
			if (msec_sc >= msec_lg_l && msec_sc <= msec_lg_r){
				if ((stat_sc >= lgr_stat && (msec_dif < lgr_dif || lgr_dif < 0)) ||
					stat_sc > lgr_stat){
					lgr_num  = j;
					lgr_stat = stat_sc;
					lgr_dif  = msec_dif;
					lgr_msec = msec_sc;
				}
			}
			// 誤検出調査用のロゴ区間付近無音シーンチェンジ検出
			if (msec_sc >= msec_lgn_l && msec_sc <= msec_lgn_r){
				if ((msec_dif < lgn_dif || lgn_dif < 0) && stat_sc <= LOGO_PRIOR_LV1){
					lgn_num = j;
					lgn_dif = msec_dif;
				}
			}
			// 誤検出調査2用のロゴ区間付近無音シーンチェンジ検出
			if (msec_sc >= msec_smute_s && msec_sc <= msec_smute_e){
				if ((msec_dif < lgn2_dif || lgn2_dif < 0) && stat_sc <= LOGO_PRIOR_LV1){
					lgn2_num = j;
					lgn2_dif = msec_dif;
				}
			}
			// 近くの確定無音シーンチェンジ位置を検出
			if ((msec_dif < lgm1_dif || lgm1_dif < 0) && stat_sc == LOGO_PRIOR_DECIDE){
				lgm1_num  = j;
				lgm1_dif  = msec_dif;
				lgm1_msec = msec_sc;
			}
			// 隣の確定無音シーンチェンジ位置を検出
			if ((msec_dif < lgm2_dif || lgm2_dif < 0) && stat_sc == LOGO_PRIOR_DECIDE && msec_dif > pdata->msecValLap2){
				if ((edge == LOGO_EDGE_RISE && msec_sc < msec_lg_c) ||
					(edge != LOGO_EDGE_RISE && msec_sc > msec_lg_c)){
					lgm2_num  = j;
					lgm2_dif  = msec_dif;
					lgm2_msec = msec_sc;
				}
			}
		}
//		printf("%d %d %d %d %d\n", lgr_stat, lgr_frm, lgm1_frm, lgm1_dif, lgm2_frm);
		// ロゴの確定度
		if (edge == LOGO_EDGE_RISE){
			dtlogo.stat_rise = lgr_stat;
		}
		else{
			dtlogo.stat_fall = lgr_stat;
		}
		// ロゴ位置を変更するか判断
		int flag_lg_change = 0;
		int flag_sc_change = 0;
		int num_sc_change;
		int msec_change;
		// 傍に無音シーンチェンジがあった場合
		if (lgm1_dif > pdata->msecValExact && lgm1_dif <= pdata->msecValNear3 && lgm2_num >= 0){
			CalcDifInfo calc1;
			CalcDifInfo calc2;
			int flag1 = calcDifFunc(calc1, lgm2_msec, msec_lg_c);
			int flag2 = calcDifFunc(calc2, lgm2_msec, lgm1_msec);
			if (flag1 == 1){
				flag1 = 0;
			}
			if (flag2 == 1){
				flag2 = 0;
			}
//			if (flag1 > 0){
//printf("[%d %d]", ncal_sec, ncal2_sec);
//				if (ncal_sec % 5 != 0 && ncal2_sec % 5 == 0){
//					flag1 = 0;
//				}
//			}
			// ロゴ範囲内に確定した無音シーンチェンジがあった場合の処理
			if (lgr_stat == LOGO_PRIOR_DECIDE){
				if (flag1 == 0 && flag2 == 0){		// 両方とも単位距離外の場合
					flag2 = 1;						// ロゴを無音シーンチェンジ位置に移す
				}
				else if (flag1 != 0 && flag2 != 0){	// 両方とも単位距離内の場合
					if (calc1.sec != calc2.sec){		// 時間が全然違う場合
						flag1 = 0;					// ロゴを無音シーンチェンジ位置に移す
					}
					else{
						calc1.gap += pdata->msecValNear1;		// ロゴ側を厳しく取る
					}
				}
			}
			// 確定シーンチェンジから単位距離になる位置に近い方を選択する
			if (flag1 == 0 && flag2 == 0){		// 両方とも単位距離外の場合
			}
			else if ((calc1.sec == calc2.sec && calc1.gap > calc2.gap) || flag1 == 0){
				flag_lg_change = 1;
				msec_change    = lgm1_msec;
				lgr_num        = lgm1_num;
			}
			else if ((calc1.sec == calc2.sec && calc1.gap < calc2.gap) || flag2 == 0){
				flag_sc_change = 1;
				num_sc_change  = lgm1_num;
				msec_change    = msec_lg_c;
			}
		}
		else if (lgr_stat == LOGO_PRIOR_DECIDE){	// ロゴ範囲内に確定した無音シーンチェンジがあった場合
			flag_lg_change = 1;
			msec_change    = lgr_msec;
		}
		else if (lgn_num >= 0 && lgm2_num >= 0){	// 不確定／確定シーンチェンジが近くに両方ある場合
			CalcDifInfo calc;
			if (calcDifFunc(calc, lgm2_msec, msec_lg_c) > 0){
				if (calc.sec % 15 == 0 || calc.sec == 10){	// ロゴが確定シーンチェンジから単位距離になる場合
					if (lgn_dif > pdata->msecValExact){		// ロゴと無音シーンチェンジが隣接していない場合
						flag_sc_change = 1;
						num_sc_change  = lgn_num;
						msec_change    = msec_lg_c;
					}
				}
			}
		}
		else if (lgn_num >= 0 && edge == LOGO_EDGE_RISE && dtlogo.rise <= 30*1000+400 &&	// 先頭部分のロゴ
				 (n == 0 || n == 1)){
			if (lgr_num >= 0){										// 候補無音シーンチェンジあり
				if (msec_lg_l <= 0 && (lgr_msec > msec_lg_c * 2)){	// 0地点の方が近い場合何もしない
				}
				else{
					flag_lg_change = 1;
					msec_change    = lgr_msec;
				}
			}
			else{											// ロゴ区間にシーンチェンジなし
				if (lgn2_num >= 0){							// ロゴ区間に無音あり
					if (pdata->getScpStatpos(lgn2_num) <= SCP_PRIOR_NONE){
						flag_sc_change = 1;
						num_sc_change  = lgn2_num;
						msec_change    = msec_lg_c;
					}
				}
			}
		}
		// 最初の開始位置候補が不確定だった場合には変更
		// 最初から開始位置で、次のロゴが30秒以内に始まる場合も追加
//		if (lgr_num >= 0){
//			if ((n == 0 && edge == LOGO_EDGE_RISE && plogo->nchk_scpos_1st != 0) ||
//				(n == 1 && edge == LOGO_EDGE_RISE && plogo->nchk_scpos_1st != 0 &&
//					pdata->getMsecLogoRise(0) <= 50 && lgr_msec <= 30*1000+400)){
//				plogo->nchk_scpos_1st = lgr_num;
//			}
//		}
		// ロゴ変更が最終シーンチェンジより後のフレームだった場合は変更せず
		if (flag_lg_change > 0 && msec_change > pdata->getMsecTotalMax()){
			flag_lg_change = 0;
		}
		// ロゴ位置の変更
		if (flag_lg_change > 0){
			if (edge == LOGO_EDGE_RISE){
				if (dtlogo.rise != msec_change){
					dtlogo.rise = msec_change;
					if (dtlogo.rise_l > msec_change){
						dtlogo.rise_l = msec_change;
					}
					if (dtlogo.rise_r < msec_change){
						dtlogo.rise_r = msec_change;
					}
					int frm_old    = pdata->cnv.getFrmFromMsec(dtlogo.rise);
					int frm_change = pdata->cnv.getFrmFromMsec(msec_change);
					if (abs(frm_old - frm_change) > 2){
						printf("change logo : %d -> %d\n", frm_old, frm_change);
					}
				}
			}
			else{
				if (dtlogo.fall != msec_change){
					dtlogo.fall = msec_change;
					if (dtlogo.fall_l > msec_change){
						dtlogo.fall_l = msec_change;
					}
					if (dtlogo.fall_r < msec_change){
						dtlogo.fall_r = msec_change;
					}
					int frm_old    = pdata->cnv.getFrmFromMsec(dtlogo.fall);
					int frm_change = pdata->cnv.getFrmFromMsec(msec_change);
					if (abs(frm_old - frm_change) > 2){
						printf("change logo : %d -> %d\n", frm_old, frm_change);
					}
				}
			}
		}
		// ロゴデータ更新
		pdata->setRecordLogo(dtlogo, n);

		// シーンチェンジ位置変更
		if (flag_sc_change > 0){
			DataScpRecord dtscp;
			pdata->getRecordScp(dtscp, num_sc_change);
			if (edge == LOGO_EDGE_RISE){
				if (dtscp.msec != msec_change){
					int frm_old    = pdata->cnv.getFrmFromMsec(dtscp.msec);
					int frm_change = pdata->cnv.getFrmFromMsec(msec_change);
					int msec_new   = pdata->cnv.getMsecFromFrm(frm_change);
					int msec_newm1 = pdata->cnv.getMsecFromFrm(frm_change-1);
					printf("change scpos : %d -> %d\n", frm_old, frm_change);
					ScpPriorType tmp_stat = dtscp.statpos;
					dtscp.statpos = SCP_PRIOR_DUPE;
					pdata->setRecordScp(dtscp, num_sc_change);
					pdata->insertScpos(msec_new, msec_newm1, num_sc_change, tmp_stat);
				}
			}
			else{
				if (dtscp.msbk != msec_change){
					int frm_old    = pdata->cnv.getFrmFromMsec(dtscp.msec);
					int frm_change = pdata->cnv.getFrmFromMsec(msec_change);
					int msec_new   = pdata->cnv.getMsecFromFrm(frm_change);
					int msec_newp1 = pdata->cnv.getMsecFromFrm(frm_change+1);
					printf("change scpos : %d -> %d\n", frm_old, frm_change);
					ScpPriorType tmp_stat = dtscp.statpos;
					dtscp.statpos = SCP_PRIOR_DUPE;
					pdata->setRecordScp(dtscp, num_sc_change);
					pdata->insertScpos(msec_newp1, msec_new, num_sc_change, tmp_stat);
				}
			}
		}
	}
}



//---------------------------------------------------------------------
// ２フレーム間の秒数を取得
// 入力：
//   frm_src  : 比較元フレーム番号
//   frm_dst  : 比較先フレーム番号
// 出力：
//   calc.sgn : 比較フレーム大小
//   calc.sec : フレーム間秒数
//   calc.gap : フレーム間秒数の誤差
// 返り値：
//   2 : 1の条件を満たす中で25秒以下または30,45,60,90,120秒
//   1 : ３秒以上かつ秒単位で誤差が少ない場合（１５秒単位では誤差甘め）
//   0 : それ以外
//---------------------------------------------------------------------
int JlsReformData::calcDifFunc(CalcDifInfo &calc, int msec_src, int msec_dst){
	int flag = 0;
	Msec msec_dif_org = msec_dst - msec_src;
	Msec msec_dif_abs = abs(msec_dif_org);
	Sec  sec_dif      = (msec_dif_abs + 500) / 1000;
	Msec msec_gap     = abs(sec_dif * 1000 - msec_dif_abs);
	if ((msec_gap <= pdata->msecValNear1 && sec_dif >= 3) ||						// 3秒以上で秒単位切替
		(msec_gap <= pdata->msecValNear2 && sec_dif == 10) ||						// 10秒
		(msec_gap <= pdata->msecValNear2 && sec_dif >= 15 && sec_dif % 15 == 0)){	// 15秒単位切り替え
		if (sec_dif <= 120){
			flag = 1;
			if (sec_dif <= 25 || sec_dif == 30 || sec_dif == 45 || sec_dif == 60 ||
				sec_dif == 90 || sec_dif == 120){
				flag = 2;
			}
		}
	}
	// 結果格納
	if (msec_dif_org < 0 && sec_dif != 0){
		calc.sgn = -1;
	}
	else if (msec_dif_org > 0 && sec_dif != 0){
		calc.sgn = 1;
	}
	else{
		calc.sgn = 0;
	}
	calc.sec = sec_dif;
	calc.gap = msec_gap;

	return flag;
}


//---------------------------------------------------------------------
// ２フレーム間の秒数を取得（誤差少ない前提）
// 入力：
//   frm_src  : 比較元フレーム番号
//   frm_dst  : 比較先フレーム番号
// 出力：
//   ncal_sgn : 比較フレーム大小
//   ncal_sec : フレーム間秒数
//   ncal_dis : フレーム間秒数の誤差
// 返り値：
//   2 : 10,15,30,45,60,90,120秒
//   1 : ３秒以上２５秒以下かつ秒単位で誤差が少ない場合
//   0 : それ以外
//---------------------------------------------------------------------
int JlsReformData::calcDifExact(CalcDifInfo &calc, int msec_src, int msec_dst){
	int flag = 0;
	Msec msec_dif_org = msec_dst - msec_src;
	Msec msec_dif_abs = abs(msec_dif_org);
	Sec  sec_dif      = (msec_dif_abs + 500) / 1000;
	Msec msec_gap     = abs(sec_dif * 1000 - msec_dif_abs);
	if (msec_gap <= pdata->msecValNear1 &&
		(sec_dif == 10 || sec_dif == 15 || sec_dif == 30 || sec_dif == 45 ||
		 sec_dif == 60 || sec_dif == 90 || sec_dif == 120)){
		flag = 2;
	}
	else if (msec_gap <= pdata->msecValExact && (sec_dif >= 3 && sec_dif <= 25)){
		flag = 1;
	}
	// 結果格納
	if (msec_dif_org < 0 && sec_dif != 0){
		calc.sgn = -1;
	}
	else if (msec_dif_org > 0 && sec_dif != 0){
		calc.sgn = 1;
	}
	else{
		calc.sgn = 0;
	}
	calc.sec = sec_dif;
	calc.gap = msec_gap;

	return flag;
}

//---------------------------------------------------------------------
// 33msを1とした単位に変換
//---------------------------------------------------------------------
int JlsReformData::getFrm30fpsFromMsec(int msec){
	int frate_n = 30000;
	int frate_d = 1001;
	int r = ((((long long)abs(msec) * frate_n) + (frate_d*1000/2)) / (frate_d*1000));
	return (msec >= 0)? r : -r;
}
int JlsReformData::getMsec30fpsFromFrm(int frm){
	int frate_n = 30000;
	int frate_d = 1001;
	int r = (((long long)abs(frm) * frate_d * 1000 + (frate_n/2)) / frate_n);
	return (frm >= 0)? r : -r;
}

