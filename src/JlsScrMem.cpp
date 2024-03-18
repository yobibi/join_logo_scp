//
// 遅延実行コマンドの保管
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScrMem.hpp"

///////////////////////////////////////////////////////////////////////
//
// 遅延実行保管用の識別子保持クラス
//
///////////////////////////////////////////////////////////////////////
JlsScrMemArg::JlsScrMemArg(){
	clearArg();
}

//---------------------------------------------------------------------
// 引数設定を初期化
//---------------------------------------------------------------------
void JlsScrMemArg::clearArg(){
	m_flagDummy  = false;
	m_flagSpecial = false;
	m_listName.clear();
	m_listName.push_back("");		// リスト1つ目は初期設定
}

//---------------------------------------------------------------------
// 設定：識別子文字列による保管用識別子情報
//  ToBase で保管する識別子の名前を設定
//  ToExt  でLazy_S/A/E ３種類それぞれの状態を保管するための識別子を設定
//---------------------------------------------------------------------
void JlsScrMemArg::setNameByStr(const string strName){
	//--- 初期化 ---
	clearArg();
	bool base = true;		// 単体の文字列設定
	string strRegName = strName;
	//--- 特殊識別子確認 ---
	MemSpecialID idName;
	if ( findSpecialName(idName, strName) ){	// 特殊識別子
		m_flagSpecial = true;
		switch( idName ){
			case MemSpecialID::DUMMY:
				m_flagDummy = true;
				base = false;
				break;
			case MemSpecialID::LAZY_FULL:
				setMapNameToExt("");			// Lazy本体にはLazy用拡張識別子をつけない
				base = false;
				break;
			case MemSpecialID::NoData:
				base = false;
				break;
			default:
				strRegName = getStringSpecialID(idName);	// 特殊は対応する識別子にする
				break;
		}
	}
	else{
		setMapNameToExt(strRegName);				// 通常識別子に保管するLazy用拡張識別子設定
	}
	//--- 文字列設定 ---
	if ( base ){
		setMapNameToBase(strRegName);
	}
}
//---------------------------------------------------------------------
// 設定：Lazy種類による保管用識別子情報
//---------------------------------------------------------------------
void JlsScrMemArg::setNameByLazy(LazyType typeLazy){
	//--- 初期化 ---
	clearArg();
	string strName = "";
	//--- 設定 ---
	switch( typeLazy ){
		case LazyType::LazyS:
			strName = getStringSpecialID(MemSpecialID::LAZY_S);
			break;
		case LazyType::LazyA:
			strName = getStringSpecialID(MemSpecialID::LAZY_A);
			break;
		case LazyType::LazyE:
			strName = getStringSpecialID(MemSpecialID::LAZY_E);
			break;
		case LazyType::FULL:
			strName = getStringSpecialID(MemSpecialID::LAZY_FULL);
			break;
		default:
			break;
	}
	if ( strName.empty() == false ){
		setNameByStr(strName);
	}
}
//---------------------------------------------------------------------
// 取得：保管用識別子の存在
//---------------------------------------------------------------------
bool JlsScrMemArg::isExistBaseName(){
	return !( m_listName[0].empty() );
}
//---------------------------------------------------------------------
// 取得：Lazy用保管用識別子の存在
//---------------------------------------------------------------------
bool JlsScrMemArg::isExistExtName(){
	int sizeName = (int)m_listName.size();
	if ( sizeName > 1 && sizeName <= SIZE_MEM_SPECIAL_ID ){
		return true;
	}
	return false;
}
//---------------------------------------------------------------------
// 取得：DUMMY文字列判定
//---------------------------------------------------------------------
bool JlsScrMemArg::isNameDummy(){
	return m_flagDummy;
}
//---------------------------------------------------------------------
// 取得：特殊文字列判定
//---------------------------------------------------------------------
bool JlsScrMemArg::isNameSpecial(){
	return m_flagSpecial;
}
//---------------------------------------------------------------------
// 取得：保管用識別子文字列
//---------------------------------------------------------------------
void JlsScrMemArg::getBaseName(string& strName){
	strName = m_listName[0];
}
//---------------------------------------------------------------------
// 取得：Lazy用を含めた保管用識別子文字列リスト
//---------------------------------------------------------------------
void JlsScrMemArg::getNameList(vector <string>& listName){
	listName = m_listName;
}

//---------------------------------------------------------------------
// 内部処理：通常の保管用識別子を設定
//---------------------------------------------------------------------
void JlsScrMemArg::setMapNameToBase(const string strName){
	m_listName[0] = strName;
}
//---------------------------------------------------------------------
// 内部処理：Lazy用の拡張保管識別子セットを設定
//---------------------------------------------------------------------
void JlsScrMemArg::setMapNameToExt(const string strName){
	//--- 識別子strNameに対応したLazy用保管文字列を設定（FULL指定時に残すLazyの種類分） ---
	for(int i=0; i < SIZE_MEM_SPECIAL_ID; i++){
		MemSpecialID id = (MemSpecialID) i;
		switch( id ){
			case MemSpecialID::LAZY_S:
			case MemSpecialID::LAZY_A:
			case MemSpecialID::LAZY_E:
				{
					string str_var = getStringSpecialID(id);	// Lazy用保管識別子
					if ( !strName.empty() ){				// 通常識別子のLazy用保管識別子
						str_var = strName + ScrMemStrLazy + str_var;
					}
					m_listName.push_back(str_var);
				}
				break;
			default:
				break;
		}
	}
}
//---------------------------------------------------------------------
// 内部処理：特殊識別子の確認・取得
//---------------------------------------------------------------------
bool JlsScrMemArg::findSpecialName(MemSpecialID& idName, const string& strName){
	bool result = false;
	for(int i=0; i < SIZE_MEM_SPECIAL_ID; i++){
		if ( strName == MemSpecialData[i].str ){
			idName = MemSpecialData[i].id;		// 識別子名(strName)に対応する番号を取得
			result = true;
			break;
		}
	}
	return result;
}
//---------------------------------------------------------------------
// 内部処理：特殊識別子に対応する文字列を取得
//---------------------------------------------------------------------
string JlsScrMemArg::getStringSpecialID(MemSpecialID idName){
	int num = static_cast<int>(idName);
	if ( num >= 0 && num < SIZE_MEM_SPECIAL_ID ){
		int n2 = static_cast<int>(MemSpecialData[num].id);
		return MemSpecialData[n2].str;
	}
	return "";
}


///////////////////////////////////////////////////////////////////////
//
// スクリプトデータ保管実行本体クラス
//
///////////////////////////////////////////////////////////////////////
JlsScrMem::JlsScrMem(){
	m_orderHold = orderInitial;
}

//---------------------------------------------------------------------
// Lazy保管されているか確認
//---------------------------------------------------------------------
bool JlsScrMem::isLazyExist(LazyType typeLazy){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- 存在確認 ---
	vector <string> slist;
	marg.getNameList(slist);
	bool exist = false;
	for(int i=0; i < (int)slist.size(); i++){
		if ( memIsExist(slist[i]) ){
			exist = true;
		}
	}
	return exist;
}

//=====================================================================
// コマンド実行
// 返り値   ：実行有無
//=====================================================================

//---------------------------------------------------------------------
// 格納時の実行順位
//---------------------------------------------------------------------
//--- 格納時の実行順位を設定 ---
void JlsScrMem::setOrderForPush(int order){
	m_orderHold = order;
}
//--- 格納時の実行順位を標準値で設定 ---
void JlsScrMem::resetOrderForPush(){
	m_orderHold = orderInitial;
}
//--- 実行順位の標準値を読み出し ---
int JlsScrMem::getOrderForPush(){
	return m_orderHold;
}
//---------------------------------------------------------------------
// 引数処理
//---------------------------------------------------------------------
//--- 引数設定 ---
bool JlsScrMem::setDefArg(vector<string>& argDef){
	string strName = argDef[0];
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 特殊文字列処理 ---
	if ( marg.isNameSpecial() ){
		if ( argDef.size() > 1 ) return false;		// 引数はダメ
		return true;
	}
	//--- 通常変数処理 ---
	string strNameBase;
	marg.getBaseName(strNameBase);
	if ( strNameBase.empty() ){		// 通常変数名として設定できないものはダメ
		return false;
	}
	if ( memIsNameExistArg(strNameBase) ){
		if ( m_mapArg[strName].size() >= 2 ){	// 引数存在時はエラーにする
			return false;
		}
		return ( argDef == m_mapArg[strName] );		// 既存時は同じ時true
	}
	m_mapArg[strNameBase] = argDef;
	return true;
}
//--- 引数取得 ---
bool JlsScrMem::getDefArg(vector<string>& argDef, const string& strName){
	argDef.clear();
	argDef.push_back(strName);
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 特殊文字列処理 ---
	if ( marg.isNameSpecial() ){
		return true;
	}
	//--- 通常変数処理 ---
	string strNameBase;
	marg.getBaseName(strNameBase);
	if ( strNameBase.empty() ){
		return false;
	}
	if ( memIsNameExistArg(strNameBase) ){
		argDef = m_mapArg[strNameBase];
		return true;
	}
	return false;	// なければ失敗
}
//---------------------------------------------------------------------
// MemSetによる未使用フラグを設定
//---------------------------------------------------------------------
void JlsScrMem::setUnusedFlag(const string& strName){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 未使用に追加 ---
	setUnused(marg);
}
//---------------------------------------------------------------------
// １行文字列を格納（識別子で格納先を指定）
//---------------------------------------------------------------------
bool JlsScrMem::pushStrByName(const string& strName, const string& strBuf){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 実行 ---
	return exeCmdPushStr(marg, strBuf, m_orderHold);
}
//---------------------------------------------------------------------
// １行文字列を格納（Lazy種類で格納先を指定）
//---------------------------------------------------------------------
bool JlsScrMem::pushStrByLazy(LazyType typeLazy, const string& strBuf){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- 実行 ---
	return exeCmdPushStr(marg, strBuf, m_orderHold);
}
//---------------------------------------------------------------------
// 保管文字列リストを取得（識別子で格納元を指定）
//---------------------------------------------------------------------
bool JlsScrMem::getListByName(queue <string>& queStr, const string& strName){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 未使用状態を解除 ---
	clearUnused(marg);
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// キューに追加
	flags.move = false;		// 元データは残す
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// 保管文字列リストを取り出し（識別子で格納元を指定）
//---------------------------------------------------------------------
bool JlsScrMem::popListByName(queue <string>& queStr, const string& strName){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 未使用状態を解除 ---
	clearUnused(marg);
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// キューに追加
	flags.move = true;		// 元データを消す
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// 保管文字列リストを取得（Lazy種類で格納元を指定）
//---------------------------------------------------------------------
bool JlsScrMem::getListByLazy(queue <string>& queStr, LazyType typeLazy){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- 未使用状態を解除 ---
	clearUnused(marg);
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// キューに追加
	flags.move = false;		// 元データは残す
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// 保管文字列リストを取り出し（Lazy種類で格納元を指定）
//---------------------------------------------------------------------
bool JlsScrMem::popListByLazy(queue <string>& queStr, LazyType typeLazy){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByLazy(typeLazy);
	//--- 未使用状態を解除 ---
	clearUnused(marg);
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// キューに追加
	flags.move = true;		// 元データを消す
	return exeCmdGetList(queStr, marg, flags);
}
//---------------------------------------------------------------------
// 保管文字列領域を消去
//---------------------------------------------------------------------
bool JlsScrMem::eraseMemByName(const string& strName){
	//--- 識別子設定 ---
	JlsScrMemArg marg;
	marg.setNameByStr(strName);
	//--- 未使用状態を解除 ---
	clearUnused(marg);
	//--- 実行 ---
	return exeCmdEraseMem(marg);
}
//---------------------------------------------------------------------
// 保管文字列領域を複写
//---------------------------------------------------------------------
bool JlsScrMem::copyMemByName(const string& strSrc, const string& strDst){
	//--- 識別子設定 ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- 未使用状態は継続 ---
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = false;		// 記憶領域に新規
	flags.move = false;		// 元データは残す
	return exeCmdCopyMem(sarg, darg, flags);
}
//---------------------------------------------------------------------
// 保管文字列領域を移動
//---------------------------------------------------------------------
bool JlsScrMem::moveMemByName(const string& strSrc, const string& strDst){
	//--- 識別子設定 ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- 未使用状態を変更 ---
	clearUnused(sarg);
	setUnused(darg);
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = false;		// 記憶領域に新規
	flags.move = true;		// 元データを消す
	return exeCmdCopyMem(sarg, darg, flags);
}
//---------------------------------------------------------------------
// 保管文字列領域を追加
//---------------------------------------------------------------------
bool JlsScrMem::appendMemByName(const string& strSrc, const string& strDst){
	//--- 識別子設定 ---
	JlsScrMemArg sarg;
	JlsScrMemArg darg;
	sarg.setNameByStr(strSrc);
	darg.setNameByStr(strDst);
	//--- 未使用状態は継続 ---
	//--- 実行 ---
	CopyFlagRecord flags = {};
	flags.add  = true;		// 記憶領域に追加
	flags.move = false;		// 元データは残す
	return exeCmdCopyMem(sarg, darg, flags);
}

//=====================================================================
// 共通の引数からコマンド実行
// 返り値   ：実行有無
//=====================================================================

//---------------------------------------------------------------------
// １行文字列を格納
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdPushStr(JlsScrMemArg& argDst, const string& strBuf, int order){
	//--- 識別子確認 ---
	bool success = false;
	if ( argDst.isExistBaseName() ){
		//--- 書き込み ---
		string str_name;
		argDst.getBaseName(str_name);
		success = memPushStr(str_name, strBuf, order);
	}
	else{
		success = argDst.isNameDummy();		// ダミー時は成功扱い
	}
	return success;
}
//---------------------------------------------------------------------
// 保管文字列リストを取得・取り出し
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdGetList(queue <string>& queStr, JlsScrMemArg& argSrc, CopyFlagRecord flags){
	bool success = false;
	//--- 存在する場合取得 ---
	if ( argSrc.isExistBaseName() ){
		string strName;
		argSrc.getBaseName(strName);
		success = memGetList(queStr, strName, flags);
	}else{
		//--- 出力クリア ---
		if ( !flags.add ){
			queue<string>().swap(queStr);
		}
		success = argSrc.isNameDummy();		// ダミー時は成功扱い
	}
	//--- 保管されたLazy用文字列をLazy用本体へ ---
	if ( argSrc.isExistExtName() ){
		//--- Lazy用識別子情報 ---
		JlsScrMemArg argDst;
		argDst.setNameByLazy(LazyType::FULL);	// Lazy用本体
		//--- コピー実施 ---
		flags.add = true;			// Lazy本体の記録は残したまま追加
		bool s2 = exeCmdCopyMem(argSrc, argDst, flags);
		if ( s2 ){					// Lazy用識別子に保管が存在してもtrueを返す
			success = true;
		}
	}
	return success;
}
//---------------------------------------------------------------------
// 保管文字列領域を消去
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdEraseMem(JlsScrMemArg& argDst){
	bool success = false;
	vector <string> listName;
	argDst.getNameList(listName);
	for(int i=0; i < (int)listName.size(); i++){
		success |= memErase(listName[i]);
	}
	if ( argDst.isNameDummy() ){		// ダミー時は成功扱い
		success = true;
	}
	return success;
}
//---------------------------------------------------------------------
// 保管文字列領域を複写・移動
//---------------------------------------------------------------------
bool JlsScrMem::exeCmdCopyMem(JlsScrMemArg& argSrc, JlsScrMemArg& argDst, CopyFlagRecord flags){
	vector <string> slist;
	vector <string> dlist;
	argSrc.getNameList(slist);
	argDst.getNameList(dlist);
	int smax = (int) slist.size();
	int dmax = (int) dlist.size();

	bool success  = false;
	int loopmax = ( smax >= dmax )? smax : dmax;
	for(int i=0; i<loopmax; i++){
		bool si = false;
		//--- 項目存在確認 ---
		bool existSrc = ( i < smax )? true : false;
		bool existDst = ( i < dmax )? true : false;
		if ( existSrc ){
			existSrc = ! slist[i].empty();
		}
		if ( existDst ){
			existDst = ! dlist[i].empty();
		}
		//--- 項目が両方存在する場合のみコピーする ---
		if ( existSrc && existDst ){
			si = memCopy(slist[i], dlist[i], flags);
		}else{
			si = existSrc & argDst.isNameDummy();	// 入力存在で複写先がダミーは成功扱い
			//--- Src側削除 ---
			if ( existSrc  && flags.move ){
				memErase(slist[i]);
			}
			//--- Dst側削除 ---
			if ( existDst  && !flags.add ){
				memErase(dlist[i]);
			}
		}
		success |= si;
	}
	return success;
}

//=====================================================================
// 記憶領域の直接操作
// 返り値   ：実行有無
//=====================================================================

//---------------------------------------------------------------------
// 記憶領域に文字列１行を追加格納
//---------------------------------------------------------------------
bool JlsScrMem::memPushStr(const string& strName, const string& strBuf, int order){
	if (m_mapVar.size() >= SIZE_MEMVARNUM_MAX){	// 念のため入れすぎ確認
		return false;
	}
	if ( strName.empty() ){
		return false;
	}
	addQueueLine(m_mapVar[strName], strBuf, order);
	return true;
}
//---------------------------------------------------------------------
// 記憶領域から文字列全体を読み出し
//---------------------------------------------------------------------
bool JlsScrMem::memGetList(queue <string>& queStr, const string& strName, CopyFlagRecord flags){
	if ( memIsExist(strName) == false ){		// 格納されてなかった場合
		queue <MemDataRecord> q;
		setQueueStr(queStr, q, flags);				// 初期化
		return false;
	}
	setQueueStr(queStr, m_mapVar[strName], flags);
	return true;
}
//---------------------------------------------------------------------
// 記憶領域消去
//---------------------------------------------------------------------
bool JlsScrMem::memErase(const string& strName){
	if ( memIsNameExist(strName) == false ) return false;	// 識別子自体がなかった場合
	m_mapVar.erase(strName);
	m_mapArg.erase(strName);	// 引数設定
	return true;
}
//---------------------------------------------------------------------
// 記憶領域を別の記憶領域へコピー
//---------------------------------------------------------------------
bool JlsScrMem::memCopy(const string& strSrc, const string& strDst, CopyFlagRecord flags){
	if ( memIsExist(strSrc) == false ){			// 格納されてなかった場合
		if ( flags.add == false ){
			memErase(strDst);
		}
		return false;
	}
	setQueueFull(m_mapVar[strDst], m_mapVar[strSrc], flags);
	m_mapArg[strDst] = m_mapArg[strSrc];	// 引数設定
	return true;
}
//---------------------------------------------------------------------
// 記憶領域確認（既に存在していたらtrueを返す）
//---------------------------------------------------------------------
//--- データ無しも含む ---
bool JlsScrMem::memIsExist(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	bool flag = ( m_mapVar.find(strName) != m_mapVar.end() )? true : false;
	if ( flag ){
		if ( m_mapVar[strName].empty() ){
			flag = false;	// データが存在しなければfalse
		}
	}
	return flag;
}
//--- 識別子自体の存在確認 ---
bool JlsScrMem::memIsNameExist(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	return ( m_mapVar.find(strName) != m_mapVar.end() );
}
//--- 引数の存在 ---
bool JlsScrMem::memIsNameExistArg(const string& strName){
	if ( strName.empty() ){
		return false;
	}
	return ( m_mapArg.find(strName) != m_mapArg.end() );
}
//---------------------------------------------------------------------
// queueに１行追加（orderによる挿入位置確認も行う）
//---------------------------------------------------------------------
void JlsScrMem::addQueueLine(queue <MemDataRecord>& queDst, const string& strBuf, int order){
	MemDataRecord din = { order, strBuf };
	//--- 最後に追加の判定 ---
	bool flagTail = false;
	if ( queDst.empty() ){
		flagTail = true;
	}else if ( queDst.back().order <= din.order ){
		flagTail = true;
	}
	//--- 追加処理 ---
	bool remain = true;
	if ( flagTail ){	// 最後に追加
		queDst.push(din);
	}else{
		queue <MemDataRecord> q;
		q.swap(queDst);
		while( q.empty() == false ){
			MemDataRecord dtmp = q.front();
			q.pop();
			if ( remain && (din.order < dtmp.order) ){
				queDst.push(din);
				remain = false;
			}
			queDst.push(dtmp);
		}
		if ( remain ){
			queDst.push(din);
		}
	}
}
//---------------------------------------------------------------------
// queueに別のqueueを格納
//---------------------------------------------------------------------
void JlsScrMem::setQueueStr(queue <string>& queDstStr, queue <MemDataRecord>& queSrc, CopyFlagRecord flags){
	if ( flags.add == false ){
		queue<string>().swap(queDstStr);		// 初期化
	}
	queue <MemDataRecord> q = queSrc;
	while( q.empty() == false ){
		queDstStr.push( q.front().buffer );
		q.pop();
	}
	if ( flags.move ){
		queue<MemDataRecord>().swap(queSrc);		// 初期化
	}
}
//---------------------------------------------------------------------
// queueに別のqueueを格納
//---------------------------------------------------------------------
void JlsScrMem::setQueueFull(queue <MemDataRecord>& queDst, queue <MemDataRecord>& queSrc, CopyFlagRecord flags){
	if ( flags.add ){
		queue <MemDataRecord> q = queSrc;
		while( q.empty() == false ){
			queDst.push( q.front() );
			q.pop();
		}
	}else{
		queDst = queSrc;
	}
	if ( flags.move ){
		queue<MemDataRecord>().swap(queSrc);		// 初期化
	}
}
//---------------------------------------------------------------------
// アクセスチェック処理
//---------------------------------------------------------------------
void JlsScrMem::setUnused(JlsScrMemArg& marg){
	if ( marg.isExistBaseName() ){
		string str;
		marg.getBaseName(str);
		m_mapUnused[str] = true;
	}
}
void JlsScrMem::clearUnused(JlsScrMemArg& marg){
	if ( marg.isExistBaseName() ){
		string str;
		marg.getBaseName(str);
		if ( m_mapUnused.count(str) != 0 ){
			m_mapUnused.erase(str);
		}
	}
}
bool JlsScrMem::getUnusedStr(string& strBuf){
	strBuf.clear();
	for(auto itr = m_mapUnused.begin(); itr != m_mapUnused.end(); itr++){
		strBuf += itr->first;
		strBuf += " ";
	}
	if ( strBuf.empty() ) return false;
	strBuf = "warning: unused MemSet: " + strBuf + "\n";
	return true;
}
//---------------------------------------------------------------------
// すべての保管内容取り出し（デバッグ用）
//---------------------------------------------------------------------
void JlsScrMem::getMapForDebug(string& strBuf){
	for( auto itr = m_mapVar.begin(); itr != m_mapVar.end(); ++itr ){
		strBuf += "[" + itr->first + "]" + "\n";
		queue <MemDataRecord> q = itr->second;
		while( q.empty() == false ){
			strBuf += q.front().buffer + "\n";
			q.pop();
		}
		strBuf += "\n";
	}
}
