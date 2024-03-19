//
// JLスクリプト用コマンド内容格納データ
//
//#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsCmdSet.hpp"

///////////////////////////////////////////////////////////////////////
//
// JLスクリプトコマンド設定反映用
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// 初期設定
//---------------------------------------------------------------------
JlsCmdLimit::JlsCmdLimit(){
	clear();
}

void JlsCmdLimit::clear(){
	process = 0;
	rmsecHeadTail = {-1, -1};
	rmsecFrameLimit = {-1, -1};

	clearLogoList();	// 有効なロゴ番号リスト
	clearLogoBase();	// 対象とする基準ロゴ選択
	clearPickList();	// -pickオプション用保持リスト
}

//---------------------------------------------------------------------
// 先頭と最後の位置
//---------------------------------------------------------------------
RangeMsec JlsCmdLimit::getHeadTail(){
	return rmsecHeadTail;
}

Msec JlsCmdLimit::getHead(){
	return rmsecHeadTail.st;
}

Msec JlsCmdLimit::getTail(){
	return rmsecHeadTail.ed;
}

bool JlsCmdLimit::setHeadTail(RangeMsec rmsec){
	process |= ARG_PROCESS_HEADTAIL;
	rmsecHeadTail = rmsec;
	return true;
}

//---------------------------------------------------------------------
// フレーム範囲(-Fオプション)
//---------------------------------------------------------------------
RangeMsec JlsCmdLimit::getFrameRange(){
	return rmsecFrameLimit;
}

bool JlsCmdLimit::setFrameRange(RangeMsec rmsec){
	if ((process & ARG_PROCESS_HEADTAIL) == 0){
		signalInternalError(ARG_PROCESS_FRAMELIMIT);
	}
	process |= ARG_PROCESS_FRAMELIMIT;
	rmsecFrameLimit = rmsec;
	return true;
}

//---------------------------------------------------------------------
// 有効なロゴ番号リスト
//---------------------------------------------------------------------
void JlsCmdLimit::clearLogoList(){
	listLogoStd.clear();
	listLogoDir.clear();
	listLogoOrg.clear();
	forceLogoStdFix = false;
	existLogoDirDmy = false;
	process &= ~ARG_PROCESS_VALIDLOGO;
}

//--- 書き込み（ロゴ番号指定） ---
bool JlsCmdLimit::addLogoListStd(Msec msec, LogoEdgeType edge){
	if ((process & ARG_PROCESS_HEADTAIL) == 0){
		signalInternalError(ARG_PROCESS_VALIDLOGO);
	}
	process |= ARG_PROCESS_VALIDLOGO;
	ArgLogoList argset = {msec, edge};
	listLogoStd.push_back(argset);
	return true;
}
//--- 直接フレーム指定（ただし無効位置）の存在設定 ---
void JlsCmdLimit::addLogoListDirectDummy(bool flag){
	existLogoDirDmy = flag;
}
//--- 書き込み（直接フレーム指定） ---
void JlsCmdLimit::addLogoListDirect(Msec msec, LogoEdgeType edge){
	ArgLogoList argset = {msec, edge};
	ArgLogoList argset2 = {-1, edge};
	listLogoDir.push_back(argset);
	listLogoOrg.push_back(argset2);
}
//--- 基準位置付加（直接フレーム指定） ---
void JlsCmdLimit::attachLogoListOrg(int num, Msec msec, LogoEdgeType edge){
	if ( num < 0 || num >= (int)listLogoOrg.size() ) return;
	listLogoOrg[num].msec = msec;
	listLogoOrg[num].edge = edge;
}
//--- 共通呼び出し ---
Msec JlsCmdLimit::getLogoListMsec(int nlist){	// 設定後基準位置(msec)取得
	if ( isErrorLogoList(nlist) ) return -1;
	if ( isLogoListDirect() ){
		return listLogoDir[nlist].msec;
	}
	return listLogoStd[nlist].msec;
}
Msec JlsCmdLimit::getLogoListOrgMsec(int nlist){	// 本来の基準位置(msec)取得
	if ( isErrorLogoList(nlist) ) return -1;
	if ( isLogoListDirect() ){
		return listLogoOrg[nlist].msec;
	}
	return listLogoStd[nlist].msec;
}
LogoEdgeType JlsCmdLimit::getLogoListEdge(int nlist){	// 設定後基準位置（エッジ情報）取得
	if ( isErrorLogoList(nlist) ) return LogoEdgeType::LOGO_EDGE_RISE;
	if ( isLogoListDirect() ){
		return listLogoDir[nlist].edge;
	}
	return listLogoStd[nlist].edge;
}
LogoEdgeType JlsCmdLimit::getLogoListOrgEdge(int nlist){	// 本来の基準位置（エッジ情報）取得
	if ( isErrorLogoList(nlist) ) return LogoEdgeType::LOGO_EDGE_RISE;
	if ( isLogoListDirect() ){
		return listLogoOrg[nlist].edge;
	}
	return listLogoStd[nlist].edge;
}
int JlsCmdLimit::sizeLogoList(){		// リスト数取得
	if ( isLogoListDirect() ){
		return (int)listLogoDir.size();
	}
	return (int)listLogoStd.size();
}
//--- フレーム直接指定用の拡張 ---
bool JlsCmdLimit::isLogoListDirect(){		// 拡張選択状態
	if ( forceLogoStdFix ) return false;
	return ( listLogoDir.size() > 0 || existLogoDirDmy );
}
void JlsCmdLimit::forceLogoListStd(bool flag){		// 一時的に拡張無効化
	forceLogoStdFix = flag;
}
//--- 内部処理 ---
bool JlsCmdLimit::isErrorLogoList(int nlist){
	if ( isLogoListDirect() ){
		return ( nlist < 0 || nlist >= (int) listLogoDir.size() );
	}
	return ( nlist < 0 || nlist >= (int) listLogoStd.size() );;
}


//---------------------------------------------------------------------
// 対象とする基準ロゴ選択
//---------------------------------------------------------------------
void JlsCmdLimit::clearLogoBase(){
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = -1;
	edgeBase = LOGO_EDGE_RISE;

	process &= ~ARG_PROCESS_BASELOGO;
}
//--- 本来の基準位置を設定（実ロゴ／推測ロゴどちらか１つのみ設定） ---
bool JlsCmdLimit::setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_VALIDLOGO) == 0){
		signalInternalError(ARG_PROCESS_BASELOGO);
	}
	process |= ARG_PROCESS_BASELOGO;
	flagBaseNrf = true;
	nrfBase = nrf;
	nscBase = -1;
	edgeBase = edge;
	return true;
}
bool JlsCmdLimit::setLogoBaseNsc(Nsc nsc, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_VALIDLOGO) == 0){
		signalInternalError(ARG_PROCESS_BASELOGO);
	}
	process |= ARG_PROCESS_BASELOGO;
	flagBaseNrf = false;
	nrfBase = -1;
	nscBase = nsc;
	edgeBase = edge;
	return true;
}
//--- 設定された基準ロゴ位置情報を取得 ---
bool JlsCmdLimit::isLogoBaseExist(){	// 本来の基準ロゴ位置が存在するか？
	return ( nrfBase >= 0 || nscBase >= 0 );
}
bool JlsCmdLimit::isLogoBaseNrf(){		// 本来の基準ロゴ位置が実ロゴか？
	return flagBaseNrf;
}
Nrf JlsCmdLimit::getLogoBaseNrf(){		// 本来の基準ロゴ位置（実ロゴのロゴ番号）
	return nrfBase;
}
Nsc JlsCmdLimit::getLogoBaseNsc(){		// 本来の基準ロゴ位置（推測ロゴのシーンチェンジ番号）
	return nscBase;
}
LogoEdgeType JlsCmdLimit::getLogoBaseEdge(){	// 本来の基準ロゴ立上り／立下り情報
	return edgeBase;
}
//---------------------------------------------------------------------
// ターゲット選択可能範囲
//---------------------------------------------------------------------
//--- ターゲットデータ消去（キャッシュは除く） ---
void JlsCmdLimit::clearTargetData(){
	//--- ターゲット選択可能範囲 ---
	wmsecTarget   = {-1, -1, -1};
	fromLogo      = false;
	//--- ターゲット許可リスト ---
	targetLocDst = {TargetScpType::None, LOGO_EDGE_RISE, false, false, -1, -1, -1};
	targetLocEnd = {TargetScpType::None, LOGO_EDGE_RISE, false, false, -1, -1, -1};

	process &= ~ARG_PROCESS_TARGETRANGE;
	process &= ~ARG_PROCESS_RESULT;
}
//--- ターゲット位置設定 ---
bool JlsCmdLimit::setTargetRange(WideMsec wmsec, bool fromLogo){
	if ((process & ARG_PROCESS_BASELOGO) == 0 && fromLogo){
		signalInternalError(ARG_PROCESS_TARGETRANGE);
	}
	process |= ARG_PROCESS_TARGETRANGE;
	wmsecTarget  = wmsec;
	this->fromLogo = fromLogo;
	return true;
}
//--- ターゲット位置取得 ---
WideMsec JlsCmdLimit::getTargetRangeWide(){
	return wmsecTarget;
}
bool JlsCmdLimit::isTargetRangeFromLogo(){
	return fromLogo;
}

//---------------------------------------------------------------------
// ターゲットに一番近い位置
//---------------------------------------------------------------------
//--- 結果位置／終了位置の書き込み ---
void JlsCmdLimit::setResultDst(TargetLocInfo tgIn){
	if ((process & ARG_PROCESS_TARGETRANGE) == 0 ||
		(process & ARG_PROCESS_SCPENABLE  ) == 0){
//		signalInternalError(ARG_PROCESS_RESULT);
	}
	setResultSubMake(tgIn);
	targetLocDst = tgIn;
}
void JlsCmdLimit::setResultEnd(TargetLocInfo tgIn){
	setResultSubMake(tgIn);
	targetLocEnd = tgIn;
}
void JlsCmdLimit::setResultSubMake(TargetLocInfo& tgIn){
	if ( tgIn.tp == TargetScpType::ScpNum || 
	     tgIn.tp == TargetScpType::Force ){
		tgIn.valid = true;
	}else{
		tgIn.valid = false;
	}
	if ( tgIn.tp == TargetScpType::Invalid ){
		tgIn.nsc = -1;
		tgIn.msout = -1;
	}else if ( tgIn.edge == LogoEdgeType::LOGO_EDGE_FALL ){
		tgIn.msout = tgIn.msbk;
	}else{
		tgIn.msout = tgIn.msec;
	}
}
//--- 結果位置／終了位置の読み出し ---
TargetLocInfo JlsCmdLimit::getResultDst(){
	if ( isPickListValid() ){		// -pick対応
		return getPickListDst();
	}
	return targetLocDst;
}
TargetLocInfo JlsCmdLimit::getResultEnd(){
	if ( isPickListValid() ){		// -pick対応
		return getPickListEnd();
	}
	return targetLocEnd;
}
Nsc JlsCmdLimit::getResultDstNsc(){
	TargetLocInfo tgDst = getResultDst();
	if ( tgDst.tp == TargetScpType::ScpNum ){
		return tgDst.nsc;
	}
	return -1;
}
LogoEdgeType JlsCmdLimit::getResultDstEdge(){
	return getResultDst().edge;
}
TargetLocInfo JlsCmdLimit::getResultDstCurrent(){
	return targetLocDst;
}
TargetLocInfo JlsCmdLimit::getResultEndCurrent(){
	return targetLocEnd;
}

//---------------------------------------------------------------------
// -pick処理
//---------------------------------------------------------------------
void JlsCmdLimit::clearPickList(){
	listPickResult.clear();
	numPickList = 0;
}
void JlsCmdLimit::addPickListCurrent(){
	TargetLocInfoSet dat;
	dat.d = targetLocDst;
	dat.e = targetLocEnd;
	listPickResult.push_back(dat);
}
int JlsCmdLimit::sizePickList(){
	return (int) listPickResult.size();
}
void JlsCmdLimit::selectPickList(int num){
	numPickList = num;
}
bool JlsCmdLimit::isPickListValid(){
	int nsize = sizePickList();
	if ( nsize > 0 && numPickList > 0 && numPickList <= nsize ){
		return true;
	}
	return false;
}
TargetLocInfo JlsCmdLimit::getPickListDst(){
	return listPickResult[numPickList-1].d;
}
TargetLocInfo JlsCmdLimit::getPickListEnd(){
	return listPickResult[numPickList-1].e;
}

//---------------------------------------------------------------------
// エラー確認
//---------------------------------------------------------------------
void JlsCmdLimit::signalInternalError(CmdProcessFlag flags){
	string mes = "error:internal flow at ArgCmdLimit flag=" + to_string(flags) + ",process=" + to_string(process);
	lcerr << mes << endl;
}

