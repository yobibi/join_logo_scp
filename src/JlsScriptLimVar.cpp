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
void JlsScriptLimVar::setPdata(JlsDataset *pdata){
	this->pdata  = pdata;
}
void JlsScriptLimVar::clear(){
	rmsecHeadTail = {-1, -1};
	rmsecFrameLimit = {-1, -1};

	clearRangeDst();
	clearPrepEnd();
	clearZone();
	clearScpEnable();
}
//=====================================================================
// コマンド共通の設定
//=====================================================================
//--- 実行コマンド引数を格納 ---
void JlsScriptLimVar::initVar(JlsCmdSet& cmdset){
	clear();
	opt = cmdset.arg;
}
//--- 共通位置情報の設定 ---
void JlsScriptLimVar::setHeadTail(RangeMsec rmsec){
	rmsecHeadTail = rmsec;
}
void JlsScriptLimVar::setFrameRange(RangeMsec rmsec){
	rmsecFrameLimit = rmsec;
}
//--- 共通位置情報の読み出し ---
RangeMsec JlsScriptLimVar::getHeadTail(){
	return rmsecHeadTail;
}
RangeMsec JlsScriptLimVar::getFrameRange(){
	return rmsecFrameLimit;
}

//=====================================================================
// ロゴ位置リスト内の指定ロゴで基準ロゴデータを作成
//=====================================================================

void JlsScriptLimVar::clearLogoBase(){
	nBaseListNum = -1;
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = -1;
	edgeBase = LOGO_EDGE_RISE;
	msecBaseBsrc = -1;
	msecBaseBorg = -1;
	edgeBaseBsrc = LOGO_EDGE_RISE;
	wmsecBaseBtg = {-1, -1, -1};
	edgeBaseBtg = LOGO_EDGE_RISE;
}
void JlsScriptLimVar::setLogoBaseListNum(int n){
	nBaseListNum = n;
}
//--- 本来の基準位置を設定（実ロゴ／推測ロゴどちらか１つのみ設定） ---
void JlsScriptLimVar::setLogoBaseNrf(Nrf nrf, LogoEdgeType edge){
	flagBaseNrf = true;
	nrfBase = nrf;
	nscBase = -1;
	edgeBase = edge;
}
void JlsScriptLimVar::setLogoBaseNsc(Nsc nsc, LogoEdgeType edge){
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = nsc;
	edgeBase = edge;
}
//--- （-fromabs等も反映した）変更後の基準ロゴ位置を設定 ---
void JlsScriptLimVar::setLogoBsrcMsec(Msec msec){
	msecBaseBsrc = msec;
}
//--- （ロゴ番号で決定する）本来の基準ロゴ位置を設定 ---
void JlsScriptLimVar::setLogoBorgMsec(Msec msec){
	msecBaseBorg = msec;
}
void JlsScriptLimVar::setLogoBsrcEdge(LogoEdgeType edge){
	edgeBaseBsrc = edge;
}
//--- ターゲット選択領域の計算に使用する基準位置を設定 ---
void JlsScriptLimVar::setLogoBtgWmsecEdge(WideMsec wmsec, jlsd::LogoEdgeType edge){
	wmsecBaseBtg = wmsec;
	edgeBaseBtg  = edge;
}

//--- 設定された基準ロゴ位置情報を取得 ---
int JlsScriptLimVar::getLogoBaseListNum(){	// 基準位置はリスト内で何番目か
	return nBaseListNum;
}
Msec JlsScriptLimVar::getLogoBsrcMsec(){	// 変更後の基準ロゴ位置
	return msecBaseBsrc;
}
Msec JlsScriptLimVar::getLogoBorgMsec(){	// 本来の基準ロゴ位置（存在しない時は変更後に一番近い位置）
	return msecBaseBorg;
}
bool JlsScriptLimVar::isLogoBaseExist(){	// 本来の基準ロゴ位置が存在するか？
	return ( nrfBase >= 0 || nscBase >= 0 );
}
bool JlsScriptLimVar::isLogoBaseNrf(){		// 本来の基準ロゴ位置が実ロゴか？
	return flagBaseNrf;
}
Nrf JlsScriptLimVar::getLogoBaseNrf(){		// 本来の基準ロゴ位置（実ロゴのロゴ番号）
	return nrfBase;
}
Nsc JlsScriptLimVar::getLogoBaseNsc(){		// 本来の基準ロゴ位置（推測ロゴのシーンチェンジ番号）
	return nscBase;
}
LogoEdgeType JlsScriptLimVar::getLogoBaseEdge(){	// 本来の基準ロゴ立上り／立下り情報
	return edgeBase;
}
LogoEdgeType JlsScriptLimVar::getLogoBsrcEdge(){	// 本来の基準ロゴ立上り／立下り情報
	return edgeBaseBsrc;
}
WideMsec JlsScriptLimVar::getLogoBtgWmsec(){	// ターゲット用基準ロゴ位置
	return wmsecBaseBtg;
}
LogoEdgeType JlsScriptLimVar::getLogoBtgEdge(){	// ターゲット用基準ロゴ立上り／立下り情報
	return edgeBaseBtg;
}
//---------------------------------------------------------------------
// 基準位置からのロゴ前後幅を取得
// 入力：
//   step     : 指定数先の前後ロゴ位置までの幅
//   flagWide : false=前後も中心位置まで  true=前後は可能性範囲まで拡張
// 出力：
//   wmsec    : 前後含むロゴ位置（.just=基準位置中心  .early=前側のロゴ位置  .late=後側のロゴ位置）
//---------------------------------------------------------------------
//--- 基準ロゴ位置からの前後ロゴ幅 ---
void JlsScriptLimVar::getWidthLogoFromBase(WideMsec& wmsec, int step, bool flagWide){
	Msec         msecLogo = getLogoBorgMsec();
	LogoEdgeType edgeLogo = getLogoBaseEdge();
	getWidthLogoCommon(wmsec, msecLogo, edgeLogo, step, flagWide);
}
//--- (-fromlast補正等含む)ターゲット位置計算用基準ロゴ位置からの前後ロゴ幅 ---
void JlsScriptLimVar::getWidthLogoFromBaseForTarget(WideMsec& wmsec, int step, bool flagWide){
	Msec         msecLogo = getLogoBtgWmsec().just;
	LogoEdgeType edgeLogo = getLogoBtgEdge();
	getWidthLogoCommon(wmsec, msecLogo, edgeLogo, step, flagWide);
}
void JlsScriptLimVar::getWidthLogoCommon(WideMsec& wmsec, Msec msecLogo, LogoEdgeType edgeLogo, int step, bool flagWide){
	int locLogo = pdata->getClogoNumNear(msecLogo, edgeLogo);
	WideMsec wTmpC, wTmpP, wTmpN;
	wTmpC = pdata->getClogoWmsecFromNum(locLogo);
	wTmpP = pdata->getClogoWmsecFromNum(locLogo-step);
	wTmpN = pdata->getClogoWmsecFromNum(locLogo+step);
	if ( flagWide ){
		wmsec.just  = wTmpC.just;
		wmsec.early = wTmpP.early;
		wmsec.late  = wTmpN.late;
	}else{
		wmsec.just  = wTmpC.just;
		wmsec.early = wTmpP.just;
		wmsec.late  = wTmpN.just;
	}
}

//=====================================================================
// Dst範囲設定
//=====================================================================
void JlsScriptLimVar::clearRangeDst(){
	listRangeDst.clear();
	numRangeDst = 0;
}
void JlsScriptLimVar::addRangeDst(WideMsec wmsecFind, WideMsec wmsecFrom){
	ArgRange argTmp;
	argTmp.wmsecFind = wmsecFind;
	argTmp.wmsecFrom = wmsecFrom;
	listRangeDst.push_back(argTmp);
}
void JlsScriptLimVar::selRangeDstNum(int num){
	numRangeDst = num;
}

//--- ターゲット位置取得 ---
int JlsScriptLimVar::sizeRangeDst(){
	return (int) listRangeDst.size();
}
bool JlsScriptLimVar::isRangeDstMultiFrom(){
	if ( sizeRangeDst() <= 1 ) return false;
	Msec msecZ = listRangeDst[0].wmsecFrom.just;
	for(int i=1; i<sizeRangeDst(); i++){
		if ( listRangeDst[i].wmsecFrom.just != msecZ ){
			return true;
		}
	}
	return false;
}
WideMsec JlsScriptLimVar::getRangeDstWide(){
	return getRangeDstItemWide(numRangeDst);
}
Msec JlsScriptLimVar::getRangeDstJust(){
	WideMsec wmsec = getRangeDstItemWide(numRangeDst);
	return wmsec.just;
}
Msec JlsScriptLimVar::getRangeDstFrom(){
	return getRangeDstItemFromWide(numRangeDst).just;
}
void JlsScriptLimVar::getRangeDstFromForScp(Msec& msec, Msec& msbk, Nsc& nsc){
	WideMsec wmsec = getRangeDstItemFromWide(numRangeDst);
	msec = wmsec.late;
	msbk = wmsec.early;
	if ( msec != msbk ){
		nsc = pdata->getNscFromMsecMgn(wmsec.just, pdata->msecValExact, SCP_END_EDGEIN);
	}else{
		nsc = -1;
	}
}
WideMsec JlsScriptLimVar::getRangeDstItemWide(int num){
	if ( isErrorRangeDst(num) ) return {-1, -1, -1};
	return listRangeDst[num].wmsecFind;
}
WideMsec JlsScriptLimVar::getRangeDstItemFromWide(int num){
	if ( isErrorRangeDst(num) ) return {-1, -1, -1};
	return listRangeDst[num].wmsecFrom;
}
bool JlsScriptLimVar::isErrorRangeDst(int num){
	return ( num < 0 || num >= sizeRangeDst() );
}

//=====================================================================
// 位置検索用の設定
//=====================================================================
//---------------------------------------------------------------------
// 検索用の設定初期化
//---------------------------------------------------------------------
void JlsScriptLimVar::initSeekVar(JlsCmdSet& cmdset){
	seek = {};
	bool seekNoEdge = opt.getOptFlag(OptType::FlagNoEdge);
	//--- 検索設定（NextTailコマンド対応） ---
	if ( cmdset.arg.cmdsel == CmdType::NextTail ){
		seek.flagNextTail = true;
		seekNoEdge        = false;
		if ( cmdset.arg.selectEdge == LogoEdgeType::LOGO_EDGE_RISE ){
			seek.selectLogoRise = true;	// ロゴ立上り位置優先
		}
	}
	seek.tgDst.exact = opt.getOptFlag(OptType::FlagFixPos);
	seek.rnscScp = pdata->getRangeNscTotal(seekNoEdge);
}
//---------------------------------------------------------------------
// Dst位置が範囲条件を満たすか判定
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToDst(Msec msecBsrc, Msec msecDst){
	WideMsec wmsecTmp = getRangeDstWide();		// Dst範囲は設定読み込み
	if ( msecDst < wmsecTmp.early || wmsecTmp.late < msecDst ){
		return false;
	}
	return isZoneAtDst(msecBsrc, msecDst);
}
//---------------------------------------------------------------------
// End位置が範囲条件を満たすか判定（End範囲は引数で与える）
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToEnd(Msec msecDst, Msec msecEnd, WideMsec wmsecRange){
	if ( msecEnd < wmsecRange.early || wmsecRange.late < msecEnd ){
		return false;
	}
	return isZoneAtEnd(msecDst, msecEnd);
}
//---------------------------------------------------------------------
// End位置が範囲条件を満たすか判定（Zone条件のみ確認）
//---------------------------------------------------------------------
bool JlsScriptLimVar::isRangeToEndZone(Msec msecDst, Msec msecEnd){
	return isZoneAtEnd(msecDst, msecEnd);
}

//=====================================================================
// 終了位置の事前準備
//=====================================================================

void JlsScriptLimVar::clearPrepEnd(){
	listPrepEndRange.clear();
	existPrepEndRefer = false;
	fromPrepEndAbs = false;
	multiPrepEndBase = false;
	tgPrepEndAbs = {};
	listPrepEndBaseMsec.clear();
	listPrepEndBaseMsbk.clear();
}
void JlsScriptLimVar::addPrepEndRange(WideMsec wmsec){
	listPrepEndRange.push_back(wmsec);
}
void JlsScriptLimVar::setPrepEndRefer(bool flag){
	existPrepEndRefer = flag;
}
void JlsScriptLimVar::setPrepEndAbs(bool fromAbs, bool multiBase, TargetLocInfo tgEnd){
	fromPrepEndAbs  = fromAbs;
	multiPrepEndBase = multiBase;
	tgPrepEndAbs     = tgEnd;
}
int JlsScriptLimVar::sizePrepEndRange(){
	return (int)listPrepEndRange.size();
}
bool JlsScriptLimVar::isPrepEndRangeExist(){
	return ( listPrepEndRange.size() > 0 );
}
bool JlsScriptLimVar::isPrepEndReferExist(){
	return existPrepEndRefer;
}
bool JlsScriptLimVar::isPrepEndFromAbs(){
	return fromPrepEndAbs;
}
bool JlsScriptLimVar::isPrepEndMultiBase(){
	return ( multiPrepEndBase && fromPrepEndAbs );
}
TargetLocInfo JlsScriptLimVar::getPrepEndAbs(){
	return tgPrepEndAbs;
}
WideMsec JlsScriptLimVar::getPrepEndRangeWithOffset(int num, Msec msecOfs){
	if ( num >= sizePrepEndRange() || num < 0 ) return {-1, -1, -1};
	WideMsec wmsecTmp = listPrepEndRange[num];
	wmsecTmp.just  += msecOfs;
	wmsecTmp.early += msecOfs;
	wmsecTmp.late  += msecOfs;
	return wmsecTmp;
}
Msec JlsScriptLimVar::getPrepEndRangeForceLen(){
	if ( sizePrepEndRange() <= 0 ) return 0;
	return listPrepEndRange[0].just;
}

//=====================================================================
// Zone設定
//=====================================================================

void JlsScriptLimVar::clearZone(){
	msecZoneSrc = -1;
	validZoneRange = true;
	rmsecZoneRange = {-1, -1};
	rmsecZoneForbid = {-1, -1};
	listTLRange.clear();
}
//---------------------------------------------------------------------
// 結果位置が基準位置からの有効領域か判定
//---------------------------------------------------------------------
bool JlsScriptLimVar::isZoneAtDst(Msec msecBsrc, Msec msecDst){
	//--- -TgtLimit確認 ---
	if ( !isTgtLimitAllow(msecDst) ){
		return false;
	}
	//--- 基準位置からでなければDstでZone判定なし ---
	if ( opt.getOptFlag(OptType::FlagZEnd) ) return true;
	//--- 範囲確認 ---
	return isZoneBothEnds(msecBsrc, msecDst);
}
//---------------------------------------------------------------------
// 終了位置が結果位置からの有効領域か判定
//---------------------------------------------------------------------
bool JlsScriptLimVar::isZoneAtEnd(Msec msecDst, Msec msecEnd){
	//--- 基準位置から結果位置の時は無条件 ---
	if ( !opt.getOptFlag(OptType::FlagZEnd) ) return true;
	//--- 範囲確認 ---
	return isZoneBothEnds(msecDst, msecEnd);
}

// 対象2点のZone範囲内確認
bool JlsScriptLimVar::isZoneBothEnds(Msec msecP, Msec msecQ){
	//--- キャッシュからデータ取得 ---
	bool validRange;
	RangeMsec rmsecRange;
	RangeMsec rmsecForbid;
	bool flagCache = getZoneCache(validRange, rmsecRange, rmsecForbid, msecP);
	//--- キャッシュになければ計算して取得 ---
	if ( !flagCache ){
		RangeMsec rmsecTmpR = rmsecRange;
		RangeMsec rmsecTmpF = rmsecForbid;
		validRange = false;
		if ( calcZoneRange(rmsecTmpR, msecP) ){	// Zone設定
			if ( calcZoneUnder(rmsecTmpR, msecP) ){	// under設定
				validRange = true;
				rmsecRange = rmsecTmpR;
			}
		}
		if ( calcZoneOver(rmsecTmpF, msecP) ){
			rmsecForbid = rmsecTmpF;
		}
		setZoneCache(msecP, validRange, rmsecRange, rmsecForbid);	// 計算結果をキャッシュに格納
	}
	//--- 範囲確認 ---
	if ( !validRange ) return false;
	if ( isZoneInForbid(msecQ, rmsecForbid) ) return false;
	return isZoneInRange(msecQ, rmsecRange);
}
// 指定位置が指定Zone内にあるか判定
bool JlsScriptLimVar::isZoneInRange(Msec msecNow, RangeMsec rmsecZone){
	if ( (msecNow < rmsecZone.st && rmsecZone.st >= 0) ||
		 (msecNow > rmsecZone.ed && rmsecZone.ed >= 0) ){
		return false;
	}
	return true;
}
// 指定位置が指定禁止領域内にあるか判定
bool JlsScriptLimVar::isZoneInForbid(Msec msecNow, RangeMsec rmsecZone){
	if ( (msecNow >= rmsecZone.st && rmsecZone.st >= 0) &&
		 (msecNow <= rmsecZone.ed && rmsecZone.ed >= 0) ){
		return true;
	}
	return false;
}
// Zone設定
bool JlsScriptLimVar::calcZoneRange(RangeMsec& rmsecZone, Msec msecB){
	bool useZoneL = opt.getOptFlag(OptType::FlagZoneL);
	bool useZoneN = opt.getOptFlag(OptType::FlagZoneN);
	if ( useZoneL || useZoneN ){
		int nsize = pdata->sizeClogoList();
		vector<Msec> listMsec;
		for(int i=0; i<nsize; i++){
			Msec msecTmp = pdata->getClogoMsecFromNum(i);
			listMsec.push_back(msecTmp);
		}
		if ( !calcZoneRangeSub(rmsecZone, msecB, listMsec, useZoneL) ){
			return false;
		}
	}
	//--- zone設定(直接指定位置) ---
	bool useZoneImmL = opt.isSetStrOpt(OptType::ListZoneImmL);
	bool useZoneImmN = opt.isSetStrOpt(OptType::ListZoneImmN);
	if ( useZoneImmL || useZoneImmN ){
		string strList;
		if ( useZoneImmN ){
			strList = opt.getStrOpt(OptType::ListZoneImmN);
		}else{
			strList = opt.getStrOpt(OptType::ListZoneImmL);
		}
		vector<Msec> listMsec;
		pdata->cnv.getListValMsecM1(listMsec, strList);
		if ( !calcZoneRangeSub(rmsecZone, msecB, listMsec, useZoneImmL) ){
			return false;
		}
	}
	return true;
}
// リストからZone設定の範囲を追加
bool JlsScriptLimVar::calcZoneRangeSub(RangeMsec& rmsecZone, Msec msecTg, vector<Msec>& listMsec, bool flagL){
	Msec msecMgn = pdata->getClogoMsecMgn();
	//--- 各ロゴ位置で領域を取得 ---
	RangeMsec rmsecL = {-1, -1};
	int nsize = (int)listMsec.size();
	for(int i=0; i<nsize-1; i=i+2){
		RangeMsec rms = {listMsec[i], listMsec[i+1]};
		if ( rms.ed < 0 || rms.st >= rms.ed ){
			continue;
		}
		if ( flagL ){	// ロゴあり領域は引数ロゴ区間のみをOR（rmsecLに格納）
			if ( rms.ed >= msecTg - msecMgn ){
				Msec msecSt = rms.st - msecMgn;
				if ( rmsecL.st > msecSt || rmsecL.st < 0 ){
					if ( msecSt < 0 ) msecSt = 0;
					rmsecL.st = msecSt;
				}
			}
			if ( rms.st <= msecTg + msecMgn ){
				Msec msecEd = rms.ed + msecMgn;
				if ( rmsecL.ed < msecEd || rmsecL.ed < 0 ){
					rmsecL.ed = msecEd;
				}
			}
		}else{	// ロゴなし領域は元のZoneから引数ロゴ区間をカット
			if ( rms.st < msecTg - msecMgn ){
				Msec msecEd = rms.ed - msecMgn;
				if ( rmsecZone.st < msecEd || rmsecZone.st < 0 ){
					rmsecZone.st = msecEd;
				}
			}
			if ( rms.ed > msecTg + msecMgn ){
				Msec msecSt = rms.st + msecMgn;
				if ( rmsecZone.ed > msecSt || rmsecZone.ed < 0 ){
					if ( msecSt < 0 ) msecSt = 0;
					rmsecZone.ed = msecSt;
				}
			}
		}
	}
	//--- ロゴあり領域は取得した引数ロゴ区間と元のZoneをAND ---
	if ( flagL ){
		if ( rmsecL.st < 0 || rmsecL.ed < 0 ){	// 引数ロゴ区間検出なし
			return false;
		}else if ( rmsecL.st >= rmsecL.ed ){	// 引数ロゴ区間検出なし
			return false;
		}else{
			if ( rmsecZone.st < rmsecL.st || rmsecZone.st < 0 ) rmsecZone.st = rmsecL.st;
			if ( rmsecZone.ed > rmsecL.ed || rmsecZone.ed < 0 ) rmsecZone.ed = rmsecL.ed;
		}
	}
	//--- 領域存在確認 ---
	if ( rmsecZone.st >= 0 && rmsecZone.ed >= 0 ){
		if ( rmsecZone.st >= rmsecZone.ed ) return false;
	}
	return true;
}
// 推測構成区切り指定範囲をZone範囲に追加
bool JlsScriptLimVar::calcZoneUnder(RangeMsec& rmsecZone, Msec msecB){
	bool useZUnderC = opt.isSetOpt(OptType::NumZUnderC);
	if ( useZUnderC ){
		int numUnder = opt.getOpt(OptType::NumZUnderC);
		Nsc nscP = pdata->getNscPrevScpDispFromMsecCount(msecB, numUnder, true);
		Nsc nscN = pdata->getNscNextScpDispFromMsecCount(msecB, numUnder, true);
		Msec msecMgn = pdata->getClogoMsecMgn();
		RangeMsec rmsecU;
		rmsecU.st = pdata->getMsecScpBk(nscP);
		rmsecU.ed = pdata->getMsecScp(nscN);
		if ( rmsecU.st >= msecMgn ){
			rmsecU.st -= msecMgn;
		}else if ( rmsecU.st >= 0 ){
			rmsecU.st = 0;
		}
		if ( rmsecU.ed >= 0 ){
			rmsecU.ed += msecMgn;
		}
		if ( rmsecU.st >= 0 ){
			if ( rmsecZone.st < rmsecU.st || rmsecZone.st < 0 ){
				 rmsecZone.st = rmsecU.st;
			}
		}
		if ( rmsecU.ed >= 0 ){
			if ( rmsecZone.ed > rmsecU.ed || rmsecZone.ed < 0 ){
				 rmsecZone.ed = rmsecU.ed;
			}
		}
	}
	//--- 領域存在確認 ---
	if ( rmsecZone.st >= 0 && rmsecZone.ed >= 0 ){
		if ( rmsecZone.st >= rmsecZone.ed ) return false;
	}
	return true;
}
// 推測構成区切り指定範囲を禁止範囲に追加
bool JlsScriptLimVar::calcZoneOver(RangeMsec& rmsecForbid, Msec msecB){
	bool useZOverC = opt.isSetOpt(OptType::NumZOverC);
	if ( useZOverC ){
		int numOver = opt.getOpt(OptType::NumZOverC);
		Nsc nscP = pdata->getNscPrevScpDispFromMsecCount(msecB, numOver, true);
		Nsc nscN = pdata->getNscNextScpDispFromMsecCount(msecB, numOver, true);
		Msec msecMgn = pdata->getClogoMsecMgn();
		RangeMsec rmsecO;
		rmsecO.st = pdata->getMsecScp(nscP);
		rmsecO.ed = pdata->getMsecScpBk(nscN);
		if ( rmsecO.st >= msecMgn ){
			rmsecO.st -= msecMgn;
		}else if ( rmsecO.st >= 0 ){
			rmsecO.st = 0;
		}
		if ( rmsecO.ed >= 0 ){
			rmsecO.ed += msecMgn;
		}
		if ( rmsecForbid.st > rmsecO.st || rmsecForbid.st < 0 ){
			 rmsecForbid.st = rmsecO.st;
		}
		if ( rmsecForbid.ed < rmsecO.ed || rmsecForbid.ed < 0 ){
			 rmsecForbid.ed = rmsecO.ed;
		}
	}
	//--- 領域存在確認 ---
	if ( rmsecForbid.st >= 0 && rmsecForbid.ed >= 0 ){
		if ( rmsecForbid.st < rmsecForbid.ed ) return true;
	}
	return false;
}
// キャッシュに指定位置のzone情報を格納
void JlsScriptLimVar::setZoneCache(Msec msecSrc, bool validRange, RangeMsec rmsecRange, RangeMsec rmsecForbid){
	msecZoneSrc = msecSrc;
	validZoneRange = validRange;
	rmsecZoneRange = rmsecRange;
	rmsecZoneForbid = rmsecForbid;
}
// キャッシュに格納された指定位置のzone情報を取得
bool JlsScriptLimVar::getZoneCache(bool& validRange, RangeMsec& rmsecRange, RangeMsec& rmsecForbid, Msec msecSrc){
	if ( msecSrc == msecZoneSrc ){
		validRange = validZoneRange;
		rmsecRange = rmsecZoneRange;
		rmsecForbid = rmsecZoneForbid;
		return true;
	}
	validRange = true;
	rmsecRange = {-1, -1};
	rmsecForbid = {-1, -1};
	return false;
}
//---------------------------------------------------------------------
// -TgtLimit設定
//---------------------------------------------------------------------
bool JlsScriptLimVar::isTgtLimitAllow(Msec msecTarget){
	//--- オプションなければ何もしない ---
	if ( !opt.isSetOpt(OptType::MsecTgtLimL) ) return true;

	int nsize = (int) listTLRange.size();
	if ( nsize == 0 ){
		nsize = setTgtLimit();
		if ( nsize == 0 ) return true;	// リストがなければ無条件で許可
	}
	//--- 各リスト検索 ---
	bool det = false;
	for(int i=0; i<nsize; i++){
		if ( listTLRange[i].st <= msecTarget && msecTarget <= listTLRange[i].ed){
			det = true;
		}
	}
	return det;
}
int JlsScriptLimVar::setTgtLimit(){
	string strList = opt.getStrOpt(OptType::StrValListR);	// $LISTHOLD
	if ( strList.empty() ) return 0;
	listTLRange.clear();
	vector<Msec> listMsec;
	if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
		Msec msecL = (Msec) opt.getOpt(OptType::MsecTgtLimL);
		Msec msecR = (Msec) opt.getOpt(OptType::MsecTgtLimR);
		if ( msecL != -1 || msecR != -1 ){
			for(int i=0; i<(int)listMsec.size(); i++){
				RangeMsec rmsec = {msecL, msecR};
				if ( msecL != -1 ) rmsec.st += listMsec[i];
				if ( msecR != -1 ) rmsec.ed += listMsec[i];
				listTLRange.push_back(rmsec);
			}
		}
	}
	return (int) listTLRange.size();
}

//=====================================================================
// 無音条件判定
//=====================================================================

//---------------------------------------------------------------------
// -SC, -NoSC系オプションに対応するシーンチェンジ有無判定
// 入力：
//   msecBase : 基準となるフレーム
//   edge     : 0:start edge  1:end edge
//   tgcat    : 判定対象種類
// 返り値：
//   false : 一致せず
//   true  : 一致確認
//---------------------------------------------------------------------
bool JlsScriptLimVar::isScpEnableAtMsec(int msecBase, LogoEdgeType edge, TargetCatType tgcat){
	//--- 相対位置検出する時は、From位置からの検索は何もしない ---
	if ( opt.tack.floatBase && (tgcat == TargetCatType::From) ){
		return true;
	}

	bool result = true;
	for(int k=0; k<opt.sizeScOpt(); k++){
		//--- 相対位置コマンド判定処理 ---
		bool needCheck = false;
		TargetCatType category = opt.getScOptCategory(k);
		switch( tgcat ){
			case TargetCatType::From :
				if ( category == TargetCatType::From ){
					needCheck = true;
				}
				break;
			case TargetCatType::Shift :
				if ( (category == TargetCatType::From) ||
					 (category == TargetCatType::RX  ) ){
					needCheck = true;
				}
				break;
			case TargetCatType::Dst :
				if ( (category == TargetCatType::Dst) ||
					 (category == TargetCatType::RX ) ){
					needCheck = true;
				}else if ( opt.tack.floatBase && category == TargetCatType::From ){
					needCheck = true;
				}
				break;
			case TargetCatType::End :
				if ( (category == TargetCatType::End) ||
					 (category == TargetCatType::RX ) ){
					needCheck = true;
				}
				break;
			default :
				break;
		}
		if ( !needCheck ){
			continue;
		}
		//--- 対象であればチェック ---
		result = isScpEnableAtMsecCheck(msecBase, edge, k);
		if ( !result ){
			break;
		}
	}
	return result;
}
//--- 個別オプションチェック ---
bool JlsScriptLimVar::isScpEnableAtMsecCheck(int msecBase, LogoEdgeType edge, int numOpt){
	//--- 関連オプション存在時のみチェック ---
	OptType sctype = opt.getScOptType(numOpt);
	if ( sctype == OptType::ScNone ){
		return true;
	}
	bool result = true;
	DataScpRecord dt;
	Nsc nscScposSc   = -1;
	Nsc nscSmuteAll  = -1;
	Nsc nscSmutePart = -1;
	Nsc nscChapAuto  = -1;
	Nsc nscChapAtc   = -1;
	Msec lensMin = opt.getScOptMin(numOpt);
	Msec lensMax = opt.getScOptMax(numOpt);
	//--- -AC用構成取得 ---
	Term term = {};
	pdata->setTermEndtype(term, SCP_END_EDGEIN);	// 端を含めて実施
	pdata->setTermForDisp(term, true);		// 表示用の構成
	bool contTerm = pdata->getTermNext(term);
	//--- 各位置チェック ---
	RangeNsc rnscScp = pdata->getRangeNscTotal( opt.getOptFlag(OptType::FlagNoEdge) );
	for(int j=rnscScp.st; j<=rnscScp.ed; j++){
		if ( contTerm ){	// -AC用更新
			if ( j > term.nsc.ed ) contTerm = pdata->getTermNext(term);
		}
		pdata->getRecordScp(dt, j);
		int msecNow;
		if ( jlsd::isLogoEdgeRise(edge) ){
			msecNow = dt.msec;
		}
		else{
			msecNow = dt.msbk;
		}
		if ((msecNow - msecBase >= lensMin || lensMin == -1) &&
			(msecNow - msecBase <= lensMax || lensMax == -1)){
			nscScposSc = j;
			// for -AC option
			if ( contTerm ){
				if ( j == term.nsc.st || j == term.nsc.ed ){
					nscChapAuto = j;
				}
			}
			ScpChapType chapNow = dt.chap;
			if (chapNow >= SCP_CHAP_DECIDE || chapNow == SCP_CHAP_CDET ){
				nscChapAtc = j;
			}
		}
		// 無音系
		int msecSmuteS = dt.msmute_s;
		int msecSmuteE = dt.msmute_e;
		if (msecSmuteS < 0 || msecSmuteE < 0){
			msecSmuteS = msecNow;
			msecSmuteE = msecNow;
		}
		// for -SMA option （無音情報がある場合のみ検出）
		if ((msecSmuteS - msecBase <= lensMin) &&
			(msecSmuteE - msecBase >= lensMax)){
			nscSmuteAll = j;
		}
		//for -SM option
		if ((msecSmuteS - msecBase <= lensMax || lensMax == -1) &&
			(msecSmuteE - msecBase >= lensMin || lensMin == -1)){
			nscSmutePart = j;
		}
	}
	if (nscScposSc < 0 && sctype == OptType::ScSC){	// -SC
		result = false;
	}
	else if (nscScposSc >= 0 && sctype == OptType::ScNoSC){	// -NoSC
		result = false;
	}
	else if (nscSmutePart < 0 && sctype == OptType::ScSM){	// -SM
		result = false;
	}
	else if (nscSmutePart >= 0 && sctype == OptType::ScNoSM){	// -NoSM
		result = false;
	}
	else if (nscSmuteAll < 0 && sctype == OptType::ScSMA){	// -SMA
		result = false;
	}
	else if (nscSmuteAll >= 0 && sctype == OptType::ScNoSMA){	// -NoSMA
		result = false;
	}
	else if (nscChapAuto < 0 && sctype == OptType::ScAC){	// -AC
		result = false;
	}
	else if (nscChapAuto >= 0 && sctype == OptType::ScNoAC){	// -NoAC
		result = false;
	}
	else if (nscChapAtc < 0 && sctype == OptType::ScACC){	// -ACC
		result = false;
	}
	else if (nscChapAtc >= 0 && sctype == OptType::ScNoACC){	// -NoACC
		result = false;
	}
	return result;
}
//---------------------------------------------------------------------
// 各無音SC位置の判定読み出し
//---------------------------------------------------------------------
bool JlsScriptLimVar::isScpEnableAtNsc(TargetCatType tgcat, Nsc nsc){
	//--- 更新判断 ---
	int sizeScp    = pdata->sizeDataScp();
	int sizeEnable = sizeScpEnable(tgcat);
	if ( sizeScp != sizeEnable ){	// 増減あれば各無音SC位置で情報保管
		setScpEnableEveryNsc();
	}
	//--- 取得 ---
	if ( isErrorScpEnable(tgcat, nsc) ) return false;
	if ( tgcat == TargetCatType::Dst ) return (int) listScpEnableDst[nsc];
	if ( tgcat == TargetCatType::End ) return (int) listScpEnableEnd[nsc];
	return false;
}

//---------------------------------------------------------------------
// 各無音SC位置における判定記憶を消去
//---------------------------------------------------------------------
void JlsScriptLimVar::clearScpEnable(){
	listScpEnableDst.clear();
	listScpEnableEnd.clear();
}
//---------------------------------------------------------------------
// 各無音SC位置における判定記憶（無音シーンチェンジ追加なければコマンド中は更新せず使用可能）
//---------------------------------------------------------------------
void JlsScriptLimVar::setScpEnableEveryNsc(){
	//--- 全無音シーンチェンジ位置でチェック ---
	int sizeScp    = pdata->sizeDataScp();
	clearScpEnable();
	for(int m=0; m<sizeScp; m++){
		int msecBase = pdata->getMsecScp(m);
		bool resultDst = isScpEnableAtMsec(msecBase, LOGO_EDGE_RISE, TargetCatType::Dst);
		bool resultEnd = isScpEnableAtMsec(msecBase, LOGO_EDGE_RISE, TargetCatType::End);
		listScpEnableDst.push_back(resultDst);
		listScpEnableEnd.push_back(resultEnd);
	}
}
//--- シーンチェンジに対応するデータ数 ---
int JlsScriptLimVar::sizeScpEnable(TargetCatType tgcat){
	if ( tgcat == TargetCatType::Dst ) return (int) listScpEnableDst.size();
	if ( tgcat == TargetCatType::End ) return (int) listScpEnableEnd.size();
	return 0;
}
//--- エラーチェック ---
bool JlsScriptLimVar::isErrorScpEnable(TargetCatType tgcat, Nsc nsc){
	if ( nsc < 0 || nsc >= sizeScpEnable(tgcat) ){
		string mes = "error:internal ScpEnable access nsc=" + to_string(nsc);
		mes += " size=" + to_string(sizeScpEnable(tgcat));
		lcerr << mes << endl;
		return true;
	}
	return false;
}

