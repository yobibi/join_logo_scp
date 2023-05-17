//
// ������Ǝ��Ԃƃt���[���ʒu�̑��ݕϊ��N���X
//
#include "stdafx.h"

using namespace std;
using Msec = int;
#include "CnvStrTime.hpp"


//---------------------------------------------------------------------
// �\�z�������ݒ�
//---------------------------------------------------------------------
CnvStrTime::CnvStrTime(){
	m_frate_n = 30000;
	m_frate_d = 1001;
	m_unitsec = 0;
	m_delimiter = "\\";		// �p�X��؂�͋N�����w��p�X�̎g�p�����Ŏ����I�ɐ؂�ւ��
}



//=====================================================================
// �t�@�C�����E�p�X�̕�������
//=====================================================================

//---------------------------------------------------------------------
// ������͂��ׂăp�X�����Ƃ��čŌ�ɋ�؂肪�Ȃ���Εt��
// �t�������ꍇ�͕Ԃ�l��true�A���̂܂܂Ȃ�false
// ���́F
//   pathname : �p�X��
// �o�́F
//   pathname : �p�X���i�Ō�͋�؂蕶���j
//---------------------------------------------------------------------
bool CnvStrTime::getStrFileAllPath(string &pathname){
	string strTmp;
	getStrFilePath(strTmp, pathname);	// ��؂�܂ł̕����擾
	if ( strTmp != pathname ){		// ��؂�܂ł̕������S�̂��m�F
		string delimiter = getStrFileDelimiter();	// ��؂蕶��
		pathname += delimiter;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
// �����񂩂�t�@�C���p�X�����ƃt�@�C���������𕪗�
// �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
// ���́F
//   fullpath : �t���p�X��
// �o�́F
//   pathname : �p�X����
//---------------------------------------------------------------------
int CnvStrTime::getStrFilePath(string &pathname, const string &fullname){
	string strTmp;
	return getStrFilePathName(pathname, strTmp, fullname);
}

//---------------------------------------------------------------------
// �����񂩂�t�@�C���p�X�����ƃt�@�C���������𕪗�
// �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
// ���́F
//   fullpath : �t���p�X��
// �o�́F
//   pathname : �p�X����
//   fname    : ���O�ȍ~����
//---------------------------------------------------------------------
int CnvStrTime::getStrFilePathName(string &pathname, string &fname, const string &fullname){
	bool flag_find = false;
	//--- "\"��؂������ ---
	int nloc = (int) fullname.rfind("\\");
	if (nloc >= 0){
		flag_find = true;
		m_delimiter = "\\";		// ��؂蕶���ύX
	}
	//--- "/"��؂������ ---
	int nloc_sl = (int) fullname.rfind("/");
	if (nloc_sl >= 0){
		if (flag_find == false || nloc < nloc_sl){
			flag_find = true;
			nloc = nloc_sl;
			m_delimiter = "/";		// ��؂蕶���ύX
		}
	}
	if (flag_find){
		pathname = fullname.substr(0, nloc+1);
		fname    = fullname.substr(nloc+1);
	}
	else{
		pathname = "";
		fname = fullname;
		nloc = -1;
	}
	return nloc;
}

//---------------------------------------------------------------------
// �t�@�C���̋�؂蕶���擾
//---------------------------------------------------------------------
string CnvStrTime::getStrFileDelimiter(){
	return m_delimiter;
}

//---------------------------------------------------------------------
// �o�b�t�@�T�C�Y�擾
//---------------------------------------------------------------------
int CnvStrTime::getBufLineSize(){
	return SIZE_BUF_MAX;
}
//---------------------------------------------------------------------
// �֐��^�C�v�̈������W���[�����ł��邩�m�F
//---------------------------------------------------------------------
bool CnvStrTime::isStrFuncModule(const string &cstr, int pos){
	//--- ���W���[���� ---
	string strWord;
	pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_NAME);
	if ( pos < 0 ) return false;
	//--- �����擪�擾 ---
	string strNext;
	int posnext = getStrItemHubFunc(strNext, cstr, pos, DELIMIT_FUNC_ARGS);
	if ( posnext < 0 ) return false;
	if ( strNext[0] != '(' ) return false;		// "("�ȊO�Ȃ�֐��ł͂Ȃ�
	//--- ���W���[�����ȈՊm�F ---
	bool match = true;
	for(int i=0; i<(int)strWord.length(); i++){
		char ch = strWord[i];
		if ( ch >= 0 && ch <= 0x7F ){
			if ( (ch >= '0' && ch <= '9') ||
			     (ch >= 'A' && ch <= 'Z') ||
			     (ch >= 'a' && ch <= 'z') ||
			     (ch == '_') ){
			}else{
				match = false;
			}
		}
	}
	return match;
}
//---------------------------------------------------------------------
// ���W���[�����ƈ����̃��X�g���擾 �t�H�[�}�b�g�F ���W���[����(���� ���� ...)
//---------------------------------------------------------------------
int CnvStrTime::getListModuleArg(vector<string>& listMod, const string &cstr, int pos){
	listMod.clear();
	string strWord;
	//--- ���W���[�����ݒ� ---
	pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_NAME);
	if ( pos < 0 ) return pos;
	listMod.push_back(strWord);
	//--- �����擪�擾 ---
	int posBak = pos;
	pos = skipCharSpace(cstr, pos);
	if ( cstr[pos] != '(' ){		// "("�ȊO�Ȃ�����Ȃ��ŏI��
		return posBak;
	}
	pos = getStrItemHubFunc(strWord, cstr, pos+1, DELIMIT_FUNC_ARGS);
	//--- �����ݒ� ---
	bool cont = true;
	while( cont && pos >= 0 ){
		if ( strWord == ")" ){
			listMod.push_back("");	// �Ō�ɋ󕶎�����i�[
			cont = false;
		}else{
			listMod.push_back(strWord);
			pos = getStrItemHubFunc(strWord, cstr, pos, DELIMIT_FUNC_ARGS);
		}
	}
	return pos;
}
//---------------------------------------------------------------------
// �p�X�ŕ���������������擾
// ���́F
//   strDiv   : �t���p�X��
//   selHead  : �����̏o�́itrue=�O���Afalse=�㑤�j
//   withDelim : true=�O���o�͎��ɋ�؂�ʒu�̋�؂���o��
// �o�́F
//   �Ԃ�l : ��؂葶�ݗL��
//   strDiv   : ��؂蕪���㕶����iHead=�O���BTail=�㑤�j
//---------------------------------------------------------------------
bool CnvStrTime::getStrDivPath(string& strDiv, bool selHead, bool withDelim){
	int pos = getStrPosPath(strDiv);
	if ( selHead ){
		if ( pos < 0 ){
			strDiv.clear();
			return false;
		}
		if ( withDelim ){	// ��؂�܂�
			pos ++;
		}
		if ( pos == 0 ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(0, pos);
		}
	}else{
		if ( pos < 0 ){
			return false;
		}
		pos ++;		// ��؂������
		if ( pos >= (int) strDiv.length() ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(pos);
		}
	}
	return true;
}
//---------------------------------------------------------------------
// ��؂�ŕ���������������擾�i�g���q�擾�p�j
// ���́F
//   strDiv   : �t���p�X��
//   strDelim : ��؂蕶��
//   selHead  : �����̏o�́itrue=�O���Afalse=�㑤�j
//   typePath : �p�X��Ɍ��肷�鎞��true
// �o�́F
//   �Ԃ�l : ��؂葶�ݗL��
//   strDiv   : ��؂蕪���㕶����iHead=�O���BTail=�㑤�j
//---------------------------------------------------------------------
bool CnvStrTime::getStrDivide(string& strDiv, const string& strDelim, bool selHead, bool typePath){
	int posdiv = getStrPosDivide(strDiv, strDelim, typePath);
	if ( selHead ){
		if ( posdiv < 0 ){
			return false;
		}
		if ( posdiv == 0 ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(0, posdiv);
		}
	}else{
		if ( posdiv < 0 ){
			strDiv.clear();
			return false;
		}
		posdiv += (int) strDelim.length();
		if ( posdiv >= (int) strDiv.length() ){
			strDiv.clear();
		}else{
			strDiv = strDiv.substr(posdiv);
		}
	}
	return true;
}

//---------------------------------------------------------------------
// �p�X��؂�ʒu���擾
//---------------------------------------------------------------------
int CnvStrTime::getStrPosPath(const string& fullname){
	bool reverse = true;
	int pos = getStrPosDivideCore(fullname, "\\", reverse);
	int postmp = getStrPosDivideCore(fullname, "/", reverse);
	if ( pos < postmp ){
		pos = postmp;
	}
	return pos;
}
//---------------------------------------------------------------------
// ��؂蕶���̈ʒu�擾
// ���́F
//   fullpath : �t���p�X��
//   strDelim : ��؂蕶��
//   typePath : �p�X��Ɍ��肷�鎞��true
// �o�́F
//   �Ԃ�l : ��؂�ʒu�i������Ȃ����̓}�C�i�X�j
//---------------------------------------------------------------------
int CnvStrTime::getStrPosDivide(const string& fullname, const string& strDelim, bool typePath){
	//--- �p�X��̎��̓p�X�ʒu���擾 ---
	int posmin = -1;
	if ( typePath ){
		posmin = getStrPosPath(fullname);
	}
	//--- �Ώۈʒu�擾 ---
	int posr = getStrPosDivideCore(fullname, strDelim, typePath);
	if ( posr < posmin ){
		posr = -1;
	}
	return posr;
}
//---------------------------------------------------------------------
// ��؂蕶���̈ʒu���擾
// ���́F
//   fullpath : �t���p�X��
//   strDelim : ��؂蕶��
//   reverse  : true=�㑤���� false=�O������
// �o�́F
//   �Ԃ�l : ��؂�ʒu�i������Ȃ�����-1�j
//---------------------------------------------------------------------
int CnvStrTime::getStrPosDivideCore(const string& fullname, const string& strDelim, bool reverse){
	int pos = -1;
	//--- ��؂������ ---
	if ( reverse ){
		auto posFind = fullname.rfind(strDelim);
		if ( posFind != string::npos ){
			pos = (int) posFind;
		}
	}else{
		auto posFind = fullname.find(strDelim);
		if ( posFind != string::npos ){
			pos = (int) posFind;
		}
	}
	return pos;
}

//=====================================================================
//  ���Ԃƃt���[���ʒu�̕ϊ�
//  ���ӓ_�F�t���[���ʒu����̕ϊ��͐擪�t���[����0�Ƃ�����Έʒu�Ŏw�肷��悤�ɂ��Ă���
//=====================================================================

//---------------------------------------------------------------------
// �~���b���t���[�����ɕϊ�
//---------------------------------------------------------------------
int CnvStrTime::getFrmFromMsec(Msec msec){
	int r = ((((long long)abs(msec) * m_frate_n) + (m_frate_d*1000/2)) / (m_frate_d*1000));
	return (msec >= 0)? r : -r;
}

//---------------------------------------------------------------------
// �t���[�����ɑΉ�����~���b�����擾
//---------------------------------------------------------------------
int CnvStrTime::getMsecFromFrm(int frm){
	int r = (((long long)abs(frm) * m_frate_d * 1000 + (m_frate_n/2)) / m_frate_n);
	return (frm >= 0)? r : -r;
}

//---------------------------------------------------------------------
// �~���b����x�t���[�����Ɋ��Z������~���b�ɕϊ��i�t���[���P�ʂɂȂ�悤�Ɂj
//---------------------------------------------------------------------
int CnvStrTime::getMsecAlignFromMsec(Msec msec){
	int frm = getFrmFromMsec(msec);
	return getMsecFromFrm(frm);
}

//---------------------------------------------------------------------
// �~���b����x�t���[�����Ɋ��Z��������������ă~���b�ɕϊ�
//---------------------------------------------------------------------
int CnvStrTime::getMsecAdjustFrmFromMsec(Msec msec, int frm){
	int frm_new = getFrmFromMsec(msec) + frm;
	return getMsecFromFrm(frm_new);
}

//---------------------------------------------------------------------
// �~���b��b���ɕϊ�
//---------------------------------------------------------------------
int CnvStrTime::getSecFromMsec(Msec msec){
	if (msec < 0){
		return -1 * ((-msec + 500) / 1000);
	}
	return ((msec + 500) / 1000);
}

//---------------------------------------------------------------------
// �t���[�����[�g�ύX�֐��i���g�p�j
//---------------------------------------------------------------------
int CnvStrTime::changeFrameRate(int n, int d){
	m_frate_n = n;
	m_frate_d = d;
	return 1;
}

//---------------------------------------------------------------------
// �������͎��̒P�ʐݒ�
//---------------------------------------------------------------------
int CnvStrTime::changeUnitSec(int n){
	m_unitsec = n;
	return 1;
}



//=====================================================================
// �����񂩂琔�l���擾
// [��{����]
//   �����񂩂�P�P���ǂݍ��ݐ��l�Ƃ��ďo��
//   src������̈ʒupos����P�P���ǂݍ��݁A���l���o��
//   �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
// ���́F
//   cstr : ������
//   pos  : �F���J�n�ʒu
// �o�́F
//   �Ԃ�l�F �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
//   val    : ���ʐ��l
//=====================================================================

//---------------------------------------------------------------------
// �P�P���ǂݍ��ݐ��l�Ƃ��ďo�́i���l�ȊO������΂����ŏI���j
//---------------------------------------------------------------------
int CnvStrTime::getStrValNumHead(int &val, const string &cstr, int pos){
	// unitsec=2�i�P�ʕϊ����Ȃ��j
	// type=EXNUM�i���l�ȊO������΂����ŏI���j
	return getStrValSubDelimit(val, cstr, pos, 2, DELIMIT_SPACE_EXNUM);
}

//---------------------------------------------------------------------
// �P�P���ǂݍ��ݐ��l�Ƃ��ďo�́i���l�ȊO������Γǂݍ��ݎ��s��Ԃ��j
//---------------------------------------------------------------------
int CnvStrTime::getStrValNum(int &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, 2);		// unitsec=2�i�P�ʕϊ����Ȃ��j
}

//---------------------------------------------------------------------
// �P�P���ǂݍ��ݐ��l�i�~���b�j�Ƃ��ďo�́i���l�ȊO������Γǂݍ��ݎ��s��Ԃ��j
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsec(Msec &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, m_unitsec);
}

//---------------------------------------------------------------------
// ���l�i�~���b�j��Ԃ����A�������͂͐ݒ�ɂ�����炸�t���[�����Ƃ��Ĉ���
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecFromFrm(Msec &val, const string &cstr, int pos){
	return getStrValSub(val, cstr, pos, 0);			// unitsec=0:�������̓t���[����
}

//---------------------------------------------------------------------
// ���l�i�~���b�j��Ԃ����A�}�C�i�X�P�̎��͓��ꈵ���ŕϊ��������̂܂ܕԂ�
//---------------------------------------------------------------------
int CnvStrTime::getStrValMsecM1(Msec &val, const string &cstr, int pos){
	int posnew = getStrValSub(val, cstr, pos, m_unitsec);
	if ((m_unitsec == 0 && getFrmFromMsec(val) == -1) ||
		(m_unitsec == 1 && val == -1000)){
		if (posnew > 0){
			if ((int)cstr.substr(pos, posnew-pos).find(".") < 0){
				val = -1;
			}
		}
	}
	return posnew;
}

//---------------------------------------------------------------------
// �P�P���ǂݍ��ݐ��l�i�b�j�Ƃ��ďo�́i���l�ȊO������Γǂݍ��ݎ��s��Ԃ��j
//---------------------------------------------------------------------
int CnvStrTime::getStrValSec(int &val, const string &cstr, int pos){
	int tmpval;
	pos = getStrValSub(tmpval, cstr, pos, m_unitsec);
	val = (abs(tmpval) + 500) / 1000;
	if (tmpval < 0){
		val = -val;
	}

	return pos;
}

//---------------------------------------------------------------------
// ���l�i�b�j��Ԃ����A�������͂͐ݒ�ɂ�����炸�b�����͂Ƃ��Ĉ���
//---------------------------------------------------------------------
int CnvStrTime::getStrValSecFromSec(int &val, const string &cstr, int pos){
	int tmpval;
	pos = getStrValSub(tmpval, cstr, pos, 1);			// unitsec=1:�������͕b��
	val = (abs(tmpval) + 500) / 1000;
	if (tmpval < 0){
		val = -val;
	}

	return pos;
}

//---------------------------------------------------------------------
// �֐�������������擾���āA���ʂ̐��l��Ԃ�
//---------------------------------------------------------------------
int CnvStrTime::getStrValFuncNum(int &val, const string &cstr, int pos){
	string dstr;
	pos = getStrItemHubFunc(dstr, cstr, pos, DELIMIT_FUNC_CALC);
	if ( getStrValNum(val, dstr, 0) < 0 ){
		return -1;
	}
	return pos;
}

//=====================================================================
// ���X�g�f�[�^�擾
//=====================================================================

//---------------------------------------------------------------------
// ���X�g�����񂩂�S���ڂ̎��ԏ��i�~���b�j�����X�g�Ŏ擾
//---------------------------------------------------------------------
//--- -1�̓��ꈵ���Ȃ� ---
bool CnvStrTime::getListValMsec(vector<Msec>& listMsec, const string& strList){
	int pos = 0;
	string dstr;
	listMsec.clear();
	while( (pos = getStrWord(dstr, strList, pos)) > 0 ){
		int val;
		if ( getStrValMsec(val, dstr, 0) > 0 ){
			listMsec.push_back(val);
		}
		while ( strList[pos] == ',' ) pos++;
	}
	if ( listMsec.empty() ) return false;
	return true;
}
//--- -1�͓��ꈵ���ł��̂܂� ---
bool CnvStrTime::getListValMsecM1(vector<Msec>& listMsec, const string& strList){
	int pos = 0;
	string dstr;
	listMsec.clear();
	while( (pos = getStrWord(dstr, strList, pos)) > 0 ){
		int val;
		if ( getStrValMsecM1(val, dstr, 0) > 0 ){
			listMsec.push_back(val);
		}
		while ( strList[pos] == ',' ) pos++;
	}
	if ( listMsec.empty() ) return false;
	return true;
}



//=====================================================================
// �����񂩂�P����擾
// [��{����]
//   �����񂩂�P�P���ǂݍ��ݏo��
//   src������̈ʒupos����P�P���ǂݍ���dst�ɏo��
//   �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
// ���́F
//   cstr : ������
//   pos  : �F���J�n�ʒu
// �o�́F
//   �Ԃ�l�F �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
//   dst    : �o�͕�����
//=====================================================================

// getStrItem    : �擪�����quote�͔F���A�r�����������quote�͖���
// getStrWord    : �X�y�[�X+�R���}��؂�A�R���}���͔̂�΂��ēǂ�
// getStrCsv     : CSV�`����1���ڂ��擾
// getStrItemCmd : �R�}���h�ǂݍ��ݗp
// getStrItemArg : �����p�Bquote�݂͂𑱂��ĘA��������
// getStrItemMonitor : �\���p�Bquote�͏����Ȃ�
// getStrItemWithQuote : �ŏ��ƍŌ��quote���c���B����������̔���p

//--- �����񂩂�X�y�[�X��؂�łP�P���ǂݍ��� ---
int CnvStrTime::getStrItem(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- �����񂩂�P�P���ǂݍ��ށi�X�y�[�X�ȊO��","����؂�Ƃ��ĔF���j ---
int CnvStrTime::getStrWord(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_COMMA;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- �����񂩂�CSV�`����1���ڂ��擾 ---
int CnvStrTime::getStrCsv(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_CSV;
	itype.concat = true;		// �_�u���N�H�[�g�����i���ꏈ���j����
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = true;		// �f�[�^�Ȃ�������
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- �����񂩂�X�y�[�X��؂�łP�P���ǂݍ��ށi�R�}���h�擾�p�j ---
int CnvStrTime::getStrItemCmd(string &dst, const string &cstr, int pos){
	return getStrItemHubFunc(dst, cstr, pos, DELIMIT_FUNC_NAME);
}
//--- �����񂩂�X�y�[�X��؂�łP�P���ǂݍ��ށiquote�O��ɋ�؂�Ȃ���Ό����j ---
int CnvStrTime::getStrItemArg(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// ��������
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- quote�͂��̂܂ܕ\������ ---
int CnvStrTime::getStrItemMonitor(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// ��������
	itype.separate = false;
	itype.remain = true;		// �擪�Ō�quote�͂��̂܂܏o��
	itype.defstr = false;
	itype.qdisp  = true;		// ������quote�͂��̂܂܏o��
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}
//--- quote����؂�Ŏc�����܂ܕ����񂩂�P�P���ǂݍ��ށi�A��quote�̏ꍇ�̂݌����j ---
int CnvStrTime::getStrItemWithQuote(string &dst, const string &cstr, int pos){
	ArgItemType itype = {};
	itype.dstype = DELIMIT_SPACE_QUOTE;
	itype.concat = true;		// ��������
	itype.separate = true;		// "aa"=="bb"�̃P�[�X��==�̑O��ŕ���
	itype.remain = true;		// ���ڂ̐擪�ƍŌ��qyote�͎c��
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	return getStrItemHubStr(dst, cstr, pos, itype);
}

//---------------------------------------------------------------------
// �R�����g�������ĕ�����擾
//---------------------------------------------------------------------
int CnvStrTime::getStrWithoutComment(string &dst, const string &cstr){
	int poscmt = getStrPosComment(cstr, 0);
	if ( poscmt > 0 ){
		dst = cstr.substr(0, poscmt);
	}else if ( poscmt == 0 ){
		dst = "";
	}else{
		dst = cstr;
	}
	return poscmt;
}
//---------------------------------------------------------------------
// �R�����g�Ƃ��Ă�#�ʒu���擾
//---------------------------------------------------------------------
int CnvStrTime::getStrPosComment(const string &cstr, int pos){
	return getStrPosChar(cstr, '#', false, pos);
}
//---------------------------------------------------------------------
// �ϐ��Ƃ��Ă�$�ʒu���擾
//---------------------------------------------------------------------
int CnvStrTime::getStrPosReplaceVar(const string &cstr, int pos){
	return getStrPosChar(cstr, '$', true, pos);
}
//---------------------------------------------------------------------
// �w�萧�䕶����pos�ȍ~�ōŏ��Ɍ����ʒu���擾�i�W�J���Ȃ����p�����͏����j
//   expand : true=�_�u���N�H�[�g���͓W�J����
//---------------------------------------------------------------------
int CnvStrTime::getStrPosChar(const string &cstr, char chsel, bool expand, int pos){
	int possel = -1;
	int poscmt = -1;
	bool flagQw = false;
	bool flagQs = false;
	bool nextSp = false;
	int len = (int) cstr.length();
	for(int i=0; i<len; i++){
		char cht = cstr[i];
		if ( nextSp ){		// ���ꕶ������
			if ( cht == '#' ){	// $#�͓��ꏈ��
				nextSp = false;
				continue;
			}
		}
		bool check = ( expand || flagQw == false ) && ( flagQs == false );
		if ( check && i >= pos && cht == chsel ){
			if ( chsel == '$' ){	// �ϐ��������̓��ꏈ��
				char chn = cstr[i+1];
				if ( (0x00 <= chn && chn <  '0' ) ||
				     ( '9' <  chn && chn <  'A' ) ||
				     ( 'Z' <  chn && chn <  'a' ) ||
				     ( 'z' <  chn && chn <= 0x7F) ){
					if ( chn != '#' && chn != '{' && chn != '_' ){
						continue;		// $���オ�ϐ��Ɩ��֌W�̋L���ł���Εϐ��ƔF�����Ȃ�
					}
				}
			}
			possel = i;
			break;			// ���ʊi�[������I��
		}
		else if ( flagQw ){
			if ( cht == '\"' ) flagQw = false;
		}
		else if ( flagQs ){
			if ( cht == '\'' ) flagQs = false;
		}
		else{
			switch( cht ){
				case '#' :
					if ( poscmt < 0 ) poscmt = i;	// �R�����g�ʒu
					break;
				case '$' :
					nextSp = true;
					break;
				case '\"' :
					flagQw = true;
					break;
				case '\'' :
					flagQs = true;
					break;
				default:
					break;
			}
		}
	}
	return possel;
}
//---------------------------------------------------------------------
// �����񂩂� .. �ɂ��͈͎w����܂ސ������擾���ĕ�����Ƃ��ĕԂ�
// �擾�ł��Ȃ��������͕Ԃ�l���}�C�i�X�ɂȂ邪�A����I���Ȃ�dst����A�ُ�Ȃ當���񂪓���
//---------------------------------------------------------------------
int CnvStrTime::getStrMultiNum(string &dst, const string &cstr, int pos){
	int posBak = pos;
	string strTmp;
	pos = getStrWord(strTmp, cstr, pos);
	if ( pos >= 0 ){
		int rloc = (int)strTmp.find("..");
		if ( rloc != (int)string::npos ){			// ..�ɂ��͈͐ݒ莞
			string strSt = strTmp.substr(0, rloc);
			string strEd = strTmp.substr(rloc+2);
			int valSt;
			int valEd;
			int posSt = getStrValNum(valSt, strSt, 0);
			int posEd = getStrValNum(valEd, strEd, 0);
			if ( posSt >= 0 && posEd >= 0 ){
				string strValSt = std::to_string(valSt);
				string strValEd = std::to_string(valEd);
				dst = strValSt + ".." + strValEd;
			}else{
				pos = -1;
				dst = strTmp;
			}
		}else if ( strTmp == "odd" || strTmp == "even" ){
			dst = strTmp;
		}else{
			int val1;
			if ( getStrValNum(val1, strTmp, 0) >= 0 ){
				dst = std::to_string(val1);
			}else{
				pos = -1;
				dst = strTmp;
			}
		}
	}else{
		pos = getStrItem(strTmp, cstr, posBak);
		if ( pos >= 0 ){		// ���X�g�Ŏ擾�ł��Ȃ��f�[�^���c���Ă��鎞�͕���������
			pos = -1;
			dst = strTmp;
		}else{					// �f�[�^�I���Ȃ�󕶎���
			dst.clear();
		}
	}
	return pos;
}
//---------------------------------------------------------------------
// �ő�lmaxNum�͈̔͂��萔�l���������curNum�����݂��邩
//---------------------------------------------------------------------
bool CnvStrTime::isStrMultiNumIn(const string &cstr, int curNum, int maxNum){
	bool exist = false;
	int pos = 0;
	while( pos >= 0 && !exist ){
		string strVal;
		pos = getStrWord(strVal, cstr, pos);
		if ( pos < 0 ) break;

		int rloc = (int)strVal.find("..");
		if ( rloc == (int)string::npos ){		// �ʏ�̐��l
			int val = stoi(strVal);
			if ( (val == 0) || (val == curNum) || (maxNum + val + 1 == curNum) ){
				exist = true;
			}
		}else if ( strVal == "odd" ){
			if ( curNum % 2 == 1 ){
				exist = true;
			}
		}else if ( strVal == "even" ){
			if ( curNum % 2 == 0 ){
				exist = true;
			}
		}else{								// �͈͎w��
			string strSt = strVal.substr(0, rloc);
			string strEd = strVal.substr(rloc+2);
			int valSt = stoi(strSt);
			int valEd = stoi(strEd);
			if ( valSt < 0 ){
				valSt = maxNum + valSt + 1;
			}
			if ( valEd < 0 ){
				valEd = maxNum + valEd + 1;
			}
			if ( (valSt <= curNum) && (curNum <= valEd) ){
				exist = true;
			}
		}
	}
	return exist;
}
//=====================================================================
// ���Ԃ𕶎���i�t���[���܂��̓~���b�j�ɕϊ�
//=====================================================================

//---------------------------------------------------------------------
// �t���[�����܂��͎��ԕ\�L�i-1�͂��̂܂܎c���j
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1(Msec msec_val){
	bool type_frm = false;
	if (m_unitsec == 0){
		type_frm = true;
	}
	return getStringMsecM1All(msec_val, type_frm);
}

//---------------------------------------------------------------------
// �t���[���\�L�i-1�͂��̂܂܎c���j
//---------------------------------------------------------------------
string CnvStrTime::getStringFrameMsecM1(Msec msec_val){
	return getStringMsecM1All(msec_val, true);
}

//---------------------------------------------------------------------
// ���ԕ\�L�i-1�͂��̂܂܎c���j
//---------------------------------------------------------------------
string CnvStrTime::getStringTimeMsecM1(Msec msec_val){
	return getStringMsecM1All(msec_val, false);
}

//---------------------------------------------------------------------
// ���Ԃ𕶎���i�t���[���܂��͎��ԕ\�L�j�ɕϊ�
//---------------------------------------------------------------------
string CnvStrTime::getStringMsecM1All(Msec msec_val, bool type_frm){
	string str_val;
	if (msec_val == -1){
		str_val = to_string(-1);
	}
	else if (type_frm){
		int n = getFrmFromMsec(msec_val);
		str_val = to_string(n);
	}
	else{
		int val_abs = (msec_val < 0)? -msec_val : msec_val;
		int val_t = val_abs / 1000;
		int val_h = val_t / 3600;
		int val_m = (val_t / 60) % 60;
		int val_s = val_t % 60;
		int val_x = val_abs % 1000;
		string str_h = getStringZeroRight(val_h, 2);
		string str_m = getStringZeroRight(val_m, 2);
		string str_s = getStringZeroRight(val_s, 2);
		string str_x = getStringZeroRight(val_x, 3);
		str_val = str_h + ":" + str_m + ":" + str_s;
		if (val_x > 0){
			str_val = str_val + "." + str_x;
		}
		if (msec_val < 0){
			str_val = "-" + str_val;
		}
	}
	return str_val;
}


//---------------------------------------------------------------------
// ���l�𕶎���ɕϊ��i���0���߁j
//---------------------------------------------------------------------
string CnvStrTime::getStringZeroRight(int val, int len){
	string str_val = "";
	for(int i=0; i<len-1; i++){
		str_val += "0";
	}
	str_val += to_string(val);
	return str_val.substr( str_val.length()-len );
}




//=====================================================================
//
// �����񏈗��̓����֐�
//
//=====================================================================
//--- �w��ʒu�̕����i�}���`�o�C�g�j�o�C�g����Ԃ� ---
int CnvStrTime::getMbStrSize(const string& str, int n){
	return getMbStrSizeUtf8(str, n);
}
int CnvStrTime::getMbStrSizeSjis(const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0) return 0;
	if ( n >= len-1 ) return 1;
	unsigned char code = (unsigned char) str[n];
	if ((code >= 0x81 && code <= 0x9F) ||
		(code >= 0xE0 && code <= 0xFC)){		// Shift-JIS 1st-byte
		code = (unsigned char) str[n+1];
		if ((code >= 0x40 && code <= 0x7E) ||
			(code >= 0x80 && code <= 0xFC)){	// Shift-JIS 2nd-byte
			return 2;
		}
	}
	return 1;
}
int CnvStrTime::getMbStrSizeUtf8(const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0 ) return 0;
	unsigned char code = (unsigned char) str[n];
	if ( (code & 0x80) == 0 ) return 1;
	for(int i=1; i<4; i++){
		if ( n+i >= len ) return i;
		code = (unsigned char) str[n+i];
		if ( (code & 0xC0) != 0x80 ) return i;
	}
	return 4;
}
//--- n�����ڂ��}���`�o�C�g��2�Ԗڈȍ~�̕����ł����true��Ԃ� ---
bool CnvStrTime::isStrMbSecond(const string& str, int n){
	if ( n >= (int)str.length() ) return false;	// �ُ������
	std::mblen(nullptr, 0);		// �ϊ���Ԃ����Z�b�g
	int i = 0;
	while( i<n && i>=0 ){
		int mbsize = getMbStrSize(str, i);
		if ( mbsize <= 0 ){
			return false;
		}
		i += mbsize;
	}
	return ( i != n );
}

//---------------------------------------------------------------------
// �����񂩂�P�P���ǂݍ��ݐ��l�i�~���b�j�Ƃ��Ċi�[�i���l�ȊO������Γǂݍ��ݎ��s��Ԃ��j
// cstr������̈ʒupos����P�P���ǂݍ��݁A���l��val�ɏo��
// ���́F
//   cstr : ������
//   pos  : �F���J�n�ʒu
//   unitsec : ���������̒P�ʁi0=�t���[����  1=�b��  2=�P�ʕϊ��Ȃ��j
// �o�́F
//   �Ԃ�l�F �ǂݏI������ʒu��Ԃ�l�Ƃ���i���s����-1�j
//   val    : ���l�i�~���b�j
//---------------------------------------------------------------------
int CnvStrTime::getStrValSub(int &val, const string &cstr, int pos, int unitsec){
	return getStrValSubDelimit(val, cstr, pos, unitsec, DELIMIT_SPACE_ONLY);
}

//---------------------------------------------------------------------
// ��؂�I��ǉ����ĉ��Z���s
//---------------------------------------------------------------------
int CnvStrTime::getStrValSubDelimit(int &val, const string &cstr, int pos, int unitsec, DelimtStrType type){
	int st, ed;

	pos = getStrItemHubRange(st, ed, cstr, pos, type);
	if ( pos < 0 ) return pos;
	try{
		val = getStrCalc(cstr, st, ed-1, unitsec);
	}
	catch(int errloc){
//		printf("err:%d\n",errloc);
		val = errloc;
		pos = -1;
	}
	return pos;
}


//---------------------------------------------------------------------
// �P���ڂ̕�����ʒu�͈͂��擾
// ���́F
//   cstr : ������
//   pos  : �ǂݍ��݊J�n�ʒu
//   type : ��ށi0=�X�y�[�X��؂�QUOTE��  1=�X�y�[�X��؂�  2=1+�R���}����؂�  3=�ŏ��̐��������̂�
// �o�́F
//   �Ԃ�l�F �ǂݍ��ݏI���ʒu
//   st   : �F���J�n�ʒu
//   ed   : �F���I���ʒu
//---------------------------------------------------------------------
int CnvStrTime::getStrItemHubRange(int &st, int &ed, const string &cstr, int pos, DelimtStrType type){
	ArgItemType itype = {};
	itype.dstype = type;
	itype.concat = false;
	itype.separate = false;
	itype.remain = false;
	itype.defstr = false;
	itype.qdisp  = false;
	itype.emptyok = false;
	string strDmy;
	return getStrItemCommon(strDmy, st, ed, cstr, pos, itype);
}
//---------------------------------------------------------------------
// �֐��^�C�v�P���ڂ̕�����擾
//---------------------------------------------------------------------
int CnvStrTime::getStrItemHubFunc(string& dstr, const string &cstr, int pos, DelimtStrType dstype){
	ArgItemType itype = {};
	if ( dstype == DELIMIT_FUNC_NAME ){		// �֐����O����
		itype.dstype = dstype;
		itype.concat = false;
		itype.separate = false;
		itype.remain = false;
		itype.defstr = false;
		itype.qdisp  = false;
		itype.emptyok = false;
	}else{
		itype.dstype = dstype;
		itype.concat = true;
		itype.separate = false;
		itype.remain = true;
		itype.defstr = true;
		itype.qdisp  = false;
		itype.emptyok = false;
	}
	int st, ed;
	return getStrItemCommon(dstr, st, ed, cstr, pos, itype);
}
//--- �擾�ʒu�s�v�ŕ�����ƏI���ʒu��Ԃ� ---
int CnvStrTime::getStrItemHubStr(string& dstr, const string &cstr, int pos, ArgItemType itype){
	int st, ed;
	return getStrItemCommon(dstr, st, ed, cstr, pos, itype);
}
//---------------------------------------------------------------------
// �P���ڂ̕�������擾
//---------------------------------------------------------------------
// ������Ɣ͈͂��擾�i���ʐݒ�j
int CnvStrTime::getStrItemCommon(string& dstr, int &st, int &ed, const string &cstr, int pos, ArgItemType itype){
	if (pos < 0) return pos;

	int pos_before = pos;
	QuoteType qtype = {};

	//--- trim left�i�󔒂���؂�̏ꍇ�A���̍��ڐ擪�܂ňړ��j ---
	if ( isCharTypeDelim(' ', itype.dstype) ){
		pos = skipCharSpace(cstr, pos);
	}
	//--- check quote ---
	dstr.clear();
	bool validDquote = isCharValidDquote(itype.dstype);
	bool validSquote = isCharValidSquote(itype.dstype);
	if ( validDquote || validSquote ){
		char ch = cstr[pos];
		bool flagPos = false;
		if ( ch == '\"' && validDquote ){
			flagPos = true;
			qtype.flagQw = true;
			qtype.existQ = true;
			qtype.edgeQw = true;
		}else if ( ch == '\''  && validSquote ){
			flagPos = true;
			qtype.flagQs = true;
			qtype.existQ = true;
			qtype.edgeQw = false;
		}
		if ( flagPos ){
			pos ++;
			if ( itype.qdisp ){
				dstr += ch;
			}
		}
	}
	//--- �J�n�ʒu�ݒ� ---
	st = pos;
	ed = pos;
	//--- �f�[�^�ʒu�m�F ---
	bool flagEnd = false;
	do{
		char ch = cstr[pos];
		if (ch == '\0' || dstr.length() >= SIZE_BUF_MAX-1){	// �����I������
			break;
		}
		QuoteState qstate;
		flagEnd = getStrItemCommonCh(qstate, qtype, ch, dstr.empty(), itype);
		if ( qstate.add ){
			dstr += ch;
			ed = pos + 1;
		}
		if ( qstate.pos || !flagEnd ){
			pos ++;
		}
	} while( !flagEnd );
	//--- �I���ʒu�ݒ� ---
	if ( qtype.flagQw || qtype.flagQs ) {				// QUOTE�ُ�
		pos = -1;
	}
	else if ( qtype.existQ ){		// QUOTE�ň͂܂ꂽ������
		if ( itype.remain && !itype.qdisp ){	// ����quote�͏������O��݂͎͂c���ꍇ
			if ( qtype.edgeQw ){
				dstr = "\"" + dstr + "\"";
			}else{
				dstr = "\'" + dstr + "\'";
			}
		}else if ( !itype.remain && itype.qdisp ){	// ����quote�͎c�����O��quote�͎c���Ȃ��ꍇ
			auto len = dstr.length();
			if ( len == 2 ){
				dstr.clear();
			}else if ( len >= 2){
				char ch = dstr[dstr.length()-1];
				if ( ch == '\"' || ch == '\'' ){
					dstr = dstr.substr(1,len-2);
				}
			}
		}
		if ( itype.defstr ){
			if ( dstr.empty() && pos > st+1 ){
				dstr = "\"\"";		// �o�͂Ȃ���QUOTE����QUOTE����ꍇ��QUOTE
			}
		}
	}
	else if ( dstr.empty() ){	// �ǂݍ��݃f�[�^�Ȃ��ꍇ
		if ( itype.emptyok ){
			if ( pos == pos_before ){	// �f�[�^�Ȃ������͍ŏ����當����Ō�̏ꍇ�̂ݖ���
				pos = -1;
			}
		}else{
			pos = -1;
		}
	}
	return pos;
}
//--- �����F�� ---
bool CnvStrTime::getStrItemCommonCh(QuoteState& qstate, QuoteType& qtype, char ch, bool yet, ArgItemType itype){
	//--- ���p���͋�؂�ȊO�̓`�F�b�N�����o�� ---
	if ( qtype.flagQw || qtype.flagQs ){
		qstate.end = false;
		qstate.add = true;
		qstate.pos = true;
		if ( (qtype.flagQw && ch != '\"') ||
		     (qtype.flagQs && ch != '\'') ){
			return qstate.end;
		}
	}
	//--- �ʏ�̋�؂蔻�� ---
	qstate.end = isCharTypeDelim(ch, itype.dstype);
	qstate.add = !qstate.end;
	qstate.pos = !qstate.end;
	//--- ���ꕶ������ ---
	bool refind_dq = ( isCharValidDquote(itype.dstype) && (itype.concat || itype.separate) );
	bool refind_sq = ( isCharValidSquote(itype.dstype) && (itype.concat || itype.separate) );
	switch(ch){
		case '\"':
			if ( qtype.flagQw ){				// ���p���Q���
				qstate.add = false;
				qstate.pos = true;
				qtype.flagQw = false;
				qtype.edgeQw = true;
				if ( itype.separate || !itype.concat ){
					qstate.end = true;
				}
			}
			else if ( refind_dq ){			// �r��������p���P���
				qstate.add = false;
				if ( itype.separate ){		// QUOTE�ŕ������鎞
					qstate.end = true;
					qstate.pos = false;
				}else{
					qstate.pos = true;
					qtype.flagQw = true;
					qtype.existQ = true;
					qtype.edgeQw = true;
					if ( itype.dstype == DELIMIT_CSV ){
						qstate.add = true;
					}
				}
			}
			if ( itype.qdisp && qstate.pos ){
				qstate.add = true;
			}
			break;
		case '\'':
			if ( qtype.flagQs ){				// ���p���Q���
				qstate.add = false;
				qstate.pos = true;
				qtype.flagQs = false;
				qtype.edgeQw = false;
				if ( itype.separate || !itype.concat ){
					qstate.end = true;
				}
			}
			else if ( refind_sq ){			// �r��������p���P���
				qstate.add = false;
				if ( itype.separate ){		// QUOTE�ŕ������鎞
					qstate.end = true;
					qstate.pos = false;
				}else{
					qstate.pos = true;
					qtype.flagQs = true;
					qtype.existQ = true;
					qtype.edgeQw = false;
				}
			}
			if ( itype.qdisp && qstate.pos ){
				qstate.add = true;
			}
			break;
		case '(':
			qtype.numPar ++;
			break;
		case ')':
			qtype.numPar --;
			if ( qtype.numPar < 0 &&
			    ( itype.dstype == DELIMIT_FUNC_ARGS ||
			      itype.dstype == DELIMIT_FUNC_CALC )){
				qstate.end = true;
				qstate.add = yet;		// �擪�����̎��͒ǉ����ďI��
				qstate.pos = qstate.add;
			}
			break;
		case ',':
			if ( itype.dstype == DELIMIT_SPACE_COMMA ||
			     itype.dstype == DELIMIT_CSV         ){
				qstate.pos = true;		// �����΂���؂蕶����F��
			}
			break;
		default:
			break;
	}
	if ( isCharTypeSpace(ch) && itype.dstype == DELIMIT_FUNC_CALC ){
		qstate.add = false;		// ���Z�����̃X�y�[�X�͏ȗ�
	}
	return qstate.end;
}
//--- �󔒕������΂� ---
int CnvStrTime::skipCharSpace(const string &cstr, int pos){
	//--- trim left�i�󔒂���؂�̏ꍇ�A���̍��ڐ擪�܂ňړ��j ---
	while( isCharTypeSpace(cstr[pos]) ){
		pos ++;
	}
	return pos;
}
//---------------------------------------------------------------------
// �����̎�ނ��擾
//---------------------------------------------------------------------
//--- ��ނ��擾 ---
CnvStrTime::CharCtrType CnvStrTime::getCharTypeSub(char ch){
	CharCtrType typeMark;

	switch(ch){
		case '\0':
			typeMark = CHAR_CTR_NULL;
			break;
		case ' ':
		case '\t':
			typeMark = CHAR_CTR_SPACE;
			break;
		default:
			if (ch >= 0 && ch < ' '){
				typeMark = CHAR_CTR_CTRL;
			}
			else{
				typeMark = CHAR_CTR_OTHER;
			}
			break;
	}
	return typeMark;
}
//--- �������󔒂��`�F�b�N ---
bool CnvStrTime::isCharTypeSpace(char ch){
	CharCtrType typeMark;

	typeMark = getCharTypeSub(ch);
	if (typeMark == CHAR_CTR_SPACE){
		return true;
	}
	return false;
}
//--- �󔒂��I�����`�F�b�N ---
bool CnvStrTime::isCharTypeSpaceEnd(char ch){
	CharCtrType typeMark;

	typeMark = getCharTypeSub(ch);
	if ( typeMark == CHAR_CTR_SPACE || typeMark == CHAR_CTR_NULL ){
		return true;
	}
	return false;
}
//--- ��؂蕶�����`�F�b�N ---
bool CnvStrTime::isCharTypeDelim(char ch, DelimtStrType dstype){
	bool flagDelim = false;
	bool typeSpace = isCharTypeSpace(ch);
	bool typeComma = (ch == ',');
	bool typeNonum = (ch < '0' || ch > '9');
	switch( dstype ){
		case DELIMIT_SPACE_QUOTE:
		case DELIMIT_SPACE_ONLY:
		case DELIMIT_FUNC_ARGS:
			flagDelim = typeSpace;
			break;
		case DELIMIT_SPACE_COMMA:
		case DELIMIT_FUNC_CALC:
			flagDelim = typeSpace || typeComma;
			break;
		case DELIMIT_SPACE_EXNUM:
			flagDelim = typeSpace || typeNonum;
			break;
		case DELIMIT_CSV:
			flagDelim = typeComma;
			break;
		case DELIMIT_FUNC_NAME:
			flagDelim = typeSpace || typeComma || ch == '(' || ch == ')';
			break;
		default:
			break;
	}
	return flagDelim;
}
//--- double quote���L���Ȍ������`�F�b�N ---
bool CnvStrTime::isCharValidDquote(DelimtStrType dstype){
	return ( dstype == DELIMIT_SPACE_QUOTE ||
	         dstype == DELIMIT_SPACE_COMMA ||
	         dstype == DELIMIT_FUNC_ARGS   ||
	         dstype == DELIMIT_FUNC_CALC   ||
	         dstype == DELIMIT_CSV         );
}
//--- single quote�̂ݖ������`�F�b�N ---
bool CnvStrTime::isCharValidSquote(DelimtStrType dstype){
	return ( dstype == DELIMIT_SPACE_QUOTE ||
	         dstype == DELIMIT_SPACE_COMMA ||
	         dstype == DELIMIT_FUNC_ARGS   ||
	         dstype == DELIMIT_FUNC_CALC   );
}

//---------------------------------------------------------------------
// ����������Z�������ă~�����Ԃ��擾
// ���́F
//   cstr : ������
//   st   : �F���J�n�ʒu
//   ed   : �F���I���ʒu
//   unitsec : ���������̒P�ʁi0=�t���[����  1=�b��  2=�ϊ��Ȃ��j
// �o�́F
//   �Ԃ�l�F ���Z���ʃ~���b
//---------------------------------------------------------------------
int CnvStrTime::getStrCalc(const string &cstr, int st, int ed, int unitsec){
	return getStrCalcDecode(cstr, st, ed, unitsec, 0);
}

//---------------------------------------------------------------------
// ����������Z�������ă~�����Ԃ��擾�͈͎̔w�艉�Z
// ���́F
//   cstr : ������
//   st   : �F���J�n�ʒu
//   ed   : �F���I���ʒu
//   dsec : �������̒l�i0=�t���[����  1=�b���j
//   draw : �揜�Z����̏����i0=�ʏ�  1=�P�ʕϊ����~  2=�ϊ��Ȃ��j
// �o�́F
//   �Ԃ�l�F ���Z���ʃ~���b
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcDecode(const string &cstr, int st, int ed, int dsec, int draw){
	//--- ���ɉ��Z���s���Q���ɕ������� ---
	int codeMark_op  = 0;				// ���Z�q�^�C�v
	int priorMark_op = 0;				// ���Z�q�D�揇��
	int nPar_i    = 0;					// ���݂̊��ʐ�
	int nPar_op   = -1;					// ���Z�̂��銇�ʐ��̍ŏ��l
	int posOpS    = -1;					// �������鉉�Z�q�ʒu�i�J�n�j
	int posOpE    = -1;					// �������鉉�Z�q�ʒu�i�I���j
	int flagHead  = 1;					// �P�����Z�q�t���O
	int flagTwoOp = 0;					// 2�������Z�q
	for(int i=st; i<=ed; i++){
		if (flagTwoOp > 0){				// �O��2�������Z�q�������ꍇ�͎��̕�����
			flagTwoOp = 0;
			continue;
		}
		int codeMark_i = getStrCalcCodeChar(cstr[i], flagHead);
		if (i < ed){								// 2�������Z�q�`�F�b�N
			int codeMark_two = getStrCalcCodeTwoChar(cstr[i], cstr[i+1], flagHead);
			if (codeMark_two > 0){
				codeMark_i = codeMark_two;
				flagTwoOp = 1;
			}
		}
		if ( codeMark_i == D_CALCOP_ERROR ){	// ���Z�ł��Ȃ�����
			throw i;
		}
		int categMark_i = getMarkCategory(codeMark_i);
		int priorMark_i = getMarkPrior(codeMark_i);
		if (codeMark_i == D_CALCOP_PARS){			// ���ʊJ�n
			nPar_i ++;
		}
		else if (codeMark_i == D_CALCOP_PARE){		// ���ʏI��
			nPar_i --;
			if (nPar_i < 0){						// ���ʂ̐�������Ȃ��G���[
				throw i;
			}
		}
		else if (categMark_i == D_CALCCAT_OP1){		// �P�����Z�q
			int next_i = ( flagTwoOp > 0 )? i+2 : i+1;
			if ( next_i > ed ){						// �P�����Z�q�̌�ɉ����Ȃ�
				throw i;
			}
			if ( codeMark_i == D_CALCOP_SEC ||		// S(�b��)
			     codeMark_i == D_CALCOP_FRM ){		// F(�t���[����)
				int codeMarkNext = getStrCalcCodeChar(cstr[next_i], flagHead);
				if ( codeMarkNext != D_CALCOP_PARS ){
					throw i;
				}
			}
		}
		else if (categMark_i == D_CALCCAT_OP2){		// 2�����Z�q
			if ((nPar_op == nPar_i && priorMark_op <= priorMark_i) ||
				(nPar_op > nPar_i) || posOpS < 0){
				posOpS = i;							// �ʒu
				posOpE = (flagTwoOp > 0)? i+1 : i;	// �ʒu
				priorMark_op = priorMark_i;			// �D�揇��
				codeMark_op  = codeMark_i;			// 2�����Z�q�f�[�^
				nPar_op      = nPar_i;				// ���ʐ�
			}
			flagHead = 1;							// ���Ɍ���镶���͒P�����Z�q
		}
		else{										// ���l����
			flagHead = 0;							// �P�����Z�q�t���O�͏���
			if (posOpS < 0 && (nPar_op > nPar_i || nPar_op < 0)){
				nPar_op = nPar_i;					// ���Z�q�Ȃ��ꍇ�̊��ʐ��ێ�
			}
		}
	}
	if (nPar_i != 0){								// ���ʂ̐�������Ȃ��G���[
		throw ed;
	}
	//--- �s�v�ȊO���̊��ʂ͊O�� ---
	int flagLoop = 1;
	while(nPar_op > 0 && flagLoop > 0){				// ���ʊO���Z���Ȃ��ꍇ���Ώ�
		int codeMark_s = getStrCalcCodeChar(cstr[st], 1);
		int codeMark_e = getStrCalcCodeChar(cstr[ed], 0);
		if (codeMark_s == D_CALCOP_PARS && codeMark_e == D_CALCOP_PARE){
			st ++;
			ed --;
			nPar_op --;
		}
		else{										// �O�������ʈȊO�Ȃ�I��
			flagLoop = 0;
		}
	}
	//--- ���Z�̎��s ---
	int dr;
	if (posOpS > 0 && nPar_op == 0){				// ���̏�����2�����Z�q�̏ꍇ
		if (posOpS == st || posOpE == ed){			// �O��ɍ��ڂ��Ȃ��ꍇ�̓G���[
			throw posOpS;
		}
		int raw2 = draw;
		if (codeMark_op == D_CALCOP_MUL ||			// �揜�Z�ł͂Q���ڂ̒P�ʕϊ����Ȃ�
			codeMark_op == D_CALCOP_DIV){
			raw2 = 1;
		}
		int d1 = getStrCalcDecode(cstr, st, posOpS-1, dsec, draw);	// �͈͑I�����čăf�R�[�h
		int d2 = getStrCalcDecode(cstr, posOpE+1, ed, dsec, raw2);	// �͈͑I�����čăf�R�[�h
		dr = getStrCalcOp2(d1, d2, codeMark_op);					// 2�����Z����
	}
	else{											// ���̏�����2�����Z�q�łȂ��ꍇ
		int codeMark_s = getStrCalcCodeChar(cstr[st], 1);
		int categMark_s = getMarkCategory(codeMark_s);
		if (categMark_s == D_CALCCAT_OP1){						// ���̏������P�����Z�q�̏ꍇ
			if (codeMark_s == D_CALCOP_SEC){
				dsec = 1;
			}
			else if (codeMark_s == D_CALCOP_FRM){
				dsec = 0;
			}
			int d1 = getStrCalcDecode(cstr, st+1, ed, dsec, draw);	// �͈͑I�����čăf�R�[�h
			dr = getStrCalcOp1(d1, codeMark_s);					// �P�����Z����
		}
		else{
			dr = getStrCalcTime(cstr, st, ed, dsec, draw);			// ���l���Ԃ̎擾
//printf("[%c,%d,%d,%d]",cstr[st],dr,st,ed);
		}
	}

	return dr;
}


//---------------------------------------------------------------------
// ������ނ̎擾 - ����
//---------------------------------------------------------------------
int CnvStrTime::getMarkCategory(int code){
	return  (code / 0x1000);
}

//---------------------------------------------------------------------
// ������ނ̎擾 - �D�揇��
//---------------------------------------------------------------------
int CnvStrTime::getMarkPrior(int code){
	return ((code % 0x1000) / 0x100);
}

//---------------------------------------------------------------------
// ���Z�p�ɕ����F��
// ���́F
//   ch   : �F�������镶��
//   head : 0=�ʏ�  1=�擪�����Ƃ��ĔF��
// �o�́F
//   �Ԃ�l�F �F���R�[�h
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcCodeChar(char ch, int head){
	int codeMark;

	if (ch >= '0' && ch <= '9'){
		codeMark = ch - '0';
	}
	else{
		switch(ch){
			case '.':
				codeMark = D_CALCOP_PERD;
				break;
			case ':':
				codeMark = D_CALCOP_COLON;
				break;
			case '+':
				codeMark = D_CALCOP_PLUS;
				if (head > 0){
					codeMark = D_CALCOP_SIGNP;
				}
				break;
			case '-':
				codeMark = D_CALCOP_MINUS;
				if (head > 0){
					codeMark = D_CALCOP_SIGNM;
				}
				break;
			case '*':
				codeMark = D_CALCOP_MUL;
				break;
			case '/':
				codeMark = D_CALCOP_DIV;
				break;
			case '%':
				codeMark = D_CALCOP_MOD;
				break;
			case '!':
				codeMark = D_CALCOP_NOT;
				break;
			case 'S':
				codeMark = D_CALCOP_SEC;
				break;
			case 'F':
				codeMark = D_CALCOP_FRM;
				break;
			case '(':
				codeMark = D_CALCOP_PARS;
				break;
			case ')':
				codeMark = D_CALCOP_PARE;
				break;
			case '<':
				codeMark = D_CALCOP_CMPLT;
				break;
			case '>':
				codeMark = D_CALCOP_CMPGT;
				break;
			case '&':
				codeMark = D_CALCOP_B_AND;
				break;
			case '^':
				codeMark = D_CALCOP_B_XOR;
				break;
			case '|':
				codeMark = D_CALCOP_B_OR;
				break;
			default:
				codeMark = D_CALCOP_ERROR;
				break;
		}
	}
	return codeMark;
}

//---------------------------------------------------------------------
// ���Z�p�ɂQ�������Z�q�̕����F��
// ���́F
//   ch1   : �F�������镶���i�P�����ځj
//   ch2   : �F�������镶���i�Q�����ځj
//   head : 0=�ʏ�  1=�擪�����Ƃ��ĔF��
// �o�́F
//   �Ԃ�l�F �F���R�[�h
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcCodeTwoChar(char ch1, char ch2, int head){
	int codeMark = -1;

	switch(ch1){
		case '=':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPEQ;
			}
			else if (ch2 == '<'){
				codeMark = D_CALCOP_CMPLE;
			}
			else if (ch2 == '>'){
				codeMark = D_CALCOP_CMPGE;
			}
			break;
		case '<':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPLE;
			}
			break;
		case '>':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPGE;
			}
			break;
		case '!':
			if (ch2 == '='){
				codeMark = D_CALCOP_CMPNE;
			}
			break;
		case '&':
			if (ch2 == '&'){
				codeMark = D_CALCOP_L_AND;
			}
			break;
		case '|':
			if (ch2 == '|'){
				codeMark = D_CALCOP_L_OR;
			}
			break;
		case '+':
			if (ch2 == '+' && head == 1){
				codeMark = D_CALCOP_P_INC;
			}
			break;
		case '-':
			if (ch2 == '-' && head == 1){
				codeMark = D_CALCOP_P_DEC;
			}
			break;
	}
	return codeMark;
}

//---------------------------------------------------------------------
// ��������~���b���Ԃɕϊ�
// ���́F
//   cstr : ������
//   st   : �F���J�n�ʒu
//   ed   : �F���I���ʒu
//   dsec : �������̒l�i0=�t���[����  1=�b��  2=�ϊ��Ȃ��j
//   draw : �揜�Z����̏����i0=�ʏ�  1=�P�ʕϊ����~�j
// �o�́F
//   �Ԃ�l�F ���Z���ʃ~���b
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcTime(const string &cstr, int st, int ed, int dsec, int draw){

	//--- �����񂩂琔�l���擾 ---
	int categMark_i;
	int codeMark_i;
	int vin = 0;				// �����������Z�r��
	int val = 0;				// �����������l����
	int vms = 0;				// �~���b�������l����
	int flag_sec = 0;			// 1:���Ԃł̋L��
	int flag_prd = 0;			// �~���b�̃s���I�h�F��
	int mult_prd = 0;			// �~���b�̉��Z�P��
	for(int i=st; i<=ed; i++){
		codeMark_i = getStrCalcCodeChar(cstr[i], 0);
		categMark_i = getMarkCategory(codeMark_i);
		if (categMark_i == D_CALCCAT_IMM){			// �f�[�^
			if (codeMark_i == D_CALCOP_COLON){		// �����b�̋�؂�
				flag_sec = 1;
				val = (val + vin) * 60;
				vin = 0;
			}
			else if (codeMark_i == D_CALCOP_PERD){	// �~���b�ʒu�̋�؂�
				flag_sec = 1;
				flag_prd ++;
				mult_prd = 100;
				val += vin;
				vin = 0;
			}
			else{
				if (flag_prd == 0){				// ��������
					vin = vin * 10 + codeMark_i;
				}
				else if (flag_prd == 1){		// �~���b����
					vms = codeMark_i * mult_prd + vms;
					mult_prd = mult_prd / 10;
				}
			}
		}
		else{
			throw i;				// ���Ԃ�\�������ł͂Ȃ��G���[
		}
	}
	val += vin;
	//--- �P�ʕϊ����ďo�� ---
	int data;
	if (draw > 0 || dsec == 2){		// �P�ʕϊ����Ȃ��ꍇ
		data = val;
	}
	else if (flag_sec == 0){		// ���͕�����͐����f�[�^
		if (dsec == 0){				// �������̓t���[���P�ʂ̐ݒ莞
			data = getMsecFromFrm(val);
		}
		else{						// �������͕b�P�ʂ̐ݒ莞
			data = val * 1000;
		}
	}
	else{							// ���͕�����͎��ԃf�[�^
		data = val * 1000 + vms;
	}
	return data;
}


//---------------------------------------------------------------------
// �P�����Z
// ���́F
//   din   : ���Z���l
//   codeMark : �P�����Z�q
// �o�́F
//   �Ԃ�l�F ���Z���ʃ~���b
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcOp1(int din, int codeMark){
	int ret;

	switch(codeMark){
		case D_CALCOP_NOT:
			ret = 0;
			if (din == 0){
				ret = 1;
			}
			break;
		case D_CALCOP_SIGNP:
			ret = din;
			break;
		case D_CALCOP_SIGNM:
			ret = din * -1;
			break;
		default:
			ret = din;
			break;
	}
	return ret;
}

//---------------------------------------------------------------------
// �Q�����Z
// ���́F
//   din1  : ���Z���l
//   din2  : ���Z���l
//   codeMark : �Q�����Z�q
// �o�́F
//   �Ԃ�l�F ���Z���ʃ~���b
//---------------------------------------------------------------------
int CnvStrTime::getStrCalcOp2(int din1, int din2, int codeMark){
	int ret;

//printf("(%d,%d,%d)",din1,din2,codeMark);
	switch(codeMark){
		case D_CALCOP_PLUS:
			ret = din1 + din2;
			break;
		case D_CALCOP_MINUS:
			ret = din1 - din2;
			break;
		case D_CALCOP_MUL:
			ret = din1 * din2;
			break;
		case D_CALCOP_DIV:
			if ( din2 == 0 ){
				throw din2;
			}
			ret = din1 / din2;
//			ret = (din1 + (din2/2)) / din2;
			break;
		case D_CALCOP_MOD:
			ret = din1 % din2;
			break;
		case D_CALCOP_CMPLT:
			ret = (din1 < din2)? 1 : 0;
			break;
		case D_CALCOP_CMPLE:
			ret = (din1 <= din2)? 1 : 0;
			break;
		case D_CALCOP_CMPGT:
			ret = (din1 > din2)? 1 : 0;
			break;
		case D_CALCOP_CMPGE:
			ret = (din1 >= din2)? 1 : 0;
			break;
		case D_CALCOP_CMPEQ:
			ret = (din1 == din2)? 1 : 0;
			break;
		case D_CALCOP_CMPNE:
			ret = (din1 != din2)? 1 : 0;
			break;
		case D_CALCOP_B_AND:
			ret = (din1 & din2);
			break;
		case D_CALCOP_B_XOR:
			ret = (din1 ^ din2);
			break;
		case D_CALCOP_B_OR:
			ret = (din1 | din2);
			break;
		case D_CALCOP_L_AND:
			ret = (din1 && din2);
			break;
		case D_CALCOP_L_OR:
			ret = (din1 || din2);
			break;
		default:
			ret = din1;
			break;
	}
	return ret;
}



