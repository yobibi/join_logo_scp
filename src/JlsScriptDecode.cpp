//
// ���s�X�N���v�g�R�}���h��������
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptDecode.hpp"
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
JlsScriptDecode::JlsScriptDecode(JlsDataset *pdata){
	this->pdata  = pdata;
}

//---------------------------------------------------------------------
// �����ݒ�ُ̈�m�F
//---------------------------------------------------------------------
void JlsScriptDecode::checkInitial(){
	for(int i=0; i<SIZE_JLCMD_SEL; i++){
		if ( CmdDefine[i].cmdsel != (CmdType) i ){
			cerr << "error:internal mismatch at CmdDefine.cmdsel " << i << endl;
		}
	}
	for(int i=0; i<SIZE_CONFIG_VAR; i++){
		if ( ConfigDefine[i].prmsel != (ConfigVarType) i ){
			cerr << "error:internal mismatch at ConfigDefine.prmsel " << i << endl;
		}
	}
	if (strcmp(OptDefine[SIZE_JLOPT_DEFINE-1].optname, "-dummy") != 0){
		cerr << "error:internal mismatch at OptDefine.data1 ";
		cerr << OptDefine[SIZE_JLOPT_DEFINE-1].optname << endl;
	}
}

//=====================================================================
// �f�R�[�h����
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h���e�𕶎���P�s������
// ���́F
//  strBuf  : ��͕�����
//  onlyCmd : �擪�̃R�}���h���������̉�͂��i0=�S�́A1=�R�}���h�̂݁j
// �o�́F
//   �Ԃ�l�F�G���[���
//   cmdarg: �R�}���h��͌���
//---------------------------------------------------------------------
CmdErrType JlsScriptDecode::decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd){
	CmdErrType retval = CmdErrType::None;

	//--- �R�}���h���e������ ---
	cmdarg.clear();
	//--- �R�}���h��t(cmdsel) ---
	string strCmd;
	int csel = 0;
	int pos = pdata->cnv.getStrItem(strCmd, strBuf, 0);
	if (pos >= 0){
		csel = decodeCmdId(strCmd);
	}
	//--- �R�}���h�ُ펞�̏I�� ---
	if (csel < 0){
		retval = CmdErrType::ErrCmd;
		return retval;
	}

	//--- �R�}���h���ݒ� ---
	CmdType  cmdsel   = CmdDefine[csel].cmdsel;
	CmdCat   category = CmdDefine[csel].category;
	int muststr   = CmdDefine[csel].muststr;
	int mustchar  = CmdDefine[csel].mustchar;
	int mustrange = CmdDefine[csel].mustrange;
	int needopt   = CmdDefine[csel].needopt;
	cmdarg.cmdsel = cmdsel;
	cmdarg.category = category;

	//--- �R�}���h��t�݂̂ŏI������ꍇ ---
	if ( onlyCmd ){
		return retval;
	}

	//--- �R�}���h��� ---
	if ( muststr > 0 || mustchar > 0 || mustrange > 0 ){
		pos = decodeCmdArgMust(cmdarg, retval, strBuf, pos, muststr, mustchar, mustrange);
	}

	//--- �I�v�V������t ---
	if (needopt > 0 && pos >= 0){
		pos = decodeCmdArgOpt(cmdarg, retval, strBuf, pos);
	}
	//--- ���������Z���H ---
	if ( muststr > 0 ){
		bool success = calcCmdArg(cmdarg);
		if ( success == false ){
			retval = CmdErrType::ErrOpt;
		}
	}

	return retval;
}

//---------------------------------------------------------------------
// �R�}���h�����擾
// �o�́F
//   �Ԃ�l  �F�擾�R�}���h�ԍ��i���s����-1�j
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdId(const string& cstr){
	int det = -1;
	const char *cmdname = cstr.c_str();

	if (cmdname[0] == '\0' || cmdname[0] == '#'){
		det = 0;
	}
	else{
		for(int i=0; i<SIZE_JLCMD_SEL; i++){
			if (_stricmp(cmdname, CmdDefine[i].cmdname) == 0){
				det = i;
				break;
			}
		}
		//--- ������Ȃ���Εʖ������� ---
		if (det < 0){
			bool flag = false;
			CmdType target;
			for(int i=0; i<SIZE_JLSCR_CMDALIAS; i++){
				if (_stricmp(cmdname, CmdAlias[i].cmdname) == 0){
					target = CmdAlias[i].cmdsel;
					flag = true;
					break;
				}
			}
			if ( flag ){
				for(int i=0; i<SIZE_JLCMD_SEL; i++){
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
//   tps: ����������i0-3=�擾�� 9=�c��S�́j
//   tpc: ��ސݒ�i0=�ݒ�Ȃ�  1=S/E/B  2=TR/SP/EC 3=�ȗ��\��S/E/B�j
//   tpw: ���Ԑݒ�i0=�ݒ�Ȃ�  1=center  3=center+left+right�j
// �o�́F
//   �Ԃ�l  : �ǂݍ��݈ʒu�i-1=�I�v�V�����ُ�j
//   errval  : �G���[�ԍ�
//   cmdarg  : �R�}���h��͌���
//---------------------------------------------------------------------
int JlsScriptDecode::decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, int tps, int tpc, int tpw){
	//--- ������Ƃ��Ĉ������擾 ---
	if ( tps > 0 && pos >= 0){
		if ( tps == 9 ){
			//--- �c��S���𕶎���Ƃ��Ď擾 ---
			while( strBuf[pos] == ' ' && pos >= 0) pos++;
			string strArg = strBuf.substr(pos);
			cmdarg.addArgString(strArg);
		}else{
			int sizeArg = tps;
			for(int i=0; i<sizeArg; i++){
				string strArg;
				pos = pdata->cnv.getStrItem(strArg, strBuf, pos);
				if ( pos >= 0 ){
					cmdarg.addArgString(strArg);
				}
			}
			if (pos < 0){
				errval = CmdErrType::ErrOpt;
			}
		}
	}
	//--- ��ޕ��� ---
	if (tpc > 0 && pos >= 0){
		string strTmp;
		int posbak = pos;
		pos = pdata->cnv.getStrItem(strTmp, strBuf, pos);
		if (pos >= 0){
			//--- ���ڂP�i�����w��j ---
			if (tpc == 1 || tpc == 3){
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
					if (tpc == 3) pos = posbak;
					else{
						pos    = -1;
						errval = CmdErrType::ErrSEB;
					}
				}
			}
			//--- TR/SP/EC �����񔻕� ---
			else if (tpc == 2){
				bool flagOption = false;
				if ( getTrSpEcID(cmdarg.selectAutoSub, strTmp, flagOption) == false ){
					pos    = -1;
					errval = CmdErrType::ErrTR;
				}
			}
		}
	}
	//--- �͈͎w�� ---
	if (tpw > 0 && pos >= 0){
		if (tpw == 1 || tpw == 3){
			JlscrDecodeRangeRecord infoDec = {};
			if (tpw == 1){
				infoDec.numRead  = 1;		// �f�[�^�ǂݍ��ݐ�=1
				infoDec.needs    = 0;		// �Œ�ǂݍ��ݐ�=0�i�S���ڏȗ��j
				infoDec.numFrom  = 0;		// �ȗ����̊J�n�w��Ȃ��i�W������j
				infoDec.flagM1   = false;	// -1�͒ʏ�̐��l�Ƃ��ēǂݍ���
				infoDec.flagSort = false;	// �P�f�[�^�Ȃ̂ŕ��ёւ��Ȃ�
			}
			else if (tpw == 3){
				infoDec.numRead  = 3;		// �f�[�^�ǂݍ��ݐ�=3
				infoDec.needs    = 0;		// �Œ�ǂݍ��ݐ�=0�i�S���ڏȗ��j
				infoDec.numFrom  = 0;		// �ȗ����̊J�n�w��Ȃ��i�W������j
				infoDec.flagM1   = false;	// -1�͒ʏ�̐��l�Ƃ��ēǂݍ���
				infoDec.flagSort = true;	// ���������̕��ёւ�����
			}
			pos = decodeRangeMsec(infoDec, strBuf, pos);
			cmdarg.wmsecDst = infoDec.wmsecVal;
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
	while(pos >= 0){
		pos = decodeCmdArgOptOne(cmdarg, errval, strBuf, pos);
	}
	reviseCmdRange(cmdarg);		// �I�v�V�����ɂ��͈͕␳
	setCmdTackOpt(cmdarg);		// ���s�I�v�V�����ݒ�
	setArgScOpt(cmdarg);		// �ꎞ�ێ�����-SC�n�I�v�V������ݒ�
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
	string strWord;
	pos = pdata->cnv.getStrItem(strWord, strBuf, pos);
	if (pos >= 0){
		//--- �R�����g���� ---
		int poscut = (int) strWord.find("#");
		if (poscut == 0){
			pos = -1;
		}
		else if (poscut > 0){
			strWord = strWord.substr(0, poscut);
		}
	}
	int optsel = -1;
	if (pos >= 0){
		//--- �I�v�V�������� ---
		const char *pstr = strWord.c_str();
		for(int i=0; i<SIZE_JLOPT_DEFINE; i++){
			if (!_stricmp(pstr, OptDefine[i].optname)){
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
				string strSub;
				pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
				if ( pos >= 0 ){
					vector<string> listStrNum;
					if ( getListStrNumFromStr(listStrNum, strSub) ){
						for(int i=0; i < (int)listStrNum.size(); i++){
							cmdarg.addLgOpt(listStrNum[i]);
						}
					}else{
						pos = -1;
					}
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
					keepSc.relative = (OptDefine[optsel].subType != 0)? true : false;
					keepSc.wmsec    = infoDec.wmsecVal;
					keepSc.abbr     = infoDec.numAbbr;
					m_listKeepSc.push_back(keepSc);		// ��Ŕ͈͕␳������̂ňꎞ�ێ�
				}
			}
			break;
		case OptCat::STR :						// ������̐ݒ�
			{
				string strSub;
				pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
				if ( pos >= 0 ){
					if ( strSub[0] != '-' ){
						cmdarg.setStrOpt(optType, strSub);
					}else{
						pos = -1;				// ����"-"���������̓I�v�V�����p������ł͂Ȃ�
					}
				}
			}
			break;
		case OptCat::NUM :						// ���l�̐ݒ�
			{
				vector<OptType> listOptType(4);
				int numUsed = getOptionTypeList(listOptType, optType, infoDec.numRead);
				int listVal[3];
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
							pos = pdata->cnv.getStrItem(strSub, strBuf, pos);
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
	cerr << "error:internal setting" << msg << endl;
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
		string strTmp;
		pos = pdata->cnv.getStrWord(strTmp, strBuf, pos);
		if (pos >= 0){
			int rloc = (int)strTmp.find("..");
			if ( rloc != (int)string::npos ){			// ..�ɂ��͈͐ݒ莞
				string strSt = strTmp.substr(0, rloc);
				string strEd = strTmp.substr(rloc+2);
				int valSt;
				int valEd;
				int posSt = pdata->cnv.getStrValNum(valSt, strSt, 0);
				int posEd = pdata->cnv.getStrValNum(valEd, strEd, 0);
				if ( posSt >= 0 && posEd >= 0 ){
					string strValSt = std::to_string(valSt);
					string strValEd = std::to_string(valEd);
					string strVal = strValSt + ".." + strValEd;
					listStrNum.push_back(strVal);
				}else{
					success = false;
				}
			}else{
				int val1;
				if (pdata->cnv.getStrValNum(val1, strTmp, 0) >= 0){
					string strVal = std::to_string(val1);
					listStrNum.push_back(strVal);
				}else{
					success = false;
				}
			}
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

	//--- �I�v�V���� -Emargin ���w�肳�ꂽ���̏��� ---
	if ( cmdarg.isSetOpt(OptType::MsecEmargin) ){
		Msec msecMargin = abs(cmdarg.getOpt(OptType::MsecEmargin));
		//--- EndLen�̈������ꕔ�ȗ����ꂽ�ꍇ ---
		if ( cmdarg.getOpt(OptType::AbbrEndlen) >= 2 ){	// �͈�2�����ȗ�
			Msec msecCenter = cmdarg.getOpt(OptType::MsecEndlenC);
			cmdarg.setOpt(OptType::MsecEndlenL, msecCenter - msecMargin);
			cmdarg.setOpt(OptType::MsecEndlenR, msecCenter + msecMargin);
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
	//--- ��r�ʒu��Ώۈʒu�ɕύX ---
	{
		bool floatbase = false;
		if ( cmdsel == CmdType::Select ||
		     cmdsel == CmdType::NextTail ){				// �R�}���h�ɂ��ύX
			floatbase = true;
		}
		if (cmdarg.isSetOpt(OptType::MsecSftC) ||		// -shift
			cmdarg.getOpt(OptType::FlagRelative) > 0){	// -relative
			floatbase = true;
		}
		cmdarg.tack.floatBase = floatbase;
	}
	//--- ���S�𐄑��ʒu�ɕύX ---
	{
		bool vtlogo = false;
		if (category == CmdCat::AUTO ||
			category == CmdCat::AUTOEACH){				// Auto�n
			vtlogo = true;
		}
		if (category == CmdCat::AUTOLOGO &&				// ���S������Auto�n
			(OptType)cmdarg.getOpt(OptType::TypeNumLogo) != OptType::LgNlogo){	// -Nlogo�ȊO
			vtlogo = true;
		}
		if ( cmdarg.getOpt(OptType::FlagFinal) > 0 ){		// -final
			vtlogo = true;
		}
		cmdarg.tack.virtualLogo = vtlogo;
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
		if (cmdsel == CmdType::DivLogo){
			limbylogo = true;
		}
		if ( cmdarg.getOpt(OptType::FlagNoLap) > 0 ){		// -nolap�w�莞�Ɍ���
			limbylogo = true;
		}
		cmdarg.tack.limitByLogo = limbylogo;
	}
	//--- ��Έʒu�w�莞�̃��S�����͂P�ӏ��݂̂ɂ��� ---
	{
		bool onepoint = false;
		if ( cmdarg.isSetOpt(OptType::MsecFromAbs)  ||	// -fromabs
			 cmdarg.isSetOpt(OptType::MsecFromHead) ||	// -fromhead
			 cmdarg.isSetOpt(OptType::MsecFromTail) ){	// -fromtail
			onepoint = true;
		}
		if (cmdsel == CmdType::GetPos){	// �P�ӏ��̂݌���
			onepoint = true;
		}
		cmdarg.tack.onePoint = onepoint;
	}
	//--- Auto�\����K�v�Ƃ���R�}���h ---
	{
		bool needauto = false;
		int numlist = cmdarg.sizeScOpt();
		if (numlist > 0){
			for(int i=0; i<numlist; i++){
				OptType sctype = cmdarg.getScOptType(i);
				if (sctype == OptType::ScAC || sctype == OptType::ScNoAC){
					needauto = true;
				}
			}
		}
		cmdarg.tack.needAuto = needauto;
	}
	//--- F�n����`���͈̔͐����펞�Ȃ� ---
	{
		bool full = false;
		if ( cmdsel == CmdType::GetPos  ||
			 cmdsel == CmdType::GetList ){
			full = true;
		}
		cmdarg.tack.fullFrame = full;
	}
	//--- �x�����s�̐ݒ��� ---
	{
		LazyType typelazy = LazyType::None;
		if ( cmdarg.getOpt(OptType::FlagLazyS) > 0 ) typelazy = LazyType::LazyS;
		if ( cmdarg.getOpt(OptType::FlagLazyA) > 0 ) typelazy = LazyType::LazyA;
		if ( cmdarg.getOpt(OptType::FlagLazyE) > 0 ) typelazy = LazyType::LazyE;
		cmdarg.tack.typeLazy = typelazy;
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
			cmdarg.addScOpt(m_listKeepSc[i].type, m_listKeepSc[i].relative,
			                m_listKeepSc[i].wmsec.early, m_listKeepSc[i].wmsec.late);
		}
	}
}

//---------------------------------------------------------------------
// �������R�}���h�ʂɉ��Z���H
//---------------------------------------------------------------------
bool JlsScriptDecode::calcCmdArg(JlsCmdArg& cmdarg){
	bool success = true;
	//--- �e�[�u�������ԂɎQ�� ---
	for(int i=0; i<SIZE_JLCMD_CALC_DEFINE; i++){
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
				default:
					string strVal  = cmdarg.getStrArg(nList);
					success = convertStringValue(strVal, typeVal);
					if ( success ){
						success = cmdarg.replaceArgString(nList, strVal);
					}
					break;
			}
		}
		if ( success == false ) break;
	}
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
		for(int i=0; i<SIZE_CONFIG_VAR; i++){
			if ( _stricmp(varname, ConfigDefine[i].namestr) == 0 ){
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
		default:
			break;
	}
	if ( pos < 0 ){
		return false;
	}
	return true;
}
