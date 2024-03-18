//
// 実行スクリプトコマンド文字列解析
//  JlsScriptDecode : コマンド文字列解析
//  出力：
//    JlsCmdArg &cmdarg
//  pdataは文字列・時間変換機能(cnv)のみ使用
//
#pragma once

class JlsCmdArg;
class JlsDataset;
class JlsScrFuncList;

///////////////////////////////////////////////////////////////////////
//
// 実行スクリプトコマンド文字列解析クラス
//
///////////////////////////////////////////////////////////////////////
class JlsScriptDecode
{
private:
	static const int msecDecodeMargin = 1200;	// 範囲指定時のデフォルトマージン

	// 命令セット構成
	enum class ConvStrType {
		None,			// 変換しない
		Msec,			// ミリ秒取得
		MsecM1,			// ミリ秒取得（マイナス１はそのまま残す）
		Sec,			// 秒取得（整数入力は秒として扱う）
		Num,			// 数値取得
		Frame,			// フレーム数表記取得
		Time,			// 時間表記取得
		TrSpEc,			// TR/SP/EC選択
		Param,			// setParam別の演算
		CondIF,			// IF文判定式
		NumR,			// 複数・範囲指定あり数値リスト
	};

	struct JlscrCmdRecord {
		string	cmdname;			// コマンド文字列
		CmdType cmdsel;				// 選択コマンド
		CmdCat  category;			// コマンド種別
		int     muststr;			// 必須オプション文字列取得（0-3=取得数 9=残り全体）
		int		mustchar;			// 必須オプション文字（0=なし 1=S/E/B 2=TR/SP/EC 3=B省略可）
		int		mustrange;			// 期間指定（0=なし  1=center  3=center+left+right）
		int		needopt;			// 追加オプション（0=なし  1=読み込み）
	};
	struct JlscrCmdAlias {
		string	cmdname;			// コマンド別名文字列
		CmdType cmdsel;				// 選択コマンド
	};
	struct JlScrCmdCalcRecord {
		CmdType     cmdsel;			// 選択コマンド
		int         numArg;			// 引数位置
		ConvStrType typeVal;		// 変換種類
	};
	struct JlOptionRecord {
		string       optname;		// コマンド文字列
		OptType      optType;		// オプションの種類
		int          subType;		// 種類補助設定
		int          numArg;		// 引数入力数
		int          minArg;		// 引数最小必要数
		int          numFrom;		// 引数省略時開始番号設定
		int          sort;			// 引数並び替え(12=1と2, 23=2と3)
		ConvStrType  convType;		// 引数の変換処理
	};
	struct ConfigDataRecord {
		string         namestr;
		ConfigVarType  prmsel;
		ConvStrType    valsel;
	};
	struct JlOptMirrorRecord {
		CmdType      cmdsel;		// 選択コマンド
		OptType      optTypeTo;		// オプションの種類
		OptType      optTypeFrom;	// オプションの種類
	};

//--- コマンドリスト（文字列、コマンド、種別、文字列引数、文字引数タイプ、引数範囲数、オプション有無） ---
//--- 引数タイプ： 0=なし 1=S/E/B 2=TR/SP/EC 3=B省略可 9=残り全体 11=1引数 12=2引数 ---
const vector<JlscrCmdRecord> CmdDefine = {
	{ "Nop"          , CmdType::Nop,        CmdCat::NONE,     0,0,0,0 },
	{ "If"           , CmdType::If,         CmdCat::COND,     9,0,0,0 },
	{ "EndIf"        , CmdType::EndIf,      CmdCat::COND,     0,0,0,0 },
	{ "Else"         , CmdType::Else,       CmdCat::COND,     0,0,0,0 },
	{ "ElsIf"        , CmdType::ElsIf,      CmdCat::COND,     9,0,0,0 },
	{ "Call"         , CmdType::Call,       CmdCat::CALL,     1,0,0,0 },
	{ "Fcall"        , CmdType::Fcall,      CmdCat::CALL,     7,0,0,1 },
	{ "Repeat"       , CmdType::Repeat,     CmdCat::REP,      1,0,0,1 },
	{ "EndRepeat"    , CmdType::EndRepeat,  CmdCat::REP,      0,0,0,0 },
	{ "Break"        , CmdType::Break,      CmdCat::FLOW,     9,0,0,0 },
	{ "{"            , CmdType::LocalSt,    CmdCat::FLOW,     0,0,0,0 },
	{ "}"            , CmdType::LocalEd,    CmdCat::FLOW,     0,0,0,0 },
	{ "};"           , CmdType::LocalEEnd,  CmdCat::FLOW,     0,0,0,0 },
	{ ";"            , CmdType::EndMulti,   CmdCat::FLOW,     0,0,0,0 },
	{ "ArgBegin"     , CmdType::ArgBegin,   CmdCat::FLOW,     0,0,0,1 },
	{ "ArgEnd"       , CmdType::ArgEnd,     CmdCat::FLOW,     0,0,0,0 },
	{ "Exit"         , CmdType::Exit,       CmdCat::FLOW,     0,0,0,0 },
	{ "Return"       , CmdType::Return,     CmdCat::FLOW,     0,0,0,0 },
	{ "Mkdir"        , CmdType::Mkdir,      CmdCat::SYS,      1,0,0,0 },
	{ "FileCopy"     , CmdType::FileCopy,   CmdCat::SYS,      2,0,0,0 },
	{ "FileOpen"     , CmdType::FileOpen,   CmdCat::SYS,      1,0,0,1 },
	{ "FileAppend"   , CmdType::FileAppend, CmdCat::SYS,      1,0,0,1 },
	{ "FileClose"    , CmdType::FileClose,  CmdCat::SYS,      0,0,0,0 },
	{ "FileCode"     , CmdType::FileCode,   CmdCat::SYS,      1,0,0,0 },
	{ "FileToMemo"   , CmdType::FileToMemo, CmdCat::SYS,      1,0,0,0 },
	{ "Echo"         , CmdType::Echo,       CmdCat::SYS,      9,0,0,0 },
	{ "EchoItem"     , CmdType::EchoItem,   CmdCat::SYS,      1,0,0,1 },
	{ "EchoItemQ"    , CmdType::EchoItemQ,  CmdCat::SYS,      1,9,0,1 },
	{ "EchoFile"     , CmdType::EchoFile,   CmdCat::SYS,      1,0,0,0 },
	{ "EchoOavs"     , CmdType::EchoOavs,   CmdCat::SYS,      0,0,0,0 },
	{ "EchoOscp"     , CmdType::EchoOscp,   CmdCat::SYS,      0,0,0,0 },
	{ "EchoMemo"     , CmdType::EchoMemo,   CmdCat::SYS,      0,0,0,0 },
	{ "LogoOff"      , CmdType::LogoOff,    CmdCat::SYS,      0,0,0,0 },
	{ "OldAdjust"    , CmdType::OldAdjust,  CmdCat::SYS,      1,0,0,0 },
	{ "IgnoreCase"   , CmdType::IgnoreCase, CmdCat::SYS,      1,0,0,0 },
	{ "SysMesDisp"   , CmdType::SysMesDisp, CmdCat::SYS,      1,0,0,0 },
	{ "SysMemoSel"   , CmdType::SysMemoSel, CmdCat::SYS,      1,0,0,0 },
	{ "SysDataGet"   , CmdType::SysDataGet, CmdCat::SYS,      1,0,0,1 },
	{ "LogoDirect"   , CmdType::LogoDirect, CmdCat::SYS,      0,0,0,1 },
	{ "LogoExact"    , CmdType::LogoExact,  CmdCat::SYS,      0,0,0,1 },
	{ "LogoReset"    , CmdType::LogoReset,  CmdCat::SYS,      0,0,0,1 },
	{ "ReadData"     , CmdType::ReadData,   CmdCat::READ,     1,0,0,1 },
	{ "ReadTrim"     , CmdType::ReadTrim,   CmdCat::READ,     1,0,0,1 },
	{ "ReadString"   , CmdType::ReadString, CmdCat::READ,     1,0,0,1 },
	{ "ReadCheck"    , CmdType::ReadCheck,  CmdCat::READ,     1,0,0,1 },
	{ "ReadOpen"     , CmdType::ReadOpen,   CmdCat::READ,     1,0,0,1 },
	{ "ReadClose"    , CmdType::ReadClose,  CmdCat::READ,     0,0,0,1 },
	{ "ReadLine"     , CmdType::ReadLine,   CmdCat::READ,     0,0,0,1 },
	{ "EnvGet"       , CmdType::EnvGet,     CmdCat::READ,     1,0,0,1 },
	{ "SetReg"       , CmdType::SetReg,     CmdCat::REG,      2,0,0,1 },
	{ "Unset"        , CmdType::Unset,      CmdCat::REG,      1,0,0,1 },
	{ "Set"          , CmdType::Set,        CmdCat::REG,      2,0,0,1 },
	{ "Default"      , CmdType::Default,    CmdCat::REG,      2,0,0,1 },
	{ "SetList"      , CmdType::SetList,    CmdCat::REG,      2,9,0,1 },
	{ "EvalFrame"    , CmdType::EvalFrame,  CmdCat::REG,      2,0,0,1 },
	{ "EvalTime"     , CmdType::EvalTime,   CmdCat::REG,      2,0,0,1 },
	{ "EvalNum"      , CmdType::EvalNum,    CmdCat::REG,      2,0,0,1 },
	{ "CountUp"      , CmdType::CountUp,    CmdCat::REG,      1,0,0,1 },
	{ "SetParam"     , CmdType::SetParam,   CmdCat::REG,      2,0,0,0 },
	{ "OptSet"       , CmdType::OptSet,     CmdCat::REG,      9,0,0,0 },
	{ "OptDefault"   , CmdType::OptDefault, CmdCat::REG,      9,0,0,0 },
	{ "UnitSec"      , CmdType::UnitSec,    CmdCat::REG,      1,0,0,0 },
	{ "LocalSet"     , CmdType::LocalSet,   CmdCat::REG,      2,0,0,0 },
	{ "LocalSetF"    , CmdType::LocalSetF,  CmdCat::REG,      2,0,0,1 },
	{ "LocalSetT"    , CmdType::LocalSetT,  CmdCat::REG,      2,0,0,1 },
	{ "LocalSetN"    , CmdType::LocalSetN,  CmdCat::REG,      2,0,0,1 },
	{ "ArgSet"       , CmdType::ArgSet,     CmdCat::REG,      2,0,0,0 },
	{ "ArgSetReg"    , CmdType::ArgSetReg,  CmdCat::REG,      2,0,0,0 },
	{ "ListGetAt"    , CmdType::ListGetAt,  CmdCat::REG,      1,0,0,1 },
	{ "ListIns"      , CmdType::ListIns,    CmdCat::REG,      1,0,0,1 },
	{ "ListDel"      , CmdType::ListDel,    CmdCat::REG,      1,0,0,1 },
	{ "ListSetAt"    , CmdType::ListSetAt,  CmdCat::REG,      1,0,0,1 },
	{ "ListJoin"     , CmdType::ListJoin,   CmdCat::REG,      0,0,0,1 },
	{ "ListRemove"   , CmdType::ListRemove, CmdCat::REG,      0,0,0,1 },
	{ "ListSel"      , CmdType::ListSel,    CmdCat::REG,      1,0,0,1 },
	{ "ListClear"    , CmdType::ListClear,  CmdCat::REG,      0,0,0,1 },
	{ "ListDim"      , CmdType::ListDim,    CmdCat::REG,      1,0,0,1 },
	{ "ListSort"     , CmdType::ListSort,   CmdCat::REG,      0,0,0,1 },
	{ "SplitCsv"     , CmdType::SplitCsv,   CmdCat::REG,      0,0,0,1 },
	{ "SplitItem"    , CmdType::SplitItem,  CmdCat::REG,      0,0,0,1 },
	{ "AutoCut"      , CmdType::AutoCut,    CmdCat::AUTO,     0,2,0,1 },
	{ "AutoAdd"      , CmdType::AutoAdd,    CmdCat::AUTO,     0,2,0,1 },
	{ "AutoEdge"     , CmdType::AutoEdge,   CmdCat::AUTOLOGO, 0,1,0,1 },
	{ "AutoCM"       , CmdType::AutoCM,     CmdCat::AUTO,     0,3,0,1 },
	{ "AutoUp"       , CmdType::AutoUp,     CmdCat::AUTO,     0,3,0,1 },
	{ "AutoBorder"   , CmdType::AutoBorder, CmdCat::AUTO,     0,3,0,1 },
	{ "AutoIClear"   , CmdType::AutoIClear, CmdCat::AUTO,     0,3,0,1 },
	{ "AutoIns"      , CmdType::AutoIns,    CmdCat::AUTOLOGO, 0,1,3,1 },
	{ "AutoDel"      , CmdType::AutoDel,    CmdCat::AUTOLOGO, 0,1,3,1 },
	{ "Find"         , CmdType::Find,       CmdCat::LOGO,     0,1,3,1 },
	{ "MkLogo"       , CmdType::MkLogo,     CmdCat::LOGO,     0,1,3,1 },
	{ "DivLogo"      , CmdType::DivLogo,    CmdCat::LOGO,     0,1,3,1 },
	{ "Select"       , CmdType::Select,     CmdCat::LOGO,     0,1,3,1 },
	{ "Force"        , CmdType::Force,      CmdCat::LOGO,     0,1,1,1 },
	{ "Abort"        , CmdType::Abort,      CmdCat::LOGO,     0,1,0,1 },
	{ "GetPos"       , CmdType::GetPos,     CmdCat::LOGO,     0,1,3,1 },
	{ "GetList"      , CmdType::GetList,    CmdCat::LOGO,     0,1,3,1 },
	{ "NextTail"     , CmdType::NextTail,   CmdCat::NEXT,     0,1,3,1 },
	{ "DivFile"      , CmdType::DivFile,    CmdCat::LOGO,     0,1,1,1 },
	{ "LazyStart"    , CmdType::LazyStart,  CmdCat::LAZYF,    0,0,0,1 },
	{ "EndLazy"      , CmdType::EndLazy,    CmdCat::LAZYF,    0,0,0,0 },
	{ "Memory"       , CmdType::Memory,     CmdCat::MEMF,     1,0,0,1 },
	{ "Function"     , CmdType::Function,   CmdCat::MEMF,     7,0,0,1 },
	{ "EndMemory"    , CmdType::EndMemory,  CmdCat::MEMF,     0,0,0,0 },
	{ "EndFunction"  , CmdType::EndFunc,    CmdCat::MEMF,     0,0,0,0 },
	{ "MemSet"       , CmdType::MemSet,     CmdCat::MEMF,     1,0,0,1 },
	{ "MemCall"      , CmdType::MemCall,    CmdCat::MEMEXE,   1,0,0,1 },
	{ "MemErase"     , CmdType::MemErase,   CmdCat::MEMEXE,   1,0,0,0 },
	{ "MemCopy"      , CmdType::MemCopy,    CmdCat::MEMEXE,   2,0,0,0 },
	{ "MemMove"      , CmdType::MemMove,    CmdCat::MEMEXE,   2,0,0,0 },
	{ "MemAppend"    , CmdType::MemAppend,  CmdCat::MEMEXE,   2,0,0,0 },
	{ "MemOnce"      , CmdType::MemOnce,    CmdCat::MEMEXE,   1,0,0,0 },
	{ "LazyFlush"    , CmdType::LazyFlush,  CmdCat::MEMEXE,   0,0,0,0 },
	{ "LazyAuto"     , CmdType::LazyAuto,   CmdCat::MEMEXE,   0,0,0,0 },
	{ "LazyStInit"   , CmdType::LazyStInit, CmdCat::MEMEXE,   0,0,0,0 },
	{ "MemEcho"      , CmdType::MemEcho,    CmdCat::MEMEXE,   1,0,0,0 },
	{ "MemDump"      , CmdType::MemDump,    CmdCat::MEMEXE,   0,0,0,0 },
	{ "ExpandOn"     , CmdType::ExpandOn,   CmdCat::MEMLAZYF, 0,0,0,0 },
	{ "ExpandOff"    , CmdType::ExpandOff,  CmdCat::MEMLAZYF, 0,0,0,0 },
};
//--- 別名設定 ---
const vector<JlscrCmdAlias> CmdAlias = {
	{ "AutoInsert"   , CmdType::AutoIns    },
	{ "AutoDelete"   , CmdType::AutoDel    },
	{ "SetF"         , CmdType::EvalFrame  },
	{ "SetT"         , CmdType::EvalTime   },
	{ "SetN"         , CmdType::EvalNum    },
};

//--- コマンド別の引数演算加工（コマンド名、引数位置、演算内容） ---
const vector<JlScrCmdCalcRecord> CmdCalcDefine = {
	{ CmdType::If,         1, ConvStrType::CondIF  },
	{ CmdType::ElsIf,      1, ConvStrType::CondIF  },
	{ CmdType::Break,      1, ConvStrType::CondIF  },
	{ CmdType::Repeat,     1, ConvStrType::Num     },
	{ CmdType::FileToMemo, 1, ConvStrType::Num     },
	{ CmdType::OldAdjust,  1, ConvStrType::Num     },
	{ CmdType::IgnoreCase, 1, ConvStrType::Num     },
	{ CmdType::SysMesDisp, 1, ConvStrType::Num     },
	{ CmdType::SysMemoSel, 1, ConvStrType::Num     },
	{ CmdType::EvalFrame,  2, ConvStrType::Frame   },
	{ CmdType::EvalTime,   2, ConvStrType::Time    },
	{ CmdType::EvalNum,    2, ConvStrType::Num     },
	{ CmdType::LocalSetF,  2, ConvStrType::Frame   },
	{ CmdType::LocalSetT,  2, ConvStrType::Time    },
	{ CmdType::LocalSetN,  2, ConvStrType::Num     },
	{ CmdType::SetParam,   2, ConvStrType::Param   },
	{ CmdType::UnitSec,    1, ConvStrType::Num     },
	{ CmdType::ListGetAt,  1, ConvStrType::Num     },
	{ CmdType::ListIns,    1, ConvStrType::Num     },
	{ CmdType::ListDel,    1, ConvStrType::Num     },
	{ CmdType::ListDim,    1, ConvStrType::Num     },
	{ CmdType::MemOnce,    1, ConvStrType::Num     },
	{ CmdType::ListSetAt,  1, ConvStrType::Num     },
	{ CmdType::LogoExact,  1, ConvStrType::Num     },
	{ CmdType::ListSel,    1, ConvStrType::NumR    },
};

//--- コマンドオプション ---
// （コマンド文字列、コマンド、補助設定、入力引数、最低必要引数、省略時設定、並び替え設定、変換種類）
const vector<JlOptionRecord> OptDefine = {
	{ "-RegPos"   , OptType::StrRegPos,     0, 1,1,0,  0, ConvStrType::None   },
	{ "-RegList"  , OptType::StrRegList,    0, 1,1,0,  0, ConvStrType::None   },
	{ "-RegSize"  , OptType::StrRegSize,    0, 1,1,0,  0, ConvStrType::None   },
	{ "-RegEnv"   , OptType::StrRegEnv,     0, 1,1,0,  0, ConvStrType::None   },
	{ "-RegOut"   , OptType::StrRegOut,     0, 1,1,0,  0, ConvStrType::None   },
	{ "-RegArg"   , OptType::StrRegArg,     0, 1,1,0,  0, ConvStrType::None   },
	{ "-val"      , OptType::StrArgVal,     0, 1,1,0,  0, ConvStrType::None   },
	{ "-counter"  , OptType::StrCounter,    0, 3,1,0,  0, ConvStrType::None   },
	{ "-filecode" , OptType::StrFileCode,   0, 1,1,0,  0, ConvStrType::None   },
	{ "-arg"      , OptType::ListArgVar,    0, 1,1,0,  0, ConvStrType::None   },
	{ "-fromabs"  , OptType::ListFromAbs,   0, 1,1,0,  0, ConvStrType::Time   },
	{ "-fromhead" , OptType::ListFromHead,  0, 1,0,0,  0, ConvStrType::Time   },
	{ "-fromtail" , OptType::ListFromTail,  0, 1,0,0,  0, ConvStrType::Time   },
	{ "-Dlist"    , OptType::ListTgDst,     0, 1,1,0,  0, ConvStrType::Time   },
	{ "-EndList"  , OptType::ListTgEnd,     0, 1,1,0,  0, ConvStrType::Time   },
	{ "-EndAbs"   , OptType::ListEndAbs,    0, 1,1,0,  0, ConvStrType::Time   },
	{ "-DstAbs"   , OptType::ListDstAbs,    0, 1,1,0,  0, ConvStrType::Time   },
	{ "-AbsSetFD" , OptType::ListAbsSetFD,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-AbsSetFE" , OptType::ListAbsSetFE,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-AbsSetFX" , OptType::ListAbsSetFX,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-AbsSetXF" , OptType::ListAbsSetXF,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-ZoneImmL" , OptType::ListZoneImmL,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-ZoneImmN" , OptType::ListZoneImmN,  0, 1,1,0,  0, ConvStrType::Time   },
	{ "-pickin"   , OptType::ListPickIn,    0, 1,1,0,  0, ConvStrType::NumR   },
	{ "-pick"     , OptType::ListPickOut,   0, 1,1,0,  0, ConvStrType::NumR   },
	{ "-N"        , OptType::LgN,           0, 1,0,0,  0, ConvStrType::None   },
	{ "-NR"       , OptType::LgNR,          0, 1,0,0,  0, ConvStrType::None   },
	{ "-Nlogo"    , OptType::LgNlogo,       0, 1,0,0,  0, ConvStrType::None   },
	{ "-Nauto"    , OptType::LgNauto,       0, 1,0,0,  0, ConvStrType::None   },
	{ "-NFlogo"   , OptType::LgNFlogo,      0, 1,0,0,  0, ConvStrType::None   },
	{ "-NFauto"   , OptType::LgNFauto,      0, 1,0,0,  0, ConvStrType::None   },
	{ "-NFXlogo"  , OptType::LgNFXlogo,     0, 1,0,0,  0, ConvStrType::None   },
	{ "-F"        , OptType::FrF,           0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-FR"       , OptType::FrFR,          0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-Fhead"    , OptType::FrFhead,       0, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-Ftail"    , OptType::FrFtail,       0, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-Fmid"     , OptType::FrFmid,        0, 2,2,0,  0, ConvStrType::MsecM1 },
	{ "-FheadX"   , OptType::FrFhead,       1, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FtailX"   , OptType::FrFtail,       1, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FmidX"    , OptType::FrFmid,        1, 2,2,0,  0, ConvStrType::MsecM1 },
	{ "-FT"       , OptType::FrF,           2, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-FTR"      , OptType::FrFR,          2, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-FThead"   , OptType::FrFhead,       2, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FTtail"   , OptType::FrFtail,       2, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FTmid"    , OptType::FrFmid,        2, 2,2,0,  0, ConvStrType::MsecM1 },
	{ "-FTheadX"  , OptType::FrFhead,       3, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FTtailX"  , OptType::FrFtail,       3, 2,1,2, 12, ConvStrType::MsecM1 },
	{ "-FTmidX"   , OptType::FrFmid,        3, 2,2,0,  0, ConvStrType::MsecM1 },
	{ "-SC"       , OptType::ScSC,          0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-NoSC"     , OptType::ScNoSC,        0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-SM"       , OptType::ScSM,          0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-NoSM"     , OptType::ScNoSM,        0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-SMA"      , OptType::ScSMA,         0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-NoSMA"    , OptType::ScNoSMA,       0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-AC"       , OptType::ScAC,          0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-NoAC"     , OptType::ScNoAC,        0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-ACC"      , OptType::ScACC,         0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-NoACC"    , OptType::ScNoACC,       0, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RSC"      , OptType::ScSC,          1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RNoSC"    , OptType::ScNoSC,        1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RSM"      , OptType::ScSM,          1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RNoSM"    , OptType::ScNoSM,        1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RSMA"     , OptType::ScSMA,         1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RNoSMA"   , OptType::ScNoSMA,       1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RAC"      , OptType::ScAC,          1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RNoAC"    , OptType::ScNoAC,        1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RACC"     , OptType::ScACC,         1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RNoACC"   , OptType::ScNoACC,       1, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RESC"     , OptType::ScSC,          2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RENoSC"   , OptType::ScNoSC,        2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RESM"     , OptType::ScSM,          2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RENoSM"   , OptType::ScNoSM,        2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RESMA"    , OptType::ScSMA,         2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RENoSMA"  , OptType::ScNoSMA,       2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-REAC"     , OptType::ScAC,          2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RENoAC"   , OptType::ScNoAC,        2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-REACC"    , OptType::ScACC,         2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RENoACC"  , OptType::ScNoACC,       2, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXSC"     , OptType::ScSC,          3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXNoSC"   , OptType::ScNoSC,        3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXSM"     , OptType::ScSM,          3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXNoSM"   , OptType::ScNoSM,        3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXSMA"    , OptType::ScSMA,         3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXNoSMA"  , OptType::ScNoSMA,       3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXAC"     , OptType::ScAC,          3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXNoAC"   , OptType::ScNoAC,        3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXACC"    , OptType::ScACC,         3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-RXNoACC"  , OptType::ScNoACC,       3, 2,0,0, 12, ConvStrType::MsecM1 },
	{ "-EndLen"   , OptType::MsecEndlenC,   0, 3,1,0, 23, ConvStrType::MsecM1 },
	{ "-EndSft"   , OptType::MsecEndSftC,   0, 3,0,0, 23, ConvStrType::MsecM1 },
	{ "-Shift"    , OptType::MsecSftC,      0, 3,0,0, 23, ConvStrType::MsecM1 },
	{ "-TgtLimit" , OptType::MsecTgtLimL,   0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-LenP"     , OptType::MsecLenPMin,   0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-LenN"     , OptType::MsecLenNMin,   0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-LenPE"    , OptType::MsecLenPEMin,  0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-LenNE"    , OptType::MsecLenNEMin,  0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-logoext"  , OptType::MsecLogoExtL,  0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-Dcenter"  , OptType::MsecDcenter,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-Dc"       , OptType::MsecDcenter,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-Drange"   , OptType::MsecDrangeL,   0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-Dr"       , OptType::MsecDrangeL,   0, 2,2,0, 12, ConvStrType::MsecM1 },
	{ "-Dmargin"  , OptType::MsecDmargin,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-Dm"       , OptType::MsecDmargin,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-Emargin"  , OptType::MsecEmargin,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-Em"       , OptType::MsecEmargin,   0, 1,1,0,  0, ConvStrType::MsecM1 },
	{ "-DstNextL" , OptType::NumDstNextL,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-DstPrevL" , OptType::NumDstPrevL,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-DstNextC" , OptType::NumDstNextC,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-DstPrevC" , OptType::NumDstPrevC,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-EndNextL" , OptType::NumEndNextL,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-EndPrevL" , OptType::NumEndPrevL,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-EndNextC" , OptType::NumEndNextC,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-EndPrevC" , OptType::NumEndPrevC,   0, 1,0,1,  0, ConvStrType::Num    },
	{ "-OverC"    , OptType::NumZOverC,     0, 1,0,1,  0, ConvStrType::Num    },
	{ "-UnderC"   , OptType::NumZUnderC,    0, 1,0,1,  0, ConvStrType::Num    },
	{ "-step"     , OptType::NumStep,       0, 1,1,0,  0, ConvStrType::Num    },
	{ "-maxsize"  , OptType::NumMaxSize,    0, 1,1,0,  0, ConvStrType::Num    },
	{ "-order"    , OptType::NumOrder,      0, 1,1,0,  0, ConvStrType::Num    },
	{ "-code"     , OptType::AutopCode,     0, 1,1,0,  0, ConvStrType::Num    },
	{ "-limit"    , OptType::AutopLimit,    0, 1,1,0,  0, ConvStrType::Num    },
	{ "-scope"    , OptType::AutopScope,    0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-scopen"   , OptType::AutopScopeN,   0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-scopex"   , OptType::AutopScopeX,   0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-period"   , OptType::AutopPeriod,   0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-maxprd"   , OptType::AutopMaxPrd,   0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-secnext"  , OptType::AutopSecNext,  0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-secprev"  , OptType::AutopSecPrev,  0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-trscope"  , OptType::AutopTrScope,  0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-trsumprd" , OptType::AutopTrSumPrd, 0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-tr1stprd" , OptType::AutopTr1stPrd, 0, 1,1,0,  0, ConvStrType::Sec    },
	{ "-info"     , OptType::AutopTrInfo,   0, 1,1,0,  0, ConvStrType::TrSpEc },
	{ "-fromC"    , OptType::FnumFromAllC,  1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromTR"   , OptType::FnumFromTr,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromSP"   , OptType::FnumFromSp,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromEC"   , OptType::FnumFromEc,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromBD"   , OptType::FnumFromBd,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromMX"   , OptType::FnumFromMx,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromTRa"  , OptType::FnumFromTra,   1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromTRr"  , OptType::FnumFromTrr,   1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromTRc"  , OptType::FnumFromTrc,   1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromAEa"  , OptType::FnumFromAea,   1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromAEc"  , OptType::FnumFromAec,   1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromCM"   , OptType::FnumFromCm,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromNL"   , OptType::FnumFromNl,    1, 0,0,0,  0, ConvStrType::None   },
	{ "-fromL"    , OptType::FnumFromL,     1, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromC"   , OptType::FnumFromAllC,  2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromTR"  , OptType::FnumFromTr,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromSP"  , OptType::FnumFromSp,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromEC"  , OptType::FnumFromEc,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromBD"  , OptType::FnumFromBd,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromMX"  , OptType::FnumFromMx,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromTRa" , OptType::FnumFromTra,   2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromTRr" , OptType::FnumFromTrr,   2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromTRc" , OptType::FnumFromTrc,   2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromAEa" , OptType::FnumFromAea,   2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromAEc" , OptType::FnumFromAec,   2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromCM"  , OptType::FnumFromCm,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromNL"  , OptType::FnumFromNl,    2, 0,0,0,  0, ConvStrType::None   },
	{ "-xfromL"   , OptType::FnumFromL,     2, 0,0,0,  0, ConvStrType::None   },
	{ "-fix"      , OptType::FlagAutopFix,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-keepC"    , OptType::FlagAutopKpC,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-keepL"    , OptType::FlagAutopKpL,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-C"        , OptType::FlagScCon,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-Coff"     , OptType::FlagScCoff,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-Cdst"     , OptType::FlagScCdst,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-Cend"     , OptType::FlagScCend,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-sort"     , OptType::FlagSort,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-silent"   , OptType::FlagSilent,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-wide"     , OptType::FlagWide,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-fromlast" , OptType::FlagFromLast,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-WithP"    , OptType::FlagWithP,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-WithN"    , OptType::FlagWithN,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-noedge"   , OptType::FlagNoEdge,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-overlap"  , OptType::FlagOverlap,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-confirm"  , OptType::FlagConfirm,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-unit"     , OptType::FlagUnit,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-else"     , OptType::FlagElse,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-cont"     , OptType::FlagCont,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-reset"    , OptType::FlagReset,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-flat"     , OptType::FlagFlat,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-force"    , OptType::FlagForce,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-noforce"  , OptType::FlagNoForce,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-preforce" , OptType::FlagNoForce,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-autochg"  , OptType::FlagAutoChg,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-autoeach" , OptType::FlagAutoEach,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndHead"  , OptType::FlagEndHead,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndTail"  , OptType::FlagEndTail,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndHold"  , OptType::FlagEndHold,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-relative" , OptType::FlagRelative,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-lazy_s"   , OptType::FlagLazyS,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-lazy_a"   , OptType::FlagLazyA,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-lazy_e"   , OptType::FlagLazyE,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-lazy"     , OptType::FlagLazyE,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-now"      , OptType::FlagNow,       0, 0,0,0,  0, ConvStrType::None   },
	{ "-nolap"    , OptType::FlagNoLap,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-EdgeS"    , OptType::FlagEdgeS,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-EdgeE"    , OptType::FlagEdgeE,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-EdgeB"    , OptType::FlagEdgeB,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-clear"    , OptType::FlagClear,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-pair"     , OptType::FlagPair,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-final"    , OptType::FlagFinal,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-local"    , OptType::FlagLocal,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-default"  , OptType::FlagDefault,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-restore"  , OptType::FlagRestore,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-unique"   , OptType::FlagUnique,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-fixpos"   , OptType::FlagFixPos,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-holdE"    , OptType::FlagHoldEnd ,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-holdB"    , OptType::FlagHoldBoth,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-ZEnd"     , OptType::FlagZEnd,      0, 0,0,0,  0, ConvStrType::None   },
	{ "-ZoneL"    , OptType::FlagZoneL,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-ZoneN"    , OptType::FlagZoneN,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-merge"    , OptType::FlagMerge,     0, 0,0,0,  0, ConvStrType::None   },
	{ "-SftLogo"  , OptType::FlagSftLogo,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-DstAnd"   , OptType::FlagDstAnd,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndAnd"   , OptType::FlagEndAnd,    0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndBase"  , OptType::FlagEndBase,   0, 0,0,0,  0, ConvStrType::None   },
	{ "-EndBaseL" , OptType::FlagEndBaseL,  0, 0,0,0,  0, ConvStrType::None   },
	{ "-dummy"    , OptType::FlagDummy,     0, 0,0,0,  0, ConvStrType::None   }
};

//--- setParamコマンドによる設定 （データ初期値はdatasetで定義） ---
const vector<ConfigDataRecord> ConfigDefine = {
	{ "WLogoTRMax"   , ConfigVarType::msecWLogoTRMax,      ConvStrType::Msec   },
	{ "WCompTRMax"   , ConfigVarType::msecWCompTRMax,      ConvStrType::Msec   },
	{ "WLogoSftMrg"  , ConfigVarType::msecWLogoSftMrg,     ConvStrType::Msec   },
	{ "WCompFirst"   , ConfigVarType::msecWCompFirst,      ConvStrType::Msec   },
	{ "WCompLast"    , ConfigVarType::msecWCompLast,       ConvStrType::Msec   },
	{ "WLogoSumMin"  , ConfigVarType::msecWLogoSumMin,     ConvStrType::Msec   },
	{ "WLogoLgMin"   , ConfigVarType::msecWLogoLgMin,      ConvStrType::Msec   },
	{ "WLogoCmMin"   , ConfigVarType::msecWLogoCmMin,      ConvStrType::Msec   },
	{ "WLogoRevMin"  , ConfigVarType::msecWLogoRevMin,     ConvStrType::Msec   },
	{ "MgnCmDetect"  , ConfigVarType::msecMgnCmDetect,     ConvStrType::Msec   },
	{ "MgnCmDivide"  , ConfigVarType::msecMgnCmDivide,     ConvStrType::Msec   },
	{ "WCompSpMin"   , ConfigVarType::secWCompSPMin,       ConvStrType::Sec    },
	{ "WCompSpMax"   , ConfigVarType::secWCompSPMax,       ConvStrType::Sec    },
	{ "CutTR"        , ConfigVarType::flagCutTR,           ConvStrType::Num    },
	{ "CutSP"        , ConfigVarType::flagCutSP,           ConvStrType::Num    },
	{ "AddLogo"      , ConfigVarType::flagAddLogo,         ConvStrType::Num    },
	{ "AddUC"        , ConfigVarType::flagAddUC,           ConvStrType::Num    },
	{ "TypeNoSc"     , ConfigVarType::typeNoSc,            ConvStrType::Num    },
	{ "CancelCntSc"  , ConfigVarType::cancelCntSc,         ConvStrType::Num    },
	{ "LogoLevel"    , ConfigVarType::LogoLevel,           ConvStrType::Num    },
	{ "LogoRevise"   , ConfigVarType::LogoRevise,          ConvStrType::Num    },
	{ "AutoCmSub"    , ConfigVarType::AutoCmSub,           ConvStrType::Num    },
	{ "PosFirst"     , ConfigVarType::msecPosFirst,        ConvStrType::MsecM1 },
	{ "LgCutFirst"   , ConfigVarType::msecLgCutFirst,      ConvStrType::MsecM1 },
	{ "ZoneFirst"    , ConfigVarType::msecZoneFirst,       ConvStrType::MsecM1 },
	{ "ZoneLast"     , ConfigVarType::msecZoneLast,        ConvStrType::MsecM1 },
	{ "LvPosFirst"   , ConfigVarType::priorityPosFirst,    ConvStrType::Num    },
};
//--- オプションの未指定時複写 ---
// （対象コマンド、複写先オプション、複写元オプション）
const vector<JlOptMirrorRecord> OptCmdMirror = {
	{ CmdType::ReadData,   OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ReadTrim,   OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ReadString, OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::EnvGet,     OptType::StrRegOut,     OptType::StrRegEnv     },
	{ CmdType::ListGetAt,  OptType::StrRegOut,     OptType::StrRegPos     },
	{ CmdType::ListIns,    OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListDel,    OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListSetAt,  OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListJoin,   OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListRemove, OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListSel,    OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListClear,  OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListDim,    OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::ListSort,   OptType::StrRegOut,     OptType::StrRegList    },
	{ CmdType::GetPos,     OptType::StrRegOut,     OptType::StrRegPos     },
	{ CmdType::GetList,    OptType::StrRegOut,     OptType::StrRegList    },
};

struct JlscrDecodeRangeRecord {		// 文字列から指定項目のミリ秒数値取得用
	int  numRead;		// 読み込むデータ数
	int  needs;			// 読み込み最低必要数
	int  numFrom;		// 省略時開始番号設定
	bool flagM1;		// -1はそのまま残す設定（0=特別扱いなし変換、1=-1は変換しない）
	bool flagSort;		// 小さい順並び替え（0=しない、1=する）
	int  numAbbr;		// （結果）省略データ数
	WideMsec wmsecVal;	// （結果）最大３項目取得ミリ秒
};
struct JlscrDecodeKeepSc {	// -SC系のオプションデータを一時保持
	OptType   type;
	int       subtype;		// 対象位置情報
	WideMsec  wmsec;		// 範囲情報
	int       abbr;			// 引数省略数
};

public:
	JlsScriptDecode(JlsDataset *pdata, JlsScrFuncList* pFuncList);
	void checkInitial();
	CmdErrType decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd);
	string getErrItem(){ return m_strErrItem; };

private:
	// デコード処理
	int  decodeCmdName(JlscrCmdRecord& cmddef, CmdErrType& errval, const string& strBuf);
	int  decodeCmdNameId(const string& cstr);
	int  decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, const JlscrCmdRecord& cmddef);
	int  decodeCmdArgOpt(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos);
	int  decodeCmdArgOptOne(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos);
	int  decodeCmdArgOptOneSub(JlsCmdArg& cmdarg, int optsel, const string& strBuf, int pos);
	int  getOptionStrMulti(JlsCmdArg& cmdarg, OptType optType, const string& strBuf, int pos);
	int  getOptionTypeList(vector<OptType>& listOptType, OptType orgOptType, int numArg);
	void castErrInternal(const string& msg);
	bool getTrSpEcID(CmdTrSpEcID& idSub, const string& strName, bool flagOption);
	int  decodeRangeMsec(JlscrDecodeRangeRecord& infoDec, const string& strBuf, int pos);
	void setRangeMargin(WideMsec& wmsecVal, Msec margin);
	bool getListStrNumFromStr(vector<string>& listStrNum, const string& strBuf);
	void sortTwoValM1(int& val_a, int& val_b);
	// デコード後の追加処理
	void reviseCmdRange(JlsCmdArg& cmdarg);
	void setCmdTackOpt(JlsCmdArg& cmdarg);
	void setArgScOpt(JlsCmdArg& cmdarg);
	void mirrorOptToUndef(JlsCmdArg& cmdarg);
	bool calcCmdArg(JlsCmdArg& cmdarg);
	// 文字列変換処理
	bool convertStringFromListStr(string& strBuf, ConvStrType typeVal);
	bool convertStringRegParam(string& strName, string& strVal);
	bool convertStringValue(string& strVal, ConvStrType typeVal);
	// エラー内容保持
	void setErrItem(const string& str){ m_strErrItem = str; };

private:
	JlsDataset *pdata;								// 入力データアクセス
	JlsScrFuncList *pFuncList;						// リスト処理
	vector<JlscrDecodeKeepSc> m_listKeepSc;			// -SC系のオプションデータを一時保持
	string m_strErrItem = "";
};
