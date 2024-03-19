//
// join_logo_scp呼び出しクラス
//
#pragma once

#include <string>
#include <vector>
#include <memory>

class JlsDataset;
class JlsScript;

//--- データ格納用 ---
class JlsIF
{
private:
	const char* ChapterStrStill = (char *) u8"＿";	// chapter_exeのマーク（画像変化なしマーク）

	//--- シーンチェンジと構成区切り用（入力データ） ---
	struct DataScpIF {
	// 読込データ
		int		msec;				// ミリ秒
		int		msbk;				// 終了位置ミリ秒
		int		msmute_s;			// シーンチェンジ無音検出開始位置
		int		msmute_e;			// シーンチェンジ無音検出終了位置
		int		still;				// 静止画
	};
	//--- ロゴデータ保持用（入力データ） ---
	struct DataLogoIF {
	// 読込データ
		int		rise;				// ロゴ各開始フレーム
		int		fall;				// ロゴ各終了フレーム
		int		rise_l;				// ロゴ各開始フレーム候補開始
		int		rise_r;				// ロゴ各開始フレーム候補終了
		int		fall_l;				// ロゴ各終了フレーム候補開始
		int		fall_r;				// ロゴ各終了フレーム候補終了
		int		fade_rise;			// 各開始フェードイン状態(0 or fadein)
		int		fade_fall;			// 各終了フェードアウト状態(0 or fadeout)
		int		intl_rise;			// インターレース状態(0:ALL 1:TOP 2:BTM)
		int		intl_fall;			// インターレース状態(0:ALL 1:TOP 2:BTM)
	};

public:
	enum ErrBootOptType {
		ERROPT_NONE    = 0,
		ERROPT_EXIT    = 1,
		ERROPT_SETTING = 2
	};
	enum ErrGetOneType {
		GETONE_ERR    = -2,
		GETONE_EXIT   = -1,
		GETONE_NONE   = 0
	};

public:
	JlsIF();
	virtual ~JlsIF();
	int start(int argc, char *argv[]);
	void setArgFull(int argc, char *argv[]);		// 引数設定（まとめて）
	void setArgEach(char *str);						// 引数設定（１個ずつ）
	int  runScript();								// JLスクリプト実行

private:
	void clear();
	int  expandArg(JlsScript &funcScript, std::vector <std::string> &listin);
	int  expandArgFromFile(JlsScript &funcScript, const std::string &fname);
	int  expandArgOne(JlsScript &funcScript, int argrest, const char* strv, const char* str1, const char* str2);
	int  readLogoframe(const std::string &fname);
	int  readScpos(const std::string &fname);
	void outputResultTrim(const std::string &outfile);
	void outputResultDetail(const std::string &outscpfile);
	void outputResultDiv(const std::string &outdivfile);
	// データ処理
	void pushRecordLogo(DataLogoIF &dt);
	void pushRecordScp(DataScpIF &dt);
	void popRecordLogo();
	int  sizeDataScp();
	bool emptyDataLogo();
	void getRecordScp(DataScpIF &dt, int n);
	void setRecordScp(DataScpIF &dt, int n);
	void outputMesErr(const std::string& mes);
	void clearRecord(DataLogoIF &dt);
	void clearRecord(DataScpIF &dt);

private:
	//--- 関数ポインタ ---
	std::unique_ptr <JlsDataset>	m_funcDataset;		// データ本体格納
	JlsDataset						*pdata;				// データポインタ

	//--- オプション保持 ---
	std::vector <std::string> m_listarg;

	//--- ファイル名保持 ---
	std::string m_logofile;			// ロゴファイル名
	std::string m_scpfile;			// 無音シーンチェンジファイル名
	std::string m_cmdfile;			// JLスクリプトファイル名
	std::string m_outfile;			// 出力結果ファイル名
	std::string m_outscpfile;		// 詳細情報出力ファイル名
	std::string m_outdivfile;		// 分割情報出力ファイル名
};

