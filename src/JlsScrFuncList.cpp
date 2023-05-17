//
// ���X�g�f�[�^��{����
//

#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsDataset.hpp"


//---------------------------------------------------------------------
// ������
//---------------------------------------------------------------------
void JlsScrFuncList::setDataPointer(JlsDataset *pdata){
	this->pdata = pdata;
}


//--- ���X�g�̍��ڐ���Ԃ� ---
int JlsScrFuncList::getListStrSize(const string& strList){
	int numList = 0;
	//--- ���X�g���ڐ����擾 ---
	if ( isListStrEmpty(strList) == false ){
		//--- Comma�i�[�p���ꕶ����̊m�F ---
		int posCmmSt;
		int posCmmEd;
		bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
		bool flagNoDetect = false;
		//--- ���ԂɌ��� ---
		numList = 1;
		bool flagFirstChar = true;
		for(int i=0; i < (int)strList.size(); i++){
			//--- Comma�i�[�p���ꕶ����̊m�F ---
			if ( flagNoDetect ){		// ���ꕶ������Ԓ�
				if ( i == posCmmEd ){
					flagNoDetect = false;
				}
			}else{
				if ( flagComma ){	// ���ꕶ���񂠂�
					if ( i == posCmmSt && flagFirstChar ){
						flagNoDetect = true;
					}else if ( i > posCmmSt ){	// ���̓��ꕶ������擾
						flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, i);
					}
				}
				if ( strList[i] == ',' ){
					numList++;
					flagFirstChar = true;
				}else{
					flagFirstChar = false;
				}
			}
		}
	}
	return numList;
}
//--- ���X�gempty�m�F ---
bool JlsScrFuncList::isListStrEmpty(const string& strList){
	if ( strList == DefStrEmpty ) return true;
	return strList.empty();

}
//--- Comma�����X�g�ϐ����Ɋ܂ޏꍇ�̓��ꕶ����J�n����I���܂ł̕�����ʒu�擾 ---
bool JlsScrFuncList::getListStrCommaCheck(int& posSt, int& posEd, const string& strList, int pos){
	bool flagComma = false;
	posSt = -1;
	posEd = -1;
	auto posIn = strList.find(DefStrCommaIn, pos);
	if ( posIn != string::npos ){
		posSt = (int) posIn;
		int nMatch = 1;		// �l�X�g��
		auto posOut = strList.find(DefStrCommaOut, posSt+1);
		posIn = strList.find(DefStrCommaIn, posSt+1);
		while( nMatch > 0 ){
			bool chkIn  = false;
			bool chkOut = false;
			if ( posOut == string::npos ){
				nMatch = -1;
			}else if ( posIn == string::npos ){
				chkOut = true;
			}else if ( posIn < posOut ){
				chkIn = true;
			}else{
				chkOut = true;
			}
			if ( chkIn ){
				nMatch += 1;
				posIn = strList.find(DefStrCommaIn, posIn+1);
			}
			if ( chkOut ){
				nMatch -= 1;
				if ( nMatch > 0 ){
					posOut = strList.find(DefStrCommaOut, posOut+1);
				}
			}
		}
		if ( nMatch == 0 ){
			flagComma = true;
			posEd = (int) (posOut + DefStrCommaOut.length() - 1);
		}
	}
	return flagComma;
}
//--- ���X�g�ϐ��ɓ����v�f��������擾�iComma�΍��t���j ---
void JlsScrFuncList::getListStrBaseStore(string& strStore, const string& strRaw){
	bool need = false;
	//--- �f�[�^�Ȃ��f�[�^�̑}���͓��ꕶ����ɂ��� ---
	if ( isListStrEmpty(strRaw) ){
		need = true;
	}
	//--- Comma���܂܂�Ă�������ꕶ����ɂ��� ---
	if ( strRaw.find(",") != string::npos ){
		need = true;
	}
	//--- ���� ---
	if ( need ){
		strStore = DefStrCommaIn + strRaw + DefStrCommaOut;
	}else{
		strStore = strRaw;
	}
}
//--- ���X�g�ϐ�����v�f����������o���iComma�΍�������j ---
void JlsScrFuncList::getListStrBaseLoad(string& strRaw, const string& strStore){
	strRaw = strStore;
	//--- ���ꕶ����m�F ---
	if ( strStore.find(DefStrCommaIn) == 0 ){
		int posSt;
		int posEd;
		if ( getListStrCommaCheck(posSt, posEd, strStore, 0) ){
			int lenS = (int) DefStrCommaIn.length();
			int lenE = (int) DefStrCommaOut.length();
			strRaw = strStore.substr(lenS, posEd-lenS-lenE+1);
		}
	}
}
//--- �w�荀�ڂ̂���J�n�ʒu�ƕ����񒷂�Ԃ��B���ڐ�num>0�̂ݑΉ� ---
bool JlsScrFuncList::getListStrBasePosItem(int& posItem, int& lenItem, const string& strList, int num){
	//--- Comma�i�[�p���ꕶ����̊m�F ---
	int posCmmSt;
	int posCmmEd;
	bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
	//--- ���ԂɈʒu�m�F ---
	int nCur = 0;
	int pos = -1;
	int posNext = 0;
	while ( posNext >= 0 && nCur < num && num > 0 ){
		pos = posNext;
		//--- ���̓��ꕶ������擾 ---
		if ( pos > posCmmSt && flagComma ){
			flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, pos);
		}
		//--- ���̍��ڈʒu ---
		int posSend = pos;
		if ( pos == posCmmSt && flagComma ){
			posSend = posCmmEd;
		}
		auto posFind = strList.find(",", posSend);
		if ( posFind == string::npos ){
			posNext = -1;
		}else{
			posNext = (int) posFind + 1;
		}
		nCur ++;
	}
	//--- ���ʈʒu�̊i�[ ---
	if ( pos >= 0 && nCur == num ){
		posItem = pos;
		if ( pos <= posNext ){
			lenItem = posNext - pos - 1;
		}else{
			lenItem = (int) strList.length() - pos;
		}
		return true;
	}
	posItem = -1;
	lenItem = 0;
	return false;
}
//--- ���X�g�̎w�荀�ڈʒu��������̉��Ԗڂ���ѕ����񒷂��擾 ---
bool JlsScrFuncList::getListStrPosItem(int& posItem, int& lenItem, const string& strList, int num, bool flagIns){
	int numList = getListStrSize(strList);	// ���ڐ��擾
	//--- ���ڂ��擾 ---
	int numAbs = ( num >= 0 )? num : numList + num + 1;
	if ( flagIns && num < 0 ){		// Ins���͍ő區�ڐ����P����
		numAbs += 1;
	}
	//--- �}�����̍Ō�� ---
	if ( numAbs > 0 && (numAbs == numList + 1) && flagIns ){
		posItem = (int)strList.size();
		if ( numList == 0 ){
			posItem = 0;
		}
		lenItem = 0;
		return true;
	}else if ( numAbs > numList || numAbs == 0 ){
		return false;
	}
	//--- �ʒu���擾 ---
	return getListStrBasePosItem(posItem, lenItem, strList, numAbs);
}
//--- ���X�g�̎w�荀�ڈʒu��������̉��Ԗڂ��擾 ---
int JlsScrFuncList::getListStrPosHead(const string& strList, int num, bool flagIns){
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		return posItem;
	}
	return -1;
}
//--- ���X�g�̎w�荀�ڈʒu�ɂ��镶�����Ԃ� ---
bool JlsScrFuncList::getListStrElement(string& strItem, const string& strList, int num){
	strItem = "";
	bool flagIns = false;
	//--- ���X�g���̈ʒu�擾 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		string strStore = strList.substr(posItem, lenItem);
		getListStrBaseLoad(strItem, strStore);		// ���X�g�ۊǗp���ꕶ���񂩂畜��
		return true;
	}
	return false;
}
//--- �󃊃X�g���̕␳ ---
void JlsScrFuncList::revListStrEmpty(string& strList){
	if ( strList.empty() ){
		setListStrClear(strList);
	}
}
//--- ���X�g������ ---
void JlsScrFuncList::setListStrClear(string& strList){
	strList = DefStrEmpty;
}
//--- ���X�g���ڐ������{������ ---
void JlsScrFuncList::setListStrDim(string& strList, int nDim, string strVal){
	setListStrClear(strList);
	for(int i=0; i<nDim; i++){
		setListStrIns(strList, strVal, -1);
	}
}
//--- ���X�g�̎w�荀�ڈʒu�ɕ������}�� ---
bool JlsScrFuncList::setListStrIns(string& strList, const string& strItem, int num){
	int lenList = (int)strList.length();
	//--- �Ώۍ��ڂ̐擪�ʒu�擾 ---
	bool flagIns = true;
	int locSt   = getListStrPosHead(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	//--- �ۊǗp������쐬 ---
	string strStore;
	getListStrBaseStore(strStore, strItem);
	//--- �}������ ---
	if ( locSt == 0 ){			// �擪����
		if ( isListStrEmpty(strList) ){	// ���ڂȂ��̎�
			strList = strStore;
		}else{
			strList = strStore + "," + strList;
		}
	}else if ( locSt == lenList ){	// �Ō�
		strList = strList + "," + strStore;
	}else{
		string strTmp = strList.substr(locSt-1);
		if ( locSt == 1 ){
			strList = "," + strStore + strTmp;
		}else{
			strList = strList.substr(0, locSt-1) + "," + strStore + strTmp;
		}
	}
	return true;
}
//--- ���X�g�̎w�荀�ڈʒu�̕�������폜 ---
bool JlsScrFuncList::setListStrDel(string& strList, int num){
	int lenList = (int)strList.length();
	//--- �Ώۍ��ڂ̐擪�ʒu�ƕ��������擾 ---
	bool flagIns = false;
	//--- ���X�g���̈ʒu�擾 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) == false ){
		return false;
	}
	//--- �폜���� ---
	if ( posItem == 0 ){
		if ( lenItem >= lenList ){
			setListStrClear(strList);	// 1���ڂ̎������󕶎��ݒ�
		}else{
			strList = strList.substr(lenItem + 1);
		}
	}else if ( posItem == 1 ){
		strList = strList.substr(lenItem + 1);
	}else{
		string strTmp = "";
		if ( posItem + lenItem < lenList ){
			strTmp = strList.substr(posItem + lenItem);
		}
		strList = strList.substr(0, posItem-1) + strTmp;
	}
	return true;
}
//--- ���X�g�̎w�荀�ڈʒu�̕������u�� ---
bool JlsScrFuncList::setListStrRep(string& strList, const string& strItem, int num){
	if ( setListStrDel(strList, num) ){
		if ( setListStrIns(strList, strItem, num) ){
			return true;
		}
	}
	return false;
}
//--- ���X�g���̎w��ԍ��i�͈͎w��\�j�̂ݑI�� ---
bool JlsScrFuncList::setListStrSel(string& strList, const string& strNumMulti){
	string strNum;
	if ( pdata->cnv.getStrMultiNum(strNum, strNumMulti, 0) < 0 ){
		return false;		// �ُ�w��̊m�F
	}
	int numMax = getListStrSize(strList);	// ���ڐ��擾
	vector<string> listResult;
	for(int i=1; i<=numMax; i++){
		if ( pdata->cnv.isStrMultiNumIn(strNum, i, numMax) ){
			string strItem;
			if ( getListStrElement(strItem, strList, i) ){
				listResult.push_back(strItem);
			}
		}
	}
	//--- ���ʊi�[ ---
	setListStrClear(strList);
	for(auto i=0; i<(int)strList.size(); i++){
		setListStrIns(strList, listResult[i], -1);
	}
	return true;
}
//--- ���X�g�f�[�^�����r���X�g���̃f�[�^���폜 ---
bool JlsScrFuncList::setListStrRemove(string& strListDat, const string& strListCmp){
	//--- ���X�g�f�[�^���ڂ��擾 ---
	int numListDat = getListStrSize(strListDat);	// ���ڐ��擾
	int numListCmp = getListStrSize(strListCmp);	// ���ڐ��擾
	bool success = true;
	vector<string> listStrCmp;
	for(int i=1; i<=numListCmp; i++){
		string strItem;
		if ( getListStrElement(strItem, strListCmp, i) ){
			listStrCmp.push_back(strItem);
		}else{
			success = false;
		}
	}
	if ( !success ) return false;
	//--- ��v�f�[�^�͍폜 ---
	for(int i=numListDat; i>=1; i--){
		string strDat;
		if ( !getListStrElement(strDat, strListDat, i) ){
			success = false;
			break;
		}
		bool eq = false;
		for(int j=0; j<(int)listStrCmp.size(); j++){
			if ( listStrCmp[j] == strDat ){
				eq = true;
			}
		}
		if ( eq ){
			setListStrDel(strListDat, i);
		}
	}
	return success;
}
//--- �̈�w��Ń��X�g�̈悩���r���X�g�̈������ ---
bool JlsScrFuncList::setListStrRemoveLap(string& strListDat, const string& strListCmp){
	//--- ��r�f�[�^�𐔒l�Ƃ��ă\�[�g ---
	string strListSortCmp = strListCmp;
	if ( !setListStrSortLap(strListSortCmp, true) ){
		return false;
	}
	//--- ���l�f�[�^���擾�i��r�f�[�^�̓\�[�g��j ---
	vector<Msec> listMsecDat;
	vector<Msec> listMsecCmp;
	int numListDat = getListStrSize(strListDat);
	if ( numListDat > 0 ){
		if ( !pdata->cnv.getListValMsec(listMsecDat, strListDat) ) return false;
		if ( numListDat != (int) listMsecDat.size() ) return false;	// ���ڐ��擾
	}
	int numListCmp = getListStrSize(strListSortCmp);
	if ( numListCmp > 0 ){
		if ( !pdata->cnv.getListValMsec(listMsecCmp, strListSortCmp) ) return false;
		if ( numListCmp != (int) listMsecCmp.size() ) return false;	// ���ڐ��擾
	}
	if ( numListDat % 2 != 0 ){
		return false;
	}
	//--- �o�͗p�f�[�^���X�g���쐬 ---
	vector<string> listStrDat;
	for(int i=1; i<=numListDat; i++){
		string strItem;
		getListStrElement(strItem, strListDat, i);
		listStrDat.push_back(strItem);
	}
	//--- ��v�f�[�^�͍폜 ---
	vector<string> listResult;
	for(int i=0; i<numListDat-1; i+=2){
		Msec msecMgn = pdata->msecValExact;
		bool remain = true;
		int  msecSt = listMsecDat[i];
		int  msecEd = listMsecDat[i+1];
		for(int j=0; j<numListCmp-1; j+=2){
			if ( !remain ) continue;
			int msecCmpSt = listMsecCmp[j];
			int msecCmpEd = listMsecCmp[j+1];
			if ( msecEd <= msecCmpSt + msecMgn ){
				if ( msecSt + msecMgn <= msecCmpSt ){
					listResult.push_back(listStrDat[i]);
					listResult.push_back(listStrDat[i+1]);
				}
				remain = false;
			}
			else{
				if ( msecSt + msecMgn <= msecCmpSt ){
					int msecTmp = msecCmpSt;
//					msecTmp = pdata->cnv.getMsecAdjustFrmFromMsec(msecCmpSt, -1);
					string strT2 = pdata->cnv.getStringTimeMsecM1(msecTmp);
					listResult.push_back(listStrDat[i]);
					listResult.push_back(pdata->cnv.getStringTimeMsecM1(msecTmp));
				}
				if ( msecEd <= msecCmpEd + msecMgn ){
					remain = false;
				}else if ( msecSt + msecMgn <= msecCmpEd ){
					msecSt = msecCmpEd;
//					msecSt = pdata->cnv.getMsecAdjustFrmFromMsec(msecCmpEd, +1);
					listStrDat[i] = pdata->cnv.getStringTimeMsecM1(msecSt);
				}
			}
		}
		if ( remain ){
			listResult.push_back(listStrDat[i]);
			listResult.push_back(listStrDat[i+1]);
			remain = false;
		}
	}
	//--- ���ʊi�[ ---
	setListStrClear(strListDat);
	for(auto i=0; i<(int)listResult.size(); i++){
		setListStrIns(strListDat, listResult[i], -1);
	}
	return true;
}
//--- ���X�g�f�[�^�������Ƀ\�[�g ---
bool JlsScrFuncList::setListStrSort(string& strList, bool flagUni){
	//--- �\�[�g�p ---
	struct data_t {
		Msec ms;
		string str;
		bool operator<( const data_t& right ) const {
			return ms < right.ms;
		}
		bool operator==( const data_t& right ) const {
			return ms == right.ms;
		}
	};
	//--- ���X�g���f�[�^�� ---
	int numList = getListStrSize(strList);	// ���ڐ��擾
	if ( numList == 0 ){
		return true;
	}
	//--- ���X�g�f�[�^���擾����r�p�̒l�ɕϊ� ---
	vector<data_t>  listSort;
	for(int i=1; i<=numList; i++){
		string strItem;
		if ( getListStrElement(strItem, strList, i) ){
			Msec msecVal;
			if ( pdata->cnv.getStrValMsec(msecVal, strItem, 0) >= 0 ){
				data_t dItem;
				dItem.ms  = msecVal;
				dItem.str = strItem;
				listSort.push_back( dItem );
			}
		}
	}
	//--- �\�[�g ---
	std::sort(listSort.begin(), listSort.end());
	if ( flagUni ){		// �d���v�f�̍폜
		listSort.erase(std::unique(listSort.begin(), listSort.end()), listSort.end());
	}
	//--- ���X�g�������� ---
	bool success = false;
	setListStrClear(strList);
	int numSort = (int)listSort.size();
	if ( numSort > 0 ){
		success = true;
		for(int i=0; i < numSort; i++){
			setListStrIns(strList, listSort[i].str, -1);
		}
	}
	return success;
}

//--- ���X�g�f�[�^�i���S�ʒu�poverwrite�j�������Ƀ\�[�g ---
bool JlsScrFuncList::setListStrSortLap(string& strList, bool merge){
	//--- �\�[�g�p ---
	struct data_t2 {
		Msec ms;
		Msec ms2;
		string str;
		string str2;
		bool operator<( const data_t2& right ) const {
			return ms < right.ms;
		}
		bool operator==( const data_t2& right ) const {
			return ms == right.ms;
		}
	};
	//--- ���X�g���f�[�^�� ---
	int numList = getListStrSize(strList);	// ���ڐ��擾
	if ( numList == 0 ){
		return true;
	}
	//--- ���X�g�f�[�^���擾����r�p�̒l�ɕϊ� ---
	vector<data_t2>  listSort;
	for(int i=1; i<=numList-1; i+=2){
		string strItem1;
		string strItem2;
		if ( getListStrElement(strItem1, strList, i) &&
			 getListStrElement(strItem2, strList, i+1) ){
			Msec msecVal1;
			Msec msecVal2;
			if ( pdata->cnv.getStrValMsec(msecVal1, strItem1, 0) >= 0 &&
				 pdata->cnv.getStrValMsec(msecVal2, strItem2, 0) >= 0 ){
				if ( msecVal1 <= msecVal2 ){
					data_t2 dItem;
					dItem.ms   = msecVal1;
					dItem.ms2  = msecVal2;
					dItem.str  = strItem1;
					dItem.str2 = strItem2;
					listSort.push_back( dItem );
				}
			}
		}
	}
	//--- �\�[�g ---
	std::sort(listSort.begin(), listSort.end());
	//--- overwrite���� ---
	Msec msecMgn = ( merge )? pdata->msecValExact : 0;
	int numSort = (int)listSort.size();
	for(int i=numSort-1; i>=1; i--){
		Msec msa1 = listSort[i].ms;
		Msec msa2 = listSort[i].ms2;
		Msec msb2 = listSort[i-1].ms2;
		if ( msa1 <= msb2 + msecMgn ){		// �d�Ȃ������͌�������
			if ( msb2 < msa2 ){
				msb2 = msa2;
				listSort[i-1].ms2 = listSort[i].ms2;
				listSort[i-1].str2 = listSort[i].str2;
			}
			listSort.erase(listSort.begin() + i);	// �����폜
		}
	}
	//--- ���X�g�������� ---
	bool success = false;
	setListStrClear(strList);
	numSort = (int)listSort.size();
	if ( numSort > 0 ){
		success = true;
		for(int i=0; i < numSort; i++){
			setListStrIns(strList, listSort[i].str, -1);
			setListStrIns(strList, listSort[i].str2, -1);
		}
	}
	return success;
}
