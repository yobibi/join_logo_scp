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
		CondIF			// IF文判定式
	};

	struct JlscrCmdRecord {
		CmdType cmdsel;				// 選択コマンド
		CmdCat  category;			// コマンド種別
		int     muststr;			// 必須オプション文字列取得（0-3=取得数 9=残り全体）
		int		mustchar;			// 必須オプション文字（0=なし 1=S/E/B 2=TR/SP/EC 3=B省略可）
		int		mustrange;			// 期間指定（0=なし  1=center  3=center+left+right）
		int		needopt;			// 追加オプション（0=なし  1=読み込み）
		char	cmdname[12];		// コマンド文字列
	};
	struct JlscrCmdAlias {
		CmdType cmdsel;				// 選択コマンド
		char	cmdname[12];		// コマンド別名文字列
	};
	struct JlScrCmdCalcRecord {
		CmdType     cmdsel;			// 選択コマンド
		int         numArg;			// 引数位置
		ConvStrType typeVal;		// 変換種類
	};
	struct JlOptionRecord {
		OptType      optType;		// オプションの種類
		int          subType;		// 種類補助設定
		int          numArg;		// 引数入力数
		int          minArg;		// 引数最小必要数
		int          numFrom;		// 引数省略時開始番号設定
		int          sort;			// 引数並び替え(12=1と2, 23=2と3)
		ConvStrType  convType;		// 引数の変換処理
		char         optname[18];	// コマンド文字列
	};
	struct ConfigDataRecord {
		ConfigVarType  prmsel;
		ConvStrType    valsel;
		char           namestr[18];
	};

static const int SIZE_JLCMD_SEL = static_cast<int>(jlscmd::CmdType::MAXSIZE);
//--- コマンドリスト（コマンド、種別、文字列引数、文字引数タイプ、引数範囲数、オプション有無、文字列） ---
//--- 引数タイプ： 0=なし 1=S/E/B 2=TR/SP/EC 3=B省略可 9=残り全体 11=1引数 12=2引数 ---
const JlscrCmdRecord  CmdDefine[SIZE_JLCMD_SEL] = {
	{ CmdType::Nop,        CmdCat::NONE,     0,0,0,0, "Nop"        },
	{ CmdType::If,         CmdCat::COND,     9,0,0,0, "If"         },
	{ CmdType::EndIf,      CmdCat::COND,     0,0,0,0, "EndIf"      },
	{ CmdType::Else,       CmdCat::COND,     0,0,0,0, "Else"       },
	{ CmdType::ElsIf,      CmdCat::COND,     9,0,0,0, "ElsIf"      },
	{ CmdType::Call,       CmdCat::CALL,     1,0,0,0, "Call"       },
	{ CmdType::Repeat,     CmdCat::REP,      1,0,0,0, "Repeat"     },
	{ CmdType::EndRepeat,  CmdCat::REP,      0,0,0,0, "EndRepeat"  },
	{ CmdType::LocalSt,    CmdCat::FLOW,     0,0,0,0, "{"          },
	{ CmdType::LocalEd,    CmdCat::FLOW,     0,0,0,0, "}"          },
	{ CmdType::ArgBegin,   CmdCat::FLOW,     0,0,0,1, "ArgBegin"   },
	{ CmdType::ArgEnd,     CmdCat::FLOW,     0,0,0,0, "ArgEnd"     },
	{ CmdType::Exit,       CmdCat::FLOW,     0,0,0,0, "Exit"       },
	{ CmdType::Return,     CmdCat::FLOW,     0,0,0,0, "Return"     },
	{ CmdType::FileOpen,   CmdCat::SYS,      1,0,0,0, "FileOpen"   },
	{ CmdType::FileAppend, CmdCat::SYS,      1,0,0,0, "FileAppend" },
	{ CmdType::FileClose,  CmdCat::SYS,      0,0,0,0, "FileClose"  },
	{ CmdType::Echo,       CmdCat::SYS,      9,0,0,0, "Echo"       },
	{ CmdType::LogoOff,    CmdCat::SYS,      0,0,0,0, "LogoOff"    },
	{ CmdType::OldAdjust,  CmdCat::SYS,      1,0,0,0, "OldAdjust"  },
	{ CmdType::LogoDirect, CmdCat::SYS,      0,0,0,1, "LogoDirect" },
	{ CmdType::LogoExact,  CmdCat::SYS,      0,0,0,1, "LogoExact"  },
	{ CmdType::LogoReset,  CmdCat::SYS,      0,0,0,1, "LogoReset"  },
	{ CmdType::ReadData,   CmdCat::SYS,      1,0,0,1, "ReadData"   },
	{ CmdType::ReadTrim,   CmdCat::SYS,      1,0,0,1, "ReadTrim"   },
	{ CmdType::ReadString, CmdCat::SYS,      1,0,0,1, "ReadString" },
	{ CmdType::EnvGet,     CmdCat::SYS,      1,0,0,1, "EnvGet"     },
	{ CmdType::Set,        CmdCat::REG,      2,0,0,1, "Set"        },
	{ CmdType::Default,    CmdCat::REG,      2,0,0,1, "Default"    },
	{ CmdType::EvalFrame,  CmdCat::REG,      2,0,0,1, "EvalFrame"  },
	{ CmdType::EvalTime,   CmdCat::REG,      2,0,0,1, "EvalTime"   },
	{ CmdType::EvalNum,    CmdCat::REG,      2,0,0,1, "EvalNum"    },
	{ CmdType::CountUp,    CmdCat::REG,      1,0,0,0, "CountUp"    },
	{ CmdType::SetParam,   CmdCat::REG,      2,0,0,0, "SetParam"   },
	{ CmdType::OptSet,     CmdCat::REG,      9,0,0,0, "OptSet"     },
	{ CmdType::OptDefault, CmdCat::REG,      9,0,0,0, "OptDefault" },
	{ CmdType::UnitSec,    CmdCat::REG,      1,0,0,0, "UnitSec"    },
	{ CmdType::LocalSet,   CmdCat::REG,      2,0,0,0, "LocalSet"   },
	{ CmdType::ArgSet,     CmdCat::REG,      2,0,0,0, "ArgSet"     },
	{ CmdType::ListGetAt,  CmdCat::REG,      1,0,0,1, "ListGetAt"  },
	{ CmdType::ListIns,    CmdCat::REG,      1,0,0,1, "ListIns"    },
	{ CmdType::ListDel,    CmdCat::REG,      1,0,0,1, "ListDel"    },
	{ CmdType::ListSetAt,  CmdCat::REG,      1,0,0,1, "ListSetAt"  },
	{ CmdType::ListClear,  CmdCat::REG,      0,0,0,1, "ListClear"  },
	{ CmdType::ListSort,   CmdCat::REG,      0,0,0,1, "ListSort"   },
	{ CmdType::AutoCut,    CmdCat::AUTO,     0,2,0,1, "AutoCut"    },
	{ CmdType::AutoAdd,    CmdCat::AUTO,     0,2,0,1, "AutoAdd"    },
	{ CmdType::AutoEdge,   CmdCat::AUTOLOGO, 0,1,0,1, "AutoEdge"   },
	{ CmdType::AutoCM,     CmdCat::AUTO,     0,3,0,1, "AutoCM"     },
	{ CmdType::AutoUp,     CmdCat::AUTO,     0,3,0,1, "AutoUp"     },
	{ CmdType::AutoBorder, CmdCat::AUTO,     0,3,0,1, "AutoBorder" },
	{ CmdType::AutoIClear, CmdCat::AUTO,     0,3,0,1, "AutoIClear" },
	{ CmdType::AutoIns,    CmdCat::AUTOLOGO, 0,1,3,1, "AutoIns"    },
	{ CmdType::AutoDel,    CmdCat::AUTOLOGO, 0,1,3,1, "AutoDel"    },
	{ CmdType::Find,       CmdCat::LOGO,     0,1,3,1, "Find"       },
	{ CmdType::MkLogo,     CmdCat::LOGO,     0,1,3,1, "MkLogo"     },
	{ CmdType::DivLogo,    CmdCat::LOGO,     0,1,3,1, "DivLogo"    },
	{ CmdType::Select,     CmdCat::LOGO,     0,1,3,1, "Select"     },
	{ CmdType::Force,      CmdCat::LOGO,     0,1,1,1, "Force"      },
	{ CmdType::Abort,      CmdCat::LOGO,     0,1,0,1, "Abort"      },
	{ CmdType::GetPos,     CmdCat::LOGO,     0,1,3,1, "GetPos"     },
	{ CmdType::GetList,    CmdCat::LOGO,     0,1,3,1, "GetList"    },
	{ CmdType::NextTail,   CmdCat::NEXT,     0,1,3,1, "NextTail"   },
	{ CmdType::DivFile,    CmdCat::LOGO,     0,1,1,1, "DivFile"    },
	{ CmdType::LazyStart,  CmdCat::LAZYF,    0,0,0,1, "LazyStart"  },
	{ CmdType::EndLazy,    CmdCat::LAZYF,    0,0,0,0, "EndLazy"    },
	{ CmdType::Memory,     CmdCat::MEMF,     1,0,0,0, "Memory"     },
	{ CmdType::EndMemory,  CmdCat::MEMF,     0,0,0,0, "EndMemory"  },
	{ CmdType::MemCall,    CmdCat::MEMEXE,   1,0,0,0, "MemCall"    },
	{ CmdType::MemErase,   CmdCat::MEMEXE,   1,0,0,0, "MemErase"   },
	{ CmdType::MemCopy,    CmdCat::MEMEXE,   2,0,0,0, "MemCopy"    },
	{ CmdType::MemMove,    CmdCat::MEMEXE,   2,0,0,0, "MemMove"    },
	{ CmdType::MemAppend,  CmdCat::MEMEXE,   2,0,0,0, "MemAppend"  },
	{ CmdType::MemOnce,    CmdCat::MEMEXE,   1,0,0,0, "MemOnce"    },
	{ CmdType::LazyFlush,  CmdCat::MEMEXE,   0,0,0,0, "LazyFlush"  },
	{ CmdType::LazyAuto,   CmdCat::MEMEXE,   0,0,0,0, "LazyAuto"   },
	{ CmdType::LazyStInit, CmdCat::MEMEXE,   0,0,0,0, "LazyStInit" },
	{ CmdType::MemEcho,    CmdCat::MEMEXE,   1,0,0,0, "MemEcho"    },
	{ CmdType::MemDump,    CmdCat::MEMEXE,   0,0,0,0, "MemDump"    },
	{ CmdType::ExpandOn,   CmdCat::MEMLAZYF, 0,0,0,0, "ExpandOn"   },
	{ CmdType::ExpandOff,  CmdCat::MEMLAZYF, 0,0,0,0, "ExpandOff"  },
};
//--- 別名設定 ---
static const int SIZE_JLSCR_CMDALIAS = 2;
const JlscrCmdAlias CmdAlias[SIZE_JLSCR_CMDALIAS] = {
	{ CmdType::AutoIns,    "AutoInsert" },
	{ CmdType::AutoDel,    "AutoDelete" },
};

//--- コマンド別の引数演算加工（コマンド名、引数位置、演算内容） ---
static const int SIZE_JLCMD_CALC_DEFINE = 15;
const JlScrCmdCalcRecord CmdCalcDefine[SIZE_JLCMD_CALC_DEFINE] = {
	{ CmdType::If,         1, ConvStrType::CondIF  },
	{ CmdType::ElsIf,      1, ConvStrType::CondIF  },
	{ CmdType::Repeat,     1, ConvStrType::Num     },
	{ CmdType::OldAdjust,  1, ConvStrType::Num     },
	{ CmdType::EvalFrame,  2, ConvStrType::Frame   },
	{ CmdType::EvalTime,   2, ConvStrType::Time    },
	{ CmdType::EvalNum,    2, ConvStrType::Num     },
	{ CmdType::SetParam,   2, ConvStrType::Param   },
	{ CmdType::UnitSec,    1, ConvStrType::Num     },
	{ CmdType::ListGetAt,  1, ConvStrType::Num     },
	{ CmdType::ListIns,    1, ConvStrType::Num     },
	{ CmdType::ListDel,    1, ConvStrType::Num     },
	{ CmdType::MemOnce,    1, ConvStrType::Num     },
	{ CmdType::ListSetAt,  1, ConvStrType::Num     },
	{ CmdType::LogoExact,  1, ConvStrType::Num     },
};

//--- コマンドオプション ---
// （コマンド、補助設定、入力引数、最低必要引数、省略時設定、並び替え設定、変換種類、コマンド文字列）
static const int SIZE_JLOPT_DEFINE = 112;		// OptDefineの項目数を設定（項目数変更時は変更必須）
const JlOptionRecord  OptDefine[SIZE_JLOPT_DEFINE] = {
	{ OptType::StrRegPos,     0, 1,1,0,  0, ConvStrType::None,   "-RegPos"   },
	{ OptType::StrRegList,    0, 1,1,0,  0, ConvStrType::None,   "-RegList"  },
	{ OptType::StrRegSize,    0, 1,1,0,  0, ConvStrType::None,   "-RegSize"  },
	{ OptType::StrRegEnv,     0, 1,1,0,  0, ConvStrType::None,   "-RegEnv"   },
	{ OptType::StrArgVal,     0, 1,1,0,  0, ConvStrType::None,   "-val"      },
	{ OptType::LgN,           0, 1,1,0,  0, ConvStrType::None,   "-N"        },
	{ OptType::LgNR,          0, 1,1,0,  0, ConvStrType::None,   "-NR"       },
	{ OptType::LgNlogo,       0, 1,1,0,  0, ConvStrType::None,   "-Nlogo"    },
	{ OptType::LgNauto,       0, 1,1,0,  0, ConvStrType::None,   "-Nauto"    },
	{ OptType::LgNFlogo,      0, 1,1,0,  0, ConvStrType::None,   "-NFlogo"   },
	{ OptType::LgNFauto,      0, 1,1,0,  0, ConvStrType::None,   "-NFauto"   },
	{ OptType::LgNFXlogo,     0, 1,1,0,  0, ConvStrType::None,   "-NFXlogo"  },
	{ OptType::FrF,           0, 2,2,0, 12, ConvStrType::MsecM1, "-F"        },
	{ OptType::FrFR,          0, 2,2,0, 12, ConvStrType::MsecM1, "-FR"       },
	{ OptType::FrFhead,       0, 2,1,2, 12, ConvStrType::MsecM1, "-Fhead"    },
	{ OptType::FrFtail,       0, 2,1,2, 12, ConvStrType::MsecM1, "-Ftail"    },
	{ OptType::FrFmid,        0, 2,2,0,  0, ConvStrType::MsecM1, "-Fmid"     },
	{ OptType::FrFhead,       1, 2,1,2, 12, ConvStrType::MsecM1, "-FheadX"   },
	{ OptType::FrFtail,       1, 2,1,2, 12, ConvStrType::MsecM1, "-FtailX"   },
	{ OptType::FrFmid,        1, 2,2,0,  0, ConvStrType::MsecM1, "-FmidX"    },
	{ OptType::FrF,           2, 2,2,0, 12, ConvStrType::MsecM1, "-FT"       },
	{ OptType::FrFR,          2, 2,2,0, 12, ConvStrType::MsecM1, "-FTR"      },
	{ OptType::FrFhead,       2, 2,1,2, 12, ConvStrType::MsecM1, "-FThead"   },
	{ OptType::FrFtail,       2, 2,1,2, 12, ConvStrType::MsecM1, "-FTtail"   },
	{ OptType::FrFmid,        2, 2,2,0,  0, ConvStrType::MsecM1, "-FTmid"    },
	{ OptType::FrFhead,       3, 2,1,2, 12, ConvStrType::MsecM1, "-FTheadX"  },
	{ OptType::FrFtail,       3, 2,1,2, 12, ConvStrType::MsecM1, "-FTtailX"  },
	{ OptType::FrFmid,        3, 2,2,0,  0, ConvStrType::MsecM1, "-FTmidX"   },
	{ OptType::ScSC,          0, 2,0,0, 12, ConvStrType::MsecM1, "-SC"       },
	{ OptType::ScNoSC,        0, 2,0,0, 12, ConvStrType::MsecM1, "-NoSC"     },
	{ OptType::ScSM,          0, 2,0,0, 12, ConvStrType::MsecM1, "-SM"       },
	{ OptType::ScNoSM,        0, 2,0,0, 12, ConvStrType::MsecM1, "-NoSM"     },
	{ OptType::ScSMA,         0, 2,0,0, 12, ConvStrType::MsecM1, "-SMA"      },
	{ OptType::ScNoSMA,       0, 2,0,0, 12, ConvStrType::MsecM1, "-NoSMA"    },
	{ OptType::ScAC,          0, 2,0,0, 12, ConvStrType::MsecM1, "-AC"       },
	{ OptType::ScNoAC,        0, 2,0,0, 12, ConvStrType::MsecM1, "-NoAC"     },
	{ OptType::ScSC,          1, 2,0,0, 12, ConvStrType::MsecM1, "-RSC"      },
	{ OptType::ScNoSC,        1, 2,0,0, 12, ConvStrType::MsecM1, "-RNoSC"    },
	{ OptType::ScSM,          1, 2,0,0, 12, ConvStrType::MsecM1, "-RSM"      },
	{ OptType::ScNoSM,        1, 2,0,0, 12, ConvStrType::MsecM1, "-RNoSM"    },
	{ OptType::ScSMA,         1, 2,0,0, 12, ConvStrType::MsecM1, "-RSMA"     },
	{ OptType::ScNoSMA,       1, 2,0,0, 12, ConvStrType::MsecM1, "-RNoSMA"   },
	{ OptType::ScAC,          1, 2,0,0, 12, ConvStrType::MsecM1, "-RAC"      },
	{ OptType::ScNoAC,        1, 2,0,0, 12, ConvStrType::MsecM1, "-RNoAC"    },
	{ OptType::MsecEndlenC,   0, 3,1,0, 23, ConvStrType::MsecM1, "-EndLen"   },
	{ OptType::MsecSftC,      0, 3,1,0, 23, ConvStrType::MsecM1, "-Shift"    },
	{ OptType::MsecTgtLimL,   0, 2,2,0, 12, ConvStrType::MsecM1, "-TgtLimit" },
	{ OptType::MsecLenPMin,   0, 2,2,0, 12, ConvStrType::MsecM1, "-LenP"     },
	{ OptType::MsecLenNMin,   0, 2,2,0, 12, ConvStrType::MsecM1, "-LenN"     },
	{ OptType::MsecLenPEMin,  0, 2,2,0, 12, ConvStrType::MsecM1, "-LenPE"    },
	{ OptType::MsecLenNEMin,  0, 2,2,0, 12, ConvStrType::MsecM1, "-LenNE"    },
	{ OptType::MsecFromAbs,   0, 1,1,0,  0, ConvStrType::MsecM1, "-fromabs"  },
	{ OptType::MsecFromHead,  0, 1,0,0,  0, ConvStrType::MsecM1, "-fromhead" },
	{ OptType::MsecFromTail,  0, 1,0,0,  0, ConvStrType::MsecM1, "-fromtail" },
	{ OptType::MsecLogoExtL,  0, 2,2,0, 12, ConvStrType::MsecM1, "-logoext"  },
	{ OptType::MsecEndAbs,    0, 1,1,0,  0, ConvStrType::MsecM1, "-EndAbs"   },
	{ OptType::MsecDcenter,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Dcenter"  },
	{ OptType::MsecDcenter,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Dc"       },
	{ OptType::MsecDrangeL,   0, 2,2,0, 12, ConvStrType::MsecM1, "-Drange"   },
	{ OptType::MsecDrangeL,   0, 2,2,0, 12, ConvStrType::MsecM1, "-Dr"       },
	{ OptType::MsecDmargin,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Dmargin"  },
	{ OptType::MsecDmargin,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Dm"       },
	{ OptType::MsecEmargin,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Emargin"  },
	{ OptType::MsecEmargin,   0, 1,1,0,  0, ConvStrType::MsecM1, "-Em"       },
	{ OptType::AutopCode,     0, 1,1,0,  0, ConvStrType::Num,    "-code"     },
	{ OptType::AutopLimit,    0, 1,1,0,  0, ConvStrType::Num,    "-limit"    },
	{ OptType::AutopScope,    0, 1,1,0,  0, ConvStrType::Sec,    "-scope"    },
	{ OptType::AutopScopeN,   0, 1,1,0,  0, ConvStrType::Sec,    "-scopen"   },
	{ OptType::AutopScopeX,   0, 1,1,0,  0, ConvStrType::Sec,    "-scopex"   },
	{ OptType::AutopPeriod,   0, 1,1,0,  0, ConvStrType::Sec,    "-period"   },
	{ OptType::AutopMaxPrd,   0, 1,1,0,  0, ConvStrType::Sec,    "-maxprd"   },
	{ OptType::AutopSecNext,  0, 1,1,0,  0, ConvStrType::Sec,    "-secnext"  },
	{ OptType::AutopSecPrev,  0, 1,1,0,  0, ConvStrType::Sec,    "-secprev"  },
	{ OptType::AutopTrScope,  0, 1,1,0,  0, ConvStrType::Sec,    "-trscope"  },
	{ OptType::AutopTrSumPrd, 0, 1,1,0,  0, ConvStrType::Sec,    "-trsumprd" },
	{ OptType::AutopTr1stPrd, 0, 1,1,0,  0, ConvStrType::Sec,    "-tr1stprd" },
	{ OptType::AutopTrInfo,   0, 1,1,0,  0, ConvStrType::TrSpEc, "-info"     },
	{ OptType::FlagWide,      0, 0,0,0,  0, ConvStrType::None,   "-wide"     },
	{ OptType::FlagFromLast,  0, 0,0,0,  0, ConvStrType::None,   "-fromlast" },
	{ OptType::FlagWithP,     0, 0,0,0,  0, ConvStrType::None,   "-WithP"    },
	{ OptType::FlagWithN,     0, 0,0,0,  0, ConvStrType::None,   "-WithN"    },
	{ OptType::FlagNoEdge,    0, 0,0,0,  0, ConvStrType::None,   "-noedge"   },
	{ OptType::FlagOverlap,   0, 0,0,0,  0, ConvStrType::None,   "-overlap"  },
	{ OptType::FlagConfirm,   0, 0,0,0,  0, ConvStrType::None,   "-confirm"  },
	{ OptType::FlagUnit,      0, 0,0,0,  0, ConvStrType::None,   "-unit"     },
	{ OptType::FlagElse,      0, 0,0,0,  0, ConvStrType::None,   "-else"     },
	{ OptType::FlagCont,      0, 0,0,0,  0, ConvStrType::None,   "-cont"     },
	{ OptType::FlagReset,     0, 0,0,0,  0, ConvStrType::None,   "-reset"    },
	{ OptType::FlagFlat,      0, 0,0,0,  0, ConvStrType::None,   "-flat"     },
	{ OptType::FlagForce,     0, 0,0,0,  0, ConvStrType::None,   "-force"    },
	{ OptType::FlagNoForce,   0, 0,0,0,  0, ConvStrType::None,   "-noforce"  },
	{ OptType::FlagAutoChg,   0, 0,0,0,  0, ConvStrType::None,   "-autochg"  },
	{ OptType::FlagAutoEach,  0, 0,0,0,  0, ConvStrType::None,   "-autoeach" },
	{ OptType::FlagEndHead,   0, 0,0,0,  0, ConvStrType::None,   "-EndHead"  },
	{ OptType::FlagEndTail,   0, 0,0,0,  0, ConvStrType::None,   "-EndTail"  },
	{ OptType::FlagEndHold,   0, 0,0,0,  0, ConvStrType::None,   "-EndHold"  },
	{ OptType::FlagRelative,  0, 0,0,0,  0, ConvStrType::None,   "-relative" },
	{ OptType::FlagLazyS,     0, 0,0,0,  0, ConvStrType::None,   "-lazy_s"   },
	{ OptType::FlagLazyA,     0, 0,0,0,  0, ConvStrType::None,   "-lazy_a"   },
	{ OptType::FlagLazyE,     0, 0,0,0,  0, ConvStrType::None,   "-lazy_e"   },
	{ OptType::FlagLazyE,     0, 0,0,0,  0, ConvStrType::None,   "-lazy"     },
	{ OptType::FlagNow,       0, 0,0,0,  0, ConvStrType::None,   "-now"      },
	{ OptType::FlagNoLap,     0, 0,0,0,  0, ConvStrType::None,   "-nolap"    },
	{ OptType::FlagEdgeS,     0, 0,0,0,  0, ConvStrType::None,   "-EdgeS"    },
	{ OptType::FlagEdgeE,     0, 0,0,0,  0, ConvStrType::None,   "-EdgeE"    },
	{ OptType::FlagClear,     0, 0,0,0,  0, ConvStrType::None,   "-clear"    },
	{ OptType::FlagPair,      0, 0,0,0,  0, ConvStrType::None,   "-pair"     },
	{ OptType::FlagFinal,     0, 0,0,0,  0, ConvStrType::None,   "-final"    },
	{ OptType::FlagLocal,     0, 0,0,0,  0, ConvStrType::None,   "-local"    },
	{ OptType::FlagDefault,   0, 0,0,0,  0, ConvStrType::None,   "-default"  },
	{ OptType::FlagUnique,    0, 0,0,0,  0, ConvStrType::None,   "-unique"   },
	{ OptType::FlagDummy,     0, 0,0,0,  0, ConvStrType::None,   "-dummy"    }
};

//--- setParamコマンドによる設定 （データ初期値はdatasetで定義） ---
const ConfigDataRecord ConfigDefine[SIZE_CONFIG_VAR] = {
	{ ConfigVarType::msecWLogoTRMax,      ConvStrType::Msec,   "WLogoTRMax"  },
	{ ConfigVarType::msecWCompTRMax,      ConvStrType::Msec,   "WCompTRMax"  },
	{ ConfigVarType::msecWLogoSftMrg,     ConvStrType::Msec,   "WLogoSftMrg" },
	{ ConfigVarType::msecWCompFirst,      ConvStrType::Msec,   "WCompFirst"  },
	{ ConfigVarType::msecWCompLast,       ConvStrType::Msec,   "WCompLast"   },
	{ ConfigVarType::msecWLogoSumMin,     ConvStrType::Msec,   "WLogoSumMin" },
	{ ConfigVarType::msecWLogoLgMin,      ConvStrType::Msec,   "WLogoLgMin"  },
	{ ConfigVarType::msecWLogoCmMin,      ConvStrType::Msec,   "WLogoCmMin"  },
	{ ConfigVarType::msecWLogoRevMin,     ConvStrType::Msec,   "WLogoRevMin" },
	{ ConfigVarType::msecMgnCmDetect,     ConvStrType::Msec,   "MgnCmDetect" },
	{ ConfigVarType::msecMgnCmDivide,     ConvStrType::Msec,   "MgnCmDivide" },
	{ ConfigVarType::secWCompSPMin,       ConvStrType::Sec,    "WCompSpMin"  },
	{ ConfigVarType::secWCompSPMax,       ConvStrType::Sec,    "WCompSpMax"  },
	{ ConfigVarType::flagCutTR,           ConvStrType::Num,    "CutTR"       },
	{ ConfigVarType::flagCutSP,           ConvStrType::Num,    "CutSP"       },
	{ ConfigVarType::flagAddLogo,         ConvStrType::Num,    "AddLogo"     },
	{ ConfigVarType::flagAddUC,           ConvStrType::Num,    "AddUC"       },
	{ ConfigVarType::typeNoSc,            ConvStrType::Num,    "TypeNoSc"    },
	{ ConfigVarType::cancelCntSc,         ConvStrType::Num,    "CancelCntSc" },
	{ ConfigVarType::LogoLevel,           ConvStrType::Num,    "LogoLevel"   },
	{ ConfigVarType::LogoRevise,          ConvStrType::Num,    "LogoRevise"  },
	{ ConfigVarType::AutoCmSub,           ConvStrType::Num,    "AutoCmSub"   },
	{ ConfigVarType::msecPosFirst,        ConvStrType::MsecM1, "PosFirst"    },
	{ ConfigVarType::msecLgCutFirst,      ConvStrType::MsecM1, "LgCutFirst"  },
	{ ConfigVarType::msecZoneFirst,       ConvStrType::MsecM1, "ZoneFirst"   },
	{ ConfigVarType::msecZoneLast,        ConvStrType::MsecM1, "ZoneLast"    },
	{ ConfigVarType::priorityPosFirst,    ConvStrType::Num,    "LvPosFirst"  },
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
	bool      relative;		// false=通常、true=R付き文字列オプション
	WideMsec  wmsec;		// 範囲情報
	int       abbr;			// 引数省略数
};

public:
	JlsScriptDecode(JlsDataset *pdata);
	void checkInitial();
	CmdErrType decodeCmd(JlsCmdArg& cmdarg, const string& strBuf, bool onlyCmd);

private:
	// デコード処理
	int  decodeCmdId(const string& cstr);
	int  decodeCmdArgMust(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos, int tps, int tpc, int tpw);
	int  decodeCmdArgOpt(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos);
	int  decodeCmdArgOptOne(JlsCmdArg& cmdarg, CmdErrType& errval, const string& strBuf, int pos);
	int  decodeCmdArgOptOneSub(JlsCmdArg& cmdarg, int optsel, const string& strBuf, int pos);
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
	bool calcCmdArg(JlsCmdArg& cmdarg);
	bool convertStringRegParam(string& strName, string& strVal);
	bool convertStringValue(string& strVal, ConvStrType typeVal);

private:
	JlsDataset *pdata;								// 入力データアクセス
	vector<JlscrDecodeKeepSc> m_listKeepSc;			// -SC系のオプションデータを一時保持
};
