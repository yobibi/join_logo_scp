//
// join_logo_scp Autoコマンド処理
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsAutoScript.hpp"
#include "JlsAutoReform.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"


///////////////////////////////////////////////////////////////////////
//
// JLスクリプトAuto系パラメータ保持
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// パラメータ設定
//---------------------------------------------------------------------
void JlsAutoArg::setParam(JlsCmdArg &cmdarg, CmdAutoType cmdtype){
	clearAll();
	m_cmdtype = cmdtype;
	switch(cmdtype){
		case CmdAutoType::CutTR :
			setParamCutTR(cmdarg);
			break;
		case CmdAutoType::CutEC :
			setParamCutEC(cmdarg);
			break;
		case CmdAutoType::AddTR :
		case CmdAutoType::AddSP :
		case CmdAutoType::AddEC :
			setParamAdd(cmdarg);
			break;
		case CmdAutoType::Edge :
			setParamEdge(cmdarg);
			break;
		case CmdAutoType::AtUP :
		case CmdAutoType::AtBorder :
		case CmdAutoType::AtIClear :
		case CmdAutoType::Ins :
		case CmdAutoType::Del :
			setParamInsDel(cmdarg);
			break;
		default :
			break;
	}
}


//---------------------------------------------------------------------
// パラメータ取得
//---------------------------------------------------------------------
int JlsAutoArg::getParam(ParamAuto type){
	int num = static_cast<int>(type);
	if (num >= 0 && num < SIZE_PARAM_AUTO){
		if (m_enable_prm[num] > 0){
			return m_val_prm[num];
		}
	}
	cerr << "unexpected error JlsAutoArg::GetParam cmdtype=" << static_cast<int>(m_cmdtype);
	cerr << " type=" << num << endl;
	return 0;
}


//---------------------------------------------------------------------
// パラメータ初期化
//---------------------------------------------------------------------
void JlsAutoArg::clearAll(){
	for(int i=0; i<SIZE_PARAM_AUTO; i++){
		m_enable_prm[i] = 0;
		m_val_prm[i]    = 0;
	}
}

//---------------------------------------------------------------------
// モード別のパラメータ設定
//---------------------------------------------------------------------
void JlsAutoArg::setVal(ParamAuto type, int val){
	int num = static_cast<int>(type);
	if (num >= 0 && num < SIZE_PARAM_AUTO){
		m_enable_prm[num] = 1;
		m_val_prm[num]    = val;
	}
}

void JlsAutoArg::setParamCutTR(JlsCmdArg &cmdarg){
	//--- パラメータ取得 ---
	int autop_code     = cmdarg.getOpt(OptType::AutopCode);
	int tmp_trscope    = cmdarg.getOpt(OptType::AutopTrScope);
	int tmp_trsumprd   = cmdarg.getOpt(OptType::AutopTrSumPrd);
	int prm_v_limit    = cmdarg.getOpt(OptType::AutopLimit);
	int prm_v_tr1stprd = cmdarg.getOpt(OptType::AutopTr1stPrd);
	int prm_c_from     = autop_code % 10;
	int prm_c_cutst    = (autop_code / 10) % 10;
	int prm_c_lgpre    = (autop_code / 100) % 10;
	int prm_c_noedge   = 1;
	//--- 実行判断 ---
	int prm_c_exe      = (prm_c_from != 0)? 1 : 0;
	//--- デフォルト値付き設定 ---
	int prm_v_trscope  = (tmp_trscope  == 0)? 30 : tmp_trscope;
	int prm_v_trsumprd = (tmp_trsumprd == 0)?  3 : tmp_trsumprd;

	setVal(ParamAuto::v_limit    , prm_v_limit    );
	setVal(ParamAuto::v_tr1stprd , prm_v_tr1stprd );
	setVal(ParamAuto::c_from     , prm_c_from     );
	setVal(ParamAuto::c_cutst    , prm_c_cutst    );
	setVal(ParamAuto::c_lgpre    , prm_c_lgpre    );
	setVal(ParamAuto::c_exe      , prm_c_exe      );
	setVal(ParamAuto::v_trscope  , prm_v_trscope  );
	setVal(ParamAuto::v_trsumprd , prm_v_trsumprd );
	setVal(ParamAuto::c_noedge   , prm_c_noedge   );
}


void JlsAutoArg::setParamCutEC(JlsCmdArg &cmdarg){
	//--- パラメータ取得 ---
	int autop_code     = cmdarg.getOpt(OptType::AutopCode);
	int tmp_period   = cmdarg.getOpt(OptType::AutopPeriod);
	int tmp_maxprd   = cmdarg.getOpt(OptType::AutopMaxPrd);
	int prm_v_limit  = cmdarg.getOpt(OptType::AutopLimit);
	int prm_c_sel    = cmdarg.getOpt(OptType::AutopCode) % 10;
	int prm_c_cutla  = ((((autop_code / 10) % 10) & 0x1) != 0)? 1 : 0;
	int prm_c_cutlp  = ((((autop_code / 10) % 10) & 0x2) != 0)? 1 : 0;
	int prm_c_cut30  = 0;
	int prm_c_cutsp  = ((((autop_code / 100) % 10) & 0x1) != 0)? 1 : 0;
	int prm_c_noedge   = 1;
	//--- デフォルト値付き設定 ---
	int prm_v_period = (tmp_period == 0)?   5 : tmp_period;
	int prm_v_maxprd = (tmp_maxprd == 0)?  13 : tmp_maxprd;

	setVal(ParamAuto::v_limit  , prm_v_limit );
	setVal(ParamAuto::c_sel    , prm_c_sel );
	setVal(ParamAuto::c_cutla  , prm_c_cutla );
	setVal(ParamAuto::c_cutlp  , prm_c_cutlp );
	setVal(ParamAuto::c_cut30  , prm_c_cut30 );
	setVal(ParamAuto::c_cutsp  , prm_c_cutsp );
	setVal(ParamAuto::v_period , prm_v_period );
	setVal(ParamAuto::v_maxprd , prm_v_maxprd );
	setVal(ParamAuto::c_noedge , prm_c_noedge );
}


void JlsAutoArg::setParamAdd(JlsCmdArg &cmdarg){
	int default_c_wmin, default_c_wmax;
	int default_scope, default_period, default_search;
	int enable_w15, enable_in1;

	//--- コマンドによる違い部分 ---
	if (m_cmdtype == CmdAutoType::AddSP){	// 番組提供
		//--- デフォルト値 ---
		default_c_wmin = 6;					// 最小期間秒数
		default_c_wmax = 13;				// 最大期間秒数
		default_scope  = 90;				// 検索範囲秒数
		default_period = 5;					// 設定期間秒数
		default_search = 1;					// 検索範囲設定
		//--- フラグ設定 ---
		enable_w15     = 1;					// 15秒の検索
		enable_in1     = 1;					// 予告に挿入
	}
	else if (m_cmdtype == CmdAutoType::AddEC){	// エンドカード
		//--- デフォルト値 ---
		default_c_wmin = 1;					// 最小期間秒数
		default_c_wmax = 13;				// 最大期間秒数
		default_scope  = 90;				// 検索範囲秒数
		default_period = 5;					// 設定期間秒数
		default_search = 1;					// 検索範囲設定
		//--- フラグ設定 ---
		enable_w15     = 0;					// 15秒の検索
		enable_in1     = 0;					// 予告に挿入
	}
	else{
		//--- デフォルト値 ---
		default_c_wmin = 1;					// 最小期間秒数
		default_c_wmax = 13;				// 最大期間秒数
		default_scope  = 90;				// 検索範囲秒数
		default_period = 5;					// 設定期間秒数
		default_search = 1;					// 検索範囲設定
		//--- フラグ設定 ---
		enable_w15     = 0;					// 15秒の検索
		enable_in1     = 0;					// 予告に挿入
	}

	//--- パラメータ取得 ---
	int autop_code     = cmdarg.getOpt(OptType::AutopCode);
	int tmp_c_w        = autop_code % 10;				// 検索秒数決定用中間値
	int tmp_c_sea      = (autop_code / 10) % 10;		// 検索範囲
	int tmp_c_lg       = (autop_code / 100) % 10;		// ロゴ状態検索
	int tmp_c_p        = (autop_code / 1000) % 10;
	int tmp_c_lim      = (autop_code / 10000) % 10;
	int tmp_c_unit     = (autop_code / 100000) % 10;
	int tmp_period     = cmdarg.getOpt(OptType::AutopPeriod);
	int tmp_maxprd     = cmdarg.getOpt(OptType::AutopMaxPrd);
	int tmp_scope      = cmdarg.getOpt(OptType::AutopScope);
	int prm_v_scopen   = cmdarg.getOpt(OptType::AutopScopeN);
	int prm_v_limit    = cmdarg.getOpt(OptType::AutopLimit);
	int prm_v_secnext  = cmdarg.getOpt(OptType::AutopSecNext);
	int prm_v_secprev  = cmdarg.getOpt(OptType::AutopSecPrev);
	int prm_v_trsumprd = cmdarg.getOpt(OptType::AutopTrSumPrd);
	int prm_c_noedge   = 1;
	bool is_scope       = cmdarg.isSetOpt(OptType::AutopScope);
	bool is_period      = cmdarg.isSetOpt(OptType::AutopPeriod);
	bool is_maxprd      = cmdarg.isSetOpt(OptType::AutopMaxPrd);

	//--- 実行判断 ---
	int prm_c_exe = (tmp_c_w != 0)? 1 : 0;
	//--- デフォルト値付き設定 ---
	int prm_v_scope    = ( !is_scope     )? default_scope  : tmp_scope;
	int prm_v_period   = ( !is_period    )? default_period : tmp_period;
	int prm_v_maxprd   = ( !is_maxprd    )? default_c_wmax : tmp_maxprd;
	int prm_c_search   = (tmp_c_sea  == 0)? default_search : tmp_c_sea;
	//--- ロゴ制約 ---
	int prm_c_lgy     = ((tmp_c_lg & 0x3) != 1)? 1 : 0;	// ロゴ付き検索
	int prm_c_lgn     = ((tmp_c_lg & 0x3) != 2)? 1 : 0;	// ロゴなし検索
	int prm_c_lgbn    = ((tmp_c_lg & 0x3) == 3)? 1 : 0;	// 両隣を含めロゴなし検索
	int prm_c_cutskip = ((tmp_c_lg & 0x4) != 0)? 1 : 0;	// 予告カット以降も有効
	//--- 位置制限 ---
	int prm_c_lgprev = (tmp_c_p == 1 || tmp_c_p == 3 || (enable_in1 > 0 && tmp_c_p >= 4))? 0 : 1;	// 前側を対象外
	int prm_c_lgpost = (tmp_c_p == 2 || tmp_c_p == 3 || (enable_in1 > 0 && tmp_c_p >= 4))? 0 : 1;	// 後側を対象外
	int prm_c_lgintr = (tmp_c_p == 3)? 1 : 0;	// 間を残す
	int prm_c_lgsp   = (tmp_c_p == 4 && enable_in1 == 0)? 1 : 0;	// 番組提供・エンドカードが直後にある場合のみ対象
	int prm_c_limloc    = ((tmp_c_lim  & 0x1) > 0)? 1 : 0;
	int prm_c_limtrsum  = ((tmp_c_lim  & 0x2) > 0)? 1 : 0;
	int prm_c_unitcmoff = ((tmp_c_unit & 0x1) > 0)? 1 : 0;
	int prm_c_unitcmon  = ((tmp_c_unit & 0x2) > 0)? 1 : 0;
	int prm_c_wdefmin   = default_c_wmin;
	int prm_c_wdefmax   = default_c_wmax;

	setVal(ParamAuto::v_scopen    , prm_v_scopen );
	setVal(ParamAuto::v_limit     , prm_v_limit );
	setVal(ParamAuto::v_secnext   , prm_v_secnext );
	setVal(ParamAuto::v_secprev   , prm_v_secprev );
	setVal(ParamAuto::v_trsumprd  , prm_v_trsumprd );
	setVal(ParamAuto::c_exe       , prm_c_exe );
	setVal(ParamAuto::v_scope     , prm_v_scope );
	setVal(ParamAuto::v_period    , prm_v_period );
	setVal(ParamAuto::v_maxprd    , prm_v_maxprd );
	setVal(ParamAuto::c_search    , prm_c_search );
	setVal(ParamAuto::c_lgy       , prm_c_lgy );
	setVal(ParamAuto::c_lgn       , prm_c_lgn );
	setVal(ParamAuto::c_lgbn      , prm_c_lgbn );
	setVal(ParamAuto::c_cutskip   , prm_c_cutskip );
	setVal(ParamAuto::c_lgprev    , prm_c_lgprev );
	setVal(ParamAuto::c_lgpost    , prm_c_lgpost );
	setVal(ParamAuto::c_lgintr    , prm_c_lgintr );
	setVal(ParamAuto::c_lgsp      , prm_c_lgsp );
	setVal(ParamAuto::c_limloc    , prm_c_limloc );
	setVal(ParamAuto::c_limtrsum  , prm_c_limtrsum );
	setVal(ParamAuto::c_unitcmoff , prm_c_unitcmoff );
	setVal(ParamAuto::c_unitcmon  , prm_c_unitcmon );
	setVal(ParamAuto::c_wdefmin   , prm_c_wdefmin );
	setVal(ParamAuto::c_wdefmax   , prm_c_wdefmax );
	setVal(ParamAuto::c_noedge    , prm_c_noedge );

	//--- 15秒特殊設定（trailerでカットされた所も検索に入れる） ---
	int prm_c_w15 = 0;
	if (enable_w15 > 0){
		prm_c_w15  = ((tmp_c_w == 6) ||
					  ((tmp_c_w == 3) && (prm_v_period % 15 == 0)))? 1 : 0;
	}
	//--- 予告部分最初に番組提供設定、本編前のエンドカード等無効化 ---
	int prm_c_in1     = 0;
	int prm_c_chklast = 1;
	if (enable_in1 > 0){
		prm_c_in1     = (tmp_c_p >= 4)? tmp_c_p - 3 : 0;
		prm_c_chklast = 0;
	}
	//--- 検索下限秒数 ---
	int prm_c_wmin = default_c_wmin;		// 標準設定
	if (tmp_c_w == 2 || tmp_c_w == 4 || tmp_c_w == 5){
		prm_c_wmin = prm_v_period;
	}
	else if (tmp_c_w == 6 && enable_w15){	// 15秒限定
		prm_c_wmin = 15;
	}
	//--- 検索上限秒数 ---
	int prm_c_wmax = default_c_wmax;		// 標準設定
	if (tmp_c_w == 2 || tmp_c_w == 3){
		prm_c_wmax = prm_v_period;
	}
	else if (tmp_c_w == 5){
		prm_c_wmax = prm_v_maxprd;
	}
	else if (tmp_c_w == 6 && enable_w15){	// 15秒限定
		prm_c_wmax = 15;
	}

	setVal(ParamAuto::c_w15     , prm_c_w15 );
	setVal(ParamAuto::c_in1     , prm_c_in1 );
	setVal(ParamAuto::c_chklast , prm_c_chklast );
	setVal(ParamAuto::c_wmin    , prm_c_wmin );
	setVal(ParamAuto::c_wmax    , prm_c_wmax );
}

void JlsAutoArg::setParamEdge(JlsCmdArg &cmdarg){
	//--- パラメータ取得 ---
	int autop_code   = cmdarg.getOpt(OptType::AutopCode);
	int tmp_scope    = cmdarg.getOpt(OptType::AutopScope);
	int tmp_period   = cmdarg.getOpt(OptType::AutopPeriod);
	int tmp_maxprd   = cmdarg.getOpt(OptType::AutopMaxPrd);
	int tmp_c_w      = autop_code % 10;					// 検索秒数決定用中間値
	int tmp_c_sea    = (autop_code / 10) % 10;			// 検索範囲
	int prm_c_cmpart = ((((autop_code / 100) % 10) & 0x1) == 1)? 1 : 0;
	int prm_c_add    = ((((autop_code / 100) % 10) & 0x2) != 0)? 1 : 0;
	int prm_c_allcom = ((((autop_code / 1000) % 10) & 0x1) == 1)? 1 : 0;
	int prm_c_noedge = ((((autop_code / 1000) % 10) & 0x2) != 0)? 0 : 1;
	//--- 実行判断 ---
	int prm_c_exe = (tmp_c_w != 0)? 1 : 0;
	//--- デフォルト値付き設定 ---
	int prm_v_scope  = (tmp_scope  == 0)? 90 : tmp_scope;
	int prm_v_period = (tmp_period == 0 && cmdarg.isSetOpt(OptType::AutopPeriod) == false)?  5  : tmp_period;
	int prm_v_maxprd = (tmp_maxprd == 0)?  10 : tmp_maxprd;
	int prm_c_search = (tmp_c_sea  == 0)?  1  : tmp_c_sea;
	//--- 検索下限秒数 ---
	int prm_c_wmin = 3;
	if (tmp_c_w == 2 || tmp_c_w == 4 || tmp_c_w == 5){
		prm_c_wmin = prm_v_period;
	}
	//--- 検索上限秒数 ---
	int prm_c_wmax = 10;
	if (tmp_c_w == 2 || tmp_c_w == 3){
		prm_c_wmax = prm_v_period;
	}
	else if (tmp_c_w == 5){
		prm_c_wmax = prm_v_maxprd;
	}

	setVal(ParamAuto::c_cmpart , prm_c_cmpart );
	setVal(ParamAuto::c_add    , prm_c_add    );
	setVal(ParamAuto::c_allcom , prm_c_allcom );
	setVal(ParamAuto::c_noedge , prm_c_noedge );
	setVal(ParamAuto::c_exe    , prm_c_exe    );
	setVal(ParamAuto::v_scope  , prm_v_scope  );
	setVal(ParamAuto::v_period , prm_v_period );
	setVal(ParamAuto::v_maxprd , prm_v_maxprd );
	setVal(ParamAuto::c_search , prm_c_search );
	setVal(ParamAuto::c_wmin   , prm_c_wmin   );
	setVal(ParamAuto::c_wmax   , prm_c_wmax   );
}

void JlsAutoArg::setParamInsDel(JlsCmdArg &cmdarg){
	//--- パラメータ取得 ---
	int autop_code   = cmdarg.getOpt(OptType::AutopCode);
	if ( !cmdarg.isSetOpt(OptType::AutopCode) ){
		autop_code   = 1;								// -code未設定時の初期値
	}
	int prm_c_unit   = cmdarg.getOpt(OptType::FlagUnit);
	int prm_v_info   = cmdarg.getOpt(OptType::AutopTrInfo);
	int tmp_c_w      = autop_code % 10;					// 中間値
	//--- 実行判断 ---
	int prm_c_exe      = (tmp_c_w != 0)? 1 : 0;
	int prm_c_restruct = (tmp_c_w == 2)? 1 : 0;
	int prm_c_noedge   = 1;

	setVal(ParamAuto::c_exe      , prm_c_exe      );
	setVal(ParamAuto::c_restruct , prm_c_restruct );
	setVal(ParamAuto::c_noedge   , prm_c_noedge   );
	setVal(ParamAuto::c_unit     , prm_c_unit     );
	setVal(ParamAuto::v_info     , prm_v_info     );
}



///////////////////////////////////////////////////////////////////////
//
// JLスクリプトAuto系実行クラス
//
///////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
// 初期設定
//---------------------------------------------------------------------
JlsAutoScript::JlsAutoScript(JlsDataset *pdata){
	//--- 関数ポインタ作成 ---
	this->pdata = pdata;
}

JlsAutoScript::~JlsAutoScript() = default;


//---------------------------------------------------------------------
// Auto系コマンド実行
//---------------------------------------------------------------------
bool JlsAutoScript::startCmd(JlsCmdSet &cmdset, bool setup_only){
	checkFirstAct(cmdset.arg);
	if (setup_only == false){
		return exeCmdMain(cmdset);
	}
	return true;
}



//=====================================================================
// Autoコマンド共通処理
//=====================================================================

//---------------------------------------------------------------------
// Auto系コマンド初回動作設定
//---------------------------------------------------------------------
void JlsAutoScript::checkFirstAct(JlsCmdArg &cmdarg){
	//--- 初回のみ実行 ---
	if ( pdata->isAutoModeInitial() ){
		//--- 推測構成を作成 ---
		pdata->setFlagAutoMode(true);				// Auto系を有効
		JlsAutoReform func_reform(pdata);
		func_reform.mkReformAll(cmdarg);			// 基本構成推測
	}
}

//---------------------------------------------------------------------
// 各Auto系コマンド実行
//---------------------------------------------------------------------
bool JlsAutoScript::exeCmdMain(JlsCmdSet &cmdset){
	//--- パラメータとコマンド種類を取得 ---
	CmdAutoType cmdtype = exeCmdParam(cmdset.arg);

	//--- 範囲設定 ---
	RangeMsec autoscope = cmdset.limit.getFrameRange();
	if (autoscope.st < 0){
		autoscope.st = cmdset.limit.getHead();
	}
	if (autoscope.ed < 0){
		autoscope.ed = cmdset.limit.getTail();
	}

	bool exeflag = false;
	switch(cmdtype){
		case CmdAutoType::CutTR :
			exeflag = startAutoCutTR(autoscope);
			break;
		case CmdAutoType::CutEC :
			exeflag = startAutoCutEC(autoscope);
			break;
		case CmdAutoType::AddTR :
			exeflag = startAutoAddTR(autoscope);
			break;
		case CmdAutoType::AddSP :
			exeflag = startAutoAddSP(autoscope);
			break;
		case CmdAutoType::AddEC :
			exeflag = startAutoAddEC(autoscope);
			break;
		case CmdAutoType::Edge :
			exeflag = startAutoEdge(cmdset.limit);
			break;
		case CmdAutoType::AtCM :
			{
				JlsAutoReform func_reform(pdata);
				exeflag = func_reform.startAutoCM(cmdset.arg);
			}
			break;
		case CmdAutoType::AtUP :
			exeflag = startAutoUp();
			break;
		case CmdAutoType::AtBorder :
			exeflag = startAutoBorder(autoscope);
			break;
		case CmdAutoType::AtIClear :
			exeflag = startAutoIClear(autoscope);
			break;
		case CmdAutoType::Ins :
			exeflag = startAutoIns(cmdset.limit);
			break;
		case CmdAutoType::Del :
			exeflag = startAutoDel(cmdset.limit);
			break;
		case CmdAutoType::AtChg :
			exeflag = startAutoChg(cmdset.limit);
			break;
		default :
			cerr << "error:internal setting (AutoType: " << static_cast<int>(cmdtype) << ")"  << endl;
			break;
	}
	return exeflag;
}

//---------------------------------------------------------------------
// Auto系コマンド用のパラメータ取得
//---------------------------------------------------------------------
jlscmd::CmdAutoType JlsAutoScript::exeCmdParam(JlsCmdArg &cmdarg){
	//--- コマンドから種類を選択 ---
	CmdAutoType cmdtype = CmdAutoType::None;
	switch(cmdarg.cmdsel){
		case CmdType::AutoCut :
			if (cmdarg.selectAutoSub == CmdTrSpEcID::TR){
				cmdtype = CmdAutoType::CutTR;
			}
			else if (cmdarg.selectAutoSub == CmdTrSpEcID::EC){
				cmdtype = CmdAutoType::CutEC;
			}
			else{
				cerr << "error: AutoCut is need TR/EC" << endl;
			}
			break;
		case CmdType::AutoAdd :
			if (cmdarg.selectAutoSub == CmdTrSpEcID::TR){
				cmdtype = CmdAutoType::AddTR;
			}
			else if (cmdarg.selectAutoSub == CmdTrSpEcID::EC){
				cmdtype = CmdAutoType::AddEC;
			}
			else if (cmdarg.selectAutoSub == CmdTrSpEcID::SP){
				cmdtype = CmdAutoType::AddSP;
			}
			else{
				cerr << "error: AutoAdd is need TR/EC/SP" << endl;
			}
			break;
		case CmdType::AutoEdge :
			cmdtype = CmdAutoType::Edge;
			break;
		case CmdType::AutoCM :
			cmdtype = CmdAutoType::AtCM;
			break;
		case CmdType::AutoBorder :
			cmdtype = CmdAutoType::AtBorder;
			break;
		case CmdType::AutoIClear :
			cmdtype = CmdAutoType::AtIClear;
			break;
		case CmdType::AutoUp :
			cmdtype = CmdAutoType::AtUP;
			break;
		case CmdType::AutoIns :
			cmdtype = CmdAutoType::Ins;
			break;
		case CmdType::AutoDel :
			cmdtype = CmdAutoType::Del;
			break;
		default :
			if (cmdarg.getOpt(OptType::FlagAutoChg) > 0){
				cmdtype = CmdAutoType::AtChg;
			}
			else{
				cerr << "error:internal setting at autoCmd(Command: " << static_cast<int>(cmdtype) << ")"  << endl;
			}
			break;
	}

	//--- パラメータ格納 ---
	if (cmdtype != CmdAutoType::None){
		if (cmdarg.cmdsel != CmdType::AutoCM){	// AutoCMは別途実行時に別クラスで設定
			m_autoArg.setParam(cmdarg, cmdtype);
		}
	}

	return cmdtype;
}



//=====================================================================
// 各Auto系コマンド
//=====================================================================

//---------------------------------------------------------------------
// AutoUp実行開始
// 出力：
//  返り値  : 位置更新実行 0=未実行 1=実行
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoUp(){
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}
	//--- コマンド実行 ---
	int restruct = getAutoParam(ParamAuto::c_restruct);
	Nlg nlg_fall = 0;
	Nsc nsc_lastfall = 0;
	bool cont_bk = true;
	//--- 構成再構築用 ---
	JlsAutoReform func_reform(pdata);
	do{
		Msec msec_rise, msec_fall;
		bool cont_next;
		int num_scpos = pdata->sizeDataScp();
		//--- ロゴ位置を取得して次のロゴ位置に移動 ---
		nlg_fall = pdata->getResultLogoNext(msec_rise, msec_fall, cont_next, nlg_fall);
		//--- ロゴ期間がある場合 ---
		if (nlg_fall >= 0){
			//--- ロゴ番号取得。ない時に挿入によるシーンチェンジ番号(nsc)変更あり ---
			Nsc nsc_rise = pdata->getNscForceMsec(msec_rise, LOGO_EDGE_RISE);
			Nsc nsc_fall = pdata->getNscForceMsec(msec_fall, LOGO_EDGE_FALL);
			//--- 前立ち下がりと次立ち上がり位置の間をCM化 ---
			if (nsc_lastfall < nsc_rise){
				func_reform.mkReformTarget(nsc_lastfall, nsc_rise, 0, restruct);	// ロゴ無
			}
			//--- ロゴ期間をロゴ有に設定 ---
			if (nsc_rise < nsc_fall){
				func_reform.mkReformTarget(nsc_rise, nsc_fall, 1, restruct);		// ロゴ有
			}
			//--- 前立ち下がりと次立ち上がりが同一位置で連続扱いでない場合は切れ目設定 ---
			if (nsc_lastfall == nsc_rise && cont_bk == false && nsc_lastfall > 0){
				pdata->setScpChap(nsc_lastfall, SCP_CHAP_DUNIT);
			}
			nsc_lastfall = nsc_fall;
			cont_bk = cont_next;			// 次ロゴとの間がロゴが切れ目なしの連続か
		}
		//--- 最後のロゴ終了から最終位置までの設定 ---
		else if (nsc_lastfall >= 0 && nsc_lastfall < num_scpos-1){
			func_reform.mkReformTarget(nsc_lastfall, num_scpos-1, 0, restruct);		// ロゴ無
		}
	}while(nlg_fall >= 0);
	return true;
}

//---------------------------------------------------------------------
// AutoBorder実行開始（Borderとなっている構成を変更）
// 出力：
//  返り値  : 位置更新実行 0=未実行 1=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoBorder(RangeMsec autoscope){
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}
	//--- コマンド実行 ---
	bool exeflag = false;
	{
		Term term = {};
		bool cont = true;
		while( cont ){
			cont = getTermNext(term);
			if (cont && autoscope.st <= term.msec.st && term.msec.ed <= autoscope.ed){
				ScpArType arstat_term = getScpArstat(term);
				if ( jlsd::isScpArTypeBorder(arstat_term) ){
					int restruct = getAutoParam(ParamAuto::c_restruct);
					if (restruct){
						if (arstat_term == SCP_AR_B_UNIT) arstat_term = SCP_AR_L_UNIT;
						else arstat_term = SCP_AR_L_OTHER;
					}
					else{
						if (arstat_term == SCP_AR_B_UNIT) arstat_term = SCP_AR_N_UNIT;
						else arstat_term = SCP_AR_N_OTHER;
					}
					exeflag = true;
					setScpArstat(term, arstat_term);
				}
			}
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// AutoIClear実行開始（予告・番組提供等の属性情報を消去）
// 出力：
//  返り値  : 消去実行 0=未実行 1=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoIClear(RangeMsec autoscope){
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}
	//--- コマンド実行 ---
	bool exeflag = false;
	{
		Term term = {};
		bool cont = true;
		while( cont ){
			cont = getTermNext(term);
			if (cont && autoscope.st <= term.msec.st && term.msec.ed <= autoscope.ed){
				setScpArext(term, SCP_AREXT_NONE);
				exeflag = true;
			}
		}
		//--- 検索位置の初期化 ---
		pdata->recHold.msecTrPoint = -1;	// CutTRコマンドの設定位置（CM構成内部分割の位置判断用）
	}
	return exeflag;
}

//---------------------------------------------------------------------
// AutoChg実行開始（従来コマンドの-autochgオプション）
// 出力：
//  返り値  : 位置更新実行 0=未実行 1=実行
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoChg(JlsCmdLimit &cmdlimit){
	bool exeflag = false;
	//--- 対象の無音SCを設定（データ挿入によるシーンチェンジ番号(nsc)変更あり） ---
	Nrf  nrf_base    = cmdlimit.getLogoBaseNrf();
	LogoEdgeType edgelogo = LOGO_EDGE_RISE;
	if ( nrf_base >= 0 ){
		edgelogo = jlsd::edgeFromNrf(nrf_base);
	}
	Nsc  nsc_target  = cmdlimit.getResultTargetSel();
	Msec msec_target = cmdlimit.getTargetRangeForce();
	if (nsc_target < 0 && msec_target >= 0){
		nsc_target = pdata->getNscForceMsec(msec_target, edgelogo);
	}
	//--- ロゴ基準ではない場合(NextTail)の処理 ---
	if ( nrf_base < 0 ){
		if ( nsc_target > 0 ){
			exeflag = true;
			Nsc nsc_arstat = pdata->getNscNextScpDecide(nsc_target-1, SCP_END_EDGEIN);
			if ( nsc_arstat != nsc_target ){	// 新規推測構成区切りなら推測設定
				bool logoon = pdata->isElgInScp(nsc_arstat);
				JlsAutoReform func_reform(pdata);
				func_reform.mkReformTarget(-1, nsc_target, logoon, 0);
			}
		}
		return exeflag;			// ロゴ基準ではない場合(NextTail)は終了
	}
	//--- 基準位置を取得 ---
	Nsc  nsc_base = getNscElgFromNrf(nrf_base);
	//--- 有効位置であれば実行 ---
	if (nsc_target != nsc_base && nsc_base >= 0 && nsc_target >= 0){
		exeflag = true;
		JlsAutoReform func_reform(pdata);
		if (edgelogo == LOGO_EDGE_RISE){
			if (nsc_base < nsc_target){
				func_reform.mkReformTarget(nsc_base, nsc_target, 0, 1);		// ロゴ無
			}
			else{
				func_reform.mkReformTarget(nsc_target, nsc_base, 1, 1);		// ロゴ有
			}
		}
		if (edgelogo == LOGO_EDGE_FALL){
			if (nsc_base < nsc_target){
				func_reform.mkReformTarget(nsc_base, nsc_target, 1, 1);		// ロゴ有
			}
			else{
				func_reform.mkReformTarget(nsc_target, nsc_base, 0, 1);		// ロゴ無
			}
		}
	}
	return exeflag;
}

//---------------------------------------------------------------------
// 実ロゴエッジに対応するロゴ扱い構成位置を取得
// 出力：
//  返り値  : ロゴ扱い構成エッジのシーンチェンジ番号
//---------------------------------------------------------------------
Nsc JlsAutoScript::getNscElgFromNrf(Nrf nrf_base){
	LogoEdgeType edgelogo = jlsd::edgeFromNrf(nrf_base);
	int msec_base_nrf = pdata->getMsecLogoNrf(nrf_base);

	//--- 構成上の対応するロゴ端を取得 ---
	Nsc nsc_base_elg = -1;
	if (nrf_base >= 0){
		//--- ロゴに対応する構成区切り位置取得 ---
		ElgCurrent elg = {};
		bool flag_cont = true;
		while( getElgNextKeep(elg) && flag_cont){
			if (edgelogo == LOGO_EDGE_RISE){
				if ((elg.msecLastFall < msec_base_nrf || elg.msecLastFall <= 0) && msec_base_nrf < elg.msecFall){
					nsc_base_elg = elg.nscRise;
					flag_cont    = false;
				}
			}
			if (edgelogo == LOGO_EDGE_FALL){
				if (elg.msecLastRise < msec_base_nrf && msec_base_nrf < elg.msecRise){
					nsc_base_elg = elg.nscLastFall;
					flag_cont    = false;
				}
			}
		}
		//--- 最後のロゴ立ち下がり検出 ---
		if (flag_cont == true && edgelogo == LOGO_EDGE_FALL){
			if (elg.msecLastRise <= msec_base_nrf){
				nsc_base_elg = elg.nscLastFall;
			}
		}
	}
	//--- 有効な位置関係か確認 ---
	if (nsc_base_elg >= 0){
		Msec msec_base_elg = pdata->getMsecScpEdge(nsc_base_elg, edgelogo);
		//--- 基準位置とロゴ扱い位置が離れていたら無効化する処理 ---
		if (abs(msec_base_nrf - msec_base_elg) > 31*1000){		// 31秒以上
			nsc_base_elg = -1;
		}
	}
	return nsc_base_elg;
}


//---------------------------------------------------------------------
// AutoIns実行開始
// 出力：
//  返り値  : 位置更新実行 0=未実行 1=実行
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoIns(JlsCmdLimit &cmdlimit){
	bool exeflag = false;
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}
	//--- コマンド実行 ---
	RangeNscMsec rangeData;
	int restruct = getAutoParam(ParamAuto::c_restruct);
	if ( subInsDelGetRange(rangeData, cmdlimit) ){
		//--- 構成情報更新 ---
		JlsAutoReform func_reform(pdata);
		func_reform.mkReformTarget(rangeData.nsc.st, rangeData.nsc.ed, 1, restruct);	// ロゴ有
		exeflag = true;
		//--- 属性情報の追加変更 ---
		bool flagAdd = true;		// 追加
		subInsDelAddUnit(rangeData.msec, flagAdd);
		subInsDelChangeArExt(rangeData.msec, flagAdd);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// AutoDel実行開始
// 出力：
//  返り値  : 位置更新実行 0=未実行 1=実行
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoDel(JlsCmdLimit &cmdlimit){
	bool exeflag = false;
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}
	//--- コマンド実行 ---
	RangeNscMsec rangeData;
	int restruct = getAutoParam(ParamAuto::c_restruct);
	if ( subInsDelGetRange(rangeData, cmdlimit) ){
		//--- 構成情報更新 ---
		JlsAutoReform func_reform(pdata);
		func_reform.mkReformTarget(rangeData.nsc.st, rangeData.nsc.ed, 0, restruct);	// ロゴ無
		exeflag = true;
		//--- 属性情報の追加変更 ---
		bool flagAdd = false;			// 追加ではなく削除
		subInsDelAddUnit(rangeData.msec, flagAdd);
		subInsDelChangeArExt(rangeData.msec, flagAdd);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// AutoIns / AutoDel用 範囲情報取得
// 出力：
//  返り値  : 基準位置の存在有無
//  nsc_target : ターゲットシーンチェンジ番号
//  nsc_base   : 基準シーンチェンジ番号
// 注意点：
//   データ挿入によるシーンチェンジ番号(nsc)変更あり
//---------------------------------------------------------------------
bool JlsAutoScript::subInsDelGetRange(RangeNscMsec& rangeData, JlsCmdLimit &cmdlimit){
	LogoEdgeType edge = cmdlimit.getLogoBaseEdge();
	//--- 基準位置取得 ---
	Nsc  nsc_base = subInsDelGetBase(cmdlimit);
	Msec msec_base = pdata->getMsecScpEdge(nsc_base, edge);
	//--- ターゲット位置取得 ---
	Nsc  nsc_target  = cmdlimit.getResultTargetSel();
	Msec msec_target = cmdlimit.getTargetRangeForce();
	if (nsc_target < 0){
		if ( msec_target >= 0 ){
			nsc_target = pdata->getNscForceMsec(msec_target, edge);
		}
	}else{
		msec_target = pdata->getMsecScp(nsc_target);
	}
	//--- 基準位置の更新（ターゲット位置が新規追加の時変わるため） ---
	if (nsc_base >= 0){
		nsc_base = pdata->getNscFromMsecAllEdgein(msec_base);
	}
	//--- 結果代入 ---
	rangeData.nsc  = { nsc_target,  nsc_base  };
	rangeData.msec = { msec_target, msec_base };
	if (nsc_target < 0 || nsc_base < 0){
		return false;
	}
	return true;
}


//---------------------------------------------------------------------
// AutoIns / AutoDel用 基準位置情報取得
// 出力：
//  返り値  : 基準位置のシーンチェンジ番号
//---------------------------------------------------------------------
Nsc JlsAutoScript::subInsDelGetBase(JlsCmdLimit &cmdlimit){
	Nsc nsc_base = -1;
	Nsc limit_nscend  = cmdlimit.getResultTargetEnd();
	Nsc limit_nscbase = cmdlimit.getLogoBaseNsc();
	Nrf limit_nrfbase = cmdlimit.getLogoBaseNrf();

	if (limit_nscend >= 0){
		nsc_base = limit_nscend;
	}
	else if (limit_nscbase >= 0){
		nsc_base = limit_nscbase;
	}
	else if (limit_nrfbase >= 0){
		nsc_base = pdata->getNscFromNrf(limit_nrfbase, pdata->msecValLap2, SCP_CHAP_DECIDE, false);
//		nsc_base = getNscElgFromNrf(limit_nrfbase);
	}
	return nsc_base;
}
//---------------------------------------------------------------------
// AutoIns / AutoDel用 独立構成の設定
//---------------------------------------------------------------------
void JlsAutoScript::subInsDelAddUnit(RangeMsec rmsec, bool flagAdd){
	//--- -unitオプション指定時のみ動作 ---
	int nUnit = getAutoParam(ParamAuto::c_unit);
	if ( nUnit > 0 ){
		//--- 位置情報取得 ---
		Nsc nscFrom = pdata->getNscFromMsecAllEdgein(rmsec.st);
		Nsc nscTo   = pdata->getNscFromMsecAllEdgein(rmsec.ed);
		//--- 設定内容（AutoInsは独立追加／AutoDelは独立解除） ---
		ScpChapType chapType = ( flagAdd )? SCP_CHAP_DUNIT : SCP_CHAP_DFORCE;
		//--- 実施 ---
		if ( nscFrom > 0 ){
			pdata->setScpChap(nscFrom, chapType);
		}
		if ( nscTo > 0 ){
			pdata->setScpChap(nscTo, chapType);
		}
	}
}
//---------------------------------------------------------------------
// AutoIns / AutoDel用 追加属性情報の設定
//---------------------------------------------------------------------
void JlsAutoScript::subInsDelChangeArExt(RangeMsec rmsec, bool flagAdd){
	//--- 追加の更新有無 ---
	CmdTrSpEcID idOpt = (CmdTrSpEcID) getAutoParam(ParamAuto::v_info);
	if ( idOpt == CmdTrSpEcID::None ){
		return;
	}
	//--- -info設定に対応する属性設定値 ---
	ScpArExtType arExt = SCP_AREXT_NONE;
	if ( flagAdd ){
		switch( idOpt ){
			case CmdTrSpEcID::TR :
				arExt = SCP_AREXT_L_TRKEEP;
				break;
			case CmdTrSpEcID::SP :
				arExt = SCP_AREXT_L_SP;
				break;
			case CmdTrSpEcID::EC :
				arExt = SCP_AREXT_L_EC;
				break;
			case CmdTrSpEcID::LG :
				arExt = SCP_AREXT_L_LGADD;
				break;
			default :
				break;
		}
	}else{
		switch( idOpt ){
			case CmdTrSpEcID::TR :
				arExt = SCP_AREXT_L_TRCUT;
				break;
			case CmdTrSpEcID::SP :
				arExt = SCP_AREXT_L_SP;
				break;
			case CmdTrSpEcID::EC :
				arExt = SCP_AREXT_L_ECCUT;
				break;
			case CmdTrSpEcID::LG :
				arExt = SCP_AREXT_L_LGCUT;
				break;
			default :
				break;
		}
	}
	//--- 更新 ---
	{
		//--- 位置情報取得 ---
		Nsc nscFrom = pdata->getNscFromMsecAllEdgein(rmsec.st);
		Nsc nscTo   = pdata->getNscFromMsecAllEdgein(rmsec.ed);
		//--- fromを前側、toを後側 ---
		if (nscFrom > nscTo){
			Nsc tmp = nscFrom;
			nscFrom = nscTo;
			nscTo   = tmp;
		}
		//--- 片側でもマイナスの時は設定なし ---
		if (nscFrom < 0){
			return;
		}
		//--- 処理開始 ---
		Term term = {};
		bool cont = true;
		while( cont ){
			cont = getTermNext(term);
			if ( cont && nscFrom <= term.nsc.st && term.nsc.ed <= nscTo ){
				setScpArext(term, arExt);
			}
		}
	}
}

//---------------------------------------------------------------------
// AutoCut TR 予告を残し番宣をカットする処理
// 出力：
//  返り値  : カット処理実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoCutTR(RangeMsec autoscope){
	//--- 初期状態設定 ---
	Msec msec_spc = pdata->msecValSpc;
	//--- 予告設定位置（CM構成内部分割を自動で行う位置） ---
	pdata->recHold.msecTrPoint = autoscope.st;
	//--- 実行有無確認 ---
	int prm_c_exe = getAutoParam(ParamAuto::c_exe);
	if (prm_c_exe == 0){
		return false;
	}

	//--- 予告開始位置を取得 ---
	Nsc nsc_trstart = subCutTRGetLocSt(autoscope);
	if (nsc_trstart < 0){	// 予告候補が見つからない場合
		//--- 予告検出なしの検出 ---
		return false;
	}

	//--- パラメータ取得 ---
	int prm_msec_wlogo_trmax = getConfig(ConfigVarType::msecWLogoTRMax);
	int prm_msec_wcomp_trmax = getConfig(ConfigVarType::msecWCompTRMax);
	int prm_limit     = getAutoParam(ParamAuto::v_limit);
	int prm_trsumprd  = getAutoParam(ParamAuto::v_trsumprd);
	int prm_c_cutst   = getAutoParam(ParamAuto::c_cutst);

	//--- 予告位置を設定 ---
	bool ret = false;
	{
		int state_cut = 0;
		int ncut_rest = prm_limit;
		Nlg nlg = 0;
		Nlg nlg_start = 0;
		Sec sec_tr_total = 0;
		ElgCurrent elg = {};
		bool det_logo = getElgNextKeep(elg);
		Term term = {};
		term.ini = nsc_trstart;
		bool cont = getTermNext(term);
		while(cont){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			bool flag_scope = checkScopeTerm(term, autoscope);
			if (flag_scope &&									// 範囲内
				(jlsd::isScpArTypeLogoBorder(arstat_term) ||	// ロゴあり部分
				 term.nsc.st == nsc_trstart) &&					// 予告先頭位置
				((arext_term != SCP_AREXT_L_LGCUT) &&			// エッジ処理済み部分は除く
				 (arext_term != SCP_AREXT_L_LGADD))){

				//--- シーンチェンジに対応するロゴ位置取得 ---
				while(term.nsc.ed > elg.nscFall && det_logo){
					nlg ++;
					det_logo = getElgNextKeep(elg);
				}
				//--- 前フレームからの期間 ---
				Msec msec_dif_term = term.msec.ed - term.msec.st;
				Sec sec_dif_term = pdata->cnv.getSecFromMsec( msec_dif_term );
				//--- 初回チェック ---
				if (state_cut == 0){
					nlg_start = nlg;
					state_cut = 1;
				}
				//--- カット開始条件判断 ---
				bool flag_cut1st = false;
				if (state_cut == 1 && ncut_rest == 0 && sec_tr_total >= prm_trsumprd && prm_c_cutst != 3){
					if (((sec_dif_term % 15 == 0) && prm_c_cutst == 0) ||
						((sec_dif_term >= 15) && prm_c_cutst == 1) ||
						(prm_c_cutst == 2)){
						state_cut = 2;
						flag_cut1st = true;
					}
				}
				//--- カット開始後の処理 ---
				if (state_cut == 2){
					if ((elg.msecFall - elg.msecRise <= prm_msec_wlogo_trmax + msec_spc ||
						 nlg == nlg_start) &&											// ロゴが２分以内か開始ロゴ
						elg.msecFall - term.msec.ed <= prm_msec_wlogo_trmax + msec_spc &&	// ロゴ終了まで２分以内
						msec_dif_term <= prm_msec_wcomp_trmax + msec_spc &&				// 構成が60秒以内
						elg.msecFall <= autoscope.ed + msec_spc){						// ロゴ終了地点が範囲内
						if (jlsd::isScpArTypeBorder(arstat_term) && flag_cut1st){		// カット開始直後のBorder
							setScpArext(term, SCP_AREXT_L_TRRAW);
						}
						else{
							subCutTRSetCut(term);
						}
						ret = true;
					}
					else{
						state_cut = 3;					// 完全終了
					}
				}
				//--- カット開始まで残す処理 ---
				if (state_cut == 1){
					if (ncut_rest > 0){
						setScpArext(term, SCP_AREXT_L_TRKEEP);
						ncut_rest --;
						sec_tr_total += sec_dif_term;
					}
					else{								// エンドカード判断待ち
						setScpArext(term, SCP_AREXT_L_TRRAW);
					}
				}
			}
			else if (flag_scope && (arstat_term == SCP_AR_N_OTHER)){	// ロゴなし不明構成
				if (state_cut == 1 || state_cut == 2){		// カット期間中
					setScpArext(term, SCP_AREXT_N_TRCUT);
				}
			}
			//--- 次の位置を設定 ---
			cont = getTermNext(term);
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// 予告開始位置を取得
// 出力：
//  返り値  : 予告開始位置となるシーンチェンジ番号（-1の時該当なし）
//---------------------------------------------------------------------
Nsc JlsAutoScript::subCutTRGetLocSt(RangeMsec autoscope){
	Nsc  nsc_cand = -1;
	bool flag_cand  = false;
	bool det_logo   = true;
	ElgCurrent elg = {};
	while(det_logo){
		det_logo = getElgNextKeep(elg);
		if (det_logo){
			subCutTRGetLocStSub(&nsc_cand, &flag_cand, autoscope, elg);
		}
	};
	return nsc_cand;
}

//---------------------------------------------------------------------
// 予告開始位置を取得（１ロゴ期間内の判断）
// 出力：
//   *r_nsc_cand   : 候補位置番号
//   *r_flag_cand  : 候補状態(false:候補なし  true:候補あり)
//---------------------------------------------------------------------
void JlsAutoScript::subCutTRGetLocStSub(Nsc *r_nsc_cand, bool *r_flag_cand, RangeMsec autoscope, ElgCurrent elg){
	//--- 設定値 ---
	Msec msec_spc = pdata->msecValSpc;
	int prm_wcomp_spmin      = getConfig(ConfigVarType::secWCompSPMin);
	int prm_wcomp_spmax      = getConfig(ConfigVarType::secWCompSPMax);
	int prm_msec_wlogo_trmax = getConfig(ConfigVarType::msecWLogoTRMax);
	int prm_msec_wcomp_trmax = getConfig(ConfigVarType::msecWCompTRMax);

	//--- 使用パラメータ ---
	int prm_c_from   = getAutoParam(ParamAuto::c_from);
	int prm_c_lgpre  = getAutoParam(ParamAuto::c_lgpre);
	int prm_trscope  = getAutoParam(ParamAuto::v_trscope);
	int prm_tr1stprd = getAutoParam(ParamAuto::v_tr1stprd);

	//--- ロゴ終了が期間内 ---
	if (elg.msecFall <= autoscope.st + msec_spc || elg.msecFall >= autoscope.ed + msec_spc){
		return;
	}

	//--- 前回状態を取り込み ---
	Nsc nsc_cand = *r_nsc_cand;
	bool flag_cand  = *r_flag_cand;

	//--- ロゴ期間が予告用の長さか確認 ---
	bool longlogo = false;
	{
		if (elg.msecRise < autoscope.st - msec_spc){
			longlogo = true;
		}
		else if (elg.msecFall - elg.msecRise > prm_msec_wlogo_trmax + msec_spc){
			longlogo = true;
		}
	}
	//--- ロゴ位置設定（ロゴエッジ処理している所は除く） ---
	RangeNsc rnsc_logo = {elg.nscRise, elg.nscFall};
	{
		Term term = {};
		term.ini = rnsc_logo.ed;					// 開始位置
		bool cont = getTermPrev(term);
		while(cont){
			if (term.nsc.st < rnsc_logo.st){
				cont = false;
			}
			else{
				ScpArExtType arext_term = getScpArext(term);
				if ((arext_term == SCP_AREXT_L_LGCUT) ||
					(arext_term == SCP_AREXT_L_LGADD)){
					rnsc_logo.ed = term.nsc.st;		// エッジ処理している所は除く
				}
				else{
					cont = false;
				}
			}
			if (cont){
				cont = getTermPrev(term);
			}
		}
	}
	Msec msec_dst_fall = pdata->getMsecScp(rnsc_logo.ed);

	//--- ロゴ開始直後にすぐ終わるかチェック ---
	bool shortlogo = false;
	{
		Term term = {};
		term.ini = rnsc_logo.st;					// 開始位置
		bool cont = getTermNext(term);
		if (term.nsc.ed != rnsc_logo.ed || !cont){
		}
		else if (pdata->cnv.getSecFromMsec(msec_dst_fall - elg.msecRise) < 15){
			ElgCurrent next_elg = elg;
			bool valid_next = getElgNextKeep(next_elg);
			//--- 次のロゴがないか終了地点よりも後まで続く場合、単体短時間フラグ ---
			if (valid_next == false || next_elg.msecFall >= autoscope.ed - msec_spc){
				shortlogo = true;
			}
		}
	}
	//--- ロゴ手前でも位置確定がまだで予告候補があればチェック ---
	if (rnsc_logo.st < rnsc_logo.ed && flag_cand == false && (prm_c_lgpre == 1 || prm_c_lgpre == 2)){
		//--- １つ前の構成を取得 ---
		Term term1 = {};
		term1.ini = rnsc_logo.st;					// 開始位置
		bool cont1 = getTermPrev(term1);
		//--- ロゴ手前がCM15秒単位検出ではない時 ---
		if (cont1 && isScpArstatCmUnit(term1) == false){
			//--- ２つ前の構成を取得 ---
			Term term2 = term1;
			bool cont2 = getTermPrev(term2);
			if (term2.nsc.st <= elg.nscLastFall){
				cont2 = false;
			}
			//--- ３つ前の構成を取得 ---
			Term term3 = term2;
			bool cont3 = getTermPrev(term2);
			if (term3.nsc.st <= elg.nscLastFall || !cont2){
				cont3 = false;
			}
			//--- 共通の構成期間制限を確認 ---
			bool flag_common_cond = false;
			if (longlogo == false && (prm_c_lgpre == 2 || shortlogo == true)){
				flag_common_cond = true;
			}
			//--- ２つ前はCM15秒単位だった場合 ---
			if (cont2 && isScpArstatCmUnit(term2) == true){
				// １つ前の構成が番組提供の構成外であれば予告に追加
				Sec sec_dif_term1 = pdata->cnv.getSecFromMsec(term1.msec.ed - term1.msec.st);
				if ((sec_dif_term1 < prm_wcomp_spmin || sec_dif_term1 > prm_wcomp_spmax) &&
					flag_common_cond &&
					(term1.msec.st >= autoscope.st) &&
					(sec_dif_term1 <= prm_tr1stprd || prm_tr1stprd == 0)){
					nsc_cand = term1.nsc.st;
					flag_cand = true;
				}
			}
			// ２つ前もCM15秒単位検出ではなく、３つ前がCM15秒単位だった場合
			else if (cont3 && isScpArstatCmUnit(term3) == true){
				// １つ前の構成が番組提供の構成外であれば予告に追加
				Sec sec_dif_term2 = pdata->cnv.getSecFromMsec(term2.msec.ed - term2.msec.st);
				if ((sec_dif_term2 < prm_wcomp_spmin || sec_dif_term2 > prm_wcomp_spmax) &&
					flag_common_cond &&
					(term1.msec.st >= autoscope.st) &&
					(sec_dif_term2 <= prm_tr1stprd || prm_tr1stprd == 0)){
					nsc_cand = term2.nsc.st;
					flag_cand = true;
				}
			}
		}
	}
	//--- 自動判別でCM明けのロゴがあれば優先させる場合のため ---
	if (rnsc_logo.st < rnsc_logo.ed && flag_cand == false && nsc_cand >= 0){
		if (nsc_cand >= 0){
			//--- 短い単一構成以外のロゴは新候補にするため過去候補をクリア ---
			Term term = {};
			term.ini = rnsc_logo.st;
			if (getTermPrev(term)){
				//--- ロゴ手前がCM検出ではない時はCM明けから ---
				if (isScpArstatCmUnit(term) == false){
					nsc_cand = -1;
				}
				else{
					if (shortlogo == false){
						nsc_cand = -1;
					}
				}
			}
		}
		if (nsc_cand >= 0){
			//--- ロゴ期間終了位置から次の構成位置を取得 ---
			Term term = {};
			term.ini = rnsc_logo.ed;
			if (getTermNext(term)){
				//--- ロゴ直後がCM検出ではない時はCM明けから ---
				if (isScpArstatCmUnit(term) == false){
					nsc_cand = -1;
				}
			}
		}
	}
	//--- ロゴ期間内のシーンチェンジ確認 ---
	if (rnsc_logo.st < rnsc_logo.ed){
		Term term = {};
		term.ini = rnsc_logo.st;
		bool cont = getTermNext(term);
		while(cont){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			if (checkScopeTerm(term, autoscope) &&		// 範囲内
				(term.nsc.st >= rnsc_logo.st && term.nsc.ed <= rnsc_logo.ed) &&		// ロゴ範囲内
				jlsd::isScpArTypeLogo(arstat_term) &&	// ロゴあり部分
				((arext_term != SCP_AREXT_L_LGCUT) &&	// エッジ処理済み部分は除く
				 (arext_term != SCP_AREXT_L_LGADD))){
				//--- 前回からの期間を取得 ---
				Msec msec_logo_ed = pdata->getMsecScp(rnsc_logo.ed);
				Msec msec_dif_term = term.msec.ed - term.msec.st;
				Sec sec_dif_term = pdata->cnv.getSecFromMsec(msec_dif_term);
				Sec sec_dif_lged = pdata->cnv.getSecFromMsec(msec_logo_ed - term.msec.ed);
				//--- 構成が予告期間以上あったら本編と認識 ---
				if (msec_dif_term > prm_msec_wcomp_trmax + msec_spc){
					longlogo = true;
					nsc_cand = -1;
				}
				//--- 最初の予告位置設定 ---
				else if (nsc_cand < 0){
					// 構成期間が所定以下なら設定
					if (sec_dif_term <= prm_tr1stprd || prm_tr1stprd == 0){
						nsc_cand = term.nsc.st;
					}
				}
				//--- 本編後の同ロゴ内位置だった場合条件を満たすか確認 ---
				if (longlogo){
					//--- CM明けから数える場合は本編ロゴ中は無効化 ---
					if (prm_c_from == 1){
						 nsc_cand = -1;
					}
					else if ((((sec_dif_term % 15) != 0) && prm_c_from == 5) ||
							 (((sec_dif_term % 5)  != 0) && prm_c_from == 4) ||
							 (sec_dif_lged > prm_trscope)){
							 nsc_cand = -1;
					}
				}
			}
			//--- 確定するロゴ期間の場合はフラグ設定 ---
			if (longlogo && prm_c_from == 2){
				flag_cand = false;
			}
			else{
				if (nsc_cand >= 0){
					flag_cand = true;
				}
			}
			//--- 次の位置を設定 ---
			cont = getTermNext(term);
		}
	}

	*r_nsc_cand = nsc_cand;
	*r_flag_cand  =flag_cand;
}

//---------------------------------------------------------------------
// 予告後の不要構成カット処理
//---------------------------------------------------------------------
void JlsAutoScript::subCutTRSetCut(Term &term){
	//--- 期間によりカット状態設定 ---
	Msec msec_dif_term = term.msec.ed - term.msec.st;
	Sec sec_dif_term = pdata->cnv.getSecFromMsec( msec_dif_term );
	if (sec_dif_term >= 14){
		setScpArext(term, SCP_AREXT_L_TRCUT);
	}
	else{
		setScpArext(term, SCP_AREXT_L_ECCUT);
	}
	//--- 設定値読み込み ---
	int prm_msec_wcomp_trmax = getConfig(ConfigVarType::msecWCompTRMax);
	int mgn_cm_detect = pdata->getConfig(ConfigVarType::msecMgnCmDetect);
	//--- 前構成と15秒単位の結合処理 ---
	{
		Term preterm = term;
		int nloop = 0;
		bool cont = true;
		while(cont && nloop <= 2){
			//--- 結合はカット状態同士のみ。それ以外では終了 ---
			ScpArExtType arext_term = getScpArext(preterm);
			if ((arext_term != SCP_AREXT_L_TRCUT) &&
				(arext_term != SCP_AREXT_L_ECCUT)){
				cont = false;
			}
			if (cont){
				//--- 15秒単位からずれがない所を探す ---
				Msec msec_dif_target = term.msec.ed - preterm.msec.st;
				Msec msec_gap = ((msec_dif_target + 7500) % 15000) - 7500;
				if (msec_dif_target > prm_msec_wcomp_trmax + mgn_cm_detect){	// 60秒を超えたら終了
					cont = false;
				}
				else if (msec_gap <= mgn_cm_detect){
					if (nloop > 0){
						//--- 15秒単位の所があれば途中を結合 ---
						Term tmpterm = preterm;
						for(int i=0; i<nloop; i++){
							pdata->setScpChap(tmpterm.nsc.ed, SCP_CHAP_CPOSIT);
							getTermNext(tmpterm);
						}
						//--- termを設定し直し ---
						Nsc nsc_end = term.nsc.ed;
						term = {};
						term.ini = preterm.nsc.st;
						bool tmpcont = getTermNext(term);
						while(term.nsc.ed < nsc_end && tmpcont){	// 念のため確認
							tmpcont = getTermNext(term);
						}
						setScpArext(term, SCP_AREXT_L_TRCUT);
					}
					cont = false;
				}
			}
			if (cont){
				cont = getTermPrev(preterm);
				nloop ++;
			}
		}
	}
}



//---------------------------------------------------------------------
// AutoCut EC エンドカード部分カット処理
// 出力：
//  返り値  : カット処理実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoCutEC(RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_limit  = getAutoParam(ParamAuto::v_limit);

	//--- 初期状態設定 ---
	bool ret = false;

	//--- 位置情報を取得 ---
	vector<int> local_cntcut(pdata->sizeDataScp());
	int ovw_force;
	int n_cutdst = subCutECGetLocSt(local_cntcut, &ovw_force, autoscope);

	//--- 確認した位置でカット処理 ---
	{
		int  state_cut = 0;
		Term term = {};
		bool cont = getTermNext(term);
		while(cont){
			ScpArExtType arext_term = getScpArext(term);
			int cntcut_term = local_cntcut[term.nsc.ed];
			//--- 範囲内でロゴ有の配置位置だった場合のみ実行 ---
			if (checkScopeTerm(term, autoscope)){
				if (state_cut == 0){
					if (cntcut_term == n_cutdst){
						state_cut = 1;
					}
					else if (cntcut_term > 0){
						if (prm_limit > 0){
							setScpArext(term, SCP_AREXT_L_EC);
						}
					}
				}
				if (state_cut == 1){
					if (cntcut_term > 0){
						if ((arext_term != SCP_AREXT_L_EC &&
							 arext_term != SCP_AREXT_L_SP) ||
							(ovw_force > 0 && ovw_force <= cntcut_term)){
							setScpArext(term, SCP_AREXT_L_ECCUT);
							ret = true;
						}
					}
					else if (arext_term == SCP_AREXT_L_TRCUT ||
							 arext_term == SCP_AREXT_L_ECCUT){
						state_cut = 2;
						cont = false;
					}
				}
			}
			else if (term.msec.ed > autoscope.ed){
				cont = false;
			}
			//--- 次位置を設定 ---
			if (cont){
				cont = getTermNext(term);
			}
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// 開始位置・情報を取得
// 出力：
//  返り値  : エンドカードのカット開始位置となるシーンチェンジ番号（-1の時該当なし）
//  local_cntcut[] : 各シーンチェンジ番号のエンドカード候補を１から順番に番号付け
//  r_ovw_force    : 0:既存情報の上書きなし 1-:既存の番組提供・エンドカード設定を上書き開始する番号
//---------------------------------------------------------------------
int JlsAutoScript::subCutECGetLocSt(vector<int> &local_cntcut, int *r_ovw_force, RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_limit   = getAutoParam(ParamAuto::v_limit);
	int prm_period  = getAutoParam(ParamAuto::v_period);
	int prm_maxprd  = getAutoParam(ParamAuto::v_maxprd);
	int prm_c_sel   = getAutoParam(ParamAuto::c_sel);
	int prm_c_cutsp = getAutoParam(ParamAuto::c_cutsp);
	int prm_c_cutla = getAutoParam(ParamAuto::c_cutla);
	int prm_c_cutlp = getAutoParam(ParamAuto::c_cutlp);
	int prm_c_cut30 = getAutoParam(ParamAuto::c_cut30);

	//--- 初期状態設定 ---
	local_cntcut.assign(local_cntcut.size(), 0);
	int prmflag_cutlast = 0;
	if (prm_limit < 0 || prm_c_cutla > 0 || prm_c_cutlp > 0){
		prmflag_cutlast = 1;
	}

	//--- 先頭位置を検索 ---
	Nsc nsc_cand = -1;
	{
		bool flag_cand    = false;
		bool flag_existec = false;
		bool flag_end     = false;
		Term term = {};
		bool cont = getTermNext(term);
		while(cont && !flag_end){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			//--- 範囲内でロゴ有の配置位置だった場合のみ実行 ---
			if (checkScopeTerm(term, autoscope)){
				Sec sec_dif_term = pdata->cnv.getSecFromMsec(term.msec.ed - term.msec.st);
				//--- エンドカードとして有効な所を検索する ---
				if (arext_term == SCP_AREXT_L_TRRAW){
					if (flag_cand == false){
						nsc_cand = term.nsc.st;
						flag_cand = true;
					}
				}
				else if (arext_term == SCP_AREXT_L_EC){
					if (flag_cand == false || flag_existec == false){
						nsc_cand = term.nsc.st;
						flag_cand = true;
						flag_existec = true;
					}
				}
				else if (arext_term == SCP_AREXT_L_SP){
					if (prmflag_cutlast > 0 || prm_c_cutsp > 0){	// カット対象とする場合
						if (nsc_cand < 0){
							nsc_cand = term.nsc.st;
						}
					}
					else{
						nsc_cand = -1;
						flag_cand = false;
					}
				}
				else if (arext_term == SCP_AREXT_L_TRCUT ||
						 arext_term == SCP_AREXT_L_ECCUT){
					flag_end = true;
				}
				else if (arext_term == SCP_AREXT_L_TRKEEP){
					nsc_cand = -1;
					flag_cand = false;
					if (prm_c_cut30 > 0){
						if (sec_dif_term == 30){
							prm_limit = 0;			// エンドカードなし
						}
					}
				}
				else if (jlsd::isScpArTypeLogo(arstat_term) &&
						 arext_term != SCP_AREXT_L_LGCUT &&
						 arext_term != SCP_AREXT_L_LGADD){
					if (flag_cand == false){
						// 検索範囲内かチェック
						if (subCutECCheckScope(sec_dif_term, prm_c_sel, prm_period, prm_maxprd)){
							if (prmflag_cutlast > 0 && nsc_cand < 0){
								nsc_cand = term.nsc.st;
							}
						}
						else{
							if (flag_cand == false && nsc_cand >= 0){
								nsc_cand = -1;
							}
						}
					}
				}
			}
			else if (term.msec.ed > autoscope.ed){
				flag_end = true;
			}
			//--- 次位置を設定 ---
			cont = getTermNext(term);
		}
	}

	//--- 各シーンチェンジ位置順番付け ---
	int n_cutmax = 0;
	Sec sec_dif_last = 0;
	if (nsc_cand >= 0){
		bool flag_end = false;
		bool flag_cont = false;
		Term term = {};
		term.ini = nsc_cand;
		bool cont = getTermNext(term);
		while(cont && flag_end == false){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			Sec sec_dif_term = pdata->cnv.getSecFromMsec(term.msec.ed - term.msec.st);
			//--- 範囲内でロゴ有の配置位置だった場合のみ実行 ---
			if (checkScopeTerm(term, autoscope)){
				bool flag_valid = false;
				//--- エンドカードとして有効な所を検索する ---
				if (arext_term == SCP_AREXT_L_TRRAW){
					flag_valid = true;
				}
				else if (arext_term == SCP_AREXT_L_EC){
					flag_valid = true;
				}
				else if (arext_term == SCP_AREXT_L_SP){
					flag_cont = true;								// 継続フラグを立てる
					if (prmflag_cutlast > 0 || prm_c_cutsp > 0){	// カット対象とする場合
						flag_valid = true;
					}
				}
				else if (arext_term == SCP_AREXT_L_TRCUT ||	// 既存カットがあれば終了
						 arext_term == SCP_AREXT_L_ECCUT){
					flag_end = true;
				}
				else if (jlsd::isScpArTypeLogo(arstat_term) &&
						 arext_term != SCP_AREXT_L_LGCUT &&
						 arext_term != SCP_AREXT_L_LGADD){
					// 検索範囲内かチェック
					if (subCutECCheckScope(sec_dif_term, prm_c_sel, prm_period, prm_maxprd)){
						if (prmflag_cutlast > 0 || flag_cont){
							flag_valid = true;
						}
					}
					else{
						flag_cont = false;					// 継続は終了
					}
				}
				if (flag_valid){
					//--- 開始位置取得 ---
					if (subCutECCheckScope(sec_dif_term, prm_c_sel, prm_period, prm_maxprd)){
						n_cutmax ++;
						local_cntcut[term.nsc.ed] = n_cutmax;
						sec_dif_last = sec_dif_term;		// 最後の期間を記憶
					}
				}
			}
			else if (term.msec.ed > term.msec.ed){
				flag_end = true;
			}
			//--- 次の位置 ---
			cont = getTermNext(term);
		}
	}
	//--- カット位置の番号を決定 ---
	int n_cutdst;
	int ovw_force = 0;
	if (prm_limit >= 0){				// 先頭から数える場合
		n_cutdst = prm_limit + 1;
		if (n_cutmax > 0 && prmflag_cutlast > 0){	// 最後をカットする場合
			if (prm_c_cutla > 0){
				ovw_force = n_cutmax;
				if (n_cutdst > n_cutmax){
					n_cutdst = n_cutmax;
				}
			}
			else if (prm_c_cutlp > 0){
				if (subCutECCheckScope(sec_dif_last, 2, prm_period, prm_maxprd)){
					ovw_force = n_cutmax;
					if (n_cutdst > n_cutmax){
						n_cutdst = n_cutmax;
					}
				}
			}
		}
	}
	else{								// 最後から数える場合
		n_cutdst = n_cutmax + prm_limit + 1;
		ovw_force = n_cutdst;
		if (prm_c_cutlp > 0){						// 期間条件がある場合
			if (subCutECCheckScope(sec_dif_last, 2, prm_period, prm_maxprd) == 0){
				n_cutdst = n_cutmax + 100;			// カット無効化
				ovw_force = 0;
			}
		}
	}
	*r_ovw_force = ovw_force;
	return n_cutdst;
}

//---------------------------------------------------------------------
// 入力秒数がエンドカード検索範囲内かチェック
// 出力：
//  返り値  : false=エンドカード検索範囲外 true=エンドカード検索範囲内
//---------------------------------------------------------------------
bool JlsAutoScript::subCutECCheckScope(Sec sec_dif, int prm_c_sel, int prm_period, int prm_maxprd){

	if ((sec_dif < 15 && prm_c_sel == 1) ||
		(sec_dif <= prm_period && prm_c_sel == 3) ||
		(sec_dif >= prm_period && sec_dif < 15 && prm_c_sel == 4) ||
		(sec_dif >= prm_period && sec_dif < prm_maxprd && prm_c_sel == 5) ||
		(sec_dif == prm_period && prm_c_sel == 2)){
		return true;
	}
	else{
		return false;
	}
}



//---------------------------------------------------------------------
// AutoAdd SP 番組提供追加処理
// 出力：
//  返り値  : カット処理実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoAddSP(RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_c_exe      = getAutoParam(ParamAuto::c_exe);
	int prm_limit      = getAutoParam(ParamAuto::v_limit);
	int prm_c_limtrsum = getAutoParam(ParamAuto::c_limtrsum);
	int prm_trsumprd   = getAutoParam(ParamAuto::v_trsumprd);
	int prm_c_in1      = getAutoParam(ParamAuto::c_in1);

	//--- 実行有無確認 ---
	if (prm_c_exe == 0){
		return false;
	}

	//--- limit確認、予告・番組提供有無確認 ---
	AddExistInfo exist_info;
	int cnt_limit = subAddGetLimit(exist_info, CmdAutoType::AddSP, autoscope);
	//--- limit上限確認、上限以上であれば何もせず終了 ---
	if (prm_limit != 0 && cnt_limit >= prm_limit){
		return false;
	}
	//--- 予告秒数が指定以上あれば何もせず終了 ---
	if (prm_c_limtrsum > 0 && exist_info.sec_tr >= prm_trsumprd){
		return false;
	}
	//--- 既存の番組提供情報は検索位置に使わない ---
	exist_info.sponsor = false;

	//--- 優先順位最大のシーンチェンジ位置確認（prm_limit == 0の時は書き換えも実行） ---
	int nsc_prior = subAddSearch(CmdAutoType::AddSP, exist_info, autoscope);
	if (nsc_prior < 0){
		return false;
	}

	//--- 一番優先順位の高い候補を番組提供とする ---
	if (nsc_prior > 0 && prm_limit > 0){
		//--- 予告と認識した最初の内容を番組提供部分とする場合 ---
		if (pdata->getScpArext(nsc_prior) == SCP_AREXT_L_TRKEEP){
			bool flag_detcand = false;
			Term term = {};
			term.ini = nsc_prior;
			bool cont = true;
			do{
				cont = getTermNext(term);
				if (cont){
					//--- 検索範囲を超えたら終了 ---
					if (term.msec.ed > autoscope.ed){
						cont = false;
					}
					else{
						//--- 候補の確認 ---
						ScpArType arstat_term = getScpArstat(term);
						ScpArExtType arext_term = getScpArext(term);
						if (jlsd::isScpArTypeLogoBorder(arstat_term) &&
							arext_term != SCP_AREXT_L_TRKEEP){
							flag_detcand = true;
						}
					}
				}
			} while(flag_detcand == false && cont);
			// 候補がある場合は予告認識を１つ後にずらす
			if (cont){
				setScpArext(term, SCP_AREXT_L_TRKEEP);
			}
			// 候補がない場合は強制設定時以外は中止
			else if (prm_c_in1 < 2){
				nsc_prior = -1;
			}
		}
		//--- 番組提供部分を設定 ---
		if (nsc_prior > 0){
			// 候補位置がカット状態だった場合は次以降のカット位置を確認
			if (pdata->getScpArext(nsc_prior) == SCP_AREXT_L_TRCUT){
				Term term = {};
				term.ini = nsc_prior;
				bool cont = true;
				do{
					cont = getTermNext(term);
					if (cont){
						ScpArExtType arext_term = getScpArext(term);
						// エンドカードカットだった場合は判断前に戻す
						if (arext_term == SCP_AREXT_L_ECCUT){
							setScpArext(term, SCP_AREXT_L_TRRAW);
						}
					}
				} while(cont);
			}
			subAddReviseUnitCm(nsc_prior);	// 合併１５秒単位CM処理
			pdata->setScpArext(nsc_prior, SCP_AREXT_L_SP);
		}
	}
	return true;
}


//---------------------------------------------------------------------
// AutoAdd EC エンドカード追加処理
// 出力：
//  返り値  : カット処理実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoAddEC(RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_c_exe      = getAutoParam(ParamAuto::c_exe);
	int prm_limit      = getAutoParam(ParamAuto::v_limit);
	int prm_c_limtrsum = getAutoParam(ParamAuto::c_limtrsum);
	int prm_trsumprd   = getAutoParam(ParamAuto::v_trsumprd);

	//--- 実行有無確認 ---
	if (prm_c_exe == 0){
		return false;
	}

	//--- limit確認、予告・番組提供有無確認 ---
	AddExistInfo exist_info;
	int cnt_limit = subAddGetLimit(exist_info, CmdAutoType::AddEC, autoscope);
	//--- limit上限確認、上限以上であれば何もせず終了 ---
	if (prm_limit != 0 && cnt_limit >= prm_limit){
		return false;
	}
	//--- 予告秒数が指定以上あれば何もせず終了 ---
	if (prm_c_limtrsum > 0 && exist_info.sec_tr >= prm_trsumprd){
		return false;
	}

	//--- 優先順位最大のシーンチェンジ位置確認（prm_limit == 0の時は書き換えも実行） ---
	int nsc_prior = subAddSearch(CmdAutoType::AddEC, exist_info, autoscope);
	if (nsc_prior < 0){
		return false;
	}

	//--- 一番優先順位の高い候補をエンドカードとする ---
	if (nsc_prior > 0 && prm_limit > 0){
		// エンドカードを設定
		subAddReviseUnitCm(nsc_prior);	// 合併１５秒単位CM処理
		pdata->setScpArext(nsc_prior, SCP_AREXT_L_EC);
	}
	return true;
}


//---------------------------------------------------------------------
// AutoAdd TR 予告追加処理
// 出力：
//  返り値  : カット処理実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoAddTR(RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_c_exe      = getAutoParam(ParamAuto::c_exe);
	int prm_limit      = getAutoParam(ParamAuto::v_limit);
	int prm_c_limtrsum = getAutoParam(ParamAuto::c_limtrsum);
	int prm_trsumprd   = getAutoParam(ParamAuto::v_trsumprd);

	//--- 実行有無確認 ---
	if (prm_c_exe == 0){
		return false;
	}

	//--- limit確認、予告・番組提供有無確認 ---
	AddExistInfo exist_info;
	int cnt_limit = subAddGetLimit(exist_info, CmdAutoType::AddTR, autoscope);
	//--- limit上限確認、上限以上であれば何もせず終了 ---
	if (prm_limit != 0 && cnt_limit >= prm_limit){
		return false;
	}
	//--- 予告秒数が指定以上あれば何もせず終了 ---
	if (prm_c_limtrsum > 0 && exist_info.sec_tr >= prm_trsumprd){
		return false;
	}

	//--- 優先順位最大のシーンチェンジ位置確認（prm_limit == 0の時は書き換えも実行） ---
	Nsc nsc_prior = subAddSearch(CmdAutoType::AddTR, exist_info, autoscope);
	if (nsc_prior < 0){
		return false;
	}
	//--- 予告検出なしの検出 ---
	if (nsc_prior <= 0 && prm_limit > 0){
		return false;
	}

	//--- 一番優先順位の高い候補を予告とする ---
	if (nsc_prior > 0 && prm_limit > 0){
		// 予告を設定
		subAddReviseUnitCm(nsc_prior);	// 合併１５秒単位CM処理
		pdata->setScpArext(nsc_prior, SCP_AREXT_L_TRKEEP);
	}
	return true;
}

//---------------------------------------------------------------------
// 既存構成を検索
// 出力：
//  返り値  : 検出した目的配置の合計
//  exist_info:
//    trailer  : 予告有無確認 false=なし true=あり
//    sponsor  : 番組提供有無確認 false=なし true=あり
//    sec_tr   : 予告認識秒数
//    devideCm : CM構成内分割（false=なし true=あり）
//---------------------------------------------------------------------
int JlsAutoScript::subAddGetLimit(AddExistInfo &exist_info, CmdAutoType cmdtype, RangeMsec autoscope){
	//--- コマンドに対応する構成取得 ---
	ScpArExtType arext_dst;
	if (cmdtype == CmdAutoType::AddSP){			// 番組提供
		arext_dst = SCP_AREXT_L_SP;
	}
	else if (cmdtype == CmdAutoType::AddEC){		// エンドカード
		arext_dst = SCP_AREXT_L_EC;
	}
	else{
		arext_dst = SCP_AREXT_L_TRKEEP;
	}

	//--- limit確認、予告・番組提供有無確認 ---
	int  cnt_limit = 0;
	bool flag_trailer = false;
	bool flag_sponsor = false;
	Sec  totalsec_tr = 0;
	{
		Term term = {};
		bool cont = true;
		while(cont){
			cont = getTermNext(term);
			if (cont){
				ScpArExtType arext_term = getScpArext(term);
				if (checkScopeTerm(term, autoscope)){
					if (arext_term == arext_dst){			// 目的存在
						cnt_limit ++;
					}
					if (arext_term == SCP_AREXT_L_TRKEEP){	// 予告存在
						flag_trailer = true;
						// 予告秒数取得
						Sec tmpsec = pdata->cnv.getSecFromMsec(term.msec.ed - term.msec.st);
						totalsec_tr += tmpsec;
					}
					if (arext_term == SCP_AREXT_L_SP){		// 番組提供存在
						flag_sponsor = true;
					}
				}
			}
		}
	}

	//--- 予告検出なしの検出後は番組提供について合併15秒構成を分解して認識させる ---
	bool divide_cm = false;
	{
		if (pdata->recHold.msecTrPoint == autoscope.st && autoscope.st > 0){
			if (cmdtype == CmdAutoType::AddSP){		// 番組提供
				divide_cm = true;
			}
		}
		if (flag_trailer == false && cmdtype == CmdAutoType::AddTR){	// 予告なし予告検出時も同様
			divide_cm = true;
		}
	}

	exist_info.trailer   = flag_trailer;
	exist_info.sponsor   = flag_sponsor;
	exist_info.sec_tr    = totalsec_tr;
	exist_info.divideCm  = divide_cm;

	return cnt_limit;
}

//---------------------------------------------------------------------
// 優先順位最大の位置を検索
// limitない時は書き換えも実行
// 入力：
//   exist_info : trailer,sponsor,divideCmを使用
// 出力：
//  返り値  : 優先順位最大のシーンチェンジ番号
//---------------------------------------------------------------------
Nsc JlsAutoScript::subAddSearch(CmdAutoType cmdtype, AddExistInfo exist_info, RangeMsec autoscope){
	//--- 使用パラメータ ---
	int prm_secprev   = getAutoParam(ParamAuto::v_secprev);
	int prm_secnext   = getAutoParam(ParamAuto::v_secnext);
	int prm_limit     = getAutoParam(ParamAuto::v_limit  );
	int prm_c_limloc  = getAutoParam(ParamAuto::c_limloc );
	int prm_c_cutskip = getAutoParam(ParamAuto::c_cutskip);
	int prm_c_wmin    = getAutoParam(ParamAuto::c_wmin);
	int prm_c_wmax    = getAutoParam(ParamAuto::c_wmax);
	int prm_c_w15     = getAutoParam(ParamAuto::c_w15);
	int prm_c_wdefmin = getAutoParam(ParamAuto::c_wdefmin);
	int prm_c_wdefmax = getAutoParam(ParamAuto::c_wdefmax);

	//--- コマンドに対応する構成取得 ---
	ScpArExtType arext_dst;
	switch(cmdtype){
		case CmdAutoType::AddSP:				// 番組提供
			arext_dst = SCP_AREXT_L_SP;
			break;
		case CmdAutoType::AddEC:				// エンドカード
			arext_dst = SCP_AREXT_L_EC;
			break;
		default:
			arext_dst = SCP_AREXT_L_TRKEEP;
			break;
	}

	//--- 各シーンチェンジ位置確認 ---
	Nsc nsc_prior     = -1;
	Nsc nsc_sub_prior = -1;
	{
		int total_prior     = 0;
		int total_sub_prior = 0;
		Term term = {};
		bool cont = getTermNext(term);
		while(cont){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			//--- 範囲内の場合 ---
			if (checkScopeTerm(term, autoscope)){
				AddLocInfo locinfo;
				//--- 指定位置の前後状態を取得 ---
				subAddGetLocInfo(locinfo, term, autoscope);
//if (cmdtype == CmdAutoType::AddSP){
//printf("(%d,%d,%d:(%d,%d),(%d,%d),(%d,%d),%d)", term.nsc.st,term.nsc.ed,arstat_term,
//	locinfo.typeLogo, locinfo.secDifLogo, locinfo.typeTr, locinfo.secDifTr,
//	locinfo.typeSp, locinfo.secDifSp, locinfo.secDifSc);
//}
				//--- 状態からの優先順位設定 ---
				int prior1 = subAddGetPriority(locinfo, cmdtype, arstat_term, arext_term, exist_info);
				//--- 次の構成時間に制約ある場合のチェック ---
				if (prm_secnext > 0){
					if (subAddCheckSec(term.nsc.ed, prm_secnext) == false){
						prior1 = 0;
					}
				}
				//--- 前の構成時間に制約ある場合のチェック ---
				if (prm_secprev > 0){
					if (subAddCheckSec(term.nsc.st, -1 * prm_secprev) == false){
						prior1 = 0;
					}
				}
				//--- 候補限定用 ---
				int sub_prior1 = prior1;
				//--- 内容期間による選択 ---
				int secdif_sc = locinfo.secDifSc;
				if ((secdif_sc >= prm_c_wmin && secdif_sc <= prm_c_wmax) ||
					(secdif_sc == 15 && prm_c_w15 > 0)){	//指定範囲内
				}
				else if (secdif_sc >= prm_c_wdefmin && secdif_sc <= prm_c_wdefmax){
					prior1 = 0;		// 指定範囲外であるが標準範囲内
				}
				else{				// 期間が範囲外
					prior1 = 0;
					sub_prior1 = 0;
				}
				//--- 最優先位置の更新 ---
				if (total_prior < prior1){
					total_prior = prior1;
					nsc_prior = term.nsc.ed;
				}
				if (total_sub_prior < sub_prior1){
					total_sub_prior = sub_prior1;
					nsc_sub_prior = term.nsc.ed;
				}
				//--- limitない場合は毎回書き換え ---
				if (prior1 > 0 && prm_limit == 0){
					subAddReviseUnitCm(term.nsc.ed);	// 合併１５秒単位CM処理
					pdata->setScpArext(term.nsc.ed, arext_dst);
				}
			}
			cont = getTermNext(term);
		}
	}
	//--- 標準期間の候補位置のみに候補を限定する処理 ---
	if (prm_c_limloc > 0){
		if (nsc_prior >= 0 && nsc_sub_prior != nsc_prior){
			nsc_prior = -1;
		}
	}
	//--- 追加位置より前の予告等カットは無効化する処理 ---
	if (nsc_prior > 0 && prm_c_cutskip > 0){
		subAddCancelCut(nsc_prior, autoscope);
	}

	return nsc_prior;
}

//---------------------------------------------------------------------
// AutoAddで判断する指定位置の前後状態から優先順位算出元データ取得
// 出力：
//  locinfo
//    typeLogo       : ロゴからの状態（0-9:ロゴなし 10-19:ロゴあり +100:ロゴ開始側）
//    secDifLogo     : ロゴからの時間
//    typeTr         : 予告からの状態（0:該当なし 1:後側 2:前側 3:最初の予告位置 4:予告間隙間）
//    secDifTr       : 予告からの時間
//    typeSp         : 番組提供からの状態（0:該当なし 1:後側 2:前側 3:内部）
//    secDifSp       : エンドカードからの時間（前側の時のみ計測）
//    typeEc         : エンドカードからの状態（0:該当なし 1:後側 2:前側）
//    secDifEc       : 番組提供からの時間
//    typeNolast     : 予告ではなく後に内容がある場合=1
//    typeEndlogo    : 範囲内に完全に含まれる最後のロゴからの状態（0:該当ロゴなし 1:ロゴ以降 2:ロゴ手前側）
//    secDifSc       : １つ前の番組構成からの時間
//---------------------------------------------------------------------
void JlsAutoScript::subAddGetLocInfo(AddLocInfo &locinfo, Term target, RangeMsec autoscope){
	//--- 設定値 ---
	int prm_msec_wcomp_trmax = getConfig(ConfigVarType::msecWCompTRMax);

	//--- 使用パラメータ ---
	int prm_c_cutskip = getAutoParam(ParamAuto::c_cutskip);
	int prm_c_lgprev  = getAutoParam(ParamAuto::c_lgprev);
	int prm_c_lgpost  = getAutoParam(ParamAuto::c_lgpost);

	//--- ロゴ位置を取得 ---
	bool logomode = false;				// ターゲット位置のロゴ状態
	RangeNsc rnsc_logo = {-1, -1};		// ターゲット位置前後のロゴ位置
	Nsc  nsc_endrise = -1;				// 範囲内に全体が含まれる最後のロゴ開始位置
	{
		ElgCurrent elg = {};
		bool cont = true;
		bool want_logo = true;
		while(cont){
			cont = getElgNextKeep(elg);
			//--- 対象ロゴに一番近い立ち上がり／立ち下がり位置を取得 ---
			if (want_logo){
				if (elg.nscLastFall <= target.nsc.st && target.nsc.st < elg.nscRise){
					want_logo = false;
					logomode = false;
					rnsc_logo = {elg.nscLastFall, elg.nscRise};
					if (elg.nscLastFall == 0){
						rnsc_logo.st = -1;
					}
				}
				else if (elg.nscRise <= target.nsc.st && target.nsc.st < elg.nscFall){
					want_logo = false;
					logomode = true;
					rnsc_logo = {elg.nscRise, elg.nscFall};
				}
				else if (!cont){
					want_logo = false;
					logomode = false;
					rnsc_logo = {elg.nscFall, -1};
				}
			}
			//--- 範囲内に全体が含まれる最後のロゴ位置を取得 ---
			RangeMsec range_logo = {elg.msecRise, elg.msecFall};
			if (checkScopeRange(range_logo, autoscope) && cont){
				Term term = {};
				term.ini = elg.nscRise;
				bool nextflag = getTermNext(term);
				while(nextflag){
					if (term.nsc.ed > elg.nscFall){
						nextflag = false;
					}
					else{
						ScpArExtType arext_term = getScpArext(term);
						if (arext_term != SCP_AREXT_L_LGCUT){
							nextflag = false;
							nsc_endrise = term.nsc.st;		// 設定位置
						}
					}
					if (nextflag){
						nextflag = getTermNext(term);
					}
				}
			}
		}
	}

	//--- 構成位置を取得 ---
	RangeNsc rnsc_tr_1st  = {-1, -1};
	RangeNsc rnsc_tr_2nd  = {-1, -1};
	RangeNsc rnsc_tr_stop = {-1, -1};
	RangeNsc rnsc_tr_near = {-1, -1};
	RangeNsc rnsc_sp      = {-1, -1};
	RangeNsc rnsc_ec      = {-1, -1};
	bool type_nolast = 0;
	bool flag_invalid_all = false;
	{
		Term term = {};
		Msec msec_difmin_tr = 0;
		ScpArExtType arext_hold = SCP_AREXT_NONE;
		bool cont = getTermNext(term);
		while(cont){
			//--- 検索範囲内の処理 ---
			if (checkScopeTerm(term, autoscope)){
				ScpArType arstat_term = getScpArstat(term);
				ScpArExtType arext_term = getScpArext(term);
				//--- 対象位置との距離 ---
				Msec msec_dif_target = 0;
				if (term.nsc.ed <= target.nsc.st){
					msec_dif_target = abs(target.msec.st - term.msec.ed);
				}
				else if (term.nsc.st >= target.nsc.ed){
					msec_dif_target = abs(term.msec.st - target.msec.ed);
				}
				//--- 境界部分だった場合の補正 ---
				if (arstat_term == SCP_AR_B_OTHER){
					if (arext_hold == SCP_AREXT_L_TRKEEP){
						arext_term = arext_hold;
					}
				}
				else{
					arext_hold = arext_term;
				}
				//--- 予告部分 ---
				if (arext_term == SCP_AREXT_L_TRKEEP){
					if (rnsc_tr_1st.st < 0){					// 一番最初
						rnsc_tr_1st  = term.nsc;
					}
					if (msec_difmin_tr > msec_dif_target || rnsc_tr_near.st < 0){	// 一番近い
						rnsc_tr_near = term.nsc;
						msec_difmin_tr = msec_dif_target;
					}
					if (rnsc_tr_stop.st >= 0 && term.nsc.ed <= target.nsc.st){	// 予告終了位置変更
						rnsc_tr_stop = {-1, -1};
						rnsc_tr_2nd  = {-1, -1};
					}
					if (rnsc_tr_stop.st >= 0 && rnsc_tr_2nd.st < 0){	// 一度予告終了後再開
						rnsc_tr_2nd = term.nsc;
					}
				}
				else if (rnsc_tr_1st.st >= 0 && rnsc_tr_stop.st < 0){	// 予告終了位置
					rnsc_tr_stop = term.nsc;
				}
				//--- 番組提供部分 ---
				if (arext_term == SCP_AREXT_L_SP){
					if (rnsc_sp.st < 0){				// 一番最初
						rnsc_sp = term.nsc;
					}
				}
				//--- エンドカード部分 ---
				if (arext_term == SCP_AREXT_L_EC){
					if (rnsc_ec.st < 0){				// 一番最初
						rnsc_ec = term.nsc;
					}
				}
				//--- 予告・番組提供有効範囲チェック ---
				if (term.msec.ed - term.msec.st > prm_msec_wcomp_trmax + pdata->msecValSpc){
					if (term.nsc.st >= target.nsc.ed){				// 現在地点より後に予告を超える構成
						if (jlsd::isScpArTypeLogo(arstat_term)){	// ロゴ内
							if (arext_term != SCP_AREXT_L_LGCUT){	// ロゴカット部分以外
								type_nolast = 1;					// 本編がまだ後に存在
							}
						}
					}
				}
				//--- 予告・番組提供無効化チェック ---
				if (arext_term == SCP_AREXT_L_TRCUT ||
					arext_term == SCP_AREXT_L_ECCUT){
					if (term.nsc.ed <= target.nsc.st){			// 予告カットが手前に存在
						if (prm_c_cutskip == 0){				// カット以降は無効の時
							flag_invalid_all = true;
						}
					}
				}
			}
			cont = getTermNext(term);
		}
	}

	//--- ロゴ隣接状態検出 ---
	Sec sec_dif_logo = 0;
	int type_logo = 0;						// ロゴ検出なし
	if (flag_invalid_all == false){
		int msec_dif_tmp1 = 600 * 1000;		// 未検出時の最大フレーム数を設定
		int msec_dif_tmp2 = 600 * 1000;		// 未検出時の最大フレーム数を設定
		if (rnsc_logo.st >= 0){					// ロゴ切り替わり前側からの距離
			// ロゴ開始側・終了側を候補から外す時の処理
			if ((prm_c_lgprev == 0 && prm_c_lgpost != 0 && logomode == true) ||
				(prm_c_lgprev != 0 && prm_c_lgpost == 0 && logomode == false)){
			}
			else{
				msec_dif_tmp1 = target.msec.st - pdata->getMsecScp(rnsc_logo.st);
			}
		}
		if (rnsc_logo.ed >= 0){					// ロゴ切り替わり後側からの距離
			// ロゴ開始側・終了側を候補から外す時の処理
			if ((prm_c_lgprev != 0 && prm_c_lgpost == 0 && logomode == true) ||
				(prm_c_lgprev == 0 && prm_c_lgpost != 0 && logomode == false)){
			}
			else{
				msec_dif_tmp2 = pdata->getMsecScp(rnsc_logo.ed) - target.msec.ed;
			}
		}
		int msec_dif_tmp;
		int type_logo_add = 0;
		int type_logo_pside = 0;
		if (msec_dif_tmp1 <= msec_dif_tmp2){	// 前側が現位置に近い
			msec_dif_tmp = msec_dif_tmp1;
			if (logomode == true){				// 開始時ロゴあり
				type_logo_add = 100;
			}
		}
		else{									// 後側が現位置に近い
			type_logo_pside = 1000;
			msec_dif_tmp = msec_dif_tmp2;
			if (logomode == false){				// 開始時ロゴなし＝終了側がロゴあり
				type_logo_add = 100;
			}
		}
		sec_dif_logo = pdata->cnv.getSecFromMsec(msec_dif_tmp);		// 秒数に変換
		if (logomode){			// ロゴ内部
			if (sec_dif_logo >= 300){
				type_logo = 10;			// ロゴ内部で５分以上離れている
			}
			else if (sec_dif_logo >= 31){
				type_logo = 12;			// ロゴ内部で31秒以上離れている
			}
			else if (sec_dif_logo >= 15){
				type_logo = 13;			// ロゴ内部で15秒以上離れている
			}
			else if (sec_dif_logo > 1){
				type_logo = 15;			// ロゴ内部で近いが隣接ではない
			}
			else{
				type_logo = 16;			// ロゴ内部で隣接
			}
		}
		else{							// ロゴ外部
			if (sec_dif_logo >= 300){
				type_logo = 0;			// ロゴ外部で５分以上離れている
			}
			else if (sec_dif_logo >= 15){
				type_logo = 3;			// ロゴ外部で15秒以上離れている
			}
			else if (sec_dif_logo > 1){
				type_logo = 4;			// ロゴ外部で近いが隣接ではない
			}
			else{
				type_logo = 7;			// ロゴ外部で隣接
			}
		}
		type_logo += type_logo_add;
		type_logo += type_logo_pside;
	}

	//--- 予告からの時間 ---
	int sec_dif_tr, type_tr;
	{
		if (rnsc_tr_near.st < 0){				// 予告が見つからない場合
			sec_dif_tr = 0;
			type_tr = 0;
		}
		else if (rnsc_tr_1st.st == target.nsc.ed){	// 現位置が予告開始に隣接
			sec_dif_tr = 0;
			type_tr = 2;
		}
		else if (rnsc_tr_1st.ed == target.nsc.ed){	// 現位置が予告開始の最初の位置
			sec_dif_tr = 0;
			type_tr = 3;
		}
		else if (rnsc_tr_2nd.st >= 0 && rnsc_tr_stop.st >= 0 &&
				 rnsc_tr_stop.ed == target.nsc.ed){		// 現位置が予告隙間
			sec_dif_tr = 0;
			type_tr = 4;
		}
		else if (rnsc_tr_near.st >= target.nsc.ed){		// 現位置が予告より前
			sec_dif_tr = pdata->cnv.getSecFromMsec(pdata->getMsecScp(rnsc_tr_near.st) - target.msec.ed);
			type_tr = 2;
		}
		else{								// 現位置が予告後
			sec_dif_tr = pdata->cnv.getSecFromMsec(target.msec.st - pdata->getMsecScp(rnsc_tr_near.ed));
			type_tr = 1;
		}
	}
	//--- 番組提供からの時間 ---
	int sec_dif_sp, type_sp;
	{
		if (rnsc_sp.st < 0){
			sec_dif_sp = 0;
			type_sp = 0;
		}
		else if (rnsc_sp.st < target.nsc.ed && rnsc_sp.ed >= target.nsc.ed){	// 現位置が番組提供と同一
			sec_dif_sp = 0;
			type_sp = 3;
		}
		else if (rnsc_sp.st >= target.nsc.ed){		// 現位置が番組提供手前
			sec_dif_sp = pdata->cnv.getSecFromMsec(pdata->getMsecScp(rnsc_sp.st) - target.msec.ed);
			type_sp = 2;
		}
		else{								// 現位置が番組提供後
			sec_dif_sp = pdata->cnv.getSecFromMsec(target.msec.st - pdata->getMsecScp(rnsc_sp.ed));
			type_sp = 1;
		}
	}
	//--- エンドカード手前の時間 ---
	int sec_dif_ec, type_ec;
	{
		if (rnsc_ec.st < 0){
			sec_dif_ec = 0;
			type_ec = 0;
		}
		else if (rnsc_ec.st >= target.nsc.ed){		// 現位置がエンドカード手前
			sec_dif_ec = pdata->cnv.getSecFromMsec(pdata->getMsecScp(rnsc_ec.st) - target.msec.ed);
			type_ec = 2;
		}
		else{								// 現位置がエンドカード後
			sec_dif_ec = 0;					// 時間チェックはしていない
			type_ec = 1;
		}
	}
	//--- 単位間隔 ---
	int sec_dif_sc = pdata->cnv.getSecFromMsec(target.msec.ed - target.msec.st);

	//--- 最後のロゴからの状態 ---
	int type_endlogo;
	{
		if (nsc_endrise < 0){
			type_endlogo = 0;
		}
		else if (target.msec.st < nsc_endrise){
			type_endlogo = 2;
		}
		else{
			type_endlogo = 1;
		}
	}

	//--- 返り値代入 ---
	locinfo.secDifLogo    = sec_dif_logo;
	locinfo.typeLogo      = type_logo;
	locinfo.secDifTr      = sec_dif_tr;
	locinfo.typeTr        = type_tr;
	locinfo.secDifSp      = sec_dif_sp;
	locinfo.typeSp        = type_sp;
	locinfo.secDifEc      = sec_dif_ec;
	locinfo.typeEc        = type_ec;
	locinfo.secDifSc      = sec_dif_sc;
	locinfo.typeNolast    = type_nolast;
	locinfo.typeEndlogo   = type_endlogo;
}


//---------------------------------------------------------------------
// 追加候補として指定箇所の優先順位を算出
// 入力：
//	locinfo				// 位置情報
//	cmdtype				// コマンド種類
//	arstat_cur			// 対象の配置状態
//	arext_cur			// 対象の配置拡張状態
//  exist_info
//	  trailer			// true:予告検出有効
//	  sponsor			// true:番組提供有効
//	  divideCm			// true:CM構成内分割する
// 出力：
//  返り値  : 入力配置の優先順位
//---------------------------------------------------------------------
int JlsAutoScript::subAddGetPriority(AddLocInfo &locinfo, CmdAutoType cmdtype, ScpArType arstat_cur, ScpArExtType arext_cur, AddExistInfo exist_info){
	//--- 使用パラメータ ---
	int prm_c_lgprev  = getAutoParam(ParamAuto::c_lgprev);
	int prm_c_lgpost  = getAutoParam(ParamAuto::c_lgpost);
	int prm_c_lgintr  = getAutoParam(ParamAuto::c_lgintr);
	int prm_c_lgy     = getAutoParam(ParamAuto::c_lgy);
	int prm_c_lgn     = getAutoParam(ParamAuto::c_lgn);
	int prm_c_lgbn    = getAutoParam(ParamAuto::c_lgbn);
	int prm_c_search  = getAutoParam(ParamAuto::c_search);
	int prm_scope     = getAutoParam(ParamAuto::v_scope);
	int prm_scopen    = getAutoParam(ParamAuto::v_scopen);
	int prm_c_lgsp    = getAutoParam(ParamAuto::c_lgsp);
	int prm_c_in1     = getAutoParam(ParamAuto::c_in1);
	int prm_c_chklast = getAutoParam(ParamAuto::c_chklast);
	int prm_c_unitcmoff = getAutoParam(ParamAuto::c_unitcmoff);
	int prm_c_unitcmon  = getAutoParam(ParamAuto::c_unitcmon);

	//--- type_logoを分類 ---
	int type_logo_prior = locinfo.typeLogo % 10;				// 優先順位
	int type_logo_in    = (locinfo.typeLogo / 10) % 10;			// ロゴ内部
	int type_logo_prev  = (locinfo.typeLogo / 100) % 10;		// ロゴ手前
	int type_logo_pside = (locinfo.typeLogo / 1000) % 10;		// ロゴ前側
	// 両隣も含めロゴなし
	int type_logo_nonbr = ((type_logo_in == 0) && (type_logo_prior <= 5))? 1 : 0;

	//--- 予告と番組提供の位置関係から無効箇所を検出 ---
	bool invalid_pos = false;
	bool invalid_sp = false;
	bool invalid_tr = false;
	int type_tr = locinfo.typeTr;
	int type_sp = locinfo.typeSp;
	int type_ec = locinfo.typeEc;
	Sec secdif_tr = locinfo.secDifTr;
	Sec secdif_sp = locinfo.secDifSp;
	Sec secdif_ec = locinfo.secDifEc;
	Sec secdif_logo = locinfo.secDifLogo;
	if (exist_info.sponsor && exist_info.trailer){
		if (prm_c_lgprev == 0){					// 前側を無効化
			if (type_sp == 2 || type_tr == 2){
				invalid_pos = true;
			}
		}
		if (prm_c_lgpost == 0){					// 後側を無効化
			if (type_sp == 1 || type_tr == 1){
				invalid_pos = true;
			}
		}
		if (prm_c_lgintr > 0){					// 中間を残す
			if (type_tr == 1 && (type_sp == 2 || type_sp == 0)){
				invalid_pos = false;
			}
			else if (type_tr == 2 && type_sp == 1){
				invalid_pos = false;
			}
			else if (type_tr == 4){
				invalid_pos = false;
			}
		}
		if (invalid_pos == false){					// 予告・番組提供ともに有効な場合
			if ((type_sp == 1 && type_tr == 1) ||	// 両方後側の場合
				(type_sp == 2 && type_tr == 2)){	// 両方前側の場合
				if (secdif_sp > secdif_tr){			// 離れていたら近い側を残す
					invalid_sp = true;
				}
				else if (secdif_sp < secdif_tr){	// 離れていたら近い側を残す
					invalid_tr = true;
				}
			}
		}
	}
	else if (exist_info.sponsor){
		if (prm_c_lgprev == 0){					// 前側を無効化
			if (type_sp == 2){
				invalid_pos = true;
			}
		}
		if (prm_c_lgpost == 0){					// 後側を無効化
			if (type_sp == 1){
				invalid_pos = true;
			}
		}
	}
	else if (exist_info.trailer){
		if (prm_c_lgprev == 0){					// 前側を無効化
			if (type_tr == 2){
				invalid_pos = true;
			}
		}
		if (prm_c_lgpost == 0){					// 後側を無効化
			if (type_tr == 1){
				invalid_pos = true;
			}
		}
	}
	//--- 検索範囲による無効設定 ---
	// ロゴなしのみ検索でロゴあり領域、またはその逆の場合は無効
	if ((prm_c_lgy == 0 && type_logo_in != 0) ||
		(prm_c_lgn == 0 && type_logo_in == 0)){
		invalid_pos = true;
	}
	// 両隣も含めてロゴなし検索で条件を満たさない場合は無効
	else if (prm_c_lgbn > 0 && type_logo_nonbr == 0){
		invalid_pos = true;
	}
	//--- 直後が番組提供・エンドカード時のみとする場合に条件を満たさない場合は無効 ---
	bool is_next_sp = (exist_info.sponsor && type_sp == 2 && secdif_sp == 0)? true : false;
	bool is_next_ec = (type_ec == 2 && secdif_ec == 0)? true : false;
	if (prm_c_lgsp != 0 && is_next_sp == false && is_next_ec == false){
		invalid_pos = true;
	}

	//--- 検索範囲起点をロゴ検出にするか判断 ---
	bool valid_onlylogo = false;
	if (prm_c_search >= 4 && prm_c_search <= 6){
		valid_onlylogo = true;
	}
	else if (prm_c_search >= 1 && prm_c_search <= 3){
		if (exist_info.trailer == false &&
			(exist_info.sponsor == false || cmdtype == CmdAutoType::AddTR || cmdtype == CmdAutoType::AddSP)){
			valid_onlylogo = true;
		}
	}
	//--- 検索範囲による優先順位設定 ---
	int prior1 = 0;
	//--- ロゴからの距離判断 ---
	if (invalid_pos == false){
		// 前側検索なしで前側、後側検索なしで後側の場合は無効
		if ((prm_c_lgprev == 0 && type_logo_prev != 0) ||
				 (prm_c_lgpost == 0 && type_logo_prev == 0)){
			prior1 = 0;
		}
		// ロゴ切り替わり位置から
		else if (type_logo_prior > 0){
			if ( (prm_c_search == 0) ||
				((prm_c_search == 1 || prm_c_search == 4) && (secdif_logo <= 1)) ||
				((prm_c_search == 2 || prm_c_search == 5) &&
				    (secdif_logo >= prm_scopen && secdif_logo <= prm_scope)) ||
				((prm_c_search == 3 || prm_c_search == 6) && (secdif_logo == prm_scope))){
				if ((prm_c_search == 2) &&
					(type_logo_pside != 0 && secdif_logo != 0)){	// 手前側は隣接に限定する場合
					prior1 = 0;
				}
				else{
					prior1 = type_logo_prior;
				}
			}
		}
	}
	//--- 予告・番組提供・エンドカードからの距離判断 ---
	bool valid_in1 = false;		// 予告と認識した最初の内容を番組提供部分にする許可フラグ
	if (invalid_pos == false && valid_onlylogo == false){
		// 予告・番組提供がある場合はロゴ端からの優先順位は（ロゴなしで隣接）以外無効にする
		int prior1bak = prior1;
		if (prior1 > 0 && prior1 < 7){
			prior1bak = 0;
		}
		// ロゴ優先順位は外し改めて優先順位を検索
		prior1 = 0;
		// 番組提供がある場合の処理
		if (exist_info.sponsor){
			if (type_sp == 0){
			}
			// 前側検索なしで前側、後側検索なしで後側の場合は無効
			else if (invalid_pos){
			}
			// 番組提供は無効の場合
			else if (invalid_sp){
			}
			// 探索範囲内
			else if ( (prm_c_search == 0) ||
					 ((prm_c_search == 1) && (secdif_sp <= 1)) ||
					 ((prm_c_search == 2) &&
					      (secdif_sp >= prm_scopen && secdif_sp <= prm_scope)) ||
					 ((prm_c_search == 3) && (secdif_sp == prm_scope))){
				// 番組提供より前
				if (type_sp == 2){
					// 隣接のみ優先順位を上げる
					if (secdif_sp <= 1){
						prior1 += 20;
					}
					// 手前側は隣接に限定する場合
					else if (prm_c_search == 2){
						prior1 = 0;
					}
				}
				else if (type_sp == 1){
					// 番組提供直後に隣接
					if (secdif_sp <= 1){
						prior1 += 130;
					}
					// 番組提供から指定時間以内
					else{
						prior1 += 30;
					}
				}
			}
		}
		if (exist_info.trailer){
			// 予告が現地点に近くない場合は無効
			if (type_tr == 0){
			}
			// 前側検索なしで前側、後側検索なしで後側の場合は無効
			else if (invalid_pos){
			}
			// 予告は無効の場合
			else if (invalid_tr){
			}
			// 探索範囲内
			else if (((prm_c_search == 1) && (secdif_tr <= 1)) ||
					 ((prm_c_search == 2) && (secdif_tr <= prm_scope)) ||
					 ((prm_c_search == 3) && (secdif_tr == prm_scope))){
				// 予告開始前
				if (type_tr == 2){
					// 隣接のみ優先順位を上げる
					if (secdif_tr == 0){
						prior1 += 20;
						// ロゴなしだった場合は優先度を上げる
						if (type_logo_in == 0){
							prior1 += 15;
						}
					}
					// 手前側は隣接に限定する場合
					else if (prm_c_search == 2){
						prior1 = 0;
					}
				}
				else if (type_tr == 1){
					// 予告直後に隣接
					if (secdif_tr <= 1){
						prior1 += 30;
						// ロゴなしだった場合は優先度を上げる
						if (type_logo_in == 0){
							prior1 += 15;
						}
					}
					// 予告から指定時間以内
					else{
						prior1 += 20;
					}
				}
				else if (type_tr == 4){		// 予告間隙間
						prior1 += 40;
				}
				else if (type_tr == 3){		// 最初の予告位置
					// type_tr == 3 は現状維持するが、番組提供に置換判断を許可
					valid_in1 = true;
				}
			}
		}
		// 有効であれば残したロゴ優先順位も追加
		if (prior1 > 0){
			prior1 += prior1bak;
		}
	}
	//--- 本編内容前のロゴなし領域に予告・エンドカード位置がないか確認 ---
	if (prm_c_chklast > 0 && locinfo.typeNolast > 0){
		if (type_logo_in == 0){
			prior1 = 0;
		}
	}
	//--- 予告が見つからず、最後のロゴより前で前側は候補を外す場合の確認 ---
	if (exist_info.trailer == false && locinfo.typeEndlogo == 2 && prm_c_lgprev == 0){
		prior1 = 0;
	}
	//--- 現状態による選択 ---
	//--- 予告と認識した最初の内容を番組提供部分とする場合は特殊処理 ---
	if (arext_cur == SCP_AREXT_L_TRKEEP && type_tr == 3 && prm_c_in1 > 0 && valid_in1){
		if (prm_c_in1 == 2){		// 優先度上げる
			prior1 += 500;
		}
		if (prm_c_in1 == 1){		// 優先度上げる
			prior1 += 50;
		}
		else{						// 優先度最小
			prior1 = 1;
		}
	}
	else if (prior1 > 0){
		if (arext_cur == SCP_AREXT_L_TRKEEP){
			prior1 = 0;
		}
		else if (arext_cur == SCP_AREXT_L_EC ||
				 arext_cur == SCP_AREXT_L_SP){
				prior1 = 0;
		}
		else if (arext_cur == SCP_AREXT_L_LGCUT ||
				 arext_cur == SCP_AREXT_L_LGADD ||
				 arext_cur == SCP_AREXT_N_LGCUT ||
				 arext_cur == SCP_AREXT_N_LGADD){
				prior1 = 0;
		}
		else if ((arstat_cur == SCP_AR_N_AUNIT) ||
				 (arstat_cur == SCP_AR_N_BUNIT)){
			// 合併１５秒単位CMの無効化処理
			if (prm_c_unitcmoff > 0){			// 合併１５秒単位CMは強制的にCM以外とする
			}
			else if (prm_c_unitcmon > 0){		// 合併１５秒単位CMは強制的にCMとする
				prior1 = 0;
			}
			else if (type_tr != 1){				// 予告後の認識期間以外
				if (exist_info.divideCm == false){	// 分割しない場合は無効でCMから変更しない
					prior1 = 0;
				}
				else{							// 分割する場合は優先度最低で保持
					prior1 = 1;
				}
			}
		}
		else if (jlsd::isScpArTypeBorder(arstat_cur)){
			if (cmdtype == CmdAutoType::AddSP){	// 番組提供のみ優先度アップ
				prior1 += 200;
			}
		}
	}
	return prior1;
}


//---------------------------------------------------------------------
// 次の構成までの時間条件に合致するかチェック
// 出力：
//  返り値  : false=指定秒の構成なし true=指定秒の構成あり
//---------------------------------------------------------------------
bool JlsAutoScript::subAddCheckSec(Nsc nsc_cur, Sec difsec){
	bool ret = false;
	SearchDirType dr = SEARCH_DIR_NEXT;
	if (difsec < 0){
		dr = SEARCH_DIR_PREV;
	}
	Term term = {};
	term.ini = nsc_cur;
	bool cont = true;
	while(cont){
		if (dr == SEARCH_DIR_NEXT){
			cont = getTermNext(term);
		}
		else{
			cont = getTermPrev(term);
		}
		if (cont){
			Sec sec_dif_term = pdata->cnv.getSecFromMsec(term.msec.ed - term.msec.st);
			if (sec_dif_term == difsec){
				ret = true;
			}
			if (sec_dif_term >= difsec){
				cont = false;
			}
		}
	}
	return ret;
}


//---------------------------------------------------------------------
// 追加位置より前の予告等カットは無効化する処理
// 出力：
//   データ(arext) : 配置状態を変更
//---------------------------------------------------------------------
void JlsAutoScript::subAddCancelCut(Nsc nsc_prior, RangeMsec autoscope){
	if (nsc_prior > 0){
		//--- 追加位置までの検索 ---
		Term term = {};
		bool cont = getTermNext(term);
		while(cont && term.nsc.ed < nsc_prior){
			//--- 範囲内の場合 ---
			ScpArExtType arext_term = getScpArext(term);
			if (checkScopeTerm(term, autoscope)){
				//--- カット処理を無効化 ---
				if (arext_term == SCP_AREXT_L_TRCUT){
					setScpArext(term, SCP_AREXT_L_TRKEEP);
				}
				else if (arext_term == SCP_AREXT_L_ECCUT){
					setScpArext(term, SCP_AREXT_L_TRRAW);
				}
			}
			cont = getTermNext(term);
		}
	}
}


//---------------------------------------------------------------------
// 合併１５秒単位CMの一部を解除する場合の処理
// 出力：
//   データ(arstat,arext) : 合併１５秒単位CM情報を書き換え
//---------------------------------------------------------------------
void JlsAutoScript::subAddReviseUnitCm(Nsc nsc_target){
	int num_scpos = pdata->sizeDataScp();
	if (nsc_target <= 0 || nsc_target >= num_scpos){
		return;
	}
	ScpArType arstat_target = pdata->getScpArstat(nsc_target);
	ScpArType arstat_new = SCP_AR_N_OTHER;
	ScpArExtType arext_new  = SCP_AREXT_NONE;

	//--- 合併１５秒単位CMだった場合の手前側処理 ---
	if ((arstat_target == SCP_AR_N_AUNIT || arstat_target == SCP_AR_N_BUNIT) && nsc_target > 1){
		//--- 手前側解除後の状態を設定 ---
		{
			//--- 手前が予告かチェック ---
			Term term = {};
			term.ini  = nsc_target;
			bool cont = getTermPrev(term);
			ScpArType arstat_term = getScpArstat(term);
			bool flag_unitcont = true;
			//--- CM分割だった場合は分割の手前構成を取得 ---
			while(cont && flag_unitcont && (arstat_term == SCP_AR_N_AUNIT || arstat_term == SCP_AR_N_BUNIT)){
				if (arstat_term == SCP_AR_N_BUNIT){		// 分割先頭の次で終了
					flag_unitcont = false;
				}
				cont = getTermPrev(term);
				arstat_term = getScpArstat(term);
			}
			if (cont){
				ScpArExtType arext_term = getScpArext(term);
				//--- 手前が予告の場合は予告状態を継続させる設定 ---
				if ((arext_term == SCP_AREXT_L_TRKEEP) ||
					(arext_term == SCP_AREXT_L_TRRAW )){
					arext_new = SCP_AREXT_L_TRRAW;		// 手前側解除後の状態を変更
				}
			}
		}
		//--- 解除処理 ---
		{
			Term term = {};
			term.ini  = nsc_target;
			bool cont = getTermPrev(term);
			ScpArType arstat_term = getScpArstat(term);
			//--- 中間位置を解除 ---
			while(cont && arstat_term == SCP_AR_N_AUNIT){
				setScpArstat(term, arstat_new);
				setScpArext(term, arext_new);
				cont = getTermPrev(term);
				arstat_term = getScpArstat(term);
			}
			//--- 先頭位置を解除 ---
			if (arstat_term == SCP_AR_N_BUNIT){
				setScpArstat(term, arstat_new);
				setScpArext(term, arext_new);
			}
		}
	}

	// 合併１５秒単位CMだった場合の後半部分処理
	if ((arstat_target == SCP_AR_N_AUNIT || arstat_target == SCP_AR_N_BUNIT) && nsc_target < num_scpos - 1){
		int keep = 0;
		Msec msec_start = 0;
		ScpArExtType arext_keep;
		bool change = true;
		Term term = {};
		term.ini  = nsc_target;
		bool cont = getTermNext(term);
		while(cont && keep >= 0 && keep <= 2){
			ScpArType arstat_term = getScpArstat(term);
			ScpArExtType arext_term = getScpArext(term);
			if ((arstat_term == SCP_AR_N_AUNIT) ||
				(arstat_term == SCP_AR_N_BUNIT && change) ||
				(arstat_term == SCP_AR_N_OTHER && change)){
				keep ++;
				if (keep == 1){
					change = true;						// 途中状態を設定
					msec_start = term.msec.st;			// 15秒単位開始位置
					arext_keep = arext_term;
				}
				else if (arext_term != arext_keep){		// 構成が変わったら分割設定しない
					change = false;
				}
				if (keep > 1 && change){				// ２回目以降の場合
					Sec sec_dif  = pdata->cnv.getSecFromMsec(term.msec.ed - msec_start);
					// 新たに合併１５秒単位となる場合は設定
					if ((sec_dif % 15) == 0 && sec_dif <= 30){
						Nsc nsc_lastwrite = -1;
						Term subterm = term;
						for(int i=0; i<keep-1; i++){
							Msec msec_prm_gap = (Msec) pdata->getConfig(ConfigVarType::msecMgnCmDivide);
							Msec msec_sub_gapp = calcDifGap(subterm.msec.st, msec_start);
							Msec msec_sub_gapn = calcDifGap(subterm.msec.st, term.msec.ed);
							if (msec_sub_gapp <= msec_prm_gap && msec_sub_gapn <= msec_prm_gap){
								setScpArstat(subterm, SCP_AR_N_AUNIT);
								nsc_lastwrite = subterm.nsc.st;
							}
							else{
								pdata->setScpChap(subterm.nsc.st, SCP_CHAP_NONE);	// 構成結合
								if (nsc_lastwrite == subterm.nsc.ed){
									setScpArstat(subterm, SCP_AR_N_AUNIT);
								}
								else if (nsc_lastwrite < 0){
									setScpArstat(subterm, SCP_AR_N_AUNIT);
									nsc_lastwrite = subterm.nsc.ed;
								}
							}
							getTermPrev(subterm);
						}
						if (nsc_lastwrite == term.nsc.ed){
							pdata->setScpArstat(nsc_lastwrite, SCP_AR_N_UNIT);
						}
						else if (nsc_lastwrite > 0){
							pdata->setScpArstat(nsc_lastwrite, SCP_AR_N_BUNIT);
						}
						keep = 0;
						change = false;
					}
					else if (sec_dif > 30) change = false;
				}
				if (keep > 0){
					setScpArstat(term, SCP_AR_N_OTHER);
				}
			}
			else{
				cont = false;
			}
			//--- 次の位置設定 ---
			if (cont){
				cont = getTermNext(term);
			}
		}
	}
}



//---------------------------------------------------------------------
// AutoEdge 開始前処理
// 出力：
//  返り値  : 動作実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoEdge(JlsCmdLimit &cmdlimit){
	Nsc nsc_base = subInsDelGetBase(cmdlimit);
	bool flag_exec = false;
	if (nsc_base >= 0){
		LogoEdgeType edge_base = cmdlimit.getLogoBaseEdge();
		flag_exec = startAutoEdgeMain(nsc_base, edge_base);
	}
	return flag_exec;
}


//---------------------------------------------------------------------
// AutoEdge ロゴ端部分のカット・追加
// 出力：
//  返り値  : 動作実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::startAutoEdgeMain(Nsc nsc_elg, LogoEdgeType edge_elg){
	//--- 使用パラメータ ---
	int prm_c_exe      = getAutoParam(ParamAuto::c_exe);
	int prm_c_cmpart   = getAutoParam(ParamAuto::c_cmpart);

	//--- 実行有無確認 ---
	bool flag_exec = false;
	if (prm_c_exe == 0){
		return false;
	}
	//--- ロゴエッジ種類を取得 ---
//	bool nedge = pdata->isElgInScp(nsc_elg);

	//--- ロゴ開始前に追加 ---
	if (nsc_elg >= 0 && edge_elg == LOGO_EDGE_RISE && prm_c_cmpart > 0){
		flag_exec = subEdgeExec(nsc_elg, LOGO_EDGE_RISE, SEARCH_DIR_PREV);
	}
	//--- ロゴ開始直後に追加 ---
	else if (nsc_elg >= 0 && edge_elg == LOGO_EDGE_RISE && prm_c_cmpart == 0){
		flag_exec = subEdgeExec(nsc_elg, LOGO_EDGE_RISE, SEARCH_DIR_NEXT);
	}
	//--- ロゴ終了直前に追加 ---
	else if (nsc_elg >= 0 && edge_elg == LOGO_EDGE_FALL && prm_c_cmpart == 0){
		flag_exec = subEdgeExec(nsc_elg, LOGO_EDGE_FALL, SEARCH_DIR_PREV);
	}
	//--- ロゴ終了直後に追加 ---
	else if (nsc_elg >= 0 && edge_elg == LOGO_EDGE_FALL && prm_c_cmpart > 0){
		flag_exec = subEdgeExec(nsc_elg, LOGO_EDGE_FALL, SEARCH_DIR_NEXT);
	}
	return flag_exec;
}

//---------------------------------------------------------------------
// ロゴ端部分のカット・追加実行部分
// 入力：
//   nsc_logo：    ロゴ基点に対応するシーンチェンジ番号
//   edge_logo:    ロゴエッジ立ち上がり／立ち下がり（LOGO_EDGE_RISE / LOGO_EDGE_FALL
//   dr:           検索方向前後（SEARCH_DIR_PREV / SEARCH_DIR_NEXT）
// 出力：
//  返り値  : 動作実行 false=未実行 true=実行
//---------------------------------------------------------------------
bool JlsAutoScript::subEdgeExec(Nsc nsc_logo, LogoEdgeType edge_logo, SearchDirType dr){
	//--- 使用パラメータ ---
	int prm_c_search   = getAutoParam(ParamAuto::c_search);
	int prm_scope      = getAutoParam(ParamAuto::v_scope);
	int prm_c_wmin     = getAutoParam(ParamAuto::c_wmin);
	int prm_c_wmax     = getAutoParam(ParamAuto::c_wmax);
	int prm_c_add      = getAutoParam(ParamAuto::c_add);
	int prm_c_allcom   = getAutoParam(ParamAuto::c_allcom);
	bool flag_adapt = (prm_c_search == 2 || prm_c_search == 3)? false : true;	// 既存カット適用後からの時1
	bool flag_ovw = (prm_c_search == 3)? true : false;

	//--- 初期化 ---
	bool flag_exec     = false;
	//--- ロゴ内の変更か判断 ---
	bool inlogo = false;
	if ((edge_logo == LOGO_EDGE_RISE && dr == SEARCH_DIR_NEXT) ||
		(edge_logo == LOGO_EDGE_FALL && dr == SEARCH_DIR_PREV)){
		inlogo = true;
	}
	//--- ロゴ終了位置を取得 ---
	Nsc nsc_endlogo;
	if (dr == SEARCH_DIR_PREV){
		nsc_endlogo = pdata->getNscPrevElg(nsc_logo, LOGO_EDGE_BOTH);
		if (nsc_endlogo < 0){			// 前がなければ先頭位置
			nsc_endlogo = 0;
		}
	}
	else{
		nsc_endlogo = pdata->getNscNextElg(nsc_logo, LOGO_EDGE_BOTH);
		if (nsc_endlogo < 0){			// 次がなければ最終位置
			nsc_endlogo = pdata->sizeDataScp() - 1;
		}
	}
	//--- ロゴ位置から順番に確認 ---
	{
		Nsc  nsc_base  = nsc_logo;
		Msec msec_logo = pdata->getMsecScp(nsc_logo);
		Msec msec_base = pdata->getMsecScp(nsc_base);
		Term term = {};
		term.ini = nsc_logo;
		bool cont = true;
		while(cont){
			Nsc nsc_dst;
			if (dr == SEARCH_DIR_PREV){			// 前側に検索時
				cont = getTermPrev(term);
				nsc_dst = term.nsc.st;
				if (nsc_dst < nsc_endlogo) cont = false;
			}
			else{								// 後側に検索時
				cont = getTermNext(term);
				nsc_dst = term.nsc.ed;
				if (nsc_dst > nsc_endlogo) cont = false;
			}
			if (cont){
				Msec msec_dst = pdata->getMsecScp(nsc_dst);
				Sec sec_dif_base = pdata->cnv.getSecFromMsec( abs(msec_dst - msec_base) );
				Sec sec_dif_logo = pdata->cnv.getSecFromMsec( abs(msec_dst - msec_logo) );
				ScpArExtType arext_term = getScpArext(term);
				//--- 既存のカット（残留）状態を取得 ---
				bool select_arext = false;
				if (arext_term == SCP_AREXT_N_LGCUT ||
					arext_term == SCP_AREXT_N_LGADD ||
					arext_term == SCP_AREXT_L_LGCUT ||
					arext_term == SCP_AREXT_L_LGADD){
					select_arext = true;					// カット（残留）が既に存在
				}
				//--- 既存のカット（残留）でシフト許可状態の場合は基準位置を変更 ---
				if (flag_adapt && select_arext){
					nsc_base = nsc_dst;						// 既存カット適用後の位置に基準を移動
					msec_base = msec_dst;
				}
				else{
					if (flag_adapt) flag_adapt = false;		// 基準位置確定
					//--- カット可能な最大秒数チェック ---
					if ((sec_dif_logo > prm_scope) || sec_dif_base > prm_c_wmax){
						cont = false;
					}
					//--- 既存カットが存在で書き換えしない ---
					else if (select_arext && flag_ovw == false){
						cont = false;
					}
					//--- カット適用範囲内の処理 ---
					else if (sec_dif_base >= prm_c_wmin && sec_dif_base <= prm_c_wmax){
						flag_exec = true;
						if (prm_c_allcom == 0){		// 見つかったら終了する場合
							cont = false;
						}
						ScpArExtType arext_add;
						if (prm_c_add == 0){
							arext_add = (inlogo)? SCP_AREXT_L_LGCUT : SCP_AREXT_N_LGCUT;
						}
						else{
							arext_add = (inlogo)? SCP_AREXT_L_LGADD : SCP_AREXT_N_LGADD;
						}
						//--- 現在位置を設定し、複数構成だった場合は全構成で設定 ---
						setScpArext(term, arext_add);		// 書き換え実行
						Term subterm = term;
						bool subcont = true;
						while(subcont){
							if (dr == SEARCH_DIR_PREV){	// 前側に検索時
								subcont = getTermNext(subterm);
								if (subterm.nsc.ed > nsc_base){
									subcont = false;
								}
							}
							else{						// 後側に検索時
								subcont = getTermPrev(subterm);
								if (subterm.nsc.st < nsc_base){
									subcont = false;
								}
							}
							if (subcont) setScpArext(subterm, arext_add);	// 書き換え実行
						}
					}
				}
			}
		}
	}
	return flag_exec;
}




//---------------------------------------------------------------------
// 次のロゴ期間構成（立ち上がり／立ち下がり）位置番号を取得（失敗時は更新しない）
// 入力
//  elg     : ロゴ扱い位置情報
// 出力：
//  返り値  : 0:ロゴなし  1:ロゴあり
//  elg     : ロゴ扱い位置情報（ロゴなしの時は更新しない）
//---------------------------------------------------------------------
bool JlsAutoScript::getElgNextKeep(ElgCurrent &elg){
	ElgCurrent elgbak = elg;
	bool ret = pdata->getElgptNext(elg);
	if (ret == false){
		elg = elgbak;
	}
	return ret;
}



//---------------------------------------------------------------------
// 設定値の取得（setParamコマンド設定）
//---------------------------------------------------------------------
int JlsAutoScript::getConfig(ConfigVarType tp){
	return pdata->getConfig(tp);
}

//---------------------------------------------------------------------
// 設定値の取得（autoコマンド実行時のパラメータ）
//---------------------------------------------------------------------
int JlsAutoScript::getAutoParam(ParamAuto tp){
	return m_autoArg.getParam(tp);
}

//---------------------------------------------------------------------
// 検索時に端を含めるか選択
//---------------------------------------------------------------------
void JlsAutoScript::setTermEndtype(Term &term, ScpEndType endtype){
	pdata->setTermEndtype(term, endtype);
}
//---------------------------------------------------------------------
// 次の構成を取得
//---------------------------------------------------------------------
bool JlsAutoScript::getTermNext(Term &term){
	if (!term.endfix){				// エッジ情報未設定時
		ScpEndType noedge = (getAutoParam(ParamAuto::c_noedge)!= 0)? SCP_END_NOEDGE : SCP_END_EDGEIN;
		setTermEndtype(term, noedge);
	}
	return pdata->getTermNext(term);
}
//---------------------------------------------------------------------
// 前の構成を取得
//---------------------------------------------------------------------
bool JlsAutoScript::getTermPrev(Term &term){
	if (!term.endfix){				// エッジ情報未設定時
		ScpEndType noedge = (getAutoParam(ParamAuto::c_noedge)!= 0)? SCP_END_NOEDGE : SCP_END_EDGEIN;
		setTermEndtype(term, noedge);
	}
	return pdata->getTermPrev(term);
}
//---------------------------------------------------------------------
// 各要素の設定・取得
//---------------------------------------------------------------------
ScpArType JlsAutoScript::getScpArstat(Term term){
	return pdata->getScpArstat(term);
}
ScpArExtType JlsAutoScript::getScpArext(Term term){
	return pdata->getScpArext(term);
}
void JlsAutoScript::setScpArstat(Term term, ScpArType arstat){
	pdata->setScpArstat(term, arstat);
}
void JlsAutoScript::setScpArext(Term term, ScpArExtType arext){
	pdata->setScpArext(term, arext);
}
//---------------------------------------------------------------------
// CM構成か判別
//---------------------------------------------------------------------
bool JlsAutoScript::isScpArstatCmUnit(Term term){
	return pdata->isScpArstatCmUnit(term);
}
//---------------------------------------------------------------------
// 検索範囲内にあるか判別
//---------------------------------------------------------------------
bool JlsAutoScript::checkScopeTerm(Term term, RangeMsec scope){
	return pdata->checkScopeTerm(term, scope);
}
bool JlsAutoScript::checkScopeRange(RangeMsec bounds, RangeMsec scope){
	return pdata->checkScopeRange(bounds, scope);
}

//---------------------------------------------------------------------
// 秒単位の誤差を取得
//---------------------------------------------------------------------
Msec JlsAutoScript::calcDifGap(Msec msec1, Msec msec2){
	Msec msec_dif = abs(msec1 - msec2);
	Msec msec_gap_pre = msec_dif - (msec_dif / 1000 * 1000);
	Msec msec_gap = (msec_gap_pre > 500)? 1000-msec_gap_pre : msec_gap_pre;
	return msec_gap;
}
