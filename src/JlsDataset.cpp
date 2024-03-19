//
// join_logo_scp データ格納クラス
//

//#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsDataset.hpp"

//=====================================================================
// 初期設定
//=====================================================================

JlsDataset::JlsDataset(){
	//--- 関数ポインタ作成 ---
	this->pdata = this;
	//--- データ初期化 ---
	initData();
}

//---------------------------------------------------------------------
// 初期値設定
//---------------------------------------------------------------------
void JlsDataset::initData(){
	m_scp.clear();
	m_logo.clear();
	resultTrim.clear();

	//--- 初期設定 ---
	setConfig(ConfigVarType::msecWLogoTRMax      , 120*1000);
	setConfig(ConfigVarType::msecWCompTRMax      , 60*1000);
	setConfig(ConfigVarType::msecWLogoSftMrg     , 4200 );
	setConfig(ConfigVarType::msecWCompFirst      , 0    );
	setConfig(ConfigVarType::msecWCompLast       , 0    );
	setConfig(ConfigVarType::msecWLogoSumMin     , 20*1000);
	setConfig(ConfigVarType::msecWLogoLgMin      , 4500 );
	setConfig(ConfigVarType::msecWLogoCmMin      , 40*1000);
	setConfig(ConfigVarType::msecWLogoRevMin     , 185*1000);
	setConfig(ConfigVarType::msecMgnCmDetect     , 1500 );
	setConfig(ConfigVarType::msecMgnCmDivide     , 500  );
	setConfig(ConfigVarType::secWCompSPMin       , 6    );
	setConfig(ConfigVarType::secWCompSPMax       , 13   );
	setConfig(ConfigVarType::flagCutTR           , 1    );
	setConfig(ConfigVarType::flagCutSP           , 0    );
	setConfig(ConfigVarType::flagAddLogo         , 1    );
	setConfig(ConfigVarType::flagAddUC           , 0    );
	setConfig(ConfigVarType::typeNoSc            , 0    );
	setConfig(ConfigVarType::cancelCntSc         , 0    );
	setConfig(ConfigVarType::LogoLevel           , 0    );
	setConfig(ConfigVarType::LogoRevise          , 0    );
	setConfig(ConfigVarType::AutoCmSub           , 0    );
	setConfig(ConfigVarType::msecPosFirst        , -1   );
	setConfig(ConfigVarType::msecLgCutFirst      , -1   );
	setConfig(ConfigVarType::msecZoneFirst       , -1   );
	setConfig(ConfigVarType::msecZoneLast        , -1   );
	setConfig(ConfigVarType::priorityPosFirst    , 0    );

	//--- 外部設定オプション ---
	extOpt = {};		// 念のため個別に初期化
	extOpt.verbose    = 0;
	extOpt.msecCutIn  = 0;
	extOpt.msecCutOut = 0;
	extOpt.frmLastcut = 0;
	extOpt.wideCutIn  = 0;
	extOpt.wideCutOut = 0;
	extOpt.flagNoLogo = 0;
	extOpt.errNoLogo  = 0;
	extOpt.flagDispNoLogo = 0;
	extOpt.fixCutIn   = 0;
	extOpt.fixCutOut  = 0;
	extOpt.fixWidCutI = 0;
	extOpt.fixWidCutO = 0;
	extOpt.oldAdjust  = 0;
	extOpt.fixVLine   = 0;
	extOpt.fixFDirect = 0;
	extOpt.fixNLgExact = 0;
	extOpt.fixNSysCode = 0;
	extOpt.fixNStdCode = 0;
	extOpt.fixSubList = 0;
	extOpt.fixSubPath = 0;
	extOpt.fixSetup   = 0;
	extOpt.fixPathRead = 0;
	extOpt.vLine      = 0;
	extOpt.flagDirect = 0;
	extOpt.nLgExact   = 0;
	extOpt.nSysCode   = 0;
	extOpt.nStdCode   = 0;
	extOpt.dispSysMes = 0;
	extOpt.subList    = "user,<,common";	// 初期検索フォルダ設定
	extOpt.subPath    = "";
	extOpt.setup      = "JL_common.txt";	// 共通先頭実行ファイル
	extOpt.pathRead   = "${JLUSERPATH},${JLDATAPATH}";	// READ系ファイル検索パスリスト

	//--- 状態初期設定 ---
	recHold = {};		// 念のため個別に初期化
	recHold.msecSelect1st = -1;
	recHold.msecTrPoint   = -1;
	recHold.rmsecHeadTail = {-1, -1};
	recHold.typeRange     = 0;
	m_msecTotalMax    = 0;
	m_levelUseLogo  = 0;
	m_flagSetupAdj  = 0;
	m_flagSetupAuto = 0;
	m_nscOutDetail = 0;
	//--- 保管データ ---
	backupLogosetSave();

	//--- 固定値設定 ---
	msecValExact = 100;
	msecValNear1 = 200;
	msecValNear2 = 400;
	msecValNear3 = 1200;
	msecValLap1  = 700;
	msecValLap2  = 2500;
	msecValSpc   = 1200;
};

//---------------------------------------------------------------------
// ロゴデータ全消去
//---------------------------------------------------------------------
void JlsDataset::clearDataLogoAll(){
	m_logo.clear();
}

//=====================================================================
// ロゴ・シーンチェンジデータの保存・読み出し
//=====================================================================

//---------------------------------------------------------------------
// バックアップデータ保存
//---------------------------------------------------------------------
void JlsDataset::backupLogosetSave(){
	m_backupData.bak_scp          = m_scp;
	m_backupData.bak_logo         = m_logo;
	m_backupData.bak_msecTotalMax = m_msecTotalMax;
	m_backupData.bak_extOpt       = extOpt;
}

//---------------------------------------------------------------------
// バックアップデータ読み出し
//---------------------------------------------------------------------
void JlsDataset::backupLogosetLoad(){
	//--- 読み込みデータ ---
	m_scp  = m_backupData.bak_scp;
	m_logo = m_backupData.bak_logo;

	//--- 元の状態に戻す外部設定オプション ---
	extOpt.flagNoLogo = m_backupData.bak_extOpt.flagNoLogo;
	extOpt.fixFDirect = m_backupData.bak_extOpt.fixFDirect;
	extOpt.flagDirect = m_backupData.bak_extOpt.flagDirect;

	//--- 状態初期設定 ---
	RangeMsec rmsecBak = recHold.rmsecHeadTail;
	int  typeRangeBak  = recHold.typeRange;
	recHold = {};		// 念のため個別に初期化
	recHold.msecSelect1st = -1;
	recHold.msecTrPoint   = -1;
//	recHold.rmsecHeadTail = {-1, -1};
	recHold.rmsecHeadTail = rmsecBak;
	recHold.typeRange     = typeRangeBak;
	m_msecTotalMax    = m_backupData.bak_msecTotalMax;
	m_levelUseLogo  = 0;
	m_flagSetupAdj  = 0;
	m_flagSetupAuto = 0;
	m_nscOutDetail = 0;
}

//=====================================================================
// 動作設定の保存・読み出し
//=====================================================================

//---------------------------------------------------------------------
// config設定
//---------------------------------------------------------------------
void JlsDataset::setConfig(ConfigVarType tp, int val){
	int nTp = static_cast<int>(tp);
	m_config[nTp] = val;
}

//---------------------------------------------------------------------
// config設定値取得
//---------------------------------------------------------------------
int JlsDataset::getConfig(ConfigVarType tp){
	int nTp = static_cast<int>(tp);
	return m_config[nTp];
}

int JlsDataset::getConfigAction(ConfigActType acttp){
	int val;
	int ret = 0;
	switch(acttp){
		case ConfigActType::LogoDelEdge:		// ロゴ端のCM判断
			val = getConfig(ConfigVarType::LogoRevise);
			ret = val % 10;
			break;
		case ConfigActType::LogoDelMid:			// ロゴ内の15秒単位CM化
			val = getConfig(ConfigVarType::LogoRevise);
			ret = ((val / 10 % 10) & 0x1)? 1 : 0;
			break;
		case ConfigActType::LogoDelWide:		// 広域ロゴなし削除
			val = getConfig(ConfigVarType::LogoRevise);
			ret = ((val / 10 % 10) & 0x2)? 1 : 0;
			{								// AddUC=1の時は無効
				int tmp = getConfig(ConfigVarType::flagAddUC);
				if ((tmp % 10) & 0x1) ret = 0;
			}
			break;
		case ConfigActType::LogoUCRemain:		// ロゴなし不明部分を残す
			val = getConfig(ConfigVarType::flagAddUC);
			ret = val % 10;
			break;
		case ConfigActType::LogoUCGapCm:		// CM単位から誤差が大きい構成を残す
			val = getConfig(ConfigVarType::flagAddUC);
			ret = ((val / 10 % 10) & 0x1)? 1 : 0;
			if ( isUnuseLevelLogo() ) ret = 1;		// ロゴ使用しない場合は常時
			break;
		case ConfigActType::MuteNoSc:			// シーンチェンジなし無音位置のCM判断
			val = getConfig(ConfigVarType::typeNoSc);
			ret = val;
			if (val == 0){					// 自動判断
				ret = ( pdata->isUnuseLevelLogo() )? 1 : 2;
			}
			break;
		default:
			break;
	}
	return ret;
}


//=====================================================================
// データサイズ取得
//=====================================================================

//---------------------------------------------------------------------
// ロゴデータサイズ取得
//---------------------------------------------------------------------
int JlsDataset::sizeDataLogo(){
	return (int) m_logo.size();
}
//---------------------------------------------------------------------
// 無音SCデータサイズ取得
//---------------------------------------------------------------------
int JlsDataset::sizeDataScp(){
	return (int) m_scp.size();
}

//---------------------------------------------------------------------
// ロゴデータが空？
//---------------------------------------------------------------------
bool JlsDataset::emptyDataLogo(){
	return m_logo.empty();
}

//---------------------------------------------------------------------
// 無音SC全体データ番号取得（端を含むか選択付き）
//---------------------------------------------------------------------
RangeNsc JlsDataset::getRangeNscTotal(bool flagNoEdge){
	int rev = ( flagNoEdge )? 1 : 0;
	RangeNsc rnsc;
	rnsc.st = rev;
	rnsc.ed = sizeDataScp() - 1 - rev;
	return rnsc;
}


//=====================================================================
// １データセット単位の処理
//=====================================================================

//---------------------------------------------------------------------
// １データセット単位初期化（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::clearRecordLogo(DataLogoRecord &dt){
	dt.org_rise     = 0;
	dt.org_fall     = 0;
	dt.org_rise_l   = 0;
	dt.org_rise_r   = 0;
	dt.org_fall_l   = 0;
	dt.org_fall_r   = 0;
	dt.rise         = 0;
	dt.fall         = 0;
	dt.rise_l       = 0;
	dt.rise_r       = 0;
	dt.fall_l       = 0;
	dt.fall_r       = 0;
	dt.fade_rise    = 0;
	dt.fade_fall    = 0;
	dt.intl_rise    = 0;
	dt.intl_fall    = 0;
	dt.stat_rise    = LOGO_PRIOR_NONE;
	dt.stat_fall    = LOGO_PRIOR_NONE;
	dt.unit_rise    = LOGO_UNIT_NORMAL;
	dt.unit_fall    = LOGO_UNIT_NORMAL;
	dt.outtype_rise = LOGO_RESULT_NONE;
	dt.outtype_fall = LOGO_RESULT_NONE;
	dt.result_rise  = 0;
	dt.result_fall  = 0;
}

//---------------------------------------------------------------------
// １データセット単位初期化（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::clearRecordScp(DataScpRecord &dt){
	dt.msec     = 0;
	dt.msbk     = 0;
	dt.msmute_s = -1;
	dt.msmute_e = -1;
	dt.still    = 0;
	dt.statpos  = SCP_PRIOR_NONE;
	dt.score    = 0;
	dt.chap     = SCP_CHAP_NONE;
	dt.arstat   = SCP_AR_UNKNOWN;
	dt.arext    = SCP_AREXT_NONE;
}

//---------------------------------------------------------------------
// １データセット単位挿入（最後の位置）（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::pushRecordLogo(DataLogoRecord &dt){
	m_logo.push_back(dt);						// add data
}

//---------------------------------------------------------------------
// １データセット単位挿入（最後の位置）（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::pushRecordScp(DataScpRecord &dt){
	m_scp.push_back(dt);						// add data
}

//---------------------------------------------------------------------
// １データセット単位削除（最後の位置）（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::popRecordLogo(){
	m_logo.pop_back();							// delete data
}

//---------------------------------------------------------------------
// １データセット単位挿入（指定位置）（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::insertRecordLogo(DataLogoRecord &dt, Nlg nlg){
	if (nlg >= 0 && nlg <= sizeDataLogo()){
		m_logo.insert(m_logo.begin()+nlg, dt);
	}
}

//---------------------------------------------------------------------
// １データセット単位挿入（指定位置）（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::insertRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc <= sizeDataScp()){
		m_scp.insert(m_scp.begin()+nsc, dt);
	}
}
//---------------------------------------------------------------------
// １データセット単位削除（指定位置）（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::deleteRecordScp(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp.erase(m_scp.begin()+nsc);
	}
}

//---------------------------------------------------------------------
// １データセット単位取得（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::getRecordLogo(DataLogoRecord &dt, Nsc nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		dt = m_logo[nlg];
	}
}

//---------------------------------------------------------------------
// １データセット単位変更（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::setRecordLogo(DataLogoRecord &dt, Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		m_logo[nlg] = dt;
	}
}

//---------------------------------------------------------------------
// １データセット単位取得（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::getRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		dt = m_scp[nsc];
	}
}

//---------------------------------------------------------------------
// １データセット単位変更（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::setRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc] = dt;
	}
}



//=====================================================================
// １要素単位の処理
//=====================================================================

//---------------------------------------------------------------------
// ミリ秒データ取得（ロゴデータ立ち上がり）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoRise(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		return m_logo[nlg].rise;
	}
	return 0;
}

//---------------------------------------------------------------------
// ミリ秒データ取得（ロゴデータ立ち下がり）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoFall(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		return m_logo[nlg].fall;
	}
	return 0;
}

//---------------------------------------------------------------------
// ミリ秒データ取得（ロゴデータエッジ）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoNrf(Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			return m_logo[n].rise;
		}
		else{
			return m_logo[n].fall;
		}
	}
	return 0;
}

//---------------------------------------------------------------------
// ミリ秒データ取得（不明確領域を含めたロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::getMsecLogoNrfWide(int &msec_c, int &msec_l, int &msec_r, Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			msec_c = m_logo[n].rise;
			msec_l = m_logo[n].rise_l;
			msec_r = m_logo[n].rise_r;
		}
		else{
			msec_c = m_logo[n].fall;
			msec_l = m_logo[n].fall_l;
			msec_r = m_logo[n].fall_r;
		}
	}
	else{
		msec_c = 0;
		msec_l = 0;
		msec_r = 0;
	}
}

//---------------------------------------------------------------------
// ミリ秒データ取得（不明確領域を含めたロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::getWideMsecLogoNrf(WideMsec &wmsec, Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			wmsec.just  = m_logo[n].rise;
			wmsec.early = m_logo[n].rise_l;
			wmsec.late  = m_logo[n].rise_r;
		}
		else{
			wmsec.just  = m_logo[n].fall;
			wmsec.early = m_logo[n].fall_l;
			wmsec.late  = m_logo[n].fall_r;
		}
	}
	else{
		wmsec.just  = 0;
		wmsec.early = 0;
		wmsec.late  = 0;
	}
}

//---------------------------------------------------------------------
// ミリ秒データ変更（ロゴデータエッジ）
//---------------------------------------------------------------------
void JlsDataset::setMsecLogoNrf(Nrf nrf, Msec val){
	int n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			m_logo[n].rise = val;
			if (m_logo[n].rise_l > val){
				m_logo[n].rise_l = val;
			}
			if (m_logo[n].rise_r < val){
				m_logo[n].rise_r = val;
			}
		}
		else{
			m_logo[n].fall = val;
			if (m_logo[n].fall_l > val){
				m_logo[n].fall_l = val;
			}
			if (m_logo[n].fall_r < val){
				m_logo[n].fall_r = val;
			}
		}
	}
}

//---------------------------------------------------------------------
// ミリ秒データ取得（無音SC位置）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScp(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].msec;
	}
	return 0;
}

//---------------------------------------------------------------------
// ミリ秒データ取得（無音SC 直前終了位置）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScpBk(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].msbk;
	}
	return 0;
}

//---------------------------------------------------------------------
// ミリ秒データ取得（無音SC 開始／直前終了位置の選択付き）
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScpEdge(Nsc nsc, LogoEdgeType edge){
	if (edge == LOGO_EDGE_FALL){
		return getMsecScpBk(nsc);
	}
	return getMsecScp(nsc);
}

//---------------------------------------------------------------------
// ミリ秒データを開始終了幅ありで取得（無音SC位置）
//---------------------------------------------------------------------
WideMsec JlsDataset::getWideMsecScp(Nsc nsc){
	Msec msecT  = getMsecScp(nsc);
	Msec msecBk = getMsecScpBk(nsc);
	WideMsec wmsec;
	wmsec.just  = msecT;	// 中心は基準位置
	wmsec.early = msecBk;
	wmsec.late  = msecT;
	return wmsec;
}

//---------------------------------------------------------------------
// 範囲ミリ秒データ取得（範囲無音シーンチェンジ番号から）
//---------------------------------------------------------------------
RangeMsec JlsDataset::getRangeMsecFromRangeNsc(RangeNsc rnsc){
	RangeMsec rmsec;
	rmsec.st = pdata->getMsecScp(rnsc.st);
	rmsec.ed = pdata->getMsecScp(rnsc.ed);
	if (rnsc.st < 0) rmsec.st = -1;
	if (rnsc.ed < 0) rmsec.ed = -1;
	return rmsec;
}

//---------------------------------------------------------------------
// 画像変化なしフラグ取得
//---------------------------------------------------------------------
bool JlsDataset::getScpStill(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].still;
	}
	return false;
}

//---------------------------------------------------------------------
// 区切り状態取得
//---------------------------------------------------------------------
jlsd::ScpPriorType JlsDataset::getScpStatpos(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].statpos;
	}
	return SCP_PRIOR_NONE;
}

//---------------------------------------------------------------------
// 構成推測用 スコア取得
//---------------------------------------------------------------------
int JlsDataset::getScpScore(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].score;
	}
	return 0;
}

//---------------------------------------------------------------------
// 構成推測用 区切り状態取得
//---------------------------------------------------------------------
jlsd::ScpChapType JlsDataset::getScpChap(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].chap;
	}
	return SCP_CHAP_NONE;
}

//---------------------------------------------------------------------
// 構成推測用 構成内容取得
//---------------------------------------------------------------------
jlsd::ScpArType JlsDataset::getScpArstat(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].arstat;
	}
	return SCP_AR_UNKNOWN;
}

//---------------------------------------------------------------------
// 構成推測用 構成内容取得
//---------------------------------------------------------------------
jlsd::ScpArExtType JlsDataset::getScpArext(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].arext;
	}
	return SCP_AREXT_NONE;
}

//---------------------------------------------------------------------
// ロゴ結果取得
//---------------------------------------------------------------------
bool JlsDataset::getResultLogoAtNrf(Msec &msec, LogoResultType &outtype, Nrf nrf){
	if (nrf >= 0 && nrf/2 < sizeDataLogo()){
		if (nrf%2 == 0){
			msec = m_logo[nrf/2].result_rise;
			outtype = m_logo[nrf/2].outtype_rise;
		}
		else{
			msec = m_logo[nrf/2].result_fall;
			outtype = m_logo[nrf/2].outtype_fall;
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
// 区切り状態設定
//---------------------------------------------------------------------
void JlsDataset::setScpStatpos(Nsc nsc, ScpPriorType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].statpos = val;
	}
}

//---------------------------------------------------------------------
// 構成推測用 スコア設定
//---------------------------------------------------------------------
void JlsDataset::setScpScore(Nsc nsc, int val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].score = val;
	}
}

//---------------------------------------------------------------------
// 構成推測用 区切り状態設定
//---------------------------------------------------------------------
void JlsDataset::setScpChap(Nsc nsc, ScpChapType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].chap = val;
	}
}

//---------------------------------------------------------------------
// 構成推測用 構成内容設定
//---------------------------------------------------------------------
void JlsDataset::setScpArstat(Nsc nsc, ScpArType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].arstat = val;
		m_scp[nsc].arext  = SCP_AREXT_NONE;		// 拡張も初期化する
	}
}

//---------------------------------------------------------------------
// 構成推測用 構成内容拡張設定
//---------------------------------------------------------------------
void JlsDataset::setScpArext(Nsc nsc, ScpArExtType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].arext = val;
	}
}


//---------------------------------------------------------------------
// ロゴ結果設定
//---------------------------------------------------------------------
void JlsDataset::setResultLogoAtNrf(Msec msec, LogoResultType outtype, Nrf nrf){
	if (nrf >= 0 && nrf/2 < sizeDataLogo()){
		if (nrf%2 == 0){
			m_logo[nrf/2].result_rise = msec;
			m_logo[nrf/2].outtype_rise = outtype;
		}
		else{
			m_logo[nrf/2].result_fall = msec;
			m_logo[nrf/2].outtype_fall = outtype;
		}
	}
}



//=====================================================================
// 優先度取得処理
//=====================================================================

//---------------------------------------------------------------------
// ロゴデータの候補として優先度取得
//---------------------------------------------------------------------
jlsd::LogoPriorType JlsDataset::getPriorLogo(Nrf nrf){
	int n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			return m_logo[n].stat_rise;
		}
		else{
			return m_logo[n].stat_fall;
		}
	}
	return LOGO_PRIOR_NONE;
}

//---------------------------------------------------------------------
// 無音SCの候補として優先度取得
// Autoコマンドによる推測有無で取得データを変更
//---------------------------------------------------------------------
jlsd::ScpPriorType JlsDataset::getPriorScp(Nsc nsc){
	if (nsc >= 0 && nsc < (int) m_scp.size()){
		if (m_flagSetupAuto <= 1){
			return m_scp[nsc].statpos;
		}
		else{
			if (m_scp[nsc].chap >= SCP_CHAP_DECIDE){
				return SCP_PRIOR_DECIDE;
			}
			else if (m_scp[nsc].chap > SCP_CHAP_NONE){
				return SCP_PRIOR_LV1;
			}
			else if (m_scp[nsc].chap < SCP_CHAP_NONE){
				return SCP_PRIOR_DUPE;
			}
		}
	}
	return SCP_PRIOR_NONE;
}



//=====================================================================
// 前後データ取得処理（ロゴ）
//=====================================================================

//---------------------------------------------------------------------
// 次のロゴ位置取得
// 入力：
//   nrf  : ロゴ番号*2 + fall時は1
//   dr   : 検索方向（前側 / 後側）
//   edge : ロゴ端（0:立ち上がりエッジ  1:立ち下がりエッジ  2:両エッジ）
//   type : 0:すべて  1:有効ロゴ
// 返り値： 次のロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
Nrf JlsDataset::getNrfDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type){
	int size_logo = sizeDataLogo();
	int r = -1;
	bool flag_end = false;
	bool check_rise = isLogoEdgeRise(edge);
	bool check_fall = isLogoEdgeFall(edge);
	int i = nrf;
	int step = (dr == SEARCH_DIR_NEXT)? +1 : -1;
	while(flag_end == false){
		i += step;
		if (i >= 0 && i < size_logo*2){
			int nlg_i  = nlgFromNrf(i);
			int edge_i = edgeFromNrf(i);
			if (edge_i == LOGO_EDGE_RISE && check_rise){
				if (isValidLogoRise(nlg_i) || type == LOGO_SELECT_ALL){
					r = i;
					flag_end = true;
				}
			}
			else if (edge_i == LOGO_EDGE_FALL && check_fall){
				if (isValidLogoFall(nlg_i) || type == LOGO_SELECT_ALL){
					r = i;
					flag_end = true;
				}
			}
		}
		else{
			flag_end = true;
		}
	};
	return r;
}

// １つ前のロゴ位置取得
Nrf JlsDataset::getNrfPrevLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type){
	return getNrfDirLogo(nrf, SEARCH_DIR_PREV, edge, type);
}

// １つ後のロゴ位置取得
Nrf JlsDataset::getNrfNextLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type){
	return getNrfDirLogo(nrf, SEARCH_DIR_NEXT, edge, type);
}

//---------------------------------------------------------------------
// 次のロゴ位置取得（立ち上りと立ち下りのセット）
// 入力：
//   nfall : ロゴ番号*2 + fall時は1
//   type  : 0:すべて  1:有効ロゴ
// 返り値： ロゴ位置取得結果（取得時:true）
//   nrise : 次の立上りロゴ位置（ない場合は-1）
//   nfall : 次の立下りロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
bool JlsDataset::getNrfNextLogoSet(Nrf &nrf_rise, Nrf &nrf_fall, LogoSelectType type){
	nrf_rise = getNrfNextLogo(nrf_fall, LOGO_EDGE_RISE, type);
	if (nrf_rise >= 0){
		nrf_fall = getNrfNextLogo(nrf_rise, LOGO_EDGE_FALL, type);
	}
	else{
		nrf_fall = -1;
	}
	if (nrf_fall < 0) return false;
	return true;
}


//---------------------------------------------------------------------
// 次の推測構成ロゴ扱い位置取得（最終出力判定入力つき）
// 入力：
//   nsc  : シーンチェンジ番号
//   dr   : 検索方向（前側 / 後側）
//   edge : ロゴ端（0:立ち上がりエッジ  1:立ち下がりエッジ  2:両エッジ）
//   flag_border : Border領域をロゴに含める
//   flag_out    : 出力用（0:内部構築用構成  1:出力用構成）
// 返り値： 次のロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirElgForAll(Nsc nsc, SearchDirType dr, LogoEdgeType edge, bool flag_border, bool flag_out){
	int size_scp = sizeDataScp();
	int r = -1;
	int i = nsc;
	if (dr == SEARCH_DIR_NEXT){			// arstatは２点間の後側を見るための移動
		i = getNscNextScpDecide(i,  SCP_END_EDGEIN);
	}
	//--- 逆エッジ部分まで探索 ---
	bool inlogo_base = isElgInScpForAll(i, flag_border, flag_out);
	{
		int inext = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
		int icheck = (dr == SEARCH_DIR_NEXT)? i : inext;
		while(((edge == LOGO_EDGE_RISE && dr == SEARCH_DIR_NEXT && inlogo_base == true) ||
			   (edge == LOGO_EDGE_RISE && dr == SEARCH_DIR_PREV && inlogo_base == false) ||
			   (edge == LOGO_EDGE_FALL && dr == SEARCH_DIR_NEXT && inlogo_base == false) ||
			   (edge == LOGO_EDGE_FALL && dr == SEARCH_DIR_PREV && inlogo_base == true))
			  && ( isElgDivScpForAll(icheck, flag_border, flag_out) == false )	// ロゴ強制区切り
			  && i >= 0){
			i = inext;
			inext = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
			icheck = (dr == SEARCH_DIR_NEXT)? i : inext;
			inlogo_base = isElgInScpForAll(i, flag_border, flag_out);
		}
	}
	//--- エッジ部分を探索 ---
	if ((i > 0 && i < size_scp) ||
		(i == 0 && dr == SEARCH_DIR_NEXT) ||
		(i == size_scp && dr == SEARCH_DIR_PREV)){
		bool flag_end = false;
		while(flag_end == false){
			int ilast = i;
			i = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
			if (i > 0 && i < size_scp){
				bool inlogo_i = isElgInScpForAll(i, flag_border, flag_out);
				int  iset = i;
				if (dr == SEARCH_DIR_NEXT){
					iset = ilast;					// ２点間の前側
				}
				if (inlogo_i != inlogo_base){		// エッジ変化あれば確定
					flag_end = true;
					r = iset;
				}
				else if ( isElgDivScpForAll(iset, flag_border, flag_out) ){	// ロゴ強制区切り
					flag_end = true;
					r = iset;
				}
			}
			else{
				flag_end = true;
				if (inlogo_base == true){			// ロゴ内で終了したら端設定
					if (dr == SEARCH_DIR_NEXT){
						r = size_scp-1;
					}
					else{
						r = 0;
					}
				}
			}
		}
	};
	return r;
}


//---------------------------------------------------------------------
// 次の推測構成ロゴ扱い位置取得
// 入力：
//   nsc  : シーンチェンジ番号
//   edge : ロゴ端（0:立ち上がりエッジ  1:立ち下がりエッジ  2:両エッジ）
// 返り値： 次のロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
// １つ前の位置取得
Nsc JlsDataset::getNscPrevElg(Nsc nsc, LogoEdgeType edge){
	return getNscDirElgForAll(nsc, SEARCH_DIR_PREV, edge, false, false);
}

// １つ後の位置取得
Nsc JlsDataset::getNscNextElg(Nsc nsc, LogoEdgeType edge){
	return getNscDirElgForAll(nsc, SEARCH_DIR_NEXT, edge, false, false);
}

//---------------------------------------------------------------------
// 次のロゴ位置取得（立ち上りと立ち下りのセット）
// 入力：
//   nrf_fall : ロゴ番号*2 + fall時は1
//   type     : 0:すべて  1:有効ロゴ
// 返り値： ロゴ位置取得結果（取得時:true）
//   nrf_rise : 次の立上りロゴ位置（ない場合は-1）
//   nrf_fall : 次の立下りロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
bool JlsDataset::getNrfptNext(NrfCurrent &logopt, LogoSelectType type){
	//--- initialize ---
	if (logopt.valid == false){
		logopt.nrfRise = -1;
		logopt.nrfFall = -1;
		logopt.valid = true;
	}
	//--- backup ---
	logopt.nrfLastRise = logopt.nrfRise;
	logopt.nrfLastFall = logopt.nrfFall;
	logopt.msecLastRise = logopt.msecRise;
	logopt.msecLastFall = logopt.msecFall;
	//--- rise ---
	logopt.nrfRise = getNrfNextLogo(logopt.nrfFall, LOGO_EDGE_RISE, type);
	if (logopt.nrfRise >= 0){
		logopt.nrfFall = getNrfNextLogo(logopt.nrfRise, LOGO_EDGE_FALL, type);
	}
	else{
		logopt.nrfFall = -1;
	}
	logopt.msecRise = getMsecLogoNrf(logopt.nrfRise);
	logopt.msecFall = getMsecLogoNrf(logopt.nrfFall);
	if (logopt.nrfFall < 0) return false;
	return true;
}


//---------------------------------------------------------------------
// 次の推測構成ロゴ扱い位置取得（立ち上りと立ち下りのセット）
// 入力：
//   nsc_fall : 前の立ち下がりシーンチェンジ番号
// 返り値： ロゴ位置取得結果（取得時:true）
//   nsc_rise : 次の立上りロゴ位置（ない場合は-1）
//   nsc_fall : 次の立下りロゴ位置（ない場合は-1）
//---------------------------------------------------------------------
bool JlsDataset::getElgptNext(ElgCurrent &elg){
	bool flag1st = false;
	//--- initialize ---
	if (elg.valid == false){
		elg.nscFall = 0;
		elg.valid = true;
		flag1st = true;
	}
	//--- backup ---
	elg.nscLastRise  = elg.nscRise;
	elg.nscLastFall  = elg.nscFall;
	elg.msecLastRise = elg.msecRise;
	elg.msecLastFall = elg.msecFall;
	//--- rise ---
	if ( isElgDivScpForAll(elg.nscFall, elg.border, elg.outflag) == false ){	// ロゴ内分割位置かチェック
		if (flag1st) elg.nscFall = -1;		// 0位置からチェックするため
		elg.nscRise = getNscDirElgForAll(elg.nscFall, SEARCH_DIR_NEXT, LOGO_EDGE_RISE, elg.border, elg.outflag);
	}
	else{
		elg.nscRise = elg.nscFall;
	}
	elg.msecRise = getMsecScp(elg.nscRise);
	//--- fall ---
	if (elg.nscRise >= 0){
		elg.nscFall = getNscDirElgForAll(elg.nscRise, SEARCH_DIR_NEXT, LOGO_EDGE_FALL, elg.border, elg.outflag);
	}
	else{
		elg.nscFall = -1;
	}
	if (elg.outflag){					// 最終出力時は正確に立ち下がり位置
		elg.msecFall = getMsecScpBk(elg.nscFall);
	}
	else{								// 通常はシーンチェンジ番号で共通位置認識
		elg.msecFall = getMsecScp(elg.nscFall);
	}
	//--- end ---
	if (elg.nscFall < 0) return false;
	return true;
}

//---------------------------------------------------------------------
// ロゴの結果位置（次の位置）を取得
// 入力：
//   nlg    : 検索を開始するロゴ番号（指定位置を含む）
// 出力：
//   返り値：取得したロゴの次のロゴ番号（-1の時は該当なし）
//   msec_rise : 立ち上がり位置（ミリ秒）
//   msec_rise : 立ち下がり位置（ミリ秒）
//   cont_next : 次のロゴが切れ目なしの連続か
//---------------------------------------------------------------------
Nlg JlsDataset::getResultLogoNext(Msec &msec_rise, Msec &msec_fall, bool &cont_next, Nlg nlg){
	int msec_val_cont = 80;					// 同一ロゴとみなす間隔msec

	int size_logo = sizeDataLogo();
	cont_next = false;
	msec_rise = -1;
	msec_fall = -1;
	//--- 立ち上がりエッジ検索時 ---
	Nlg nlg_rise = nlg;
	bool flag_rise = false;
	while(nlg_rise >= 0 && nlg_rise < size_logo && flag_rise == false){
		DataLogoRecord dtlogo;
		getRecordLogo(dtlogo, nlg_rise);
		if (dtlogo.outtype_rise == LOGO_RESULT_DECIDE){				// 確定エッジ
			flag_rise = true;
			msec_rise = dtlogo.result_rise;
		}
		if (flag_rise == false){
			nlg_rise ++;
		}
	}
	//--- 立ち下がりエッジ検索時 ---
	Nlg nlg_fall = nlg_rise;
	bool flag_fall = false;
	bool flag_unit = false;
	if (flag_rise == false){
		nlg_fall = nlg;
	}
	while(nlg_fall >= 0 && nlg_fall < size_logo && flag_fall == false){
		DataLogoRecord dtlogo;
		getRecordLogo(dtlogo, nlg_fall);
		if (dtlogo.outtype_fall == LOGO_RESULT_DECIDE){				// 確定エッジ
			flag_fall = true;
			msec_fall = dtlogo.result_fall;
			if (dtlogo.unit_fall){
				flag_unit = true;
			}
		}
		if (flag_fall == false){
			nlg_fall ++;
		}
	}
	//--- 次のロゴが同一ロゴか確認 ---
	if (flag_fall){
		Nlg  nlg_next = nlg_fall + 1;
		bool flag_next = false;
		while(nlg_next >= 0 && nlg_next < size_logo && flag_next == false){
			DataLogoRecord dtlogo;
			getRecordLogo(dtlogo, nlg_next);
			if (dtlogo.outtype_rise == LOGO_RESULT_DECIDE){				// 確定エッジ
				flag_next = true;
				int msec_next = dtlogo.result_rise;
				if (msec_fall + msec_val_cont > msec_next){				// 同一ロゴで補正する
					if ( (extOpt.nLgExact & 0x02) != 0 ){
						msec_fall = cnv.getMsecAdjustFrmFromMsec(msec_next, -1);	// 切れ目なしに補正
					}
					if (flag_unit == false){					// 独立ロゴでなければ切れ目なし判定
						if ( dtlogo.unit_rise == false ){
							cont_next = true;
						}
					}
				}
			}
			nlg_next ++;
		}
	}
	//--- 結果 ---
	Nlg nlg_ret = -1;
	//--- 正常取得時 ---
	if (flag_rise == true && flag_fall == true){
		nlg_ret = nlg_fall + 1;
	}
	else{
		//--- 立ち下がりがない場合 ---
		if ((flag_rise == true || nlg == 0) && flag_fall == false){
			nlg_ret = size_logo;
			msec_fall = getMsecTotalMax();
		}
		//--- 最初からロゴが全くない場合 ---
		if (nlg == 0){
			msec_rise = 0;
		}
	}
	return nlg_ret;
}

// 出力ロゴ対応
//---------------------------------------------------------------------
// 次のロゴ位置取得（立ち上りと立ち下りのセット）
// 入力：
//   nrf_fall : ロゴ番号*2 + fall時は1
//   type     : ALL=すべて  VALID=有効ロゴ
//   final    : false=通常  true=最終出力
// 返り値： ロゴ位置取得結果（取得時=true）
//   logopt   : ロゴ位置情報
//---------------------------------------------------------------------
bool JlsDataset::getNrfptOutNext(NrfCurrent &logopt, LogoSelectType type, bool final){
	//--- initialize ---
	if (logopt.valid == false){
		logopt.nrfRise = -1;
		logopt.nrfFall = -1;
		logopt.valid = true;
	}
	//--- backup ---
	logopt.nrfLastRise = logopt.nrfRise;
	logopt.nrfLastFall = logopt.nrfFall;
	logopt.msecLastRise = logopt.msecRise;
	logopt.msecLastFall = logopt.msecFall;
	//--- rise ---
	logopt.nrfRise = getNrfMsecOutNextLogo(logopt.msecRise, logopt.nrfFall, LOGO_EDGE_RISE, type, final);
	if (logopt.nrfRise >= 0){
		logopt.nrfFall = getNrfMsecOutNextLogo(logopt.msecFall, logopt.nrfRise, LOGO_EDGE_FALL, type, final);
		if ( logopt.nrfFall < 0 ){
			logopt.nrfFall  = sizeDataLogo()*2-1;
			logopt.msecFall = getMsecTotalMax();
		}
	}
	else{
		logopt.nrfFall = -1;
		return false;
	}
	return true;
}
// unit設定も考慮した出力用の１つ後のロゴ位置取得
Nrf JlsDataset::getNrfMsecOutNextLogo(Msec& msecOut, Nrf nrf, LogoEdgeType edge, LogoSelectType type, bool final){
	int msec_val_cont = 80;					// 同一ロゴとみなす間隔msec

	int r = -1;
	Msec msecRef = ( nrf >= 0 )? getMsecLogoNrf(nrf) : 0;
	Nrf nrfNext = nrf;
	bool flag_end = false;
	while(flag_end == false){
		nrfNext = getNrfOutDirLogo(nrfNext, SEARCH_DIR_NEXT, edge, type, final);
		r = nrfNext;
		msecOut = getMsecLogoNrf(nrfNext);
		flag_end = true;
		if ( nrfNext < 0 || final == false ){
			break;
		}
		//--- 最終出力用処理 ---
		DataLogoRecord dtlogo1;
		getRecordLogo(dtlogo1, nlgFromNrf(nrfNext));
		bool flagRise = isLogoEdgeRiseFromNrf(nrfNext);
		msecOut = ( flagRise )? dtlogo1.result_rise : dtlogo1.result_fall;
		if ( msecOut < msecRef ){
			flag_end = false;
		}
		//--- 次のロゴが同一ロゴか確認 ---
		if ( flagRise == false ){
			Nrf nrfRise = getNrfOutDirLogo(nrfNext, SEARCH_DIR_NEXT, LOGO_EDGE_RISE, type, final);
			if ( nrfRise >= 0 ){
				DataLogoRecord dtlogo2;
				getRecordLogo(dtlogo2, nlgFromNrf(nrfRise));
				if ( dtlogo2.result_rise - dtlogo1.result_fall < msec_val_cont ){
					if ( (extOpt.nLgExact & 0x02) == 0 ){
						msecOut = cnv.getMsecAdjustFrmFromMsec(dtlogo2.result_rise, -1);	// 切れ目なしに補正
					}
					if ( dtlogo1.unit_fall == false && dtlogo2.unit_rise == false ){
						flag_end = false;	// 同一ロゴのため次を検索
						nrfNext = nrfRise;
					}
				}
			}
		}
	}
	return r;
}
// 出力用の次のロゴ位置取得
Nrf JlsDataset::getNrfOutDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type, bool final){
	int size_logo = sizeDataLogo();
	int r = -1;
	bool flag_end = false;
	bool check_rise = isLogoEdgeRise(edge);
	bool check_fall = isLogoEdgeFall(edge);
	int i = nrf;
	int step = (dr == SEARCH_DIR_NEXT)? +1 : -1;
	while(flag_end == false){
		i += step;
		if (i >= 0 && i < size_logo*2){
			int nlg_i  = nlgFromNrf(i);
			int edge_i = edgeFromNrf(i);
			DataLogoRecord dtlogo;
			getRecordLogo(dtlogo, nlg_i);
			if (edge_i == LOGO_EDGE_RISE && check_rise){
				if ( (final == true && dtlogo.outtype_rise == LOGO_RESULT_DECIDE) ||
					 (final == false && (isValidLogoRise(nlg_i) || type == LOGO_SELECT_ALL)) ){
					r = i;
					flag_end = true;
				}
			}
			else if (edge_i == LOGO_EDGE_FALL && check_fall){
				if ( (final == true && dtlogo.outtype_fall == LOGO_RESULT_DECIDE) ||
					 (final == false && (isValidLogoFall(nlg_i) || type == LOGO_SELECT_ALL)) ){
					r = i;
					flag_end = true;
				}
			}
		}
		else{
			flag_end = true;
		}
	};
	return r;
}



//=====================================================================
// 前後データ取得処理（無音シーンチェンジ）
//=====================================================================

//---------------------------------------------------------------------
// 次の構成位置を取得
// 入力：
//   nsc  : シーンチェンジ番号
//   dr   : 検索方向（前側 / 後側）
//   chap_th : 区切りとする状態閾値
// 出力：
//   返り値： 手前構成区切りの位置番号（-1の時は該当なし）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirScpChap(Nsc nsc, SearchDirType dr, ScpChapType chap_th){
	if (dr == SEARCH_DIR_PREV){
		return getNscPrevScpChap(nsc, chap_th);
	}
	return getNscNextScpChap(nsc, chap_th);
}

// １つ前の位置取得
Nsc JlsDataset::getNscPrevScpChap(Nsc nsc, ScpChapType chap_th){
	ScpEndType edgetype = SCP_END_NOEDGE;
	return getNscPrevScpChapEdge(nsc, chap_th, edgetype);
}
Nsc JlsDataset::getNscPrevScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int nEdge = ( noedge == SCP_END_NOEDGE )? 1 : 0;
	int r = -1;
	if (nsc-1 < num_scpos){
		for(int i=nsc-1; i>=0+nEdge; i--){
			ScpChapType chap_i = m_scp[i].chap;
			if (chap_i >= chap_th){
				r = i;
				break;
			}
		}
	}
	return r;
}
// １つ後の位置取得
Nsc JlsDataset::getNscNextScpChap(Nsc nsc, ScpChapType chap_th){
	ScpEndType edgetype = SCP_END_NOEDGE;
	return getNscNextScpChapEdge(nsc, chap_th, edgetype);
}
// １つ後の位置取得
Nsc JlsDataset::getNscNextScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int nEdge = ( noedge == SCP_END_NOEDGE )? 1 : 0;
	int r = -1;
	if (nsc+1 >= 0){
		for(int i=nsc+1; i<num_scpos-nEdge; i++){
			ScpChapType chap_i = m_scp[i].chap;
			if (chap_i >= chap_th || i == 0 || i == num_scpos-1){
				r = i;
				break;
			}
		}
	}
	return r;
}

//---------------------------------------------------------------------
// 次の構成位置を取得（2構成以上のCMは結合）
// 入力：
//   nsc  : シーンチェンジ番号
//   noedge : 0フレームと最終フレームを除く設定
// 出力：
//   返り値： 次の構成区切り位置番号（-1の時は該当なし）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscNextScpCheckCmUnit(Nsc nsc, ScpEndType noedge){
	int nsc_next = nsc;
	int nsc_cur;
	ScpArType arstat = SCP_AR_UNKNOWN;
	//--- 2構成以上のCMは最後の位置まで移動 ---
	do{
		nsc_cur  = nsc_next;
		nsc_next = getNscNextScpChapEdge(nsc_cur, SCP_CHAP_DECIDE, noedge);
		arstat = getScpArstat(nsc_next);
	} while(nsc_next > 0 &&
			(arstat == SCP_AR_N_AUNIT ||
			(arstat == SCP_AR_N_BUNIT && nsc_cur == nsc)));
	//--- 次の位置に設定 ---
	if (nsc_cur == nsc){
		nsc_cur = nsc_next;
	}
	return nsc_cur;
}
Nsc JlsDataset::getNscPrevScpCheckCmUnit(Nsc nsc, ScpEndType noedge){
	int nsc_prev = nsc;
	int nsc_cur;
	bool overEdge;
	ScpArType arstat = SCP_AR_UNKNOWN;
	//--- 2構成以上のCMは最初の位置まで移動 ---
	do{
		nsc_cur  = nsc_prev;
		overEdge = ( nsc_cur == sizeDataScp() && noedge == SCP_END_NOEDGE );
		ScpEndType adaptEdge = ( overEdge )? SCP_END_EDGEIN : noedge;
		nsc_prev = getNscPrevScpChapEdge(nsc_cur, SCP_CHAP_DECIDE, adaptEdge);
		arstat = getScpArstat(nsc_cur);
	} while( nsc_prev > 0 && (arstat == SCP_AR_N_AUNIT || overEdge) );

	// 2構成以上のCMは最初がSCP_AR_N_BUNIT、以降の連結がSCP_AR_N_AUNIT

	//--- 次の位置に設定 ---
	nsc_cur = nsc_prev;
	return nsc_cur;
}

//---------------------------------------------------------------------
// 次の構成位置を取得
// 入力：
//   nsc  : シーンチェンジ番号
//   dr   : 検索方向（前側 / 後側）
//   noedge : 0フレームと最終フレームを除く設定
// 出力：
//   返り値： 次の構成区切り位置番号（-1の時は該当なし）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirScpDecide(Nsc nsc, SearchDirType dr, ScpEndType noedge){
	if (dr == SEARCH_DIR_PREV){
		return getNscPrevScpDecide(nsc, noedge);
	}
	return getNscNextScpDecide(nsc, noedge);
}

// １つ前の位置取得
int JlsDataset::getNscPrevScpDecide(int nsc, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	int nstart = nsc-1;
	if (nstart == num_scpos-1 && noedge == SCP_END_NOEDGE) nstart = num_scpos-2;
	for(int i=nstart; i>=noedge; i--){
		if (getPriorScp(i) >= SCP_PRIOR_DECIDE || i == 0 || i == num_scpos-1){
			r = i;
			break;
		}
	}
	return r;
}

// １つ後の位置取得
Nsc JlsDataset::getNscNextScpDecide(Nsc nsc, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	int nstart = nsc+1;
	if (nstart == 0 && noedge == SCP_END_NOEDGE) nstart = 1;
	for(int i=nstart; i<num_scpos-noedge; i++){
		if (getPriorScp(i) >= SCP_PRIOR_DECIDE || i == 0 || i == num_scpos-1){
			r = i;
			break;
		}
	}
	return r;
}

// 表示用の構成
Nsc JlsDataset::getNscNextScpDisp(Nsc nsc, ScpEndType noedge){
	Msec msecFrom = -1;
	if ( isScpChapTypeDecideFromNsc(nsc) ){		// 元位置が区切りなら参照元とする
		 msecFrom = getMsecScp(nsc);
	}
	int r = nsc;
	bool cont = false;		// 通常は1回で終了
	do{
		if ( isAutoModeUse() ){
			r = getNscNextScpCheckCmUnit(r, noedge);
		}
		else{
			r = getNscNextScpDecide(r, noedge);
		}
		cont = false;
		if ( r>=0 && r<sizeDataScp() && msecFrom>=0 ){
			if ( msecFrom >= getMsecScp(r) ){	// 現在位置以下は飛ばす
				cont = true;
			}
		}
	} while( r>=0 && cont );
	return r;
}
Nsc JlsDataset::getNscPrevScpDisp(Nsc nsc, ScpEndType noedge){
	Msec msecFrom = -1;
	if ( isScpChapTypeDecideFromNsc(nsc) ){		// 元位置が区切りなら参照元とする
		msecFrom = getMsecScp(nsc);
	}
	int rdet = -1;
	int r = nsc;
	bool cont = false;
	do{
		if ( isAutoModeUse() ){
			r = getNscPrevScpCheckCmUnit(r, noedge);
		}
		else{
			r = getNscPrevScpDecide(r, noedge);
		}
		cont = false;
		if ( r >= 0 ){
			if ( msecFrom < 0 ){	// 参照元なし
				rdet = r;		// 結果位置にする
				msecFrom = getMsecScp(r);
				cont = true;
			}else{
				if ( msecFrom <= getMsecScp(r) ){	// 現在位置以上は飛ばす
					rdet = r;
					cont = true;
				}else if ( rdet < 0 ){
					rdet = r;
				}
			}
		}
	} while( r>=0 && cont );
	return rdet;
}

// ミリ秒から区切りが指定数前の位置取得
Nsc JlsDataset::getNscPrevScpDispFromMsecCount(Msec msec, int nCount, bool clip){
	Msec msecSrc = msec - msecValExact;
	WideMsec wmsecTmp;
	wmsecTmp.just  = msecSrc;
	wmsecTmp.early = 0;
	wmsecTmp.late  = msecSrc;
	Nsc nscT = pdata->getNscFromWideMsecFull(wmsecTmp, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
	nscT += 1;
	for(int i=1; i<=nCount; i++){
		nscT = pdata->getNscPrevScpDisp(nscT, SCP_END_EDGEIN);
		if ( nscT < 0 ) break;
	}
	if ( clip && nscT < 0 ) nscT = 0;
	return nscT;
}
// ミリ秒から区切りが指定数後の位置取得
Nsc JlsDataset::getNscNextScpDispFromMsecCount(Msec msec, int nCount, bool clip){
	Msec msecMax = getMsecTotalMax();
	Msec msecSrc = msec + msecValExact;
	WideMsec wmsecTmp;
	wmsecTmp.just  = msecSrc;
	wmsecTmp.early = msecSrc;
	wmsecTmp.late  = msecMax;
	Nsc nscT = pdata->getNscFromWideMsecFull(wmsecTmp, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
	nscT -= 1;
	for(int i=1; i<=nCount; i++){
		nscT = pdata->getNscNextScpDisp(nscT, SCP_END_EDGEIN);
		if ( nscT < 0 ) break;
	}
	if ( clip && nscT < 0 ) nscT = sizeDataScp() - 1;
	return nscT;
}

//=====================================================================
// 位置に対応するデータ取得処理
//=====================================================================

//---------------------------------------------------------------------
// 対象位置が含まれるロゴ番号を取得
// 入力：
//   msec_target : 対象位置
//   edge        : 0=立ち上がりエッジ  1=立ち下がりエッジ  2=両エッジ
// 出力：
//   返り値： ロゴ番号*2 + fall時は1（-1の時は該当なし）
//---------------------------------------------------------------------
Nrf JlsDataset::getNrfLogoFromMsec(Msec msec_target, LogoEdgeType edge){
	int size_logo = sizeDataLogo();
	int r = -1;
	for(int i=0; i<size_logo; i++){
		if ( isLogoEdgeRise(edge) ){
			int msec_s = m_logo[i].rise_l;
			int msec_e = m_logo[i].rise_r;
			if (msec_s <= msec_target && msec_target <= msec_e){
				r = i*2;
				break;
			}
		}
		if ( isLogoEdgeFall(edge) ){
			int msec_s = m_logo[i].fall_l;
			int msec_e = m_logo[i].fall_r;
			if (msec_s <= msec_target && msec_target <= msec_e){
				r = i*2+1;
				break;
			}
		}
	}
	return r;
}
Nrf JlsDataset::getNrfLogoFromMsecResult(Msec msec_target, LogoEdgeType edge, bool result){
	int size_logo = sizeDataLogo();
	int r = -1;
	for(int i=0; i<size_logo; i++){
		if ( isLogoEdgeRise(edge) ){
			if ( result ){
				if ( m_logo[i].outtype_rise == LOGO_RESULT_DECIDE ){
					int msec_s = m_logo[i].result_rise;
					if ( abs(msec_target - msec_s) <= msecValExact ){
						r = i*2;
						break;
					}
				}
			}else{
				int msec_s = m_logo[i].rise_l;
				int msec_e = m_logo[i].rise_r;
				if (msec_s <= msec_target && msec_target <= msec_e){
					r = i*2;
					break;
				}
			}
		}
		if ( isLogoEdgeFall(edge) ){
			if ( result ){
				if ( m_logo[i].outtype_fall == LOGO_RESULT_DECIDE ){
					int msec_s = m_logo[i].result_fall;
					if ( abs(msec_target - msec_s) <= msecValExact ){
						r = i*2+1;
						break;
					}
				}
			}else{
				int msec_s = m_logo[i].fall_l;
				int msec_e = m_logo[i].fall_r;
				if (msec_s <= msec_target && msec_target <= msec_e){
					r = i*2+1;
					break;
				}
			}
		}
	}
	return r;
}

//---------------------------------------------------------------------
// ロゴ切り替わり位置の無音シーンチェンジ取得
// 入力：
//   nrf_target   : 対象ロゴ番号
//   msec_th      : 検索範囲（-1の時は制限なし）
//   chap_th      : 対象とする構成区切り状態閾値
// 出力：
//   返り値： 対象位置に一番近い構成区切りの位置番号（-1の時は該当なし）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscFromNrf(Nrf nrf_target, Msec msec_th, ScpChapType chap_th, bool flat){
	if (nrf_target < 0 || nrf_target >= sizeDataLogo()*2){
		return -1;
	}
	//--- ロゴ可能性範囲取得 ---
	int msec_lg_c = -1;
	int msec_lg_l = -1;
	int msec_lg_r = -1;;
	getMsecLogoNrfWide(msec_lg_c, msec_lg_l, msec_lg_r, nrf_target);
	//--- 一番近い位置の取得 ---
	int nsc_det  = -1;
	int msec_dif_det = -1;
	ScpChapType chap_det;
	int i = 0;
	bool over = false;
	do{
		i = getNscNextScpChap(i, chap_th);
		if (i > 0){
			int msec_i = getMsecScp(i);
			ScpChapType chap_i = getScpChap(i);
			if (flat){
				chap_i = SCP_CHAP_NONE;
			}
			else if (chap_i >= SCP_CHAP_DECIDE){		// 確定なら同じレベルに設定
				chap_i = SCP_CHAP_DECIDE;
			}
			if (msec_i > msec_lg_r + msecValNear3){
				over = true;
			}
			else if (msec_i >= msec_lg_l - msecValNear3){
				int msec_dif_i = abs(msec_i - msec_lg_c);
				if (msec_dif_i <= msec_th || msec_th < 0){
					if (nsc_det < 0 ||
						chap_i > chap_det ||
						(chap_i == chap_det && msec_dif_i < msec_dif_det)){
						nsc_det = i;
						msec_dif_det = msec_dif_i;
						chap_det = chap_i;
					}
				}
			}
		}
	}while(i > 0 && over == false);
	return nsc_det;
}

//---------------------------------------------------------------------
// 対象位置に一番近い位置を検索
// 入力：
//   msec_target  : 対象位置
//   msec_th      : 検索範囲（-1の時は制限なし）
//   chap_th      : 対象とする構成区切り状態閾値
//   noedge       : 0フレームと最終フレームを除く設定
// 出力：
//   返り値： 対象位置に一番近い構成区切りの位置番号（-1の時は該当なし）
//---------------------------------------------------------------------
Nsc JlsDataset::getNscFromMsecFull(Msec msec_target, Msec msec_th, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	Msec msec_min = -1;
	Msec msec_r = -1;
	bool decide_min = false;
	for(int i=noedge; i<num_scpos-noedge; i++){
		int msec_i = m_scp[i].msec;
		ScpChapType chap_i = m_scp[i].chap;
		bool edge = ( i == 0 || i == num_scpos-1);
		if (chap_i >= chap_th || edge){
			//--- 対象位置からの差分最小値箇所を取得 ---
			int msec_dif = abs(msec_target - msec_i);
			if (msec_dif <= msec_th || msec_th < 0){
				bool decide_flag = ( jlsd::isScpChapTypeDecide(chap_i) || edge );
				bool sameloc = ( abs(msec_i - msec_r) <= msecValExact );
				bool prefer = ( sameloc && !decide_min && decide_flag);
				if ( msec_dif < msec_min || msec_min < 0 || prefer ){
					r = i;
					msec_min = msec_dif;
					msec_r   = msec_i;
					decide_min = decide_flag;
				}
			}
//			//--- 対象位置を過ぎたら終了 ---
//			if ( msec_i > msec_target + msecValExact ){
//				break;
//			}
		}
	}
	return r;
}

// 構成区切りを取得
Nsc JlsDataset::getNscFromMsecChap(Msec msec_target, Msec msec_th, ScpChapType chap_th){
	return getNscFromMsecFull(msec_target, msec_th, chap_th, SCP_END_NOEDGE);
}

// 無音SC位置を取得
Nsc JlsDataset::getNscFromMsecMgn(Msec msec_target, Msec msec_th, ScpEndType noedge){
	return getNscFromMsecFull(msec_target, msec_th, SCP_CHAP_DUPE, noedge);
}
// 無音SC位置を取得
Nsc JlsDataset::getNscFromMsecAll(Msec msec_target){
	return getNscFromMsecFull(msec_target, msecValNear2, SCP_CHAP_DUPE, SCP_END_NOEDGE);
}

// 無音SC位置を取得
Nsc JlsDataset::getNscFromMsecAllEdgein(Msec msec_target){
	return getNscFromMsecFull(msec_target, msecValNear2, SCP_CHAP_DUPE, SCP_END_EDGEIN);
}

// 範囲内の無音SC位置を取得
Nsc JlsDataset::getNscFromWideMsecFull(WideMsec wmsec_target, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	Msec msec_min = -1;
	for(int i=noedge; i<num_scpos-noedge; i++){
		Msec msec_i = m_scp[i].msec;
		ScpChapType chap_i = m_scp[i].chap;
		if (chap_i >= chap_th || i == 0 || i == num_scpos-1){
			//--- 対象位置からの差分最小値箇所を取得 ---
			int msec_dif = abs(wmsec_target.just - msec_i);
			if ((wmsec_target.early <= msec_i || wmsec_target.early < 0) &&
				(msec_i <= wmsec_target.late  || wmsec_target.late  < 0)){
				if (msec_dif < msec_min || msec_min < 0){
					r = i;
					msec_min = msec_dif;
				}
			}
			//--- 対象位置を過ぎたら終了 ---
			if (msec_i >= wmsec_target.late){
				break;
			}
		}
	}
	return r;
}
// 範囲内の無音SC位置を取得
Nsc JlsDataset::getNscFromWideMsecByChap(WideMsec wmsec_target, ScpChapType chap_th){
	return getNscFromWideMsecFull(wmsec_target, chap_th, SCP_END_NOEDGE);
}

// 一番近い無音SC位置を取得（表示用の構成区切りのみ）
Nsc JlsDataset::getNscFromMsecDisp(Msec msec_target, Msec msec_th, ScpEndType noedge){
	WideMsec wmsec;
	wmsec.just  = msec_target;
	wmsec.early = msec_target-msec_th;
	wmsec.late  = msec_target+msec_th;
	return getNscFromWideMsecDisp(wmsec, noedge);
}
// 範囲内の無音SC位置を取得（表示用の構成区切りのみ）
Nsc JlsDataset::getNscFromWideMsecDisp(WideMsec wmsec_target, ScpEndType noedge){
	Term term = {};
	pdata->setTermEndtype(term, noedge);	// 端を含めて実施するか
	pdata->setTermForDisp(term, true);		// 表示用の構成
	Nsc r = -1;
	Msec msecMin = -1;
	bool first = true;
	bool cont = getTermNext(term);
	while( cont ){
		Nsc nscNow = ( first )? term.nsc.st : term.nsc.ed;
		Msec msecNow = ( first )? term.msec.st : term.msec.ed;
		Msec msecDif = abs(msecNow - wmsec_target.just);
		if ((wmsec_target.early <= msecNow || wmsec_target.early < 0) &&
		    (msecNow <= wmsec_target.late  || wmsec_target.late  < 0)){
			if (msecDif < msecMin || msecMin < 0){
				r = nscNow;
				msecMin = msecDif;
			}
			//--- 対象位置を過ぎたら終了 ---
			if (msecNow >= wmsec_target.late){
				break;
			}
		}
		//--- 次の位置設定 ---
		if ( first ){
			first = false;
		}else{
			cont = getTermNext(term);
		}
	}
	return r;
}

//---------------------------------------------------------------------
// 開始終了位置に対応する無音シーンチェンジ番号を取得
//---------------------------------------------------------------------
bool JlsDataset::getRangeNscFromRangeMsec(RangeNsc &rnsc, RangeMsec rmsec){
	//--- 位置から正確なロゴ番号を取得する ---
	bool det = true;
	rnsc.st = -1;
	rnsc.ed = -1;

	if (rmsec.st >= 0){
		rnsc.st = getNscFromMsecChap(rmsec.st, pdata->msecValLap2, SCP_CHAP_DECIDE);
		if (rnsc.st < 0){
			det = false;
		}
	}
	if (rmsec.ed >= 0){
		rnsc.ed = getNscFromMsecChap(rmsec.ed, pdata->msecValLap2, SCP_CHAP_DECIDE);
		if (rnsc.ed < 0){
			det = false;
		}
	}
	return det;
}





//=====================================================================
// 状態設定
//=====================================================================

//---------------------------------------------------------------------
// ロゴレベル設定・読み出し
//---------------------------------------------------------------------
void JlsDataset::setLevelUseLogo(int level){
	m_levelUseLogo = level;
}
int JlsDataset::getLevelUseLogo(){
	return m_levelUseLogo;
}

//---------------------------------------------------------------------
// 従来から必要な初期調整
//---------------------------------------------------------------------
void JlsDataset::setFlagSetupAdj(bool flag){
	m_flagSetupAdj = flag;
}

//---------------------------------------------------------------------
// 自動構成推測のモード設定
//---------------------------------------------------------------------
void JlsDataset::setFlagAutoMode(bool flag){
	if (flag){
		m_flagSetupAuto = 2;
	}
	else{
		m_flagSetupAuto = 1;
	}
}



//=====================================================================
// 状態判定
//=====================================================================

//---------------------------------------------------------------------
// 読み込んだロゴが存在するか判断
//---------------------------------------------------------------------
bool JlsDataset::isExistLogo(){
	return (pdata->extOpt.flagNoLogo > 0)? false : true;
}

//---------------------------------------------------------------------
// ロゴをが存在するか判断
//---------------------------------------------------------------------
bool JlsDataset::isUnuseLogo(){
	return (m_levelUseLogo == CONFIG_LOGO_LEVEL_UNUSE_ALL)? true : false;
}
bool JlsDataset::isUnuseLevelLogo(){
	return (m_levelUseLogo < CONFIG_LOGO_LEVEL_USE_LOW)? true : false;
}

//---------------------------------------------------------------------
// Auto構成が初期状態か確認
//---------------------------------------------------------------------
bool JlsDataset::isSetupAdjInitial(){
	return (m_flagSetupAdj == 0)? true : false;
}

//---------------------------------------------------------------------
// Auto構成が初期状態か確認
//---------------------------------------------------------------------
bool JlsDataset::isAutoModeInitial(){
	return (m_flagSetupAuto == 0)? true : false;
}

//---------------------------------------------------------------------
// Auto構成を使用するモードか確認
//---------------------------------------------------------------------
bool JlsDataset::isAutoModeUse(){
	return (m_flagSetupAuto == 2)? true : false;
}

//---------------------------------------------------------------------
// 実ロゴを使わず構成推測ロゴが有効なモードか確認
//---------------------------------------------------------------------
bool JlsDataset::isAutoLogoOnly(){
	return (m_flagSetupAuto > 0 && isUnuseLevelLogo())? true : false;
}

//---------------------------------------------------------------------
// 対象位置のロゴが有効か判断（ロゴ立ち上がり）
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoRise(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		if (m_logo[nlg].outtype_rise == LOGO_RESULT_NONE || m_logo[nlg].outtype_rise == LOGO_RESULT_DECIDE){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// 対象位置のロゴが有効か判断（ロゴ立ち下がり）
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoFall(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		if (m_logo[nlg].outtype_fall == LOGO_RESULT_NONE || m_logo[nlg].outtype_fall == LOGO_RESULT_DECIDE){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// 対象位置のロゴが有効か判断（ロゴ立ち下がり）
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoNrf(Nrf nrf){
	Nlg nlg = nlgFromNrf(nrf);
	if (jlsd::isLogoEdgeRiseFromNrf(nrf)){
		return isValidLogoRise(nlg);
	}
	return isValidLogoFall(nlg);
}


//---------------------------------------------------------------------
// 推測ロゴ分割箇所か判断
//---------------------------------------------------------------------
bool JlsDataset::isElgDivScpForAll(Nsc nsc, bool flag_border, bool flag_out){
	bool ret = false;
	if ( getScpChap(nsc) == SCP_CHAP_DUNIT ){	// 分割構成指定
		int  nsc2     = getNscNextScpDecide(nsc, SCP_END_EDGEIN);
		bool inlogo_p = isElgInScpForAll(nsc,  flag_border, flag_out);
		bool inlogo_n = isElgInScpForAll(nsc2, flag_border, flag_out);
		if ( inlogo_p && inlogo_n ){	// 前後ともにロゴ扱い領域
			ret = true;
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// 直前が推測構成のロゴ扱い構成か判断（内部構築用）
//---------------------------------------------------------------------
bool JlsDataset::isElgInScp(Nsc nsc){
	return isElgInScpForAll(nsc, false, false);
}

//---------------------------------------------------------------------
// 直前が推測構成のロゴ扱い構成か判断（選択付き）
//---------------------------------------------------------------------
bool JlsDataset::isElgInScpForAll(Nsc nsc, bool flag_border, bool flag_out){
	bool ret = false;
	int num_scpos = sizeDataScp();
	if (nsc > 0 && nsc < num_scpos){
		//--- 構成区切りでなければ次の位置取得 ---
		if (isScpChapTypeDecide( getScpChap(nsc) ) == false){
			if (nsc < num_scpos-1){
				nsc = getNscNextScpDecide(nsc, SCP_END_EDGEIN);
			}
		}
		//--- ロゴ扱い構成か判断 ---
		ret = isLabelLogoFromNsc(nsc, flag_border, flag_out);
	}
	return ret;
}

//---------------------------------------------------------------------
// 指定位置のAuto区切りが確定状態か判断
//---------------------------------------------------------------------
bool JlsDataset::isScpChapTypeDecideFromNsc(Nsc nsc){
	ScpChapType chap_nsc = getScpChap(nsc);
	return jlsd::isScpChapTypeDecide(chap_nsc);
}

//---------------------------------------------------------------------
// 指定位置の無音構成が動きなしか判断
//---------------------------------------------------------------------
bool JlsDataset::isStillFromMsec(Msec msec_target){
	int num_scpos = sizeDataScp();
	bool det = true;
	bool over = false;
	int i = 1;
	while(i < num_scpos-1 && det == false && over == false){
		Msec msec_s = m_scp[i].msmute_s;
		Msec msec_e = m_scp[i].msmute_e;
		if (msec_s <= msec_target && msec_target <= msec_e){
			if (m_scp[i].still == 0){
				det = false;
			}
		}
		if (msec_s > msec_target){
			over = true;
		}
		i ++;
	}
	return det;
}

//---------------------------------------------------------------------
// 指定位置が無音構成か判断
//---------------------------------------------------------------------
bool JlsDataset::isSmuteFromMsec(Msec msec_target){
	int num_scpos = sizeDataScp();
	bool det = false;
	bool over = false;
	int i = 1;
	while(i < num_scpos-1 && det == false && over == false){
		int msec_s = m_scp[i].msmute_s;
		int msec_e = m_scp[i].msmute_e;
		if (msec_s <= msec_target && msec_target <= msec_e){
			det = true;
		}
		if (msec_s > msec_target){
			over = true;
		}
		i ++;
	}
	return det;
}

//---------------------------------------------------------------------
// ２地点が同じ無音区間のデータか判断
//---------------------------------------------------------------------
bool JlsDataset::isSmuteSameArea(Nsc nsc1, Nsc nsc2){
	int num_scpos = sizeDataScp();
	if (nsc1 > 0 && nsc1 < num_scpos-1 && nsc2 > 0 && nsc2 < num_scpos-1){
		if (m_scp[nsc1].msmute_s == m_scp[nsc2].msmute_s &&
			m_scp[nsc1].msmute_e == m_scp[nsc2].msmute_e){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// 範囲を限定
// 入力：
//    rmsec  : 限定する範囲
// 入出力：
//    wmsec  : 対象データ（範囲なしの時は書き換えない）
// 出力:
//   返り値： false=範囲なし  true=範囲設定
//---------------------------------------------------------------------
bool JlsDataset::limitWideMsecFromRange(WideMsec& wmsec, RangeMsec rmsec){
	if (wmsec.late  < rmsec.st && wmsec.late  >= 0 && rmsec.st >= 0) return false;
	if (wmsec.early > rmsec.ed && wmsec.early >= 0 && rmsec.ed >= 0) return false;
	if (rmsec.st >= 0){
		if (wmsec.early < rmsec.st) wmsec.early = rmsec.st;
		if (wmsec.just  < rmsec.st) wmsec.just  = rmsec.st;
	}
	if (rmsec.ed >= 0){
		if (wmsec.late > rmsec.ed || wmsec.late < 0) wmsec.late  = rmsec.ed;
		if (wmsec.just > rmsec.ed || wmsec.just < 0) wmsec.just  = rmsec.ed;
	}
	return true;
}



//=====================================================================
// Term構成処理
//=====================================================================

//---------------------------------------------------------------------
// 検索時に端を含めるか選択
//---------------------------------------------------------------------
void JlsDataset::setTermEndtype(Term &term, ScpEndType endtype){
	term.endfix = true;
	term.endtype = endtype;
}
//---------------------------------------------------------------------
// 内部用ではなく表示用の構成にするか選択
//---------------------------------------------------------------------
void JlsDataset::setTermForDisp(Term &term, bool flag){
	term.disp = flag;
}
//---------------------------------------------------------------------
// 次の構成を取得
//---------------------------------------------------------------------
bool JlsDataset::getTermNext(Term &term){
	bool ret = false;
	if (!term.valid){				// 初回実行
		term.valid = true;
		if (!term.endfix){
			term.endfix = true;
			term.endtype = SCP_END_NOEDGE;
		}
		if (term.ini > 0){		// 開始位置設定時
			term.nsc.ed = term.ini;
		}
		else{
			term.nsc.ed = -1;
			if ( term.disp ){
				term.nsc.ed = pdata->getNscNextScpDisp(term.nsc.ed, term.endtype);
			}else{
				term.nsc.ed = pdata->getNscNextScpDecide(term.nsc.ed, term.endtype);
			}
		}
		term.msec.ed = pdata->getMsecScp(term.nsc.ed);;
	}
	term.nsc.st = term.nsc.ed;
	term.msec.st = term.msec.ed;
	if ( term.disp ){
		term.nsc.ed = pdata->getNscNextScpDisp(term.nsc.ed, term.endtype);
	}else{
		term.nsc.ed = pdata->getNscNextScpDecide(term.nsc.ed, term.endtype);
	}
	term.msec.ed = pdata->getMsecScp(term.nsc.ed);
	if (term.nsc.ed >= 0){
		ret = true;
	}
	return ret;
}

//---------------------------------------------------------------------
// 前の構成を取得
//---------------------------------------------------------------------
bool JlsDataset::getTermPrev(Term &term){
	bool ret = false;
	if (!term.valid){				// 初回実行
		term.valid = true;
		if (!term.endfix){
			term.endfix = true;
			term.endtype = SCP_END_NOEDGE;
		}
		if (term.ini > 0){		// 開始位置設定時
			term.nsc.st = term.ini;
		}
		else{
			term.nsc.st = pdata->sizeDataLogo();
			if ( term.disp ){
				term.nsc.st = pdata->getNscPrevScpDisp(term.nsc.st, term.endtype);
			}else{
				term.nsc.st = pdata->getNscPrevScpDecide(term.nsc.st, term.endtype);
			}
		}
		term.msec.st = pdata->getMsecScp(term.nsc.st);
	}
	term.nsc.ed = term.nsc.st;
	term.msec.ed = term.msec.st;
	if ( term.disp ){
		term.nsc.st = pdata->getNscPrevScpDisp(term.nsc.st, term.endtype);
	}else{
		term.nsc.st = pdata->getNscPrevScpDecide(term.nsc.st, term.endtype);
	}
	term.msec.st = pdata->getMsecScp(term.nsc.st);
	if (term.nsc.st >= 0){
		ret = true;
	}
	return ret;
}

//---------------------------------------------------------------------
// 各要素の設定・取得
//---------------------------------------------------------------------
ScpArType JlsDataset::getScpArstat(Term term){
	return pdata->getScpArstat(term.nsc.ed);
}
ScpArExtType JlsDataset::getScpArext(Term term){
	return pdata->getScpArext(term.nsc.ed);
}
void JlsDataset::setScpArstat(Term term, ScpArType arstat){
	pdata->setScpArstat(term.nsc.ed, arstat);
}
void JlsDataset::setScpArext(Term term, ScpArExtType arext){
	pdata->setScpArext(term.nsc.ed, arext);
}

//---------------------------------------------------------------------
// CM構成か判別
//---------------------------------------------------------------------
bool JlsDataset::isScpArstatCmUnit(Term term){
	ScpArType arstat_term = getScpArstat(term);
	if ((arstat_term == SCP_AR_N_UNIT)  ||
		(arstat_term == SCP_AR_N_AUNIT) ||
		(arstat_term == SCP_AR_N_BUNIT)){
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
// 検索範囲内にあるか判別
//---------------------------------------------------------------------
bool JlsDataset::checkScopeTerm(Term term, RangeMsec scope){
	int msec_spc  = pdata->msecValSpc;
	return (term.msec.st >= scope.st - msec_spc && term.msec.ed <= scope.ed + msec_spc)? true : false;
}

bool JlsDataset::checkScopeRange(RangeMsec bounds, RangeMsec scope){
	int msec_spc  = pdata->msecValSpc;
	return (bounds.st >= scope.st - msec_spc && bounds.ed <= scope.ed + msec_spc)? true : false;
}


//=====================================================================
// カスタムロゴ作成
//=====================================================================

//---------------------------------------------------------------------
// 指定設定でロゴ位置リストを作成して返す（内部データは変更しない）
//---------------------------------------------------------------------
void JlsDataset::trialClogo(vector<WideMsec>& listWmsec, LogoCustomType custom){
	makeClogoMain(listWmsec, custom);
}
//---------------------------------------------------------------------
// 指定設定でロゴ位置リストを作成・格納
//---------------------------------------------------------------------
void JlsDataset::makeClogo(LogoCustomType custom){
	makeClogoMain(m_listClogo, custom);
	m_customLogo = custom;		// 設定値も保管
}
// 指定設定でロゴ位置リストを作成する共通処理
void JlsDataset::makeClogoMain(vector<WideMsec>& listWmsec, LogoCustomType& custom){
	if ( custom.extLogo && (isAutoModeUse() == false) ){
		custom.extLogo = false;		// AutoModeでなければ推測ロゴは使わない
	}
	if ( custom.extLogo ){
		makeClogoFromVirtual(listWmsec, custom);
	}else{
		makeClogoFromReal(listWmsec, custom);
	}
}
// 指定設定（実際のロゴ）でロゴ位置リストを作成
void JlsDataset::makeClogoFromReal(vector<WideMsec>& listWmsec, LogoCustomType custom){
	listWmsec.clear();		// リスト初期化

	LogoSelectType lgsel = LogoSelectType::LOGO_SELECT_VALID;
	if ( custom.selectAll ){		// 設定値
		lgsel = LogoSelectType::LOGO_SELECT_ALL;
	}
	bool final = custom.final;		// 設定値
	//--- 実際のロゴ位置をロゴ番号とする場合のロゴ位置 ---
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = getNrfptOutNext(logopt, lgsel, final);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			if ( final ){
				wmsecRise = { logopt.msecRise, logopt.msecRise, logopt.msecRise};
				wmsecFall = { logopt.msecFall, logopt.msecFall, logopt.msecFall};
			}else{
				pdata->getWideMsecLogoNrf(wmsecRise, logopt.nrfRise);
				pdata->getWideMsecLogoNrf(wmsecFall, logopt.nrfFall);
			}
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
		}
	}
}
// 指定設定（推測ロゴ）でロゴ位置リストを作成
void JlsDataset::makeClogoFromVirtual(vector<WideMsec>& listWmsec, LogoCustomType custom){
	listWmsec.clear();		// リスト初期化

	//--- 推測構成変化点をロゴ番号とする場合のロゴ位置 ---
	ElgCurrent elgTmp = {};
	elgTmp.border  = custom.border;		// 設定値
	elgTmp.outflag = custom.final;		// 設定値
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getElgptNext(elgTmp);
		if ( flag_cont ){
			WideMsec wmsecRise = getWideMsecScp(elgTmp.nscRise);
			WideMsec wmsecFall = getWideMsecScp(elgTmp.nscFall);
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
		}
	}
}
//---------------------------------------------------------------------
// カスタムロゴリストを取得
//---------------------------------------------------------------------
//--- 設定取得 ---
LogoCustomType JlsDataset::getClogoCustom(){
	return m_customLogo;
}
//--- リスト取得 ---
vector<WideMsec> JlsDataset::getClogoList(){
	return m_listClogo;
}
//--- サイズ取得 ---
int JlsDataset::sizeClogoList(){
	return (int)m_listClogo.size();
}
//---------------------------------------------------------------------
// 現在位置がカスタムロゴの何番目に一番近いか取得
// 返り値：
//   -2    : データなし
//   -1    : 最初の立ち上がりより手前
//   0以上 : ロゴリスト番号（偶数：立上り、奇数：立下り）
//  最大番号+1  : 最後の立ち下がりより後
//---------------------------------------------------------------------
int JlsDataset::getClogoNumNear(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
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
	//--- 両エッジ検出対応 ---
	if ( edge == LogoEdgeType::LOGO_EDGE_BOTH ){
		if ( loc < 0 ){
			if ( loc == -1 ) loc = 0;
		}else if ( loc >= nmax ){
			if ( loc == nmax ) loc = nmax-1;
		}else{
			Msec msecRef = listWmsec[loc].just;
			int step = ( msecLogo < msecRef )? -1 : +1;
			if ( 0 <= loc+step && loc+step < nmax ){
				if ( abs(listWmsec[loc+step].just - msecLogo) < abs(msecRef - msecLogo) ){
					loc = loc+step;
				}
			}
		}
	}
	return loc;
}
//---------------------------------------------------------------------
// 現在位置から１つ前のカスタムロゴを取得（リスト番号）
//---------------------------------------------------------------------
int JlsDataset::getClogoNumPrev(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
	if ( listWmsec.empty() ){
		return -2;
	}
	Msec msecLogoRev = msecLogo - getClogoMsecMgn();
	int locRef = getClogoNumNear(msecLogoRev, edge);
	int nmax = (int)listWmsec.size();
	int step = ( edge == LogoEdgeType::LOGO_EDGE_BOTH )? -1 : -2;
	if ( locRef < 0 || locRef >= nmax+1 ){
		return locRef;
	}else if ( locRef == nmax ){
		if ( isLogoEdgeFall(edge) ) return nmax-1;
		return nmax-2;
	}else{
		WideMsec wmsecRef = listWmsec[locRef];
		if ( wmsecRef.late < msecLogoRev ){
			return locRef;
		}
		else if ( locRef+step < 0 ){
			return -1;
		}
	}
	return locRef+step;
}
//---------------------------------------------------------------------
// 現在位置から１つ後のカスタムロゴを取得（リスト番号）
//---------------------------------------------------------------------
int JlsDataset::getClogoNumNext(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
	if ( listWmsec.empty() ){
		return -2;
	}
	Msec msecLogoRev = msecLogo + getClogoMsecMgn();
	int locRef = getClogoNumNear(msecLogoRev, edge);
	int nmax = (int)listWmsec.size();
	int step = ( edge == LogoEdgeType::LOGO_EDGE_BOTH )? +1 : +2;
	if ( locRef < -1 || locRef >= nmax ){
		return locRef;
	}else if ( locRef == -1 ){
		if ( jlsd::isLogoEdgeRise(edge) ) return 0;
		return 1;
	}else{
		WideMsec wmsecRef = listWmsec[locRef];
		if ( msecLogoRev < wmsecRef.early ){
			return locRef;
		}
		else if ( locRef+step >= nmax ){
			return nmax;
		}
	}
	return locRef+step;
}
//---------------------------------------------------------------------
// 指定回数前後
//---------------------------------------------------------------------
int JlsDataset::getClogoNumPrevCount(Msec msecLogo, int nCount){
	int loc = getClogoNumPrev(msecLogo, LOGO_EDGE_BOTH);
	loc -= nCount - 1;
	if ( loc < -1 ) loc = -1;
	return loc;
}
int JlsDataset::getClogoNumNextCount(Msec msecLogo, int nCount){
	int loc = getClogoNumNext(msecLogo, LOGO_EDGE_BOTH);
	if ( loc >= -1 ){
		loc += nCount - 1;
	}
	int nsize = sizeClogoList();
	if ( loc < -1 || loc > nsize ) loc = nsize;
	return loc;
}
//---------------------------------------------------------------------
// ミリ秒単位で対応するカスタムロゴを取得
//---------------------------------------------------------------------
WideMsec JlsDataset::getClogoWmsecFromNum(int num){
	int nmax = sizeClogoList();
	if ( nmax == 0 ) return {-1, -1, -1};
	if ( num == -1 ) return {0, 0, 0};
	if ( num == nmax ){
		Msec ms = getMsecTotalMax();
		return {ms, ms, ms};
	}
	if ( 0 <= num && num < nmax ) return m_listClogo[num];
	return {-1, -1, -1};
}
Msec JlsDataset::getClogoMsecFromNum(int num){
	WideMsec wmsec = getClogoWmsecFromNum(num);
	return wmsec.just;
}
Msec JlsDataset::getClogoMsecNear(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumNear(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
Msec JlsDataset::getClogoMsecPrev(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumPrev(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
Msec JlsDataset::getClogoMsecNext(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumNext(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
//---------------------------------------------------------------------
// カスタムロゴのロゴ情報取得
//---------------------------------------------------------------------
//--- 対象位置がロゴ切替地点か判定 ---
bool JlsDataset::isClogoMsecExist(Msec msecLogo, LogoEdgeType edge){
	for(int i=0; i<sizeClogoList(); i++){
		if ( getClogoMsecFromNum(i) == msecLogo ){
			if ( i%2 == 0 ){
				if ( jlsd::isLogoEdgeRise(edge) ) return true;
			}else{
				if ( jlsd::isLogoEdgeFall(edge) ) return true;
			}
		}
	}
	return false;
}
//--- 実際のロゴ使用判定 ---
bool JlsDataset::isClogoReal(){
	return ( m_customLogo.extLogo == false );
}
//--- 通算ロゴ番号（実際のロゴ） ---
Nrf JlsDataset::getClogoRealNrf(Msec msecLogo, LogoEdgeType edge){
	if ( m_customLogo.extLogo ) return -1;		// 実際のロゴ以外は無効
	if ( msecLogo < 0 ) return -1;
	// 念のため広く番号取得するため最終時は最終以外と両方確認
	if ( m_customLogo.final ){
		Nrf nrfTmp = getNrfLogoFromMsecResult(msecLogo, edge, true);
		if ( nrfTmp >= 0 ) return nrfTmp;
	}
	return getNrfLogoFromMsecResult(msecLogo, edge, false);
}
//--- ロゴに対応する無音シーンチェンジ位置番号 ---
Nsc JlsDataset::getClogoNsc(Msec msecLogo){
	Nsc nscTmp = getNscFromMsecAllEdgein(msecLogo);
	if ( nscTmp < 0 && isClogoReal() ){		// 実際ロゴは広めに無音SC検索
		nscTmp = getNscFromMsecMgn(msecLogo, msecValSpc, ScpEndType::SCP_END_EDGEIN);
	}
	return nscTmp;
}
Msec JlsDataset::getClogoMsecMgn(){
	if ( isClogoReal() ) return msecValSpc;
	return msecValExact;
}
//=====================================================================
// データ挿入
//=====================================================================

//---------------------------------------------------------------------
// ロゴ区切り挿入
//   msec_target : ロゴ区切り挿入位置
//   confirm : 確定も同時に行うか（0=設定のみ  1=確定処理も実行）
//   unit    : 挿入位置だけで独立構成にするか（0=従来ロゴと混合  1=挿入ロゴだけで独立構成）
//   edge    : エッジによる位置補正（0=立ち上がりエッジ  1=立ち下がりエッジ）
// 出力：
//   返り値： 挿入位置番号 挿入しなかった場合は-1
//---------------------------------------------------------------------
Nsc JlsDataset::insertDivLogo(Msec msec_target, bool confirm, bool unit, LogoEdgeType edge){
	bool overlap = true;
	return insertLogoEdge(msec_target, msec_target, overlap, confirm, unit, edge);
}

//---------------------------------------------------------------------
// ロゴデータ挿入
// 入力：
//   msec_st : ロゴデータ挿入開始位置
//   msec_ed : ロゴデータ挿入終了位置
//   overlap : 既存ロゴと重複許可（0=無効  1=重複許可）
//   confirm : 確定も同時に行うか（0=設定のみ  1=確定処理も実行）
//   unit    : 挿入位置だけで独立構成にするか（0=従来ロゴと混合  1=挿入ロゴだけで独立構成）
// 出力：
//   返り値： 挿入位置番号 挿入しなかった場合は-1
//---------------------------------------------------------------------
Nsc JlsDataset::insertLogo(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit){
	return insertLogoEdge(msec_st, msec_ed, overlap, confirm, unit, LOGO_EDGE_RISE);
}

//---------------------------------------------------------------------
// ロゴデータ挿入（区切り対応）
//   edge    : エッジによる位置補正（0=立ち上がりエッジ  1=立ち下がりエッジ）
//---------------------------------------------------------------------
Nsc JlsDataset::insertLogoEdge(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit, LogoEdgeType edge){
	int size_logo = sizeDataLogo();
	Msec wid_ovl = pdata->msecValSpc;

	if ( msec_st < 0 || msec_ed < 0 || msec_st > getMsecTotalMax() || msec_ed > getMsecTotalMax() ){
		return -1;		// エラーチェック
	}
	bool separator = false;
	if ( msec_ed <= msec_st ){		// 範囲ではなく区切り指定
		separator = true;
		msec_ed = msec_st;
		if ( jlsd::isLogoEdgeRise(edge) ){		// start edge
			msec_st = cnv.getMsecAdjustFrmFromMsec(msec_st, -1);
			if ( msec_st < 0 ){
				msec_st = 0;
			}
		}else{
			msec_ed = cnv.getMsecAdjustFrmFromMsec(msec_ed, +1);
			if ( msec_ed > getMsecTotalMax() ){
				msec_ed =getMsecTotalMax();
			}
		}
	}

	// ロゴ挿入箇所を検索
	int num_ins = -1;
	for(int i=1; i<size_logo; i++){
		if (msec_st >= getMsecLogoFall(i-1) && msec_ed <= getMsecLogoRise(i)){
			num_ins = i;
		}
	}
	if (num_ins < 0){
		if (msec_ed <= getMsecLogoRise(0)){
			num_ins = 0;
		}
		else if (msec_st >= getMsecLogoFall(size_logo-1)){
			num_ins = size_logo;
		}
	}
	if (size_logo >= MAXSIZE_LOGO){
		num_ins = -1;
	}

	int total_ins = 1;
	// 挿入箇所と同じ位置の確定情報保存用
	Msec bak_result_rise, bak_result_fall;
	bool flag_bak_rise = false;
	bool flag_bak_fall = false;
	LogoResultType bak_outtype_rise = LOGO_RESULT_NONE;
	LogoResultType bak_outtype_fall = LOGO_RESULT_NONE;
	// overlap許可時の追加検索
	if (overlap && num_ins < 0 && size_logo < MAXSIZE_LOGO){
		for(int i=0; i<size_logo; i++){
			DataLogoRecord dt;
			getRecordLogo(dt, i);
			// 挿入箇所と同じ位置の確定情報保存
			if (abs(msec_st - dt.rise) <= wid_ovl){
				flag_bak_rise    = true;
				bak_outtype_rise = dt.outtype_rise;
				bak_result_rise  = dt.result_rise;
			}
			if (abs(msec_ed - dt.fall) <= wid_ovl){
				flag_bak_fall    = true;
				bak_outtype_fall = dt.outtype_fall;
				bak_result_fall  = dt.result_fall;
			}
			// 挿入ロゴと一致する場合
			if (msec_st >= dt.rise - wid_ovl && msec_st <= dt.rise + wid_ovl &&
				msec_ed >= dt.fall - wid_ovl && msec_ed <= dt.fall + wid_ovl){
				dt.rise         = msec_st;
				dt.rise_l       = msec_st;
				dt.rise_r       = msec_st;
				dt.fall         = msec_ed;
				dt.fall_l       = msec_ed;
				dt.fall_r       = msec_ed;
				dt.fade_rise   = 0;
				dt.fade_fall   = 0;
				dt.intl_rise   = 0;
				dt.intl_fall   = 0;
				dt.stat_rise   = LOGO_PRIOR_DECIDE;
				dt.stat_fall   = LOGO_PRIOR_DECIDE;
				dt.outtype_rise = LOGO_RESULT_NONE;
				dt.outtype_fall = LOGO_RESULT_NONE;
				if (confirm){
					dt.outtype_rise = LOGO_RESULT_DECIDE;
					dt.outtype_fall = LOGO_RESULT_DECIDE;
					dt.result_rise  = msec_st;
					dt.result_fall  = msec_ed;
				}
				setRecordLogo(dt, i);
				return i;		// 更新のみ
			}
			// 挿入ロゴの前半のみロゴと重なる場合
			if (msec_st > dt.rise + wid_ovl && msec_st < dt.fall &&
				msec_ed > dt.fall + wid_ovl){
				dt.fall         = msec_st;
				dt.fall_l       = msec_st;
				dt.fall_r       = msec_st;
				dt.fade_fall    = 0;
				dt.intl_fall    = 0;
				dt.stat_fall    = LOGO_PRIOR_DECIDE;
				dt.outtype_fall = LOGO_RESULT_NONE;
				setRecordLogo(dt, i);
				if (confirm){
					setResultLogoAtNrf(dt.fall, LOGO_RESULT_DECIDE, i*2+1);
				}
			}
			// 挿入ロゴの後半のみロゴと重なる場合
			if (msec_st < dt.rise + wid_ovl &&
				msec_ed > dt.rise && msec_ed < dt.fall - wid_ovl){
				dt.rise         = msec_ed;
				dt.rise_l       = msec_ed;
				dt.rise_r       = msec_ed;
				dt.fade_rise    = 0;
				dt.intl_rise    = 0;
				dt.stat_rise    = LOGO_PRIOR_DECIDE;
				dt.outtype_rise = LOGO_RESULT_NONE;
				setRecordLogo(dt, i);
				if (confirm){
					setResultLogoAtNrf(dt.rise, LOGO_RESULT_DECIDE, i*2);
				}
			}
			// 挿入ロゴ内にロゴ全体が入る場合
			if (msec_st - wid_ovl <= dt.rise && msec_ed + wid_ovl >= dt.fall){
					dt.outtype_rise = LOGO_RESULT_ABORT;			// abort
					dt.outtype_fall = LOGO_RESULT_ABORT;			// abort
					dt.result_rise  = -1;
					dt.result_fall  = -1;
					setRecordLogo(dt, i);
					if (num_ins < 0){
						num_ins = i;
						total_ins = 0;		// 挿入ロゴは上書きする
					}
			}
			// 挿入ロゴ全体がロゴ範囲内に入る場合
			if (msec_st > dt.rise + wid_ovl && msec_ed < dt.fall - wid_ovl){
				num_ins = i;
				total_ins = 2;				// 挿入ロゴが２つになる
			}
			// 挿入位置
			if (num_ins < 0 && msec_st <= dt.rise){
				num_ins = i;
			}
		}
		if (num_ins < 0){
			num_ins = size_logo;
		}
	}
	if (size_logo + total_ins > MAXSIZE_LOGO){
		num_ins = -1;
	}

	// 結果を格納
	int retval = -1;
	if (num_ins >= 0){
		DataLogoRecord dt;
		if (total_ins == 2){		// 既存ロゴ途中にロゴ挿入して元ロゴを２分割する場合
			getRecordLogo(dt, num_ins);
			if ( separator && unit ){		// 区切りの場合はここで-unit設定
				dt.unit_rise = LOGO_UNIT_DIVIDE;
			}
			dt.rise        = msec_ed;
			dt.rise_l      = msec_ed;
			dt.rise_r      = msec_ed;
			dt.fade_rise   = 0;
			dt.intl_rise   = 0;
			dt.stat_rise   = LOGO_PRIOR_DECIDE;
			if (confirm){
				dt.outtype_rise = LOGO_RESULT_DECIDE;
				dt.result_rise  = msec_ed;
			}
			else{
				dt.outtype_rise = LOGO_RESULT_NONE;
				dt.result_rise  = 0;
			}
			insertRecordLogo(dt, num_ins+1);

			getRecordLogo(dt, num_ins);
			if ( separator && unit ){		// 区切りの場合はここで-unit設定
				dt.unit_fall    = LOGO_UNIT_DIVIDE;
			}
			dt.fall        = msec_st;
			dt.fall_l      = msec_st;
			dt.fall_r      = msec_st;
			dt.fade_fall   = 0;
			dt.intl_fall   = 0;
			dt.stat_fall   = LOGO_PRIOR_DECIDE;
			if (confirm){
				dt.outtype_fall = LOGO_RESULT_DECIDE;
				dt.result_fall  = msec_st;
			}
			else{
				dt.outtype_fall = LOGO_RESULT_NONE;
				dt.result_fall  = 0;
			}
			setRecordLogo(dt, num_ins);
			num_ins = num_ins + 1;
		}
		retval = num_ins;
		getRecordLogo(dt, num_ins);
		dt.rise        = msec_st;
		dt.rise_l      = msec_st;
		dt.rise_r      = msec_st;
		dt.fall        = msec_ed;
		dt.fall_l      = msec_ed;
		dt.fall_r      = msec_ed;
		dt.fade_rise   = 0;
		dt.fade_fall   = 0;
		dt.intl_rise   = 0;
		dt.intl_fall   = 0;
		dt.stat_rise   = LOGO_PRIOR_DECIDE;
		dt.stat_fall   = LOGO_PRIOR_DECIDE;
		if (confirm){
			dt.outtype_rise = LOGO_RESULT_DECIDE;
			dt.outtype_fall = LOGO_RESULT_DECIDE;
			dt.result_rise  = msec_st;
			dt.result_fall  = msec_ed;
		}
		else{
			dt.outtype_rise = LOGO_RESULT_NONE;
			dt.outtype_fall = LOGO_RESULT_NONE;
			dt.result_rise  = 0;
			dt.result_fall  = 0;
		}
		if (flag_bak_rise){
			dt.outtype_rise = bak_outtype_rise;
			dt.result_rise  = bak_result_rise;
		}
		if (flag_bak_fall){
			dt.outtype_fall = bak_outtype_fall;
			dt.result_fall  = bak_result_fall;
		}
		dt.unit_rise        = LOGO_UNIT_NORMAL;
		dt.unit_fall        = LOGO_UNIT_NORMAL;
		if (unit){
			dt.unit_rise    = LOGO_UNIT_DIVIDE;
			dt.unit_fall    = LOGO_UNIT_DIVIDE;
		}
		if ( separator == false ){		// 2点指定区間の挿入
			if (total_ins == 0){
				setRecordLogo(dt, num_ins);
			}
			else{
				insertRecordLogo(dt, num_ins);
			}
		}
	}
	return retval;
}


//---------------------------------------------------------------------
// シーンチェンジを挿入
// 入力：
//   msec_dst_s  : 挿入位置
//   msec_dst_bk : 直前終了位置
//   nsc_mute    : 無音位置関係の情報をコピーする無音SC番号（-1の時無効）
//   stat_scpos_dst : 挿入構成に設定する優先度
// 出力：
//   返り値：挿入した場所番号
//---------------------------------------------------------------------
Nsc JlsDataset::insertScpos(Msec msec_dst_s, Msec msec_dst_bk, Nsc nsc_mute, ScpPriorType stat_scpos_dst){
	int num_scpos = sizeDataScp();
	if (num_scpos <= 1){		// シーンチェンジを読み込みできてない場合
		return -1;
	}

	// 挿入場所を検索
	Nsc nsc_ins = 1;
	while( getMsecScp(nsc_ins) < msec_dst_s && nsc_ins < num_scpos - 1){
		nsc_ins ++;
	}

	// 挿入場所を確保
	bool flag_ins = 0;
	if ( getMsecScp(nsc_ins) != msec_dst_s || nsc_ins == num_scpos - 1){
		flag_ins = true;
	}

	// 書き込み位置選択
	DataScpRecord  dtscp;
	if (flag_ins){
		clearRecordScp(dtscp);
		dtscp.msec = msec_dst_s;
		dtscp.msbk = msec_dst_bk;
		if (nsc_mute >= 0 && nsc_mute < num_scpos){
			DataScpRecord dttmp;
			getRecordScp(dttmp, nsc_mute);
			dtscp.msmute_s = dttmp.msmute_s;
			dtscp.msmute_e = dttmp.msmute_e;
		}
		else{
			dtscp.msmute_s = msec_dst_bk;
			dtscp.msmute_e = msec_dst_s;
		}
		dtscp.statpos = stat_scpos_dst;
		insertRecordScp(dtscp, nsc_ins);			// 挿入による更新
	}
	else{
		getRecordScp(dtscp, nsc_ins);
		dtscp.statpos = stat_scpos_dst;
		setRecordScp(dtscp, nsc_ins);				// 書き換えによる更新
	}
	return nsc_ins;
}

//---------------------------------------------------------------------
// 指定位置（ミリ秒）の無音SC番号を取得（LogoExact設定により調整有無選択）
// なければ強制的に作成して設定
// 入力：
//   msec_in : 対象位置（ミリ秒）
//   edge    : 選択エッジ（LOGO_EDGE_RISE=開始側、LOGO_EDGE_FALL=終了側）
//   exact   : false=通常  true=正確な位置指定
// 出力：
//   返り値：対応する場所番号
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsecExact(Msec msec_in, LogoEdgeType edge, bool exact){
	if ( exact ){
		return getNscForceExactFixMsec(msec_in, edge);
	}
	return getNscForceMsec(msec_in, edge);
}
//---------------------------------------------------------------------
// 不要な重複無音シーンチェンジを削除
//---------------------------------------------------------------------
bool JlsDataset::restructScp(){
	if ( !isAutoModeUse() ) return false;	// Auto推測なければ何もしない
	int num_scpos = sizeDataScp();
	Nsc  nscHold = -1;
	Msec msecHold = -1;
	bool decideHold = false;
	bool change = false;
	for(int i=num_scpos-1; i>=0; i--){
		Msec msecCur = getMsecScp(i);
		bool decideCur = jlsd::isScpChapTypeDecide(getScpChap(i));
		if ( i == 0 || i == num_scpos-1 ){
			decideCur = true;
		}
		bool update = false;
		if ( nscHold < 0 ){	// 初回
			update = true;
		}else if ( abs(msecCur - msecHold) > msecValExact ){	// 前と離れていたら次
			update = true;
		}else if ( !decideHold ){	// 同じ位置で後側が未確定なら後側削除
			deleteRecordScp(nscHold);
			change = true;
			update = true;
		}else if ( !decideCur ){	// 同じ位置で後側確定、前側未確定なら前側削除
			deleteRecordScp(i);
			change = true;
			nscHold -= 1;			// 後側は1つ前にずれる
		}
		if ( update ){
			nscHold = i;
			msecHold = msecCur;
			decideHold = decideCur;
		}
	}
	return change;
}
//---------------------------------------------------------------------
// 指定位置（ミリ秒）の無音SC番号を取得（LogoExact設定により調整有無選択）
// なければ強制的に作成して設定
// 入力：
//   msec_in : 対象位置（ミリ秒）
//   edge    : 選択エッジ（LOGO_EDGE_RISE=開始側、LOGO_EDGE_FALL=終了側）
// 出力：
//   返り値：対応する場所番号
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsec(Msec msec_in, LogoEdgeType edge){
	if ( (extOpt.nLgExact & 0x02) != 0 ){		// ロゴ位置調整なし
		return getNscForceExactFixMsec(msec_in, edge);
	}
	return getNscForceMsecOrg(msec_in, edge);
}

//---------------------------------------------------------------------
// 指定位置（ミリ秒）の無音SC番号を取得
// なければ強制的に作成して設定
// 入力：
//   msec_in : 対象位置（ミリ秒）
//   edge    : 選択エッジ（LOGO_EDGE_RISE=開始側、LOGO_EDGE_FALL=終了側）
// 出力：
//   返り値：対応する場所番号
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsecOrg(Msec msec_in, LogoEdgeType edge){
	Msec msec_clr = msecValNear2;			// 重なった所の確定箇所を解除する範囲
	int num_scpos = sizeDataScp();

	if (msec_in <= 0 ){
		return 0;
	}
	if ( msec_in >= getMsecTotalMax() ){
		return num_scpos-1;
	}

	Nsc nsc_nearest = 0;
	Msec difmsec_nearest = 0;
	Nsc nsc_mute = -1;
	//--- 一番近い所を検索 ---
	int i = 1;
	bool flag_search = true;
	while(flag_search){
		DataScpRecord dtscp;
		getRecordScp(dtscp, i);
		Msec msec_i = getMsecScpEdge(i, edge);
		Msec difmsec_i = abs(msec_in - msec_i);
		//--- 一番近い場合の更新 ---
		if (difmsec_nearest > difmsec_i || nsc_nearest == 0){
			//--- 重なっている対象外となった場所を外す ---
			if (nsc_nearest > 0 && difmsec_nearest <= msec_clr){
				setScpChap(nsc_nearest, SCP_CHAP_DUPE);
			}
			nsc_nearest = i;
			difmsec_nearest = difmsec_i;
		}
		else if (difmsec_i <= msec_clr){		// 重なっている所を外す
			setScpChap(i, SCP_CHAP_DUPE);
		}
		//--- 無音期間領域の確認 ---
		if (dtscp.msmute_s <= msec_i && msec_i <= dtscp.msmute_e){
			if (msec_i <= msec_in || nsc_nearest == i){
				nsc_mute = i;
			}
		}
		//--- 次の位置設定 ---
		i ++;
		if (i >= num_scpos-1 || msec_i >= msec_in + msec_clr){
			flag_search = false;
		}
	}
	//--- 挿入位置を設定 ---
	Msec msec_in_s  = msec_in;
	Msec msec_in_bk = msec_in;
	if (edge == LOGO_EDGE_RISE){
		msec_in_bk = cnv.getMsecAdjustFrmFromMsec(msec_in, -1);
	}
	else{
		msec_in_s  = cnv.getMsecAdjustFrmFromMsec(msec_in, +1);
	}
	Nsc nsc_ret = nsc_nearest;
	//--- 既存無音SC箇所か確認 ---
	bool flag_new = true;
	if (nsc_nearest > 0){
		int msec_near_s  = getMsecScp(nsc_nearest);
		int msec_near_bk = getMsecScpBk(nsc_nearest);
		if (msec_near_bk <= msec_in_s && msec_in_bk <= msec_near_s){
			flag_new = false;
		}
	}
	//--- 新規箇所なら挿入 ---
	if (flag_new){
		nsc_ret = insertScpos(msec_in_s, msec_in_bk, nsc_mute, SCP_PRIOR_DECIDE);
	}
	return nsc_ret;
}

//---------------------------------------------------------------------
// 指定位置（ミリ秒）の無音SC番号を取得（位置調整なしで入力を正確な固定位置とする）
// なければ強制的に作成して設定
// 入力：
//   msec_in : 対象位置（ミリ秒）
//   edge    : 選択エッジ（LOGO_EDGE_RISE=開始側、LOGO_EDGE_FALL=終了側）
// 出力：
//   返り値：対応する場所番号
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceExactFixMsec(Msec msec_in, LogoEdgeType edge){
	Msec msec_clr = msecValNear2;			// 重なった所の確定箇所を解除する範囲
	int num_scpos = sizeDataScp();

	if (msec_in <= 0 ){
		return 0;
	}
	if ( msec_in >= getMsecTotalMax() ){
		return num_scpos-1;
	}

	Nsc nsc_nearest = 0;
	Msec difmsec_nearest = 0;
	Nsc nsc_mute = -1;

	int frmIn = cnv.getFrmFromMsec(msec_in);	// 入力位置のフレーム
	bool flagExactNearest = false;	// 同一座標判定
	bool flagFcNearest = false;		// 保持座標は固定
	Msec msecFcPrev = 0;		// 前側にある固定位置
	Msec msecFcOver = 0;		// 後側にある固定位置
	//--- 一番近い所を検索 ---
	int i = 1;
	bool flag_search = true;
	while(flag_search){
		DataScpRecord dtscp;
		getRecordScp(dtscp, i);
		Msec msec_i = getMsecScpEdge(i, edge);
		Msec difmsec_i = abs(msec_in - msec_i);
		ScpChapType chap_i = getScpChap(i);
		bool flagChapFc = ( chap_i == SCP_CHAP_DFORCE || chap_i == SCP_CHAP_DUNIT)? true : false;
		//--- 位置関係 ---
		int frmSt = cnv.getFrmFromMsec(dtscp.msec);
		int frmBk = cnv.getFrmFromMsec(dtscp.msbk);
		bool flagLocSame = false;
		if ( frmSt-2 <= frmIn && frmIn <= frmBk+2 ){
			if ( (frmBk <= frmIn || jlsd::isLogoEdgeRise(edge) == false) &&
			     (frmSt >= frmIn || jlsd::isLogoEdgeFall(edge) == false) ){
				flagLocSame = true;
			}
		}
		//--- 一番近い場合の更新 ---
		if (difmsec_nearest > difmsec_i || nsc_nearest == 0){
			bool flagErase = false;
			bool flagChange;
			if ( flagLocSame ){			// 同一座標
				flagChange = true;
				flagExactNearest = true;
			}else if ( msecFcPrev >= msec_i && msecFcPrev > 0 ){	// 前の固定より前
				flagChange = false;
			}else if ( msecFcOver <= msec_i && msecFcOver > 0 ){	// 後の固定より後
				flagChange = false;
			}else if ( flagChapFc ){	// 変更不可（固定）位置
				flagChange = false;
				if ( msec_i < msec_in && (msecFcPrev < msec_i || msecFcPrev == 0) ){
					msecFcPrev = msec_i;
					if ( nsc_nearest > 0 && nsc_nearest < i ){	// 前の候補は削除する
						flagErase = true;
					}
				}else if ( msec_i > msec_in && (msecFcOver > msec_i || msecFcOver == 0) ){
					msecFcOver = msec_i;
				}
			}else{
				flagChange = true;
			}
			if ( flagChange || flagErase ){
				//--- 重なっている対象外となった場所を外す ---
				if (nsc_nearest > 0 && difmsec_nearest <= msec_clr && flagFcNearest == false){
					setScpChap(nsc_nearest, SCP_CHAP_DUPE);
				}
				nsc_nearest = 0;	// 候補なし
			}
			if ( flagChange ){
				nsc_nearest = i;
				difmsec_nearest = difmsec_i;
				flagFcNearest = flagChapFc;
			}
		}
		else if (difmsec_i <= msec_clr){		// 重なっている所を外す
			if ( flagChapFc == false ){			// 固定でなければ
				setScpChap(i, SCP_CHAP_DUPE);
			}
		}
		//--- 無音期間領域の確認 ---
		if (dtscp.msmute_s <= msec_i && msec_i <= dtscp.msmute_e){
			if (msec_i <= msec_in || nsc_nearest == i){
				nsc_mute = i;
			}
		}
		//--- 次の位置設定 ---
		i ++;
		if (i >= num_scpos-1 || msec_i >= msec_in + msec_clr){
			flag_search = false;
		}
	}
	//--- 挿入位置を設定 ---
	Msec msec_in_s  = msec_in;
	Msec msec_in_bk = msec_in;
	if (edge == LOGO_EDGE_RISE){
		msec_in_bk = cnv.getMsecAdjustFrmFromMsec(msec_in, -1);
	}
	else{
		msec_in_s  = cnv.getMsecAdjustFrmFromMsec(msec_in, +1);
	}
	Nsc nsc_ret = nsc_nearest;
	//--- 既存無音SC箇所か確認 ---
	bool flag_new = true;
	if (nsc_nearest > 0){
		if ( flagExactNearest ){
			flag_new = false;
			//--- 既存内容を修正 ---
			DataScpRecord dtscp;
			getRecordScp(dtscp, nsc_nearest);
			int frmSt = cnv.getFrmFromMsec(dtscp.msec);
			int frmBk = cnv.getFrmFromMsec(dtscp.msbk);
			if ( jlsd::isLogoEdgeRise(edge) ){
				if ( frmSt != frmIn ){
					dtscp.msec = msec_in;
				}
			}
			if ( jlsd::isLogoEdgeFall(edge) ){
				if ( frmBk != frmIn ){
					dtscp.msbk = msec_in;
				}
			}
			setRecordScp(dtscp, nsc_nearest);
		}
	}
	//--- 新規箇所なら挿入 ---
	if (flag_new){
		nsc_ret = insertScpos(msec_in_s, msec_in_bk, nsc_mute, SCP_PRIOR_DECIDE);
	}
	return nsc_ret;
}


//---------------------------------------------------------------------
// ロゴあり/なし変更
//---------------------------------------------------------------------
void JlsDataset::changeLogoOnOff(Nsc nsc, bool logoOn){
	ScpArType arStatOrg = getScpArstat(nsc);
	ScpArType arStatNew;
	switch( arStatOrg ){
		case SCP_AR_L_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_BUNIT;
			break;
		case SCP_AR_L_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		case SCP_AR_L_MIXED :
			arStatNew = (logoOn)? SCP_AR_L_MIXED : SCP_AR_N_OTHER;
			break;
		case SCP_AR_N_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_UNIT;
			break;
		case SCP_AR_N_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		case SCP_AR_N_AUNIT :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_AUNIT;
			break;
		case SCP_AR_N_BUNIT :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_BUNIT;
			break;
		case SCP_AR_B_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_UNIT;
			break;
		case SCP_AR_B_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		default :
			arStatNew = SCP_AR_UNKNOWN;
			break;
	}
	// ロゴありからロゴなしに変更した時は期間でCM判断
	if ( jlsd::isScpArTypeLogo(arStatOrg) && logoOn == false){
		Nsc nscPrev = getNscPrevScpDecide(nsc, SCP_END_EDGEIN);
		if ( isCmLenNscToNsc(nscPrev, nsc) ){
			arStatNew = SCP_AR_N_UNIT;
		}else{
			arStatNew = SCP_AR_N_OTHER;
		}
	}
	setScpArstat(nsc, arStatNew);
	setScpArext(nsc, SCP_AREXT_NONE);
}
//---------------------------------------------------------------------
// 構成変更時の表示用に補正処理
//---------------------------------------------------------------------
void JlsDataset::changeChapDispUnit(Nsc nscFrom, Nsc nscTo, bool cutDivUnit){
	//--- 対象構成の補正 ---
	changeChapDispUnitSub(nscFrom, nscTo, cutDivUnit);
}
void JlsDataset::changeChapDispUnitWithSide(Nsc nscFrom, Nsc nscTo){
	//--- 単位構成内分割のカットなし ---
	bool cutDivUnit = false;
	//--- 対象構成の補正 ---
	changeChapDispUnitSub(nscFrom, nscTo, cutDivUnit);
	//--- 前後構成の補正 ---
	if ( nscFrom >= 0 ){
		Nsc nscPrev = getNscPrevScpDisp(nscFrom, SCP_END_EDGEIN);
		changeChapDispUnitSub(nscPrev, nscFrom, cutDivUnit);
	}
	if ( nscTo >= 0 ){
		Nsc nscNext = getNscNextScpDisp(nscTo, SCP_END_EDGEIN);
		changeChapDispUnitSub(nscTo, nscNext, cutDivUnit);
	}
}
void JlsDataset::changeChapDispUnitSub(Nsc nscFrom, Nsc nscTo, bool cutDivUnit){
	if ( nscFrom < 0 ) return;

	bool flagDiv = false;		// 表示用にない分割途中
	bool flagUnit = false;		// 15秒単位となるCM期間
	Nsc nscCur  = nscFrom;
	Nsc nscDisp = nscFrom;
	Nsc nscPrev = nscFrom;
	while( nscFrom <= nscCur && nscCur <= nscTo && nscCur >= 0 ){
		//--- 次の位置を取得 ---
		if ( nscDisp <= nscCur ){
			nscPrev = nscDisp;
			nscDisp = getNscNextScpDisp(nscCur, SCP_END_EDGEIN);	// 表示用次位置
			flagUnit = isCmLenNscToNsc(nscPrev, nscDisp);
		}
		nscCur = getNscNextScpDecide(nscCur, SCP_END_EDGEIN);
		if ( nscCur < 0 || nscDisp < 0 ) break;

		//--- 表示用にない区切り分割をカット ---
		if ( nscCur != nscDisp && cutDivUnit ){	// 15/30秒構成内CM分割のカット処理
			if ( nscCur < nscTo ){			// 念のため範囲内側の時のみ
				setScpChap(nscCur, SCP_CHAP_NONE);
			}
			continue;
		}
		//--- 構成区切りの補正 ---
		ScpArType arstat = pdata->getScpArstat(nscCur);
		ScpArType arstatRev;
		switch( arstat ){
			case SCP_AR_N_UNIT:
			case SCP_AR_N_OTHER:
			case SCP_AR_N_AUNIT:
			case SCP_AR_N_BUNIT:
				if ( !cutDivUnit && (nscCur != nscDisp || flagDiv) ){
					if ( !flagUnit ){
						arstatRev = SCP_AR_N_OTHER;
					}else if ( flagDiv ){
						arstatRev = SCP_AR_N_AUNIT;
					}else{
						arstatRev = SCP_AR_N_BUNIT;
					}
				}else if ( flagUnit ){
					arstatRev = SCP_AR_N_UNIT;
				}else{
					arstatRev = SCP_AR_N_OTHER;
				}
				break;
			default:
				arstatRev =arstat;
				break;
		}
		if ( arstatRev != arstat ){
			setScpArstat(nscCur, arstatRev);
		}
		flagDiv = ( nscCur != nscDisp );
	}
}
bool JlsDataset::isCmLenNscToNsc(Nsc nscFrom, Nsc nscTo){
	if ( nscFrom < 0 || nscTo < 0 ) return false;
	Msec msec1 = getMsecScp(nscFrom);
	Msec msec2 = getMsecScp(nscTo);
	int n = cnv.getSecFromMsec(abs(msec1 - msec2));
	if ( n > 0 && n <= 120 && (n % 15 == 0) ) return true;
	return false;
}

//=====================================================================
// 推測構成ラベル分別
//=====================================================================
//---------------------------------------------------------------------
// コメント種類を取得
//---------------------------------------------------------------------
bool JlsDataset::isLabelLogoFromNsc(Nsc nsc, bool flagBorder, bool flagOut){
	ComLabelType label = getLabelTypeFromNsc(nsc, flagOut);
	return isLabelLogo(label, flagBorder, flagOut);
}
ComLabelType JlsDataset::getLabelTypeFromNsc(Nsc nsc, bool flagOut){
	ScpArType arstat = getScpArstat(nsc);
	ScpArExtType arext = getScpArext(nsc);
	return getLabelTypeFromStat(arstat, arext, flagOut);
}
// ロゴ扱いの判定
bool JlsDataset::isLabelLogo(ComLabelType label, bool flagBorder, bool flagOut){
	bool logoOn;
	switch( label ){
		case ComLabelType::Mix :
		case ComLabelType::Logo :
		case ComLabelType::AddNEdge :
		case ComLabelType::AddLEdge :
		case ComLabelType::AddTR :
		case ComLabelType::RawTR :
		case ComLabelType::CanTR :
		case ComLabelType::AddSP :
		case ComLabelType::AddEC :
			logoOn = true;
			break;
		case ComLabelType::Bd :
		case ComLabelType::Bd15s :
			logoOn = (flagBorder || flagOut);
			break;
		default :
			logoOn = false;
			break;
	}
	return logoOn;
}
// 種類を取得
ComLabelType JlsDataset::getLabelTypeFromStat(ScpArType arstat, ScpArExtType arext, bool flagOut){
	ComLabelType label;

	switch( arstat ){
		case SCP_AR_N_UNIT :
		case SCP_AR_N_AUNIT :
		case SCP_AR_N_BUNIT :
			label = ComLabelType::CM;	// ":CM"
			break;
		case SCP_AR_N_OTHER :
			label = ComLabelType::NoLogo;	// ":Nologo"
			break;
		case SCP_AR_B_UNIT  :
			label = ComLabelType::Bd15s;	// ":Border15s"
			break;
		case SCP_AR_B_OTHER :
			label = ComLabelType::Bd;	// ":Border"
			break;
		case SCP_AR_L_UNIT :
		case SCP_AR_L_OTHER :
			label = ComLabelType::Logo;	// ":L"
			break;
		case SCP_AR_L_MIXED :
			label = ComLabelType::Mix;	// ":Mix"
			break;
		default :
			label = ComLabelType::None;	// ":"
			break;
	}
	if ( flagOut == false ){
		return label;
	}
	switch( arext ){
		case SCP_AREXT_L_TRKEEP :
			label = ComLabelType::AddTR;	// ":Trailer(add)"
			break;
		case SCP_AREXT_L_ECCUT :
			label = ComLabelType::CutTR;	// ":Trailer(cut)"
			break;
		case SCP_AREXT_L_TRRAW :
			label = ComLabelType::RawTR;	// ":Trailer"
			break;
		case SCP_AREXT_L_TRCUT :
			if (getConfig(ConfigVarType::flagCutTR) == 1){
				label = ComLabelType::CutTR;	// ":Trailer(cut)"
			}
			else{
				label = ComLabelType::CanTR;	// ":Trailer(cut-cancel)"
			}
			break;
		case SCP_AREXT_L_SP :
			if (getConfig(ConfigVarType::flagCutSP) == 1){
				label = ComLabelType::CutSP;	// ":Sponsor(cut)"
			}
			else{
				label = ComLabelType::AddSP;	// ":Sponsor(add)"
			}
			break;
		case SCP_AREXT_L_EC :
			label = ComLabelType::AddEC;	// ":Endcard(add)"
			break;
		case SCP_AREXT_L_LGCUT :
			label = ComLabelType::CutLEdge;	// ":L-Edge(cut)"
			break;
		case SCP_AREXT_L_LGADD :
			label = ComLabelType::AddLEdge;	// ":L-Edge(add)"
			break;
		case SCP_AREXT_N_TRCUT :
			label = ComLabelType::CutNoLogo;	// ":Nologo(cut)"
			break;
		case SCP_AREXT_N_LGCUT :
			label = ComLabelType::CutNEdge;	// ":N-Edge(cut)"
			break;
		case SCP_AREXT_N_LGADD :
			label = ComLabelType::AddNEdge;	// ":N-Edge(add)"
			break;
		default:
			break;
	}
	return label;
}
// 文字列取得
string JlsDataset::getLabelStr(ComLabelType label){
	const char *pstr = "";
	switch( label ){
		case ComLabelType::CM :
			pstr = ":CM";
			break;
		case ComLabelType::NoLogo :
			pstr = ":Nologo";
			break;
		case ComLabelType::CutNoLogo :
			pstr = ":Nologo(cut)";
			break;
		case ComLabelType::CutNEdge :
			pstr = ":N-Edge(cut)";
			break;
		case ComLabelType::CutLEdge :
			pstr = ":L-Edge(cut)";
			break;
		case ComLabelType::CutTR :
			pstr = ":Trailer(cut)";
			break;
		case ComLabelType::CutSP :
			pstr = ":Sponsor(cut)";
			break;
		case ComLabelType::Bd :
			pstr = ":Border";
			break;
		case ComLabelType::Bd15s :
			pstr = ":Border15s";
			break;
		case ComLabelType::Mix :
			pstr = ":Mix";
			break;
		case ComLabelType::Logo :
			pstr = ":L";
			break;
		case ComLabelType::AddNEdge :
			pstr = ":N-Edge(add)";
			break;
		case ComLabelType::AddLEdge :
			pstr = ":L-Edge(add)";
			break;
		case ComLabelType::AddTR :
			pstr = ":Trailer(add)";
			break;
		case ComLabelType::RawTR :
			pstr = ":Trailer";
			break;
		case ComLabelType::CanTR :
			pstr = ":Trailer(cut-cancel)";
			break;
		case ComLabelType::AddSP :
			pstr = ":Sponsor(add)";
			break;
		case ComLabelType::AddEC :
			pstr = ":Endcard(add)";
			break;
		default :
			pstr = ":";
			break;
	}
	string strRet = pstr;
	return strRet;
}



//=====================================================================
// 構成内のロゴ表示期間の取得
//=====================================================================

//---------------------------------------------------------------------
// 範囲内にあるロゴ表示期間の秒数を取得
//---------------------------------------------------------------------
Sec JlsDataset::getSecLogoComponent(Msec msec_s, Msec msec_e){
	//--- 実ロゴデータを使わない場合のロゴ期間設定 ---
	if ( isAutoLogoOnly() ){
		return getSecLogoComponentFromElg(msec_s, msec_e);
	}
	return getSecLogoComponentFromLogo(msec_s, msec_e);
}

// 実ロゴデータから
Sec JlsDataset::getSecLogoComponentFromLogo(Msec msec_s, Msec msec_e){
	NrfCurrent logopt = {};
	Msec sum_msec = 0;
	//--- 構成の端部分を捨てる判別期間 ---
	Msec msec_dif_mid = abs(msec_e - msec_s) / 2;
	Msec msec_dif_min = abs(msec_e - msec_s) / 4;
	if (msec_dif_mid > 11500){
		msec_dif_mid = 11500;
	}
	if (msec_dif_min > 4500){
		msec_dif_min = 4500;
	}
	while( getNrfptNext(logopt, LOGO_SELECT_VALID) ){
		//--- 範囲内にロゴ表示期間がある場合 ---
		if (logopt.msecRise + msecValSpc < msec_e && logopt.msecFall > msec_s + msecValSpc){
			Msec tmp_s = (logopt.msecRise < msec_s)? msec_s : logopt.msecRise;
			Msec tmp_e = (logopt.msecFall > msec_e)? msec_e : logopt.msecFall;
			//--- 端部分の処理(rise) ---
			{
				WideMsec wmsec;
				getWideMsecLogoNrf(wmsec, logopt.nrfRise);
				if (logopt.msecRise < msec_s + msec_dif_mid && logopt.msecFall >= msec_e){
					if (wmsec.early <= msec_s + msecValSpc && tmp_s > msec_s){
						tmp_s = msec_s;
					}
				}
				if (logopt.msecRise > msec_e - msec_dif_min){
					tmp_s = tmp_e;
				}
				else if (logopt.msecRise > msec_e - msec_dif_mid && logopt.msecFall >= msec_e){
					if (wmsec.late >= msec_e - msec_dif_min){
						tmp_s = tmp_e;
					}
				}
			}
			//--- 端部分の処理(fall) ---
			{
				WideMsec wmsec;
				getWideMsecLogoNrf(wmsec, logopt.nrfFall);
				if (logopt.msecFall > msec_e - msec_dif_mid && logopt.msecRise <= msec_s){
					if (wmsec.late >= msec_e - msecValSpc && tmp_e < msec_e){
						tmp_e = msec_e;
					}
				}
				if (logopt.msecFall < msec_s + msec_dif_min){
					tmp_e = tmp_s;
				}
				else if (logopt.msecFall < msec_s + msec_dif_mid && logopt.msecRise <= msec_s){
					if (wmsec.early <= msec_s + msec_dif_min){
						tmp_s = tmp_e;
					}
				}
			}
			// ロゴ表示期間を追加
			if (tmp_s < tmp_e){
				sum_msec += (tmp_e - tmp_s);
			}
		}
	}
	// 秒数に変換してリターン
	return cnv.getSecFromMsec(sum_msec);
}

// 推測構成のロゴ扱いから
Sec JlsDataset::getSecLogoComponentFromElg(Msec msec_s, Msec msec_e){
	Msec sum_msec = 0;
	ElgCurrent elg = {};
	while( getElgptNext(elg) ){
		// 範囲内にロゴ表示期間がある場合
		if (elg.msecRise + msecValSpc < msec_e && elg.msecFall > msec_s + msecValSpc){
			Msec tmp_s, tmp_e;
			if (elg.msecRise < msec_s){
				tmp_s = msec_s;
			}
			else{
				tmp_s = elg.msecRise;
			}
			if (elg.msecFall > msec_e){
				tmp_e = msec_e;
			}
			else{
				tmp_e = elg.msecFall;
			}
			// ロゴ表示期間を追加
			if (tmp_s < tmp_e){
				sum_msec += (tmp_e - tmp_s);
			}
		}
	}
	// 秒数に変換してリターン
	return cnv.getSecFromMsec(sum_msec);
}



//=====================================================================
// Trim位置直接設定
//=====================================================================
void JlsDataset::setOutDirect(vector<Msec>& listMsec){
	m_listOutDirect = listMsec;
}

//=====================================================================
// 結果Trim作成
//=====================================================================

//---------------------------------------------------------------------
// カット結果作成
//---------------------------------------------------------------------
void JlsDataset::outputResultTrimGen(){
	resultTrim.clear();
	if ( m_listOutDirect.empty() ){
		if ( isAutoModeUse() ){
			outputResultTrimGenAuto();
		}else{
			outputResultTrimGenManual();
		}
	}else{		// Trim位置直接設定
		for(int i=0; i<(int)m_listOutDirect.size(); i++){
			resultTrim.push_back( m_listOutDirect[i] );
		}
	}
}

// カット結果作成（構成推測しない場合）
void JlsDataset::outputResultTrimGenManual(){
	LogoSelectType lgsel = LogoSelectType::LOGO_SELECT_VALID;
	bool final = true;
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = getNrfptOutNext(logopt, lgsel, final);
		if ( flag_cont ){
			//--- 結果データ追加 ---
			resultTrim.push_back( logopt.msecRise );
			resultTrim.push_back( logopt.msecFall );
		}
	}
}


// カット結果作成（構成推測する場合）
void JlsDataset::outputResultTrimGenAuto(){
	ElgCurrent elg = {};
	elg.outflag = true;					// 最終出力用の設定
	while ( getElgptNext(elg) ){
		resultTrim.push_back( elg.msecRise );
		resultTrim.push_back( elg.msecFall );
	}
}



//=====================================================================
// 詳細情報出力作成
//=====================================================================

//---------------------------------------------------------------------
// 詳細情報結果の読み出し位置リセット
//---------------------------------------------------------------------
void JlsDataset::outputResultDetailReset(){
	m_nscOutDetail = 0;
}

//---------------------------------------------------------------------
// 詳細情報結果の文字列作成
// 出力
//   返り値： 0=正常  1=終了
//   strBuf： 出力文字列
//---------------------------------------------------------------------
bool JlsDataset::outputResultDetailGetLine(string &strBuf){
	int num_scpos = sizeDataScp();
	//--- 現在位置を取得 ---
	Nsc i = m_nscOutDetail;
	if (i < 0 || i >= num_scpos-1){
		return 1;
	}
	Msec msec_from = getMsecScp(i);
	//--- 次の位置を取得 ---
	Msec msec_next;
	do{
		i = getNscNextScpDisp(i, SCP_END_EDGEIN);		// 次の構成区切り位置取得（端を含む）
		msec_next = getMsecScp(i);
	} while(msec_from == msec_next && i >= 0);
	//--- 位置を設定 ---
	m_nscOutDetail = i;
	if (i >= 0){
		//--- 前後間隔の期間取得 ---
		Msec msec_dif = msec_next - msec_from;
		Sec  sec_dif  = cnv.getSecFromMsec( msec_dif  );
		int frm_from = cnv.getFrmFromMsec( msec_from );
		int frm_next = cnv.getFrmFromMsec( msec_next );
		int frm_dif  = frm_next - cnv.getFrmFromMsec(sec_dif*1000 + msec_from);
		Sec sec_logo = getSecLogoComponent(msec_from, msec_next);
		//--- 終了地点を取得 ---
		Msec msec_to  = getMsecScpBk(i);
		int frm_to   = cnv.getFrmFromMsec( msec_to   );
		//--- 表示 ---
		strBuf = sformat("%6d %6d %4d %3d %4d ",
					frm_from, frm_to, sec_dif, frm_dif, sec_logo);
		//--- 構成名称を取得 ---
		if (m_flagSetupAuto > 1){
			outputResultDetailGetLineLabel(strBuf, getScpArstat(i), getScpArext(i));
		}
		return 0;
	}
	return 1;		// EOF
}





//---------------------------------------------------------------------
// 詳細情報結果の文字列作成（推測部分名称）
//---------------------------------------------------------------------
void JlsDataset::outputResultDetailGetLineLabel(string &strBuf, ScpArType arstat, ScpArExtType arext){
	bool flagOut = true;
	ComLabelType label = getLabelTypeFromStat(arstat, arext, flagOut);
	strBuf += getLabelStr(label);
}



//=====================================================================
// ログ表示
//=====================================================================
void JlsDataset::dispSysMesN(const string& msg, SysMesType typeMsg){
	int code1 = pdata->extOpt.dispSysMes % 10;
	int code2 = pdata->extOpt.dispSysMes / 10;
	bool flagDisp;
	switch( typeMsg ){
		case SysMesType::CutMrg :
			flagDisp = ( (code1 & 0x4) != 0 );
			break;
		case SysMesType::OutDirect :
			flagDisp = ( (code1 & 0x2) != 0 );
			break;
		case SysMesType::CallFile :
			flagDisp = ( (code1 & 0x1) != 0 );
			break;
		case SysMesType::LogoOff :
			flagDisp = ( (code2 & 0x1) != 0 );
			break;
		default :
			flagDisp = false;
			break;
	}
	if ( flagDisp ){
		lcout << msg << endl;
	}else{
		LSys.bufMemoIns(msg + "\n");
	}
}

//=====================================================================
// デバッグ用表示
//=====================================================================

//---------------------------------------------------------------------
// デバッグ用表示（ロゴデータ）
//---------------------------------------------------------------------
void JlsDataset::displayLogo(){
	int n = (int) m_logo.size();
	string buf;
	for(int i=0; i<n; i++){
		buf = sformat("%3d %6d %6d %6d %6d %6d %6d %d%d%d%d %d %d %d %d %d %d %6d %6d\n",
			i, cnv.getFrmFromMsec(m_logo[i].rise), cnv.getFrmFromMsec(m_logo[i].fall),
			cnv.getFrmFromMsec(m_logo[i].rise_l), cnv.getFrmFromMsec(m_logo[i].rise_r),
			cnv.getFrmFromMsec(m_logo[i].fall_l), cnv.getFrmFromMsec(m_logo[i].fall_r),
			m_logo[i].fade_rise, m_logo[i].fade_fall, m_logo[i].intl_rise, m_logo[i].intl_fall,
			m_logo[i].stat_rise, m_logo[i].stat_fall, m_logo[i].unit_rise, m_logo[i].unit_fall,
			m_logo[i].outtype_rise, m_logo[i].outtype_fall,
			cnv.getFrmFromMsec(m_logo[i].result_rise), cnv.getFrmFromMsec(m_logo[i].result_fall)
		);
		lcout << buf;
	};
}

//---------------------------------------------------------------------
// デバッグ用表示（無音SCデータ）
//---------------------------------------------------------------------
void JlsDataset::displayScp(){
	int n = (int) m_scp.size();
	string buf;
	for(int i=0; i<n; i++){
		buf = sformat("%3d %6d %6d %6d %6d %d %d (%8d %8d %8d %8d)%4d %2d %2d %2d)\n",
			i, cnv.getFrmFromMsec(m_scp[i].msec), cnv.getFrmFromMsec(m_scp[i].msbk),
			cnv.getFrmFromMsec(m_scp[i].msmute_s), cnv.getFrmFromMsec(m_scp[i].msmute_e),
			m_scp[i].still, m_scp[i].statpos,
			m_scp[i].msec, m_scp[i].msbk, m_scp[i].msmute_s, m_scp[i].msmute_e,
			m_scp[i].score, m_scp[i].chap, m_scp[i].arstat, m_scp[i].arext
		);
		lcout << buf;
	}
}

