//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsIF.hpp"
#include "JlsScript.hpp"
#include "JlsDataset.hpp"

//---------------------------------------------------------------------
// 初期設定
//---------------------------------------------------------------------
JlsIF::JlsIF(){
	//--- データ本体作成 ---
	m_funcDataset.reset(new JlsDataset);
	this->pdata = m_funcDataset->pdata;

	//--- 初期化 ---
	m_logofile.clear();
	m_scpfile.clear();
	m_cmdfile.clear();
	m_outfile.clear();
	m_outscpfile.clear();
}

JlsIF::~JlsIF() = default;


//=====================================================================
// 実行コマンド
//=====================================================================

//---------------------------------------------------------------------
// オプション読み込み
// 入力：
//  argc    : 引数合計
//  *argv[] : 引数文字列
//---------------------------------------------------------------------
void JlsIF::setArgFull(int argc, char *argv[]) {
	m_listarg.clear();
	//--- argv[1]から後を設定 ---
	for(int i=1; i<argc; i++){
		m_listarg.push_back(argv[i]);
	}
}

void JlsIF::setArgEach(char *str) {
	m_listarg.push_back(str);
}

//---------------------------------------------------------------------
// join_logo_scpスクリプト実行
// 出力：
// 返り値：
//  0 : 正常オプション読み込み(ERROPT_NONE)
//  1 : 実行しないで終了する(ERROPT_EXIT)
//  2 : 設定エラー終了(ERROPT_SETTING)
//---------------------------------------------------------------------
int JlsIF::runScript() {
	JlsScript funcScript(pdata);
	//--- オプション展開 ---
	int errnum = expandArg(funcScript, m_listarg);
	if (errnum > 0) return errnum;

	//--- check filename ---
	if (m_logofile.empty()) {
		fprintf(stderr, "warning: not found logo file(-inlogo filename)\n");
		pdata->extOpt.flagNoLogo = 1;
	}
	if (m_scpfile.empty()) {
		fprintf(stderr, "error: need -inscp filename\n");
		return ERROPT_SETTING;
	}
	if (m_cmdfile.empty()) {
		fprintf(stderr, "error: need -incmd filename\n");
		return ERROPT_SETTING;
	}
	if (m_outfile.empty()) {
		fprintf(stderr, "error: need -o filename\n");
		return ERROPT_SETTING;
	}

	//--- ファイル読み込み ---
	if (pdata->extOpt.flagNoLogo == 0) {
		readLogoframe(m_logofile);				// ロゴデータ読み込み
	}
	errnum = readScpos(m_scpfile);				// 無音シーンチェンジ読み込み

	//--- JLスクリプト実行 ---
	if (errnum == 0) {
		errnum = funcScript.startCmd(m_cmdfile);
	}

	//--- 結果出力 ---
	if (errnum == 0) {
		outputResultTrim(m_outfile);			// Trim情報出力
		outputResultDetail(m_outscpfile);		// 構成情報出力
		outputResultDiv(m_outdivfile);		// 分割情報出力
	}

	return errnum;
}



//=====================================================================
// 内部実行コマンド
//=====================================================================

//---------------------------------------------------------------------
// 引数を展開
// 返り値：
//  0 : 正常オプション読み込み(ERROPT_NONE)
//  1 : 実行しないで終了する(ERROPT_EXIT)
//  2 : 設定エラー終了(ERROPT_SETTING)
//---------------------------------------------------------------------
int JlsIF::expandArg(JlsScript &funcScript, vector <string> &listin){
	int argc = (int) listin.size();
	if (argc <= 0){
		return ERROPT_NONE;
	}
	//--- 引数読み込み ---
	int i = 0;
	while(i >= 0 && i < argc){
		int argrest = argc - i;
		const char* strv = listin[i].c_str();
		const char* str1 = nullptr;
		const char* str2 = nullptr;
		if (argrest >= 2){
			str1 = listin[i+1].c_str();
		}
		if (argrest >= 3){
			str2 = listin[i+2].c_str();
		}
		int numarg = expandArgOne(funcScript, argrest, strv, str1, str2);
		if (numarg == GETONE_EXIT){
			return ERROPT_EXIT;
		}
		else if (numarg < 0){			// GETONE_ERR
			return ERROPT_SETTING;
		}
		if (numarg > 0){
			i += numarg;
		}
		else{
			i ++;
		}
	}
	return ERROPT_NONE;
}

//---------------------------------------------------------------------
// ファイルから引数読み込み
//---------------------------------------------------------------------
int JlsIF::expandArgFromFile(JlsScript &funcScript, const string &fname){
	CnvStrTime *ptcnv = &(pdata->cnv);
	vector <string> listFromFile;
	//--- ファイルからオプション文字列取得 ---
	if (fname.empty() == false){
		string strBuf;
		string strWord;
		ifstream ifs(fname);
		if (ifs.fail()){
			cerr << "error: failed to open " << fname << "\n";
			return 2;
		}
		while( getline(ifs, strBuf) ){
			if (strBuf[0] != '#'){
				int pos = 0;
				while(pos >= 0){
					pos = ptcnv->getStrItem(strWord, strBuf, pos);
					if (pos >= 0){
						listFromFile.push_back(strWord);
					}
				}
			}
		}
	}
	return expandArg(funcScript, listFromFile);
}

//---------------------------------------------------------------------
// オプション読み込み
// 入力：
//   argrest    ：引数残り数
//   strv       ：引数コマンド
//   str1       ：引数値１
//   str2       ：引数値２
// 出力：
//   返り値  ：引数取得数(-1の時強制終了(GETONE_EXIT), -2の時取得エラー(GETONE_ERR))
//---------------------------------------------------------------------
int JlsIF::expandArgOne(JlsScript &funcScript, int argrest, const char* strv, const char* str1, const char* str2){
	if (argrest <= 0){
		return GETONE_NONE;
	}
	bool exist2 = false;
	if (argrest >= 2){
		exist2 = true;
	}
	int numarg = 0;
	if(strv[0] == '-' && strv[1] != '\0') {
		if(!_stricmp(strv, "-v")){
			pdata->extOpt.verbose = 1;
			numarg = 1;
		}
		else if (!_stricmp(strv, "-ver")){
			printf("join_logo_scp ver4.0\n");
			return GETONE_EXIT;
		}
		else if (!_stricmp(strv, "-F")){
			int erropt = expandArgFromFile(funcScript, str1);
			if (erropt == ERROPT_EXIT){
				return GETONE_EXIT;
			}
			else if (erropt == ERROPT_SETTING){
				return GETONE_ERR;
			}
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inlogo")){
			if (!exist2){
				fprintf(stderr, "-inlogo needs an argument\n");
				return GETONE_ERR;
			}
			m_logofile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-inscp")){
			if (!exist2){
				fprintf(stderr, "-inscp needs an argument\n");
				return GETONE_ERR;
			}
			m_scpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-incmd")){
			if (!exist2){
				fprintf(stderr, "-incmd needs an argument\n");
				return GETONE_ERR;
			}
			m_cmdfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-o")){
			if (!exist2){
				fprintf(stderr, "-o needs an argument\n");
				return GETONE_ERR;
			}
			m_outfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-oscp")){
			if (!exist2){
				fprintf(stderr, "-oscp needs an argument\n");
				return GETONE_ERR;
			}
			m_outscpfile = str1;
			numarg = 2;
		}
		else if (!_stricmp(strv, "-lastcut")){
			if (!exist2){
				fprintf(stderr, "-lastcut needs an argument\n");
				return GETONE_ERR;
			}
			pdata->extOpt.frmLastcut = atoi(str1);
			numarg = 2;
		}
		else if (!_stricmp(strv, "-odiv")) {
			if (!exist2) {
				fprintf(stderr, "-odiv needs an argument\n");
				return GETONE_ERR;
			}
			m_outdivfile = str1;
			numarg = 2;
		}
		else{
			bool overwrite = true;
			numarg = funcScript.setOptionsGetOne(argrest, strv, str1, str2, overwrite);
			if (numarg <= 0){
				if (numarg == 0){
					fprintf(stderr, "unknown option(%s)\n", strv);
				}
				return GETONE_ERR;
			}
			
		}
	}
	else{
		fprintf(stderr, "unknown argument(%s)\n", strv);
		return GETONE_ERR;
	}
	return numarg;
}

//---------------------------------------------------------------------
// ロゴ表示期間情報を読み込み（ファイル名fnameの内容を取得）
// 出力：
//   返り値  ：0=正常終了 2=ファイル異常
//---------------------------------------------------------------------
int JlsIF::readLogoframe(const string &fname){
	CnvStrTime *ptcnv = &(pdata->cnv);
	int pos;
	int n1, n2, n3, interlace, fade, flag_start;
	DataLogoIF dtlogo;
	string strBuf;
	string strWord;

	int set_rise = 0;
	int line = 0;
	clearRecord( dtlogo );
	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		cerr << "error: failed to open " << fname << "\n";
		return 2;
	}
	while( getline(ifs, strBuf) ){
		line ++;
		if (strBuf[0] != '#'){
			pos = 0;
			pos = ptcnv->getStrValMsecFromFrm(n1, strBuf, pos);		// get frame number
			if (pos >= 0){
				fade = 0;
				interlace = 0;
				n2 = n1;
				n3 = n1;
				pos = ptcnv->getStrItem(strWord, strBuf, pos);	// get S or E
				if (pos < 0){
					if (set_rise == 0){
						strWord = "S";
					}
					else{
						strWord = "E";
					}
				}
				if (strWord[0] == 'S'){
					flag_start = 1;
				}
				else if (strWord[0] == 'E'){
					flag_start = 0;
				}
				else{
					flag_start = -1;
				}

				pos = ptcnv->getStrValNum(fade, strBuf, pos);		// get fade
				pos = ptcnv->getStrItem(strWord, strBuf, pos);		// get interlace
				if (pos >= 0){
					if (strWord[0] == 'T'){
						interlace = 1;
					}
					else if (strWord[0] == 'B'){
						interlace = 2;
					}
				}
				pos = ptcnv->getStrValMsecFromFrm(n2, strBuf, pos);		// get frame-left
				pos = ptcnv->getStrValMsecFromFrm(n3, strBuf, pos);		// get frame-right

				if (flag_start > 0 && set_rise == 0){		// set start edge
					dtlogo.rise       = n1;
					dtlogo.rise_l     = n2;
					dtlogo.rise_r     = n3;
					dtlogo.fade_rise  = fade;
					dtlogo.intl_rise  = interlace;
					set_rise = 1;
				}
				else if (flag_start == 0 && set_rise > 0){	// set end edge
					dtlogo.fall       = n1;
					dtlogo.fall_l     = n2;
					dtlogo.fall_r     = n3;
					dtlogo.fade_fall  = fade;
					dtlogo.intl_fall  = interlace;
					set_rise = 0;
					pushRecordLogo(dtlogo);					// add data
					clearRecord( dtlogo );					// clear
				}
				else{
					cerr << "error:ignored line" << line << ":'" << strBuf << "'\n";
				}
			}
		}
	}
	if (set_rise > 0 && emptyDataLogo() == 0){					// ignore if not found end edge
		popRecordLogo();
	}

	if (emptyDataLogo() != 0){
		cerr << "warning: no logo information found in '" << fname << "'\n";
	}
	return 0;
}

//---------------------------------------------------------------------
// 無音シーンチェンジ情報を読み込み（ファイル名fnameの内容を取得）
// 出力：
//   返り値  ：0=正常終了 2=ファイル異常
//---------------------------------------------------------------------
int JlsIF::readScpos(const string &fname){
	CnvStrTime *ptcnv = &(pdata->cnv);
	int j, jmax, n, n2;
	int pos;
	int flag_insert;
	DataScpIF dtscp;
	DataScpIF dttmp;
	string strBuf;

	// 最初は0フレームとする
	clearRecord( dtscp );
	dtscp.msec = 0;
	dtscp.msbk = 0;
	pushRecordScp( dtscp );			// データ追加
	clearRecord( dtscp );				// クリア

	// 無音範囲情報初期化
	int msec_smute_s = -1;
	int msec_smute_w = -1;

	ifstream ifs(fname.c_str());
	if (ifs.fail()){
		cerr << "error: failed to open " << fname << "\n";
		return 2;
	}
	while( getline(ifs, strBuf) ){
		// 無音範囲情報
		n = (int) strBuf.find("NAME=");
		if (n >= 0){
			int frm_smute_w;
			if (ptcnv->getStrValNumHead(frm_smute_w, strBuf, n+5) >= 0){
				msec_smute_w = ptcnv->getMsecFromFrm(frm_smute_w);
			}
		}
		else{
			n = (int) strBuf.find("CHAPTER");
			if (n >= 0){
				pos = n+7;
				while(strBuf[pos] >= '0' && strBuf[pos] <= '9') pos ++;
				if (strBuf[pos] == '='){
					pos ++;
					pos = ptcnv->getStrValMsecFromFrm(msec_smute_s, strBuf, pos);
				}
			}
		}

		// シーンチェンジ情報
		n = (int) strBuf.find("SCPos:");
		if (n >= 0){
			clearRecord(dtscp);
			pos = ptcnv->getStrValMsecFromFrm(dtscp.msec, strBuf, n+6);
			if (pos >= 0){
				pos = ptcnv->getStrValMsecFromFrm(dtscp.msbk, strBuf, pos);
				if (pos < 0){			// 終了側がなければ開始側フレームから逆算
					dtscp.msbk = ptcnv->getMsecAdjustFrmFromMsec( dtscp.msec, -1 );
				}
				// シーンチェンジ変化情報
				n2 = (int) strBuf.find("＿");	// マーク検出
				if (n2 >= 0 && n2 < n){			// マークがSCPos:より前
					dtscp.still = 1;			// シーンチェンジ変化なしフラグ付加
				}

				// 無音範囲情報設定
				if (msec_smute_s >= 0 && msec_smute_w >= 0){
					dtscp.msmute_s = msec_smute_s;
					dtscp.msmute_e = ptcnv->getMsecAlignFromMsec(msec_smute_s + msec_smute_w);
					// 連続無音区間の確認および追加
					int msmute_s_prev  = ptcnv->getMsecAdjustFrmFromMsec(msec_smute_s, -1);
					j = sizeDataScp() - 1;
					while(j > 0){
						getRecordScp(dttmp, j);
						if (dttmp.msmute_e >= msmute_s_prev){
							if (dttmp.msmute_s < dtscp.msmute_s){
								dtscp.msmute_s = dttmp.msmute_s;
							}
							if (dttmp.msmute_e < dtscp.msmute_e){
								dttmp.msmute_e = dtscp.msmute_e;
								setRecordScp(dttmp, j);
							}
						}
						else{
							j = 0;
						}
						j--;
					}
				}
				msec_smute_s = -1;
				msec_smute_w = -1;

				// シーンチェンジ重複時の処理
				flag_insert = 1;				// データ追加設定
				j = sizeDataScp() - 1;
				if (j > 0 && dtscp.msbk != dtscp.msec){
					getRecordScp(dttmp, j);
					// ２領域の無音区間でシーンチェンジが重なった場合
					if (((dttmp.msbk <= dtscp.msbk) &&
						 (dttmp.msec >= dtscp.msbk)) ||
						((dttmp.msbk <= dtscp.msec) &&
						 (dttmp.msec >= dtscp.msec))){
						// 少しだけずれていた場合は後側のシーンチェンジ位置を有効にする
						if (dttmp.msbk < dtscp.msbk){
							dttmp.msbk = dtscp.msbk;
							dttmp.msec = dtscp.msec;
							setRecordScp(dttmp, j);
						}
						// 無音区間の結合
						if (dttmp.msec > dtscp.msec){
							dttmp.msec = dtscp.msec;
							setRecordScp(dttmp, j);
						}
						if (dttmp.msbk < dtscp.msbk){
							dttmp.msbk = dtscp.msbk;
							setRecordScp(dttmp, j);
						}
						flag_insert = 0;					// データ追加なし;
					}
				}
				if (flag_insert > 0){
					pushRecordScp(dtscp);
				}
			}
		}
	}
	// 最終シーンチェンジ補正
	jmax = sizeDataScp() - 1;
	getRecordScp(dttmp, jmax);
	pdata->setMsecTotalMax( dttmp.msec );
	if (jmax > 1){
		int frm_lastcut = pdata->extOpt.frmLastcut;
		dttmp.msec = ptcnv->getMsecAdjustFrmFromMsec( dttmp.msec, -1*frm_lastcut );
		dttmp.msbk = ptcnv->getMsecAdjustFrmFromMsec( dttmp.msbk, -1*frm_lastcut );
		if (dttmp.msmute_s < 0 && dttmp.msmute_e < 0){
			dttmp.msmute_s = dttmp.msec;
			dttmp.msmute_e = dttmp.msec;
		}
		int msec_totalmax = dttmp.msec;
		pdata->setMsecTotalMax( dttmp.msec );
		setRecordScp(dttmp, jmax);
		for(j=1; j<0; j++){
			getRecordScp(dttmp, j);
			if (dttmp.msec > msec_totalmax){
				dttmp.msec = msec_totalmax;
				setRecordScp(dttmp, j);
			}
			if (dttmp.msbk > msec_totalmax){
				dttmp.msbk = msec_totalmax;
				setRecordScp(dttmp, j);
			}
			if (dttmp.msmute_s > msec_totalmax){
				dttmp.msmute_s = msec_totalmax;
				setRecordScp(dttmp, j);
			}
			if (dttmp.msmute_e > msec_totalmax){
				dttmp.msmute_e = msec_totalmax;
				setRecordScp(dttmp, j);
			}
		}
	}

	return 0;
}

//---------------------------------------------------------------------
// カット結果作成および出力
//---------------------------------------------------------------------
void JlsIF::outputResultTrim(const string &outfile){
	CnvStrTime *ptcnv = &(pdata->cnv);

	//--- 結果作成 ---
	pdata->outputResultTrimGen();
	//--- 結果出力 ---
	ofstream ofs(outfile.c_str());
	if (ofs.fail()){
		cerr << "error:failed to open " << outfile << "\n";
		return;
	}
	int num_data = (int) pdata->resultTrim.size();
	for(int i=0; i<num_data-1; i+=2){
		if (i > 0){
			ofs << " ++ ";
		}
		int frm_st = ptcnv->getFrmFromMsec( pdata->resultTrim[i] );
		int frm_ed = ptcnv->getFrmFromMsec( pdata->resultTrim[i+1] );
		ofs << "Trim(" << frm_st << "," << frm_ed << ")";
	}
	ofs << endl;
}

//---------------------------------------------------------------------
// 詳細情報結果作成および出力
//---------------------------------------------------------------------
void JlsIF::outputResultDetail(const string &outscpfile){
	if (outscpfile.empty()){
		return;
	}

	//--- ファイル設定 ---
	ofstream ofs(outscpfile.c_str());
	if (ofs.fail()){
		cerr << "error:failed to open " << outscpfile << "\n";
		return;
	}
	//--- 初期化 ---
	pdata->outputResultDetailReset();

	//--- データ読み込み・出力 ---
	string strBuf;
	while( pdata->outputResultDetailGetLine(strBuf) == 0){
		ofs << strBuf << endl;
	}
}

//---------------------------------------------------------------------
// 分割結果出力
//---------------------------------------------------------------------
void JlsIF::outputResultDiv(const string &outdivfile) {
	if (outdivfile.empty()) {
		return;
	}

	//--- 結果出力 ---
	CnvStrTime *ptcnv = &(pdata->cnv);
	ofstream ofs(outdivfile.c_str());
	if (ofs.fail()) {
		cerr << "error:failed to open " << outdivfile << "\n";
		return;
	}
	int num_data = (int)pdata->divFile.size();
	for (int i = 0; i<num_data; ++i) {
		int frm = ptcnv->getFrmFromMsec(pdata->divFile[i]);
		ofs << frm << std::endl;
	}
}



//=====================================================================
// データ関数
//=====================================================================

//---------------------------------------------------------------------
// １項目初期化
//---------------------------------------------------------------------
void JlsIF::clearRecord(DataLogoIF &dt){
	dt.rise        = 0;
	dt.fall        = 0;
	dt.rise_l      = 0;
	dt.rise_r      = 0;
	dt.fall_l      = 0;
	dt.fall_r      = 0;
	dt.fade_rise   = 0;
	dt.fade_fall   = 0;
	dt.intl_rise   = 0;
	dt.intl_fall   = 0;
}

void JlsIF::clearRecord(DataScpIF &dt){
	dt.msec     = 0;
	dt.msbk     = 0;
	dt.msmute_s = -1;
	dt.msmute_e = -1;
	dt.still    = 0;
}

//---------------------------------------------------------------------
// １データセット単位挿入（最後の位置）（ロゴInterFaceデータ）
//---------------------------------------------------------------------
void JlsIF::pushRecordLogo(DataLogoIF &dtbs){
	struct DataLogoRecord dtlogo;

	pdata->clearRecordLogo(dtlogo);
	dtlogo.rise    = dtbs.rise;
	dtlogo.fall    = dtbs.fall;
	dtlogo.rise_l  = dtbs.rise_l;
	dtlogo.rise_r  = dtbs.rise_r;
	dtlogo.fall_l  = dtbs.fall_l;
	dtlogo.fall_r  = dtbs.fall_r;

	dtlogo.org_rise    = dtbs.rise;
	dtlogo.org_fall    = dtbs.fall;
	dtlogo.org_rise_l  = dtbs.rise_l;
	dtlogo.org_rise_r  = dtbs.rise_r;
	dtlogo.org_fall_l  = dtbs.fall_l;
	dtlogo.org_fall_r  = dtbs.fall_r;

	dtlogo.fade_rise = dtbs.fade_rise;
	dtlogo.fade_fall = dtbs.fade_fall;
	dtlogo.intl_rise = dtbs.intl_rise;
	dtlogo.intl_fall = dtbs.intl_fall;

	pdata->pushRecordLogo(dtlogo);				// add data
}

//---------------------------------------------------------------------
// １データセット単位挿入（最後の位置）（無音SCInterFaceデータ）
//---------------------------------------------------------------------
void JlsIF::pushRecordScp(DataScpIF &dtbs){
	struct DataScpRecord dtscp;

	pdata->clearRecordScp(dtscp);
	dtscp.msec     = dtbs.msec;
	dtscp.msbk     = dtbs.msbk;
	dtscp.msmute_s = dtbs.msmute_s;
	dtscp.msmute_e = dtbs.msmute_e;
	dtscp.still    = ( dtbs.still != 0 )? true : false;

	pdata->pushRecordScp(dtscp);				// add data
}

//---------------------------------------------------------------------
// １データセット単位削除（最後の位置）（ロゴデータ）
//---------------------------------------------------------------------
void JlsIF::popRecordLogo(){
	pdata->popRecordLogo();							// delete data
}

//---------------------------------------------------------------------
// 無音SCデータサイズ取得
//---------------------------------------------------------------------
int JlsIF::sizeDataScp(){
	return pdata->sizeDataScp();
}

//---------------------------------------------------------------------
// ロゴデータが空？
//---------------------------------------------------------------------
bool JlsIF::emptyDataLogo(){
	return pdata->emptyDataLogo();
}

//---------------------------------------------------------------------
// １データセット単位取得（無音SC InterFaceデータ）
//---------------------------------------------------------------------
void JlsIF::getRecordScp(DataScpIF &dtbs, int nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		struct DataScpRecord dtscp;

		pdata->getRecordScp(dtscp, nsc);
		dtbs.msec     = dtscp.msec;
		dtbs.msbk     = dtscp.msbk;
		dtbs.msmute_s = dtscp.msmute_s;
		dtbs.msmute_e = dtscp.msmute_e;
		dtbs.still    = dtscp.still;
	}
}

//---------------------------------------------------------------------
// １データセット単位変更（無音SC InterFaceデータ）
//---------------------------------------------------------------------
void JlsIF::setRecordScp(DataScpIF &dtbs, int nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		struct DataScpRecord dtscp;

		pdata->clearRecordScp(dtscp);
		dtscp.msec     = dtbs.msec;
		dtscp.msbk     = dtbs.msbk;
		dtscp.msmute_s = dtbs.msmute_s;
		dtscp.msmute_e = dtbs.msmute_e;
		dtscp.still    = ( dtbs.still != 0 )? true : false;
		pdata->setRecordScp(dtscp, nsc);
	}
}
