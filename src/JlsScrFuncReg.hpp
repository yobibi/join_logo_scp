//
// �ϐ��A�N�Z�X�֘A����
//

#pragma once

class JlsDataset;
class JlsCmdArg;
class JlsScrGlobal;
class JlsScrFuncList;

class JlsScrFuncReg
{
private:
	const string DefRegExpTrim  = R"(Trim\s*\(\s*(\d+)\s*,\s*(\d+)\s*\))";	// Trim�擾���K�\��
	const string DefRegExtChar = "EXTCHAR";		// �g���q�����񏈗��p�̕ϐ���
	const string DefRegExtCsub = "EXTCHSUB";	// �g���q�����񏈗��p�̕ϐ���
	const string DefRegDQuote  = "DQUOTE";		// �_�u���N�H�[�g�u���ޔ�p�̕ϐ���
	const string DefRegSQuote  = "SQUOTE";		// �V���O���N�H�[�g�u���ޔ�p�̕ϐ���
	const string DefStrRepDQ   = "__!(DQ)!__";	// �_�u���N�H�[�g�𐧌䖳�֌W�����ɒu���ޔ�
	const string DefStrRepSQ   = "__!(SQ)!__";	// �V���O���N�H�[�g�𐧌䖳�֌W�����ɒu���ޔ�

	enum class VarProcType {	// �ϐ��̕����񏈗�
		none,
		path,		// �p�X�����擾
		divext,		// �g���q�����擾
		substr,		// ����������
		exchg,		// ������u��
		blank,		// �󔒏����S��
		trim,		// �󔒏����O��
		chpath,		// �p�X�����ǉ�
		frame,		// �t���[�����擾
		match,		// ���K�\������
		count,		// �g���q�o����
		len,		// ������
	};
	struct VarProcRecord {
		VarProcType typeProc;
		bool selHead;
		bool selTail;
		bool selPath;
		bool withDelim;
		bool selRegEx;
		bool selInStr;
		bool selQuote;
		bool selBackup;
	};
	enum class ReadFileType {
		Check,
		Trim,
		List,
		String,
	};

public:
	void setDataPointer(JlsDataset *pdata, JlsScrGlobal *pglobal, JlsScrFuncList *plist);
public:
	// �N���I�v�V��������
	int  setOptionsGetOne(int argrest, const char* strv, const char* str1, const char* str2, bool overwrite);
private:
	Msec setOptionsCnvCutMrg(const char* str);
	bool setInputReg(const char *name, const char *val, bool overwrite);
	bool setInputFlags(const char *flags, bool overwrite);
public:
	// �֐������A�N�Z�X
	void setArgFuncName(const string& strName);
	bool setArgRefReg(const string& strName, const string& strVal);
	bool setArgRegByVal(const string& strName, const string& strVal);
	bool setArgRegByName(const string& strName, const string& strValName);
	bool setArgRegByBoth(const string& strName, const string& strVal, bool quote);
private:
	bool isValidAsRegName(const string& strName);
	bool setArgRegCheckName(const string& strName);
public:
	// ���W�X�^�A�N�Z�X(write)
	bool unsetJlsRegVar(const string& strName, bool flagLocal);
	bool setJlsRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarLocal(const string& strName, const string& strVal, bool overwrite);
	bool setJlsRegVarWithLocal(const string& strName, const string& strVal, bool overwrite, bool flagLocal);
	bool setJlsRegVarCountUp(const string& strName, int step, bool flagLocal);
private:
	void setJlsRegVarCouple(const string& strName, const string& strVal);
public:
	// ���W�X�^�A�N�Z�X(read)
	bool getJlsRegVarNormal(string& strVal, const string& strName);
	int  getJlsRegVarPartName(string& strVal, const string& strCandName, bool exact);
	// �iprivate�ݒ�\��j
	int  getJlsRegVar(string& strVal, const string& strCandName, bool exact);
private:
	bool checkJlsRegDivide(string& strNamePart, string& strDivPart, int& lenFullVar);
	bool divideJlsRegVar(string& strVal, const string& strDivPart);
	bool divideJlsRegVarDecode(string& strVal, const string& strCmd);
	bool divideJlsRegVarDecodeIn(VarProcRecord& var, const string& strCmd);
	bool fixJlsRegNameAtList(string& strNamePart, int& lenFullVar, bool exact);
	void replaceStrAllFind(string& strVal, const string& strFrom, const string& strTo);
public:
	void backupStrQuote(string& strVal);
	void restoreStrQuote(string& strVal);
	// �ϐ��������̕ϐ��l�ɒu��
	bool replaceBufVar(string& dstBuf, const string& srcBuf);
private:
	int  replaceRegVarInBuf(string& strVal, const string& strBuf, int pos);
public:
	// �V�X�e���ϐ��ݒ�
	void setSystemRegInit();
	void setSystemRegUpdate();
	void setSystemRegFilePath();
	void setSystemRegFileOpen();
	void setSystemRegHeadtail(int headframe, int tailframe);
	void setSystemRegNologo(bool need_check);
	void setSystemRegReadValid(bool valid);
	void setSystemRegLastexe(bool exe_command);
	bool isSystemRegLastexe();
	void setOutDirect();
	bool setSystemRegOptions(const string& strBuf, int pos, bool overwrite);
	void getSystemData(JlsCmdArg& cmdarg, const string& strIdent);
	// �R�}���h���ʂɂ��ϐ��X�V
	void updateResultRegWrite(JlsCmdArg& cmdarg);
	void setResultRegWriteSize(JlsCmdArg& cmdarg, const string& strList);
	void setResultRegPoshold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListhold(JlsCmdArg& cmdarg, Msec msecPos);
	void setResultRegListGetAt(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListIns(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListDel(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListJoin(JlsCmdArg& cmdarg);
	void setResultRegListRemove(JlsCmdArg& cmdarg);
private:
	bool setResultRegSubGetRegVal(JlsCmdArg& cmdarg, string& strListSub, bool must);
public:
	// ���X�g����R�}���h
	void setResultRegListSel(JlsCmdArg& cmdarg, string strListNum);
	void setResultRegListRep(JlsCmdArg& cmdarg, int numItem);
	void setResultRegListClear(JlsCmdArg& cmdarg);
	void setResultRegListDim(JlsCmdArg& cmdarg, int num);
	void setResultRegListSort(JlsCmdArg& cmdarg);
private:
	void sortResultRegList(JlsCmdArg& cmdarg, string& strList);
	void writeResultRegListW(JlsCmdArg& cmdarg, const string& strList);
public:
	// �����񃊃X�g���R�}���h
	string getStrRegListByCsvStr(const string& strBuf);
	void setStrRegListByCsv(JlsCmdArg& cmdarg);
	void setStrRegListBySpc(JlsCmdArg& cmdarg);
private:
	void setStrRegListCommon(JlsCmdArg& cmdarg, const string& strBuf, int readtype);
public:
	// �f�[�^�p�t�@�C���ǂݍ���
	bool readDataCheck(JlsCmdArg& cmdarg, const string& fname);
	bool readDataList(JlsCmdArg& cmdarg, const string& fname);
	bool readDataTrim(JlsCmdArg& cmdarg, const string& fname);
	bool readDataString(JlsCmdArg& cmdarg, const string& fname);
private:
	bool readDataCommon(JlsCmdArg& cmdarg, const string& fname, ReadFileType rtype);
	bool readDataCommonIns(JlsCmdArg& cmdarg, int& nCur, const string& strLine, int nMax, ReadFileType rtype);
	void readDataStrAdd(JlsCmdArg& cmdarg, const string& sdata);
	bool readDataStrTrimGet(string& strLocSt, string& strLocEd, string& strTarget);
	bool readDataStrTrimDetect(const string& strLine);
	bool readDataFileLine(string& strLine, LocalIfs& ifs);
	bool readDataFileTrim(string& strCmd, LocalIfs& ifs);
public:
	// �O���[�o���̈�쐬�̃t�@�C���ǂݍ���
	bool readGlobalOpen(JlsCmdArg& cmdarg, const string& fname);
	void readGlobalClose(JlsCmdArg& cmdarg);
	bool readGlobalLine(JlsCmdArg& cmdarg);
private:
	bool getRegArg(JlsCmdArg& cmdarg, string& strArg);
	bool setRegOutSingle(JlsCmdArg& cmdarg, const string& strVal, bool certain);
public:
	// ���ݒ�ǂݍ���
	bool readDataEnvGet(JlsCmdArg& cmdarg, const string& strEnvName);
private:
	void outputMesErr(const std::string& mes);

private:
	JlsDataset *pdata;
	JlsScrGlobal *pGlobalState;
	JlsScrFuncList *pFuncList;
};
