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
	m_funcDecode.reset(new JlsScriptDecode(pdata, &funcList));
	m_funcLimit.reset(new JlsScriptLimit(pdata));
	// Auto�n�R�}���h���g���g�p
	m_funcAutoScript.reset(new JlsAutoScript(pdata));

	// ���W�X�^�A�N�Z�X�֘A�ݒ� ---
	funcList.setDataPointer(pdata);
	funcReg.setDataPointer(pdata, &globalState, &funcList);

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

	// �N���I�v�V���������� funcReg �Ŏ��{
	return funcReg.setOptionsGetOne(argrest, strv, str1, str2, overwrite);
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
	pdata->cnv.getStrFilePath(strPathOnly, strSrc);	// �p�X�����ۊ�
	//--- ��{�t�@�C�����ݒ� ---
	if ( flagFull ){			// ���C���t�@�C���̃t���p�X���͎��̓p�X�����擾����
		globalState.setFullPathJL(strSrc);
		globalState.setPathNameJL(strPathOnly);
		strFull= strSrc;
		strName = strSrc.substr( strPathOnly.length() );
	}
	else{					// Call�Ńt�@�C���������݂̂̏ꍇ�̓p�X����ǉ�����
		bool needPath = strPathOnly.empty();
		if ( !needPath ){	// �t�H���_�w��Ȃ�or�w��ꏊ�Ƀt�@�C���Ȃ��̎��͕ۊǃp�X����t��
			needPath = !makeFullPathIsExist(strSrc.c_str() );
		}
		if ( needPath ){
			strPathOnly = globalState.getPathNameJL();
			strFull = strPathOnly + strSrc;
			strName = strSrc;
		}else{
			strFull= strSrc;
			strName = strSrc.substr( strPathOnly.length() );
		}
	}
	//--- -sublist�ݒ���擾 ---
	string remain = pdata->extOpt.subList;		// ���O�ݒ�̃T�u�t�H���_�擾
	//--- �T�u�t�H���_�̃p�X�w�肠��ΕύX ---
	if ( !pdata->extOpt.subPath.empty() ){
		strPathOnly = pdata->extOpt.subPath;
		pdata->cnv.getStrFileAllPath(strPathOnly);	// �Ō�ɋ�؂�t��
		if ( flagFull ){	// ���C���t�@�C���̃t���p�X���͎���-sublistz���鎞�t���p�X�w��ꏊ���ŗD��
			remain = "<," + remain;
		}
		remain = remain + ",.";		// �T�u�łȂ��ꏊ���Ō�ɒǉ�
	}else{
		remain = remain + ",";		// ���̃t���p�X�ʒu�����p
	}
	//--- �t���p�X�ʒu���ݒ�Ȃ���Βǉ����鏈�� ---
	{
		auto posCur = remain.find(",<,");
		if ( posCur == string::npos ){
			if ( remain.substr(0,2) != "<," ){
				remain = "<," + remain;				// �t���p�X�ʒu���ŏ��ɒǉ�
			}
		}
	}
	//--- -sublist�ݒ肩��R���}��؂��1���m�F ---
	bool decide = false;
	while( !remain.empty() && !decide ){
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
		string strTry;
		if ( subname == "<" ){		// -incmd�ꏊ
			strTry = strFull;
		}else if ( subname == "." ){	// -sublist�ύX��̏ꏊ
			strTry = strPathOnly + strName;
		}else{
			pdata->cnv.getStrFileAllPath(subname);	// �Ō�ɋ�؂�t��
			strTry = strPathOnly + subname + strName;
		}
		decide = makeFullPathIsExist(strTry);
		if ( decide ){			// �T�u�t�H���_�̃p�X���݂Ō��ʍX�V
			strFull = strTry;
		}
	}
	//--- �t�@�C���p�X�\���p ---
	if ( decide ){
		string mes = "join_logo_scp_Call : " + strFull;
		pdata->dispSysMesN(mes, JlsDataset::SysMesType::CallFile);
	}
	return decide;
}
bool JlsScript::makeFullPathIsExist(const string& str){
	LocalIfs ifs(str.c_str());
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

	//--- ���ʐ擪���s�t�@�C���ݒ� ---
	string nameSetup = pdata->extOpt.setup;
	if ( !nameSetup.empty() ){
		string strPart = nameSetup;
		bool flagFull = false;		// ��ƂȂ�t���p�X�ł͂Ȃ�
		if ( !makeFullPath(nameSetup, strPart, flagFull) ){
			nameSetup.clear();
			globalState.addMsgErrorN("warning: not found setup-file " + strPart);
		}
	}
	//--- �V�X�e���ϐ��̏����l��ݒ� ---
	funcReg.setSystemRegInit();

	//--- ���S���Z�b�g���̃o�b�N�A�b�v�f�[�^�ۑ� ---
	pdata->backupLogosetSave();

	//--- JL�X�N���v�g���s ---
	int errnum = startCmdEnter(nameFullPath, nameSetup);

	//--- �f�o�b�O�p�̕\�� ---
	if (pdata->extOpt.verbose > 0 && errnum == 0){
		pdata->displayLogo();
		pdata->displayScp();
	}

	return errnum;
}


//---------------------------------------------------------------------
// �R�}���h���s�J�n
// ���́F
//   fnameMain    : �X�N���v�g�t�@�C�����i���C���j
//   fnameSetup   : �X�N���v�g�t�@�C�����i���ʐ擪���s�j
// �o�́F
//   �Ԃ�l  �F0=����I�� 2=�t�@�C���ُ�
//---------------------------------------------------------------------
int JlsScript::startCmdEnter(const string& fnameMain, const string& fnameSetup){
	//--- ������s ---
	globalState.setExe1st(true);

	//--- ����M���i�I�[�K�w�j ---
	JlsScriptState stateEnd(&globalState);

	//--- ���ʐ擪���s�t�@�C���ǂݍ��� ---
	if ( !fnameSetup.empty() ){
		startCmdLoop(fnameSetup, 0);
	}
	globalState.checkMsgError(true);

	//--- ���C���t�@�C���ǂݍ��� ---
	int errnum = startCmdLoop(fnameMain, 0);

	//--- ��ԍŌ�Ɏ��s�ݒ肳�ꂽ�R�}���h�����s ---
	startCmdLoopLazyEnd(stateEnd);
	startCmdLoopLazyOut(stateEnd, "FINALIZE");		// �ŏI����
	funcReg.setOutDirect();							// Trim���ڏo�͑Ή�
	startCmdLoopLazyOut(stateEnd, "OUTPUT");		// �o�͗p���Ō�Ɏ��s
	globalState.checkMemUnused();		// ���g�pMemSet�m�F
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
	funcReg.setSystemRegLastexe(false);

	//--- Fcall�ɂ��Ăяo���Ή� ---
	bool byFcall = false;
	if ( isFcallName(fname) ){	// Fcall�ɂ��Ăяo��
		byFcall = true;
	}
	//--- �t�@�C���ǂݍ��� ---
	LocalIfs ifs;
	if ( !byFcall ){	// Fcall�ȊO�̓t�@�C������
		ifs.open(fname.c_str());
		if ( !ifs.is_open() ){
			globalState.addMsgErrorN("error: failed to open " + fname);
			return 2;
		}
	}
	//--- ����M�� ---
	JlsScriptState state(&globalState);
	//--- ���[�J���ϐ��K�w�쐬 ---
	int numLayerStart = globalState.setLocalRegCreateCall();	// �ŏ�ʊK�w�����ō쐬
	//--- Fcall���̃R�}���h�ݒ� ---
	if ( byFcall ){
		state.setMemCall( getFcallName(fname) );
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
	int numLayerEnd = globalState.setLocalRegReleaseAny();
	//--- �l�X�g�G���[�m�F ---
	if ( forceExit == false ){
		bool flagErr = false;
		if ( numLayerStart != numLayerEnd ){
			globalState.addMsgError("error: { and } are not matched");
			flagErr = true;
		}
		int flags_remain = state.isRemainNest();
		if (flags_remain & 0x01){
			globalState.addMsgError("error: EndIf is not found");
			flagErr = true;
		}
		if (flags_remain & 0x02){
			globalState.addMsgError("error: EndRepeat is not found");
			flagErr = true;
		}
		if ( state.isLazyArea() ){
			globalState.addMsgError("error: EndLazy is not found");
			flagErr = true;
		}
		if ( state.isMemArea() ){
			globalState.addMsgError("error: EndMemory or EndFunction is not found");
			flagErr = true;
		}
		if ( flagErr ){
			globalState.addMsgErrorN("in " + fname);
		}
	}
	//--- �ϐ��K�w�̉���s���͌��ɖ߂� ---
	if ( numLayerStart < numLayerEnd ){
		for(int i=0; i < (numLayerEnd - numLayerStart); i++){
			globalState.setLocalRegReleaseAny();
		}
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
//		if ( state.isCmdReturnExit() ){	// Exit�Ŕ��������ɒx�����s�̎c������s���邩�Ō��߂�
//			break;
//		}
		startCmdLoopSub(state, strBufOrg, 0);
	};
}
//---------------------------------------------------------------------
// �o�͗p��memory���ʎqOUTPUT�̃R�}���h�����s
//---------------------------------------------------------------------
void JlsScript::startCmdLoopLazyOut(JlsScriptState& state, const string& name){
	//--- �Ō�Ɏ��s���郁���������s�L���b�V���ɐݒ� ---
	state.setMemCall(name);

	//--- lazy���s�L���b�V������ǂݏo�����s ---
	string strBufOrg;
	while( startCmdGetLineOnlyCache(strBufOrg, state) ){
//		if ( state.isCmdReturnExit() ){	// Exit�Ŕ���������FINALIZE/OUTPUT�����s���邩�Ō��߂�
//			break;
//		}
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
	bool exe_command = funcReg.isSystemRegLastexe();
	//--- �ϐ���u�� ---
	string strBuf;
	funcReg.replaceBufVar(strBuf, strBufOrg);

	//--- �f�R�[�h�����i�R�}���h�����j ---
	JlsCmdSet cmdset;									// �R�}���h�i�[
	bool onlyCmd = true;
	CmdErrType errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, onlyCmd);	// �R�}���h�̂݉��

	//--- �f�R�[�h�����i�S�́j ---
	bool fullDecode = false;
	if ( errval == CmdErrType::None ){
		if ( state.isNeedRaw(cmdset.arg.category) ){	// LazyStart��Memory��Ԓ��͕ϐ���W�J���Ȃ�
			strBuf = strBufOrg;
		}
		errval = m_funcDecode->decodeCmd(cmdset.arg, strBuf, false);	// �R�}���h�S�̉��
		//--- �X�L�b�v�s�̃G���[���� ---
		fullDecode = state.isNeedFullDecode(cmdset.arg.cmdsel, cmdset.arg.category);
		if ( !fullDecode ){
			globalState.clearRegError();			// �R�}���h�݂̂̃P�[�X�͕ϐ��W�J���̃G���[����
			if ( errval != CmdErrType::ErrCmd ){	// �R�}���h�F���ȊO�̉�̓G���[������
				errval = CmdErrType::None;
			}
		}
		//--- �R�}���h�l�X�g(End*�ɂ��I���܂�)�A; ��END��ޔ��ʑΉ� ---
		state.addNestInfoForEnd(cmdset.arg.cmdsel, cmdset.arg.category);
	}
	//--- �x�������A�R�}���h��͌�̕ϐ��W�J ---
	bool enable_exe = false;
	if ( errval == CmdErrType::None ){
		//--- �x�����s�����i���ݎ��s���Ȃ��ꍇ��false��Ԃ��j ---
		enable_exe = setStateMem(state, cmdset.arg, strBuf);

		//--- �ϐ��W�J�iIF�����莮�����A�R�}���h���g�p�ϐ��擾�j ---
		if ( enable_exe ){
			bool success = true;
			if ( fullDecode ){
				success = expandDecodeCmd(state, cmdset.arg, strBuf);
			}
			//--- �ϐ��W�J�̃G���[������ ---
			if ( success == false ){
				//enable_exe = false;		// ��������G���[�ŃR�}���h���̂𒆎~���邩�ǂ���
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- �R�}���h���s���� ---
	bool update_exe = false;
	if (enable_exe){
		bool success = true;
		switch( cmdset.arg.category ){
			case CmdCat::NONE:						// �R�}���h�Ȃ�
				break;
			case CmdCat::COND:						// ��������
				success = setCmdCondIf(cmdset.arg, state);
				break;
			case CmdCat::CALL:						// Call��
				success = setCmdCall(cmdset.arg, state, loop);
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
			case CmdCat::READ:						// Read�R�}���h
				success = setCmdRead(cmdset.arg);
				break;
			case CmdCat::REG:						// �ϐ��ݒ�
				success = setCmdReg(cmdset.arg, state);
				break;
			case CmdCat::LAZYF:						// Lazy����
			case CmdCat::MEMF:						// Memory����
			case CmdCat::MEMLAZYF:					// Memory/Lazy���ʐ���
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
						funcReg.setSystemRegNologo(true);
					}
				}
				exe_command = exeCmd(cmdset);
				update_exe  = true;
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
		string mesB = ":" + strBuf;
		if ( pdata->extOpt.vLine == 2 ){
			string mes = to_string(enable_exe) + to_string(update_exe) + to_string(exe_command) + mesB;
			lcout << mes << endl;
		}else{
			string mes = to_string(enable_exe) + to_string(exe_command) + mesB;
			lcout << mes << endl;
		}
	}

	//--- �O�R�}���h�̎��s�L������ ---
	funcReg.setSystemRegLastexe(exe_command);
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
bool JlsScript::startCmdGetLine(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state){
	bool flagRead = false;

	//--- cache����̓ǂݍ��� ---
	flagRead = startCmdGetLineOnlyCache(strBufOrg, state);

	if ( flagRead == false ){
		if ( ifs.is_open() ){
			//--- �t�@�C������̓ǂݍ��� ---
			if ( startCmdGetLineFromFile(ifs, strBufOrg, state) ){
				flagRead = true;
				//--- Repeat�p�L���b�V���ɕۑ� ---
				state.addCmdCache(strBufOrg);
			}
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
	if ( flagRead ){
		globalState.setMsgBufForErr(strBufOrg);		// �o�b�t�@���G���[�\���p�ɕۊ�
	}
	return flagRead;
}
//---------------------------------------------------------------------
// �t�@�C������P�s�ǂݍ��݁i \�ɂ��s�p���ɑΉ��j
//---------------------------------------------------------------------
bool JlsScript::startCmdGetLineFromFile(LocalIfs& ifs, string& strBufOrg, JlsScriptState& state){
	strBufOrg = "";
	bool success = false;
	//--- �R�}���h��������̓ǂݍ��݁A�Ȃ���Βʏ�t�@�C���ǂݍ��� ---
	if ( startCmdGetLineFromFileDivCache(strBufOrg, state) ){
		success = true;
	}else{
		bool cont = true;
		while( cont ){
			cont = false;
			string buf;
			if ( ifs.getline(buf) ){
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
		}
		if ( success ){
			globalState.setMsgBufForErr(strBufOrg);		// �����O�o�b�t�@���G���[�\���p�ɕۊ�
		}
	}
	if ( success ){
		startCmdGetLineFromFileParseDiv(strBufOrg, state);		// �R�}���h��������ΐݒ�
	}
	return success;
}
//--- {}�ɂ��s�����������i���o���j ---
bool JlsScript::startCmdGetLineFromFileDivCache(string& strBufOrg, JlsScriptState& state){
	string bufHold;
	if ( !state.popBufDivCmd(bufHold) ){		// �ʏ�̕����ێ����Ă��Ȃ����
		return false;
	}
	strBufOrg = bufHold;
	return true;
}
//--- {}�ɂ��s�����������i�ۊǁj ---
bool JlsScript::startCmdGetLineFromFileParseDiv(string& strBufOrg, JlsScriptState& state){
	//--- �R�����g���� ---
	string strBuf;
	pdata->cnv.getStrWithoutComment(strBuf, strBufOrg);
	//--- ������������ ---
	int divDet = -1;
	auto braceS = strBuf.find("{");
	auto braceE = strBuf.find("}");
	if ( braceS == string::npos && braceE == string::npos ){
		return false;
	}
	int braceB = (int)braceE;		// �擪�̏o���ʒu
	if ( braceE == string::npos || (braceS != string::npos && braceS < braceE) ){
		braceB = (int)braceS;
	}
	//--- �����ʒu�̎擾 ---
	string strItem;
	int pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, 0);
	if ( pos < 0 ) return false;
	if ( braceB < pos ){		// �擪�R�}���h���ɏo��
		if ( braceB > 0 ){
			string strP = strBuf.substr(0, braceB);
			string strTmp;
			if ( pdata->cnv.getStrItemWithQuote(strTmp, strP, 0) >= 0 ){
				divDet = braceB;	// brace�O�ɕ����񂪂��蕪��
			}
		}
		if ( divDet < 0 ){
			string strN = strBuf.substr(braceB+1);
			string strTmp;
			if ( pdata->cnv.getStrItemWithQuote(strTmp, strN, 0) >= 0 ){
				divDet = braceB+1;	// brace��ɕ����񂪂��蕪��
			}
		}
		if ( divDet < 0 ){			// �O��ɕ�����Ȃ���Ε������Ȃ�
			return false;
		}
	}else{		// 2�Ԗڈȍ~�̃R�}���h�ɏo��
		bool chkFunc = true;
		while( pos >= 0 && divDet < 0 ){
			int posBak = pos;
			if ( chkFunc ){		// function�\���̉\������
				chkFunc = pdata->cnv.isStrFuncModule(strBuf, pos);
			}
			if ( chkFunc ){		// function�\��
				vector<string> listMod;
				pos = pdata->cnv.getListModuleArg(listMod, strBuf, pos);
				strItem = "";
				chkFunc = false;
			}else{
				pos = pdata->cnv.getStrItemWithQuote(strItem, strBuf, pos);
			}
			if ( pos >= 0 ){
				if ( strItem == "{" || strItem == "}" || strItem == "};" ){
					divDet = posBak;
				}
			}
		}
	}
	if ( divDet < 0 ) return false;
	//--- �����ݒ菈�� ---
	state.pushBufDivCmd( strBuf.substr(divDet) );
	strBufOrg = strBuf.substr(0, divDet);
	return true;
}
//---------------------------------------------------------------------
// �G���[�\��
//---------------------------------------------------------------------
void JlsScript::startCmdDispErr(const string& strBuf, CmdErrType errval){
	if ( errval != CmdErrType::None ){
		string strErr = "";
		bool flagAdd = false;
		switch(errval){
			case CmdErrType::ErrOpt:
				strErr = "error: wrong argument";
				flagAdd = true;
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
				flagAdd = true;
				break;
			default:
				break;
		}
		if ( strErr.empty() == false ){
			string mesErr = strErr;
			if ( flagAdd ){
				mesErr += "(";
				mesErr += m_funcDecode->getErrItem();
				mesErr += ")";
			}
			globalState.addMsgErrorN(mesErr + " in " + strBuf);
			string strBufLine = globalState.getMsgBufForErr();
			if ( strBuf != strBufLine ){
				globalState.addMsgErrorN("  (read data)" + strBufLine);
			}
		}
	}
	globalState.checkErrorGlobalState(true);
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
		if ( state.isFlowMem(cmdarg.category) == false ){	// Memory����͏���
			enable_exe = state.setMemStore(state.getMemName(), strBuf);
		}
	}
	//--- Memory���Ԓ��ȊO��Lazy�����m�F ---
	else{
		if ( state.isFlowLazy(cmdarg.category) == false ){	// Lazy����͏���
			enable_exe = setStateMemLazy(state, cmdarg, strBuf);
		}
	}
	//--- ���[�J���ϐ������}���̏��� ---
	state.exeArgMstoreInsert(cmdarg.cmdsel);		// �����ϐ��}�����f���s

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
		if ( state.isLazyArea() == false ){		// LazyStart�ɂ���ԓ��̕␳�͎��s���s�����ߏ���
			setStateMemLazyRevise(typeLazy, state, cmdarg);	// ���R�}���h�̕K�v�ȏ��
		}
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
// �o�́F
//   �Ԃ�l  : ����
//   flagCond  �F�t���O����i0=false  1=true�j
//---------------------------------------------------------------------
bool JlsScript::getCondFlag(bool& flagCond, const string& strBuf){
	string strItem;
	string strCalc = "";
	//--- �R�����g�J�b�g ---
	string strBufRev;
	pdata->cnv.getStrWithoutComment(strBufRev, strBuf);
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
		globalState.addMsgErrorN("error: can not evaluate(" + strCalc + ")");
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
		bool match = funcReg.getJlsRegVarNormal(strVal, strRemain);
		if (match && strVal != "0"){	// �ϐ������݂���0�ȊO�̏ꍇ
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
	funcList.revListStrEmpty(strDefaultList);		// �󃊃X�g�ݒ�
	//--- �ϐ��l�擾�ݒ� ---
	{
		string strVal;
		//--- POSHOLD�̒l��ݒ� ---
		if ( funcReg.getJlsRegVarNormal(strVal, strRegPos) ){	// �ϐ��擾
			cmdarg.setStrOpt(OptType::StrValPosR, strVal);	// �ύX
			cmdarg.setStrOpt(OptType::StrValPosW, strVal);	// �ύX
			cmdarg.clearStrOptUpdate(OptType::StrValPosW);	// ���W�X�^�X�V�s�v
		}
		//--- -val�I�v�V������`����POSHOLD�ǂݏo���l�ύX ---
		if ( cmdarg.isSetStrOpt(OptType::StrArgVal) ){
			strVal = cmdarg.getStrOpt(OptType::StrArgVal);
			cmdarg.setStrOpt(OptType::StrValPosR, strVal);	// �ύX
		}

		//--- LISTHOLD�̒l��ݒ� ---
		if ( funcReg.getJlsRegVarNormal(strVal, strRegList) ){	// �ϐ��擾
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
		case CmdType::ReadData:
		case CmdType::ReadTrim:
		case CmdType::ReadString:
			{
				cmdarg.setStrOpt(OptType::StrValListW, strDefaultList);	// �ύX
			}
			break;
		default:
			break;
	}
}


//=====================================================================
// ���S���ڐݒ�
//=====================================================================

//---------------------------------------------------------------------
// ���X�g�ϐ����烍�S�ʒu�𒼐ڐݒ�
//---------------------------------------------------------------------
void JlsScript::setLogoDirect(JlsCmdArg& cmdarg){
	//--- ���W�X�^�̌��ݒl���擾 ---
	string strList   = cmdarg.getStrOpt(OptType::StrValListR);
	setLogoDirectString(strList);
}
void JlsScript::setLogoDirectString(const string& strList){
	//--- ���X�g���ڐ����擾 ---
	int numList = funcList.getListStrSize(strList);

	//--- ����LogoOff���ݒ肳��Ă����ꍇ�͉��� ---
	if ( pdata->extOpt.flagNoLogo ){
		pdata->extOpt.flagNoLogo = 0;	// ���S�͓ǂݍ��݂ɐݒ�
		funcReg.setSystemRegUpdate();
	}

	//--- ���S�f�[�^���폜�X�V ---
	pdata->clearDataLogoAll();		// ���݂̃��S�͍폜
	pdata->extOpt.flagDirect = 1;	// ���S���ڐݒ�l
	pdata->extOpt.fixFDirect = 1;	// ���S���ڐݒ�ς�
	pdata->extOpt.nLgExact = 3;		// ���S�ʒu�͐��m�Ƃ��Ĉ���

	//--- ���S�f�[�^�ݒ� ---
	Msec msecLast = 0;
	bool flagFall = false;
	for(int i=0; i<numList; i++){
		Msec msecCur;
		bool flagValid = false;
		{
			string strCur;
			if ( funcList.getListStrElement(strCur, strList, i+1) ){
				if ( pdata->cnv.getStrValMsec(msecCur, strCur, 0) >= 0 ){
					flagValid = true;
				}
			}
		}
		if ( flagValid && (msecLast <= msecCur) ){
			if ( (flagFall == false) && (i%2 == 0) ){
				msecLast = msecCur;
				flagFall = true;
			}else if ( flagFall && (i%2 != 0) ){
				struct DataLogoRecord dtlogo;
				pdata->clearRecordLogo(dtlogo);
				Msec msecRise = msecLast;
				Msec msecFall = msecCur;
				dtlogo.rise   = msecRise;
				dtlogo.fall   = msecFall;
				dtlogo.rise_l = msecRise;
				dtlogo.rise_r = msecRise;
				dtlogo.fall_l = msecFall;
				dtlogo.fall_r = msecFall;
				dtlogo.org_rise   = msecRise;
				dtlogo.org_fall   = msecFall;
				dtlogo.org_rise_l = msecRise;
				dtlogo.org_rise_r = msecRise;
				dtlogo.org_fall_l = msecFall;
				dtlogo.org_fall_r = msecFall;
				dtlogo.unit_rise  = LOGO_UNIT_DIVIDE;	// Trim�o�͂̓��S�ʂɕ���
				dtlogo.unit_fall  = LOGO_UNIT_DIVIDE;	// Trim�o�͂̓��S�ʂɕ���
				pdata->pushRecordLogo(dtlogo);			// add data
				msecLast = msecCur;
				flagFall = false;
			}
		}
	}
}

//---------------------------------------------------------------------
// ���S��Ԃ��J�n���ɖ߂��Đݒ�
//---------------------------------------------------------------------
void JlsScript::setLogoReset(){
	pdata->backupLogosetLoad();		// ���S��ۑ���Ԃɖ߂�
	globalState.setExe1st(true);	// �������s�O�ɏ�Ԃ�߂�
}


//=====================================================================
// Call��Memory�p�����ǉ�����
//=====================================================================

//---------------------------------------------------------------------
// �����ݒ�̈�(ArgBegin - ArgEnd)�ŕϐ��ݒ肵�����̏���
//---------------------------------------------------------------------
bool JlsScript::setArgAreaDefault(JlsCmdArg& cmdarg, JlsScriptState& state){
	//--- Default�R�}���h�ȊO�͖��֌W ---
	if ( cmdarg.cmdsel != CmdType::Default ){
		return false;
	}
	//--- �����ݒ�̈�ȊO�͖��֌W ---
	if ( state.isArgAreaEnter() == false ){
		return false;
	}
	//--- �����Ƃ��Đݒ肷��ϐ��̏��� ---
	string strName = cmdarg.getStrArg(1);
	string strVal;
	{	// Default�l�̓ǂݍ���
		if ( !funcReg.getJlsRegVarNormal(strVal, strName) ){
			strVal  = cmdarg.getStrArg(2);
		}
	}
	bool overwrite = true;		// Default�ł��l��ǂݍ��񂾏�ōĐݒ�
	bool flagLocal = true;		// ���[�J���ϐ��Ƃ��Đݒ�
	bool success = funcReg.setJlsRegVarWithLocal(strName, strVal, overwrite, flagLocal);
	if ( success ){
		state.addArgAreaName(strName);	// �ϐ����������Ƃ��ċL��
	}
	return success;
}
//---------------------------------------------------------------------
// �ۊǌ^������ݒ肷��R�}���h�̏����iMemSet/LazyStart/Memory�R�}���h�j
//---------------------------------------------------------------------
//--- �ݒ莞�̕ϐ����i�[���邽�߂̈����ݒ� ---
bool JlsScript::makeArgMemStore(JlsCmdArg& cmdarg, JlsScriptState& state){
	state.clearArgMstoreBuf();				// �������N���A
	bool success1 = makeArgMemStoreByDefault(state);
	bool success2 = makeArgMemStoreByMemSet(cmdarg, state);
	return ( success1 && success2 );
}
//--- �i�[�R�}���h������ ---
void JlsScript::makeArgMemStoreLocalSet(JlsScriptState& state, const string& strName, const string& strVal){
	string strBuf = "LocalSet " + strName + " " + strVal;
	state.addArgMstoreBuf(strBuf);
}
//--- �����ݒ�̈�(ArgBegin - ArgEnd)�Őݒ肳�ꂽ�ϐ��̌��ݒl���i�[ ---
bool JlsScript::makeArgMemStoreByDefault(JlsScriptState& state){
	//--- �����ۊǂ�����Βx�����s�o�b�t�@�Ɍ��݂̕ϐ��ݒ��ǉ����� ---
	int numArg = state.sizeArgAreaNameList();
	if ( numArg <= 0 ){
		return true;
	}
	if ( state.isMemExe() ){	// ����������̎��s���͑ΏۊO
		return true;
	}
	//--- �����̐������}�� ---
	bool success = true;
	for(int i=0; i<numArg; i++){
		string strName;
		bool cont = state.getArgAreaName(strName, i);	// �ϐ���
		if ( cont ){
			string strVal;
			if ( funcReg.getJlsRegVarNormal(strVal, strName) ){	// �ϐ��l
				makeArgMemStoreLocalSet(state, strName, strVal);
			}else{
				success = false;
			}
		}
	}
	if ( !success ){
		globalState.addMsgErrorN("error: not defined variable by ArgBegin");
	}
	return success;
}
//--- MemSet�R�}���h�̈����ϐ��l���i�[ ---
bool JlsScript::makeArgMemStoreByMemSet(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;
	if ( !cmdarg.isSetStrOpt(OptType::ListArgVar) ){	// -arg ���Ȃ���Βǉ����Ȃ�
		return success;
	}
	string strArgList = cmdarg.getStrOpt(OptType::ListArgVar);
	int pos = 0;
	while( pos >= 0 && success ){
		string strName;
		pos = pdata->cnv.getStrWord(strName, strArgList, pos);
		if ( pos > 0 ){
			string strVal;
			if ( funcReg.getJlsRegVarNormal(strVal, strName) ){	// �ϐ��l
				makeArgMemStoreLocalSet(state, strName, strVal);
			}else{
				globalState.addMsgErrorN("error: not defined variable by -arg.");
				success = false;
			}
		}
	}
	return success;
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
					globalState.addMsgErrorN("error: too many EndIf.");
					success = false;
				}
			}
			break;
		case CmdType::Else:						// Else��
		case CmdType::ElsIf:					// ElsIf��
			{
				int errnum = state.ifElse(flagCond);
				if (errnum > 0){
					globalState.addMsgErrorN("error: not exist 'If' but exist 'Else/ElsIf' .");
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
bool JlsScript::setCmdCall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop){
	bool success = true;

	switch( cmdarg.cmdsel ){
		case CmdType::Call:
			success = taskCmdCall(cmdarg.getStrArg(1), loop, false);	// fcall=false
			break;
		case CmdType::Fcall:
			success = taskCmdFcall(cmdarg, state, loop);
		default:
			break;
	}
	return success;
}
//--- Call/Fcall���� ---
bool JlsScript::taskCmdCall(string strName, int loop, bool fcall){
	string strFileName;
	if ( fcall ){	// Fcall�ɂ��Ăяo��
		strFileName = setFcallName(strName);	// Call�Ƌ��ʉ��̂���Fcall�p�F���q�t��
	}
	else{			// �ʏ��Call�Ăяo��
		//--- Call���ߗp��Path�ݒ�A�ǂݍ��݃t�@�C�����ݒ� ---
		bool flagFull = false;		// ���͂̓t�@�C���������̂�
		makeFullPath(strFileName, strName, flagFull);
	}
	//--- Call���s���� ---
	loop ++;
	if (loop < SIZE_CALL_LOOP){				// �ċA�Ăяo���͉񐔐���
		startCmdLoop(strFileName, loop);
	}
	else{
		// �����Ăяo���ɂ��o�b�t�@�I�[�o�[�t���[�h�~�̂���
		globalState.addMsgErrorN("error: many recursive call");
		return false;
	}
	return true;
}
//--- Fcall���� ---
bool JlsScript::taskCmdFcall(JlsCmdArg& cmdarg, JlsScriptState& state, int loop){
	bool success = true;

	//--- �������擾���āA�������������m�F ---
	vector<string> listMod;
	vector<string> listMem;
	int sizeMod = cmdarg.getListStrArgs(listMod);	// �R�}���h�����擾
	int sizeMem = -1;
	if ( sizeMod <= 0 ){
		success = false;
	}else if ( state.getMemDefArg(listMem, listMod[0]) ){	// Memory�ێ����e�擾
		sizeMem = (int) listMem.size();
	}
	if ( sizeMod != sizeMem ){		// �����̐����s��v
		success = false;
	}
	string msgErrItem;
	if ( !success ){
		msgErrItem = "(argnum) " + to_string(sizeMod-2) + " defined: " + to_string(sizeMem-2);
	}
	//--- �֐�������̐ݒ� ---
	string strFuncName;	// �֐���
	if ( success ){
		strFuncName = listMod[0];
		funcReg.setArgFuncName(strFuncName);	// �֐�����Ԃ�l�Ɋm�ۂ���ݒ�
		if ( cmdarg.isSetStrOpt(OptType::StrRegOut) ){		// �Ԃ�l�ݒ�
			string strOut = cmdarg.getStrOpt(OptType::StrRegOut);
			bool flagLocal = cmdarg.getOptFlag(OptType::FlagLocal);
			bool overwrite = true;
			funcReg.setJlsRegVarWithLocal(strOut, "", overwrite, flagLocal);	// �ϐ�������
			funcReg.setArgRefReg(strFuncName, strOut);
		}
	}
	//--- �e�����̐ݒ� ---
	if ( success && sizeMod >= 3 ){			// �Ō�̈����͋��`�̂���2�ȉ��͈����ǉ��Ȃ�
		for(int i=1; i<sizeMod-1; i++){		// �ŏ��̃��W���[�����ƍŌ�̋��`�͏���
			if ( success ){
				string strName = listMem[i];	// ����������Function����
				string strVal  = listMod[i];	// �Ăяo�����̈���
				if ( strName.substr(0, 4) == "ref(" ){		// �Q�Ɠn��
					if ( strName.substr(strName.length()-1) == ")" ){
						strName = strName.substr(4, strName.length()-5);
						success = funcReg.setArgRefReg(strName, strVal);	// �Q�Ɠn��
					}else{
						success = false;
						msgErrItem += " ref:" + strVal + " ";
					}
				}else{
					success = funcReg.setArgRegByBoth(strName, strVal, true); // quote�W�J=true
				}
			}
		}
	}
	//--- ���s���� ---
	if ( success ){
		success = taskCmdCall(strFuncName, loop, true);	// fcall=true
	}else{
		string msg = "error: not match argument for function call.  ";
		msg += msgErrItem;
		globalState.addMsgErrorN(msg);
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
					globalState.addMsgErrorN("error: overflow at Repeat");
					success = false;
				}
				else if ( cmdarg.isSetStrOpt(OptType::StrCounter) ){	// -counter
					string strName = cmdarg.getStrOpt(OptType::StrCounter);
					int valI = cmdarg.getOpt(OptType::NumCounterI);
					int valS = cmdarg.getOpt(OptType::NumCounterS);
					state.repeatVarSet(strName, valS);
					funcReg.setJlsRegVarLocal(strName, to_string(valI), true);	// overwrite
				}
			}
			break;
		case CmdType::EndRepeat:			// EndRepeat��
			{
				{
					string strName;
					int valS;
					if ( state.repeatVarGet(strName, valS) ){	// -counter�X�V
						funcReg.setJlsRegVarCountUp(strName, valS, true);	// local=true
					}
				}
				int errum = state.repeatEnd();
				if (errum > 0){
					globalState.addMsgErrorN("error: Repeat - EndRepeat not match");
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
		case CmdType::Break:
			if ( cmdarg.getCondFlag() ){
				state.setBreak();
			}
			break;
		case CmdType::LocalSt:
			globalState.setLocalRegCreateOne();
			break;
		case CmdType::LocalEd:
			globalState.setLocalRegReleaseOne();
			break;
		case CmdType::ArgBegin:
			state.setArgAreaEnter(true);
			if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -local�I�v�V����
				globalState.setLocalOnly(true);
			}
			break;
		case CmdType::ArgEnd:
			state.setArgAreaEnter(false);
			globalState.setLocalOnly(false);
			break;
		case CmdType::Exit:
			globalState.setCmdExit(true);
			break;
		case CmdType::Return:
			state.setCmdReturn(true);
			break;
		case CmdType::EndMulti:	// �������ʎ��s
			globalState.addMsgErrorN("error:not match End* for ;");
			success = false;
			break;
		default:
			globalState.addMsgErrorN("error:internal setting");
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
		case CmdType::Mkdir:
			{
				string strName = cmdarg.getStrArg(1);
				if ( !LSys.cmdMkdir(strName) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error: failed mkdir " + strName);
					}
				}
			}
			break;
		case CmdType::FileCopy:
			{
				string strFrom = cmdarg.getStrArg(1);
				string strTo   = cmdarg.getStrArg(2);
				if ( !LSys.cmdCopy(strFrom, strTo) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error: failed copy from:" + strFrom + " to:" + strTo);
					}
				}
			}
			break;
		case CmdType::FileOpen:
		case CmdType::FileAppend:
			{
				if ( cmdarg.isSetStrOpt(OptType::StrFileCode) ){	// �����R�[�h�ݒ�
					string strCode = cmdarg.getStrOpt(OptType::StrFileCode);
					if ( !globalState.fileSetCodeNum(strCode) ){
						globalState.addMsgErrorN("error : -filecode(" + strCode + ")");
					}
				}
				string strName = cmdarg.getStrArg(1);
				bool append = ( cmdarg.cmdsel == CmdType::FileAppend );
				if ( !globalState.fileOpen(strName, append) ){
					if ( !cmdarg.getOptFlag(OptType::FlagSilent) ){
						globalState.addMsgErrorN("error : file open(" + strName + ")");
					}
				}
				funcReg.setSystemRegFileOpen();
			}
			break;
		case CmdType::FileClose:
			globalState.fileClose();
			funcReg.setSystemRegFileOpen();
			break;
		case CmdType::FileCode:
			{
				string strCode = cmdarg.getStrArg(1);
				if ( !globalState.fileSetCodeDefault(strCode) ){
					globalState.addMsgErrorN("error : FileCode(" + strCode + ")");
				}
			}
			break;
		case CmdType::FileToMemo:
			{
				int val = cmdarg.getValStrArg(1);
				globalState.fileMemoOnly( (val != 0) );
			}
			break;
		case CmdType::Echo:
			globalState.fileOutput(cmdarg.getStrArg(1) + "\n");
			break;
		case CmdType::EchoItem:
		case CmdType::EchoItemQ:
			{
				string strVal = cmdarg.getStrArg(1);
				if ( cmdarg.getOptFlag(OptType::FlagRestore) ){	// -restore�I�v�V����
					funcReg.restoreStrQuote(strVal);
				}
				if ( !cmdarg.getOptFlag(OptType::FlagMerge) ){
					strVal += "\n";
				}
				globalState.fileOutput(strVal);
			}
			break;
		case CmdType::EchoFile:
			setCmdEchoTextFile(cmdarg.getStrArg(1));
			break;
		case CmdType::EchoOavs:
			setCmdEchoResultTrim();
			break;
		case CmdType::EchoOscp:
			setCmdEchoResultDetail();
			break;
		case CmdType::EchoMemo:
			globalState.fileMemoFlush();
			break;
		case CmdType::LogoOff:
			funcReg.setSystemRegNologo(false);
			break;
		case CmdType::OldAdjust:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.oldAdjust = val;
			}
			break;
		case CmdType::IgnoreCase:
			{
				int val = cmdarg.getValStrArg(1);
				globalState.setIgnoreCase( (val != 0) );
			}
			break;
		case CmdType::SysMesDisp:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.dispSysMes = val;
			}
			break;
		case CmdType::SysMemoSel:
			{
				int val = cmdarg.getValStrArg(1);
				LSys.setMemoSel(val);
			}
			break;
		case CmdType::SysDataGet:
			funcReg.getSystemData(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::LogoDirect:
			setLogoDirect(cmdarg);
			break;
		case CmdType::LogoExact:
			{
				int val = cmdarg.getValStrArg(1);
				pdata->extOpt.nLgExact = val;
				pdata->extOpt.fixNLgExact = 1;
			}
			break;
		case CmdType::LogoReset:
			setLogoReset();
			break;
		default:
			globalState.addMsgErrorN("error:internal setting");
			success = false;
			break;
	}
	return success;
}
//--- �e�L�X�g�t�@�C�����e�����̂܂܏o�� ---
void JlsScript::setCmdEchoTextFile(const string& fname){
	LocalIfs ifs(fname.c_str());
	if ( !ifs.is_open() ){
		globalState.addMsgErrorN("error:not exist the filename:" + fname);
		return;
	}
	string strBuf;
	while( ifs.getline(strBuf) ){
		globalState.fileOutput(strBuf + "\n");
	}
}
//--- Trim�o�́iJlsIf��outputResultTrim�̓��e�j---
void JlsScript::setCmdEchoResultTrim(){
	//--- ���ʍ쐬 ---
	pdata->outputResultTrimGen();
	string strBuf;
	int num_data = (int) pdata->resultTrim.size();
	for(int i=0; i<num_data-1; i+=2){
		if (i > 0){
			strBuf += " ++ ";
		}
		int frm_st = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i] );
		int frm_ed = pdata->cnv.getFrmFromMsec( pdata->resultTrim[i+1] );
		strBuf += "Trim(" + to_string(frm_st) + "," + to_string(frm_ed) + ")";
	}
	strBuf += "\n";
	globalState.fileOutput(strBuf);
}
//--- �ڍ׏�񌋉ʏo�́iJlsIf��outputResultDetail�̓��e�j---
void JlsScript::setCmdEchoResultDetail(){
	//--- ������ ---
	pdata->outputResultDetailReset();

	//--- �f�[�^�ǂݍ��݁E�o�� ---
	string strBuf;
	while( pdata->outputResultDetailGetLine(strBuf) == 0){
		globalState.fileOutput(strBuf + "\n");
	}
}

//---------------------------------------------------------------------
// Read�֘A�R�}���h����
//---------------------------------------------------------------------
bool JlsScript::setCmdRead(JlsCmdArg& cmdarg){
	bool success = true;
	bool valid = true;

	switch ( cmdarg.cmdsel ){
		case CmdType::ReadData:
			valid = funcReg.readDataList(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadTrim:
			valid = funcReg.readDataTrim(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadString:
			valid = funcReg.readDataString(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadCheck:
			valid = funcReg.readDataCheck(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadOpen:
			valid = funcReg.readGlobalOpen(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ReadClose:
			funcReg.readGlobalClose(cmdarg);
			break;
		case CmdType::ReadLine:
			valid = funcReg.readGlobalLine(cmdarg);
			break;
		case CmdType::EnvGet:
			valid = funcReg.readDataEnvGet(cmdarg, cmdarg.getStrArg(1));
			break;
		default:
			globalState.addMsgErrorN("error:internal setting");
			success = false;
			break;
	}
	funcReg.setSystemRegReadValid(valid);	// READVALID�ϐ��ݒ�
	return success;
}

//---------------------------------------------------------------------
// ���W�X�^�ݒ�֘A����
//---------------------------------------------------------------------
bool JlsScript::setCmdReg(JlsCmdArg& cmdarg, JlsScriptState& state){
	bool success = true;

	//--- �����ݒ菈���������ꍇ�͎��s���Ċ��� ---
	if ( setArgAreaDefault(cmdarg, state) ){
		return true;
	}

	switch( cmdarg.cmdsel ){
		case CmdType::SetReg:
		case CmdType::Default:
		case CmdType::Set:
		case CmdType::EvalFrame:
		case CmdType::EvalTime:
		case CmdType::EvalNum:
		case CmdType::LocalSet:
		case CmdType::LocalSetF:
		case CmdType::LocalSetT:
		case CmdType::LocalSetN:
		case CmdType::SetList:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::Default )? false : true;
				bool flagLocal = ( cmdarg.cmdsel == CmdType::LocalSet )? true : false;
				if ( cmdarg.cmdsel == CmdType::LocalSetF ||
				     cmdarg.cmdsel == CmdType::LocalSetT ||
				     cmdarg.cmdsel == CmdType::LocalSetN ){
					flagLocal = true;
				}
				if ( cmdarg.getOptFlag(OptType::FlagLocal) ){	// -local�I�v�V����
					flagLocal = true;
				}
				if ( cmdarg.getOptFlag(OptType::FlagDefault) ){	// -default�I�v�V����
					overwrite = false;
				}
				string strVal = cmdarg.getStrArg(2);
				if ( cmdarg.cmdsel == CmdType::SetReg ){	// SetReg�͕ϐ��l���擾
					string strName = strVal;
					if ( !funcReg.getJlsRegVarNormal(strVal, strName) ){
						globalState.addMsgErrorN("error: not found the registrer name(" + strName + ")");
					}
				}else if ( cmdarg.cmdsel == CmdType::SetList ){	// SetList��csv�F������
					strVal = funcReg.getStrRegListByCsvStr(strVal);
				}
				if ( cmdarg.getOptFlag(OptType::FlagRestore) ){	// -restore�I�v�V����
					funcReg.restoreStrQuote(strVal);
				}
				success = funcReg.setJlsRegVarWithLocal(cmdarg.getStrArg(1), strVal, overwrite, flagLocal);
			}
			break;
		case CmdType::Unset:
			{
				bool flagLocal = false;
				if ( cmdarg.getOpt(OptType::FlagLocal) > 0 ){	// -local�I�v�V����
					flagLocal = true;
				}
				success = funcReg.unsetJlsRegVar(cmdarg.getStrArg(1), flagLocal);
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
				funcReg.setJlsRegVarWithLocal(cmdarg.getStrArg(1), "0", false, flagLocal);	// ���ݒ莞��0��������
				int step = 1;
				if ( cmdarg.isSetOpt(OptType::NumStep) ){
					step = cmdarg.getOpt(OptType::NumStep);
				}
				success = funcReg.setJlsRegVarCountUp(cmdarg.getStrArg(1), step, flagLocal);
			}
			break;
		case CmdType::OptSet:
		case CmdType::OptDefault:
			{
				bool overwrite = ( cmdarg.cmdsel == CmdType::OptDefault )? false : true;
				funcReg.setSystemRegOptions(cmdarg.getStrArg(1), 0, overwrite);
			}
			break;
		case CmdType::UnitSec:					// ���背�W�X�^�ݒ�
			{
				pdata->cnv.changeUnitSec(cmdarg.getValStrArg(1));
				funcReg.setSystemRegUpdate();
			}
			break;
		case CmdType::ArgSet:
			success = funcReg.setArgRegByVal(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::ArgSetReg:
			success = funcReg.setArgRegByName(cmdarg.getStrArg(1), cmdarg.getStrArg(2));
			break;
		case CmdType::ListGetAt:
			funcReg.setResultRegListGetAt(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListIns:
			funcReg.setResultRegListIns(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListDel:
			funcReg.setResultRegListDel(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListJoin:
			funcReg.setResultRegListJoin(cmdarg);
			break;
		case CmdType::ListRemove:
			funcReg.setResultRegListRemove(cmdarg);
			break;
		case CmdType::ListSel:
			funcReg.setResultRegListSel(cmdarg, cmdarg.getStrArg(1));
			break;
		case CmdType::ListSetAt:
			funcReg.setResultRegListRep(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListClear:
			funcReg.setResultRegListClear(cmdarg);
			break;
		case CmdType::ListDim:
			funcReg.setResultRegListDim(cmdarg, cmdarg.getValStrArg(1));
			break;
		case CmdType::ListSort:
			funcReg.setResultRegListSort(cmdarg);
			break;
		case CmdType::SplitCsv:
			funcReg.setStrRegListByCsv(cmdarg);
			break;
		case CmdType::SplitItem:
			funcReg.setStrRegListBySpc(cmdarg);
			break;
		default:
			globalState.addMsgErrorN("error:internal setting(RegCmd)");
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
				makeArgMemStore(cmdarg, state);			// �ۊǌ^�����ϐ��̑}�����e�����肷�鏈��
			}
			break;
		case CmdType::EndLazy:				// EndLazy��
			if ( state.isLazyArea() == false ){
				globalState.addMsgErrorN("warning:exist no effect EndLazy");
			}
			state.setLazyStartType(LazyType::None);	// lazy����
			break;
		case CmdType::Memory:				// Memory��
		case CmdType::MemSet:				// MemSet��
			{
				string strMemName = cmdarg.getStrArg(1);
				state.startMemArea(strMemName);
				//--- ������`�̐ݒ� ---
				makeArgMemStore(cmdarg, state);	// �ۊǌ^�����ϐ��̑}�����e�����肷�鏈��
				if ( cmdarg.isSetOpt(OptType::NumOrder) ){	// ���s���ʂ�ݒ�
					state.setMemOrder(cmdarg.getOpt(OptType::NumOrder));
				}
				vector<string> listMod;
				cmdarg.getListStrArgs(listMod);
				bool valid = state.setMemDefArg(listMod);	// �����Ȃ��Ŏ��ʎq�g�p�ɐݒ�
				if ( valid ){
					if ( cmdarg.cmdsel == CmdType::MemSet ){
						state.setMemUnusedFlag(strMemName);		// MemSet�ł͖��g�p��Ԃɐݒ�
					}
				}else{
					globalState.addMsgErrorN("error:already defined the name as Functiont");
				}
			}
			break;
		case CmdType::Function:				// Function��
			{
				string strMemName = cmdarg.getStrArg(1);
				state.startMemArea(strMemName);
				//--- ������`�̐ݒ� ---
				vector<string> listMod;
				cmdarg.getListStrArgs(listMod);
				if ( !state.setMemDefArg(listMod) ){	// �֐��^�����̒�`��ۑ�
					globalState.addMsgErrorN("error:new argument insertion");
				}
			}
			break;
		case CmdType::EndMemory:			// EndMemory��
		case CmdType::EndFunc:				// EndFunc��
			if ( state.isMemArea() == false ){
				globalState.addMsgErrorN("warning:exist no effect EndMemory");
			}
			state.endMemArea();
			break;
		case CmdType::ExpandOn:
			state.setMemExpand(true);
			break;
		case CmdType::ExpandOff:
			state.setMemExpand(false);
			break;
		default:
			globalState.addMsgErrorN("error:internal setting(LazyCategory)");
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
			globalState.addMsgErrorN("error:internal setting(MemCategory)");
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
		if ( funcReg.isSystemRegLastexe() ){	// ���O�R�}���h�����s�����ꍇ
			valid_exe = false;			// ����R�}���h�͎��s���Ȃ���
			exe_command = true;			// ���s�ς݈���
		}
	}
	//--- �O�R�}���h���s�ς݂��m�F (-cont option) ---
	if (cmdset.arg.getOpt(OptType::FlagCont) > 0){
		if ( funcReg.isSystemRegLastexe() == false ){	// ���O�R�}���h�����s���Ă��Ȃ��ꍇ
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
			funcReg.setSystemRegUpdate();		// NOLOGO�X�V
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
	//--- ���X�g�쐬���͊J�n�O�ɓ��e���� ---
	if ( cmdset.arg.cmdsel == CmdType::GetList ||
	     cmdset.arg.cmdsel == CmdType::GetPos ){
		funcReg.updateResultRegWrite(cmdset.arg);	// �ϐ�($POSHOLD/$LISTHOLD)�N���A
	}
	//--- -pickin/-pick�I�ʁB-pick�I�v�V�����g�p���͎��s�ӏ����S�������Ă�����s���� ---
	bool picki = cmdset.arg.tack.pickIn;
	bool picko = cmdset.arg.tack.pickOut;
	cmdset.limit.clearPickList();		// �O�̂��ߏ�����
	//--- �e�L�����S�����s ---
	bool exeflag_total = false;
	for(int i=0; i<nmax_list; i++){
			//--- ��������𖞂����Ă��郍�S���m�F ---
			bool exeflag = m_funcLimit->selectTargetByLogo(cmdset, i);
			if ( exeflag ){
				 exeflag = exeCmdLogoIsValidExe(cmdset);	// -pick�p�Ɏ��O���s����
			}
			if (exeflag){
				if ( picko ){
					cmdset.limit.addPickListCurrent();	// ��Ŕ��f���邽�߂����ł͊i�[
				}else{
					if ( picki ){
						string strArgPick = cmdset.arg.getStrOpt(OptType::ListPickIn);	// -pickin
						exeflag = pdata->cnv.isStrMultiNumIn(strArgPick, i+1, nmax_list);	// �擪=1
					}
					if ( exeflag ){
						exeflag = exeCmdLogoFromCat(cmdset);	// ���ۂ̎��s
					}
				}
			}
			//--- ���s���Ă�������s�t���O�ݒ� ---
			if (exeflag){
				exeflag_total = true;
			}
	}
	//--- -pick�I�v�V�����g�p���̌ォ����s ---
	if ( picko ){
		exeflag_total = false;	// ��蒼��
		int sizeListExe = cmdset.limit.sizePickList();		// ���s�����𖞂������S�̐�
		for(int i=1; i<=sizeListExe; i++){
			string strArgPick = cmdset.arg.getStrOpt(OptType::ListPickOut);	// -pick
			if ( pdata->cnv.isStrMultiNumIn(strArgPick, i, sizeListExe) ){
				cmdset.limit.selectPickList(i);		// �g�p���錋�ʂ�I��
				bool exeflag = exeCmdLogoFromCat(cmdset);	// ���ۂ̎��s
				//--- ���s���Ă�������s�t���O�ݒ� ---
				if (exeflag){
					exeflag_total = true;
				}
			}
		}
		cmdset.limit.clearPickList();		// �O�̂��ߏ�����
	}
	return exeflag_total;
}
//--- -pick�R�}���h�̂��߂Ɏ��O���s���� ---
bool JlsScript::exeCmdLogoIsValidExe(JlsCmdSet& cmdset){
	//--- ���ʈʒu�^�I���ʒu�擾 ---
	TargetLocInfo tgDst = cmdset.limit.getResultDst();
	TargetLocInfo tgEnd = cmdset.limit.getResultEnd();

	bool valid = true;
	switch(cmdset.arg.cmdsel){
		case CmdType::GetPos:
		case CmdType::GetList:
			valid = (( tgDst.valid || (tgDst.tp == TargetScpType::Direct)) &&
			         ( tgEnd.valid || (tgEnd.tp == TargetScpType::Direct)));
			break;
		case CmdType::AutoIns:
		case CmdType::AutoDel:
			valid = ( tgDst.valid && tgEnd.valid );
			break;
		default:
			break;
	}
	return valid;
}
//--- �J�e�S���ʂɕ��򂷂���s ---
bool JlsScript::exeCmdLogoFromCat(JlsCmdSet& cmdset){
	bool exeflag = true;
	//--- ���s���� ---
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
	return exeflag;
}
//---------------------------------------------------------------------
// ���S�ʒu�ʂ̎��s�R�}���h
//---------------------------------------------------------------------
bool JlsScript::exeCmdLogoTarget(JlsCmdSet& cmdset){
	bool exe_command = false;
	Nsc nsc_scpos_sel = cmdset.limit.getResultDstNsc();
	Nrf nrf = cmdset.limit.getLogoBaseNrf();

	switch(cmdset.arg.cmdsel){
		case CmdType::Find:
			{
				Msec msec;
				if ( getMsecTargetDst(msec, cmdset, false) ){
					if (cmdset.arg.getOpt(OptType::FlagAutoChg) > 0){	// �����\���ɔ��f
						exeCmdCallAutoScript(cmdset);					// Auto�����N���X�Ăяo��
					}
					else{								// �]���\���ɔ��f
						pdata->setResultLogoAtNrf(msec, LOGO_RESULT_DECIDE, nrf);
					}
				}
				exe_command = true;
			}
			break;
		case CmdType::MkLogo:
			{
				RangeMsec rmsec;
				if ( getRangeMsecTarget(rmsec, cmdset) ){
					bool overlap = ( cmdset.arg.getOpt(OptType::FlagOverlap) != 0 )? true : false;
					bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
					bool unit    = ( cmdset.arg.getOpt(OptType::FlagUnit)    != 0 )? true : false;
					int nsc_ins = pdata->insertLogo(rmsec.st, rmsec.ed, overlap, confirm, unit);
					exe_command = (nsc_ins >= 0)? true : false;
				}
			}
			break;
		case CmdType::DivLogo:
			{
				Msec msec;
				if ( getMsecTargetDst(msec, cmdset, false) ){
					bool confirm = ( cmdset.arg.getOpt(OptType::FlagConfirm) != 0 )? true : false;
					bool unit    = true;
					LogoEdgeType edge = cmdset.limit.getResultDstEdge();
					Nsc nsc_ins = pdata->insertDivLogo(msec, confirm, unit, edge);
					exe_command = (nsc_ins >= 0)? true : false;
				}
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
			exe_command = true;  // exeCmdLogoIsValidExe(cmdset);  // ���{�ς�
			if ( exe_command ){
				Msec msecDst;
				Msec msecEnd;
				getMsecTargetDst(msecDst, cmdset, true);	// allow forceSCP
				getMsecTargetEnd(msecEnd, cmdset, true);
				if ( cmdset.arg.getOptFlag(OptType::FlagHoldEnd) ){	// -HoldE
					funcReg.setResultRegPoshold(cmdset.arg, msecEnd);		// �ϐ��ɐݒ�
				}else{
					funcReg.setResultRegPoshold(cmdset.arg, msecDst);		// �ϐ��ɐݒ�
				}
			}
			break;
		case CmdType::GetList:
			exe_command = true;  // exeCmdLogoIsValidExe(cmdset);  // ���{�ς�
			if ( exe_command ){
				Msec msecDst;
				Msec msecEnd;
				getMsecTargetDst(msecDst, cmdset, true);	// allow forceSCP
				getMsecTargetEnd(msecEnd, cmdset, true);
				if ( cmdset.arg.getOptFlag(OptType::FlagHoldBoth) ){	// -HoldB
					Msec msec1 = ( msecDst <= msecEnd )? msecDst : msecEnd;
					Msec msec2 = ( msecDst <= msecEnd )? msecEnd : msecDst;
					funcReg.setResultRegListhold(cmdset.arg, msec1);	// �ϐ��ɐݒ�
					funcReg.setResultRegListhold(cmdset.arg, msec2);	// �ϐ��ɐݒ�
				}else if ( cmdset.arg.getOptFlag(OptType::FlagHoldEnd) ){	// HoldE
					// END�p�ϐ�
					funcReg.setResultRegListhold(cmdset.arg, msecEnd);		// �ϐ��ɐݒ�
				}else{
					funcReg.setResultRegListhold(cmdset.arg, msecDst);		// �ϐ��ɐݒ�
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
	if (funcReg.getJlsRegVarNormal(cstr, "TAILTIME") ){
		pdata->cnv.getStrValMsecM1(msec_headframe, cstr, 0);
	}
	//--- �͈͂��擾 ---
	WideMsec wmsecBase;
	wmsecBase.just  = msec_headframe;	// ��ʒu�̂ݐݒ肵�ă^�[�Q�b�g,force�͏������ʉ�
	wmsecBase.early = msec_headframe;
	wmsecBase.late  = msec_headframe;
	//--- ��ԋ߂��V�[���`�F���W�ʒu���擾 ---
	m_funcLimit->selectTargetByRange(cmdset, wmsecBase);

	//--- ���ʂ��i�[ --
	bool exeflag = false;
	Msec msec_tailframe;
	exeflag = getMsecTargetDst(msec_tailframe, cmdset, true);	// allow forceSCP
	if ( exeflag ){
		funcReg.setSystemRegHeadtail(msec_headframe, msec_tailframe);
		//--- -autochg�I�v�V�����Ή� ---
		if ( cmdset.arg.getOpt(OptType::FlagAutoChg) > 0 ){		// �����\���ɔ��f
			exeCmdCallAutoScript(cmdset);						// Auto�����N���X�Ăяo��
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// cmdset��񂩂�ΏۂƂȂ閳���V�[���`�F���W���擾�B�Ȃ���ΐݒ�ɂ���Ă͋����쐬
// �o��
//  �Ԃ�l     �F���ʎ擾�L��
//  msecResult : �擾�����ʒu
//---------------------------------------------------------------------
bool JlsScript::getMsecTargetDst(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp){
	return getMsecTargetSub(msecResult, cmdset, allowForceScp, false);
}
bool JlsScript::getMsecTargetEnd(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp){
	return getMsecTargetSub(msecResult, cmdset, allowForceScp, true);
}
bool JlsScript::getMsecTargetSub(Msec& msecResult, JlsCmdSet& cmdset, bool allowForceScp, bool flagEnd){
	TargetLocInfo tgLoc;
	if ( flagEnd ){
		tgLoc = cmdset.limit.getResultEnd();
	}else{
		tgLoc = cmdset.limit.getResultDst();
	}
	msecResult = tgLoc.msout;

	bool success = ( msecResult >= 0 );
	if ( tgLoc.tp == TargetScpType::Force ){
		if ( msecResult < 0 || msecResult > pdata->getMsecTotalMax() ){
			success = false;
		}
		//--- force�ɂ�閳���V�[���`�F���W�쐬 ---
		else if ( allowForceScp ){
			if ( cmdset.arg.getOpt(OptType::FlagForce) > 0 ){	// -force�I�v�V�����ŋ����쐬��
				pdata->getNscForceMsecExact(tgLoc.msout, tgLoc.edge, tgLoc.exact);
			}
		}
	}
	return success;
}
//---------------------------------------------------------------------
// cmdset��񂩂�Ώ۔͈͂ƂȂ閳���V�[���`�F���W�Q�_���擾
// �o��
//  �Ԃ�l     �F���ʎ擾�L��
//  rmsecResult : �擾�����͈͈ʒu
//---------------------------------------------------------------------
bool JlsScript::getRangeMsecTarget(RangeMsec& rmsecResult, JlsCmdSet& cmdset){
	TargetLocInfo tgDst = cmdset.limit.getResultDst();
	TargetLocInfo tgEnd = cmdset.limit.getResultEnd();
	Msec msecDst = tgDst.msout;
	Msec msecEnd = tgEnd.msout;

	if ( msecDst < msecEnd ){
		rmsecResult.st = msecDst;
		rmsecResult.ed = msecEnd;
	}else{
		rmsecResult.st = msecEnd;
		rmsecResult.ed = msecDst;
	}
	return ( rmsecResult.st >= 0 && rmsecResult.ed >= 0 );
}

