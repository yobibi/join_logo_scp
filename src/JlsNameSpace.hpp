//
// join_logo_scp データ用定義
//
#pragma once

//---------------------------------------------------------------------
// データ格納用
//---------------------------------------------------------------------
namespace jlsd
{
	using Msec = int;
	using Sec = int;
	using Nsc = int;
	using Nrf = int;
	using Nlg = int;

	//--- Auto構成区切り(scp.chap) ---
	enum ScpChapType {
		SCP_CHAP_DUPE = -1,		// 重複箇所
		SCP_CHAP_NONE,			// 初期状態
		SCP_CHAP_CPOSIT,		// 5秒単位可能性保持
		SCP_CHAP_CPOSQ,			// 15秒単位可能性保持
		SCP_CHAP_CDET,			// 15秒単位保持
		SCP_CHAP_DINT,			// 構成内部整数秒単位区切り
		SCP_CHAP_DBORDER,		// 無音シーンチェンジなし区切り
		SCP_CHAP_DFIX,			// 構成区切り
		SCP_CHAP_DFORCE,		// 強制区切り設定箇所
		SCP_CHAP_DUNIT			// Trimも強制区切り
	};
	static const ScpChapType SCP_CHAP_DECIDE = SCP_CHAP_DINT;		// 確定箇所閾値
	inline bool isScpChapTypeDecide(ScpChapType type){
		return (type >= SCP_CHAP_DECIDE)? true : false;
	}

	//--- Auto構成推測基本内容(scp.arstat) ---
	enum ScpArType {
		SCP_AR_UNKNOWN,			// 不明
		SCP_AR_L_UNIT,			// ロゴ有 １５秒単位
		SCP_AR_L_OTHER,			// ロゴ有 その他
		SCP_AR_L_MIXED,			// ロゴ有 ロゴ無も混合
		SCP_AR_N_UNIT,			// ロゴ無 １５秒単位
		SCP_AR_N_OTHER,			// ロゴ無 その他
		SCP_AR_N_AUNIT,			// ロゴ無 合併で１５秒の中間地点
		SCP_AR_N_BUNIT,			// ロゴ無 合併で１５秒の端
		SCP_AR_B_UNIT,			// ロゴ境界 １５秒単位
		SCP_AR_B_OTHER			// ロゴ境界 その他
	};
	static const int SCP_ARR_L_LOW  = SCP_AR_L_UNIT;		// ロゴ有の下限値
	static const int SCP_ARR_L_HIGH = SCP_AR_L_MIXED;		// ロゴ有の上限値
	inline bool isScpArTypeLogo(ScpArType type){
		return (type >= SCP_ARR_L_LOW && type <= SCP_ARR_L_HIGH)? true : false;
	}
	inline bool isScpArTypeBorder(ScpArType type){
		return (type >= SCP_AR_B_UNIT && type <= SCP_AR_B_OTHER)? true : false;
	}
	inline bool isScpArTypeLogoBorder(ScpArType type){
		return ((type >= SCP_ARR_L_LOW && type <= SCP_ARR_L_HIGH) ||
				(type >= SCP_AR_B_UNIT && type <= SCP_AR_B_OTHER))? true : false;
	}
	//--- Auto構成推測拡張内容(scp.arext) ---
	enum ScpArExtType {
		SCP_AREXT_NONE,			// 追加構成なし
		SCP_AREXT_L_TRKEEP,		// ロゴ有 残す予告
		SCP_AREXT_L_TRCUT,		// ロゴ有 カット番宣
		SCP_AREXT_L_TRRAW,		// ロゴ有 エンドカード判断前
		SCP_AREXT_L_ECCUT,		// ロゴ有 カット番宣
		SCP_AREXT_L_EC,			// ロゴ有 エンドカード
		SCP_AREXT_L_SP,			// ロゴ有 番組提供
		SCP_AREXT_L_LGCUT,		// ロゴ有 ロゴ端部分カット
		SCP_AREXT_L_LGADD,		// ロゴ有 ロゴ端部分残す
		SCP_AREXT_N_TRCUT,		// ロゴ無 カット番宣
		SCP_AREXT_N_LGCUT,		// ロゴ無 ロゴ端部分カット
		SCP_AREXT_N_LGADD		// ロゴ無 ロゴ端部分残す
	};
	

	//--- 設定値保持 ---
	enum class ConfigVarType {
		msecWLogoTRMax,			// AutoCutコマンドでカット対象とするロゴ期間最大フレーム期間
		msecWCompTRMax,			// AutoCutコマンドTRで予告と認識する構成最大フレーム期間
		msecWLogoSftMrg,		// Autoコマンド前調整でロゴ切り替わりのずれを許すフレーム期間
		msecWCompFirst,			// 先頭構成カット扱いにする構成最大フレーム期間
		msecWCompLast,			// 最後構成カット扱いにする構成最大フレーム期間
		msecWLogoSumMin,		// ロゴ合計期間が指定フレーム未満の時はロゴなしとして扱う
		msecWLogoLgMin,			// CM推測時にロゴ有情報確定と認識する最小ロゴ期間
		msecWLogoCmMin,			// CM推測時にロゴ無情報確定と認識する最小ロゴ期間
		msecWLogoRevMin,		// ロゴ情報補正する時に本編と認識する最小期間
		msecMgnCmDetect,		// CM構成で15秒単位ではない可能性と認識する誤差フレーム期間
		msecMgnCmDivide,		// CM構成内分割を許す１秒単位からの誤差フレーム期間
		secWCompSPMin,			// Autoコマンド番組提供で標準最小秒数
		secWCompSPMax,			// Autoコマンド番組提供で標準最大秒数
		flagCutTR,				// 15秒以上番宣をカットする場合は1をセット
		flagCutSP,				// 番組提供をカットする場合は1をセット
		flagAddLogo,			// ロゴあり通常構成を残す場合は1をセット（現在は未使用）
		flagAddUC,				// ロゴなし不明構成を残す場合は1をセット
		typeNoSc,				// シーンチェンジなし無音位置のCM判断（0:自動 1:なし 2:あり）
		cancelCntSc,			// 無音が多い構成を分離しない処理を1の時は使用しない
		LogoLevel,				// ロゴ使用レベル
		LogoRevise,				// ロゴからの補正
		AutoCmSub,				// ロゴなし時の補助設定
		msecPosFirst,			// ロゴ開始位置検出設定期間
		msecLgCutFirst,			// ロゴが最初からある時にカット扱いにする構成最大フレーム期間
		msecZoneFirst,			// ロゴ無効とする開始位置検出設定期間
		msecZoneLast,			// ロゴ無効とする終了位置検出設定期間
		priorityPosFirst,		// 最初の位置設定優先度（0:制御なし 1:ロゴあり 2:位置優先 3:Select優先）
		MAXSIZE
	};
    static const int SIZE_CONFIG_VAR = static_cast<int>(ConfigVarType::MAXSIZE);

	enum class ConfigActType {	// 設定値を動作別に取得用
		LogoDelEdge,			// ロゴ端のCM判断
		LogoDelMid,				// ロゴ内の15秒単位CM化
		LogoDelWide,			// 広域ロゴなし削除
		LogoUCRemain,			// ロゴなし不明部分を残す
		LogoUCGapCm,			// CM単位から誤差が大きい構成を残す
		MuteNoSc				// シーンチェンジなし無音位置のCM判断（1:使用しない 2:使用する）
	};

	//--- 構成候補優先順位(scp.stat) ---
	enum ScpPriorType {
		SCP_PRIOR_DUPE = -1,	// 間引き
		SCP_PRIOR_NONE,			// 初期状態
		SCP_PRIOR_LV1,			// 候補
		SCP_PRIOR_DECIDE		// 決定
	};

	//--- 構成候補優先順位(logo.stat_*) ---
	enum LogoPriorType {
		LOGO_PRIOR_DUPE = -1,	// 間引き
		LOGO_PRIOR_NONE,		// 初期状態
		LOGO_PRIOR_LV1,			// 候補
		LOGO_PRIOR_DECIDE		// 決定
	};
	//--- ロゴ分離構成状態(logo.unit_*) ---
	enum LogoUnitType {
		LOGO_UNIT_NORMAL,		// 通常
		LOGO_UNIT_DIVIDE		// ロゴ分離
	};
	//--- ロゴ結果確定状態(logo.flag_*) ---
	enum LogoResultType {
		LOGO_RESULT_NONE,		// 初期状態
		LOGO_RESULT_DECIDE,		// 確定
		LOGO_RESULT_ABORT		// abort破棄確定
	};

	//--- 選択方向 ---
	enum SearchDirType {
		SEARCH_DIR_PREV,
		SEARCH_DIR_NEXT
	};

	//--- ロゴの選択エッジ ---
	enum LogoEdgeType {
		LOGO_EDGE_RISE,			// ロゴの立ち上がりエッジ
		LOGO_EDGE_FALL,			// ロゴの立ち下がりエッジ
		LOGO_EDGE_BOTH			// ロゴの両エッジ
	};
	//--- ロゴの選択 ---
	enum LogoSelectType {
		LOGO_SELECT_ALL,		// 全選択
		LOGO_SELECT_VALID		// 有効のみ選択
	};
	//--- シーンチェンジで全体の先頭最後を除く選択 ---
	enum ScpEndType {
		SCP_END_EDGEIN,			// シーンチェンジ番号の先頭最後含む
		SCP_END_NOEDGE			// シーンチェンジ番号の先頭最後除く
	};

	//--- ロゴのエッジ方向認識 ---
	inline bool isLogoEdgeRise(LogoEdgeType type){
		return (type == LOGO_EDGE_RISE || type == LOGO_EDGE_BOTH)? true : false;
	}
	inline bool isLogoEdgeFall(LogoEdgeType type){
		return (type == LOGO_EDGE_FALL || type == LOGO_EDGE_BOTH)? true : false;
	}
	inline LogoEdgeType edgeInvert(LogoEdgeType type){
		return (type == LOGO_EDGE_FALL)? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
	}
	inline bool isLogoEdgeRiseFromNrf(int nrf){
		return (nrf % 2 == 0)? true : false;
	}
	//--- ロゴ番号変換（nrf - nlg） ---
	inline LogoEdgeType edgeFromNrf(int nrf){
		return (nrf % 2 == 0)? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
	}
	inline LogoEdgeType edgeInvertFromNrf(int nrf){
		return (nrf % 2 == 0)? LOGO_EDGE_FALL : LOGO_EDGE_RISE;
	}
	inline int nlgFromNrf(int nrf){
		return nrf / 2;
	}
	inline int nrfFromNlg(int nlg, LogoEdgeType edge){
		return (edge == LOGO_EDGE_FALL)? nlg*2+1 : nlg*2;
	}
	inline int nrfFromNlgRise(int nlg){
		return nlg*2;
	}
	inline int nrfFromNlgFall(int nlg){
		return nlg*2+1;
	}
	//--- 優先順位の変換 ---
	inline LogoPriorType priorLogoFromScp(ScpPriorType n){
		return (LogoPriorType) n;
	}

	//--- 保持設定値のフラグ値定義 ---
	enum ConfigBitType {
		CONFIG_LOGO_LEVEL_DEFAULT   = 0,
		CONFIG_LOGO_LEVEL_UNUSE_ALL = 1,
		CONFIG_LOGO_LEVEL_UNUSE_EX1 = 2,
		CONFIG_LOGO_LEVEL_UNUSE_EX2 = 3,
		CONFIG_LOGO_LEVEL_USE_LOW   = 4,
		CONFIG_LOGO_LEVEL_USE_MIDL  = 5,
		CONFIG_LOGO_LEVEL_USE_MIDH  = 6,
		CONFIG_LOGO_LEVEL_USE_HIGH  = 7,
		CONFIG_LOGO_LEVEL_USE_MAX   = 8
	};

	//--- 構造体 ---
	struct RangeNsc {
		Nsc st;
		Nsc ed;
	};
	struct RangeNrf {
		Nrf st;
		Nrf ed;
	};
	struct RangeMsec {
		Msec st;
		Msec ed;
	};
	struct WideMsec {
		Msec just;
		Msec early;
		Msec late;
	};
	struct RangeFixMsec {
		Msec st;
		Msec ed;
		bool fixSt;				// true=確定開始地点
		bool fixEd;				// true=確定終了地点
	};
	struct RangeWideMsec {
		WideMsec st;
		WideMsec ed;
		bool fixSt;				// true=確定開始地点
		bool fixEd;				// true=確定終了地点
		bool logomode;			// false=CM期間  true=ロゴ期間
	};
	struct RangeNscMsec {
		RangeNsc  nsc;
		RangeMsec msec;
	};
	struct Term {
		bool valid;
		bool endfix;
		ScpEndType endtype;
		Nsc ini;
		RangeNsc nsc;
		RangeMsec msec;
	};
	struct NrfCurrent {
		bool valid;				// 0=データ格納なし  1=データ格納あり
		Nrf nrfRise;
		Nrf nrfFall;
		Nrf nrfLastRise;
		Nrf nrfLastFall;
		Msec msecRise;
		Msec msecFall;
		Msec msecLastRise;
		Msec msecLastFall;
	};
	struct ElgCurrent {
		bool valid;				// 0=データ格納なし  1=データ格納あり
		bool border;			// 0=border含めない  1=border含む
		bool outflag;			// 0=内部動作 1=最終出力動作
		Nsc nscRise;
		Nsc nscFall;
		Nsc nscLastRise;
		Nsc nscLastFall;
		Msec msecRise;
		Msec msecFall;
		Msec msecLastRise;
		Msec msecLastFall;
	};
	struct CalcDifInfo {
		int  sgn;
		Sec  sec;
		Msec gap;
	};
	struct CalcModInfo {
		Msec mod15;
		Msec mod05;
	};

}

//---------------------------------------------------------------------
// JLスクリプトの引数関連保持
//---------------------------------------------------------------------
namespace jlscmd
{
	const int SIZE_VARNUM_MAX = 2048;	// 変数の最大数を念のため設定(JlsRegFile)
	const int SIZE_MEMVARNUM_MAX = 2048;	// 遅延保管文字列の識別名最大数(JlsScrMemBody)
	const int SIZE_MEMVARLINE_MAX = 4096;	// 遅延保管文字列の各識別名の最大行数(JlsScrMemBody)
	const int SIZE_REPLINE  = 4096;		// キャッシュ保持最大行数（Repeat用）(JlsScriptState)
	const int SIZE_MEMLINE  = 8192;		// キャッシュ保持最大行数（Mem/Lazy用）(JlsScrGlobal)
	const int SIZE_CALL_LOOP = 10;		// Callコマンドの再帰最大回数(JlsScript)

	//--- JLスクリプト命令 ---
	enum class CmdType {
		Nop,
		If,
		EndIf,
		Else,
		ElsIf,
		Call,
		Repeat,
		EndRepeat,
		LocalSt,
		LocalEd,
		ArgBegin,
		ArgEnd,
		Exit,
		Return,
		FileOpen,
		FileAppend,
		FileClose,
		Echo,
		LogoOff,
		OldAdjust,
		LogoDirect,
		LogoExact,
		LogoReset,
		ReadData,
		ReadTrim,
		ReadString,
		EnvGet,
		Set,
		Default,
		EvalFrame,
		EvalTime,
		EvalNum,
		CountUp,
		SetParam,
		OptSet,
		OptDefault,
		UnitSec,
		LocalSet,
		ArgSet,
		ListGetAt,
		ListIns,
		ListDel,
		ListSetAt,
		ListClear,
		ListSort,
		AutoCut,
		AutoAdd,
		AutoEdge,
		AutoCM,
		AutoUp,
		AutoBorder,
		AutoIClear,
		AutoIns,
		AutoDel,
		Find,
		MkLogo,
		DivLogo,
		Select,
		Force,
		Abort,
		GetPos,
		GetList,
		NextTail,
		DivFile,
		LazyStart,
		EndLazy,
		Memory,
		EndMemory,
		MemCall,
		MemErase,
		MemCopy,
		MemMove,
		MemAppend,
		MemOnce,
		LazyFlush,
		LazyAuto,
		LazyStInit,
		MemEcho,
		MemDump,
		ExpandOn,
		ExpandOff,
		MAXSIZE
	};
	//--- JLスクリプト命令種類 ---
	enum class CmdCat {
		NONE,
		COND,
		CALL,
		REP,
		FLOW,
		SYS,
		REG,
		NEXT,
		LOGO,
		AUTOLOGO,
		AUTOEACH,
		AUTO,
		LAZYF,
		MEMF,
		MEMEXE,
		MEMLAZYF,
	};
	//--- JLスクリプト遅延実行用Cache種類 ---
	enum class CacheExeType {
		None,
		LazyS,
		LazyA,
		LazyE,
		Mem,
	};
	//--- JLスクリプトLazy動作種類 ---
	enum class LazyType {
		None,
		FULL,
		LazyS,
		LazyA,
		LazyE,
	};
	//--- JLスクリプトオプション分類 ---
	enum class OptCat {
		None,
		PosSC,
		NumLG,
		FRAME,
		STR,
		NUM,
	};
	//--- JLスクリプトオプション命令 ---
	enum class OptType {
		None,			// 未定義認識用
		ArrayMIN,		// 開始識別用
		TypeNumLogo,
		TypeFrame,
		TypeFrameSub,
		MsecFrameL,
		MsecFrameR,
		MsecEndlenC,
		MsecEndlenL,
		MsecEndlenR,
		MsecSftC,
		MsecSftL,
		MsecSftR,
		MsecTgtLimL,
		MsecTgtLimR,
		MsecLenPMin,
		MsecLenPMax,
		MsecLenNMin,
		MsecLenNMax,
		MsecLenPEMin,
		MsecLenPEMax,
		MsecLenNEMin,
		MsecLenNEMax,
		MsecFromAbs,
		MsecFromHead,
		MsecFromTail,
		MsecLogoExtL,
		MsecLogoExtR,
		MsecEndAbs,
		MsecDcenter,
		MsecDrangeL,
		MsecDrangeR,
		MsecDmargin,
		MsecEmargin,
		AutopCode,
		AutopLimit,
		AutopScope,
		AutopScopeN,
		AutopScopeX,
		AutopPeriod,
		AutopMaxPrd,
		AutopSecNext,
		AutopSecPrev,
		AutopTrScope,
		AutopTrSumPrd,
		AutopTr1stPrd,
		AutopTrInfo,
		FlagWide,
		FlagFromLast,
		FlagWithP,
		FlagWithN,
		FlagNoEdge,
		FlagOverlap,
		FlagConfirm,
		FlagUnit,
		FlagElse,
		FlagCont,
		FlagReset,
		FlagFlat,
		FlagForce,
		FlagNoForce,
		FlagAutoChg,
		FlagAutoEach,
		FlagEndHead,
		FlagEndTail,
		FlagEndHold,
		FlagRelative,
		FlagLazyS,
		FlagLazyA,
		FlagLazyE,
		FlagNow,
		FlagNoLap,
		FlagEdgeS,
		FlagEdgeE,
		FlagClear,
		FlagPair,
		FlagFinal,
		FlagLocal,
		FlagDefault,
		FlagUnique,
		FlagDummy,
		AbbrEndlen,
		AbbrSft,
		AbbrFromHead,
		AbbrFromTail,
		ArrayMAX,		// JlsCmdSetでデータ格納する配列はここまで

		ScMIN,
		ScNone,
		ScSC,
		ScNoSC,
		ScSM,
		ScNoSM,
		ScSMA,
		ScNoSMA,
		ScAC,
		ScNoAC,
		ScMAX,

		LgMIN,
		LgNone,
		LgN,
		LgNR,
		LgNlogo,
		LgNauto,
		LgNFlogo,
		LgNFauto,
		LgNFXlogo,
		LgMAX,

		FrMIN,
		FrF,
		FrFR,
		FrFhead,
		FrFtail,
		FrFmid,
		FrFheadX,
		FrFtailX,
		FrFmidX,
		FrMAX,

		StrMIN,			// JlsCmdSetでデータ格納する文字列オプション開始
		StrRegPos,
		StrValPosR,
		StrValPosW,
		StrRegList,
		StrValListR,
		StrValListW,
		StrRegSize,
		StrRegEnv,
		StrArgVal,
		StrMAX,			// JlsCmdSetでデータ格納する文字列オプション終了
	};

	//--- JLスクリプト命令サブ選択 ---
	enum CmdTrSpEcID {
		None,
		Off,
		TR,
		SP,
		EC,
		LG,
	};
	//--- JLスクリプトデコード結果エラー ---
	enum class CmdErrType {
		None,
		ErrOpt,				// コマンド異常（オプション）
		ErrRange,			// コマンド異常（範囲）
		ErrSEB,				// コマンド異常（S/E/B選択）
		ErrVar,				// コマンド異常（変数関連）
		ErrTR,				// コマンド異常（TR/SP/ED選択）
		ErrCmd,				// コマンド異常（コマンド）
	};
	//--- JLスクリプトAuto系コマンド ---
	enum class CmdAutoType {
		None,
		CutTR,
		CutEC,
		AddTR,
		AddSP,
		AddEC,
		Edge,
		AtCM,
		AtUP,
		AtBorder,
		AtIClear,
		AtChg,
		Ins,
		Del,
	};
	//--- Autoコマンドパラメータ ---
	enum class ParamAuto {
		// codeパラメータ
		c_exe,			// 0:コマンド実行なし 1:コマンド実行
		c_search,		// 検索する範囲を選択
		c_wmin,			// 構成期間の最小値秒数
		c_wmax,			// 構成期間の最大値秒数
		c_w15,			// 1:番組構成で15秒を検索
		c_lgprev,		// 0:ロゴ・予告の前側を対象外
		c_lgpost,		// 0:ロゴ・予告の後側を対象外
		c_lgintr,		// 1:予告と番組提供の間のみ対象とする
		c_lgsp,			// 1:番組提供が直後にある場合のみ対象
		c_cutskip,		// 1:予告カット以降も対象とする
		c_in1,			// 1:予告位置に番組提供を入れる
		c_chklast,		// 1:本体構成が後にあれば対象外とする
		c_lgy,			// 1:ロゴ内を対象とする
		c_lgn,			// 1:ロゴ外を対象とする
		c_lgbn,			// 1:両隣を含めロゴ外の場合を対象とする
		c_limloc,		// 1:標準期間の候補位置のみに限定
		c_limtrsum,		// 1:予告期間により無効化する
		c_unitcmoff,		// 1:CM分割した構成の検出を強制無効
		c_unitcmon,		// 1:CM分割した構成の検出を強制設定
		c_wdefmin,		// 標準の構成期間の最小値秒数
		c_wdefmax,		// 標準の構成期間の最大値秒数
		// autocut用
		c_from,			// cuttr
		c_cutst,			// cuttr
		c_lgpre,			// cuttr
		c_sel,			// cutec
		c_cutla,			// cutec
		c_cutlp,			// cutec
		c_cut30,			// cutec
		c_cutsp,			// cutec
		// edge用
		c_cmpart,
		c_add,
		c_allcom,
		c_noedge,
		// autoins,autodel用
		c_restruct,
		c_unit,
		// 数値パラメータ
		v_limit,
		v_scope,
		v_scopen,
		v_period,
		v_maxprd,
		v_trsumprd,
		v_secprev,
		v_secnext,
		// autocut用
		v_trscope,
		v_tr1stprd,
		// autoins,autodel用
		v_info,
		// 合計数
		MAXSIZE
	};
}

