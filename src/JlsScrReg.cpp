//
// �ϐ��̊i�[
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrReg.hpp"


///////////////////////////////////////////////////////////////////////
//
// �ϐ��N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// �ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsRegFile::setRegVar(const string& strName, const string& strVal, bool overwrite){
	int n;
	int nloc   = -1;
	int nlenvar = (int) strName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;
	string strPair;

	//--- �����ϐ��̏����������`�F�b�N ---
	for(int i=0; i<nMaxList; i++){
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		if (nlenvar == n){
			if (_stricmp(strName.c_str(), strOrgName.c_str()) == 0){
				nloc = i;
			}
		}
	}
	//--- �ݒ蕶����쐬 ---
	strPair = strName + ":" + strVal;
	//--- �����ϐ��̏������� ---
	if (nloc >= 0){
		if (overwrite){
			m_strListVar[nloc] = strPair;
		}
	}
	//--- �V�K�ϐ��̒ǉ� ---
	else{
		if (nMaxList < SIZE_VARNUM_MAX){		// �O�̂��ߕϐ��ő吔�܂�
			m_strListVar.push_back(strPair);
		}
		else{
			return false;
		}
	}
	return true;
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
int JlsRegFile::getRegVar(string& strVal, const string& strCandName, bool exact){
	int n;
	int nmatch = 0;
	int nloc   = -1;
	int nlencand = (int) strCandName.size();
	int nMaxList = (int) m_strListVar.size();
	string strOrgName, strOrgVal;

	//--- ���O�ƃ}�b�`����ʒu������ ---
	for(int i=0; i<nMaxList; i++){
		//--- �ϐ����ƒl������e�[�u������擾 ---
		n = getRegNameVal(strOrgName, strOrgVal, m_strListVar[i]);
		//--- �����e�[�u���ϐ����������܂ł̍ő��v��蒷����Ό��� ---
		if (nmatch < n){
			if (_strnicmp(strCandName.c_str(), strOrgName.c_str(), n) == 0 &&	// �擪�ʒu����}�b�`
				(n == nlencand || exact == false)){								// ���ꕶ����exact=false
				nloc   = i;
				nmatch = n;
			}
		}
	}
	//--- �}�b�`�����ꍇ�̒l�̓ǂݏo�� ---
	if (nloc >= 0){
		n = getRegNameVal(strOrgName, strVal, m_strListVar[nloc]);			// �ϐ��l���o��
		if ( strOrgName != strCandName.substr(0, n) ){
			msgErr += "warning : mismatch capital letter of register name(";
			msgErr += strCandName.substr(0, n) + " " + strOrgName + ")\n";
		}
	}
	return nmatch;
}

//---------------------------------------------------------------------
// �i�[�ϐ��𖼑O�ƒl�ɕ����i�ϐ��ǂݏ����֐�����̃T�u���[�`���j
//---------------------------------------------------------------------
int JlsRegFile::getRegNameVal(string& strName, string& strVal, const string& strPair){
	//--- �ŏ��̃f���~�^���� ---
	int n = (int) strPair.find(":");
	//--- �f���~�^�𕪉����ďo�͂ɐݒ� ---
	if (n > 0){
		strName = strPair.substr(0, n);
		int nLenPair = (int) strPair.length();
		if (n < nLenPair-1){
			strVal = strPair.substr(n+1);
		}
		else{
			strVal = "";
		}
	}
	return n;
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�����݂�������o��
// �o�́F
//   �Ԃ�l   : �G���[���b�Z�[�W�L���i0=�Ȃ��A1=����j
//   msg      : �擾�����G���[���b�Z�[�W
//---------------------------------------------------------------------
bool JlsRegFile::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}


///////////////////////////////////////////////////////////////////////
//
// �K�w�\���ϐ��N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ���[�J���ϐ��K�w���쐬
// ���́F
//   flagBase  : �����K�w�i0=��ʊK�w��������  1=�ŏ�ʊK�w�����j
// �o�́F
//   �Ԃ�l    : �쐬�K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrReg::createLocal(bool flagBase){
	if ( layerReg.size() >= INT_MAX/4 ){		// �O�̂��߃T�C�Y����
		msgErr += "error:too many create local-register\n";
		return 0;
	}
	RegLayer layer;
	layer.base   = flagBase;
	//--- ���[�J���ϐ��K�w���쐬 ---
	layerReg.push_back(layer);
	return (int) layerReg.size();
}
//---------------------------------------------------------------------
// ���[�J���ϐ��K�w�̏I��
// ���́F
//   flagBase  : �I�����K�w�i0=��ʊK�w�������K�w�̂�  1=�ŏ�ʊK�w�����j
// �o�́F
//   �Ԃ�l    : �I���K�w�i0=���s�A1�ȏ�=�K�w�j
//---------------------------------------------------------------------
int JlsScrReg::releaseLocal(bool flagBase){
	int numLayer = 0;
	if ( layerReg.empty() == false ){
		numLayer = (int) (layerReg.size() - 1);
		if ( flagBase || (layerReg[numLayer].base == false) ){	// �I������
			layerReg.pop_back();
			numLayer = (int) (layerReg.size() + 1);	
		}
	}
	if ( numLayer == 0 ){
		msgErr += "error:too many release local-register layer\n";
		return 0;
	}
	return numLayer;
}
//---------------------------------------------------------------------
// ���[�J���ϐ���ݒ�
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsScrReg::setLocalRegVar(const string& strName, const string& strVal, bool overwrite){
	if ( layerReg.empty() ){	// ���[�J���ϐ��K�w�̑��݂�O�̂��߃`�F�b�N
		msgErr += "error:internal setting(empty local-register layer)\n";
		return false;
	}
	if ( checkErrRegName(strName) ){	// �ϐ����ُ펞�̏I��
		return false;
	}
	//--- ���݂̃��[�J���ϐ��K�w�ɏ������� ---
	int numLayer = (int) (layerReg.size() - 1);
	return layerReg[numLayer].regfile.setRegVar(strName, strVal, overwrite);
}
//---------------------------------------------------------------------
// �ϐ���ݒ�i���[�J���ϐ��ɑ��݂�����D��A�Ȃ���΃O���[�o���ϐ��Ɂj
// ���́F
//   strName   : �ϐ���
//   strVal    : �ϐ��l
//   overwrite : 0=����`���̂ݐݒ�  1=�㏑�����ݒ�
// �o�́F
//   �Ԃ�l    : �ʏ�=true�A���s��=false
//---------------------------------------------------------------------
bool JlsScrReg::setRegVar(const string& strName, const string& strVal, bool overwrite){
	if ( checkErrRegName(strName) ){	// �ϐ����ُ펞�̏I��
		return false;
	}
	//--- ���[�J���ϐ��ɑ��݂��邩���� ---
	int numLayer;
	{
		bool exact = true;		// ���͕����Ɗ��S��v����ϐ�
		string strTmp;
		getLayerRegVar(numLayer, strTmp, strName, exact);
	}
	//--- ���[�J���ϐ��ɑ��݂�����D��A�Ȃ���΃O���[�o���ϐ��ɏ������� ---
	bool success;
	if ( numLayer >= 0 ){
		success = layerReg[numLayer].regfile.setRegVar(strName, strVal, overwrite);
	}else{
		success = globalReg.setRegVar(strName, strVal, overwrite);
	}
	return success;
}
//---------------------------------------------------------------------
// �ϐ���ǂݏo���i���[�J���ϐ��D��A�Ȃ���΃O���[�o���ϐ��j
// ���́F
//   strCandName : �ǂݏo���ϐ����i���j
//   excact      : 0=���͕����ɍő�}�b�`����ϐ�  1=���͕����Ɗ��S��v����ϐ�
// �o�́F
//   �Ԃ�l  : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   strVal  : �ϐ��l
//---------------------------------------------------------------------
int JlsScrReg::getRegVar(string& strVal, const string& strCandName, bool exact){
	//--- ���[�J���ϐ����猟�� ---
	int numLayer;
	int numMatch = getLayerRegVar(numLayer, strVal, strCandName, exact);
	//--- ���[�J���ϐ��ɂȂ���΃O���[�o���ϐ��ǂݍ��� ---
	if ( numLayer < 0 ){
		numMatch = globalReg.getRegVar(strVal, strCandName, exact);
		if ( numMatch > 0 ){
			popErrLower(globalReg);
		}
	}
	return numMatch;
}
//---------------------------------------------------------------------
// �G���[���b�Z�[�W�����݂�������o��
// �o�́F
//   �Ԃ�l   : �G���[���b�Z�[�W�L���i0=�Ȃ��A1=����j
//   msg      : �擾�����G���[���b�Z�[�W
//---------------------------------------------------------------------
bool JlsScrReg::popMsgError(string& msg){
	if ( msgErr.empty() ){
		return false;
	}
	msg = msgErr;
	msgErr = "";
	return true;
}

//---------------------------------------------------------------------
// ���[�J���ϐ��K�w����ϐ���ǂݏo��
// �o�́F
//   �Ԃ�l   : �ϐ����̕������i0�̎��͑Ή��ϐ��Ȃ��j
//   numLayer : �ǂݏo�������[�J���ϐ��K�w�i-1:�Y���Ȃ��A0�X�^�[�g�̊K�w�j
//   strVal   : �ϐ��l
//---------------------------------------------------------------------
int JlsScrReg::getLayerRegVar(int& numLayer, string& strVal, const string& strCandName, bool exact){
	numLayer = -1;
	int numMatch = 0;
	if ( layerReg.empty() == false ){	// ���[�J���ϐ��K�w������ꍇ�̂݌���
		int n = (int) layerReg.size();
		bool scope = true;
		//--- ���ʊK�w���猟�����K�w�܂ŕϐ����� ---
		while( scope && n > 0 ){
			n --;
			string str;
			int nmatch = layerReg[n].regfile.getRegVar(str, strCandName, exact);
			if ( nmatch > 0 ){				// �ϐ�����
				numLayer = n;
				numMatch = nmatch;
				strVal   = str;
				scope    = false;
				popErrLower(layerReg[n].regfile);
			}else if ( layerReg[n].base ){	// ��ʊK�w���������Ȃ��K�w
				scope    = false;
			}
		}
	}
	return numMatch;
}
//---------------------------------------------------------------------
// �ϐ����̍Œ���̈ᔽ�����m�F
// �o�́F
//   �Ԃ�l   : �G���[�L���i0=����A1=�G���[����j
//---------------------------------------------------------------------
bool JlsScrReg::checkErrRegName(const string& strName){
	//--- �Œ���̈ᔽ�����m�F ---
	string strCheckFull  = "!#$%&'()*+,-./:;<=>?";			// �ϐ�������Ƃ��Ďg�p�֎~
	string strCheckFirst = strCheckFull + "0123456789";		// �ϐ��擪�����Ƃ��Ďg�p�֎~
	string strFirst = strName.substr(0, 1);
	if ( strCheckFirst.find(strFirst) != string::npos ){
		msgErr += "error: register setting, invalid first char(" + strName + ")\n";
		return true;
	}
	for(int i=0; i < (int)strCheckFull.length(); i++){
		string strNow = strCheckFull.substr(i, 1);
		if ( strName.find(strNow) != string::npos ){
			msgErr += "error: register setting, bad char exist(" + strName + ")\n";
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------
// ���ʊK�w�̃G���[�擾
//---------------------------------------------------------------------
bool JlsScrReg::popErrLower(JlsRegFile& regfile){
	string msgTmp;
	if ( regfile.popMsgError(msgTmp) ){
		msgErr += msgTmp;
		return true;
	}
	return false;
}
