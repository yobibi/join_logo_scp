//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScript.hpp"
#include "JlsScriptState.hpp"
#include "JlsScriptDecode.hpp"
#include "JlsScriptLimit.hpp"
#include "JlsReformData.hpp"
#include "JlsAutoScript.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"


///////////////////////////////////////////////////////////////////////
//
// JL�X�N���v�g���s�N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ������
//---------------------------------------------------------------------
JlsScript::JlsScript(JlsDataset *pdata){
	this->pdata  = pdata;

	// Decoder/Limiter�ݒ�
	m_funcDecode.reset(new JlsScriptDecode(pdata));
	m_funcLimit.reset(new JlsScriptLimit(pdata));
	// Auto�n�R�}���h���g���g�p
	m_funcAutoScript.reset(new JlsAutoScript(pdata));

	// �O�̂��ߓ����ݒ�ُ�̊m�F
	checkInitial();
}

JlsScript::~JlsScript() = default;

//---------------------------------------------------------------------
// �����ݒ�ُ̈�m�F
//---------------------------------------------------------------------
void JlsScript::checkInitial(){
	m_funcDecode->checkInitial();
}


//=====================================================================
// ���W�X�^�A�N�Z�X����
//=====================================================================

//---------------------------------------------------------------------
// �ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
//--- �ʏ�̕ϐ���ݒ� ---
bool JlsScript::setJlsRegVar(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = false;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- ���[�J���ϐ���ݒ�i�����͒ʏ�ϐ��Ɠ���j ---
bool JlsScript::setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite){
	bool flagLocal = true;
	return setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
}
//--- �ʏ�̕ϐ��ƃ��[�J���ϐ���I�����Đݒ� ---
bool JlsScript::setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal){
	if ( strName.empty() ) return false;
	bool success = globalState.setRegVarCommon(strName, strVal, overwrite, flagLocal);
	setJlsRegVarCouple(strName, strVal);
	return success;
}
//--- �ϐ��ݒ��̃V�X�e���ϐ��X�V ---
void JlsScript::setJlsRegVarCouple(const string& strName, const string& strVal){
	//--- �V�X�e���ϐ��̓��ꏈ�� ---
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
			globalState.setRegVarCommon(strAddName, strAddVal, overwrite, flagLocal);
		}
		//--- head/tail�����X�V ---
		{
			string strSub;
			if (getJlsRegVar(strSub, "HEADTIME", true) > 0){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.st, strSub, 0);
			}
			if (getJlsRegVar(strSub, "TAILTIME", true) > 0){
				pdata->cnv.getStrValMsecM1(pdata->recHold.rmsecHeadTail.ed, strSub, 0);
			}
		}
	}
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
int JlsScript::getJlsRegVar(string& strVal, const string& strCandName, bool exact){
	//--- �ʏ�̃��W�X�^�ǂݏo�� ---
	return globalState.getRegVarCommon(strVal, strCandName, exact);
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
int JlsScript::setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite){
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
				fprintf(stderr, "-flags needs an argument\n");
				return -1;
			}
			else{
				if (setInputFlags(str1, overwrite) == false){
					fprintf(stderr, "-flags bad argument\n");
					return -1;
				}
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-set")){
			if (!exist3){
				fprintf(stderr, "-set needs two arguments\n");
				return -1;
			}
			else{
				if (setInputReg(str1, str2, overwrite) == false){
					fprintf(stderr, "-set bad argument\n");
					return -1;
				}
			}
			numarg = 3;
		}
		else if (!_stricmp(strv, "-cutmrgin")){
			if (!exist2){
				fprintf(stderr, "-cutmrgin needs an argument\n");
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
				fprintf(stderr, "-cutmrgout needs an argument\n");
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
				fprintf(stderr, "-cutmrgwi needs an argument\n");
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
				fprintf(stderr, "-cutmrgwo needs an argument\n");
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
				fprintf(stderr, "-sublist needs an argument\n");
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
				fprintf(stderr, "-subpath needs an argument\n");
				return -1;
			}
			else if (overwrite || pdata->extOpt.fixSubPath == 0){
				pdata->extOpt.subPath = str1;
				pdata->extOpt.fixSubPath = 1;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-vline")){	// debug
			if (!exist2){
				fprintf(stderr, "-vline needs an argument\n");
				return -1;
			}
			if (overwrite || pdata->extOpt.fixVLine == 0){
				int val = atoi(str1);
				pdata->extOpt.vLine = val;
				pdata->extOpt.fixVLine = 1;
			}
			numarg = 2;
		}
	}
	return numarg;
}

//---------------------------------------------------------------------
// CutMrgIn / CutMrgOut �I�v�V���������p 30fps�t���[�������͂Ń~���b��Ԃ�
//---------------------------------------------------------------------
Msec JlsScript::setOptionsCnvCutMrg(const char* str){
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
bool JlsScript::setInputReg(const char *name, const char *val, bool overwrite){
	return setJlsRegVar(name, val, overwrite);
}

//---------------------------------------------------------------------
// �I�v�V�����t���O��ݒ�
// �o�́F
//   �Ԃ�l  �Ftrue=����I��  false=���s
//---------------------------------------------------------------------
bool JlsScript::setInputFlags(const char *flags, bool overwrite){
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
// �R�}���h���s�J�n���ݒ�
//=====================================================================

//---------------------------------------------------------------------
// ���O�ݒ�̃T�u�t�H���_�������Ɋ܂߃t�@�C�������݂���t���p�X���擾
// ���́F
//   strSrc   : ���̓t�@�C����
//   flagFull : ���̓t�@�C�������t���p�X���i0=���O�̂݁A1=�t���p�X�L�ځj
// �o�́F
//   strFull : �t�@�C�����݊m�F��̃t���p�X
//   �Ԃ�l  �Ftrue=�t�@�C������  false=�t�@�C���������s
//---------------------------------------------------------------------
bool JlsScript::makeFullPath(string& strFull, const string& strSrc, bool flagFull){
	string strName;				// �t�@�C��������
	string strPathOnly = "";	// �p�X����
	//--- ��{�t�@�C�����ݒ� ---
	if ( flagFull ){			// �t���p�X���͎��̓p�X�����擾����
		pdata->cnv.getStrFilePath(strPathOnly, strSrc);	// �p�X�����ۊ�
		globalState.setPathNameJL(strPathOnly);
		strFull= strSrc;
		strName = strSrc.substr( strPathOnly.length() );
	}
	else{					// Call�Ńt�@�C���������݂̂̏ꍇ�̓p�X����ǉ�����
		strPathOnly = globalState.getPathNameJL();
		strFull = strPathOnly + strSrc;
		strName = strSrc;
	}
	//--- �w��ꏊ�Ńt�@�C���L���m�F ---
	bool decide = makeFullPathIsExist(strFull.c_str());
	//--- ������Ȃ�������T�u�t�H���_���� ---
	if ( decide == false ){
		//--- ��؂蕶���ݒ� ---
		string delimiter = pdata->cnv.getStrFileDelimiter();
		//--- �T�u�t�H���_�̃p�X�w�肠��ΕύX ---
		if ( pdata->extOpt.subPath.empty() == false ){
			strPathOnly = pdata->extOpt.subPath;
			string strTmp;
			pdata->cnv.getStrFilePath(strTmp, strPathOnly);
			delimiter = pdata->cnv.getStrFileDelimiter();	// ��؂�Ċm�F
			if ( strTmp != strPathOnly ){		// �Ōオ��؂蕶�����m�F
				strPathOnly = strPathOnly + delimiter;
			}
		}
		//--- -sublist�ݒ肩��R���}��؂��1���m�F ---
		string remain = pdata->extOpt.subList;		// ���O�ݒ�̃T�u�t�H���_�擾
		remain = remain + ",";						// �T�u�łȂ��ꏊ���Ō�ɒǉ�
		while( remain.empty() == false && decide == false ){
			string subname;
			auto pos = remain.find(",");
			if ( pos != string::npos ){
				subname = remain.substr(0, pos);
				if ( remain.length() > pos ){
					remain = remain.substr(pos+1);
				}else{
					remain = "";
				}
			}
			else{
				subname = remain;
				remain = "";
			}
			if ( subname.empty() == false ){
				subname = subname + delimiter;
			}
			string strTry = strPathOnly + subname + strName;
			decide = makeFullPathIsExist(strTry);
			if ( decide ){			// �T�u�t�H���_�̃p�X���݂Ō��ʍX�V
				strFull = strTry;
			}
		}
	}
	return decide;
}
bool JlsScript::makeFullPathIsExist(const string& str){
	ifstream ifs(str.c_str());
	return ifs.is_open();
}


//=====================================================================
// �R�}���h���s�J�n����
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h�J�n
// �o�́F
//   �Ԃ�l  �F0=����I�� 2=�t�@�C���ُ�
//---------------------------------------------------------------------
int JlsScript::startCmd(const string& fname){
	//--- Call���ߗp��Path�ݒ�A�ǂݍ��݃t�@�C�����ݒ� ---
	string nameFullPath;
	bool flagFull = true;		// ���͂̓t���p�X�t�@�C����
	makeFullPath(nameFullPath, fname, flagFull);

	//--- �V�X�e���ϐ��̏����l��ݒ� ---
	setSystemRegInit();

	//--- JL�X�N���v�g���s ---
	int errnum = startCmdLoop(nameFullPath, 0);

	//--- �f�o�b�O�p�̕\�� ---
	if (pdata->extOpt.verbose > 0 && errnum == 0){
		pdata->displayLogo();
		pdata->displayScp();
	}

	return errnum;
}


//---------------------------------------------------------------------
// �R�}���h�ǂݍ��݁E���s�J�n
// ���́F
//   fname   : �X�N���v�g�t�@�C����
//   loop    : 0=������s 1-:Call�R�}���h�ŌĂ΂ꂽ�ꍇ�̃l�X�g��
// �o�́F
//   �Ԃ�l  �F0=����I�� 2=�t�@�C���ُ�
//---------------------------------------------------------------------
int JlsScript::startCmdLoop(const string& fname, int loop){
	//--- �O��R�}���h�̎��s��Ԃ�false�ɐݒ� ---
	setSystemRegLastexe(false);

	//--- ������s ---
	if (loop == 0){
		globalState.setExe1st(true);
	}
	//--- ����M�� ---
	JlsScriptState state(&globalState);
	//--- ���[�J���ϐ��K�w�쐬 ---
	int numLayerStart = globalState.setLocalRegCreateBase();	// �ŏ�ʊK�w�����ō쐬
	//--- �t�@�C���ǂݍ��� ---
	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		cerr << "error: failed to open " << fname << "\n";
		return 2;
	}
	//--- �e�s�̎��s ---
	bool forceExit = false;
	string strBufOrg;
	while( startCmdGetLine(ifs, strBufOrg, state) ){
		if ( state.isCmdReturnExit() ){
			forceExit = true;	// Exit/Return�ɂ��I��
			break;
		}
		startCmdLoopSub(state, strBufOrg, loop);
	}
	//--- ���[�J���ϐ��K�w�I�� ---
	int numLayerEnd = globalState.setLocalRegReleaseBase();
	//--- �l�X�g�G���[�m�F ---
	if ( forceExit == false ){
		if ( numLayerStart != numLayerEnd ){
			cerr << "error : { and } are not matched\n";
		}
		int flags_remain = state.isRemainNest();
		if (flags_remain & 0x01){
			fprintf(stderr, "error : EndIf is not found\n");
		}
		if (flags_remain & 0x02){
			fprintf(stderr, "error : EndRepeat is not found\n");
		}
		if ( state.isLazyArea() ){
			cerr << "error : EndLazy is not found\n";
		}
		if ( state.isMemArea() ){
			cerr << "error : EndMemory is not found\n";
		}
	}
	//--- �ϐ��K�w�̉���s���͌��ɖ߂� ---
	if ( numLayerStart < numLayerEnd ){
		for(int i=0; i < (numLayerEnd - numLayerStart); i++){
			globalState.setLocalRegReleaseBase();
		}
	}
	//--- ��ԍŌ��-lazy_e�ݒ肳�ꂽ�R�}���h�����s ---
	if (loop == 0){
		startCmdLoopLazyEnd(state);
		startCmdLoopLazyOut(state);		// �o�͗p���Ō�Ɏ��s
	}
	return 0;
}
//---------------------------------------------------------------------
// �Ō��-lazy_e�ݒ肳�ꂽ�R�}���h�����s
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyEnd(JlsScriptState& state){
	//--- lazy_e�ɂ��R�}���h�����o���Ď��s�L���b�V���ɐݒ� ---
//	state.setLazyExe(LazyType::LazyE, "");
	state.setLazyExe(LazyType::FULL, "");	// LAZY_E�����ł͂Ȃ�LAZY_E�ȑO�����s

	//--- lazy���s�L���b�V������ǂݏo�����s ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// �o�͗p��memory���ʎqOUTPUT�̃R�}���h�����s
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyOut(JlsScriptState& state){
	//--- lazy_e�ɂ��R�}���h�����o���Ď��s�L���b�V���ɐݒ� ---
	state.setMemCall("OUTPUT");

	//--- lazy���s�L���b�V������ǂݏo�����s ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// �R�}���h�P�s���̎��s�J�n
// ���o�́F
//   state        : ������
// �o�́F
//   �Ԃ�l    : ���ݍs�̎��s�L���i0=���s�Ȃ�  1=���s����j
//---------------------------------------------------------------------
void JlsScript::startCmdLoopSub(JlsScriptState& state, const string& strBufOrg, int loop){
	//--- �O�R�}���h�̎��s�L������ ---
	bool exe_command = isSystemRegLastexe();
	//--- �ϐ���u�� ---
	string strBuf;
	replaceBufVar(strBuf, strBufOrg);

	//--- �f�R�[�h�����i�R�}���h�����j ---
	JlsCmdSet cmdset;									// �R�}���h�i�[
	bool onlyCmd = true;
	CmdErrType errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// �R�}���h�̂݉��

	//--- �f�R�[�h�����i�S�́j ---
	if ( errval == CmdErrType::None ){
		if ( state.isNeedRaw(cmdset.arg.category) ){	// LazyStart��Memory��Ԓ��͕ϐ���W�J���Ȃ�
			strBuf = strBufOrg;
		}
		if ( state.isNeedFullDecode(cmdset.arg.cmdsel, cmdset.arg.category) ){
			onlyCmd = false;
			errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// �R�}���h�S�̉��
		}else{
			globalState.clearRegError();			// �R�}���h�݂̂̃P�[�X�͓W�J���̃G���[����
		}
	}
	//--- �x�������A�R�}���h��͌�̕ϐ��W�J ---
	bool enable_exe = false;
	if ( errval == CmdErrType::None ){
		//--- �x�����s�����i���ݎ��s���Ȃ��ꍇ��false��Ԃ��j ---
		enable_exe = setStateMem(state, cmdset.arg, strBuf);

		//--- �ϐ��W�J�iIF�����莮�����A�R�}���h���g�p�ϐ��擾�j ---
		if ( enable_exe ){
			bool success = expandDecodeCmd(state, cmdset.arg, strBuf);
			//--- �G���[���͎��s���Ȃ� ---
			if ( success == false ){
				enable_exe = false;
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- �R�}���h���s���� ---
	if (enable_exe){
		bool success = true;
		switch( cmdset.arg.category ){
			case CmdCat::NONE:						// �R�}���h�Ȃ�
				break;
			case CmdCat::COND:						// ��������
				success = setCmdCondIf(cmdset.arg, state);
				break;
			case CmdCat::CALL:						// Call��
				success = setCmdCall(cmdset.arg, loop);
				break;
			case CmdCat::REP:						// �J��Ԃ���
				success = setCmdRepeat(cmdset.arg, state);
				break;
			case CmdCat::FLOW:						// ���s����
				success = setCmdFlow(cmdset.arg, state);
				break;
			case CmdCat::SYS:						// �V�X�e���R�}���h
				success = setCmdSys(cmdset.arg);
				break;
			case CmdCat::REG:						// �ϐ��ݒ�
				success = setCmdReg(cmdset.arg);
				break;
			case CmdCat::LAZYF:						// Lazy����
			case CmdCat::MEMF:						// Memory����
				success = setCmdMemFlow(cmdset.arg, state);
				break;
			case CmdCat::MEMEXE:					// �x���ݒ���s����
				success = setCmdMemExe(cmdset.arg, state);
				break;
			default:								// ��ʃR�}���h
				if ( globalState.isExe1st() ){		// ����݂̂̃`�F�b�N
					globalState.setExe1st(false);
					if ( pdata->isSetupAdjInitial() ){
						pdata->setFlagSetupAdj( true );
						//--- �ǂݍ��݃f�[�^������ ---
						JlsReformData func_reform(pdata);
						func_reform.adjustData();
						setSystemRegNologo(true);
					}
				}
				exe_command = exeCmd(cmdset);
				break;
		}
		if ( success == false ){
			errval = CmdErrType::ErrCmd;
		}
	}
	//--- �G���[�`�F�b�N ---
	if ( errval != CmdErrType::None ){
		exe_command = false;
	}
	startCmdDispErr(strBuf, errval);
	//--- debug ---
	if ( pdata->extOpt.vLine > 0 ){
		cout << enable_exe << exe_command << ":" << strBuf << endl;
	}

	//--- �O�R�}���h�̎��s�L������ ---
	setSystemRegLastexe(exe_command);
}

//---------------------------------------------------------------------
// ���̕�����擾
// ���o�́F
//   ifs          : �t�@�C�����
//   state        : ������
//   fromFile     : �t�@�C�������read����
// �o�́F
//   �Ԃ�l    : ������擾���ʁi0=�擾�Ȃ�  1=�擾����j
//   strBufOrg : �擾������
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLine(ifstream& ifs, string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- cache����̓ǂݍ��� ---
	flagRead = startCmdGetLineOnlyCache(strBufOrg, state);

	if ( flagRead == false ){
		//--- �t�@�C������̓ǂݍ��� ---
		if ( startCmdGetLineFromFile(ifs, strBufOrg) ){
			flagRead = true;
			//--- Repeat�p�L���b�V���ɕۑ� ---
			state.addCmdCache(strBufOrg);
		}
	}
	return flagRead;
}
//---------------------------------------------------------------------
// �t�@�C��read�Ȃ��̃L���b�V���f�[�^����̂�
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineOnlyCache(string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- �x�����s����̓ǂݍ��� ---
	if ( state.readLazyMemNext(strBufOrg) ){
		flagRead = true;
	}
	//--- Repeat�p�L���b�V���ǂݍ��� ---
	else if ( state.readCmdCache(strBufOrg) ){
		flagRead = true;
	}
	return flagRead;
}
//---------------------------------------------------------------------
// �t�@�C������P�s�ǂݍ��݁i \�ɂ��s�p���ɑΉ��j
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineFromFile(ifstream& ifs, string& strBufOrg){
	strBufOrg = "";
	bool success = false;
	bool cont = true;
	while( cont ){
		cont = false;
		string buf;
		if ( getline(ifs, buf) ){
			auto len = buf.length();
			if ( len >= INT_MAX/4 ){		// �ʓ|���͍ŏ��ɃJ�b�g
				return false;
			}
			//--- ���s�p���m�F ---
			if ( len >= 2 ){
				auto pos = buf.find(R"( \ )");
				if ( pos != string::npos ){
					buf = buf.substr(0, pos+1);		// �󔒂͎c��
					cont = true;
				}
				else if ( buf.substr(len-2) == R"( \)" ){
					buf = buf.substr(0, len-1);		// �󔒂͎c��
					cont = true;
				}
			}
			//--- �ݒ� ---
			strBufOrg += buf;
			success = true;
		}
	};
	return success;
}

//---------------------------------------------------------------------
// �G���[�\��
//---------------------------------------------------------------------
void JlsScript::startCmdDispErr(const string& strBuf, CmdErrType errval){
	if ( errval != CmdErrType::None ){
		string strErr = "";
		switch(errval){
			case CmdErrType::ErrOpt:
				strErr = "error: wrong argument";
				break;
			case CmdErrType::ErrRange:
				strErr = "error: wrong range argument";
				break;
			case CmdErrType::ErrSEB:
				strErr = "error: need Start or End";
				break;
			case CmdErrType::ErrVar:
				strErr = "error: failed variable setting";
				break;
			case CmdErrType::ErrTR:
				strErr = "error: need auto command TR/SP/EC";
				break;
			case CmdErrType::ErrCmd:
				strErr = "error: wrong command";
				break;
			default:
				break;
		}
		if ( strErr.empty() == false ){
			cerr << strErr << " in " << strBuf << "\n";
		}
	}
	globalState.checkErrorGlobalState(true);
}

//---------------------------------------------------------------------
// �ϐ�������u������������o��
// ���́F
//   strBuf : ������
// �o�́F
//   �Ԃ�l  �F�u�����ʁitrue=����  false=���s�j
//   dstBuf  : �o�͕�����
//---------------------------------------------------------------------
bool JlsScript::replaceBufVar(string& dstBuf, const string& srcBuf){
	string strName, strVal;
	int len_var, pos_var;

	dstBuf.clear();
	bool success = true;
	int pos_cmt = (int) srcBuf.find("#");
	int pos_base = 0;
	while(pos_base >= 0){
		//--- �ϐ������̒u�� ---
		pos_var = (int) srcBuf.find("$", pos_base);
		if (pos_var >= 0){
			//--- $��O�܂ł̕�������m�� ---
			if (pos_var > pos_base){
				dstBuf += srcBuf.substr(pos_base, pos_var-pos_base);
				pos_base = pos_var;
			}
			//--- �ϐ����������Ēu�� ---
			len_var = replaceRegVarInBuf(strVal, srcBuf, pos_var);
			if (len_var > 0){
				dstBuf += strVal;
				pos_base += len_var;
			}
			else{
				if (pos_var < pos_cmt || pos_cmt < 0){		// �R�����g�łȂ���Βu�����s
					success = false;
					globalState.addMsgError("error: not defined variable in " + srcBuf + "\n");
				}
				pos_var = -1;
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
int JlsScript::replaceRegVarInBuf(string& strVal, const string& strBuf, int pos){
	int var_st, var_ed;
	bool exact;

	int ret = 0;
	if (strBuf[pos] == '$'){
		//--- �ϐ��������擾 ---
		pos ++;
		if (strBuf[pos] == '{'){		// ${�ϐ���}�t�H�[�}�b�g���̏���
			exact = true;
			pos ++;
			var_st = pos;
			while(strBuf[pos] != '}' && strBuf[pos] != '\0'){
				pos ++;
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
		if (var_st <= var_ed){
			string strCandName = strBuf.substr(var_st, var_ed-var_st+1);
			int nmatch = getJlsRegVar(strVal, strCandName, exact);
			if (nmatch > 0){
				ret = nmatch + 1 + (exact*2);	// �ϐ����� + $ + {}
			}
		}
	}
	return ret;
}



//=====================================================================
// �x�����s�̐ݒ�
//=====================================================================

//---------------------------------------------------------------------
// Memory�ɂ��L������ + �R�}���h�I�v�V�������e����Lazy�����̐ݒ�
// ���́F
//   cmdset   : �R�}���h�I�v�V�������e
//   strBuf   : ���ݍs�̕�����
// ���o�́F
//   state     : mem/lazy�����ǉ�
// �o�́F
//   �Ԃ�l   �F���ݍs�̃R�}���h���s�L���i�L���b�V���Ɉڂ������͎��s���Ȃ��j
//---------------------------------------------------------------------
bool JlsScript::setStateMem(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool enable_exe = true;
	//--- If�����ɂ��skip�i����s���ԁj���͒x���֘A�������������Ȃ� ---
	if ( state.isSkipCmd() ){
	}
	//--- Memory���Ԓ��Ȃ�Memory���Ԑ���ȊO�̕�����͋L���̈�Ɋi�[ ---
	else if ( state.isMemArea() ){
		if ( cmdarg.category != CmdCat::MEMF ){
			enable_exe = state.setMemStore(state.getMemName(), strBuf);
		}
	}
	//--- Memory���Ԓ��ȊO��Lazy�����m�F ---
	else{
		if ( cmdarg.category != CmdCat::LAZYF ){	// LazyStart,EndLazy�͏���
			enable_exe = setStateMemLazy(state, cmdarg, strBuf);
		}
	}

	//--- �����Ԃ���̃R�}���h���s�L���� ---
	if ( state.isInvalidCmdLine(cmdarg.category) ){
		enable_exe = false;
	}
	return enable_exe;
}
//---------------------------------------------------------------------
// �R�}���h�I�v�V�������e����Lazy�����̐ݒ�
// ������ setStateMem() �Q��
//---------------------------------------------------------------------
bool JlsScript::setStateMemLazy(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool enable_exe = true;
	bool flagNormalCmd = false;		// �ʏ�R�}���h�������ꍇ�͌��true���ݒ肳���

	//--- lazy���s���łȂ����lazy�`�F�b�N���{ ---
	if ( state.isLazyExe() == false ){
		//--- Lazy��ގ擾 ---
		LazyType typeLazy = state.getLazyStartType();	// LazyStart�ɂ����Lazy�^�C�v�擾
		//--- Lazy��ޕ␳ ---
		setStateMemLazyRevise(typeLazy, state, cmdarg);	// ���R�}���h�̕K�v�ȏ��
		//--- lazy�ݒ� ---
		switch( typeLazy ){
			case LazyType::LazyS:				// -lazy_s
			case LazyType::LazyA:				// -lazy_a
			case LazyType::LazyE:				// -lazy_e
				//--- lazy��ނɑΉ�����ꏊ��lazy�R�}���h��ۊǂ��� ---
				enable_exe = state.setLazyStore(typeLazy, strBuf);
				break;
			default:						// lazy�w��Ȃ�
				flagNormalCmd = true;		// �ʏ�̃R�}���h
				break;
		}
	}
	//--- lazy�������̊m�F ---
	else{
		LazyType lazyPassed = state.getLazyExeType();	// �Ǎ�����Lazy���
		LazyType lazyNeed = LazyType::None;
		bool needA = setStateMemLazyRevise(lazyNeed, state, cmdarg);	// �Ǎ��R�}���h�̕K�v�ȏ��

		//--- Auto�K�v������Auto�����s�̏ꍇ�A����LAZY_E�Ȃ���s���Ȃ� ---
		if ( isLazyAutoModeInitial(state) && needA && lazyPassed == LazyType::LazyE ){
			enable_exe = false;
		}else{
			//--- �����_�ł͂܂����s�ł��Ȃ���Ԋm�F ---
			LazyType lazyNext = LazyType::None;
			switch( lazyNeed ){
				case LazyType::LazyE :
					if ( lazyPassed == LazyType::LazyA ||
					     lazyPassed == LazyType::LazyS ){
						lazyNext = LazyType::LazyE;
					}
					break;
				case LazyType::LazyA :
					if ( lazyPassed == LazyType::LazyS ){
						lazyNext = LazyType::LazyA;
					}
					break;
				default :
					break;
			}
			if ( lazyNext != LazyType::None ){		// �܂�����Lazy��Ԃɓ��꒼��
					enable_exe = state.setLazyStore(lazyNext, strBuf);
			}
		}
	}
	//--- �R�}���h���s����Lazy���s�J�n ---
	if ( enable_exe ){
		//--- �ΏۃR�}���h�����s�����ꍇ�ɂ�lazy�ۊǍs��lazy���s�s�Ɉڂ� ---
		switch( cmdarg.category ){
			case CmdCat::AUTO:
			case CmdCat::AUTOEACH:
			case CmdCat::AUTOLOGO:
				if ( flagNormalCmd ){
					enable_exe = state.setLazyExe(LazyType::LazyA, strBuf);
				}
				if ( enable_exe ){
					state.setLazyStateIniAuto(false);	// Auto���s�ςݏ�Ԃɂ���
				}
				break;
			case CmdCat::LOGO:
				if ( flagNormalCmd ){
					enable_exe = state.setLazyExe(LazyType::LazyS, strBuf);
				}
				break;
			default:
				break;
		}
	}
	return enable_exe;
}
//--- Lazy����p��Auto�����s��Ԋm�F ---
bool JlsScript::isLazyAutoModeInitial(JlsScriptState& state){
	return pdata->isAutoModeInitial() || state.isLazyStateIniAuto();
}
//---------------------------------------------------------------------
// �R�}���h�ɂ��Lazy��ނ̕␳
//---------------------------------------------------------------------
bool JlsScript::setStateMemLazyRevise(LazyType& typeLazy, JlsScriptState& state, JlsCmdArg& cmdarg){
	//--- -now�I�v�V�������͕␳���Ȃ� ---
	if ( cmdarg.getOpt(OptType::FlagNow) > 0 ){
		return false;
	}
	//--- �I�v�V���������������Lazy�^�C�v�ύX ---
	if ( cmdarg.tack.typeLazy != LazyType::None ){
		typeLazy = cmdarg.tack.typeLazy;
	}
	//--- �R�}���h���ޕʂ�Lazy�R�}���h����␳ ---
	bool flagAuto = state.isLazyAuto();		// LazyAuto�R�}���h�ɂ�鎩���t���t���O
	bool flagInit = isLazyAutoModeInitial(state);
	bool flagInitAuto = flagInit && flagAuto;
	bool needS = false;
	bool needA = false;
	bool needE = false;
	switch( cmdarg.category ){
		case CmdCat::AUTOEACH:
		case CmdCat::AUTOLOGO:
			needA = flagInit;
			break;
		case CmdCat::AUTO:
			needA = flagInitAuto;
			break;
		case CmdCat::LOGO:
			needS = flagInitAuto;
			break;
		default :
			break;
	}
	if ( flagInitAuto ){
		if ( cmdarg.getOpt(OptType::FlagAutoChg) > 0 ){	// Auto�n�ɐݒ肷��I�v�V����
				needA = true;
		}
		if ( cmdarg.tack.needAuto ){	// �I�v�V������Auto�n�\�����K�v�ȏꍇ(-AC -NoAC)
				needA = true;
		}
	}
	if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){
		if ( needA || needS ){
			needE = true;		// -final�ݒ莞�͕ύX���̓����lazy_e�ɂ���
		}
	}
	//--- �ݒ�ύX ---
	if ( needE ){
		typeLazy = LazyType::LazyE;
	}
	else if ( needA && (typeLazy == LazyType::None || typeLazy == LazyType::LazyS) ){
		typeLazy = LazyType::LazyA;
	}
	else if ( needS && typeLazy == LazyType::None ){
		typeLazy = LazyType::LazyS;
	}
	return needA;		// �Ԃ�l��Auto���K�v�ȃR�}���h���ǂ���
}



//=====================================================================
// �R�}���h��͌�̕ϐ��W�J�iIF�����莮�����A�R�}���h���g�p�ϐ��擾�j
//=====================================================================

//---------------------------------------------------------------------
// �ϐ��W�J�iIF����������Ǝg�p�ϐ�(POSHOLD/LISTHOLD)�擾�j
//---------------------------------------------------------------------
bool JlsScript::expandDecodeCmd(JlsScriptState& state, JlsCmdArg& cmdarg, const string& strBuf){
	bool success = true;

	//--- IF���������� ---
	bool flagCond = true;					// �����Ȃ����true
	int  num = cmdarg.getNumCheckCond();	// ����K�v�Ȉ����ʒu�擾
	if ( num > 0 ){
		success = getCondFlag(flagCond, cmdarg.getStrArg(num));
	}
	cmdarg.setCondFlag(flagCond);			// IF�����̌��ʐݒ�

	//--- �R�}���h�ŕK�v�ƂȂ�ϐ����擾 ---
	getDecodeReg(cmdarg);

	return success;
}
//---------------------------------------------------------------------
// ������Ώۈʒu�ȍ~�̃t���O�𔻒�
// ���́F
//   strBuf : ������
//   pos    : �F���J�n�ʒu
// �o�́F
//   �Ԃ�l  : ����
//   flagCond  �F�t���O����i0=false  1=true�j
//---------------------------------------------------------------------
bool JlsScript::getCondFlag(bool& flagCond, const string& strBuf){
	string strItem;
	string strCalc = "";
	string strBufRev = strBuf;
	//--- �R�����g�J�b�g ---
	int ntmp = (int) strBuf.find("#");
	if (ntmp >= 0){
		strBufRev = strBuf.substr(0, ntmp);
	}
	//--- �P�P�ꂸ�m�F ---
	int pos = 0;
	while(pos >= 0){
		pos = getCondFlagGetItem(strItem, strBufRev, pos);
		if (pos >= 0){
			getCondFlagConnectWord(strCalc, strItem);
		}
	}
	int pos_calc = 0;
	int val;
	if ((int)strCalc.find(":") >= 0 || (int)strCalc.find(".") >= 0){		// ���ԕ\�L�������ꍇ
		pos_calc = pdata->cnv.getStrValMsec(val, strCalc, 0);	// ���ԒP�ʂŔ�r
	}
	else{
		pos_calc = pdata->cnv.getStrValNum(val, strCalc, 0);	// strCalc�̐擪����擾
	}
	//--- ���ʊm�F ---
	bool success = true;
	if (pos_calc < 0){
		val = 0;
		success = false;
		cerr << "error: can not evaluate(" << strCalc << ")" << endl;
	}
	flagCond = (val != 0)? true : false;
	return success;
}
//--- ���̍��ڂ��擾�A������̏ꍇ�͔�r���ʂ��m�F���� ---
int JlsScript::getCondFlagGetItem(string& strItem, const string& strBuf, int pos){
	pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, pos);
	if ( pos >= 0 ){
		//--- quote�������ꍇ�̂ݕ������r���s�� ---
		if ( strItem[0] == '\"' ){
			string str2;
			string str3;
			int pos2 = pdata->cnv.getStrItemWithQuote(str2, strBuf, pos);
			int pos3 = pdata->cnv.getStrItemWithQuote(str3, strBuf, pos2);
			if ( pos3 >= 0 ){
				bool flagEq = ( strItem == str3 )? true : false;
				if ( str2 == "==" ){
					pos = pos3;
					strItem = ( flagEq )? "1" : "0";
				}
				else if ( str2 == "!=" ){
					pos = pos3;
					strItem = ( flagEq )? "0" : "1";
				}
			}
		}
	}
	return pos;
}

//---------------------------------------------------------------------
// �t���O�p�ɕ������A��
// ���o�́F
//   strCalc : �A���敶����
// ���́F
//   strItem : �ǉ�������
//---------------------------------------------------------------------
void JlsScript::getCondFlagConnectWord(string& strCalc, const string& strItem){

	//--- �A�������̒ǉ��i��r���Z�q���Q���ԂɂȂ����OR(||)��ǉ�����j ---
	char chNextFront = strItem.front();
	char chNextFr2   = strItem[1];
	if (strCalc.length() > 0 && strItem.length() > 0){
		char chPrevBack = strCalc.back();
		if (chPrevBack  != '=' && chPrevBack  != '<' && chPrevBack  != '>' &&
			chPrevBack  != '|' && chPrevBack  != '&' &&
			chNextFront != '|' && chNextFront != '&' &&
			chNextFront != '=' && chNextFront != '<' && chNextFront != '>'){
			if (chNextFront == '!' && chNextFr2 == '='){
			}
			else{
				strCalc += "||";
			}
		}
	}
	//--- ���]���Z�̔��� ---
	string strRemain;
	if (chNextFront == '!'){
		strCalc += "!";
		strRemain = strItem.substr(1);
	}
	else{
		strRemain = strItem;
	}
	//--- �t���O�ϐ��̔��� ---
	char chFront = strRemain.front();
	if ((chFront >= 'A' && chFront <= 'Z') || (chFront >= 'a' && chFront <= 'z')){
		string strVal;
		//--- �ϐ�����t���O�̒l���擾 ---
		int nmatch = getJlsRegVar(strVal, strRemain, true);
		if (nmatch > 0 && strVal != "0"){	// �ϐ������݂���0�ȊO�̏ꍇ
			strVal = "1";
		}
		else{
			strVal = "0";
		}
		strCalc += strVal;				// �t���O�̒l�i0�܂���1�j��ǉ�
	}
	else{
		strCalc += strRemain;			// �ǉ�����������̂܂ܒǉ�
	}
//printf("(rstr:%s)",strCalc.c_str());
}

//---------------------------------------------------------------------
// ��͂ŕK�v�ƂȂ�ϐ����擾(POSHOLD/LISTHOLD)
//---------------------------------------------------------------------
void JlsScript::getDecodeReg(JlsCmdArg& cmdarg){
	//--- �ϐ����擾 ---
	string strRegPos  = cmdarg.getStrOpt(OptType::StrRegPos);
	string strRegList = cmdarg.getStrOpt(OptType::StrRegList);
	//--- �����l�ێ� ---
	string strDefaultPos  = cmdarg.getStrOpt(OptType::StrValPosW);
	string strDefaultList = cmdarg.getStrOpt(OptType::StrValListW);
	//--- �ϐ��l�擾�ݒ� ---
	{
		string strVal;
		//--- POSHOLD�̒l��ݒ� ---
		if ( getJlsRegVar(strVal, strRegPos, true) > 0 ){	// �ϐ��擾
			cmdarg.setStrOpt(OptType::StrValPosR, strVal);	// �ύX
			cmdarg.setStrOpt(OptType::StrValPosW, strVal);	// �ύX
			cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// ���W�X�^�X�V�s�v
		}

		//--- LISTHOLD�̒l��ݒ� ---
		if ( getJlsRegVar(strVal, strRegList, true) > 0 ){	// �ϐ��擾
			cmdarg.setStrOpt(OptType::StrValListR, strVal);	// �ύX
			cmdarg.setStrOpt(OptType::StrValListW, strVal);	// �ύX
			cmdarg.clearStrOptUpdate(OptType::StrValListW);	// ���W�X�^�X�V�s�v
		}
	}
	//--- ���ʊi�[�p�̏����l�ύX ---
	switch( cmdarg.cmdsel ){
		case CmdType::GetPos:
		case CmdType::ListGetAt:
			if ( cmdarg.getOpt(OptType::FlagClear) > 0 ){		// -clear�I�v�V�������̂�
				cmdarg.setStrOpt(OptType::StrValPosW, strDefaultPos);	// �ύX
			}
			break;
		case CmdType::GetList:
			{
				cmdarg.setStrOpt(OptType::StrValListW, strDefaultList);	// �ύX
			}
			break;
		default:
			break;
	}
}



//=====================================================================
// �V�X�e���ϐ��ݒ�
//=====================================================================

//---------------------------------------------------------------------
// �����ݒ�ϐ�
//---------------------------------------------------------------------
void JlsScript::setSystemRegInit(){
	setSystemRegHeadtail(-1, -1);
	setSystemRegUpdate();
}

//---------------------------------------------------------------------
// �����ݒ�ϐ��̌��ݒl�ɂ��ύX
//---------------------------------------------------------------------
void JlsScript::setSystemRegUpdate(){
	int n = pdata->getMsecTotalMax();
	string str_val = pdata->cnv.getStringFrameMsecM1(n);
	string str_time = pdata->cnv.getStringTimeMsecM1(n);
	setJlsRegVar("MAXFRAME", str_val, true);
	setJlsRegVar("MAXTIME", str_time, true);
	setJlsRegVar("NOLOGO", to_string(pdata->extOpt.flagNoLogo), true);
}

//---------------------------------------------------------------------
// HEADFRAME/TAILFRAME��ݒ�
//---------------------------------------------------------------------
void JlsScript::setSystemRegHeadtail(int headframe, int tailframe){
	string str_head = pdata->cnv.getStringTimeMsecM1(headframe);
	string str_tail = pdata->cnv.getStringTimeMsecM1(tailframe);
	setJlsRegVar("HEADTIME", str_head, true);
	setJlsRegVar("TAILTIME", str_tail, true);
}

//---------------------------------------------------------------------
// �����ȃ��S�̊m�F�i���S���Ԃ��ɒ[�ɒZ�������烍�S�Ȃ������ɂ���j
//---------------------------------------------------------------------
void JlsScript::setSystemRegNologo(bool need_check){
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
// �O��̎��s��Ԃ�ݒ�
//---------------------------------------------------------------------
void JlsScript::setSystemRegLastexe(bool exe_command){
	//--- �O��̎��s��Ԃ�ϐ��ɐݒ� ---
	setJlsRegVar("LASTEXE", to_string((int)exe_command), true);
}
//--- ���O�̎��s��Ԏ擾 ---
bool JlsScript::isSystemRegLastexe(){
	string strVal = "0";
	getJlsRegVar(strVal, "LASTEXE", true);
	bool lastExe = ( strVal != "0" )? true : false;
	return lastExe;
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
bool JlsScript::setSystemRegOptions(const string& strBuf, int pos, bool overwrite){
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
	return true;
}


//=====================================================================
// �R�}���h���ʂɂ��ϐ��X�V
//=====================================================================

//--- $POSHOLD/$LISTHOLD�̃��W�X�^�X�V ---
void JlsScript::updateResultRegWrite(JlsCmdArg& cmdarg){
	//--- POSHOLD�̍X�V ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValPosW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegPos);	// �ϐ���($POSHOLD)
		string strVal    = cmdarg.getStrOpt(OptType::StrValPosW);
		bool   overwrite = true;
		bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
		setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);	// $POSHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// �X�V�����ʒm
	}
	//--- LISTHOLD�̍X�V ---
	if ( cmdarg.isUpdateStrOpt(OptType::StrValListW) ){
		string strName   = cmdarg.getStrOpt(OptType::StrRegList);	// �ϐ���($LISTHOLD)
		string strList   = cmdarg.getStrOpt(OptType::StrValListW);
		bool   overwrite = true;
		bool   flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
		setJlsRegVarWithLocal(strName, strList, overwrite, flagLocal);	// $LISTHOLD
		cmdarg.clearStrOptUpdate(OptType::StrValListW);	// �X�V�����ʒm
	}
}
//--- $SIZEHOLD�̃��W�X�^�ݒ� ---
void JlsScript::setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList){
	string strSizeName = cmdarg.getStrOpt(OptType::StrRegSize);	// �ϐ���($SIZEHOLD)
	int    numList     = getListStrSize(strList);		// ���ڐ��擾
	string strNumList  = std::to_string(numList);
	bool   overwrite   = true;
	bool   flagLocal   = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
	setJlsRegVarWithLocal(strSizeName, strNumList, overwrite, flagLocal);	// $SIZEHOLD
}
//--- $POSHOLD�̍X�V ---
void JlsScript::setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos){
	string strVal  = pdata->cnv.getStringTimeMsecM1(msecPos);
	cmdarg.setStrOpt(OptType::StrValPosW, strVal);			// $POSHOLD write
	updateResultRegWrite(cmdarg);
}

//--- $LISTHOLD�̍X�V ---
void JlsScript::setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList = cmdarg.getStrOpt(OptType::StrValListW);	// write�ϐ��l

	//--- ���ڒǉ� ---
	if ( msecPos != -1 ){
		if ( strList.empty() == false ){
			strList += ",";
		}
		string strVal = pdata->cnv.getStringTimeMsecM1(msecPos);
		strList += strVal;
	}
	cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
	updateResultRegWrite(cmdarg);
	//--- ���X�g���X�V ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListGetAt�ɂ��X�V ---
void JlsScript::setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosW);
	//--- �Ώۈʒu�̍��ڂ��擾 ---
	{
		string strItem;
		if ( getListStrElement(strItem, strList, numItem) ){
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
void JlsScript::setResultRegListIns(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	string strValPos = cmdarg.getStrOpt(OptType::StrValPosR);
	//--- Ins���� ---
	if ( setListStrIns(strList, strValPos, numItem) ){
		//--- ListHold�ɐݒ� ---
		cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
		updateResultRegWrite(cmdarg);
	}
	//--- ���X�g���X�V ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ListDel�ɂ��X�V ---
void JlsScript::setResultRegListDel(JlsCmdArg& cmdarg, int numItem){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	//--- Del���� ---
	if ( setListStrDel(strList, numItem) ){
		//--- ListHold�ɐݒ� ---
		cmdarg.setStrOpt(OptType::StrValListW, strList);		// $LISTHOLD write
		updateResultRegWrite(cmdarg);
	}
	//--- ���X�g���X�V ---
	setResultRegWriteSize(cmdarg, strList);
}
//--- ���X�g�̍��ڐ���Ԃ� ---
int JlsScript::getListStrSize(const string& strList){
	int numList = 0;
	//--- ���X�g���ڐ����擾 ---
	if ( strList.empty() == false ){
		numList = 1;
		for(int i=0; i < (int)strList.size(); i++){
			if ( strList[i] == ',' ) numList++;
		}
	}
	return numList;
}
//--- ���X�g�̎w�荀�ڈʒu��������̉��Ԗڂ��擾 ---
int JlsScript::getListStrPos(const string& strList, int num, bool flagIns){
	int numList = getListStrSize(strList);	// ���ڐ��擾
	//--- ���ڂ��擾 ---
	int numAbs = ( num >= 0 )? num : numList + num + 1;
	if ( flagIns && num < 0 ){		// Ins���͍ő區�ڐ����P����
		numAbs += 1;
	}
	//--- �ʒu���擾 ---
	int pos = -1;
	if ( numAbs > 0 && numAbs <= numList ){
		pos = 0;
		int nc = 1;
		for(int i=0; i < (int)strList.size(); i++){
			if ( strList[i] == ',' ){
				nc ++;
				if ( nc == numAbs ){
					pos = i + 1;
				}
			}
		}
	}else if ( numAbs > 0 && (numAbs == numList + 1) && flagIns ){
		pos = (int)strList.size();
	}
	return pos;
}
//--- ���X�g�̎w�荀�ڈʒu����̍��ڂ����������擾 ---
int JlsScript::getListStrPosLen(const string& strList, int pos){
	bool det = false;
	int len = 0;
	int posMax = (int)strList.length();
	while ( pos >= 0 && pos < posMax && det == false ){
		if ( strList[pos] == ',' ){
			det = true;
		}else{
			len ++;
		}
		pos ++;
	}
	return len;
}
//--- ���X�g�̎w�荀�ڈʒu�ɂ��镶�����Ԃ� ---
bool JlsScript::getListStrElement(string& strItem, const string& strList, int num){
	strItem = "";
	bool flagIns = false;
	int locSt   = getListStrPos(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	int lenItem = getListStrPosLen(strList, locSt);
	if ( lenItem > 0 ){			// 1�����ȏ�擾�Ő���
		strItem = strList.substr(locSt, lenItem);
		return true;
	}
	return false;
}
//--- ���X�g�̎w�荀�ڈʒu�ɕ������}�� ---
bool JlsScript::setListStrIns(string& strList, const string& strItem, int num){
	int lenList = (int)strList.length();
	//--- �Ώۍ��ڂ̐擪�ʒu�擾 ---
	bool flagIns = true;
	int locSt   = getListStrPos(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	//--- �}������ ---
	if ( locSt == 0 ){			// �擪����
		if ( strList.empty() ){	// �ŏ��̍���
			strList = strItem;
		}else{
			strList = strItem + "," + strList;
		}
	}else if ( locSt == lenList ){	// �Ō�
		strList = strList + "," + strItem;
	}else{
		string strTmp = strList.substr(locSt-1);
		if ( locSt == 1 ){
			strList = "," + strItem + strTmp;
		}else{
			strList = strList.substr(0, locSt-1) + "," + strItem + strTmp;
		}
	}
	return true;
}
//--- ���X�g�̎w�荀�ڈʒu�̕�������폜 ---
bool JlsScript::setListStrDel(string& strList, int num){
	int lenList = (int)strList.length();
	//--- �Ώۍ��ڂ̐擪�ʒu�ƕ��������擾 ---
	bool flagIns = false;
	int locSt   = getListStrPos(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	int lenItem = getListStrPosLen(strList, locSt);
	//--- �폜���� ---
	if ( locSt == 0 ){
		if ( lenItem + 1 >= lenList ){
			strList.clear();
		}else{
			strList = strList.substr(lenItem + 1);
		}
	}else if ( locSt == 1 ){
		strList = strList.substr(lenItem + 1);
	}else{
		string strTmp = "";
		if ( locSt + lenItem < lenList ){
			strTmp = strList.substr(locSt + lenItem);
		}
		strList = strList.substr(0, locSt-1) + strTmp;
	}
	return true;
}


//=====================================================================
// �ݒ�R�}���h����
//=====================================================================

//---------------------------------------------------------------------
// If������
//---------------------------------------------------------------------
bool JlsScript::setCmdCondIf(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;
	bool flagCond = cmdarg.getCondFlag();
	switch( cmdarg.cmdsel ){
		case CmdType::If:						// If��
			state.ifBegin(flagCond);
			break;
		case CmdType::EndIf:					// EndIf��
			{
				int errnum = state.ifEnd();
				if (errnum > 0){
					globalState.addMsgError("error: too many EndIf.\n");
					success = false;
				}
			}
			break;
		case CmdType::Else:						// Else��
		case CmdType::ElsIf:					// ElsIf��
			{
				int errnum = state.ifElse(flagCond);
				if (errnum > 0){
					globalState.addMsgError("error: not exist 'If' but exist 'Else/ElsIf' .\n");
					success = false;
				}
			}
			break;
		default:
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Call����
//---------------------------------------------------------------------
bool JlsScript::setCmdCall(JlsCmdArg& cmdarg, int loop){
	bool success = true;

	if ( cmdarg.cmdsel == CmdType::Call ){
		//--- Call���ߗp��Path�ݒ�A�ǂݍ��݃t�@�C�����ݒ� ---
		string strFileName;
		bool flagFull = false;		// ���͂̓t�@�C���������̂�
		makeFullPath(strFileName, cmdarg.getStrArg(1), flagFull);
		//--- Call���s���� ---
		loop ++;
		if (loop < SIZE_CALL_LOOP){				// �ċA�Ăяo���͉񐔐���
			startCmdLoop(strFileName, loop);
		}
		else{
			// �����Ăяo���ɂ��o�b�t�@�I�[�o�[�t���[�h�~�̂���
			globalState.addMsgError("error: many recursive call\n");
			success = false;
		}
	}
	return success;
}

//---------------------------------------------------------------------
// ���s�[�g�R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdRepeat(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::Repeat:				// Repeat��
			{
				int val = cmdarg.getValStrArg(1);
				int errnum = state.repeatBegin(val);
				if (errnum > 0){
					globalState.addMsgError("error: overflow at Repeat\n");
					success = false;
				}
			}
			break;
		case CmdType::EndRepeat:			// EndRepeat��
			{
				int errum = state.repeatEnd();
				if (errum > 0){
					globalState.addMsgError("error: Repeat - EndRepeat not match\n");
					success = false;
				}
			}
			break;
		default:
			break;
	}
	return success;
}
//---------------------------------------------------------------------
// �X�N���v�g���s����R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdFlow(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::LocalSt:
			globalState.setLocalRegCreateOne();
			break;
		case CmdType::LocalEd:
			globalState.setLocalRegReleaseOne();
			break;
		case CmdType::Exit:
			globalState.setCmdExit(true);
			break;
		case CmdType::Return:
			state.setCmdReturn(true);
			break;
		default:
			globalState.addMsgError("error:internal setting\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// �V�X�e���֘A�R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdSys(JlsCmdArg& cmdarg){
	bool success = true;

	switch ( cmdarg.cmdsel ){
		case CmdType::FileOpen:
			globalState.fileOpen(cmdarg.getStrArg(1), false);
			break;
		case CmdType::FileAppend:
			globalState.fileOpen(cmdarg.getStrArg(1), true);
			break;
		case CmdType::FileClose:
			globalState.fileClose();
			break;
		case CmdType::Echo:
			globalState.fileOutput(cmdarg.getStrArg(1) + "\n");
			break;
		case CmdType::LogoOff:
			setSystemRegNologo(false);
			break;
		case CmdType::OldAdjust:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.oldAdjust = val;
			}
			break;
		default:
			globalState.addMsgError("error:internal setting\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// ���W�X�^�ݒ�֘A����
//---------------------------------------------------------------------
bool JlsScript::setCmdReg(JlsCmdArg& cmdarg){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::Default:
		case CmdType::Set:
		case CmdType::EvalFrame:
		case CmdType::EvalTime:
		case CmdType::EvalNum:
		case CmdType::LocalSet:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::Default )? false : true;
				bool flagLocal = ( cmdarg.cmdsel == CmdType::LocalSet )? true : false;
				if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -local�I�v�V����
					flagLocal = true;
				}
				success = setJlsRegVarWithLocal(cmdarg.getStrArg(1), cmdarg.getStrArg(2), overwrite, flagLocal);
			}
			break;
		case CmdType::SetParam:
			{
				ConfigVarType typePrm = (ConfigVarType) cmdarg.getValStrArg(1);
				int val = cmdarg.getValStrArg(2);
				pdata->setConfig(typePrm, val);
			}
			break;
		case CmdType::CountUp:
			{
				success = false;
				bool flagLocal = ( cmdarg.getOpt(OptType::FlagLocal) > 0 )? true : false;
				setJlsRegVarWithLocal(cmdarg.getStrArg(1), "0", false, flagLocal);	// ���ݒ莞��0��������
				string strVal;
				bool exact = true;
				if ( getJlsRegVar(strVal, cmdarg.getStrArg(1), exact) > 0 ){
					int val;
					int pos = 0;
					pos = pdata->cnv.getStrValNum(val, strVal, pos);
					if ( pos >= 0 ){
						val += 1;			// count up
						success = setJlsRegVarWithLocal(cmdarg.getStrArg(1), to_string(val), true, flagLocal);
					}
				}
			}
			break;
		case CmdType::OptSet:
		case CmdType::OptDefault:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::OptDefault )? false : true;
				setSystemRegOptions(cmdarg.getStrArg(1), 0, overwrite);
			}
			break;
		case CmdType::UnitSec:					// ���背�W�X�^�ݒ�
			{
				pdata->cnv.changeUnitSec(cmdarg.getValStrArg(1));
				setSystemRegUpdate();
			}
			break;
		case CmdType::ListGetAt:
			setResultRegListGetAt(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListIns:
			setResultRegListIns(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListDel:
			setResultRegListDel(cmdarg, cmdarg.getValStrArg(1));
			break;
		default:
			globalState.addMsgError("error:internal setting(RegCmd)\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Lazy/Memory����R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdMemFlow(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::LazyStart:			// LazyStart��
			{
				LazyType typeLazy = LazyType::LazyE;			// �f�t�H���g��lazy�ݒ�
				if ( cmdarg.tack.typeLazy != LazyType::None ){	// �I�v�V�����w�肪����ΗD��
					typeLazy = cmdarg.tack.typeLazy;
				}
				state.setLazyStartType(typeLazy);		// lazy�ݒ�
			}
			break;
		case CmdType::EndLazy:				// EndLazy��
			state.setLazyStartType(LazyType::None);	// lazy����
			break;
		case CmdType::Memory:				// Memory��
			state.startMemArea(cmdarg.getStrArg(1));
			break;
		case CmdType::EndMemory:			// EndMemory��
			state.endMemArea();
			break;
		default:
			globalState.addMsgError("error:internal setting(LazyCategory)\n");
			success = false;
			break;
	}
	return success;
}

//---------------------------------------------------------------------
// Lazy/Memory���s�R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdMemExe(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	//--- ���s���� ---
	switch( cmdarg.cmdsel ){
		case CmdType::MemCall:			// MemCall��
			state.setMemCall(cmdarg.getStrArg(1));
			break;
		case CmdType::MemErase:			// MemErase��
			state.setMemErase(cmdarg.getStrArg(1));
			break;
		case CmdType::MemCopy:			// MemCopy��
			state.setMemCopy(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::MemMove:			// MemMove��
			state.setMemMove(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::MemAppend:		// MemAppend��
			state.setMemAppend(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::LazyFlush:		// LazyFlush��
			state.setLazyFlush();
			break;
		case CmdType::LazyAuto:			// LazyAuto��
			state.setLazyAuto(true);
			break;
		case CmdType::LazyStInit:		// LazyStInit(Lazy�pAuto�����s��Ԃɖ߂��B�O�̂��ߎc��)
			state.setLazyStateIniAuto(true);
			break;
		case CmdType::MemOnce:			// MemOnce��
			{
				int val = cmdarg.getValStrArg(1);
				bool dupe = ( val != 1 )? true : false;
				state.setMemDupe(dupe);
			}
			break;
		case CmdType::MemEcho:			// MemEcho��
			state.setMemEcho(cmdarg.getStrArg(1));
			break;
		case CmdType::MemDump:			// Debug
			state.setMemGetMapForDebug();
			break;
		default:
			globalState.addMsgError("error:internal setting(MemCategory)\n");
			success = false;
			break;
	}
	return success;
}


//=====================================================================
// �R�}���h���s����
//=====================================================================

//---------------------------------------------------------------------
// �X�N���v�g�e�s�̃R�}���h���s
//---------------------------------------------------------------------
bool JlsScript::exeCmd(JlsCmdSet& cmdset){
	//--- �R�}���h���s�̊m�F�t���O ---
	bool valid_exe = true;				// ����̎��s
	bool exe_command = false;			// ���s���
	//--- �O�R�}���h���s�ς݂��m�F (-else option) ---
	if (cmdset.arg.getOpt(OptType::FlagElse) > 0){
		if ( isSystemRegLastexe() ){	// ���O�R�}���h�����s�����ꍇ
			valid_exe = false;			// ����R�}���h�͎��s���Ȃ���
			exe_command = true;			// ���s�ς݈���
		}
	}
	//--- �O�R�}���h���s�ς݂��m�F (-cont option) ---
	if (cmdset.arg.getOpt(OptType::FlagCont) > 0){
		if ( isSystemRegLastexe() == false ){	// ���O�R�}���h�����s���Ă��Ȃ��ꍇ
			valid_exe = false;			// ����R�}���h�����s���Ȃ�
		}
	}
	//--- �R�}���h���s ---
	if (valid_exe){
		//--- ���ʐݒ� ---
		m_funcLimit->limitCommonRange(cmdset);		// �R�}���h���ʂ͈̔͌���

		//--- �I�v�V������Auto�n�\�����K�v�ȏꍇ(-AC -NoAC)�̍\���쐬 ---
		if ( cmdset.arg.tack.needAuto ){
			exeCmdCallAutoSetup(cmdset);
		}
		//--- ���ޕʂɃR�}���h���s ---
		switch( cmdset.arg.category ){
			case CmdCat::AUTO:
				exe_command = exeCmdCallAutoScript(cmdset);		// Auto�����N���X�Ăяo��
				break;
			case CmdCat::AUTOEACH:
				exe_command = exeCmdAutoEach(cmdset);			// �e���S���Ԃ�Auto�n����
				break;
			case CmdCat::LOGO:
			case CmdCat::AUTOLOGO:
				exe_command = exeCmdLogo(cmdset);				// ���S�ʂɎ��s
				break;
			case CmdCat::NEXT:
				exe_command = exeCmdNextTail(cmdset);			// ���̈ʒu�擾����
				break;
			default:
				break;
		}
	}

	return exe_command;
}

//---------------------------------------------------------------------
// AutoScript�g�������s
//---------------------------------------------------------------------
//--- �R�}���h��͌�̎��s ---
bool JlsScript::exeCmdCallAutoScript(JlsCmdSet& cmdset){
	bool setup_only = false;
	return exeCmdCallAutoMain(cmdset, setup_only);
}
//--- �R�}���h��͂̂� ---
bool JlsScript::exeCmdCallAutoSetup(JlsCmdSet& cmdset){
	bool setup_only = true;
	return exeCmdCallAutoMain(cmdset, setup_only);
}

//--- ���s���C������ ---
bool JlsScript::exeCmdCallAutoMain(JlsCmdSet& cmdset, bool setup_only){
	//--- ����̂ݎ��s ---
	if ( pdata->isAutoModeInitial() ){
		//--- ���S�g�p���x����ݒ� ---
		if (pdata->isExistLogo() == false){		// ���S���Ȃ��ꍇ�̓��S�Ȃ��ɐݒ�
			pdata->setLevelUseLogo(CONFIG_LOGO_LEVEL_UNUSE_ALL);
		}
		else{
			int level = pdata->getConfig(ConfigVarType::LogoLevel);
			if (level <= CONFIG_LOGO_LEVEL_DEFAULT){		// ���ݒ莞�͒l��ݒ�
				level = CONFIG_LOGO_LEVEL_USE_HIGH;
			}
			pdata->setLevelUseLogo(level);
		}
		if (pdata->isUnuseLogo()){				// ���S�g�p���Ȃ��ꍇ
			pdata->extOpt.flagNoLogo = 1;		// ���S�Ȃ��ɐݒ�
			setSystemRegUpdate();				// NOLOGO�X�V
		}
	}
	//--- Auto�R�}���h���s ---
	return m_funcAutoScript->startCmd(cmdset, setup_only);		// AutoScript�N���X�Ăяo��
}

//---------------------------------------------------------------------
// �e���S���Ԃ�͈͂Ƃ��Ď��s����Auto�R�}���h (-autoeach�I�v�V����)
//---------------------------------------------------------------------
bool JlsScript::exeCmdAutoEach(JlsCmdSet& cmdset){
	bool exeflag_total = false;
	NrfCurrent logopt = {};
	while( pdata->getNrfptNext(logopt, LOGO_SELECT_VALID) ){
		RangeMsec rmsec_logo;
		LogoResultType rtype_rise;
		LogoResultType rtype_fall;
		//--- �m���Ԃ��m�F ---
		pdata->getResultLogoAtNrf(rmsec_logo.st, rtype_rise, logopt.nrfRise);
		pdata->getResultLogoAtNrf(rmsec_logo.ed, rtype_fall, logopt.nrfFall);
		//--- �m�莞�ȊO�͌��ꏊ�ɂ��� ---
		if (rtype_rise != LOGO_RESULT_DECIDE){
			rmsec_logo.st = logopt.msecRise;
		}
		if (rtype_fall != LOGO_RESULT_DECIDE){
			rmsec_logo.ed = logopt.msecFall;
		}
		//--- �e���S���Ԃ�͈͂Ƃ��Ĉʒu��ݒ� ---
		m_funcLimit->resizeRangeHeadTail(cmdset, rmsec_logo);
		//--- Auto�R�}���h���s ---
		int exeflag = exeCmdCallAutoScript(cmdset);	// Auto�����N���X�Ăяo��
		//--- ���s���Ă�������s�t���O�ݒ� ---
		if (exeflag){
			exeflag_total = true;
		}
	};
	return exeflag_total;
}

//---------------------------------------------------------------------
// �S���S�̒��őI�����S�����s
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogo(JlsCmdSet& cmdset){
	//--- ���S�ԍ��I�v�V��������L���ȃ��S�ԍ��ʒu�����ׂĎ擾 ---
	int nmax_list = m_funcLimit->limitLogoList(cmdset);
	//--- �P�ӏ������̃R�}���h�� ---
	bool flag_onepoint = cmdset.arg.tack.onePoint;
	//--- �P�ӏ������R�}���h�̏ꍇ�͈�ԋ߂���₾����L���ɂ��� ---
	int nlist_base = -1;
	if (flag_onepoint){
		int difmsec_base = 0;
		for(int i=0; i<nmax_list; i++){
			//--- ��������𖞂����Ă��郍�S���m�F ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			if (exeflag){
				WideMsec wmsec = cmdset.limit.getTargetRangeWide();
				Msec msec_target = wmsec.just;
				Nsc  nsc_sel = cmdset.limit.getResultTargetSel();
				Msec msec_sel = pdata->getMsecScp(nsc_sel);
				Msec difmsec_sel = abs(msec_target - msec_sel);
				if (difmsec_sel < difmsec_base || nlist_base < 0){
					nlist_base = i;
					difmsec_base = difmsec_sel;
				}
			}
		}
	}
	//--- ���X�g�쐬���͊J�n�O�ɓ��e���� ---
	if ( cmdset.arg.cmdsel == CmdType::GetList ||
	     cmdset.arg.cmdsel == CmdType::GetPos ){
		updateResultRegWrite(cmdset.arg);	// �ϐ�($POSHOLD/$LISTHOLD)�N���A
	}
	//--- �e�L�����S�����s ---
	bool exeflag_total = false;
	for(int i=0; i<nmax_list; i++){
		if (flag_onepoint == false || nlist_base == i){
			//--- ��������𖞂����Ă��郍�S���m�F ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			//--- ���s���� ---
			if (exeflag){
				switch(cmdset.arg.category){
					case CmdCat::LOGO :
						//--- ���S�ʒu�𒼐ڐݒ肷��R�}���h ---
						exeflag = exeCmdLogoTarget(cmdset);
						break;
					case CmdCat::AUTOLOGO :
						//--- �����\�����琶������R�}���h ---
						exeflag = exeCmdCallAutoScript(cmdset);
						break;
					default:
						break;
				}
			}
			//--- ���s���Ă�������s�t���O�ݒ� ---
			if (exeflag){
				exeflag_total = true;
			}
		}
	}

	return exeflag_total;
}

//---------------------------------------------------------------------
// ���S�ʒu�ʂ̎��s�R�}���h
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogoTarget(JlsCmdSet& cmdset){
	bool exe_command = false;
	Msec msec_force   = cmdset.limit.getTargetRangeForce();
	Nsc nsc_scpos_sel = cmdset.limit.getResultTargetSel();
	Nsc nsc_scpos_end = cmdset.limit.getResultTargetEnd();
	Nrf nrf = cmdset.limit.getLogoBaseNrf();

	switch(cmdset.arg.cmdsel){
		case CmdType::Find:
			if (nsc_scpos_sel >= 0){
				if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){	// �����\���ɔ��f
					exeCmdCallAutoScript(cmdset);					// Auto�����N���X�Ăяo��
				}
				else{								// �]���\���ɔ��f
					Msec msec_tmp = pdata->getMsecScpEdge(nsc_scpos_sel, jlsd::edgeFromNrf(nrf));
					pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
				}
				exe_command = true;
			}
			break;
		case CmdType::MkLogo:
			if (nsc_scpos_sel >= 0 && nsc_scpos_end >= 0){
				int msec_st, msec_ed;
				if (nsc_scpos_sel < nsc_scpos_end){
					msec_st = pdata->getMsecScp(nsc_scpos_sel);
					msec_ed = pdata->getMsecScpBk(nsc_scpos_end);
				}
				else{
					msec_st = pdata->getMsecScp(nsc_scpos_end);
					msec_ed = pdata->getMsecScpBk(nsc_scpos_sel);
				}
				bool overlap = ( cmdset.arg.getOpt(OptType::FlagOverlap) != 0 )? true : false;
				bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
				bool unit    = ( cmdset.arg.getOpt(OptType::FlagUnit)    != 0 )? true : false;
				int nsc_ins = pdata->insertLogo(msec_st, msec_ed, overlap, confirm, unit);
				exe_command = (nsc_ins >= 0)? true : false;
			}
			break;
		case CmdType::DivLogo:
			if (nsc_scpos_sel >= 0 || msec_force >= 0){
				//--- �Ώۈʒu�擾 ---
				Msec msec_target;
				Msec msec_st, msec_ed;
				if (nsc_scpos_sel >= 0){
					msec_target = pdata->getMsecScp(nsc_scpos_sel);
				}
				else{
					msec_target = msec_force;
				}
				if (jlsd::isLogoEdgeRiseFromNrf(nrf)){		// start edge
					msec_st = pdata->getMsecLogoNrf(nrf);
					msec_ed = pdata->cnv.getMsecAdjustFrmFromMsec(msec_target, -1);
				}
				else{										// end edge
					msec_st = pdata->cnv.getMsecAlignFromMsec(msec_target);
					msec_ed = pdata->getMsecLogoNrf(nrf);
				}
				bool overlap = true;
				bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
				bool unit = true;
				Nsc nsc_ins = pdata->insertLogo(msec_st, msec_ed, overlap, confirm, unit);
				exe_command = (nsc_ins >= 0)? true : false;
			}
			break;
		case CmdType::Select:
			if (nsc_scpos_sel >= 0){
				// �]���̊m��ʒu������
				Nsc nsc_scpos = pdata->sizeDataScp();
				Msec msec_nrf = pdata->getMsecLogoNrf(nrf);
				for(int j=1; j<nsc_scpos - 1; j++){
					Msec msec_j = pdata->getMsecScp(j);
					if (msec_j == msec_nrf){
						if (pdata->getScpStatpos(j) > SCP_PRIOR_NONE){
							pdata->setScpStatpos(j, SCP_PRIOR_NONE);
						}
					}
				}
				// �擪��؂�ʒu�̕ێ�
				if (nrf == 0){
					pdata->recHold.msecSelect1st = msec_nrf;
				}
				// �V�����m��ʒu��ݒ�
				Msec msec_tmp = pdata->getMsecScp(nsc_scpos_sel);
				pdata->setMsecLogoNrf(nrf, msec_tmp);
				pdata->setScpStatpos(nsc_scpos_sel, SCP_PRIOR_DECIDE);
				exe_command = true;
				if (cmdset.arg.getOpt(OptType::FlagConfirm) > 0){
					pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
				}
			}
			break;
		case CmdType::Force:
			{
				WideMsec wmsec = cmdset.limit.getTargetRangeWide();
				Msec msec_tmp = pdata->cnv.getMsecAlignFromMsec( wmsec.just );
				if (msec_tmp >= 0){
					exe_command = true;
					if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){		// �����\���ɔ��f
						exeCmdCallAutoScript(cmdset);						// Auto�����N���X�Ăяo��
					}
					else{
						pdata->setResultLogoAtNrf(msec_tmp, LOGO_RESULT_DECIDE, nrf);
					}
				}
			}
			break;
		case CmdType::Abort:
			exe_command = true;
			pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf);
			if (cmdset.arg.getOpt(OptType::FlagWithN) > 0){
				pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf+1);
			}
			if (cmdset.arg.getOpt(OptType::FlagWithP) > 0){
				pdata->setResultLogoAtNrf(-1, LOGO_RESULT_ABORT, nrf-1);
			}
			break;
		case CmdType::GetPos:
			{
				Msec msecHold;
				exe_command = getMsecTargetWithForce(msecHold, cmdset);
				if ( exe_command ){
					setResultRegPoshold(cmdset.arg, msecHold);		// �ϐ��ɐݒ�
				}
			}
			break;
		case CmdType::GetList:
			{
				Msec msecHold;
				exe_command = getMsecTargetWithForce(msecHold, cmdset);
				if ( exe_command ){
					setResultRegListhold(cmdset.arg, msecHold);	// �ϐ��ɐݒ�
				}
			}
			break;
		case CmdType::DivFile:
			{
				Msec msec = cmdset.limit.getTargetRangeWide().just;
				exe_command = true;
				auto it = std::lower_bound(pdata->divFile.begin(), pdata->divFile.end(), msec);
				if (it == pdata->divFile.end() || *it != msec) {
					pdata->divFile.insert(it, msec);
				}
			}
			break;
		default:
			break;
	}
	return exe_command;
}

//---------------------------------------------------------------------
// ����HEADTIME/TAILTIME���擾
//---------------------------------------------------------------------
bool JlsScript::exeCmdNextTail(JlsCmdSet& cmdset){
	//--- TAILFRAME������HEADFRAME�� ---
	string cstr;
	Msec msec_headframe = -1;
	if (getJlsRegVar(cstr, "TAILTIME", true) > 0){
		pdata->cnv.getStrValMsecM1(msec_headframe, cstr, 0);
	}
	//--- �͈͂��擾 ---
	WideMsec wmsec_target;
	wmsec_target.just  = msec_headframe + cmdset.arg.wmsecDst.just;
	wmsec_target.early = msec_headframe + cmdset.arg.wmsecDst.early;
	wmsec_target.late  = msec_headframe + cmdset.arg.wmsecDst.late;
	bool force = false;
	if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){
		force = true;
	}
	//--- ��ԋ߂��V�[���`�F���W�ʒu���擾 ---
	m_funcLimit->selectTargetByRange(cmdset, wmsec_target, force);

	//--- ���ʂ��i�[ --
	bool exeflag = false;
	Msec msec_tailframe;
	exeflag = getMsecTargetWithForce(msec_tailframe, cmdset);
	if ( exeflag ){
		setSystemRegHeadtail(msec_headframe, msec_tailframe);
		//--- -autochg�I�v�V�����Ή� ---
		if ( cmdset.arg.getOpt(OptType::FlagAutoChg) > 0 ){		// �����\���ɔ��f
			exeCmdCallAutoScript(cmdset);						// Auto�����N���X�Ăяo��
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// cmdset��񂩂�ΏۂƂȂ閳���V�[���`�F���W���擾�A�Ȃ���ΐݒ�ɂ���Ă͋����쐬
// �o��
//  �Ԃ�l     �F���ʎ擾�L��
//  msecResult : �擾���������V�[���`�F���W�ʒu
//---------------------------------------------------------------------
bool JlsScript::getMsecTargetWithForce(Msec& msecResult, JlsCmdSet& cmdset){
	Nsc  nscSel     = cmdset.limit.getResultTargetSel();
	Msec msecForce  = cmdset.limit.getTargetRangeForce();
	LogoEdgeType edgeOut = cmdset.limit.getTargetOutEdge();

	bool success = false;
	if ( nscSel >= 0 ){		// �Ή����閳���V�[���`�F���W�����݂���ꍇ
		msecResult = pdata->getMsecScpEdge(nscSel, edgeOut);
		success = true;
	}
	else if ( msecForce >= 0 ){	// �����V�[���`�F���W�Ȃ�-force/-noforce�I�v�V��������
		msecResult = pdata->cnv.getMsecAlignFromMsec(msecForce);
		RangeMsec rmsec;
		if ( jlsd::isLogoEdgeRise(edgeOut) ){		// start edge
			rmsec.st = msecResult;
			rmsec.ed = pdata->cnv.getMsecAdjustFrmFromMsec(msecResult, -1);
		}else{
			rmsec.st = pdata->cnv.getMsecAdjustFrmFromMsec(msecResult, +1);
			rmsec.ed = msecResult;
		}
		if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){	// -force�I�v�V�����ŋ����쐬��
			pdata->insertScpos(rmsec.st, rmsec.ed, -1, SCP_PRIOR_DECIDE);
		}
		success = true;
	}
	return success;
}

