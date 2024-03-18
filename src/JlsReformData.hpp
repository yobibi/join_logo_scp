//
// join_logo_scp データ構成初期補正
//
#pragma once

class JlsDataset;


class JlsReformData
{
public:
	JlsReformData(JlsDataset *pdata);
	void adjustData();

private:
	void updateCutmrg();
	void adjustScpos();
	void detectCutmrg();
	void detectCutmrgDispval(string &strdisp, Msec mrg);
	Msec detectCutmrgSub(int rise);
	void adjustLogo();
	int  calcDifFunc(CalcDifInfo &calc, int msec_src, int msec_dst);
	int  calcDifExact(CalcDifInfo &calc, int msec_src, int msec_dst);
	int  getFrm30fpsFromMsec(int msec);
	int  getMsec30fpsFromFrm(int frm);

private:
	//--- 関数ポインタ作成 ---
	JlsDataset *pdata;
};
