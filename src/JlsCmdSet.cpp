//
// JLスクリプト用コマンド内容格納データ
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsCmdSet.hpp"

///////////////////////////////////////////////////////////////////////
//
// JLスクリプトコマンド設定値
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// 初期設定
//---------------------------------------------------------------------
JlsCmdArg::JlsCmdArg(){
//	this->cmdset = this;
	clear();
}

//---------------------------------------------------------------------
// コマンド保持内容初期化
//---------------------------------------------------------------------
void JlsCmdArg::clear(){
	tack = {};		// 念のため個別に初期化
	tack.floatBase   = false;
	tack.virtualLogo = false;
	tack.ignoreComp  = false;
	tack.limitByLogo = false;
	tack.onePoint    = false;
	tack.needAuto    = false;
	tack.typeLazy    = LazyType::None;
	cond = {};		// 念のため個別に初期化
	cond.numCheckCond = 0;
	cond.flagCond     = false;

	cmdsel        = CmdType::Nop;
	category      = CmdCat::NONE;
	wmsecDst      = {0, 0, 0};
	selectEdge    = LOGO_EDGE_RISE;
	selectAutoSub = CmdTrSpEcID::None;
	listStrArg.clear();

	listLgVal.clear();
	listScOpt.clear();

	for(int i=0; i<SIZE_JLOPT_OPTNUM; i++){
		optdata[i] = 0;
		flagset[i] = false;
	}
	for(int i=0; i<SIZE_JLOPT_OPTSTR; i++){
		optStrData[i] = "";
		flagStrSet[i] = false;
		flagStrUpdate[i] = false;
	}

	//--- 0以外の設定 ---
	setOptDefault(OptType::MsecFrameL,   -1);
	setOptDefault(OptType::MsecFrameR,   -1);
	setOptDefault(OptType::MsecLenPMin,  -1);
	setOptDefault(OptType::MsecLenPMax,  -1);
	setOptDefault(OptType::MsecLenNMin,  -1);
	setOptDefault(OptType::MsecLenNMax,  -1);
	setOptDefault(OptType::MsecLenPEMin, -1);
	setOptDefault(OptType::MsecLenPEMax, -1);
	setOptDefault(OptType::MsecLenNEMin, -1);
	setOptDefault(OptType::MsecLenNEMax, -1);
	setOptDefault(OptType::MsecFromAbs,  -1);
	setOptDefault(OptType::MsecFromHead, -1);
	setOptDefault(OptType::MsecFromTail, -1);
	//--- 初期文字列 ---
	setStrOptDefault(OptType::StrRegPos,   "POSHOLD");
	setStrOptDefault(OptType::StrValPosR,  "-1");
	setStrOptDefault(OptType::StrValPosW,  "-1");
	setStrOptDefault(OptType::StrRegList,  "LISTHOLD");
	setStrOptDefault(OptType::StrValListR, "");
	setStrOptDefault(OptType::StrValListW, "");
	setStrOptDefault(OptType::StrRegSize,  "SIZEHOLD");
}

//---------------------------------------------------------------------
// オプションを設定
//---------------------------------------------------------------------
//--- オプション数値 ---
void JlsCmdArg::setOpt(OptType tp, int val){
	int num;
	if ( getRangeOptArray(num, tp) ){
		optdata[num] = val;
		flagset[num] = true;
	}
}
void JlsCmdArg::setOptDefault(OptType tp, int val){
	int num;
	if ( getRangeOptArray(num, tp) ){
		optdata[num] = val;
		flagset[num] = false;
	}
}
int JlsCmdArg::getOpt(OptType tp){
	int num;
	if ( getRangeOptArray(num, tp) ){
		return optdata[num];
	}
	return false;
}
bool JlsCmdArg::isSetOpt(OptType tp){
	int num;
	if ( getRangeOptArray(num, tp) ){
		return flagset[num];
	}
	return false;
}

//--- オプション文字列 ---
void JlsCmdArg::setStrOpt(OptType tp, const string& str){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		optStrData[num]  = str;
		flagStrSet[num]  = true;
		flagStrUpdate[num] = true;
	}
}
void JlsCmdArg::setStrOptDefault(OptType tp, const string& str){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		optStrData[num]  = str;
		flagStrSet[num]  = false;
		flagStrUpdate[num] = false;
	}
}
string JlsCmdArg::getStrOpt(OptType tp){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		return optStrData[num];
	}
	return "";
}
bool JlsCmdArg::isSetStrOpt(OptType tp){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		return flagStrSet[num];
	}
	return false;
}
void JlsCmdArg::clearStrOptUpdate(OptType tp){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		flagStrUpdate[num] = false;
	}
}
bool JlsCmdArg::isUpdateStrOpt(OptType tp){
	int num;
	if ( getRangeStrOpt(num, tp) ){
		return flagStrUpdate[num];
	}
	return false;
}

//--- オプションのカテゴリ分類 ---
bool JlsCmdArg::getOptCategory(OptCat& category, OptType tp){
	int nTp = static_cast<int>(tp);
	if ( nTp < 0 ){
		return false;
	}
	else if ( nTp > static_cast<int>(OptType::ScMIN) && nTp < static_cast<int>(OptType::ScMAX) ){
		category = OptCat::PosSC;
	}
	else if ( nTp > static_cast<int>(OptType::LgMIN) && nTp < static_cast<int>(OptType::LgMAX) ){
		category = OptCat::NumLG;
	}
	else if ( nTp > static_cast<int>(OptType::FrMIN) && nTp < static_cast<int>(OptType::FrMAX) ){
		category = OptCat::FRAME;
	}
	else if ( nTp > static_cast<int>(OptType::StrMIN) && nTp < static_cast<int>(OptType::StrMAX) ){
		category = OptCat::STR;
	}
	else if ( nTp > static_cast<int>(OptType::ArrayMIN) && nTp < static_cast<int>(OptType::ArrayMAX) ){
		category = OptCat::NUM;
	}
	else{
		return false;
	}
	return true;
}
//--- オプションを各カテゴリの0から順番の番号に変換 ---
bool JlsCmdArg::getRangeOptArray(int& num, OptType tp){
	int nTp = static_cast<int>(tp);
	if ( nTp > static_cast<int>(OptType::ArrayMIN) && nTp < static_cast<int>(OptType::ArrayMAX) ){
		num = nTp - static_cast<int>(OptType::ArrayMIN) - 1;
		return true;
	}
	return false;
}
bool JlsCmdArg::getRangeStrOpt(int& num, OptType tp){
	int nTp = static_cast<int>(tp);
	if ( nTp > static_cast<int>(OptType::StrMIN) && nTp < static_cast<int>(OptType::StrMAX) ){
		num = nTp - static_cast<int>(OptType::StrMIN) - 1;
		return true;
	}
	return false;
}


//---------------------------------------------------------------------
// -SC系オプションの設定追加
//---------------------------------------------------------------------
void JlsCmdArg::addScOpt(OptType tp, bool relative, int tmin, int tmax){
	CmdArgSc scset;
	scset.type     = tp;
	scset.relative = relative;
	scset.min      = tmin;
	scset.max      = tmax;
	listScOpt.push_back(scset);
}

//---------------------------------------------------------------------
// -SC系オプションを取得
//---------------------------------------------------------------------
//--- コマンド取得 ---
OptType JlsCmdArg::getScOptType(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].type;
	}
	return OptType::ScNone;
}
//--- 相対位置コマンド(-RSC等）の判別 ---
bool JlsCmdArg::isScOptRelative(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].relative;
	}
	return false;
}
//--- 設定範囲取得 ---
Msec JlsCmdArg::getScOptMin(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].min;
	}
	return -1;
}
//--- 設定範囲取得 ---
Msec JlsCmdArg::getScOptMax(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].max;
	}
	return -1;
}
//--- 格納数取得 ---
int JlsCmdArg::sizeScOpt(){
	return (int) listScOpt.size();
}

//---------------------------------------------------------------------
// -LG系オプションの設定追加
//---------------------------------------------------------------------
void JlsCmdArg::addLgOpt(string strNlg){
	listLgVal.push_back(strNlg);
}

//---------------------------------------------------------------------
// -LG系オプションを取得
//---------------------------------------------------------------------
//--- 値取得 ---
string JlsCmdArg::getLgOpt(int num){
	if (num >= 0 && num < (int) listLgVal.size()){
		return listLgVal[num];
	}
	return 0;
}
//--- 格納数取得 ---
int JlsCmdArg::sizeLgOpt(){
	return (int) listLgVal.size();
}

//---------------------------------------------------------------------
// 引数取得
//---------------------------------------------------------------------
//--- 追加格納 ---
void JlsCmdArg::addArgString(const string& strArg){
	listStrArg.push_back(strArg);
}
//--- 差し替え ---
bool JlsCmdArg::replaceArgString(int n, const string& strArg){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		listStrArg[num] = strArg;
		return true;
	}
	return false;
}
//--- 引数取得 ---
string JlsCmdArg::getStrArg(int n){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		return listStrArg[num];
	}
	return "";
}
//--- 引数を数字に変換して取得 ---
int JlsCmdArg::getValStrArg(int n){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		return stoi(listStrArg[num]);
	}
	return 0;
}
//---------------------------------------------------------------------
// IF条件式用
//---------------------------------------------------------------------
void JlsCmdArg::setNumCheckCond(int num){
	cond.numCheckCond = num;
}
int JlsCmdArg::getNumCheckCond(){
	return cond.numCheckCond;
}
void JlsCmdArg::setCondFlag(bool flag){
	cond.flagCond = flag;
}
bool JlsCmdArg::getCondFlag(){
	return cond.flagCond;
}

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
	listValidLogo.clear();
	nrfBase = -1;
	nscBase = -1;
	edgeBase = LOGO_EDGE_RISE;
	listWmsecLogoBase.clear();
	locMsecLogoBase = -1;
	wmsecTarget = {-1, -1, -1};
	msecTargetFc = -1;
	fromLogo = false;
	listTLRange.clear();
	listScpEnable.clear();
	nscSel = -1;
	nscEnd = -1;
	edgeOutPos = LOGO_EDGE_RISE;
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
Msec JlsCmdLimit::getLogoListMsec(int nlist){
	if (nlist < 0 || nlist >= (int) listValidLogo.size()){
		return -1;
	}
	return listValidLogo[nlist].msec;
}

LogoEdgeType JlsCmdLimit::getLogoListEdge(int nlist){
	if (nlist < 0 || nlist >= (int) listValidLogo.size()){
		return LOGO_EDGE_RISE;
	}
	return listValidLogo[nlist].edge;
}

bool JlsCmdLimit::addLogoList(Msec &rmsec, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_HEADTAIL) == 0){
		signalInternalError(ARG_PROCESS_VALIDLOGO);
	}
	process |= ARG_PROCESS_VALIDLOGO;
	ArgValidLogo argset = {rmsec, edge};
	listValidLogo.push_back(argset);
	return true;
}

int JlsCmdLimit::sizeLogoList(){
	return (int) listValidLogo.size();
}

//---------------------------------------------------------------------
// 対象とする基準ロゴ選択
//---------------------------------------------------------------------
Nrf JlsCmdLimit::getLogoBaseNrf(){
	return nrfBase;
}

Nsc JlsCmdLimit::getLogoBaseNsc(){
	return nscBase;
}

LogoEdgeType JlsCmdLimit::getLogoBaseEdge(){
	return edgeBase;
}

bool JlsCmdLimit::setLogoBaseNrf(Nrf nrf, jlsd::LogoEdgeType edge){
	if ((process & ARG_PROCESS_VALIDLOGO) == 0){
		signalInternalError(ARG_PROCESS_BASELOGO);
	}
	process |= ARG_PROCESS_BASELOGO;
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
	nrfBase = -1;
	nscBase = nsc;
	edgeBase = edge;
	return true;
}

void JlsCmdLimit::setLogoWmsecList(vector<WideMsec>& listWmsec, int locBase){
	listWmsecLogoBase = listWmsec;
	locMsecLogoBase = locBase;
}
void JlsCmdLimit::getLogoWmsecBase(WideMsec& wmsec, int step, bool flagWide){
	getLogoWmsecBaseShift(wmsec, step, flagWide, 0);
}
void JlsCmdLimit::getLogoWmsecBaseShift(WideMsec& wmsec, int step, bool flagWide, int sft){
	int locMd   = locMsecLogoBase + sft;
	int locSt   = locMsecLogoBase + sft - step;
	int locEd   = locMsecLogoBase + sft + step;
	int locSize = (int)listWmsecLogoBase.size();
	if ( 0 <= locMd && locMd < locSize ){
		wmsec.just = listWmsecLogoBase[locMd].just;
	}else{
		wmsec.just = -1;
	}
	if ( 0 <= locSt && locSt < locSize ){
		if ( flagWide ){
			wmsec.early = listWmsecLogoBase[locSt].early;
		}else{
			wmsec.early = listWmsecLogoBase[locSt].just;
		}
	}else{
		wmsec.early = -1;
	}
	if ( 0 <= locEd && locEd < locSize ){
		if ( flagWide ){
			wmsec.late = listWmsecLogoBase[locEd].late;
		}else{
			wmsec.late = listWmsecLogoBase[locEd].just;
		}
	}else{
		wmsec.late = -1;
	}
}

//---------------------------------------------------------------------
// ターゲット選択可能範囲
//---------------------------------------------------------------------
WideMsec JlsCmdLimit::getTargetRangeWide(){
	return wmsecTarget;
}

Msec JlsCmdLimit::getTargetRangeForce(){
	return msecTargetFc;
}

bool JlsCmdLimit::isTargetRangeLogo(){
	return fromLogo;
}

bool JlsCmdLimit::setTargetRange(WideMsec wmsec, Msec msec_force, bool from_logo){
	if ((process & ARG_PROCESS_BASELOGO) == 0 && from_logo){
		signalInternalError(ARG_PROCESS_TARGETRANGE);
	}
	process |= ARG_PROCESS_TARGETRANGE;
	wmsecTarget  = wmsec;
	msecTargetFc = msec_force;
	fromLogo     = from_logo;
	return true;
}

//--- 出力エッジ設定 ---
void JlsCmdLimit::setTargetOutEdge(LogoEdgeType edge){
	edgeOutPos = edge;
}
LogoEdgeType JlsCmdLimit::getTargetOutEdge(){
	return edgeOutPos;
}

//---------------------------------------------------------------------
// ターゲット許可リスト
//---------------------------------------------------------------------
bool JlsCmdLimit::isTargetListed(Msec msec_target){
	int nsize = (int) listTLRange.size();
	//--- リストがなければ無条件で許可 ---
	if (nsize == 0) return true;
	//--- 各リスト検索 ---
	bool det = false;
	for(int i=0; i<nsize; i++){
		if (msec_target >= listTLRange[i].st && msec_target <= listTLRange[i].ed){
			det = true;
		}
	}
	return det;
}

void JlsCmdLimit::clearTargetList(){
	listTLRange.clear();
}

void JlsCmdLimit::addTargetList(RangeMsec rmsec){
	listTLRange.push_back(rmsec);
}

//---------------------------------------------------------------------
// 無音条件判定
//---------------------------------------------------------------------
bool JlsCmdLimit::getScpEnable(Nsc nsc){
	if (nsc < 0 || nsc >= (int) listScpEnable.size()){
		return false;
	}
	return listScpEnable[nsc];
}

bool JlsCmdLimit::setScpEnable(vector<bool> &listEnable){
	process |= ARG_PROCESS_SCPENABLE;
	listScpEnable = listEnable;
	return true;
}

int JlsCmdLimit::sizeScpEnable(){
	return (int) listScpEnable.size();
}

//---------------------------------------------------------------------
// ターゲットに一番近い位置
//---------------------------------------------------------------------
Nsc JlsCmdLimit::getResultTargetSel(){
	return nscSel;
}

Nsc JlsCmdLimit::getResultTargetEnd(){
	return nscEnd;
}

bool JlsCmdLimit::setResultTarget(Nsc nscSelIn, Nsc nscEndIn){
	if ((process & ARG_PROCESS_TARGETRANGE) == 0 ||
		(process & ARG_PROCESS_SCPENABLE  ) == 0){
		signalInternalError(ARG_PROCESS_RESULT);
	}
	process |= ARG_PROCESS_RESULT;
	nscSel = nscSelIn;
	nscEnd = nscEndIn;
	return true;
}

//---------------------------------------------------------------------
// エラー確認
//---------------------------------------------------------------------
void JlsCmdLimit::signalInternalError(CmdProcessFlag flags){
	cerr << "error:internal flow at ArgCmdLimit flag=" << flags << ",process=" << process << endl;
}

