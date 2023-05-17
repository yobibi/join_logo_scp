//
// ���s�X�N���v�g�R�}���h��������
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptDecode.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"

///////////////////////////////////////////////////////////////////////
//
// ���s�X�N���v�g�R�}���h�������̓N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ������
//  pdata�͕�����E���ԕϊ��@�\(cnv)�̂ݎg����
//---------------------------------------------------------------------
JlsScriptDecode::JlsScriptDecode(JlsDataset *pdata, JlsScrFuncList* pFuncList){
	this->pdata  = pdata;
	this->pFuncList = pFuncList;
}

//---------------------------------------------------------------------
// �����ݒ�ُ̈�m�F
//---------------------------------------------------------------------
void JlsScriptDecode::checkInitial(){
	int SIZE_JLCMD_SEL = static_cast<int>(jlscmd::CmdType::MAXSIZE);
	if ( (int)CmdDefine.size() != SIZE_JLCMD_SEL ){
		castErrInternal("mismatch at CmdDefine size from CmdType size");
	}
	if ( (int)ConfigDefine.size() != SIZE_CONFIG_VAR ){
		castErrInternal("mismatch at ConfigDefine size from SIZE_CONFIG_VAR");
	}
	for(int i=0; i<(int)CmdDefine.size(); i++){
		if ( CmdDefine[i].cmdsel != (CmdType) i ){
			castErrInternal("error:internal mismatch at CmdDefine.cmdsel " + to_string(i));
		}
	}
	for(int i=0; i<(int)ConfigDefine.size(); i++){
		if ( ConfigDefine[i].prmsel != (ConfigVarType) i ){
			castErrInternal("error:internal mismatch at ConfigDefine.prmsel " + to_string(i));
		}
	}
}

//=====================================================================
// �f�R�[�h����
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h���e�𕶎���P�s������
// ���́F
//  strBuf  : ��͕�����
//  onlyCmd : �擪�̃R�}���h���������̉�͂��ifalse=�S�́Atrue=�R�}���h�̂݁j
// �o�́F
//   �Ԃ�l�F�G���[���
//   cmdarg: �R�}���h��͌���
//---------------------------------------------------------------------
CmdErrType JlsScriptDecode::decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd){
	CmdErrType retval = CmdErrType::None;

	//--- �R�}���h���e������ ---
	cmdarg.clear();
	//--- �R�}���h��t ---
	JlscrCmdRecord cmddef;
	int pos = decodeCmdName(cmddef, retval, strBuf);
	if ( retval != CmdErrType::None ){
		return retval;		// �R�}���h�ُ펞�̏I��
	}
	//--- �R�}���h�i�[ ---
	cmdarg.cmdsel   = cmddef.cmdsel;
	cmdarg.category = cmddef.category;
	//--- �R�}���h��t�݂̂ŏI������ꍇ ---
	if ( onlyCmd ){
		return retval;
	}
	//--- �R�����g���� ---
	string strNewBuf;
	if ( cmddef.muststr == 9 ){			// �������S���ǂ݂̎��͎c��
		strNewBuf = strBuf;
	}else{
		pdata->cnv.getStrWithoutComment(strNewBuf, strBuf);
		if ( pos >= (int)strNewBuf.length() ){
			pos = -1;	// �f�[�^�Ȃ�
		}
	}
	//--- �R�}���h��� ---
	if ( cmddef.muststr > 0 || cmddef.mustchar > 0 || cmddef.mustrange > 0 ){
		pos = decodeCmdArgMust(cmdarg, retval, strNewBuf, pos, cmddef);
	}
	//--- �I�v�V������t ---
	if (cmddef.needopt > 0 && pos >= 0){
		pos = decodeCmdArgOpt(cmdarg, retval, strNewBuf, pos);
	}
	//--- ���������Z���H ---
	if ( cmddef.muststr > 0 ){
		bool success = calcCmdArg(cmdarg);
		if ( success == false ){
			setErrItem("mismatch fix argument type - evaluation failed");
			retval = CmdErrType::ErrOpt;
		}
	}
	return retval;
}

//---------------------------------------------------------------------
// �����񂩂�R�}���h�����擾
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdName(JlscrCmdRecord& cmddef, CmdErrType& errval, const string& strBuf){
	//--- �R�}���h��t(cmdsel) ---
	string strCmd;
	int pos = pdata->cnv.getStrItemCmd(strCmd, strBuf, 0);
	int csel = ( pos >= 0 )? decodeCmdNameId(strCmd) : 0;
	if (csel < 0){
		setErrItem(strCmd);
		errval = CmdErrType::ErrCmd;
		return -1;
	}
	//--- �R�}���h�i�[ ---
	cmddef = CmdDefine[csel];
	return pos;
}
//---------------------------------------------------------------------
// �R�}���h�������X�g����ԍ��Ŏ擾
// �o�́F
//   �Ԃ�l  �F�擾�R�}���h�ԍ��i���s����-1�j
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdNameId(const string& cstr){
	int det = -1;
	const char *cmdname = cstr.c_str();

	if (cmdname[0] == '\0' || cmdname[0] == '#'){
		det = 0;
	}
	else{
		for(int i=0; i<(int)CmdDefine.size(); i++){
			if (_stricmp(cmdname, CmdDefine[i].cmdname.c_str()) == 0){
				det = i;
				break;
			}
		}
		//--- ������Ȃ���Εʖ������� ---
		if (det < 0){
			bool flag = false;
			CmdType target;
			for(int i=0; i<(int)CmdAlias.size(); i++){
				if (_stricmp(cmdname, CmdAlias[i].cmdname.c_str()) == 0){
					target = CmdAlias[i].cmdsel;
					flag = true;
					break;
				}
			}
			if ( flag ){
				for(int i=0; i<(int)CmdDefine.size(); i++){
					if ( CmdDefine[i].cmdsel == target ){
						det = i;
						break;
					}
				}
			}
		}
	}
	return det;
}

//---------------------------------------------------------------------
// �K�{�����̎擾
// ���́F
//   strBuf : ������
//   pos    : �F���J�n�ʒu
//   cmddef
//   muststr  : ����������i0-3=�擾�� 9=�c��S�� 8=�������c��S�� 7=�֐��^�����j
//   mustchar : ��ސݒ�i0=�ݒ�Ȃ�  1=S/E/B  2=TR/SP/EC 3=�ȗ��\��S/E/B�j tps>0���͘A��quote���c�������Ɏg�p
//   mustrange: ���Ԑݒ�i0=�ݒ�Ȃ�  1=center  3=center+left+right�j
// �o�́F
//   �Ԃ�l  : �ǂݍ��݈ʒu�i-1=�I�v�V�����ُ�j
//   errval  : �G���[�ԍ�
//   cmdarg  : �R�}���h��͌���
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, const JlscrCmdRecord& cmddef){
	//--- ������Ƃ��Ĉ������擾 ---
	if ( cmddef.muststr > 0 && pos >= 0){
		if ( cmddef.muststr == 9 ){
			//--- �c��S���𕶎���Ƃ��Ď擾 ---
			while( strBuf[pos] == ' ' && pos >= 0) pos++;
			string strArg = strBuf.substr(pos);
			cmdarg.addArgString(strArg);
			pos = -1;
		}else if ( cmddef.muststr == 7 ){
			//--- ���W���[�����ƈ������擾 ---
			if ( pdata->cnv.isStrFuncModule(strBuf, pos) ){
				vector<string> listMod;
				pos = pdata->cnv.getListModuleArg(listMod, strBuf, pos);
				if ( pos >= 0 ){
					for(int i=0; i<(int)listMod.size(); i++){
						cmdarg.addArgString(listMod[i]);
					}
				}else{
					setErrItem("not module format");
					errval = CmdErrType::ErrOpt;
				}
			}else{
				pos = -1;
				setErrItem("not module format");
				errval = CmdErrType::ErrOpt;
			}
		}else{
			int sizeArg = cmddef.muststr;
			for(int i=0; i<sizeArg; i++){
				string strArg;
				if ( cmddef.mustchar == 9 ){
					pos = pdata->cnv.getStrItemMonitor(strArg, strBuf, pos);
				}else{
					pos = pdata->cnv.getStrItemArg(strArg, strBuf, pos);
				}
				if ( pos >= 0 ){
					cmdarg.addArgString(strArg);
				}
			}
			if (pos < 0){
				setErrItem("need argSize:" + to_string(sizeArg));
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- ��ޕ��� ---
	if (cmddef.mustchar > 0 && cmddef.muststr == 0 && pos >= 0){
		string strTmp;
		int posbak = pos;
		pos = pdata->cnv.getStrItemArg(strTmp, strBuf, pos);
		if (pos >= 0){
			//--- ���ڂP�i�����w��j ---
			if (cmddef.mustchar == 1 || cmddef.mustchar == 3){
				if (strTmp[0] == 'S' || strTmp[0] == 's'){
					cmdarg.selectEdge = LOGO_EDGE_RISE;
				}
				else if (strTmp[0] == 'E' || strTmp[0] == 'e'){
					cmdarg.selectEdge = LOGO_EDGE_FALL;
				}
				else if (strTmp[0] == 'B' || strTmp[0] == 'b'){
					cmdarg.selectEdge = LOGO_EDGE_BOTH;
				}
				else{
					if ( cmddef.mustchar == 1 || cmddef.mustchar == 3 ){
						pos = posbak;
						cmdarg.selectEdge = LOGO_EDGE_RISE;
					}
					else{
						pos    = -1;
						errval = CmdErrType::ErrSEB;
					}
				}
			}
			//--- TR/SP/EC �����񔻕� ---
			else if (cmddef.mustchar == 2){
				bool flagOption = false;
				if ( getTrSpEcID(cmdarg.selectAutoSub, strTmp, flagOption) == false ){
					pos    = -1;
					errval = CmdErrType::ErrTR;
				}
			}
		}
	}
	//--- �͈͎w�� ---
	if (cmddef.mustrange > 0 && pos >= 0){
		if (cmddef.mustrange == 1 || cmddef.mustrange == 3){
			JlscrDecodeRangeRecord infoDec = {};
			if (cmddef.mustrange == 1){
				infoDec.numRead  = 1;		// �f�[�^�ǂݍ��ݐ�=1
				infoDec.needs    = 0;		// �Œ�ǂݍ��ݐ�=0�i�S���ڏȗ��j
				infoDec.numFrom  = 0;		// �ȗ����̊J�n�w��Ȃ��i�W������j
				infoDec.flagM1   = false;	// -1�͒ʏ�̐��l�Ƃ��ēǂݍ���
				infoDec.flagSort = false;	// �P�f�[�^�Ȃ̂ŕ��ёւ��Ȃ�
			}
			else if (cmddef.mustrange == 3){
				infoDec.numRead  = 3;		// �f�[�^�ǂݍ��ݐ�=3
				infoDec.needs    = 0;		// �Œ�ǂݍ��ݐ�=0�i�S���ڏȗ��j
				infoDec.numFrom  = 0;		// �ȗ����̊J�n�w��Ȃ��i�W������j
				infoDec.flagM1   = false;	// -1�͒ʏ�̐��l�Ƃ��ēǂݍ���
				infoDec.flagSort = true;	// ���������̕��ёւ�����
			}
			pos = decodeRangeMsec(infoDec, strBuf, pos);
			cmdarg.wmsecDst = infoDec.wmsecVal;
			if ( cmddef.mustrange == 3 && infoDec.numAbbr != 3 ){
				cmdarg.setOpt(OptType::FlagDstPoint, 1);	// Dst�ݒ肠��Ƃ���
			}
		}
		if (pos < 0){
			errval = CmdErrType::ErrRange;
		}
	}

	return pos;
}


//---------------------------------------------------------------------
// �����I�v�V�����̎擾
// �o�b�t�@�c�蕔������P�ݒ������
// �o�́F
//   �Ԃ�l  : �ǂݍ��݈ʒu�i-1=�I�v�V�����ُ�j
//   errval  : �G���[�ԍ�
//   cmdarg  : �R�}���h��͌���
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgOpt(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos){
	m_listKeepSc.clear();		// -SC�n�̃I�v�V�����f�[�^���ꎞ�ێ��̏�����
	//--- �e�����擾 ---
	while(pos >= 0){
		pos = decodeCmdArgOptOne(cmdarg, errval, strBuf, pos);
	}
	reviseCmdRange(cmdarg);		// �I�v�V�����ɂ��͈͕␳
	setCmdTackOpt(cmdarg);		// ���s�I�v�V�����ݒ�
	setArgScOpt(cmdarg);		// �ꎞ�ێ�����-SC�n�I�v�V������ݒ�
	mirrorOptToUndef(cmdarg);	// ���w��̃I�v�V�����ɕ���
	return pos;
}
//---------------------------------------------------------------------
// �����I�v�V�����̎擾
// �o�b�t�@�c�蕔������P�ݒ������
// �o�́F
//   �Ԃ�l  : �ǂݍ��݈ʒu�i-1=�I�v�V�����ُ�j
//   errval  : �G���[�ԍ�
//   cmdarg  : �R�}���h��͌���
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgOptOne(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos){
	//--- ���̃I�v�V�����ǂݍ��� ---
	string strWord;
	if (pos >= 0){
		pos = pdata->cnv.getStrItemArg(strWord, strBuf, pos);
	}
	//--- �ʏ�̈����擾 ---
	int optsel = -1;
	if (pos >= 0){
		//--- �I�v�V�������� ---
		const char *pstr = strWord.c_str();
		for(int i=0; i<(int)OptDefine.size(); i++){
			if (!_stricmp(pstr, OptDefine[i].optname.c_str())){
				optsel = i;
			}
		}
		if (optsel < 0){		// �I�v�V�����Ή�������Ȃ�
			pos = -1;
		}
		if (pos >= 0){
			//--- �ݒ� ---
			pos = decodeCmdArgOptOneSub(cmdarg, optsel, strBuf, pos);
		}
		//--- �����s�����̃G���[ ---
		if (pos < 0){
			setErrItem(strWord);
			errval = CmdErrType::ErrOpt;
		}
	}
	return pos;
}

//--- optsel�ɑΉ������I�v�V��������ݒ� ---
int JlsScriptDecode::decodeCmdArgOptOneSub(JlsCmdArg& cmdarg, int optsel, const string& strBuf, int pos){
	//--- optsel�̕�����ɑΉ�����R�}���h�����擾 ---
	OptType optType = OptDefine[optsel].optType;
	int optTypeInt = static_cast<int>(optType);
	OptCat category;
	if ( cmdarg.getOptCategory(category, optType) == false ){
		pos = -1;
		category = OptCat::None;
		castErrInternal("(OptDefine-category)" + strBuf);
		return pos;
	}
	//--- Msec�擾�p���쐬 ---
	JlscrDecodeRangeRecord infoDec = {};
	infoDec.numRead  = OptDefine[optsel].numArg;	// �f�[�^�ǂݍ��ݐ�
	infoDec.needs    = OptDefine[optsel].minArg;	// �Œ�ǂݍ��ݐ�
	infoDec.numFrom  = OptDefine[optsel].numFrom;	// �ȗ����J�n�ԍ��ݒ�
	infoDec.flagM1   = false;			// -1�͒ʏ�̐����Ƃ��ēǂݍ���
	infoDec.flagSort = false;			// ���������̕��ёւ��Ȃ�
	if ( OptDefine[optsel].convType == ConvStrType::MsecM1 ){
		infoDec.flagM1   = true;			// -1�͕ϊ����Ȃ��œǂݍ���
	}
	if ( OptDefine[optsel].sort == 12 && infoDec.numRead == 2){
		infoDec.flagSort = true;	// ���������̕��ёւ�����
	}else if ( OptDefine[optsel].sort == 23 && infoDec.numRead == 3){
		infoDec.flagSort = true;	// ���������̕��ёւ�����
	}else if ( OptDefine[optsel].sort > 0 ){
		castErrInternal("(OptDefine-sort)" + strBuf);
	}
	//--- �ݒ� ---
	switch( category ){
		case OptCat::NumLG :					// ���S�ԍ��̌���
			if ( cmdarg.isSetOpt(OptType::TypeNumLogo) == false ){
				// ��ނ�ݒ�
				cmdarg.setOpt(OptType::TypeNumLogo, optTypeInt);
				// �ԍ���ݒ�
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				string strSub;
				pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
				if ( pos >= 0 ){
					vector<string> listStrNum;
					if ( getListStrNumFromStr(listStrNum, strSub) ){
						for(int i=0; i < (int)listStrNum.size(); i++){
							cmdarg.addLgOpt(listStrNum[i]);
						}
					}else{
						if ( posBak >= 0 ){
							cmdarg.addLgOpt("0");	// �ȗ��ݒ�
							pos = posBak;
						}else{
							pos = -1;			// �ϊ����s
						}
					}
				}else if ( posBak >= 0 ){
					cmdarg.addLgOpt("0");	// �ȗ��ݒ�
					pos = posBak;
				}
			}
			break;
		case OptCat::FRAME :					// �t���[���ʒu�ɂ�����
			{
				pos = decodeRangeMsec(infoDec, strBuf, pos);
				if ( pos >= 0 ){
					cmdarg.setOpt(OptType::MsecFrameL, infoDec.wmsecVal.early);
					cmdarg.setOpt(OptType::MsecFrameR, infoDec.wmsecVal.late);
					cmdarg.setOpt(OptType::TypeFrame, optTypeInt);
					cmdarg.setOpt(OptType::TypeFrameSub, OptDefine[optsel].subType);
				}
			}
			break;
		case OptCat::PosSC :					// ����SC�ɂ�����
			{
				pos = decodeRangeMsec(infoDec, strBuf, pos);
				if ( pos >= 0 ){
					JlscrDecodeKeepSc keepSc;
					keepSc.type     = optType;
					keepSc.subtype  = OptDefine[optsel].subType;
					keepSc.wmsec    = infoDec.wmsecVal;
					keepSc.abbr     = infoDec.numAbbr;
					m_listKeepSc.push_back(keepSc);		// ��Ŕ͈͕␳������̂ňꎞ�ێ�
				}
			}
			break;
		case OptCat::STR :						// ������̐ݒ�i���X�g�ϐ����܂ށj
			{
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				string strSub;
				pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
				if ( pos >= 0 ){
					ConvStrType tp = OptDefine[optsel].convType;
					if ( tp == ConvStrType::None ){		// �ϊ��Ȃ��ꍇ
						if ( strSub[0] == '-' ){
							int tmpval;			// ����'-'�ł����l�Ȃ瑱�s
							if ( pdata->cnv.getStrValNum(tmpval, strSub, 0) < 0 ){
								pos = -1;
							}
						}
					}else{
						if ( convertStringFromListStr(strSub, tp) == false ){
							if ( posBak >= 0 ){
								strSub = "0";		// �ȗ��ݒ�
								pos = posBak;
							}else{
								pos = -1;			// �ϊ����s
							}
						}
					}
				}else if ( posBak >= 0 ){
					strSub = "0";
					pos = posBak;
				}
				if ( pos >= 0 ){
					cmdarg.setStrOpt(optType, strSub);
				}
				if ( OptDefine[optsel].numArg > 1 ){	// 2�����ȏ�̒ǉ�����
					pos = getOptionStrMulti(cmdarg, optType, strBuf, pos);
				}
			}
			break;
		case OptCat::NUM :						// ���l�̐ݒ�
			{
				int posBak = ( OptDefine[optsel].minArg == 0 )? pos : -1;
				vector<OptType> listOptType(4);
				int numUsed = getOptionTypeList(listOptType, optType, infoDec.numRead);
				vector<int> listVal(4);
				switch( OptDefine[optsel].convType ){
					case ConvStrType::MsecM1 :
						pos = decodeRangeMsec(infoDec, strBuf, pos);
						if ( pos >= 0 ){
							if ( infoDec.numRead == 3 ){
								listVal[0] = (int)infoDec.wmsecVal.just;
								listVal[1] = (int)infoDec.wmsecVal.early;
								listVal[2] = (int)infoDec.wmsecVal.late;
							}else if ( infoDec.numRead == 2 ){
								listVal[0] = (int)infoDec.wmsecVal.early;
								listVal[1] = (int)infoDec.wmsecVal.late;
							}else{
								listVal[0] = (int)infoDec.wmsecVal.just;
							}
							if ( numUsed > infoDec.numRead ){
								listVal[infoDec.numRead] = infoDec.numAbbr;
							}
						}
						break;
					case ConvStrType::Num :
						pos = pdata->cnv.getStrValNum(listVal[0], strBuf, pos);
						if ( pos < 0 && posBak >= 0 ){	// �ȗ���
							listVal[0] = OptDefine[optsel].numFrom;
							pos = posBak;
						}
						if ( infoDec.numRead > 1 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
					case ConvStrType::Sec :
						pos = pdata->cnv.getStrValSecFromSec(listVal[0], strBuf, pos);
						if ( infoDec.numRead > 1 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
					case ConvStrType::TrSpEc :
						{
							string strSub;
							pos = pdata->cnv.getStrItemArg(strSub, strBuf, pos);
							if ( pos >= 0 ){
								CmdTrSpEcID idSub;
								bool flagOption = true;
								if ( getTrSpEcID(idSub, strSub, flagOption) ){
									listVal[0] = static_cast<int>(idSub);
								}else{
									pos = -1;
								}
							}
							if ( infoDec.numRead > 1 ){
								castErrInternal("(OptDefine-numArg)" + strBuf);
							}
						}
						break;
					default :	// for flag
						listVal[0] = 1;
						if ( OptDefine[optsel].subType > 0 ){
							listVal[0] = OptDefine[optsel].subType;
						}
						if ( infoDec.numRead > 0 ){
							castErrInternal("(OptDefine-numArg)" + strBuf);
						}
						break;
				}
				for(int i=0; i < numUsed; i++){
					cmdarg.setOpt(listOptType[i], listVal[i]);
				}
			}
			break;
		default :
			castErrInternal("(OptDefine-category)" + strBuf);
			break;
	}
	return pos;
}
//--- ������擾��2�����ȏ�K�v�Ƃ���P�[�X�̏��� ---
int JlsScriptDecode::getOptionStrMulti(JlsCmdArg& cmdarg, OptType optType, const string& strBuf, int pos){
	switch( optType ){
		case OptType::StrCounter :
			{
				bool exist1 = false;
				int val1;
				if ( pos >= 0 ){
					int posBak = pos;
					pos = pdata->cnv.getStrValNum(val1, strBuf, pos);
					if ( pos >= 0 ){
						exist1 = true;
					}else{
						pos = posBak;
					}
				}
				bool exist2 = false;
				int val2;
				if ( exist1 ){
					int posBak = pos;
					pos = pdata->cnv.getStrValNum(val2, strBuf, pos);
					if ( pos >= 0 ){
						exist2 = true;
					}else{
						pos = posBak;
					}
				}
				if ( !exist1 ) val1 = 0;	// �ȗ����ݒ�
				if ( !exist2 ) val2 = 1;	// �ȗ����ݒ�
				cmdarg.setOpt(OptType::NumCounterI, val1);		// ccounter initial
				cmdarg.setOpt(OptType::NumCounterS, val2);		// ccounter step
			}
			break;
		default :
			break;
	}
	return pos;
}
//--- �I�v�V�����̊i�[����擾 ---
int JlsScriptDecode::getOptionTypeList(vector<OptType>& listOptType, OptType orgOptType, int numArg){
	int numUsed = 0;
	//--- �i�[����擾 ---
	switch( orgOptType ){
		case OptType::MsecEndlenC :
			numUsed = 4;
			listOptType[0] = OptType::MsecEndlenC;
			listOptType[1] = OptType::MsecEndlenL;
			listOptType[2] = OptType::MsecEndlenR;
			listOptType[3] = OptType::AbbrEndlen;
			break;
		case OptType::MsecEndSftC :
			numUsed = 4;
			listOptType[0] = OptType::MsecEndSftC;
			listOptType[1] = OptType::MsecEndSftL;
			listOptType[2] = OptType::MsecEndSftR;
			listOptType[3] = OptType::AbbrEndSft;
			break;
		case OptType::MsecSftC :
			numUsed = 4;
			listOptType[0] = OptType::MsecSftC;
			listOptType[1] = OptType::MsecSftL;
			listOptType[2] = OptType::MsecSftR;
			listOptType[3] = OptType::AbbrSft;
			break;
		case OptType::MsecTgtLimL :
			numUsed = 2;
			listOptType[0] = OptType::MsecTgtLimL;
			listOptType[1] = OptType::MsecTgtLimR;
			break;
		case OptType::MsecLenPMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenPMin;
			listOptType[1] = OptType::MsecLenPMax;
			break;
		case OptType::MsecLenNMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenNMin;
			listOptType[1] = OptType::MsecLenNMax;
			break;
		case OptType::MsecLenPEMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenPEMin;
			listOptType[1] = OptType::MsecLenPEMax;
			break;
		case OptType::MsecLenNEMin :
			numUsed = 2;
			listOptType[0] = OptType::MsecLenNEMin;
			listOptType[1] = OptType::MsecLenNEMax;
			break;
		case OptType::MsecFromHead :
			numUsed = 2;
			listOptType[0] = OptType::MsecFromHead;
			listOptType[1] = OptType::AbbrFromHead;
			break;
		case OptType::MsecFromTail :
			numUsed = 2;
			listOptType[0] = OptType::MsecFromTail;
			listOptType[1] = OptType::AbbrFromTail;
			break;
		case OptType::MsecLogoExtL :
			numUsed = 2;
			listOptType[0] = OptType::MsecLogoExtL;
			listOptType[1] = OptType::MsecLogoExtR;
			break;
		case OptType::MsecDrangeL :
			numUsed = 2;
			listOptType[0] = OptType::MsecDrangeL;
			listOptType[1] = OptType::MsecDrangeR;
			break;
		default :
			numUsed = 1;
			listOptType[0] = orgOptType;
			break;
	}
	if ( numUsed < numArg ){
		castErrInternal("(numArg) type:" + static_cast<int>(orgOptType));
	}
	return numUsed;
}

void JlsScriptDecode::castErrInternal(const string& msg){
	string mes = "error:internal setting" + msg;
	lcerr << mes << endl;
}


//---------------------------------------------------------------------
// TR/SP/EC������̔���
// �o�́F
//   �Ԃ�l   : ���ʐ���
//   autoSub  : ���ʕ�������
//---------------------------------------------------------------------
bool JlsScriptDecode::getTrSpEcID(CmdTrSpEcID& idSub, const string& strName, bool flagOption){
	bool det = false;
	if ( ! _stricmp(strName.c_str(), "TR") ){
		det = true;
		idSub = CmdTrSpEcID::TR;
	}
	else if ( ! _stricmp(strName.c_str(), "SP") ){
		det = true;
		idSub = CmdTrSpEcID::SP;
	}
	else if ( ! _stricmp(strName.c_str(), "EC") ){
		det = true;
		idSub = CmdTrSpEcID::EC;
	}
	else if ( ! _stricmp(strName.c_str(), "LG") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::LG;
	}
	else if ( ! _stricmp(strName.c_str(), "NLG") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::NLG;
	}
	else if ( ! _stricmp(strName.c_str(), "NTR") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::NTR;
	}
	else if ( ! _stricmp(strName.c_str(), "Off") && flagOption ){
		det = true;
		idSub = CmdTrSpEcID::Off;
	}
	else{
		det = false;
		idSub = CmdTrSpEcID::None;
	}
	return det;
}
//---------------------------------------------------------------------
// �����񂩂�ő�R���ځi���S�w�� �͈͐擪 �͈͖����j�̃~���b���l���擾
// �擾�ł��Ȃ�������f�t�H���g�l�������ēǂݍ��݈ʒu�͎擾�ł������܂Ŗ߂�
// ���́F
//   numRead : �ǂݍ��ރf�[�^��
//   needs   : �ǂݍ��ݍŒ�K�v��
//   flagM1  : -1�͂��̂܂܎c���ݒ�i0=���ʈ����Ȃ��ϊ��A1=-1�͕ϊ����Ȃ��j
//   strBuf  : ������
//   pos     : �F���J�n�ʒu
// �o�́F
//   �Ԃ�l   : ���̓ǂݍ��݈ʒu
//   wmsecVal : �R���ڎ擾�~���b
//   flagAbbr : �ȗ��f�[�^��
//---------------------------------------------------------------------
int JlsScriptDecode::decodeRangeMsec(JlscrDecodeRangeRecord& infoDec, const string& strBuf, int pos){
	WideMsec wmsecVal = {};
	int pos1 = -1;
	int pos2 = -1;
	int pos3 = -1;
	//--- �����񂩂�ǂݏo�� ---
	switch( infoDec.numRead ){
		case 3:
			if ( infoDec.flagM1 ){		// -1�͕ϊ����Ȃ�����
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.just,  strBuf, pos);
				pos2 = pdata->cnv.getStrValMsecM1(wmsecVal.early, strBuf, pos1);
				pos3 = pdata->cnv.getStrValMsecM1(wmsecVal.late,  strBuf, pos2);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.just,  strBuf, pos);
				pos2 = pdata->cnv.getStrValMsec(wmsecVal.early, strBuf, pos1);
				pos3 = pdata->cnv.getStrValMsec(wmsecVal.late,  strBuf, pos2);
			}
			break;
		case 2:
			if ( infoDec.flagM1 ){		// -1�͕ϊ����Ȃ�����
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.early, strBuf, pos);
				pos2 = pdata->cnv.getStrValMsecM1(wmsecVal.late,  strBuf, pos1);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.early, strBuf, pos);
				pos2 = pdata->cnv.getStrValMsec(wmsecVal.late,  strBuf, pos1);
			}
			break;
		case 1:
			if ( infoDec.flagM1 ){		// -1�͕ϊ����Ȃ�����
				pos1 = pdata->cnv.getStrValMsecM1(wmsecVal.just,  strBuf, pos);
			}else{
				pos1 = pdata->cnv.getStrValMsec(wmsecVal.just,  strBuf, pos);
			}
			break;
		default:
			break;
	}
	//--- ���l�ȊO�������ꍇ�̓f�t�H���g�l��ݒ� ---
	if ( pos1 < 0 ){
		wmsecVal.just  = 0;			// �f�t�H���g�l�F0ms
		wmsecVal.early = 0;			// �f�t�H���g�l�F0ms
		wmsecVal.late  = 0;			// �f�t�H���g�l�F0ms
	}
	//--- �ȗ����̃f�t�H���g�ݒ� ---
	switch( infoDec.numRead ){
		case 3:
			//--- 3���ږڂ����l�ȊO�������ꍇ��2,3���ږڂɃf�t�H���g�l��ݒ� ---
			if ( pos3 < 0 ){
				setRangeMargin(wmsecVal, -1);	// �f�t�H���g�l����
			}
			break;
		case 2:
			//--- 2����read��2���ڂ����l�ȊO�������ꍇ ---
			if ( pos2 < 0 ){
				if ( infoDec.numFrom != 0 ){
					//--- �ȗ��������ڂ��X�L�b�v���Đݒ肷��ꍇ ---
					if ( infoDec.numFrom == 2 ){
						wmsecVal.late  = wmsecVal.early;
						wmsecVal.early = 0;
					}
				}else{
					//--- 1���ږڂ𒆐S�Ƃ��ă}�[�W���̓f�t�H���g�l��ݒ� ---
					if ( pos1 >= 0 ){
						wmsecVal.just = wmsecVal.early;		// 1���ږڂ𒆐S�ɐݒ�
					}
					setRangeMargin(wmsecVal, -1);		// �f�t�H���g�l����
				}
			}
			break;
		default:
			break;
	}
	//--- �����������ёւ� ---
	if ( infoDec.flagSort ){
		switch( infoDec.numRead ){
			case 3:
			case 2:
				//--- 2���ږڂ�3���ږڂ͏������ق����ɂ��� ---
				if ( infoDec.flagM1 ){		// -1�͕ϊ����Ȃ�����
					sortTwoValM1(wmsecVal.early, wmsecVal.late);	// �͈͂͏��������ɕ��ёւ�
				}else{
					if (wmsecVal.early > wmsecVal.late){			// �͈͂͏��������ɕ��ёւ�
						swap(wmsecVal.early, wmsecVal.late);
					}
				}
				break;
			default:
				break;
		}
	}
	//--- �ǂݍ��ݐ����������܂œǂݍ��݈ʒu�X�V ---
	int numAbbr = 0;
	switch( infoDec.numRead ){
		case 3:
			if ( pos3 >= 0 || infoDec.needs >= 3 ){
				pos = pos3;
				numAbbr = 0;		// �ȗ��Ȃ�
			}else if ( pos2 >= 0 ){	// 3���ڒ�2���ڂ����ǂݏo�����ꍇ�͎��s�Ƃ���i�ݒ�~�X���������̂��߁j
				pos = -1;
				numAbbr = 1;
			}else if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 2;
			}else{
				numAbbr = 3;
			}
			break;
		case 2:
			if ( pos2 >= 0 || infoDec.needs >= 2 ){
				pos = pos2;
				numAbbr = 0;		// �ȗ��Ȃ�
			}else if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 1;
			}else{
				numAbbr = 2;
			}
			break;
		case 1:
			if ( pos1 >= 0 || infoDec.needs >= 1 ){
				pos = pos1;
				numAbbr = 0;		// �ȗ��Ȃ�
			}else{
				numAbbr = 1;
			}
			break;
		default:
			break;
	}
	infoDec.numAbbr  = numAbbr;
	infoDec.wmsecVal = wmsecVal;
	return pos;
}
//---------------------------------------------------------------------
// ���S�w�莞�ɑO�㓯�Ԋu�̃}�[�W����ݒ�
// �}�[�W���Ƀ}�C�i�X���w�肵���ꍇ�̓f�t�H���g�Ԋu��ݒ�
//---------------------------------------------------------------------
void JlsScriptDecode::setRangeMargin(WideMsec& wmsecVal, Msec margin){
	if ( margin >= 0 ){
		wmsecVal.early = wmsecVal.just - margin;
		wmsecVal.late  = wmsecVal.just + margin;
	}
	else{
		wmsecVal.early = wmsecVal.just - msecDecodeMargin;		// �f�t�H���g�l�F���S-1200ms;
		wmsecVal.late  = wmsecVal.just + msecDecodeMargin;		// �f�t�H���g�l�F���S-1200ms
	}
}
//---------------------------------------------------------------------
// �����񂩂�ԍ����X�g���擾�i-N�n�I�v�V�����p�j
//---------------------------------------------------------------------
bool JlsScriptDecode::getListStrNumFromStr(vector<string>& listStrNum, const string& strBuf){
	listStrNum.clear();
	bool success = true;
	int pos = 0;
	while(pos >= 0){		// comma��؂�ŕ����l�ǂݍ���
		string strVal;
		pos = pdata->cnv.getStrMultiNum(strVal, strBuf, pos);
		if ( pos >= 0 ){
			listStrNum.push_back(strVal);
		}else if ( !strVal.empty() ){
			success = false;
		}
	}
	if ( success ){
		if ( listStrNum.empty() ){
			success = false;
		}
	}
	return success;
}
//---------------------------------------------------------------------
// �����I�v�V�����̕��ёւ�
// ����-1�ȊO�̎��A�������l���ɂ���
//---------------------------------------------------------------------
void JlsScriptDecode::sortTwoValM1(int& val_a, int& val_b){
	if (val_a != -1 && val_b != -1){
		if (val_a > val_b){
			int tmp = val_a;
			val_a = val_b;
			val_b = tmp;
		}
	}
}


//=====================================================================
// �f�R�[�h��̒ǉ�����
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h�I�v�V�������e����͈͐ݒ���Đݒ�
//---------------------------------------------------------------------
void JlsScriptDecode::reviseCmdRange(JlsCmdArg& cmdarg){
	//--- ���S�w�� ---
	if ( cmdarg.isSetOpt(OptType::MsecDcenter) ){
		cmdarg.wmsecDst.just = cmdarg.getOpt(OptType::MsecDcenter);
		//--- �͈͊O�̎w��Ȃ�͈͂��Đݒ� ---
		if (cmdarg.wmsecDst.just < cmdarg.wmsecDst.early ||
			cmdarg.wmsecDst.just > cmdarg.wmsecDst.late){
			setRangeMargin(cmdarg.wmsecDst, -1);	// �͈̓}�[�W�����f�t�H���g�l�ɐݒ�
		}
	}
	//--- �͈͎w��i�}�[�W���w��j ---
	if ( cmdarg.isSetOpt(OptType::MsecDmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecDmargin));
		setRangeMargin(cmdarg.wmsecDst, msecMargin);	// �͈̓}�[�W���ݒ�
	}
	//--- �͈͐擪 ---
	if ( cmdarg.isSetOpt(OptType::MsecDrangeL) ){
		cmdarg.wmsecDst.early = cmdarg.getOpt(OptType::MsecDrangeL);
	}
	//--- �͈͖��� ---
	if ( cmdarg.isSetOpt(OptType::MsecDrangeR) ){
		cmdarg.wmsecDst.late = cmdarg.getOpt(OptType::MsecDrangeR);
	}
	//--- ���S���ݒ�{�͈͐ݒ莞�̒��S�␳ ---
	if ( !cmdarg.isSetOpt(OptType::MsecDcenter) &&
	     (cmdarg.isSetOpt(OptType::MsecDrangeL) || cmdarg.isSetOpt(OptType::MsecDrangeR)) ){
		if ( cmdarg.wmsecDst.just < cmdarg.wmsecDst.early && cmdarg.wmsecDst.early != -1 ){
			cmdarg.wmsecDst.just = cmdarg.wmsecDst.early;
		}
		else if ( cmdarg.wmsecDst.just > cmdarg.wmsecDst.late && cmdarg.wmsecDst.late != -1 ){
			cmdarg.wmsecDst.just = cmdarg.wmsecDst.late;
		}
	}

	//--- �I�v�V���� -Emargin ���w�肳�ꂽ���̏��� ---
	if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecEmargin));
		//--- EndLen�̈������ꕔ�ȗ����ꂽ�ꍇ ---
		if ( cmdarg.getOpt(OptType::AbbrEndlen) >= 2 ){	// �͈�2�����ȗ�
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndlenC);
			cmdarg.setOpt(OptType::MsecEndlenL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndlenR, msecCenter + msecMargin);
		}
		//--- EndSft�̈������ꕔ�ȗ����ꂽ�ꍇ ---
		if ( cmdarg.getOpt(OptType::AbbrEndSft) >= 2 ){	// �͈�2�����ȗ�
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndSftC);
			cmdarg.setOpt(OptType::MsecEndSftL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndSftR, msecCenter + msecMargin);
		}
		//--- Shift�̈������ꕔ�ȗ����ꂽ�ꍇ ---
		if ( cmdarg.getOpt(OptType::AbbrSft) >= 2 ){	// �͈�2�����ȗ�
			Msec msecCenter = cmdarg.getOpt(OptType::MsecSftC);
			cmdarg.setOpt(OptType::MsecSftL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecSftR, msecCenter + msecMargin);
		}
		//--- -SC�n�̏ȗ����m�F ---
		if ( m_listKeepSc.empty() == false ){
			int sizeSc = (int)m_listKeepSc.size();
			for(int i=0; i < sizeSc; i++){
				if ( m_listKeepSc[i].abbr >= 1 ){	// �͈͏ȗ���
					WideMsec wmsecVal = m_listKeepSc[i].wmsec;	// �ȗ����̒��S�w����g�p
					setRangeMargin(wmsecVal, msecMargin);		// �}�[�W���ݒ�
					m_listKeepSc[i].wmsec = wmsecVal;			// �����߂�
				}
			}
		}
	}
}
//---------------------------------------------------------------------
// �R�}���h�I�v�V�������e������s�I�v�V�����̐ݒ�
// �o�́F
//   cmdarg.tack  : �R�}���h��͌���
//---------------------------------------------------------------------
void JlsScriptDecode::setCmdTackOpt(JlsCmdArg& cmdarg){
	CmdType  cmdsel    = cmdarg.cmdsel;
	CmdCat   category  = cmdarg.category;
	//--- �����\��from ---
	bool comUsed = false;		// ���ʎg�p�̐ݒ�
	{
		bool comFrom = false;
		if ( cmdarg.getOpt(OptType::FnumFromAllC) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTr  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromSp  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromEc  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromBd  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromMx  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTra ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTrr ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromTrc ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromAea ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromAec ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromCm  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromNl  ) > 0 ||
		     cmdarg.getOpt(OptType::FnumFromL   ) > 0 ){
			comFrom = true;
		}
		cmdarg.tack.comFrom = comFrom;

		//--- -C�I�v�V�����t�� ---
		bool useScC = false;
		if ( cmdarg.getOptFlag(OptType::FlagScCon) || comFrom ){
			if ( !cmdarg.getOptFlag(OptType::FlagScCoff) &&
			     !cmdarg.getOptFlag(OptType::FlagScCdst) &&
			     !cmdarg.getOptFlag(OptType::FlagScCend) ){
				useScC = true;
			}
		}
		cmdarg.tack.useScC = useScC;

		//--- ���ʎg�p�̐ݒ� ---
		if ( cmdarg.getOptFlag(OptType::FlagScCon)  ||
		     cmdarg.getOptFlag(OptType::FlagScCdst) ||
		     cmdarg.getOptFlag(OptType::FlagScCend) ||
		     comFrom ){
			comUsed = true;
		}
	}
	//--- ��r�ʒu��Ώۈʒu�ɕύX ---
	{
		bool floatbase = false;
		if ( cmdsel == CmdType::Select ||
		     cmdsel == CmdType::NextTail ){				// �R�}���h�ɂ��ύX
			floatbase = true;
		}
		if (cmdarg.getOpt(OptType::FlagRelative) > 0){	// -relative
			floatbase = true;
		}
		cmdarg.tack.floatBase = floatbase;
	}
	//--- �V�t�g��ʒu ---
	{
		bool sft = false;
		if ( cmdarg.isSetOpt(OptType::MsecSftC) ){		// -shift
			sft = true;
		}
		cmdarg.tack.shiftBase = sft;
	}
	//--- ���S�𐄑��ʒu�ɕύX ---
	{
		bool vtlogo = false;
		if (category == CmdCat::AUTO ||
			category == CmdCat::AUTOEACH){				// Auto�n
			vtlogo = true;
		}
		if ( category == CmdCat::AUTOLOGO || comUsed ){		// ���S������Auto�n�Ɛ����\���g�p
			if ((OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNlogo &&	// -Nlogo�ȊO
			    (OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNFlogo &&	// -NFlogo�ȊO
			    (OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNFXlogo ){	// -NFXlogo�ȊO
				vtlogo = true;
			}
		}
		if ( (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNauto ||	// -Nauto
			 (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNFauto ){	// -NFauto
			if ( cmdsel == CmdType::GetPos  ||
				 cmdsel == CmdType::GetList ){
				vtlogo = true;
			}
		}
		if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){		// -final
			vtlogo = true;
		}
		cmdarg.tack.virtualLogo = vtlogo;
	}
	//--- ���SAbort��Ԃł����s����R�}���h ---
	{
		bool ignabort = false;
		if ( (OptType)cmdarg.getOpt(OptType::TypeNumLogo) == OptType::LgNFXlogo ){	// -NFXlogo
			if ( cmdsel == CmdType::GetPos  ||
				 cmdsel == CmdType::GetList ){
				ignabort = true;
			}
		}
		cmdarg.tack.ignoreAbort = ignabort;
	}
	//--- ���S�m���Ԃł����s����R�}���h ---
	{
		bool igncomp = false;
		if (cmdsel == CmdType::MkLogo  ||
			cmdsel == CmdType::DivLogo ||
			cmdsel == CmdType::DivFile ||
			cmdsel == CmdType::GetPos  ||
			cmdsel == CmdType::GetList){
			igncomp = true;
		}
		cmdarg.tack.ignoreComp = igncomp;
	}
	//--- �O��̃��S�ʒu�ȓ��ɔ͈͌��肷��ꍇ�i-nolap�w���DivLogo�R�}���h�j ---
	{
		bool limbylogo = false;
		if ( cmdarg.getOpt(OptType::FlagNoLap) > 0 ){		// -nolap�w�莞�Ɍ���
			limbylogo = true;
		}
		cmdarg.tack.limitByLogo = limbylogo;
	}
	//--- Auto�\����K�v�Ƃ���R�}���h ---
	{
		bool needauto = false;
		int numlist = cmdarg.sizeScOpt();
		if (numlist > 0){
			for(int i=0; i<numlist; i++){
				OptType sctype = cmdarg.getScOptType(i);
				if (sctype == OptType::ScAC || sctype == OptType::ScNoAC ||
				    sctype == OptType::ScACC || sctype == OptType::ScNoACC){
					needauto = true;
				}
			}
		}
		if ( comUsed ){
			needauto = true;
		}
		cmdarg.tack.needAuto = needauto;
	}
	//--- F�n����`���͈̔͐����펞�Ȃ� ---
	{
		bool fullA = false;
		bool fullB = false;
		if ( cmdsel == CmdType::GetPos  ||
			 cmdsel == CmdType::GetList ||
			 cmdsel == CmdType::NextTail ){
			fullA = true;
		}
		if ( cmdsel == CmdType::NextTail ){
			fullB = true;
		}
		cmdarg.tack.fullFrameA = fullA;
		cmdarg.tack.fullFrameB = fullB;
	}
	//--- �x�����s�̐ݒ��� ---
	{
		LazyType typelazy = LazyType::None;
		if ( cmdarg.getOpt(OptType::FlagLazyS) > 0 ) typelazy = LazyType::LazyS;
		if ( cmdarg.getOpt(OptType::FlagLazyA) > 0 ) typelazy = LazyType::LazyA;
		if ( cmdarg.getOpt(OptType::FlagLazyE) > 0 ) typelazy = LazyType::LazyE;
		cmdarg.tack.typeLazy = typelazy;
	}
	//--- ���ڃt���[���w��from ---
	{
		bool immfrom = false;
		if ( cmdarg.isSetStrOpt(OptType::ListFromAbs) ||
			 cmdarg.isSetStrOpt(OptType::ListFromHead) ||
			 cmdarg.isSetStrOpt(OptType::ListFromTail) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFD) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFE) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetFX) ||
			 cmdarg.isSetStrOpt(OptType::ListAbsSetXF) ){
			immfrom = true;
		}
		cmdarg.tack.immFrom = immfrom;
	}
	//--- Dst�ʒu�w��I�v�V�������� ---
	{
		bool existDstOpt = false;
		if ( cmdarg.isSetStrOpt(OptType::ListTgDst  ) ||
		     cmdarg.isSetStrOpt(OptType::ListDstAbs ) ||
		     cmdarg.isSetOpt(OptType::MsecDcenter   ) ||
		     cmdarg.isSetOpt(OptType::MsecDrangeL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstNextL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstPrevL   ) ||
		     cmdarg.isSetOpt(OptType::NumDstNextC   ) ||
		     cmdarg.isSetOpt(OptType::NumDstPrevC   ) ||
		     cmdarg.getOptFlag(OptType::FlagDstPoint) ){
			existDstOpt = true;
		}
		cmdarg.tack.existDstOpt = existDstOpt;
	}
	//--- �����ݒ� ---
	{
		bool fc = false;
		if ( cmdarg.getOpt(OptType::FlagForce) > 0 ||
		     cmdarg.getOpt(OptType::FlagNoForce) > 0 ||
		     cmdarg.getOpt(OptType::FlagFixPos) > 0 ){
			fc = true;
		}
		cmdarg.tack.forcePos = fc;
	}
	//--- �o�͑I�� ---
	{
		bool picki = false;
		bool picko = false;
		if ( cmdsel == CmdType::GetPos  ||
		     cmdsel == CmdType::GetList ||
		     cmdsel == CmdType::AutoIns ||
		     cmdsel == CmdType::AutoDel ){
			if ( cmdarg.isSetStrOpt(OptType::ListPickIn) ){
				if ( !cmdarg.getStrOpt(OptType::ListPickIn).empty() ){
					picki = true;
				}
			}
			if ( cmdarg.isSetStrOpt(OptType::ListPickOut) ){
				if ( !cmdarg.getStrOpt(OptType::ListPickOut).empty() ){
					picko = true;
				}
			}
		}
		cmdarg.tack.pickIn  = picki;
		cmdarg.tack.pickOut = picko;
	}
	//--- �e���S�ʃI�v�V������Auto�R�}���h ---
	{
		if (cmdsel == CmdType::AutoCut ||
			cmdsel == CmdType::AutoAdd){
			if (cmdarg.getOpt(OptType::FlagAutoEach) > 0){
				category = CmdCat::AUTOEACH;
				cmdarg.category = category;		// �I�v�V����(-autoeach)�ɂ��R�}���h�̌n�ύX
			}
		}
	}
}


//---------------------------------------------------------------------
// -SC�n�I�v�V������ݒ�
//---------------------------------------------------------------------
void JlsScriptDecode::setArgScOpt(JlsCmdArg& cmdarg){
	if ( m_listKeepSc.empty() == false ){
		int sizeSc = (int)m_listKeepSc.size();
		for(int i=0; i < sizeSc; i++){
			TargetCatType tgcat;
			switch( m_listKeepSc[i].subtype ){
				case 0:
					tgcat = TargetCatType::From;
					break;
				case 1:
					tgcat = TargetCatType::Dst;
					break;
				case 2:
					tgcat = TargetCatType::End;
					break;
				case 3:
					tgcat = TargetCatType::RX;
					break;
				default:
					tgcat = TargetCatType::None;
					break;
			}
			cmdarg.addScOpt(m_listKeepSc[i].type, tgcat,
			                m_listKeepSc[i].wmsec.early, m_listKeepSc[i].wmsec.late);
		}
	}
	//--- -C�n�I�v�V�����p�}�[�W���擾 ---
	WideMsec wmsec = {0,0,0};
	{
		Msec mgn = -1;
		if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
			mgn = abs(cmdarg.getOpt(OptType::MsecEmargin));
		}
		setRangeMargin(wmsec, mgn);
	}
	//--- -C�I�v�V�����ǉ� ---
	if ( cmdarg.tack.useScC ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::RX, wmsec.early, wmsec.late);
	}
	if ( cmdarg.getOptFlag(OptType::FlagScCdst) ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::Dst, wmsec.early, wmsec.late);
	}
	if ( cmdarg.getOptFlag(OptType::FlagScCend) ){
		cmdarg.addScOpt(OptType::ScAC, TargetCatType::End, wmsec.early, wmsec.late);
	}
}
//---------------------------------------------------------------------
// �I�v�V�����̖��w�莞����
//---------------------------------------------------------------------
void JlsScriptDecode::mirrorOptToUndef(JlsCmdArg& cmdarg){
	for(int i=0; i<(int)OptCmdMirror.size(); i++){
		if ( cmdarg.cmdsel == OptCmdMirror[i].cmdsel ){		// �ΏۃR�}���h
			OptType optTo = OptCmdMirror[i].optTypeTo;
			if ( !cmdarg.isSetStrOpt(optTo) ){			// �ΏۃI�v�V���������w�莞
				OptType optFrom = OptCmdMirror[i].optTypeFrom;
				cmdarg.setStrOpt(optTo, cmdarg.getStrOpt(optFrom) );
			}
		}
	}
}

//---------------------------------------------------------------------
// �������R�}���h�ʂɉ��Z���H
//---------------------------------------------------------------------
bool JlsScriptDecode::calcCmdArg(JlsCmdArg& cmdarg){
	bool success = true;
	//--- �e�[�u�������ԂɎQ�� ---
	for(int i=0; i<(int)CmdCalcDefine.size(); i++){
		//--- �ΏۃR�}���h���Ɏ��s ---
		if ( cmdarg.cmdsel == CmdCalcDefine[i].cmdsel ){
			int          nList   = CmdCalcDefine[i].numArg;		// �����̃��X�g�ԍ�
			ConvStrType  typeVal = CmdCalcDefine[i].typeVal;	// ���Z���

			//--- �����������ɍ��킹�ĉ��Z���H ---
			switch( typeVal ){
				case ConvStrType::CondIF :			// IF�������͕ϐ��̊m�F����̂Ō�Ŏ��s
					cmdarg.setNumCheckCond(nList);
					break;
				case ConvStrType::Param :
					if ( nList >= 2 ){	// �����Q�K�v
						string strName = cmdarg.getStrArg(nList-1);
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringRegParam(strName, strVal);
						if ( success ){
							success = cmdarg.replaceArgString(nList-1, strName);
						}
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
				case ConvStrType::Frame :
				case ConvStrType::Time :
					{
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringFromListStr(strVal, typeVal);	// ���X�g�Ή�
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
				default:
					{
						string strVal  = cmdarg.getStrArg(nList);
						success = convertStringValue(strVal, typeVal);
						if ( success ){
							success = cmdarg.replaceArgString(nList, strVal);
						}
					}
					break;
			}
		}
		if ( success == false ) break;
	}
	return success;
}

//=====================================================================
// ������ϊ�����
//=====================================================================

//---------------------------------------------------------------------
// ������̃��X�g�e���ڂ�ϊ����Č��̕�����ɖ߂�
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringFromListStr(string& strBuf, ConvStrType typeVal){
	bool success = true;
	if ( strBuf.empty() ){
		pFuncList->setListStrClear(strBuf);
		return success;
	}
	string strDst = "";
	int pos = 0;
	while( pos >= 0 ){		// comma��؂�ŕ����l�ǂݍ���
		string strTmp;
		pos = pdata->cnv.getStrWord(strTmp, strBuf, pos);
		if ( pos >= 0 ){
			if ( convertStringValue(strTmp, typeVal) ){
				pFuncList->setListStrIns(strDst, strTmp, -1);
			}else{
				success = false;
				pos = -1;
			}
		}
	}
	strBuf = strDst;
	return success;
}
//---------------------------------------------------------------------
// setParam�̕ϐ�����ԍ�������ɕϊ��A�l�����Z���Đ��l������ɕϊ�
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringRegParam(string& strName, string& strVal){
	//--- ���͕�����ɑΉ�����ԍ����擾 ---
	int csel = -1;
	{
		const char *varname = strName.c_str();
		//--- �����񂩂�p�����[�^������ ---
		for(int i=0; i<(int)ConfigDefine.size(); i++){
			if ( _stricmp(varname, ConfigDefine[i].namestr.c_str()) == 0 ){
				csel = i;
				break;
			}
		}
	}
	//--- ����������Z���ĕϊ� ---
	if ( csel >= 0 ){
		ConfigVarType typeParam  = ConfigDefine[csel].prmsel;
		ConvStrType   typeVal    = ConfigDefine[csel].valsel;
		strName = std::to_string((int)typeParam);		// ���O�͔ԍ��ɕϊ�
		return convertStringValue(strVal, typeVal);		// �l�͉��Z���ĕϊ�
	}
	return false;
}
//---------------------------------------------------------------------
// ����������Z���ĕϊ�������������i�[
//---------------------------------------------------------------------
bool JlsScriptDecode::convertStringValue(string& strVal, ConvStrType typeVal){
	int pos = 0;
	int val;
	switch( typeVal ){
		case ConvStrType::Msec :
			pos = pdata->cnv.getStrValMsec(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::MsecM1 :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Sec :
			pos = pdata->cnv.getStrValSec(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Num :
			pos = pdata->cnv.getStrValNum(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = to_string(val);
			}
			break;
		case ConvStrType::Frame :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = pdata->cnv.getStringFrameMsecM1(val);
			}
			break;
		case ConvStrType::Time :
			pos = pdata->cnv.getStrValMsecM1(val, strVal, 0);
			if ( pos >= 0 ){
				strVal = pdata->cnv.getStringTimeMsecM1(val);
			}
			break;
		case ConvStrType::NumR :
			pos = pdata->cnv.getStrMultiNum(strVal, strVal, 0);
			break;
		default:
			break;
	}
	if ( pos < 0 ){
		return false;
	}
	return true;
}
