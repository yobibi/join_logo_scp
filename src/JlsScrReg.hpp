//
// �ϐ��̊i�[
//
// �N���X�\��
//   JlsScrReg       : ���[�J���ϐ��i�K�w�ʁj�ƃO���[�o���ϐ����ꂼ��JlsRegFile��ێ�
//     |- JlsRegFile : �ϐ��i�[
//
#pragma once

#include "JlsScrFuncList.hpp"

///////////////////////////////////////////////////////////////////////
//
// �ϐ��i�[�N���X
//
///////////////////////////////////////////////////////////////////////
class JlsRegFile
{
public:
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	bool unsetRegVar(const string& strName);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	void setIgnoreCase(bool valid);
	void setFlagAsRef(const string& strName);
	bool isRegNameRef(const string& strName);
	bool popMsgError(string& msg);

private:
	bool isSameInLen(const string& s1, const string& s2, int nLen);
	int  getRegNameVal(string& strName, string& strVal, const string& strPair);

private:
	vector<string>   m_strListVar;	// �ϐ��i�[
	string           msgErr;		// �G���[���b�Z�[�W�i�[
	bool             m_ignoreCase;	// �啶�����������
	unordered_map<string, bool>  m_flagListRef;		// �Q�Ɠn���ϐ�
};

///////////////////////////////////////////////////////////////////////
//
// �K�w�\���ϐ��N���X
//
///////////////////////////////////////////////////////////////////////
class JlsScrReg
{
private:
	enum class RegOwner {		// �K�w�쐬��
		Any,					// �i�폜���j�S������
		Call,					// Call
		Func,					// Function
		One,					// �ʏ�R�}���h
	};
	struct RegLayer {
		RegOwner owner;			// �K�w�쐬��
		bool base;				// ��ʊK�w���������Ȃ��K�w
		JlsRegFile regfile;
	};
	class RegDivList {			// �ϐ����𖼑O�ƃ��X�g�v�f�ɕ���
		public:
			string nameBase;		// �ϐ���
			vector<int>  listElem;	// �v�f�ԍ�
			int    nMatch;			// �ϐ���
		public:
			RegDivList(){};
			RegDivList(const string& str){ set(str); };
			void set(const string& str){	// �ݒ�
				listElem.clear();
				ref(str);
			};
			void ref(const string& str){	// �Q�Ɠn���X�V
				bool flagDim = false;
				auto ns = str.find("[");
				if ( ns != string::npos ){
					nameBase = str.substr(0, ns);
					vector<int> listTmp;
					bool cont = true;
					while( cont ){
						cont = false;
						auto ne = str.find("]", ns);
						if ( ne != string::npos && ns+1 < ne ){
							flagDim = true;
							listTmp.push_back( atoi(str.substr(ns+1, ne-ns-1).c_str()) );
							nMatch = (int)ne + 1;
							if ( str[ne+1] == '[' ){	// �������z��
								ns = ne+1;
								cont = true;
							}
						}
					}
					for(int i=(int)listTmp.size()-1; i>=0; i--){
						listElem.push_back( listTmp[i] );	// ��Ԑ[���v�f��擪��
					}
				}
				if ( !flagDim ){	// ���X�g�v�f�ł͂Ȃ��ʏ�̍X�V
					nameBase = str;
					nMatch = (int)nameBase.length();
				}
			};
	};
	class RegSearch {
		public:
			string strName;			// �ϐ���
			string strVal;			// �ϐ��l
			bool   exact;			// true=�ϐ����͑S������v  false=�ϐ����͐擪���畔����v
			int    numLayer;		// �K�w�i-1=���o�Ȃ�  0=�O���[�o���ϐ�  1-=���[�J���ϐ��K�w�j
			bool   stopRef;			// �Q�Ɠn���͑ΏۊO
			bool   onlyOneLayer;	// ������1�K�w�̂�
			bool   flagRef;			// �Q�Ɠn���ϐ�=true
			int    numMatch;		// �}�b�`�����ϐ����̒���
			RegDivList regOrg;		// �v�f���������ϐ����i���̕ϐ��j
			RegDivList regSel;		// �v�f���������ϐ����i�I����ϐ��j
		public:
			RegSearch(){};
			RegSearch(const string& str){ set(str); };
			void set(const string& str){		// �����ݒ�
				regOrg.set(str);
				strName = regOrg.nameBase;
				exact = true;
				numLayer = -1;
				stopRef = false;
				onlyOneLayer = false;
				flagRef  = false;
				numMatch = 0;
			}
			void decide(){		// �������ʂ��i�[
				// �ϐ����𕔕���v�Ŏ擾�Ȃ�[]�͊܂߂���v�����̂�
				if ( 0 < numMatch && numMatch < (int)regOrg.nameBase.length() ){
					regOrg.set( regOrg.nameBase.substr(0, numMatch) );
				}
				regSel = regOrg;
			}
			bool updateRef(const string& str){	// �Q�Ɛ��V�����ϐ����ɂ���
				regSel.ref(str);
				strName = regSel.nameBase;
				numLayer -= 1;
				exact = true;		// �Q�Ƃ͐��m�ȕϐ���
				return ( numLayer>=0 );
			}
	};

public:
	JlsScrReg();
	// �K�w����
	int  createLocalCall();
	int  createLocalFunc();
	int  createLocalOne();
	int  releaseLocalAny();
	int  releaseLocalCall();
	int  releaseLocalFunc();
	int  releaseLocalOne();
	int  getLocalLayer();
private:
	int  createLocalCommon(RegOwner owner);
	int  releaseLocalCommon(RegOwner owner);
public:
	// �ϐ��A�N�Z�X
	bool unsetRegVar(const string& strName, bool flagLocal);
	bool setLocalRegVar(const string& strName, const string& strVal, bool overwrite);
	bool setRegVar(const string& strName, const string& strVal, bool overwrite);
	int  getRegVar(string& strVal, const string& strCandName, bool exact);
	// �����ݒ�
	bool setArgReg(const string& strName, const string& strVal);
	bool setArgRefReg(const string& strName, const string& strVal);
	void setArgFuncName(const string& strName);
	// ���̑�����
	void setLocalOnly(bool flag);
	void setIgnoreCase(bool valid);
	void setGlobalLock(const string& strName, bool flag);
	bool isGlobalLocked(const string& strName);
	bool checkErrRegName(const string& strName, bool silent = false);
	bool popMsgError(string& msg);

private:
	// �ϐ����K�w�w��ŏ�������
	bool unsetRegCore(const string& strName, int numLayer);
	bool setRegCore(const string& strName, const string& strVal, bool overwrite, int numLayer);
	bool setRegCoreAsRef(const string& strName, const string& strVal, int numLayer);
	// �ϐ����������ēǂݏo��
	bool findRegForUnset(int& numLayer, const string& strName, bool flagLocal);
	bool findRegForWrite(string& strName, string& strVal, bool& overwrite, int& numLayer);
	int  findRegForRead(const string& strName, string& strVal, bool exact);
	bool findRegListForWrite(RegDivList& regName, string& strVal, bool& overwrite, const string& strRead);
	bool findRegListForRead(RegDivList& regName, string& strVal);
	bool findRegData(RegSearch& data);
	bool findRegDataFromLayer(RegSearch& data);
	bool isRegNameRef(const string& strName, int numLayer);
	// �����̐ݒ�
	void clearArgReg();
	void setRegFromArg();
	void setRegFromArgSub(vector<string>& listArg, bool ref);
	// �G���[����
	bool popErrLower(JlsRegFile& regfile);
	// ��������
	string makeDummyReg(const string& str){ return "[]"+str; };
	string backDummyReg(const string& str){ return str.substr(2); };

private:
	vector<RegLayer> layerReg;		// �K�w�ʃ��[�J���ϐ�
	JlsRegFile       globalReg;		// �O���[�o���ϐ�
	vector<string>   listValArg;	// Call�p�����i�[
	vector<string>   listRefArg;	// �Q�Ɠn�������i�[
	string           nameFuncReg;	// ���W�X�^���Ƃ��Ďg�p����鎟�̊֐���
	bool             onlyLocal;		// �O���[�o���ϐ���ǂݏo���Ȃ��ݒ�=true
	bool             ignoreCase;	// �啶���������̖���
	string           msgErr;		// �G���[���b�Z�[�W�i�[
	unordered_map <string, bool> m_mapGlobalLock;

	JlsScrFuncList   funcList;		// ���X�g����
};

