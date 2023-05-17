//
// ���ˑ��Ŗʓ|�����ȏ������܂Ƃ߂�����
//
// Unicode�֘A�̏����́A���o�͂̈Ⴂ��S�������ŋz�����A�����͑S��UTF-8�œ��삳����
// �i�^��string,char�̂܂܁BShift-JIS�݂����Ȑ���R�[�h�d�Ȃ���Ȃ��ȈՉ��ł���j
// Shift-JIS�̏����͑S��OS�ɂ܂�����i���C�h�o�C�gUTF-16�Ԃ�Windows�@�\�ő��ݕϊ��j
// Windows�̂ݓ���m�F�iLinux������l�����č쐬�j
//

#pragma once


#if defined(_MSC_VER)
	// _s���t�����ꂽ���S�ɃA�N�Z�X����֐����g���ꍇ�͒�`
	#define USE_SAFETY_CALL
#endif


#if defined(_WIN32)
  // Shift-JIS���g�p����ꍇ�͒�`�iOS�����̂���Windows�̂ݒ�`�j
  #define WIDE_BYTE_SJIS		// Shift-JIS���g���ꍇ�̓��C�h�o�C�g���g����OS�ŕϊ�
#endif


namespace LcParam
{
	enum class UtfType {		// �����R�[�h���
		none,
		UTF8,
		SJIS,
		UTF16LE,
		UTF16BE,
	};

#if defined(_WIN32)
	static const UtfType UtfDefault = UtfType::SJIS;	// Windows��Shift-JIS���W��
#else
	static const UtfType UtfDefault = UtfType::UTF8;	// Windows�ȊO��UTF-8���W��
#endif
}

namespace LocalInc		// ��ʓI�ȋ��ʃR�}���h���L��
{
	//--- �������`�isnprintf�̗̈�m�ۂ��s����string�ŏo�́j ---
	template<typename ... Args>
	std::string sformat(const std::string& fmt, Args ... args){
		size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args ... );
		std::vector<char> buf(len+1);
		std::snprintf(&buf[0], len+1, fmt.c_str(), args ... );
		return std::string(&buf[0], &buf[0]+len);
	}
}

class LocalOutStream;
class LocalErrStream;
class LocalSys;
class LocalWbCnv;
class LocalOfs;

//---------------------------------------------------------------------
// ���ʗ��p�̕W���o�́^�G���[�ϐ��^�V�X�e������
// cout/cerr �L�ډӏ��� lcout/lcerr �ɕύX���邱�Ƃ�Unicode�Ή�
//---------------------------------------------------------------------
extern LocalOutStream lcout;	// cout����
extern LocalErrStream lcerr;	// cerr����
extern LocalSys LSys;			// �V�X�e������

extern LocalWbCnv LWbCnv;	// Local�e�N���X���Ŏg�p�̕����R�[�h�ϊ�����
//---------------------------------------------------------------------
// �V�X�e������
//---------------------------------------------------------------------
class LocalSys {
	LcParam::UtfType m_utfStd = LcParam::UtfDefault;
	LcParam::UtfType m_utfFile = LcParam::UtfDefault;
	int  m_nMemoHold = 0;
	vector<string> m_listMemo;
	LocalWbCnv& wbc = LWbCnv;
public:
	LocalSys(){ m_listMemo.clear(); };
	//--- ������o�� ---
	void bufcout(const string& buf);
	void bufcerr(const string& buf);
	void bufMemoIns(const string& buf);
private:
	void bufMemoInsSel(const string& buf, bool chkStd, bool chkErr);
public:
	void bufMemoFlush(LocalOfs& ofs);
	//--- OS�R�}���h ---
	bool cmdMkdir(const string& strName);
	bool cmdCopy(const string& strFrom, const string& strTo);
	string getCurrentPath();
	bool getEnvString(string& strVal, const string& strEnvName);
	vector<string> getMainArg(int argc, char *argv[]);

	// �����������ݒ�
	void setMemoSel(int n);
	// �����R�[�h�̔ԍ�-�R�[�h�Ԃ�ϊ�
	LcParam::UtfType getUtfCodeFromNum(int num);
	int  getUtfNumFromCode(LcParam::UtfType utfcode);
	int  getUtfNumFromStr(const string& strUtf);
	// �W���o�́^�W���G���[�̕����R�[�h�ݒ�
	void setStdUtfCodeFromNum(int num);
	void setStdUtfCode(LcParam::UtfType code);
	int  getStdUtfNum();
	LcParam::UtfType getStdUtfCode();
	// �t�@�C���o�͂̃f�t�H���g�����R�[�h�ݒ�
	void setFileUtfCodeFromNum(int num);
	void setFileUtfCode(LcParam::UtfType code);
	int  getFileUtfNum();
	LcParam::UtfType getFileUtfCode();

	// �f�o�b�O�p
	void echoCodeWB(wstring str, int len = -1);
	void echoCodeByte(string str, int len = -1);
};
//---------------------------------------------------------------------
// �W���X�g���[��(lcout/lcerr�p)
//---------------------------------------------------------------------
//--- ���������p ---
class LocalUtf8StreamBuf : public std::streambuf {	// UTF-8�����ۂ̕����P�ʂő��M����
	int m_size = 0;
	int m_pos = 0;
	char m_buf[4];
protected:
	LocalWbCnv& wbc = LWbCnv;	// �����R�[�h�ϊ��֐�
	virtual int_type overflow(int_type ich = EOF);
};
class LocalOutStreamBuf : public LocalUtf8StreamBuf {	// lcout�p�o�b�t�@�o��
protected:
	virtual std::streamsize xsputn(const char_type* s, std::streamsize count);
};
class LocalErrStreamBuf : public LocalUtf8StreamBuf {	// lcerr�p�o�b�t�@�o��
protected:
	virtual std::streamsize xsputn(const char_type* s, std::streamsize count);
};
//--- lcout�X�g���[�� ---
class LocalOutStream : public std::ostream {		// lcout�p�N���X
	LocalOutStreamBuf *m_streambuf;
public:
	~LocalOutStream() { delete m_streambuf; }
	LocalOutStream() : std::ostream(m_streambuf = new LocalOutStreamBuf) {}
};
//--- lcerr�X�g���[�� ---
class LocalErrStream : public std::ostream {		// lcerr�p�N���X
	LocalErrStreamBuf *m_streambuf;
public:
	~LocalErrStream() { delete m_streambuf; }
	LocalErrStream() : std::ostream(m_streambuf = new LocalErrStreamBuf) {}
};
//---------------------------------------------------------------------
// �t�@�C���^�C�v���
//---------------------------------------------------------------------
class LocalFileType {
	bool m_set = false;			// �f�[�^���ݒ肳�ꂽ��true
	LcParam::UtfType m_utf;		// �����R�[�h���
	int m_unit;					// �Œ�P�ʃo�C�g��
	int m_bom;					// BOM�̃o�C�g���i0=BOM�t���Ȃ��j
	string m_strbom;			// BOM�̕�����iBOM�t���Ȃ��̎���������͎c���j
	LocalWbCnv& wbc = LWbCnv;	// �����R�[�h�ϊ��֐�
public:
	bool  setFromFile(const string& strName);
	void  setDirect(LcParam::UtfType utfcode);
	bool  isSet();

	LcParam::UtfType utf() { return m_utf; };	// �����R�[�h���
	int              unit(){ return m_unit; };	// �ŏ��P�ʃo�C�g��
	int              bom() { return m_bom; };	// BOM�������i0=BOM�t���Ȃ��j
	string           strbom(){ return m_strbom; };	// BOM������
};
//---------------------------------------------------------------------
// ifstream�����i�쐬�R�}���h�̂ݑΉ��j
//---------------------------------------------------------------------
class LocalIfs
{
	LocalWbCnv& wbc = LWbCnv;
	ifstream ifs;
	LocalFileType attr;
	bool flagOpen = false;
public:
	LocalIfs(){};
	LocalIfs(const string& strName){ open(strName); };
	bool open(const string& strName);
	bool getline(string& buf);
private:
	bool getlineCore(string& buf);
public:
	void close();
	bool is_open();
	int  getCodeNum();
};
//---------------------------------------------------------------------
// ofstream�����i�쐬�R�}���h�̂ݑΉ��j
//---------------------------------------------------------------------
class LocalOfs
{
	LocalWbCnv& wbc = LWbCnv;
	ofstream ofs;
	LocalFileType attr;
	bool flagOpen = false;
public:
	LocalOfs(){};
	LocalOfs(const string& strName){ open(strName); };
	bool open(const string& strName, bool append = false);
	bool append(const string& strName);
	bool write(const string& buf);
private:
	string writeRevStr(const string& buf);
public:
	void close();
	bool is_open();
	void setCodeTypeFromNum(int num);
	void setCodeType(LcParam::UtfType utf);
};
//---------------------------------------------------------------------
// UTF-8�̕����񑀍�i���K�\���܂ށj
//---------------------------------------------------------------------
class LocalStr
{
public:
	int getStrLen(const string& str);
	string getSubStr(const string& str, int st);
	string getSubStrLen(const string& str, int st, int len);
private:
	string getSubStrCommon(const string& str, int st, int len, bool validLen);
public:
	int countInStr(const string& strSrc, const string& strEach);
	bool replaceInStr(string& strSrc, const string& strEach, const string& strSub);
	int countRegExMatch(const string& strSrc, const string& strRe);
	string getRegMAtch(const string& strSrc, const string& strRe);
private:
	LocalWbCnv& wbc = LWbCnv;
};


//---------------------------------------------------------------------
// UTF-8 - ���`�� ������ϊ�����
//---------------------------------------------------------------------
class LocalWbCnv {
public:
	// �t�@�C��IO�ƕ����R�[�h�ϊ�����
	string cnvFromFileString(const string& lstr, LcParam::UtfType utype);
	string cnvToFileString(const string& ustr, LcParam::UtfType utype);
#if defined(WIDE_BYTE_SJIS)
private:
	wstring getWstrFromSjis(const string& str);
	string getSjisFromWstr(const wstring& wstr);
#endif
public:
	// WideByte - UTF-8 ������ϊ�����
	int getWlenFromUtf8(const string& str);
	int getLenToUtf8(const wstring& wstr);
	wstring getWstrFromUtf8(const string& str);
	string getUtf8FromWstr(const wstring& wstr);
	int getNeedByteFromUtf8Head(const char ch);
	int getWordByteFromUtf8(const string& str, int n);
private:
	wchar_t getWordStrFromUtf8(int& nSize, const string& str, int n);
	int getWordUtf8FromWstr(char* str, const wstring& wstr, int n);
};
