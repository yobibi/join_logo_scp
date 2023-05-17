//
// �ϐ��A�N�Z�X�֘A����
//

#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrFuncReg.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsScrGlobal.hpp"
#include "JlsCmdArg.hpp"
#include "JlsDataset.hpp"


void JlsScrFuncReg::setDataPointer(JlsDataset *pdata, JlsScrGlobal *pglobal, JlsScrFuncList *plist){
	this->pdata = pdata;
	this->pGlobalState = pglobal;
	this->pFuncList = plist;
}

//=====================================================================
// �N���I�v�V��������
//=====================================================================

//---------------------------------------------------------------------
// �X�N���v�g���ŋL�ڂ���N���I�v�V����
// ���́F
//   argrest    �F�����c�萔
//   strv       �F�����R�}���h
//   str1       �F�����l�P
//   str2       �F�����l�Q
//   overwrite  �F�������ݍς݂̃I�v�V�����ݒ�ifalse=���Ȃ� true=����j
//   checklevel �F�G���[�m�F���x���i0=�Ȃ� 1=�F�������I�v�V�����`�F�b�N�j
// �o�́F
//   �Ԃ�l  �F�����擾��(-1�̎��擾�G���[�A0�̎��Y���R�}���h�Ȃ�)
//---------------------------------------------------------------------
int JlsScrFuncReg::setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite){
	if (argrest <= 0){
		return 0;
	}
	bool exist2 = false;
	bool exist3 = false;
	if (argrest >= 2){
		exist2 = true;
	}
	if (argrest >= 3){
		exist3 = true;
	}
	int numarg = 0;
	if(strv[0] == '-' && strv[1] != '\0') {
		if (!_stricmp(strv, "-flags")){
			if (!exist2){
				outputMesErr("-flags needs an argument\n");
				return -1;
			}
			else{
				if (setInputFlags(str1, overwrite) == false){
					outputMesErr("-flags bad argument\n");
					return -1;
				}
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-set")){
			if (!exist3){
				outputMesErr("-set needs two arguments\n");
				return -1;
			}
			else{
				if (setInputReg(str1, str2, overwrite) == false){
					outputMesErr("-set bad argument\n");
					return -1;
				}
			}
			numarg = 3;
		}
		else if (!_stricmp(strv, "-cutmrgin")){
			if (!exist2){
				outputMesErr("-cutmrgin needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixCutIn == 0){
				pdata->extOpt.msecCutIn = setOptionsCnvCutMrg(str1);
				pdata->extOpt.fixCutIn = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgout")){
			if (!exist2){
				outputMesErr("-cutmrgout needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixCutOut == 0){
				pdata->extOpt.msecCutOut = setOptionsCnvCutMrg(str1);
				pdata->extOpt.fixCutOut = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgwi")){
			if (!exist2){
				outputMesErr("-cutmrgwi needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixWidCutI == 0){
				int val = atoi(str1);
				pdata->extOpt.wideCutIn  = val;
				pdata->extOpt.fixWidCutI = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-cutmrgwo")){
			if (!exist2){
				outputMesErr("-cutmrgwo needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixWidCutO == 0){
				int val = atoi(str1);
				pdata->extOpt.wideCutOut = val;
				pdata->extOpt.fixWidCutO = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-sublist")){
			if (!exist2){
				outputMesErr("-sublist needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubList == 0){
				if ( str1[0] == '+' ){
					string tmp = str1;
					pdata->extOpt.subList = tmp.substr(1) + "," + pdata->extOpt.subList;
				}else{
					pdata->extOpt.subList = str1;
				}
				pdata->extOpt.fixSubList = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-subpath")){
			if (!exist2){
				outputMesErr("-subpath needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubPath == 0){
				pdata->extOpt.subPath = str1;
				pdata->extOpt.fixSubPath = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-setup")){
			if (!exist2){
				outputMesErr("-setup needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSetup == 0){
				pdata->extOpt.setup = str1;
				pdata->extOpt.fixSetup = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-syscode")){
			if (!exist2){
				outputMesErr("-syscode needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixNSysCode == 0){
				//--- �W���o��/�G���[�̕����R�[�h�͍ŗD��Őݒ肵�Ă��������̂ł������s ---
				int num = LSys.getUtfNumFromStr(str1);
				if ( num < 0 ){
					outputMesErr("-syscode unsupported data\n");
					return -1;
				}
				pdata->extOpt.nSysCode = num;
				pdata->extOpt.fixNSysCode = 1;
				LSys.setStdUtfCodeFromNum(num);		// �����ݒ�
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-vline")){	// debug
			if (!exist2){
				outputMesErr("-vline needs an argument\n");
				return -1;
			}
			if (overwrite || pdata->extOpt.fixVLine == 0){
				int val = atoi(str1);
				pdata->extOpt.vLine = val;
				pdata->extOpt.fixVLine = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inlogo")){	// ���O�̂ݕێ�
			pdata->extOpt.logofile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inscp")){	// ���O�̂ݕێ�
			pdata->extOpt.scpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-incmd")){	// ���O�̂ݕێ�
			pdata->extOpt.cmdfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-o")){	// ���O�̂ݕێ�
			pdata->extOpt.outfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-oscp")){	// ���O�̂ݕێ�
			pdata->extOpt.outscpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-odiv")){	// ���O�̂ݕێ�
			pdata->extOpt.outdivfile = str1;
			numarg = 2;
		}
	}
	return numarg;
}

//---------------------------------------------------------------------
// CutMrgIn / CutMrgOut �I�v�V���������p 30fps�t���[�������͂Ń~���b��Ԃ�
//---------------------------------------------------------------------
Msec JlsScrFuncReg::setOptionsCnvCutMrg(const char* str){
	int num = atoi(str);
	int frac = 0;
	const char *tmpstr = strchr(str, '.');
	if (tmpstr != nullptr){
		if (tmpstr[1] >= '0' && tmpstr[1] <= '9'){
			frac = (tmpstr[1] - '0') * 10;
			if (tmpstr[2] >= '0' && tmpstr[2] <= '9'){
				frac += (tmpstr[2] - '0');
			}
		}
	}
	//--- 30fps�Œ�ϊ����� ---
	Msec msec_num  = (abs(num) * 1001 + 30/2) / 30;
	Msec msec_frac = (frac * 1001 + 30/2) / 30 / 100;
	Msec msec_result = msec_num + msec_frac;
	if (num < 0) msec_result = -1 * msec_result;
	return msec_result;
}

//---------------------------------------------------------------------
// �ϐ����O������ݒ�
// �o�́F
//   �Ԃ�l  �Ftrue=����I��  false=���s
//---------------------------------------------------------------------
bool JlsScrFuncReg::setInputReg(const char *name, const char *val, bool overwrite){
	return setJlsRegVar(name, val, overwrite);
}

//---------------------------------------------------------------------
// �I�v�V�����t���O��ݒ�
// �o�́F
//   �Ԃ�l  �Ftrue=����I��  false=���s
//---------------------------------------------------------------------
bool JlsScrFuncReg::setInputFlags(const char *flags, bool overwrite){
	bool ret = true;
	int pos = 0;
	string strBuf = flags;
	while(pos >= 0){
		string strFlag;
		pos = pdata->cnv.getStrWord(strFlag, strBuf, pos);
		if (pos >= 0){
			string strName, strVal;
			//--- �e�t���O�̒l��ݒ� ---
			int nloc = (int) strFlag.find(":");
			if (nloc >= 0){
				strName = strFlag.substr(0, nloc);
				strVal  = strFlag.substr(nloc+1);
			}
			else{
				strName = strFlag;
				strVal  = "1";
			}
			//--- �ϐ��i�[ ---
			bool flagtmp = setJlsRegVar(strName, strVal, overwrite);
			if (flagtmp == false) ret = false;
		}
	}
	return ret;
}

//=====================================================================
// ���W�X�^�A�N�Z�X����
//=====================================================================

//---------------------------------------------------------------------
// Call���s�������Ƀ��[�J���ϐ��ݒ肳��������ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//---------------------------------------------------------------------
//--- �֐��̕Ԃ�l�ɂȂ�ϐ�����ݒ� ---
void JlsScrFuncReg::setArgFuncName(const string& strName){
	pGlobalState->setArgFuncName(strName);
}
//--- �Q�Ɠn���Őݒ� ---
bool JlsScrFuncReg::setArgRefReg(const string& strName, const string& strVal){
	if ( !setArgRegCheckName(strName) ) return false;	// �֐������Ƃ��Ă̕ϐ����ُ�`�F�b�N
	if ( !isValidAsRegName(strVal) ) return false;		// ��ʂ̕ϐ����Ƃ��Ĉُ�`�F�b�N
	string strRefVal = strVal;
	int lenFullVar;
	if ( fixJlsRegNameAtList(strRefVal, lenFullVar, true) ){	// ���X�g�v�f�Ή��Őݒ�
		return pGlobalState->setArgRefReg(strName, strRefVal);
	}
	return false;
}
//--- �ϐ��l��ݒ� ---
bool JlsScrFuncReg::setArgRegByVal(const string& strName, const string& strVal){
	if ( !setArgRegCheckName(strName) ) return false;	// �֐������Ƃ��Ă̕ϐ����ُ�`�F�b�N
	return pGlobalState->setArgReg(strName, strVal);
}
//--- �l���ϐ����̏ꍇ�̐ݒ� ---
bool JlsScrFuncReg::setArgRegByName(const string& strName, const string& strValName){
	string strVal;
	if ( !getJlsRegVarNormal(strVal, strValName) ){
		pGlobalState->addMsgErrorN("error: not exist " + strValName);
		return false;
	}
	return setArgRegByVal(strName, strVal);
}
//--- �l���ϐ������l���̂��̂����ʂ��Đݒ� ---
bool JlsScrFuncReg::setArgRegByBoth(const string& strName, const string& strVal, bool quote){
	if ( isValidAsRegName(strVal) ){
		return setArgRegByName(strName, strVal);
	}
	string strEval = strVal;
	if ( quote ){	// ���p�����O������
		pdata->cnv.getStrItemArg(strEval, strVal, 0);
	}
	return setArgRegByVal(strName, strEval);
}
//--- �ϐ����Ƃ��ėL���Ȗ��O���`�F�b�N ---
bool JlsScrFuncReg::isValidAsRegName(const string& strName){
	string strNamePart = strName;
	if ( strName.find("\"") != string::npos ) return false;
	if ( strName.find("\'") != string::npos ) return false;
	//--- ���X�g�ϐ��̓��X�g������ ---
	auto npos = strNamePart.find("[");
	if ( npos != string::npos ){
		strNamePart = strNamePart.substr(npos);
	}
	if ( strNamePart.empty() ) return false;
	//--- �m�F ---
	return !(pGlobalState->checkErrRegName(strNamePart));
}
//--- �֐��������Ƃ��Ă̕ϐ����ُ�`�F�b�N ---
bool JlsScrFuncReg::setArgRegCheckName(const string& strName){
	bool success = true;
	string mesErr;
	if ( strName.empty() ){
		success = false;
		mesErr = "empty";
	}else if ( !isValidAsRegName(strName) ){
		success = false;
		mesErr = "illegal";
	}else{
		//--- ���X�g�ϐ��̗v�f�ň����錾�͔�Ή� ---
		auto locSt = strName.find("[");
		if ( locSt != string::npos ){
			success = false;
			mesErr = "argument array";
		}
	}
	if ( !success ){
		mesErr = "error: name " + mesErr;
		mesErr += " ";
		mesErr += strName;
		pGlobalState->addMsgErrorN(mesErr);
	}
	return success;
}
//---------------------------------------------------------------------
// �ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
//--- �ϐ��̖���`�� ---
bool JlsScrFuncReg::unsetJlsRegVar(const string& strName, bool flagLocal){
	return pGlobalState->unsetRegVar(strName, flagLocal);
}
//--- �ʏ�̕ϐ���ݒ� ---
bool JlsScrFuncReg::setJlsRegVar(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = false;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- ���[�J���ϐ���ݒ�i�����͒ʏ�ϐ��Ɠ���j ---
bool JlsScrFuncReg::setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = true;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- �ʏ�̕ϐ��ƃ��[�J���ϐ���I�����Đݒ� ---
bool JlsScrFuncReg::setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	if ( strName.empty() ) return false;
	//--- ���X�g�ϐ��Ή� ---
	string strNameWrite = strName;
	int lenFullVar;
	bool success = fixJlsRegNameAtList(strNameWrite, lenFullVar, true);		// exact=true
	//--- �������ݏ��� ---
	if ( success ){
		success = pGlobalState->setRegVarCommon(strNameWrite, strVal, overwrite, flagLocal);
		setJlsRegVarCouple(strNameWrite, strVal);
	}
	return success;
}
bool JlsScrFuncReg::setJlsRegVarCountUp(const string& strName, int step, bool flagLocal){
	//--- ���ݒl�擾 ---
	string strVal;
	if ( !getJlsRegVarNormal(strVal, strName) ) return false;
	//--- �����ɕϊ����ĉ��Z�㏑���߂� ---
	int val;
	int pos = 0;
	pos = pdata->cnv.getStrValNum(val, strVal, pos);
	if ( pos < 0 ) return false;
	val += step;
	return setJlsRegVarWithLocal(strName, to_string(val), true, flagLocal);
}
//--- �ϐ��ݒ��̃V�X�e���ϐ��X�V ---
void JlsScrFuncReg::setJlsRegVarCouple(const string& strName, const string& strVal){
	//--- �V�X�e���ϐ��̓��ꏈ�� ---
	if (_stricmp(strName.c_str(), "RANGETYPE") == 0){
		pdata->cnv.getStrValNum(pdata->recHold.typeRange, strVal, 0);
	}
	//--- HEAD/TAIL���ԏ��� ---
	int type_add = 0;
	string strAddName;
	if (_stricmp(strName.c_str(), "HEADFRAME") == 0){
		strAddName = "HEADTIME";
		type_add = 1;
	}
	else if (_stricmp(strName.c_str(), "TAILFRAME") == 0){
		strAddName = "TAILTIME";
		type_add = 1;
	}
	else if (_stricmp(strName.c_str(), "HEADTIME") == 0){
		strAddName = "HEADFRAME";
		type_add = 2;
	}
	else if (_stricmp(strName.c_str(), "TAILTIME") == 0){
		strAddName = "TAILFRAME";
		type_add = 2;
	}
	if (type_add > 0){
		int val;
		if (pdata->cnv.getStrValMsecM1(val, strVal, 0)){
			string strAddVal;
			if (type_add == 2){
				strAddVal = pdata->cnv.getStringFrameMsecM1(val);
			}
			else{
				strAddVal = pdata->cnv.getStringTimeMsecM1(val);
			}
			bool flagLocal = false;
			bool overwrite = true;
			pGlobalState->setRegVarCommon(strAddName, strAddVal, overwrite, flagLocal);
		}
		//--- head/tail�����X�V ---
		{
			string strHead;
			if ( getJlsRegVarNormal(strHead, "HEADTIME") ){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.st, strHead, 0);
			}
			string strTail;
			if ( getJlsRegVarNormal(strTail, "TAILTIME") ){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.ed, strTail, 0);
			}
			//--- ABSHEAD/ABSTAIL���X�V ---
			string strAbsHead = strHead;
			string strAbsTail = strTail;
			if ( pdata->recHold.rmsecHeadTail.st < 0 ){
				int valZero = 0;
				strAbsHead = pdata->cnv.getStringTimeMsecM1(valZero);
			}
			if ( pdata->recHold.rmsecHeadTail.ed < 0 ){
				getJlsRegVarNormal(strAbsTail, "MAXTIME");
			}
			bool flagLocal = false;
			bool overwrite = true;
			pGlobalState->setRegVarCommon("ABSHEAD", strAbsHead, overwrite, flagLocal);
			pGlobalState->setRegVarCommon("ABSTAIL", strAbsTail, overwrite, flagLocal);
		}
	}
	//--- MAXTIME�X�V ---
	if ( _stricmp(strName.c_str(), "MAXTIME") == 0 ){
		string strTail;
		bool validTail = false;
		if ( getJlsRegVarNormal(strTail, "TAILTIME") ){
			int val;
			if ( pdata->cnv.getStrValMsecM1(val, strTail, 0) ){
				if ( val >= 0 ) validTail = true;
			}
		}
		if ( !validTail ){
			strTail = strVal;
		}
		bool flagLocal = false;
		bool overwrite = true;
		pGlobalState->setRegVarCommon("ABSTAIL", strTail, overwrite, flagLocal);
	}
}

//---------------------------------------------------------------------
// �ϐ���ǂݏo��
//---------------------------------------------------------------------
bool JlsScrFuncReg::getJlsRegVarNormal(string& strVal, const string& strName){
	return ( getJlsRegVar(strVal, strName, true) > 0 );
}
int JlsScrFuncReg::getJlsRegVarPartName(string& strVal, const string& strCandName, bool exact){
	return getJlsRegVar(strVal, strCandName, exact);
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo��
// ���́F
//   strCandName : �ǂݏo���ϐ����i���j
//   excact      : 0=���͕����ɍő�}�b�`����ϐ�  1=���͕����Ɗ��S��v����ϐ�
// �o�́F
//   �Ԃ�l  : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsScrFuncReg::getJlsRegVar(string& strVal, const string& strCandName, bool exact){
	//--- �����I�v�V�����`�F�b�N ---
	string strNamePart = strCandName;
	string strDivPart;
	int lenDivFullVar;
	bool flagDivOpt = false;
	if ( exact ){
		flagDivOpt = checkJlsRegDivide(strNamePart, strDivPart, lenDivFullVar);
	}
	//--- ���X�g�ϐ��̃`�F�b�N ---
	bool flagNum = false;
	if ( strNamePart[0] == '#' ){		// ���X�g�̗v�f��
		flagNum = true;
		strNamePart = strNamePart.substr(1);
	}
	int lenFullVar;
	if ( !fixJlsRegNameAtList(strNamePart, lenFullVar, exact) ){	// ���X�g�v�f�␳
		strVal.clear();
		return 0;		// ���X�g�v�f�����ُ̈�ɂ��I��
	}

	//--- �ʏ�̃��W�X�^�ǂݏo�� ---
	int lenVar = pGlobalState->getRegVarCommon(strVal, strNamePart, exact);

	//--- ���X�g�ϐ����̕␳ ---
	if ( (int)strNamePart.length() == lenVar ){	// ������̍Ō�܂ŕϐ��������ꍇ
		lenVar = lenFullVar;		// [���ڔԍ�]���݂̕ϐ������񒷂ɂ���
	}
	if ( flagNum ){		// ���X�g�̗v�f���擾
		int numList = pFuncList->getListStrSize(strVal);
		strVal = to_string(numList);
	}
	//--- �����I�v�V�����ɂ�镪�� ---
	if ( flagDivOpt ){
		if ( divideJlsRegVar(strVal, strDivPart) ){
			lenVar = lenDivFullVar;		// �������͌��̕����񒷂ɖ߂�
		}else{
			lenVar = 0;		// �I�v�V�����G���[��
		}
	}
	return lenVar;
}
//---------------------------------------------------------------------
// �p�X�E�g���q�����I�v�V�����t�����`�F�b�N
// ���́F
//   strNamePart : �ϐ����i�����I�v�V�������݁j
// �o�́F
//   �Ԃ�l  : true=�����I�v�V�����t��  false=�ʏ�̕ϐ�
//   strNamePart : �ϐ����i�����I�v�V�����͏����j
//   strDivPart  : �����I�v�V����
//   lenFullVar  : �����I�v�V�������݂̕ϐ�������
//---------------------------------------------------------------------
bool JlsScrFuncReg::checkJlsRegDivide(string& strNamePart, string& strDivPart, int& lenFullVar){
	lenFullVar = (int) strNamePart.length();

	auto locSt = strNamePart.find("]");		// ���X�g�z�񎞂͗v�f�ォ��
	if ( locSt == string::npos ){
		locSt = 0;
	}
	locSt = strNamePart.find(":", locSt);
	if ( locSt == string::npos ){
		strDivPart.clear();
		return false;
	}
	strDivPart = strNamePart.substr(locSt);
	strNamePart = strNamePart.substr(0, locSt);
	return true;
}
//---------------------------------------------------------------------
// �p�X�E�g���q�����I�v�V�����̎��s
// ���́F
//   strVal      : �ϐ��l�i�����O�j
//   strDivPart  : �����I�v�V����
// �o�́F
//   �Ԃ�l  : true=�����I�v�V�������s  false=�����I�v�V�����G���[
//   strVal      : �ϐ��l�i������j
//---------------------------------------------------------------------
bool JlsScrFuncReg::divideJlsRegVar(string& strVal, const string& strDivPart){
	auto locSt = strDivPart.find(":");
	if ( locSt == string::npos ) return true;	// �ʏ�i:�Ȃ��j�͐���I��

	string strOpt = strDivPart;
	if ( strDivPart.find("$") != string::npos ){	// �����ɕϐ����g���Ă�����u��
		replaceBufVar(strOpt, strDivPart);
	}
	while( locSt != string::npos ){
		string strCmd;
		auto locNext = strOpt.find(":", locSt+1);
		if ( locNext == string::npos ){
			strCmd = strOpt.substr(locSt+1);
		}else{
			strCmd = strOpt.substr(locSt+1, locNext-locSt-1);
		}
		if ( !divideJlsRegVarDecode(strVal, strCmd) ){
			return false;
		}
		locSt = locNext;
	}
	return true;
}
//--- �p�X�E�g���q�����R�}���h�̃f�R�[�h�Ǝ��s�B�G���[�̎���false��Ԃ� ---
bool JlsScrFuncReg::divideJlsRegVarDecode(string& strVal, const string& strCmd){
	//--- �R�}���h������f�R�[�h ---
	VarProcRecord var;
	bool success = divideJlsRegVarDecodeIn(var, strCmd);
	if ( !success ){
		return false;
	}
	//--- �g���q�ݒ� ---
	string strDelim;
	if ( pGlobalState->getRegVarCommon(strDelim, DefRegExtChar, true) <= 0 ){
		strDelim.clear();
	}
	if ( strDelim.empty() ){	// �ꍇ�����s�v�ɂ��邽�ߕK������1�ȏ�̕�����ɂ���
		strDelim = ".";
	}
	string strSubDelim;
	pGlobalState->getRegVarCommon(strSubDelim, DefRegExtCsub, true);
	//--- �e�폈�����s ---
	switch( var.typeProc ){
		case VarProcType::path :		// �p�X����
			pdata->cnv.getStrDivPath(strVal, var.selHead, var.withDelim);
			break;
		case VarProcType::divext :		// �g���q����
			pdata->cnv.getStrDivide(strVal, strDelim, var.selHead, var.selPath);
			break;
		case VarProcType::substr :	// ����������̊J�n�ʒu�ƒ�����ǂݍ��ݎ��s
			{
				int nSt;
				int pos = pdata->cnv.getStrValFuncNum(nSt, strCmd, 1);
				if ( pos >= 0 ){
					LocalStr lcstr;
					if ( pos == (int) strCmd.length() ){	// �����ȗ���
						strVal = lcstr.getSubStr(strVal, nSt);
					}else if ( strCmd[pos] != ',' ){	// ��؂�� , ����
						pos = -1;
					}else{
						int nLen;
						pos = pdata->cnv.getStrValFuncNum(nLen, strCmd, pos+1);
						strVal = lcstr.getSubStrLen(strVal, nSt, nLen);
					}
				}
				if ( pos < 0 ) return false;
			}
			break;
		case VarProcType::exchg :	// �g���q�������u��
			if ( var.selInStr ){	// �e�������ꂼ��u��
				LocalStr lcstr;
				success = lcstr.replaceInStr(strVal, strDelim, strSubDelim);
			}else if ( var.selQuote ){		// �N�H�[�g�u��
				if ( var.selBackup ){
					backupStrQuote(strVal);
				}else{
					restoreStrQuote(strVal);
				}
			}else{
				replaceStrAllFind(strVal, strDelim, strSubDelim);
			}
			break;
		case VarProcType::blank :	// �X�y�[�X������S������
			{
				std::regex re("\\s+");	// �X�y�[�X�͐��K�\���ł��}���`�o�C�g�����s�v
				strVal = std::regex_replace(strVal, re, "");
			}
			break;
		case VarProcType::trim :	// �X�y�[�X������O��ŏ���
			{
				if ( var.selHead ){
					std::regex re("^\\s+");
					strVal = std::regex_replace(strVal, re, "");
				}
				if ( var.selTail ){
					std::regex re("\\s+$");
					strVal = std::regex_replace(strVal, re, "");
				}
			}
			break;
		case VarProcType::chpath :	// �Ōオ�p�X��؂�łȂ���΃p�X��؂��ǉ�
			pdata->cnv.getStrFileAllPath(strVal);	// �Ō�ɋ�؂�t��
			break;
		case VarProcType::frame :	// ���Ԃ��t���[�����ɕϊ�
			{
				vector<Msec> listTmp;
				if ( pdata->cnv.getListValMsecM1(listTmp, strVal) ){
					strVal.clear();
					for(int i=0; i<(int)listTmp.size(); i++){
						if ( i>0 ) strVal += ",";
						strVal += pdata->cnv.getStringFrameMsecM1(listTmp[i]);
					}
				}
			}
			break;
		case VarProcType::match :	// ���K�\������
			{
				LocalStr lcstr;
				strVal = lcstr.getRegMAtch(strVal, strDelim);
			}
			break;
		case VarProcType::count :	// �g���q�o�������o��
			{
				int mc = 0;
				if ( var.selInStr ){		// �����ꂩ�̕���
					LocalStr lcstr;
					mc = lcstr.countInStr(strVal, strDelim);
				}
				else if ( var.selRegEx ){		// ���K�\����
					LocalStr lcstr;
					mc = lcstr.countRegExMatch(strVal, strDelim);
				}
				else{
					auto n = strVal.find(strDelim);
					while( n != string::npos ){
						mc ++;
						n = strVal.find(strDelim, n+1);
					}
				}
				strVal = to_string(mc);
			}
			break;
		case VarProcType::len :	// �����񒷂��o��
			{
				LocalStr lcstr;
				int len = lcstr.getStrLen(strVal);
				strVal = to_string(len);
			}
			break;
		default :
			break;
	}
	return true;
}
//--- �R�}���h�����������F�� ---
bool JlsScrFuncReg::divideJlsRegVarDecodeIn(VarProcRecord& var, const string& strCmd){
	bool success   = true;
	var.typeProc = VarProcType::none;
	var.selHead   = false;
	var.selTail   = false;
	var.selPath   = true;		// �g���q�����̓p�X��Ɍ��肵�čs���ݒ�
	var.withDelim = false;		// �p�X�̍ŏI��؂�͏o�͂��Ȃ��ݒ�
	var.selRegEx  = false;		// ���K�\���Ō���
	var.selInStr  = false;		// �e�����Ō���
	var.selQuote  = false;		// ���p������
	var.selBackup = false;		// �o�b�N�A�b�v�I��
	if ( strCmd.length() == 1 ){
		switch( strCmd[0] ){
			case 'r' :			// �g���q����菜��
				var.typeProc = VarProcType::divext;
				var.selHead = true;
				break;
			case 'e' :			// �g���q�̂ݎc��
				var.typeProc = VarProcType::divext;
				var.selTail = true;
				break;
			case 'h' :			// �p�X�����̂ݎc��
				var.typeProc = VarProcType::path;
				var.selHead = true;
				break;
			case 't' :			// �t�@�C�����ȍ~�̂ݎc��
				var.typeProc = VarProcType::path;
				var.selTail = true;
				break;
			case '~' :			// ������������c��
				var.typeProc = VarProcType::substr;
				break;
			case '<' :			// �X�y�[�X������O���ŏ���
				var.typeProc = VarProcType::trim;
				var.selHead = true;
				break;
			case '>' :			// �X�y�[�X�������㑤�ŏ���
				var.typeProc = VarProcType::trim;
				var.selTail = true;
				break;
			case '/' :			// �Ōオ�p�X��؂�łȂ���΃p�X��؂��ǉ�
				var.typeProc = VarProcType::chpath;
				break;
			case 'f' :			// ���Ԃ��t���[�����ɕϊ�
				var.typeProc = VarProcType::frame;
				break;
			case 'X' :			// �g���q�������u��
				var.typeProc = VarProcType::exchg;
				break;
			case 'R' :			// �g���q����菜���i�p�X�𖳎����Đ擪���猟���j
				var.typeProc = VarProcType::divext;
				var.selHead = true;
				var.selPath = false;
				break;
			case 'E' :			// �g���q�̂ݎc���i�p�X�𖳎����Đ擪���猟���j
				var.typeProc = VarProcType::divext;
				var.selTail = true;
				var.selPath = false;
				break;
			case 'H' :			// �p�X�����̂ݎc���i�Ō�̃p�X��؂���܂ށj
				var.typeProc = VarProcType::path;
				var.selHead = true;
				var.withDelim = true;
				break;
			case 'M' :
				var.typeProc = VarProcType::match;
				break;
			case 'C' :			// �g���q�o�������o��
				var.typeProc = VarProcType::count;
				break;
			case 'L' :			// �����񒷂��o��
				var.typeProc = VarProcType::len;
				break;
			default :
				var.typeProc = VarProcType::none;
				success = false;
				break;
		}
	}
	else{
		if ( strCmd == "<>" ){		// �X�y�[�X������O��ŏ���
			var.typeProc = VarProcType::trim;
			var.selHead = true;
			var.selTail = true;
		}
		else if ( strCmd == "Xi" ){		// �g���q�����ꂩ������u��
			var.typeProc = VarProcType::exchg;
			var.selInStr = true;
		}
		else if ( strCmd == "Xb" ){		//  �X�y�[�X������S������
			var.typeProc = VarProcType::blank;
		}
		else if ( strCmd == "Xq" ){		// �N�H�[�g��u���ޔ�
			var.typeProc = VarProcType::exchg;
			var.selQuote = true;
			var.selBackup = true;
		}
		else if ( strCmd == "Xqr" ){	// �u���ޔ������ɖ߂�
			var.typeProc = VarProcType::exchg;
			var.selQuote = true;
			var.selBackup = false;
		}
		else if ( strCmd == "Ci" ){		// �g���q�����ꂩ�����̏o�������o��
			var.typeProc = VarProcType::count;
			var.selInStr = true;
		}
		else if ( strCmd == "Cm" ){		// ���K�\���Ŋg���q�o�������o��
			var.typeProc = VarProcType::count;
			var.selRegEx = true;
		}
		else{
			success = false;
		}
	}
	if ( strCmd.length() > 0 ){
		if ( strCmd[0] == '~' ){		// ������������c��
			var.typeProc = VarProcType::substr;
			success = true;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// ���X�g�ϐ��̗v�f���C���B�v�f���������͗v�f���̉��Z�����Đ����l�ɂ���
// ���́F
//   strNamePart : �ϐ����i[�ԍ�]���݁j
//   exact       : true=�ϐ����S��  false=�擪����ϐ����ƈ�v����
// �o�́F
//   �Ԃ�l  : true=����  false=���s
//   strNamePart : �ϐ����i[�ԍ�]�͉��Z�ς݂̐��l�j
//   lenFullVar  : [�ԍ�]���݂̕ϐ������񒷁i�ύX�O�̕ϐ����Łj
//---------------------------------------------------------------------
bool JlsScrFuncReg::fixJlsRegNameAtList(string& strNamePart, int& lenFullVar, bool exact){
	//--- �ϐ���[���ڔԍ�] �Ȃ烊�X�g�ϐ��̗v�f ---
	lenFullVar = (int)strNamePart.length();
	auto locSt = strNamePart.find("[");
	if ( locSt == string::npos ){	// �z�񂪂Ȃ���ΐ���I��
		return true;
	}else{
		auto locCt = strNamePart.find("$");
		if ( locCt != string::npos && locCt < locSt ){	// [�̑O��$������΍���̕ϐ��ł͂Ȃ��̂ŏ���
			if ( exact ){
				pGlobalState->addMsgErrorN("error: bad $ in " + strNamePart);
				return false;
			}else{
				lenFullVar = (int)locCt;
				strNamePart = strNamePart.substr(0, locCt);
				return true;
			}
		}
	}
	string strNew = strNamePart.substr(0, locSt);
	bool success = true;
	bool cont = true;
	while( cont ){		// �������Ή����[�v
		int dup = 0;
		auto locEd = locSt + 1;
		while( (strNamePart[locEd] != ']' || dup > 0) && strNamePart[locEd] != '\0' ){
			if ( strNamePart[locEd] == ']' ){
				dup --;
			}else if ( strNamePart[locEd] == '[' ){
				dup ++;
			}
			locEd ++;
		}
		success = false;
		cont = false;
		if ( strNamePart[locEd] == ']' && locSt+1 < locEd ){	// []���̕�������擾���A���̒��̕ϐ��u�������{
			string strAryBuf = strNamePart.substr(locSt + 1, locEd - locSt - 1);
			string strItem;
			if ( replaceBufVar(strItem, strAryBuf) ){
				//--- ���ڔԍ��̎擾 ---
				int numItem;
				if ( pdata->cnv.getStrValNum(numItem, strItem, 0) >= 0 ){
					success = true;
					strNew += "[";
					strNew += to_string(numItem);
					strNew += "]";
					lenFullVar = (int) locEd + 1;
					if ( strNamePart[locEd+1] == '[' ){		// �������Ή�
						locSt = locEd + 1;
						cont = true;
					}
				}
			}
		}
	}
	if ( success ){
		if ( (int)strNamePart.length() != lenFullVar && exact ){
			pGlobalState->addMsgErrorN("warning: unrecognized charactor in " + strNamePart);
		}
		strNamePart = strNew;
		return true;
	}
	pGlobalState->addMsgErrorN("error: [value] must be integer in " + strNamePart);
	return false;
}
//--- �Ώە���������ׂĒu�� ---
void JlsScrFuncReg::replaceStrAllFind(string& strVal, const string& strFrom, const string& strTo){
	if ( !strVal.empty() && !strFrom.empty() ){
		auto n = strVal.find(strFrom);
		while( n != string::npos ){
			strVal = strVal.replace(n, strFrom.length(), strTo);
			n = strVal.find(strFrom, n+strTo.length());
		}
	}
}
//--- ���p���𐧌䖳�֌W�����ɒu���ޔ� ---
void JlsScrFuncReg::backupStrQuote(string& strVal){
	string strFrom = "\"";
	string strTo;
	pGlobalState->getRegVarCommon(strTo, DefRegDQuote, true);
	replaceStrAllFind(strVal, strFrom, strTo);
	strFrom = "\'";
	pGlobalState->getRegVarCommon(strTo, DefRegSQuote, true);
	replaceStrAllFind(strVal, strFrom, strTo);
}
//--- �ޔ����Ă�����p������������ɖ߂� ---
void JlsScrFuncReg::restoreStrQuote(string& strVal){
	string strFrom;
	pGlobalState->getRegVarCommon(strFrom, DefRegDQuote, true);
	string strTo   = "\"";
	replaceStrAllFind(strVal, strFrom, strTo);
	pGlobalState->getRegVarCommon(strFrom, DefRegSQuote, true);
	strTo   = "\'";
	replaceStrAllFind(strVal, strFrom, strTo);
}

//=====================================================================
// �ϐ��������̒u��
//=====================================================================

//---------------------------------------------------------------------
// �ϐ�������u������������o��
// ���́F
//   strBuf : ������
// �o�́F
//   �Ԃ�l  �F�u�����ʁitrue=����  false=���s�j
//   dstBuf  : �o�͕�����
//---------------------------------------------------------------------
bool JlsScrFuncReg::replaceBufVar(string& dstBuf, const string& srcBuf){
	dstBuf.clear();
	bool success = true;
	int pos_cmt = pdata->cnv.getStrPosComment(srcBuf, 0);
	int pos_base = 0;
	while(pos_base >= 0){
		//--- �ϐ������̒u�� ---
		int pos_var = pdata->cnv.getStrPosReplaceVar(srcBuf, pos_base);	// $�ʒu
		if (pos_var >= 0){
			//--- �R�����g�̈�m�F ---
			bool bakCmt = pGlobalState->isStopAddMsgError();
			bool flagCmt = ( pos_var >= pos_cmt && pos_cmt >= 0 )? true : bakCmt;
			if ( flagCmt ){
				pGlobalState->stopAddMsgError(true);	// �G���[�o�͒�~
			}
			//--- $��O�܂ł̕�������m�� ---
			if (pos_var > pos_base){
				dstBuf += srcBuf.substr(pos_base, pos_var-pos_base);
				pos_base = pos_var;
			}
			//--- �ϐ����������Ēu�� ---
			string strVal;
			int len_var = replaceRegVarInBuf(strVal, srcBuf, pos_var);
			if (len_var > 0){
				string strRev;
				success = replaceBufVar(strRev, strVal);	// �����Č����i���W�J�ϐ��Ή��j
				dstBuf += strRev;
				pos_base += len_var;
			}
			else{
				if ( flagCmt == false ){		// �R�����g�łȂ���Βu�����s
					success = false;
					pGlobalState->addMsgErrorN("warning: not defined variable in " + srcBuf);
				}
				pos_var = -1;
			}
			if ( flagCmt ){
				pGlobalState->stopAddMsgError(bakCmt);	// �G���[�o�͏�ԕ��A
			}
		}
		//--- �ϐ����Ȃ���Ύc�肷�ׂăR�s�[ ---
		if (pos_var < 0){
			dstBuf += srcBuf.substr(pos_base);
			pos_base = -1;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// �Ώۈʒu�̕ϐ���ǂݏo��
// ���́F
//   strBuf : ������
//   pos    : �F���J�n�ʒu
// �o�́F
//   �Ԃ�l  �F�ϐ������̕�����
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsScrFuncReg::replaceRegVarInBuf(string& strVal, const string& strBuf, int pos){
	int var_st, var_ed;
	bool exact;
	bool flagNum = false;
	bool flagConv = false;

	int ret = 0;
	if (strBuf[pos] == '$'){
		flagConv = true;
		//--- �ϐ��������擾 ---
		pos ++;
		if ( strBuf[pos] == '#' ){		// $#�̓��X�g�v�f��
			flagNum = true;
			pos ++;
		}
		if (strBuf[pos] == '{'){		// ${�ϐ���}�t�H�[�}�b�g���̏���
			exact = true;
			pos ++;
			int dup = 0;
			var_st = pos;
			while( (strBuf[pos] != '}' || dup > 0) && strBuf[pos] != '\0' ){
				if ( strBuf[pos] == '}' ){
					dup --;
				}else if ( strBuf[pos] == '{' ){
					dup ++;
				}
				pos ++;
			}
			if ( strBuf[pos] == '\0' ){
				flagConv = false;
			}
		}
		else{							// $�ϐ����t�H�[�}�b�g���̏���
			exact = false;
			var_st = pos;
			while(strBuf[pos] != ' ' && strBuf[pos] != '\0'){
				pos ++;
			}
		}
		var_ed = pos;
		if (strBuf[pos] == '}' || strBuf[pos] == ' '){
			var_ed -= 1;
		}
		//--- �ϐ��ǂݏo�����s ---
		if ( var_st <= var_ed && flagConv ){
			string strCandName = strBuf.substr(var_st, var_ed-var_st+1);
			if ( flagNum ){			// ���X�g�v�f��
				strCandName = "#" + strCandName;
			}
			int nmatch = getJlsRegVarPartName(strVal, strCandName, exact);
			if (nmatch > 0){
				ret = nmatch + 1;	// �ϐ����� + $
				if ( flagNum ){
					ret += 1;		// #
				}
				if ( exact ){
					ret += 2;		// {}
					if ( ret < var_ed-var_st+2 ){
						// �G���[�ł��邪�ُ�̓G���[�o���Ȃ��Ă��킩��̂ł��̂܂�
					}
				}
			}
		}
	}
	return ret;
}


//=====================================================================
// �V�X�e���ϐ��ݒ�
//=====================================================================

//---------------------------------------------------------------------
// �����ݒ�ϐ�
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegInit(){
	setSystemRegHeadtail(-1, -1);
	setSystemRegUpdate();
	//--- ����������warning���o���O���[�o���ϐ��ݒ�̉��� ---
	pGlobalState->setGlobalLock(DefRegExtChar, false);
	pGlobalState->setGlobalLock(DefRegDQuote, false);
	pGlobalState->setGlobalLock(DefRegSQuote, false);
	//--- �Œ�O���[�o���ϐ� ---
	setJlsRegVar(DefRegExtChar, ".", true);
	setJlsRegVar(DefRegExtCsub, ".", true);
	setJlsRegVar(DefRegDQuote, DefStrRepDQ, true);
	setJlsRegVar(DefRegSQuote, DefStrRepSQ, true);
	//--- ����������warning���o���O���[�o���ϐ��ݒ� ---
	pGlobalState->setGlobalLock(DefRegExtChar, true);
	pGlobalState->setGlobalLock(DefRegDQuote, true);
	pGlobalState->setGlobalLock(DefRegSQuote, true);
}

//---------------------------------------------------------------------
// �����ݒ�ϐ��̌��ݒl�ɂ��ύX
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegUpdate(){
	int n = pdata->getMsecTotalMax();
	string str_val = pdata->cnv.getStringFrameMsecM1(n);
	string str_time = pdata->cnv.getStringTimeMsecM1(n);
	setJlsRegVar("MAXFRAME", str_val, true);
	setJlsRegVar("MAXTIME", str_time, true);
	setJlsRegVar("NOLOGO", to_string(pdata->extOpt.flagNoLogo), true);
	setJlsRegVar("RANGETYPE", to_string(pdata->recHold.typeRange), true);
	setSystemRegFilePath();
	setSystemRegFileOpen();
}
//---------------------------------------------------------------------
// Path�֘A�̌��ݒl�ɂ��ύX
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegFilePath(){
	string strDataLast = "data";		// ���X�g�f�[�^�ǂݍ��ݗpJL�T�u�p�X����̈ʒu

	//--- JL�T�u�p�X�ǂݍ��� ---
	string strPathSub;
	if ( pdata->extOpt.subPath.empty() == false ){
		strPathSub = pdata->extOpt.subPath;
	}else{
		strPathSub = pGlobalState->getPathNameJL();
	}
	pdata->cnv.getStrFileAllPath(strPathSub);	// �Ō�ɋ�؂�t��
	//--- JL�f�[�^�p�X ---
	string strPathData = strPathSub + strDataLast;
	pdata->cnv.getStrFileAllPath(strPathData);	// �Ō�ɋ�؂�t��
	//--- �����ݒ�ϐ��̕ύX ---
	setJlsRegVar("JLDATAPATH", strPathData, true);
}
//---------------------------------------------------------------------
// Echo�o�͐悪�t�@�C�������ʐݒ�
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegFileOpen(){
	string strVal = ( pGlobalState->fileIsOpen() )? "1" : "0";
	setJlsRegVar("FILEOPEN", strVal, true);
}

//---------------------------------------------------------------------
// HEADFRAME/TAILFRAME��ݒ�
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegHeadtail(int headframe, int tailframe){
	string str_head = pdata->cnv.getStringTimeMsecM1(headframe);
	string str_tail = pdata->cnv.getStringTimeMsecM1(tailframe);
	setJlsRegVar("HEADTIME", str_head, true);
	setJlsRegVar("TAILTIME", str_tail, true);
}

//---------------------------------------------------------------------
// �����ȃ��S�̊m�F�i���S���Ԃ��ɒ[�ɒZ�������烍�S�Ȃ������ɂ���j
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegNologo(bool need_check){
	bool flag_nologo = false;
	//--- ���S���Ԃ��ɒ[�ɏ��Ȃ��ꍇ�Ƀ��S����������ꍇ�̏��� ---
	if (need_check == true && pdata->extOpt.flagNoLogo == 0){
		int msec_sum = 0;
		int nrf_rise = -1;
		int nrf_fall = -1;
		do{
			nrf_rise = pdata->getNrfNextLogo(nrf_fall, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
			nrf_fall = pdata->getNrfNextLogo(nrf_rise, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
			if (nrf_rise >= 0 && nrf_fall >= 0){
				msec_sum += pdata->getMsecLogoNrf(nrf_fall) - pdata->getMsecLogoNrf(nrf_rise);
			}
		} while(nrf_rise >= 0 && nrf_fall >= 0);
		if (msec_sum < pdata->getConfig(ConfigVarType::msecWLogoSumMin)){
			flag_nologo = true;
		}
	}
	else{		// �`�F�b�N�Ȃ��Ń��S�����̏ꍇ
			flag_nologo = true;
	}
	if (flag_nologo == true){
		// ���S�ǂݍ��݂Ȃ��ɕύX
		pdata->extOpt.flagNoLogo = 1;
		// �V�X�e���ϐ����X�V
		setJlsRegVar("NOLOGO", "1", true);	// �㏑������"1"�ݒ�
	}
}

//---------------------------------------------------------------------
// Read�R�}���h���ʂ̗L���L����ݒ�
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegReadValid(bool valid){
	string strVal = ( valid )? "1" : "0";
	setJlsRegVar("READVALID", strVal, true);
}
//---------------------------------------------------------------------
// �O��̎��s��Ԃ�ݒ�
//---------------------------------------------------------------------
void JlsScrFuncReg::setSystemRegLastexe(bool exe_command){
	//--- �O��̎��s��Ԃ�ϐ��ɐݒ� ---
	setJlsRegVar("LASTEXE", to_string((int)exe_command), true);
}
//--- ���O�̎��s��Ԏ擾 ---
bool JlsScrFuncReg::isSystemRegLastexe(){
	string strVal = "0";
	getJlsRegVarNormal(strVal, "LASTEXE");
	bool lastExe = ( strVal != "0" )? true : false;
	return lastExe;
}


//---------------------------------------------------------------------
// CM�J�b�g�ʒu�𒼐ڏo��
//---------------------------------------------------------------------
void JlsScrFuncReg::setOutDirect(){
	string strList;
	getJlsRegVarNormal(strList, "OUTDIRECT");
	if ( strList.empty() ){
		return;
	}
	int nsize = pFuncList->getListStrSize(strList);
	if ( nsize % 2 == 1 ){	// �K��2�f�[�^�P��
		nsize = nsize - 1;
	}
	vector<Msec> listMsec;
	for(int i=0; i<nsize; i++){
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strList, i+1) == false ){
			return;
		}
		int val;
		if ( pdata->cnv.getStrValMsec(val, strItem, 0) <= 0 ){
			return;
		}
		listMsec.push_back(val);
	}
	pdata->setOutDirect(listMsec);
	if ( !listMsec.empty() ){
		string mes = "OUTDIRECT is set for Trim.";
		pdata->dispSysMesN(mes, JlsDataset::SysMesType::OutDirect);
	}
}


//---------------------------------------------------------------------
// �X�N���v�g���ŋL�ڂ���N���I�v�V����
// ���́F
//   strBuf     �F�I�v�V�������܂ޕ�����
//   pos        �F�ǂݍ��݊J�n�ʒu
//   overwrite  �F�������ݍς݂̃I�v�V�����ݒ�ifalse=���Ȃ� true=����j
// �o�́F
//   �Ԃ�l  �Ftrue=����I�� false=�ݒ�G���[
//---------------------------------------------------------------------
bool JlsScrFuncReg::setSystemRegOptions(const string& strBuf, int pos, bool overwrite){
	//--- �������؂�F�� ---
	vector <string> listarg;
	string strWord;
	while(pos >= 0){
		pos = pdata->cnv.getStrItem(strWord, strBuf, pos);
		if (pos >= 0){
			listarg.push_back(strWord);
		}
	}
	int argc = (int) listarg.size();
	if (argc <= 0){
		return true;
	}
	//--- �X�N���v�g���Őݒ�\�ȃI�v�V���� ---
	int i = 0;
	while(i >= 0 && i < argc){
		int argrest = argc - i;
		const char* strv = listarg[i].c_str();
		const char* str1 = nullptr;
		const char* str2 = nullptr;
		if (argrest >= 2){
			str1 = listarg[i+1].c_str();
		}
		if (argrest >= 3){
			str2 = listarg[i+2].c_str();
		}
		int numarg = setOptionsGetOne(argrest, strv, str1, str2, overwrite);
		if (numarg < 0){
			return false;
		}
		if (numarg > 0){
			i += numarg;
		}
		else{		// ���s�\�R�}���h�łȂ���Ύ��Ɉڍs
			i ++;
		}
	}
	setSystemRegUpdate();	// Path���̍X�V�i�O�̂��ߏ����ݒ�ϐ��S�́j
	return true;
}


//---------------------------------------------------------------------
// ���擾
//---------------------------------------------------------------------
void JlsScrFuncReg::getSystemData(JlsCmdArg& cmdarg, const string& strIdent){
	string strData;
	if ( strIdent == "AUTOMODE" ){
		strData = ( pdata->isAutoModeUse() )? "1" : "0";
	}
	else if ( strIdent == "DETMODE" ){
		strData = ( pGlobalState->isExe1st() )? "0" : "1";
	}
	else if ( strIdent == "FCURRENT" ){
		strData = LSys.getCurrentPath();
	}
	else if ( strIdent == "FINLOGO" ){
		strData = pdata->extOpt.logofile;
	}
	else if ( strIdent == "FINSCP" ){
		strData = pdata->extOpt.scpfile;
	}
	else if ( strIdent == "FINCMD" ){
		strData = pdata->extOpt.cmdfile;
	}
	else if ( strIdent == "FOUTAVS" ){
		strData = pdata->extOpt.outfile;
	}
	else if ( strIdent == "FOUTSCP" ){
		strData = pdata->extOpt.outscpfile;
	}
	else if ( strIdent == "FOUTDIV" ){
		strData = pdata->extOpt.outdivfile;
	}
	else if ( strIdent == "SYSCODE" ){
		strData = to_string(pdata->extOpt.nSysCode);
	}
	else if ( strIdent == "FILECODE" ){
		strData = to_string(pGlobalState->fileGetCodeDefaultNum());
	}
	else if ( strIdent == "READCODE" ){
		strData = to_string(pGlobalState->readGCodeNum());
	}
	else if ( strIdent == "TRIMLIST" ){
		//--- ���ʍ쐬 ---
		pdata->outputResultTrimGen();
		pFuncList->setListStrClear(strData);
		int num_data = (int) pdata->resultTrim.size();
		for(int i=0; i<num_data-1; i+=2){
			int frm_st = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i] );
			int frm_ed = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i+1] );
			pFuncList->setListStrIns(strData, to_string(frm_st), -1);
			pFuncList->setListStrIns(strData, to_string(frm_ed), -1);
		}
	}
	else{
		pGlobalState->addMsgErrorN("error: No SysData name(" + strIdent + ")");
		return;
	}
	setRegOutSingle(cmdarg, strData, true);	// RegOut�ɖ������ŏ�������
}


//=====================================================================
// �R�}���h���ʂɂ��ϐ��X�V
//=====================================================================

//--- $POSHOLD/$LISTHOLD�̃��W�X�^�X�V ---
void JlsScrFuncReg::updateResultRegWrite(JlsCmdArg& cmdarg){
	//--- ���ʐݒ� ---
	bool   overwrite = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	//--- POSHOLD�̍X�V ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValPosW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegOut);	// �ϐ���($POSHOLD)
		string strVal    = cmdarg.getStrOpt(OptType::StrValPosW);
		setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// $POSHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// �X�V�����ʒm
	}
	//--- LISTHOLD�̍X�V ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValListW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegOut);	// �ϐ���($LISTHOLD)
		string strList   = cmdarg.getStrOpt(OptType::StrValListW);
		setJlsRegVarWithLocal(strName, strList, overwrite, flagLocal);	// $LISTHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValListW);	// �X�V�����ʒm
	}
}
//--- $SIZEHOLD�̃��W�X�^�ݒ� ---
void JlsScrFuncReg::setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList){
	string strSizeName = cmdarg.getStrOpt(OptType::StrRegSize);	// �ϐ���($SIZEHOLD)
	int    numList     = pFuncList->getListStrSize(strList);	// ���ڐ��擾
	string strNumList  = std::to_string(numList);
	bool   overwrite   = ( cmdarg.getOpt(OptType::FlagDefault) == 0 )? true : false;
	bool   flagLocal   = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	setJlsRegVarWithLocal(strSizeName, strNumList, overwrite, flagLocal);	// $SIZEHOLD
}
//--- $POSHOLD�̍X�V ---
void JlsScrFuncReg::setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos){
	string strVal  = pdata->cnv.getStringTimeMsecM1(msecPos);
	cmdarg.setStrOpt(OptType::StrValPosW, strVal);			// $POSHOLD write
	updateResultRegWrite(cmdarg);
}

//--- $LISTHOLD�̍X�V ---
void JlsScrFuncReg::setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write�ϐ��l

	//--- ���ڒǉ� ---
	if ( msecPos != -1 ){
		if ( pFuncList->isListStrEmpty(strList) ){
			strList.clear();
		}else{
			strList += ",";
		}
		string strVal = pdata->cnv.getStringTimeMsecM1(msecPos);
		strList += strVal;
	}
	//--- ListHold�ɐݒ� ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListGetAt�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosW);
	//--- �Ώۈʒu�̍��ڂ��擾 ---
	{
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strList, numItem) ){
			strValPos = strItem;
		}
	}
	//--- PosHold�ɐݒ� ---
	cmdarg.setStrOpt(OptType::StrValPosW, strValPos);		// $POSHOLD write
	updateResultRegWrite(cmdarg);
	//--- ���X�g���X�V ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListIns�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListIns(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){		// -RegArg
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		if ( !getJlsRegVarNormal(strValPos, strName) ){
			pGlobalState->addMsgErrorN("error: -RegArg " + strName + " at ListIns");
		}
	}
	//--- Ins���� ---
	if ( pFuncList->setListStrIns(strList, strValPos, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHold�ɐݒ�
	}
}
//--- ListDel�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListDel(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- Del���� ---
	if ( pFuncList->setListStrDel(strList, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHold�ɐݒ�
	}
}
//--- ListJoin�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListJoin(JlsCmdArg& cmdarg){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	int numItem = -1;	// ���X�g�Ō�ɒǉ�
	//--- ���X�g�l�f�[�^���擾 ---
	string strListIns;
	if ( !setResultRegSubGetRegVal(cmdarg, strListIns, true) ){	// must=true
		return;
	}
	//--- Ins���� ---
	int numCur = numItem;
	int nsizeIns = pFuncList->getListStrSize(strListIns);
	for(int i=1; i<=nsizeIns; i++){
		string strItem;
		if ( pFuncList->getListStrElement(strItem, strListIns, i) ){
			if ( pFuncList->setListStrIns(strList, strItem, numCur) ){
				if ( numItem > 0 ) numCur++;	// �O���琔���鎞�͎��̑}���ʒu��ύX
			}
		}
	}
	if ( cmdarg.getOptFlag(OptType::FlagSort) ){	// -sort
		sortResultRegList(cmdarg, strList);
	}
	//--- ListHold�ɐݒ� ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListRemove�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListRemove(JlsCmdArg& cmdarg){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	bool   flagLap   = cmdarg.getOptFlag(OptType::FlagOverlap);
	//--- ���X�g�l�f�[�^���擾 ---
	string strListRm;
	if ( !setResultRegSubGetRegVal(cmdarg, strListRm, true) ){	// must=true
		return;
	}
	//--- Remove���� ---
	bool success = true;
	if ( flagLap ){
		success = pFuncList->setListStrRemoveLap(strList, strListRm);
	}else{
		success = pFuncList->setListStrRemove(strList, strListRm);
	}
	//--- ListHold�ɐݒ� ---
	if ( success ){
		writeResultRegListW(cmdarg, strList);	// ListHold�ɐݒ�
	}else{
		pGlobalState->addMsgErrorN("error: list data at ListRemove");
	}
}
//--- �������X�g�l���擾 ---
bool JlsScrFuncReg::setResultRegSubGetRegVal(JlsCmdArg& cmdarg, string& strListSub, bool must){
	bool success = false;
	string mesErr;
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		success = getJlsRegVarNormal(strListSub, strName);
		mesErr = "not found data by -RegArg " + strName + " ";
	}
	else if ( cmdarg.isSetStrOpt(OptType::StrArgVal) ){
		strListSub = cmdarg.getStrOpt(OptType::StrArgVal);
		success = true;
	}
	else if ( must ){
		mesErr = "need arg option for the list join/remove command";
	}
	if ( !success && !mesErr.empty() ){
		pGlobalState->addMsgErrorN("error: " + mesErr);
	}
	return success;
}
//--- ListSel�ɂ���荞�� ---
void JlsScrFuncReg::setResultRegListSel(JlsCmdArg& cmdarg, string strListNum){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- List�I������ ---
	if ( pFuncList->setListStrSel(strList, strListNum) ){
		writeResultRegListW(cmdarg, strList);	// ListHold�ɐݒ�
	}
}
//--- ListSetAt�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListRep(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	//--- Replace���� ---
	if ( pFuncList->setListStrRep(strList, strValPos, numItem) ){
		writeResultRegListW(cmdarg, strList);	// ListHold�ɐݒ�
	}
}
//--- ListClear�ɂ�郊�X�g�ϐ����e������ ---
void JlsScrFuncReg::setResultRegListClear(JlsCmdArg& cmdarg){
	//--- ���X�g�ϐ��̏����l ---
	string strList;
	pFuncList->setListStrClear(strList);
	//--- ListHold�ɐݒ� ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListDim�ɂ�郊�X�g�ϐ����ڊm�� ---
void JlsScrFuncReg::setResultRegListDim(JlsCmdArg& cmdarg, int num){
	string strVal;
	setResultRegSubGetRegVal(cmdarg, strVal, false);	// must=false
	string strList;
	pFuncList->setListStrDim(strList, num, strVal);
	//--- ListHold�ɐݒ� ---
	writeResultRegListW(cmdarg, strList);
}
//--- ListSort�ɂ��X�V ---
void JlsScrFuncReg::setResultRegListSort(JlsCmdArg& cmdarg){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	sortResultRegList(cmdarg, strList);
	//--- ListHold�ɐݒ� ---
	writeResultRegListW(cmdarg, strList);
}
void JlsScrFuncReg::sortResultRegList(JlsCmdArg& cmdarg, string& strList){
	bool   flagUni   = ( cmdarg.getOpt(OptType::FlagUnique)>0 )? true : false;
	bool   flagLap   = ( cmdarg.getOpt(OptType::FlagOverlap) > 0 );
	bool   flagMerge = ( cmdarg.getOpt(OptType::FlagMerge) > 0 );
	//--- sort���� ---
	if ( flagLap ){		// -overlap
		pFuncList->setListStrSortLap(strList, flagMerge);
	}else{
		pFuncList->setListStrSort(strList, flagUni);
	}
}
//--- ���X�g�ϐ��o�͍X�V ---
void JlsScrFuncReg::writeResultRegListW(JlsCmdArg& cmdarg, const string& strList){
	//--- ListHold�ɐݒ� ---
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- ���X�g���X�V ---
	setResultRegWriteSize(cmdarg, strList);
}

//--- ���ڕ�����w���CSV�`������������X�g������ɕ��� ---
string JlsScrFuncReg::getStrRegListByCsvStr(const string& strBuf){
	//--- ���X�g�ɕ��� ---
	string strList;
	pFuncList->setListStrClear(strList);
	int pos = 0;
	while( pos >= 0 ){
		string strItem;
		pos = pdata->cnv.getStrCsv(strItem, strBuf, pos);
		if ( pos >= 0 ){
			pFuncList->setListStrIns(strList, strItem, -1);
		}
	}
	return strList;
}
//--- CSV�`������������X�g�ɕ������Ċi�[ ---
void JlsScrFuncReg::setStrRegListByCsv(JlsCmdArg& cmdarg){
	string strArg;
	if ( getRegArg(cmdarg, strArg) ){
		string strList = getStrRegListByCsvStr(strArg);
		setRegOutSingle(cmdarg, strList, true);		// �������ŏ�������
	}
}
//--- �X�y�[�X��؂�̕���������X�g�ɕ������Ċi�[ ---
void JlsScrFuncReg::setStrRegListBySpc(JlsCmdArg& cmdarg){
	string strArg;
	if ( getRegArg(cmdarg, strArg) ){
		setStrRegListCommon(cmdarg, strArg, 0);
	}
}
//--- ����������X�g�ɕ������Ċi�[�̋��ʏ��� ---
void JlsScrFuncReg::setStrRegListCommon(JlsCmdArg& cmdarg, const string& strBuf, int readtype){
	//--- ���X�g�ɕ��� ---
	string strList;
	pFuncList->setListStrClear(strList);
	int pos = 0;
	while( pos >= 0 ){
		string strItem;
		if ( readtype == 1 ){
			pos = pdata->cnv.getStrCsv(strItem, strBuf, pos);
		}else{
			pos = pdata->cnv.getStrItem(strItem, strBuf, pos);
		}
		if ( pos >= 0 ){
			pFuncList->setListStrIns(strList, strItem, -1);
		}
	}
	setRegOutSingle(cmdarg, strList, true);		// �������ŏ�������
}

//=====================================================================
// �f�[�^�p�t�@�C���ǂݍ���
//=====================================================================

//---------------------------------------------------------------------
// �t�@�C�������݂��邩���X�g�f�[�^�ɐݒ�
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataCheck(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::Check;	// �t�@�C���L�����m�F
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// ���X�g�f�[�^�i���l�j���t�@�C������ǂݍ��݁i�P�s�P�f�[�^�j
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataList(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::List;	// ���X�g���琔�l��ǂݍ���
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// ���X�g�f�[�^��AVS�t�@�C����Trim����ǂݍ���
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataTrim(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::Trim;	// Trim����̐��l��ǂݍ���
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// ���X�g�f�[�^�i������j���t�@�C������ǂݍ��݁i�P�s�P�f�[�^�j
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataString(JlsCmdArg& cmdarg, const string& fname){
	ReadFileType rtype = ReadFileType::String;	// �������ǂݍ���
	return readDataCommon(cmdarg, fname, rtype);
}
//---------------------------------------------------------------------
// ���X�g�f�[�^���t�@�C������ǂݍ��݂̋��ʏ���
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataCommon(JlsCmdArg& cmdarg, const string& fname, ReadFileType rtype){
	//--- ���X�g�f�[�^�N���A ---
	updateResultRegWrite(cmdarg);	// �ϐ�($LISTHOLD)�N���A
	//--- �f�[�^������ݒ� ---
	int nMax = -1;
	if ( cmdarg.isSetOpt(OptType::NumMaxSize) ){
		nMax = cmdarg.getOpt(OptType::NumMaxSize);
	}
	//--- �t�@�C���ǂݍ��� ---
	LocalIfs ifs(fname.c_str());
	//--- ���݃`�F�b�N ---
	if ( rtype == ReadFileType::Check ){
		bool exist = ifs.is_open();
		if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
			string strVal = ( exist )? "1" : "0";
			setRegOutSingle(cmdarg, strVal, true);	// RegOut�ɖ������ŏ�������
		}
		return exist;
	}
	if ( !ifs.is_open() ){
		if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
			pGlobalState->fileOutput("warning: file not found(" + fname + ")\n");
		}
		return false;
	}
	//--- �f�[�^�ǂݍ��� ---
	int nCur = 0;
	if ( rtype == ReadFileType::Trim ){
		string strCmd;
		if ( readDataFileTrim(strCmd, ifs) ){	// Trim�s�ǂݍ���
			readDataCommonIns(cmdarg, nCur, strCmd, nMax, rtype);
		}
	}else{
		bool cont = true;
		while( cont ){
			string strLine;
			cont = readDataFileLine(strLine, ifs);	// �P�s�ǂݍ���
			if ( cont ){
				cont = readDataCommonIns(cmdarg, nCur, strLine, nMax, rtype);
			}
		}
	}
	return ( nCur > 0 );
}
//--- 1�s���f�[�^���i�[ ---
bool JlsScrFuncReg::readDataCommonIns(JlsCmdArg& cmdarg, int& nCur, const string& strLine, int nMax, ReadFileType rtype){
	//--- �f�[�^������̊m�F ---
	if ( nCur >= nMax && nMax >= 0 ){
		return false;
	}
	//--- ��ޕʂ̑}������ ---
	switch( rtype ){
		case ReadFileType::Trim :
			{
				string strRest = strLine;
				bool cont = true;
				while( cont ){			// Trim�̐������p��
					string strLocSt;
					string strLocEd;
					if ( nCur+1 >= nMax && nMax >= 0 ){		// �f�[�^�����
						cont = false;
					}else{
						cont = readDataStrTrimGet(strLocSt, strLocEd, strRest);
					}
					if ( cont ){
						nCur += 2;
						readDataStrAdd(cmdarg, strLocSt);
						readDataStrAdd(cmdarg, strLocEd);
					}
				}
			}
			break;
		case ReadFileType::List :
			{
				string sdata;
				if ( pdata->cnv.getStrItem(sdata, strLine, 0) > 0 ){	// 1���ڎ擾
					int val;
					if ( pdata->cnv.getStrValNum(val, sdata, 0) > 0 ){	// ���ڂ����l
						nCur ++;
						readDataStrAdd(cmdarg, sdata);
					}
				}
			}
			break;
		default :
			nCur ++;
			readDataStrAdd(cmdarg, strLine);
			break;
	}
	//--- �f�[�^������̊m�F ---
	if ( nCur >= nMax && nMax >= 0 ){
		return false;
	}
	return true;
}
//---------------------------------------------------------------------
// ���X�g�ɂP���ڒǉ�
//---------------------------------------------------------------------
void JlsScrFuncReg::readDataStrAdd(JlsCmdArg& cmdarg, const string& sdata){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write�ϐ��l
	cmdarg.setStrOpt(OptType::StrValListR, strList);
	cmdarg.setStrOpt(OptType::StrValPosR, sdata);
	setResultRegListIns(cmdarg, -1);
}
//---------------------------------------------------------------------
// Trim������̊J�n�E�I���ʒu���擾���ĕ�����͎��̈ʒu�Ɉڍs
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataStrTrimGet(string& strLocSt, string& strLocEd, string& strTarget){
	std::regex re(DefRegExpTrim, std::regex_constants::icase);
	std::smatch m;
	bool success = std::regex_search(strTarget, m, re);
	if ( success ){
		strLocSt = m[1].str();
		strLocEd = m[2].str();
		strTarget = m.suffix();
	}
	return success;
}
//---------------------------------------------------------------------
// Trim������̑��݊m�F
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataStrTrimDetect(const string& strLine){
	string strTarget = strLine;
	string strLocSt;
	string strLocEd;
	return readDataStrTrimGet(strLocSt, strLocEd, strTarget);
}

//---------------------------------------------------------------------
// �f�[�^�P�s�ǂݍ���
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataFileLine(string& strLine, LocalIfs& ifs){
	return pGlobalState->readLineIfs(strLine, ifs);
}
//---------------------------------------------------------------------
// AVS�t�@�C��Trim�s�ǂݍ���
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataFileTrim(string& strCmd, LocalIfs& ifs){
	strCmd = "";
	bool flagTrim = false;
	bool flagKeepNext = false;
	bool cont = true;
	while( cont ){
		string strLine;
		cont = readDataFileLine(strLine, ifs);	// �P�s�ǂݍ���
		if ( cont ){
			int len = (int) strLine.length();
			//--- �p���m�F ---
			bool flagKeepCur = flagKeepNext;
			flagKeepNext = false;
			if ( len >= 1 ){
				if ( strLine.substr(0,1) == R"(\)" ){	// �ŏ��Ɍp������
					flagKeepCur = true;
					strLine = strLine.substr(1);
					len = (int) strLine.length();
				}
			}
			if ( len >= 1 ){
				if ( strLine.substr(len-1) == R"(\)" ){	// �Ō�Ɍp������
					flagKeepNext = true;
					strLine = strLine.substr(0, len-1);
					len = (int) strLine.length();
				}
			}
			//--- �p�����͖������ɒǉ� ---
			if ( flagKeepCur ){
				strCmd += strLine;
			}else{
				//--- ��荞�ݍς݂̎��͏I�� ---
				if ( flagTrim ){
					cont = false;
				}else{
					strCmd = strLine;
				}
			}
		}
		//--- Trim������m�F ---
		if ( cont ){
			if ( flagTrim == false ){
				flagTrim = readDataStrTrimDetect(strCmd);
			}
		}
	}
	return flagTrim;
}
//---------------------------------------------------------------------
// �O���[�o���̈�쐬�̃t�@�C���ǂݍ���
//---------------------------------------------------------------------
bool JlsScrFuncReg::readGlobalOpen(JlsCmdArg& cmdarg, const string& fname){
	bool success = pGlobalState->readGOpen(fname);
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		string strVal = ( success )? "1" : "0";
		return setRegOutSingle(cmdarg, strVal, true);	// RegOut�ɖ������ŏ�������
	}
	return success;
}
void JlsScrFuncReg::readGlobalClose(JlsCmdArg& cmdarg){
	pGlobalState->readGClose();
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		string strVal = "1";
		setRegOutSingle(cmdarg, strVal, true);	// RegOut�ɖ������ŏ�������
	}
}
bool JlsScrFuncReg::readGlobalLine(JlsCmdArg& cmdarg){
	string strLine;
	bool success = pGlobalState->readGLine(strLine);
	if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){
		setRegOutSingle(cmdarg, strLine, true);	// RegOut�ɖ������ŏ�������
	}
	return success;
}
//---------------------------------------------------------------------
// -RegArg�Ŏw�肳�ꂽ��������擾
//---------------------------------------------------------------------
bool JlsScrFuncReg::getRegArg(JlsCmdArg& cmdarg, string& strArg){
	//--- �������镶������擾 ---
	if ( cmdarg.isSetStrOpt(OptType::StrRegArg) ){
		string strName;
		strName = cmdarg.getStrOpt(OptType::StrRegArg);
		if ( !getJlsRegVarNormal(strArg, strName) ){
			pGlobalState->addMsgErrorN("error: not found data by -RegArg " + strName);
			return false;
		}
	}else{
		pGlobalState->addMsgErrorN("error: need -RegArg option");
		return false;
	}
	return true;
}
//---------------------------------------------------------------------
// RegOut�ɒP�̃f�[�^��������
//---------------------------------------------------------------------
bool JlsScrFuncReg::setRegOutSingle(JlsCmdArg& cmdarg, const string& strVal, bool certain){
	//--- �I�v�V�����擾 ---
	string strName   =  cmdarg.getStrOpt(OptType::StrRegOut);	// �ϐ���
	bool   overwrite = !cmdarg.getOptFlag(OptType::FlagDefault);
	bool   flagLocal =  cmdarg.getOptFlag(OptType::FlagLocal);
	bool   flagClear =  cmdarg.getOptFlag(OptType::FlagClear);
	//--- �ϐ��ݒ� ---
	if ( certain || flagClear ){
		bool flagW = setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// �ϐ��ݒ�
		if ( !flagW ) return false;	// �������ݎ��s
	}
	return certain;
}

//---------------------------------------------------------------------
// ���ϐ�����f�[�^�擾
//---------------------------------------------------------------------
bool JlsScrFuncReg::readDataEnvGet(JlsCmdArg& cmdarg, const string& strEnvName){
	//--- �l���擾���ĕϐ��ɐݒ� ---
	string strVal;
	bool success = LSys.getEnvString(strVal, strEnvName);
	//--- �ϐ��ݒ� ---
	success = setRegOutSingle(cmdarg, strVal, success);
	return success;
}
//---------------------------------------------------------------------
// �G���[���ڏo��
//---------------------------------------------------------------------
void JlsScrFuncReg::outputMesErr(const string& mes){
	lcerr << mes;
}
