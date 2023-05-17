//
// ���X�g�f�[�^��{����
//

#pragma once

class JlsDataset;


class JlsScrFuncList
{
private:
	const string DefStrCommaIn  = "@**!(";	// ���X�g�ϐ���comma�i�[�̎��ʗp�i�擪�j
	const string DefStrCommaOut = ")!**@";	// ���X�g�ϐ���comma�i�[�̎��ʗp�i�Ō�j
	const string DefStrEmpty    = R"("")";	// �󃊃X�g

public:
	void setDataPointer(JlsDataset *pdata);
	// ���X�g���ʏ���
	int  getListStrSize(const string& strList);
	bool isListStrEmpty(const string& strList);
private:
	bool getListStrCommaCheck(int& posSt, int& posEd, const string& strList, int pos);
	void getListStrBaseStore(string& strStore, const string& strRaw);
	void getListStrBaseLoad(string& strRaw, const string& strStore);
	bool getListStrBasePosItem(int& posItem, int& lenItem, const string& strList, int num);
	bool getListStrPosItem(int& posItem, int& lenItem, const string& strList, int num, bool flagIns);
	int  getListStrPosHead(const string& strList, int num, bool flagIns);
public:
	bool getListStrElement(string& strItem, const string& strList, int num);
	void revListStrEmpty(string& strList);
	void setListStrClear(string& strList);
	void setListStrDim(string& strList, int nDim, string strVal);
	bool setListStrIns(string& strList, const string& strItem, int num);
	bool setListStrDel(string& strList, int num);
	bool setListStrRep(string& strList, const string& strItem, int num);
	bool setListStrSel(string& strList, const string& strNumMulti);
	bool setListStrRemove(string& strListDat, const string& strListCmp);
	bool setListStrRemoveLap(string& strListDat, const string& strListCmp);
	bool setListStrSort(string& strList, bool flagUn);
	bool setListStrSortLap(string& strList, bool merge);
	
private:
	JlsDataset *pdata;
};
