//
// �ϐ��̊i�[
//
// �N���X�\��
//   JlsScrReg       : ���[�J���ϐ��i�K�w�ʁj�ƃO���[�o���ϐ����ꂼ��JlsRegFile��ێ�
//     |- JlsRegFile : �ϐ��i�[
//
#pragma once

///////////////////////////////////////////////////////////////////////
//
// �ϐ��i�[�N���X
//
///////////////////////////////////////////////////////////////////////
class JlsRegFile
{
public:
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	bool popMsgError(string& msg);

private:
	int  getRegNameVal(string& strName, string& strVal, const string& strPair);

private:
	vector<string>   m_strListVar;	// �ϐ��i�[
	string           msgErr;		// �G���[���b�Z�[�W�i�[

};

///////////////////////////////////////////////////////////////////////
//
// �K�w�\���ϐ��N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScrReg
{
private:
	struct RegLayer {
		bool base;				// ��ʊK�w���������Ȃ��K�w
		JlsRegFile regfile;
	};

public:
	int  createLocal(bool flagBase);
	int  releaseLocal(bool flagBase);
	bool setLocalRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	bool setArgReg(const string& strName, const string& strVal);
	void setLocalOnly(bool flag);
	bool popMsgError(string& msg);

private:
	int  getLayerRegVar(int& numLayer, string& strVal, const string& strCandName, bool exact);
	void clearArgReg();
	void setRegFromArg();
	bool checkErrRegName(const string& strName);
	bool popErrLower(JlsRegFile& regfile);

private:
	vector<RegLayer> layerReg;		// �K�w�ʃ��[�J���ϐ�
	JlsRegFile       globalReg;		// �O���[�o���ϐ�
	vector<string>   listArg;		// Call�p�����i�[
	bool             onlyLocal = false;	// �O���[�o���ϐ���ǂݏo���Ȃ��ݒ�=true
	string           msgErr;		// �G���[���b�Z�[�W�i�[
};

