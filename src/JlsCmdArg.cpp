//
// JL�X�N���v�g�p�R�}���h���e�i�[�f�[�^
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsCmdArg.hpp"

///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g�R�}���h�ݒ�l
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// �����ݒ�
//---------------------------------------------------------------------
JlsCmdArg::JlsCmdArg(){
//	this->cmdset = this;
	clear();
}

//---------------------------------------------------------------------
// �R�}���h�ێ����e������
//---------------------------------------------------------------------
void JlsCmdArg::clear(){
	tack = {};		// �O�̂��ߌʂɏ�����
	tack.comFrom     = false;
	tack.useScC      = false;
	tack.floatBase   = false;
	tack.shiftBase   = false;
	tack.virtualLogo = false;
	tack.ignoreComp  = false;
	tack.limitByLogo = false;
	tack.needAuto    = false;
	tack.fullFrameA  = false;
	tack.fullFrameB  = false;
	tack.typeLazy    = LazyType::None;
	tack.ignoreAbort = false;
	tack.immFrom     = false;
	tack.forcePos    = false;
	tack.pickIn      = false;
	tack.pickOut     = false;
	cond = {};		// �O�̂��ߌʂɏ�����
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

	//--- 0�ȊO�̐ݒ� ---
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
	//--- ���������� ---
	setStrOptDefault(OptType::StrRegPos,   "POSHOLD");
	setStrOptDefault(OptType::StrValPosR,  "-1");
	setStrOptDefault(OptType::StrValPosW,  "-1");
	setStrOptDefault(OptType::StrRegList,  "LISTHOLD");
	setStrOptDefault(OptType::StrValListR, "");
	setStrOptDefault(OptType::StrValListW, "");
	setStrOptDefault(OptType::StrRegSize,  "SIZEHOLD");
	setStrOptDefault(OptType::StrRegEnv,   "");
	setStrOptDefault(OptType::StrRegOut,   "LISTHOLD");
	setStrOptDefault(OptType::StrArgVal,   "");
	setStrOptDefault(OptType::StrCounter,  "");
	setStrOptDefault(OptType::StrFileCode, "");
	setStrOptDefault(OptType::ListArgVar,   "");
	setStrOptDefault(OptType::ListFromAbs,  "");
	setStrOptDefault(OptType::ListFromHead, "");
	setStrOptDefault(OptType::ListFromTail, "");
	setStrOptDefault(OptType::ListTgDst,    "");
	setStrOptDefault(OptType::ListTgEnd,    "");
	setStrOptDefault(OptType::ListEndAbs,   "");
	setStrOptDefault(OptType::ListAbsSetFD, "");
	setStrOptDefault(OptType::ListAbsSetFE, "");
	setStrOptDefault(OptType::ListAbsSetFX, "");
	setStrOptDefault(OptType::ListAbsSetXF, "");
	setStrOptDefault(OptType::ListZoneImmL, "");
	setStrOptDefault(OptType::ListZoneImmN, "");
	setStrOptDefault(OptType::ListPickIn,   "");
	setStrOptDefault(OptType::ListPickOut,  "");
}

//---------------------------------------------------------------------
// �I�v�V������ݒ�
//---------------------------------------------------------------------
//--- �I�v�V�������l ---
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
bool JlsCmdArg::getOptFlag(OptType tp){
	int num;
	if ( getRangeOptArray(num, tp) ){
		int ndata = optdata[num];
		if ( ndata < 0 || ndata > 1 ) signalInternalRegError("getOptFlag", tp);
		return ( ndata != 0 );
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

//--- �I�v�V���������� ---
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

//--- �I�v�V�����̃J�e�S������ ---
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
//--- �I�v�V�������e�J�e�S����0���珇�Ԃ̔ԍ��ɕϊ� ---
bool JlsCmdArg::getRangeOptArray(int& num, OptType tp){
	int nTp = static_cast<int>(tp);
	if ( nTp > static_cast<int>(OptType::ArrayMIN) && nTp < static_cast<int>(OptType::ArrayMAX) ){
		num = nTp - static_cast<int>(OptType::ArrayMIN) - 1;
		return true;
	}
	signalInternalRegError("getRangeOptArray", tp);
	return false;
}
bool JlsCmdArg::getRangeStrOpt(int& num, OptType tp){
	int nTp = static_cast<int>(tp);
	if ( nTp > static_cast<int>(OptType::StrMIN) && nTp < static_cast<int>(OptType::StrMAX) ){
		num = nTp - static_cast<int>(OptType::StrMIN) - 1;
		return true;
	}
	signalInternalRegError("getRangeStrOpt", tp);
	return false;
}
void JlsCmdArg::signalInternalRegError(string msg, OptType tp){
	string mes = "error:internal reg access(" + msg + ") OptType=" + to_string(static_cast<int>(tp));
	lcerr << mes << endl;
}

//---------------------------------------------------------------------
// -SC�n�I�v�V�����̐ݒ�ǉ�
//---------------------------------------------------------------------
void JlsCmdArg::addScOpt(OptType tp, TargetCatType tgcat, int tmin, int tmax){
	CmdArgSc scset;
	scset.type     = tp;
	scset.category = tgcat;
	scset.min      = tmin;
	scset.max      = tmax;
	listScOpt.push_back(scset);
}

//---------------------------------------------------------------------
// -SC�n�I�v�V�������擾
//---------------------------------------------------------------------
//--- �R�}���h�擾 ---
OptType JlsCmdArg::getScOptType(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].type;
	}
	return OptType::ScNone;
}
TargetCatType JlsCmdArg::getScOptCategory(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].category;
	}
	return TargetCatType::None;
}
//--- �ݒ�͈͎擾 ---
Msec JlsCmdArg::getScOptMin(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].min;
	}
	return -1;
}
//--- �ݒ�͈͎擾 ---
Msec JlsCmdArg::getScOptMax(int num){
	if (num >= 0 && num < (int) listScOpt.size()){
		return listScOpt[num].max;
	}
	return -1;
}
//--- �i�[���擾 ---
int JlsCmdArg::sizeScOpt(){
	return (int) listScOpt.size();
}

//---------------------------------------------------------------------
// -LG�n�I�v�V�����̐ݒ�ǉ�
//---------------------------------------------------------------------
void JlsCmdArg::addLgOpt(string strNlg){
	listLgVal.push_back(strNlg);
}

//---------------------------------------------------------------------
// -LG�n�I�v�V�������擾
//---------------------------------------------------------------------
//--- �l�擾 ---
string JlsCmdArg::getLgOpt(int num){
	if (num >= 0 && num < (int) listLgVal.size()){
		return listLgVal[num];
	}
	return "";
}
string JlsCmdArg::getLgOptAll(){
	string str;
	for(auto i=0; i<(int)listLgVal.size(); i++){
		if ( i>0 ) str += ",";
		str += listLgVal[i];
	}
	return str;
}
//--- �i�[���擾 ---
int JlsCmdArg::sizeLgOpt(){
	return (int) listLgVal.size();
}

//---------------------------------------------------------------------
// �����擾
//---------------------------------------------------------------------
//--- �ǉ��i�[ ---
void JlsCmdArg::addArgString(const string& strArg){
	listStrArg.push_back(strArg);
}
//--- �����ւ� ---
bool JlsCmdArg::replaceArgString(int n, const string& strArg){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		listStrArg[num] = strArg;
		return true;
	}
	return false;
}
//--- �����擾 ---
string JlsCmdArg::getStrArg(int n){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		return listStrArg[num];
	}
	return "";
}
//--- �����𐔎��ɕϊ����Ď擾 ---
int JlsCmdArg::getValStrArg(int n){
	int num = n - 1;
	if ( num >= 0 && num < (int) listStrArg.size() ){
		return stoi(listStrArg[num]);
	}
	return 0;
}
//--- �������܂Ƃ߂Ď擾 ---
int JlsCmdArg::getListStrArgs(vector<string>& listStr){
	listStr = listStrArg;
	return (int) listStrArg.size();
}
//---------------------------------------------------------------------
// IF�������p
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
