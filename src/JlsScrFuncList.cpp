//
// リストデータ基本処理
//

//#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrFuncList.hpp"
#include "JlsDataset.hpp"


//---------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------
void JlsScrFuncList::setDataPointer(JlsDataset *pdata){
	this->pdata = pdata;
}


//--- リストの項目数を返す ---
int JlsScrFuncList::getListStrSize(const string& strList){
	int numList = 0;
	//--- リスト項目数を取得 ---
	if ( isListStrEmpty(strList) == false ){
		//--- Comma格納用特殊文字列の確認 ---
		int posCmmSt;
		int posCmmEd;
		bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
		bool flagNoDetect = false;
		//--- 順番に検索 ---
		numList = 1;
		bool flagFirstChar = true;
		for(int i=0; i < (int)strList.size(); i++){
			//--- Comma格納用特殊文字列の確認 ---
			if ( flagNoDetect ){		// 特殊文字列期間中
				if ( i == posCmmEd ){
					flagNoDetect = false;
				}
			}else{
				if ( flagComma ){	// 特殊文字列あり
					if ( i == posCmmSt && flagFirstChar ){
						flagNoDetect = true;
					}else if ( i > posCmmSt ){	// 次の特殊文字列を取得
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
//--- リストempty確認 ---
bool JlsScrFuncList::isListStrEmpty(const string& strList){
	if ( strList == DefStrEmpty ) return true;
	return strList.empty();

}
//--- Commaがリスト変数内に含む場合の特殊文字列開始から終了までの文字列位置取得 ---
bool JlsScrFuncList::getListStrCommaCheck(int& posSt, int& posEd, const string& strList, int pos){
	bool flagComma = false;
	posSt = -1;
	posEd = -1;
	auto posIn = strList.find(DefStrCommaIn, pos);
	if ( posIn != string::npos ){
		posSt = (int) posIn;
		int nMatch = 1;		// ネスト数
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
//--- リスト変数に入れる要素文字列を取得（Comma対策を付加） ---
void JlsScrFuncList::getListStrBaseStore(string& strStore, const string& strRaw){
	bool need = false;
	//--- データなしデータの挿入は特殊文字列にする ---
	if ( isListStrEmpty(strRaw) ){
		need = true;
	}
	//--- Commaが含まれていたら特殊文字列にする ---
	if ( strRaw.find(",") != string::npos ){
		need = true;
	}
	//--- 処理 ---
	if ( need ){
		strStore = DefStrCommaIn + strRaw + DefStrCommaOut;
	}else{
		strStore = strRaw;
	}
}
//--- リスト変数から要素文字列を取り出し（Comma対策を解除） ---
void JlsScrFuncList::getListStrBaseLoad(string& strRaw, const string& strStore){
	strRaw = strStore;
	//--- 特殊文字列確認 ---
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
//--- 指定項目のある開始位置と文字列長を返す。項目数num>0のみ対応 ---
bool JlsScrFuncList::getListStrBasePosItem(int& posItem, int& lenItem, const string& strList, int num){
	//--- Comma格納用特殊文字列の確認 ---
	int posCmmSt;
	int posCmmEd;
	bool flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, 0);
	//--- 順番に位置確認 ---
	int nCur = 0;
	int pos = -1;
	int posNext = 0;
	while ( posNext >= 0 && nCur < num && num > 0 ){
		pos = posNext;
		//--- 次の特殊文字列を取得 ---
		if ( pos > posCmmSt && flagComma ){
			flagComma = getListStrCommaCheck(posCmmSt, posCmmEd, strList, pos);
		}
		//--- 次の項目位置 ---
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
	//--- 結果位置の格納 ---
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
//--- リストの指定項目位置が文字列の何番目および文字列長を取得 ---
bool JlsScrFuncList::getListStrPosItem(int& posItem, int& lenItem, const string& strList, int num, bool flagIns){
	int numList = getListStrSize(strList);	// 項目数取得
	//--- 項目を取得 ---
	int numAbs = ( num >= 0 )? num : numList + num + 1;
	if ( flagIns && num < 0 ){		// Ins時は最大項目数が１多い
		numAbs += 1;
	}
	//--- 挿入時の最後尾 ---
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
	//--- 位置を取得 ---
	return getListStrBasePosItem(posItem, lenItem, strList, numAbs);
}
//--- リストの指定項目位置が文字列の何番目か取得 ---
int JlsScrFuncList::getListStrPosHead(const string& strList, int num, bool flagIns){
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		return posItem;
	}
	return -1;
}
//--- リストの指定項目位置にある文字列を返す ---
bool JlsScrFuncList::getListStrElement(string& strItem, const string& strList, int num){
	strItem = "";
	bool flagIns = false;
	//--- リスト内の位置取得 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) ){
		string strStore = strList.substr(posItem, lenItem);
		getListStrBaseLoad(strItem, strStore);		// リスト保管用特殊文字列から復元
		return true;
	}
	return false;
}
//--- 空リスト時の補正 ---
void JlsScrFuncList::revListStrEmpty(string& strList){
	if ( strList.empty() ){
		setListStrClear(strList);
	}
}
//--- リスト初期化 ---
void JlsScrFuncList::setListStrClear(string& strList){
	strList = DefStrEmpty;
}
//--- リスト項目数生成＋初期化 ---
void JlsScrFuncList::setListStrDim(string& strList, int nDim, string strVal){
	setListStrClear(strList);
	for(int i=0; i<nDim; i++){
		setListStrIns(strList, strVal, -1);
	}
}
//--- リストの指定項目位置に文字列を挿入 ---
bool JlsScrFuncList::setListStrIns(string& strList, const string& strItem, int num){
	int lenList = (int)strList.length();
	//--- 対象項目の先頭位置取得 ---
	bool flagIns = true;
	int locSt   = getListStrPosHead(strList, num, flagIns);
	if ( locSt < 0 ){
		return false;
	}
	//--- 保管用文字列作成 ---
	string strStore;
	getListStrBaseStore(strStore, strItem);
	//--- 挿入処理 ---
	if ( locSt == 0 ){			// 先頭項目
		if ( isListStrEmpty(strList) ){	// 項目なしの時
			strList = strStore;
		}else{
			strList = strStore + "," + strList;
		}
	}else if ( locSt == lenList ){	// 最後
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
//--- リストの指定項目位置の文字列を削除 ---
bool JlsScrFuncList::setListStrDel(string& strList, int num){
	int lenList = (int)strList.length();
	//--- 対象項目の先頭位置と文字数を取得 ---
	bool flagIns = false;
	//--- リスト内の位置取得 ---
	int posItem;
	int lenItem;
	if ( getListStrPosItem(posItem, lenItem, strList, num, flagIns) == false ){
		return false;
	}
	//--- 削除処理 ---
	if ( posItem == 0 ){
		if ( lenItem >= lenList ){
			setListStrClear(strList);	// 1項目の時だけ空文字設定
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
//--- リストの指定項目位置の文字列を置換 ---
bool JlsScrFuncList::setListStrRep(string& strList, const string& strItem, int num){
	if ( setListStrDel(strList, num) ){
		if ( setListStrIns(strList, strItem, num) ){
			return true;
		}
	}
	return false;
}
//--- リスト内の指定番号（範囲指定可能）のみ選択 ---
bool JlsScrFuncList::setListStrSel(string& strList, const string& strNumMulti){
	string strNum;
	if ( pdata->cnv.getStrMultiNum(strNum, strNumMulti, 0) < 0 ){
		return false;		// 異常指定の確認
	}
	int numMax = getListStrSize(strList);	// 項目数取得
	vector<string> listResult;
	for(int i=1; i<=numMax; i++){
		if ( pdata->cnv.isStrMultiNumIn(strNum, i, numMax) ){
			string strItem;
			if ( getListStrElement(strItem, strList, i) ){
				listResult.push_back(strItem);
			}
		}
	}
	//--- 結果格納 ---
	setListStrClear(strList);
	for(auto i=0; i<(int)strList.size(); i++){
		setListStrIns(strList, listResult[i], -1);
	}
	return true;
}
//--- リストデータから比較リスト内のデータを削除 ---
bool JlsScrFuncList::setListStrRemove(string& strListDat, const string& strListCmp){
	//--- リストデータ項目を取得 ---
	int numListDat = getListStrSize(strListDat);	// 項目数取得
	int numListCmp = getListStrSize(strListCmp);	// 項目数取得
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
	//--- 一致データは削除 ---
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
//--- 領域指定でリスト領域から比較リスト領域を除去 ---
bool JlsScrFuncList::setListStrRemoveLap(string& strListDat, const string& strListCmp){
	//--- 比較データを数値としてソート ---
	string strListSortCmp = strListCmp;
	if ( !setListStrSortLap(strListSortCmp, true) ){
		return false;
	}
	//--- 数値データを取得（比較データはソート後） ---
	vector<Msec> listMsecDat;
	vector<Msec> listMsecCmp;
	int numListDat = getListStrSize(strListDat);
	if ( numListDat > 0 ){
		if ( !pdata->cnv.getListValMsec(listMsecDat, strListDat) ) return false;
		if ( numListDat != (int) listMsecDat.size() ) return false;	// 項目数取得
	}
	int numListCmp = getListStrSize(strListSortCmp);
	if ( numListCmp > 0 ){
		if ( !pdata->cnv.getListValMsec(listMsecCmp, strListSortCmp) ) return false;
		if ( numListCmp != (int) listMsecCmp.size() ) return false;	// 項目数取得
	}
	if ( numListDat % 2 != 0 ){
		return false;
	}
	//--- 出力用データリストを作成 ---
	vector<string> listStrDat;
	for(int i=1; i<=numListDat; i++){
		string strItem;
		getListStrElement(strItem, strListDat, i);
		listStrDat.push_back(strItem);
	}
	//--- 一致データは削除 ---
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
	//--- 結果格納 ---
	setListStrClear(strListDat);
	for(auto i=0; i<(int)listResult.size(); i++){
		setListStrIns(strListDat, listResult[i], -1);
	}
	return true;
}
//--- リストデータを昇順にソート ---
bool JlsScrFuncList::setListStrSort(string& strList, bool flagUni){
	//--- ソート用 ---
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
	//--- リスト内データ数 ---
	int numList = getListStrSize(strList);	// 項目数取得
	if ( numList == 0 ){
		return true;
	}
	//--- リストデータを取得し比較用の値に変換 ---
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
	//--- ソート ---
	std::sort(listSort.begin(), listSort.end());
	if ( flagUni ){		// 重複要素の削除
		listSort.erase(std::unique(listSort.begin(), listSort.end()), listSort.end());
	}
	//--- リスト書き換え ---
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

//--- リストデータ（ロゴ位置用overwrite）を昇順にソート ---
bool JlsScrFuncList::setListStrSortLap(string& strList, bool merge){
	//--- ソート用 ---
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
	//--- リスト内データ数 ---
	int numList = getListStrSize(strList);	// 項目数取得
	if ( numList == 0 ){
		return true;
	}
	//--- リストデータを取得し比較用の値に変換 ---
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
	//--- ソート ---
	std::sort(listSort.begin(), listSort.end());
	//--- overwrite結合 ---
	Msec msecMgn = ( merge )? pdata->msecValExact : 0;
	int numSort = (int)listSort.size();
	for(int i=numSort-1; i>=1; i--){
		Msec msa1 = listSort[i].ms;
		Msec msa2 = listSort[i].ms2;
		Msec msb2 = listSort[i-1].ms2;
		if ( msa1 <= msb2 + msecMgn ){		// 重なった時は結合する
			if ( msb2 < msa2 ){
				msb2 = msa2;
				listSort[i-1].ms2 = listSort[i].ms2;
				listSort[i-1].str2 = listSort[i].str2;
			}
			listSort.erase(listSort.begin() + i);	// 結合削除
		}
	}
	//--- リスト書き換え ---
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
