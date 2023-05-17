//
// ���ˑ��Ŗʓ|�����ȏ������܂Ƃ߂�����
//
#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "CommonJls.hpp"

using namespace LcParam;

//---------------------------------------------------------------------
// ���ʗ��p�̕W���o�́^�G���[�ϐ�
//---------------------------------------------------------------------
LocalOutStream lcout;
LocalErrStream lcerr;
LocalSys LSys;
LocalWbCnv LWbCnv;

//---------------------------------------------------------------------
// �V�X�e������
//---------------------------------------------------------------------
//
// �W���o�́^�W���G���[����
//
void LocalSys::bufcout(const string& buf){
	cout << wbc.cnvToFileString(buf, m_utfStd);
	bufMemoInsSel(buf, true, false);	// ���������ݒ�
}
void LocalSys::bufcerr(const string& buf){
	cerr << wbc.cnvToFileString(buf, m_utfStd);
	bufMemoInsSel(buf, false, true);	// ���������ݒ�
}
void LocalSys::bufMemoIns(const string& buf){
	bufMemoInsSel(buf, false, false);
}
void LocalSys::bufMemoInsSel(const string& buf, bool chkStd, bool chkErr){
	if ( (m_nMemoHold & 0x01) == 0 && chkStd ) return;
	if ( (m_nMemoHold & 0x02) == 0 && chkErr ) return;
	m_listMemo.push_back(buf);		// ���O�Ƃ��ĕۊ�
}
void LocalSys::bufMemoFlush(LocalOfs& ofs){
	int sizeLine = (int)m_listMemo.size();
	for(int i=0; i<sizeLine; i++){
		if ( ofs.is_open() ){
			ofs.write(m_listMemo[i]);	// ���O���t�@�C���ɏo��
		}else{
			cout << wbc.cnvToFileString(m_listMemo[i], m_utfStd);
		}
	}
	m_listMemo.clear();
}
//
// �t�H���_�쐬
//
bool LocalSys::cmdMkdir(const string& strName){
	filesystem::path fname = wbc.getWstrFromUtf8(strName);
	return filesystem::create_directory(fname);
}
//
// �t�@�C���R�s�[
//
bool LocalSys::cmdCopy(const string& strFrom, const string& strTo){
	filesystem::path fstrFrom = wbc.getWstrFromUtf8(strFrom);
	filesystem::path fstrTo   = wbc.getWstrFromUtf8(strTo);
	return filesystem::copy_file(fstrFrom, fstrTo, filesystem::copy_options::overwrite_existing);
}
//
// �J�����g�t�H���_�擾
//
string LocalSys::getCurrentPath(){
	wstring fpath = filesystem::current_path();
	return wbc.getUtf8FromWstr(fpath);
}
//
// ���ϐ����擾
//   Windows��Unicode��������g��������΃��C�h�o�C�g�Ŏ擾����UTF-8�ɕϊ�����
//
bool LocalSys::getEnvString(string& strVal, const string& strEnvName){
#if defined(_WIN32) && defined(USE_SAFETY_CALL)		// ���C�h�o�C�g�ɂ��擾
	wstring wname = wbc.getWstrFromUtf8(strEnvName);
	size_t retSize;
	_wgetenv_s(&retSize, NULL, 0, wname.c_str());
	bool success = false;
	if ( retSize > 0 ){
		wchar_t* wbuf = new(std::nothrow) wchar_t[retSize];
		if ( wbuf != nullptr ){
			_wgetenv_s(&retSize, wbuf, retSize, wname.c_str());
			strVal = wbc.getUtf8FromWstr(wbuf);
			delete [] wbuf;
			success = true;
		}
	}
	if ( success ) return true;
#elif defined(_WIN32)		// ���C�h�o�C�g�ɂ��擾
	wstring wname = wbc.getWstrFromUtf8(strEnvName);
	const wchar_t *pstr = _wgetenv(wname.c_str());
	if ( pstr != nullptr ){
		strVal = wbc.getUtf8FromWstr(pstr);
		return true;
	}
#elif defined(USE_SAFETY_CALL)		// C11��`�̈��S�Ȏ�荞�݁iC++�ł͎����ˑ��j
	size_t retSize;
	getenv_s(&retSize, NULL, 0, strEnvName.c_str());
	if ( retSize > 0 ){
		char* buf = new(std::nothrow) char[retSize];
		if ( buf == nullptr ) return false;
		getenv_s(&retSize, buf, retSize, strEnvName.c_str());
		strVal = buf;
		delete [] buf;
		return true;
	}
#else
	const char *pstr = getenv(strEnvName.c_str());
	if ( pstr != nullptr ){
		strVal = pstr;
		return true;
	}
#endif
	strVal = "";
	return false;
}
//
// �N�����̈������擾
//   �ݒ�Ȃ���΋N������main���������̂܂܎g�p����
//   Windows��Unicode��������g��������΃��C�h�o�C�g(Windows�p)�Ŏ擾����UTF-8�ɕϊ�
//
vector<string> LocalSys::getMainArg(int argc, char *argv[]){
	vector<string> listArg;
#if defined(_WIN32)			// ���C�h�o�C�g�ɂ������擾(MS-windows)
	//--- windows�R�}���h�ň����擾 ---
	wchar_t* lpLine = ::GetCommandLineW();
	int nArgc = 0;
	wchar_t** lppArgv = ::CommandLineToArgvW( lpLine, &nArgc );
	for(int i=0; i<nArgc; i++){
		string str = wbc.getUtf8FromWstr(lppArgv[i]);
		listArg.push_back(str);
	}
#else		// �ݒ�Ȃ����main���������̂܂܎g�p
	for(int i=0; i<argc; i++){
		string str = argv[i];
		listArg.push_back(str);
	}
#endif
	return listArg;
}
//
// ���O���ێ��ݒ�
//
void LocalSys::setMemoSel(int n){
	m_nMemoHold = n;
	if ( n == -1 ){
		m_listMemo.clear();
		m_nMemoHold = 0;
	}
}
//
// �����R�[�h�̃X�N���v�g���ԍ�-���R�[�h�Ԃ�ϊ�
//
LcParam::UtfType LocalSys::getUtfCodeFromNum(int num){
	LcParam::UtfType utfcode;
	switch( num ){
		case 1:
			utfcode = LcParam::UtfDefault;
			break;
		case 2:
			utfcode = UtfType::UTF8;
			break;
		case 3:
			utfcode = UtfType::UTF16LE;
			break;
		case 4:
			utfcode = UtfType::UTF16BE;
			break;
		default:
			break;
	}
	return utfcode;
}
int LocalSys::getUtfNumFromCode(LcParam::UtfType utfcode){
	int num;
	switch( utfcode ){
		case UtfType::SJIS :
			num = 1;
			break;
		case UtfType::UTF8 :
			num = 2;
			break;
		case UtfType::UTF16LE :
			num = 3;
			break;
		case UtfType::UTF16BE :
			num = 4;
			break;
		default:
			num = 0;
			break;
	}
	return num;
}
int LocalSys::getUtfNumFromStr(const string& strUtf){
	int num;
	if ( strUtf == "1" || strUtf == "STD" ){
		num = 1;
	}
	else if ( strUtf == "2" || strUtf == "UTF8" ){
		num = 2;
	}
	else if ( strUtf == "3" || strUtf == "UTF16" || strUtf == "UTF16LE" ){
		num = 3;
	}
	else if ( strUtf == "4" || strUtf == "UTF16BE" ){
		num = 4;
	}
	else if ( strUtf == "0" ){
		num = 0;
	}
	else{
		num = -1;
	}
	return num;
}
//
// �W���o�́^�W���G���[�̕����R�[�h�ݒ�
//
void LocalSys::setStdUtfCodeFromNum(int num){
	if ( num > 0 && num <= 4 ){
		m_utfStd = getUtfCodeFromNum(num);
	}
}
void LocalSys::setStdUtfCode(LcParam::UtfType utfcode){
	m_utfStd = utfcode;
}
int LocalSys::getStdUtfNum(){
	return getUtfNumFromCode(m_utfStd);
}
LcParam::UtfType LocalSys::getStdUtfCode(){
	return m_utfStd;
}
//
// �t�@�C���o�͂̃f�t�H���g�����R�[�h�ݒ�
//
void LocalSys::setFileUtfCodeFromNum(int num){
	if ( num > 0 && num <= 4 ){
		m_utfFile = getUtfCodeFromNum(num);
	}
}
void LocalSys::setFileUtfCode(LcParam::UtfType utfcode){
	m_utfFile = utfcode;
}
int LocalSys::getFileUtfNum(){
	return getUtfNumFromCode(m_utfFile);
}
LcParam::UtfType LocalSys::getFileUtfCode(){
	return m_utfFile;
}
//
// �f�o�b�O�p
//
void LocalSys::echoCodeWB(wstring str, int len){	// WideByte�������16�i���ŕ\��
	if ( len < 0 ) len = (int)str.length();
	cout << "len(WB):" << len << "\n";
	for(int i=0; i<len; i++){
		wchar_t ch = str[i];
		fprintf(stdout, "%04x ", (unsigned int)ch );
	}
	cout << "\n";
}
void LocalSys::echoCodeByte(string str, int len){	// UTF-8�̕������16�i���ŕ\��
	if ( len < 0 ) len = (int)str.length();
	cout << "len(byte):" << len << "\n";
	for(int i=0; i<len; i++){
		unsigned char ch = str[i];
		fprintf(stdout, "%02x ", (unsigned int)ch );
	}
	cout << "\n";
}
//---------------------------------------------------------------------
// �W���X�g���[��(lcout/lcerr�p)
//---------------------------------------------------------------------
//--- ���������p ---
iostream::int_type LocalUtf8StreamBuf::overflow(iostream::int_type ich){
	if ( ich == EOF ) return ich;
	if ( m_pos < 0 || m_pos >= 4 ){		// �O�̂��ߔ͈͓��m�F
		m_pos = 0;
	}
	char ch = (char) ich;
	m_buf[m_pos++] = ch;
	if ( m_pos == 1 ){		// �擪����
		m_size = wbc.getNeedByteFromUtf8Head(ch);
	}
	if ( m_pos == m_size ){		// UTF-8�̕����P�ʂő��M
		xsputn(m_buf, m_size);
		m_pos = 0;
	}else if ( m_size < 0 ){
		m_pos = 0;
	}
	return 1;
}
std::streamsize LocalOutStreamBuf::xsputn(const iostream::char_type* s, std::streamsize count){
	string str(s, count);
	LSys.bufcout(str);
	return count;
}
std::streamsize LocalErrStreamBuf::xsputn(const iostream::char_type* s, std::streamsize count){
	string str(s, count);
	LSys.bufcerr(str);
	return count;
}
//---------------------------------------------------------------------
// �t�@�C���^�C�v���
//---------------------------------------------------------------------
//--- �t�@�C����BOM����ݒ� ---
bool LocalFileType::setFromFile(const string& strName){
	filesystem::path fname = wbc.getWstrFromUtf8(strName);
	//--- BOM�擾 ---
	ifstream ifs_bom(fname);
	if ( ifs_bom.fail() ) return false;
	unsigned long bomid = 0;
	for(int i=0; i<4; i++){
		unsigned char ch = ifs_bom.get();
		bomid = ( bomid << 8 ) + (unsigned long) ch;
	}
	//--- BOM���� ---
	LcParam::UtfType selUtf;
	bool selBom = false;
	if ( (bomid >> 8) == 0xEFBBBF ){
		selUtf = UtfType::UTF8;
		selBom = true;
	}
	else if ( (bomid >> 16) == 0xFFFE ){
		selUtf = UtfType::UTF16LE;
		selBom = true;
	}
	else if ( (bomid >> 16) == 0xFEFF ){
		selUtf = UtfType::UTF16BE;
		selBom = true;
	}
	else{
		selUtf = LcParam::UtfDefault;	// windows��BOM�Ȃ���Shift-JIS�Ɣ���
		selBom = false;
	}
	setDirect(selUtf);
	if ( !selBom ) m_bom = 0;	// BOM�����ݎ�
	ifs_bom.close();
	return true;
}
//--- ���ڐݒ� ---
void LocalFileType::setDirect(LcParam::UtfType utfcode){
	m_utf = utfcode;
	switch( m_utf ){
		case UtfType::UTF8 :
			m_unit = 1;
			m_bom = 3;
			m_strbom.assign({(char)0xEF, (char)0xBB, (char)0xBF});
			break;
		case UtfType::UTF16LE :
			m_unit = 2;
			m_bom = 2;
			m_strbom.assign({(char)0xFF, (char)0xFE});
			break;
		case UtfType::UTF16BE :
			m_unit = 2;
			m_bom = 2;
			m_strbom.assign({(char)0xFE, (char)0xFF});
			break;
		default :
			m_unit = 1;
			m_bom = 0;
			break;
	}
	m_set = true;
}
bool LocalFileType::isSet(){
	return m_set;
}
//---------------------------------------------------------------------
// ifstream����
//---------------------------------------------------------------------
bool LocalIfs::open(const string& strName){
	if ( flagOpen ) return false;
	//--- �t�@�C����ގ擾 ---
	if ( !attr.setFromFile(strName) ){
		return false;
	}
	//--- Windows�p�X�Ή� ---
	filesystem::path fname = wbc.getWstrFromUtf8(strName);
	//--- open���� ---
	if ( attr.unit() != 2 ){
		ifs.open(fname);
	}else{		// 2�o�C�g�P�ʓǂݏo���̓o�C�i������
		ifs.open(fname, ios::binary);
	}
	if ( ifs.fail() ) return false;
	ifs.ignore(attr.bom());	// BOM�ǂݔ�΂�
	flagOpen = true;
	return true;
}
bool LocalIfs::getline(string& buf){
	if ( !flagOpen ) return false;
	string lfbuf;
	if ( !getlineCore(lfbuf) ){
		return false;
	}
	buf = wbc.cnvFromFileString(lfbuf, attr.utf());
	return true;
}
bool LocalIfs::getlineCore(string& buf){	// ���s�R�[�h�̕␳�t��
	if ( attr.unit() != 2 ){
		std::getline(ifs, buf);
		return ( ifs.fail() == 0 );		// 2�o�C�g�P�ʈȊO�͒ʏ�ǂݏo��
	}
	//--- 2�o�C�g�P�ʂ͉��s�R�[�h��F�����邽�ߕʓr�쐬 ---
	buf.clear();
	auto posBase = ifs.tellg();
	auto posCur  = posBase;
	unsigned int codeCR = 0x000D;
	unsigned int codeLF = 0x000A;
	int sw = ( attr.utf() == UtfType::UTF16LE )? 1 : 0;
	bool cont = true;
	bool needBk = false;
	bool flagCR = false;
	bool flagLF = false;
	while( cont && !needBk && !flagLF ){
		posCur = ifs.tellg();
		unsigned char ch1 = ifs.get();
		unsigned char ch2 = ifs.get();
		unsigned int codeCur;
		if ( sw ){
			codeCur = (((unsigned int)ch2) << 8) + ((unsigned int)ch1);
		}else{
			codeCur = (((unsigned int)ch1) << 8) + ((unsigned int)ch2);
		}
		if ( ifs.fail() ){
			cont = false;
		}else if ( codeCur == codeCR ){
			if ( flagCR ){
				needBk = true;
			}else{
				flagCR = true;
			}
		}else if ( codeCur == codeLF ){
			flagLF = true;
		}else if ( flagCR ){
			needBk = true;
		}else{
			buf += ch1;
			buf += ch2;
		}
	}
	if ( needBk ){
		ifs.seekg(posCur, ios_base::beg);
	}
	if ( posBase == posCur ){
		if ( !flagCR && !flagLF ) return false;
	}
	return true;
}
void LocalIfs::close(){
	flagOpen = false;
	ifs.close();
}
bool LocalIfs::is_open(){
	return flagOpen;
}
int LocalIfs::getCodeNum(){
	return LSys.getUtfNumFromCode(attr.utf());
}
//---------------------------------------------------------------------
// ofstream����
//---------------------------------------------------------------------
bool LocalOfs::open(const string& strName, bool append){
	if ( flagOpen ) return false;
	//--- �t�@�C����ސݒ� ---
	if ( !attr.isSet() ){		// �R�[�h���ݒ�Ȃ�V�X�e���ݒ���g�p
		attr.setDirect(LSys.getFileUtfCode());
	}
	//--- Windows�p�X�Ή� ---
	filesystem::path fname = wbc.getWstrFromUtf8(strName);
	//--- open���� ---
	if ( append ){
		if ( attr.unit() != 2 ){
			ofs.open(fname, ios::app);
		}else{		// WideByte�̓o�C�i������
			ofs.open(fname, ios::app | ios::binary);
		}
	}else{
		if ( attr.unit() != 2 ){
			ofs.open(fname);
		}else{		// WideByte�̓o�C�i������
			ofs.open(fname, ios::binary);
		}
	}
	if ( ofs.fail() ) return true;
	if ( attr.bom() > 0 && !append ){
		ofs << attr.strbom();	// BOM�ݒ�
	}
	flagOpen = true;
	return true;
}
bool LocalOfs::append(const string& strName){
	return open(strName, true);
}
bool LocalOfs::write(const string& buf){
	if ( !flagOpen ) return false;
	string revbuf = writeRevStr(buf);		// WideByte�R�[�h�̓o�C�i���ŏ������邽�ߕ␳���K�v
	string lfbuf = wbc.cnvToFileString(revbuf, attr.utf());
	ofs << lfbuf;
	return true;
}
string LocalOfs::writeRevStr(const string& buf){		// WideByte�R�[�h�Ή��␳
	if ( attr.unit() != 2 ) return buf;	// �ʏ�e�L�X�g�����͉������Ȃ�

//--- �o�C�i�������ł́AWindows�̂�LF��CR+LF�ɕς��鏈�� ---
	string revbuf = buf;
#if defined(_WIN32)
	unsigned char chCR = 0x0D;
	unsigned char chLF = 0x0A;
	auto pos = revbuf.rfind(chLF);
	while( pos != string::npos ){
		bool needCR = true;
		if ( pos > 0 ){		// ����CR+LF�ɂȂ��Ă�����}�����Ȃ�
			if ( revbuf[pos-1] == chCR ) needCR = false;
		}
		if ( needCR ){
			revbuf.insert(pos, 1, chCR);
		}
		pos = revbuf.rfind(chLF, pos-1);
	}
#endif
	return revbuf;
}	
void LocalOfs::close(){
	flagOpen = false;
	ofs.close();
}
bool LocalOfs::is_open(){
	return flagOpen;
}
void LocalOfs::setCodeTypeFromNum(int num){
	LcParam::UtfType utfcode = LSys.getUtfCodeFromNum(num);
	attr.setDirect(utfcode);
}
void LocalOfs::setCodeType(LcParam::UtfType utfcode){
	attr.setDirect(utfcode);
}
//---------------------------------------------------------------------
// UTF-8�̕����񑀍�
//---------------------------------------------------------------------
//--- ���ۂ̕����񒷂�Ԃ� ---
int LocalStr::getStrLen(const string& str){
	return wbc.getWlenFromUtf8(str);
}
//--- �w��ʒu�̕����������Ԃ��i�w��ʒu����Ō�܂Łj ---
string LocalStr::getSubStr(const string& str, int st){
	return getSubStrCommon(str, st, 0, false);		// validLen=false
}
//--- �w��ʒu�̕����������Ԃ��i�w��ʒu����w�蕶�����j ---
string LocalStr::getSubStrLen(const string& str, int st, int len){
	return getSubStrCommon(str, st, len, true);		// validLen=true
}
string LocalStr::getSubStrCommon(const string& str, int st, int len, bool validLen){
	wstring wstr = wbc.getWstrFromUtf8(str);
	int wlen = (int)wstr.length();
	string strSub;
	if ( wlen > 0 ){
		int wlocS = ( st >= 0 )? st : wlen - st;
		int wlocE = wlen;
		if ( validLen ){
			wlocE = ( len >= 0 )? wlocS + len : wlen - len;
		}
		bool valid = true;
		if ( wlocS >= wlen ) valid = false;
		if ( wlocE < wlocS ) valid = false;
		if ( wlocE > wlen  ) wlocE = wlen;
		if ( valid ){
			wstring wsub = wstr.substr(wlocS, wlocE - wlocS);
			strSub = wbc.getUtf8FromWstr(wsub);
		}
	}
	return strSub;
}
//--- �������X�g�Ɋ܂܂�镶���̐���Ԃ� ---
int LocalStr::countInStr(const string& strSrc, const string& strEach){
	int count = 0;
	wstring wstrSrc = wbc.getWstrFromUtf8(strSrc);
	wstring wstrEach = wbc.getWstrFromUtf8(strEach);
	for(int i=0; i<(int)wstrSrc.length(); i++){
		wchar_t wchSrc = wstrSrc[i];
		for(int j=0; j<(int)wstrEach.length(); j++){
			if ( wstrEach[j] == wchSrc ){
				count ++;
				break;
			}
		}
	}
	return count;
}
//--- �������X�g(strEach)�̕�����Ή�����w�蕶��(strSub)�ɕϊ� ---
bool LocalStr::replaceInStr(string& strSrc, const string& strEach, const string& strSub){
	wstring wstrSrc  = wbc.getWstrFromUtf8(strSrc);
	wstring wstrEach = wbc.getWstrFromUtf8(strEach);
	wstring wstrSub  = wbc.getWstrFromUtf8(strSub);
	if ( wstrEach.length() != wstrSub.length() ) return false;
	for(int i=0; i<(int)wstrSrc.length(); i++){
		wchar_t wchSrc = wstrSrc[i];
		for(int j=0; j<(int)wstrEach.length(); j++){
			if ( wstrEach[j] == wchSrc ){
				wstrSrc[i] = wstrSub[j];
				break;
			}
		}
	}
	strSrc = wbc.getUtf8FromWstr(wstrSrc);
	return true;
}
//--- ���K�\���Ń}�b�`�����ʒu�̍��v��Ԃ� ---
int LocalStr::countRegExMatch(const string& strSrc, const string& strRe){
	//--- �ėp���K�\���g�p�ɂ̓��C�h�o�C�g���K�v�Ȃ��ߕϊ� ---
	wstring wstrSrc = wbc.getWstrFromUtf8(strSrc);
	wstring wstrRe  = wbc.getWstrFromUtf8(strRe);
	if ( wstrSrc.empty() || wstrRe.empty() ){
		return 0;
	}
	//--- ���C�h�o�C�g�Ő��K�\���̊m�F ---
	int count = 0;
	std::wregex re(wstrRe);
	auto iter = wstrSrc.cbegin();
	auto end  = wstrSrc.cend();
	std::wsmatch m;
	while( std::regex_search(iter, end, m, re) ){
		count ++;
		iter = m[0].second;
	}
	return count;
}
//--- ���K�\���Ń}�b�`�����������Ԃ� ---
string LocalStr::getRegMAtch(const string& strSrc, const string& strRe){
	wstring wstrSrc = wbc.getWstrFromUtf8(strSrc);
	wstring wstrRe  = wbc.getWstrFromUtf8(strRe);
	string strRet;
	if ( wstrSrc.empty() || wstrRe.empty() ){
		return strRet;
	}
	//--- ���C�h�o�C�g�Ő��K�\���̊m�F ---
	std::wregex re(wstrRe);
	std::wsmatch m;
	if ( std::regex_search(wstrSrc, m, re) ){
		wstring wstrRet = m[0].str();
		strRet = wbc.getUtf8FromWstr(wstrRet);
	}
	return strRet;
}


//---------------------------------------------------------------------
// �t�@�C�� - UTF-8 �����R�[�h�ϊ�
//---------------------------------------------------------------------
//--- �w��`���̕������UTF-8�ɕϊ� ---
string LocalWbCnv::cnvFromFileString(const string& lstr, LcParam::UtfType utype){
	switch( utype ){
		case UtfType::UTF8 :
			return lstr;
			break;
#if defined(WIDE_BYTE_SJIS)		// Shift-JIS�����iWindows��p�j
		case UtfType::SJIS :
			{
				wstring wstr = getWstrFromSjis(lstr);
				return getUtf8FromWstr(wstr);
			}
			break;
#endif
		case UtfType::UTF16LE :
		case UtfType::UTF16BE :
			{
				int wlen = (int)((lstr.length()) / 2);
				int sw = ( utype == UtfType::UTF16LE )? 1 : 0;
				wstring wstr(wlen, 'a');	// �̈�m��
				for(auto i=0; i<wlen; i++){
					unsigned char chh = lstr[i*2+sw];
					unsigned char chl = lstr[i*2+1-sw];
					unsigned long val = (((unsigned long) chh)<<8) + chl;
					wstr[i] = (wchar_t) val;
				}
				return getUtf8FromWstr(wstr);
			}
			break;
		default:
			break;
	}
	string ustr;
	return ustr;
}
//--- UTF-8�̕�������w��`���ɕϊ� ---
string LocalWbCnv::cnvToFileString(const string& ustr, LcParam::UtfType utype){
	switch( utype ){
		case UtfType::UTF8 :
			return ustr;
			break;
#if defined(WIDE_BYTE_SJIS)		// Shift-JIS�����iWindows��p�j
		case UtfType::SJIS :
			{
				wstring wstr = getWstrFromUtf8(ustr);
				return getSjisFromWstr(wstr);
			}
			break;
#endif
		case UtfType::UTF16LE :
		case UtfType::UTF16BE :
			{
				wstring wstr = getWstrFromUtf8(ustr);
				int wlen = (int)wstr.length();
				int sw = ( utype == UtfType::UTF16LE )? 1 : 0;
				string lstr(wlen*2, 'a');	// �̈�m��
				for(auto i=0; i<wlen; i++){
					wchar_t wch = wstr[i];
					unsigned long val = (unsigned long) wch;
					unsigned char chh = (unsigned char)(val >> 8);
					unsigned char chl = (unsigned char)(val & 0xFF);
					lstr[i*2+sw] = chh;
					lstr[i*2+1-sw] = chl;
				}
				return lstr;
			}
			break;
		default:
			break;
	}
	string lstr;
	return lstr;
}

#if defined(WIDE_BYTE_SJIS)		// Shift-JIS�����iWindows��p�j
//--- Shift-JIS -> WideByte �iWindows��p�j ---
wstring LocalWbCnv::getWstrFromSjis(const string& str){
	wstring wstr;
	// Windows�R�}���h(CP_ACP=932 : Shift-JIS)
	const int wlen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	wchar_t* wbuf = new(std::nothrow) wchar_t[wlen];
	if ( wbuf == nullptr ) return wstr;
	if ( ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuf, wlen) ){
		wstr = wbuf;
	}
	delete [] wbuf;
	return wstr;
}
//--- WideByte -> Shift-JIS �iWindows��p�j ---
string LocalWbCnv::getSjisFromWstr(const wstring& wstr){
	string str;
	// Windows�R�}���h(CP_ACP=932 : Shift-JIS)
	const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* buf = new(std::nothrow) char[len];
	if ( buf == nullptr ) return str;
	if ( ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf, len, nullptr, nullptr) ){
		str = buf;
	}
	delete [] buf;
	return str;
}
#endif

//---------------------------------------------------------------------
// WideByte - UTF-8 ������ϊ�����
//---------------------------------------------------------------------
//--- UTF-8�̎��ۂ̕����񒷂��擾 ---
int LocalWbCnv::getWlenFromUtf8(const string& str){
	int nlen = (int) str.length();
	int mlen = 0;
	int i = 0;
	while( i < nlen && i >= 0 ){
		int mblen = getWordByteFromUtf8(str, i);
		if ( mblen <= 0 ){
			i = -1;			// �ُ�I��
		}else{
			mlen ++;
			i += mblen;
		}
	}
	return mlen;
}
//--- WideByte��UTF-8�ɕϊ��������ɕK�v�ȕ����񒷂��擾 ---
int LocalWbCnv::getLenToUtf8(const wstring& wstr){
	int len = 0;
	for(int i=0; i<(int)wstr.length(); i++){
		wchar_t ch = wstr[i];
		long val = (long) ch;
		if ( sizeof(wchar_t) == 2 ) val &= 0xFFFF;
		if ( val <= 0x007F ) len += 1;
		else if ( val <= 0x07FF ) len += 2;
		else if ( val <= 0xFFFF ) len += 3;
		else len += 4;
	}
	return len;
}
//--- UTF-8��WideByte������ɂ��Ď擾 ---
wstring LocalWbCnv::getWstrFromUtf8(const string& str){
	wstring wstr;
	int wlen = getWlenFromUtf8(str);
	if ( wlen <= 0 ) return wstr;

	wchar_t* wbuf = new(std::nothrow) wchar_t[wlen+1];
	if ( wbuf == nullptr ) return wstr;
	int pos = 0;
	for(int i=0; i<wlen+1; i++){
		int sz;
		wbuf[i] = getWordStrFromUtf8(sz, str, pos);
		pos += sz;
	}
	if ( pos == (int)str.length() ){		// �O�̂��ߊm�F
		wstr = wbuf;
	}
	delete [] wbuf;
	return wstr;
}
//--- WideByte�������UTF-8������ɂ��Ď擾 ---
string LocalWbCnv::getUtf8FromWstr(const wstring& wstr){
	string str;
	int wlen = (int)wstr.length();
	int len = getLenToUtf8(wstr);
	if ( len <= 0 ) return str;

	char* buf = new(std::nothrow) char[len+1];
	if ( buf == nullptr ) return str;
	int pos = 0;
	for(int i=0; i<wlen+1; i++){
		int sz = getWordUtf8FromWstr(&buf[pos], wstr, i);
		pos += sz;
	}
	if ( pos == len ){		// �O�̂��ߊm�F
		str = buf;
	}
	delete [] buf;
	return str;
}
//--- UTF-8�̐擪��������K�v�ȃo�C�g�����擾�i-1�̎��͐擪�����ł͂Ȃ��j ---
int LocalWbCnv::getNeedByteFromUtf8Head(const char ch){
	if ( (ch & 0x80) == 0 ) return 1;
	if ( (ch & 0x40) == 0 ) return -1;
	if ( (ch & 0x20) == 0 ) return 2;
	if ( (ch & 0x10) == 0 ) return 3;
	return 4;
}
//--- UTF-8�̑Ώۈʒu����1������byte�����擾 ---
int LocalWbCnv::getWordByteFromUtf8(const string& str, int n){
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
//--- UTF-8�̑Ώۈʒu����1������WideByte�Ŏ擾 ---
wchar_t LocalWbCnv::getWordStrFromUtf8(int& nSize, const string& str, int n){
	int len = (int)str.length();
	if ( n >= len || n < 0 ){
		nSize = 0;
		return (wchar_t) 0;
	}
	unsigned char code = (unsigned char) str[n];
	if ( (code & 0x80) == 0 ){
		nSize = 1;
		return (wchar_t) code;
	}
	nSize = len - n;
	long val = 0;
	if ( (code & 0xE0) == 0xC0 ){
		if ( nSize >= 2 ){
			nSize = 2;
			unsigned char code2 = (unsigned char) str[n+1];
			val = (((unsigned long)(code  & 0x1F)) << 6) +
			       ((unsigned long)(code2 & 0x3F));
		}else{
			val = 0x3F;
		}
	}else if ( (code & 0xF0) == 0xE0 ){
		if ( nSize >= 3 ){
			nSize = 3;
			unsigned char code2 = (unsigned char) str[n+1];
			unsigned char code3 = (unsigned char) str[n+2];
			val = (((unsigned long)(code  & 0x0F)) << 12) +
			      (((unsigned long)(code2 & 0x3F)) << 6 ) +
			       ((unsigned long)(code3 & 0x3F));
		}else{
			val = 0x3F;
		}
	}else{
		if ( nSize >= 4 ){
			nSize = 4;
		}
		val = 0x3F;		// 17bit�ȏ�̃R�[�h�͍��̏��K�v�Ȃ��̂őΉ��Ȃ�
	}
	return (wchar_t) val;
}
//--- WideByte�̑Ώۈʒu1������UTF-8������Ŏ擾 ---
int LocalWbCnv::getWordUtf8FromWstr(char* str, const wstring& wstr, int n){
	int wlen = (int) wstr.length();
	if ( n >= wlen || n < 0 ){
		str[0] = '\0';
		return 0;
	}
	wchar_t wch =wstr[n];
	long val = (long) wch;
	int nSize = 0;
	if ( val <= 0x007F ){
		nSize = 1;
		str[0] = (char) ( val & 0x7F );
	}
	else if ( val <= 0x07FF ){
		nSize = 2;
		str[0] = (char) ( 0xC0 + (0x1F & (val >> 6)) );
		str[1] = (char) ( 0x80 + (0x3F & val) );
	}
	else if ( val <= 0xFFFF ){
		nSize = 3;
		str[0] = (char) ( 0xE0 + (0x0F & (val >> 12)) );
		str[1] = (char) ( 0x80 + (0x3F & (val >> 6)) );
		str[2] = (char) ( 0x80 + (0x3F & val) );
	}
	else{
		nSize = 4;
		str[0] = (char) ( 0xF0 + (0x07 & (val >> 18)) );
		str[1] = (char) ( 0x80 + (0x3F & (val >> 12)) );
		str[2] = (char) ( 0x80 + (0x3F & (val >> 6)) );
		str[3] = (char) ( 0x80 + (0x3F & val) );
	}
	return nSize;
}
