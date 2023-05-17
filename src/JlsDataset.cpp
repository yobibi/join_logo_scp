//
// join_logo_scp �f�[�^�i�[�N���X
//

#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsDataset.hpp"

//=====================================================================
// �����ݒ�
//=====================================================================

JlsDataset::JlsDataset(){
	//--- �֐��|�C���^�쐬 ---
	this->pdata = this;
	//--- �f�[�^������ ---
	initData();
}

//---------------------------------------------------------------------
// �����l�ݒ�
//---------------------------------------------------------------------
void JlsDataset::initData(){
	m_scp.clear();
	m_logo.clear();
	resultTrim.clear();

	//--- �����ݒ� ---
	setConfig(ConfigVarType::msecWLogoTRMax      , 120*1000);
	setConfig(ConfigVarType::msecWCompTRMax      , 60*1000);
	setConfig(ConfigVarType::msecWLogoSftMrg     , 4200 );
	setConfig(ConfigVarType::msecWCompFirst      , 0    );
	setConfig(ConfigVarType::msecWCompLast       , 0    );
	setConfig(ConfigVarType::msecWLogoSumMin     , 20*1000);
	setConfig(ConfigVarType::msecWLogoLgMin      , 4500 );
	setConfig(ConfigVarType::msecWLogoCmMin      , 40*1000);
	setConfig(ConfigVarType::msecWLogoRevMin     , 185*1000);
	setConfig(ConfigVarType::msecMgnCmDetect     , 1500 );
	setConfig(ConfigVarType::msecMgnCmDivide     , 500  );
	setConfig(ConfigVarType::secWCompSPMin       , 6    );
	setConfig(ConfigVarType::secWCompSPMax       , 13   );
	setConfig(ConfigVarType::flagCutTR           , 1    );
	setConfig(ConfigVarType::flagCutSP           , 0    );
	setConfig(ConfigVarType::flagAddLogo         , 1    );
	setConfig(ConfigVarType::flagAddUC           , 0    );
	setConfig(ConfigVarType::typeNoSc            , 0    );
	setConfig(ConfigVarType::cancelCntSc         , 0    );
	setConfig(ConfigVarType::LogoLevel           , 0    );
	setConfig(ConfigVarType::LogoRevise          , 0    );
	setConfig(ConfigVarType::AutoCmSub           , 0    );
	setConfig(ConfigVarType::msecPosFirst        , -1   );
	setConfig(ConfigVarType::msecLgCutFirst      , -1   );
	setConfig(ConfigVarType::msecZoneFirst       , -1   );
	setConfig(ConfigVarType::msecZoneLast        , -1   );
	setConfig(ConfigVarType::priorityPosFirst    , 0    );

	//--- �O���ݒ�I�v�V���� ---
	extOpt = {};		// �O�̂��ߌʂɏ�����
	extOpt.verbose    = 0;
	extOpt.msecCutIn  = 0;
	extOpt.msecCutOut = 0;
	extOpt.frmLastcut = 0;
	extOpt.wideCutIn  = 0;
	extOpt.wideCutOut = 0;
	extOpt.flagNoLogo = 0;
	extOpt.fixCutIn   = 0;
	extOpt.fixCutOut  = 0;
	extOpt.fixWidCutI = 0;
	extOpt.fixWidCutO = 0;
	extOpt.oldAdjust  = 0;
	extOpt.fixVLine   = 0;
	extOpt.fixFDirect = 0;
	extOpt.fixNLgExact = 0;
	extOpt.fixSubList = 0;
	extOpt.fixSubPath = 0;
	extOpt.fixSetup   = 0;
	extOpt.vLine      = 0;
	extOpt.flagDirect = 0;
	extOpt.nLgExact   = 0;
	extOpt.dispSysMes = 0;
	extOpt.subList    = "user,<,common";	// ���������t�H���_�ݒ�
	extOpt.subPath    = "";
	extOpt.setup      = "JL_common.txt";	// ���ʐ擪���s�t�@�C��

	//--- ��ԏ����ݒ� ---
	recHold = {};		// �O�̂��ߌʂɏ�����
	recHold.msecSelect1st = -1;
	recHold.msecTrPoint   = -1;
	recHold.rmsecHeadTail = {-1, -1};
	recHold.typeRange     = 0;
	m_msecTotalMax    = 0;
	m_levelUseLogo  = 0;
	m_flagSetupAdj  = 0;
	m_flagSetupAuto = 0;
	m_nscOutDetail = 0;
	//--- �ۊǃf�[�^ ---
	backupLogosetSave();

	//--- �Œ�l�ݒ� ---
	msecValExact = 100;
	msecValNear1 = 200;
	msecValNear2 = 400;
	msecValNear3 = 1200;
	msecValLap1  = 700;
	msecValLap2  = 2500;
	msecValSpc   = 1200;
};

//---------------------------------------------------------------------
// ���S�f�[�^�S����
//---------------------------------------------------------------------
void JlsDataset::clearDataLogoAll(){
	m_logo.clear();
}

//=====================================================================
// ���S�E�V�[���`�F���W�f�[�^�̕ۑ��E�ǂݏo��
//=====================================================================

//---------------------------------------------------------------------
// �o�b�N�A�b�v�f�[�^�ۑ�
//---------------------------------------------------------------------
void JlsDataset::backupLogosetSave(){
	m_backupData.bak_scp          = m_scp;
	m_backupData.bak_logo         = m_logo;
	m_backupData.bak_msecTotalMax = m_msecTotalMax;
	m_backupData.bak_extOpt       = extOpt;
}

//---------------------------------------------------------------------
// �o�b�N�A�b�v�f�[�^�ǂݏo��
//---------------------------------------------------------------------
void JlsDataset::backupLogosetLoad(){
	//--- �ǂݍ��݃f�[�^ ---
	m_scp  = m_backupData.bak_scp;
	m_logo = m_backupData.bak_logo;

	//--- ���̏�Ԃɖ߂��O���ݒ�I�v�V���� ---
	extOpt.flagNoLogo = m_backupData.bak_extOpt.flagNoLogo;
	extOpt.fixFDirect = m_backupData.bak_extOpt.fixFDirect;
	extOpt.flagDirect = m_backupData.bak_extOpt.flagDirect;

	//--- ��ԏ����ݒ� ---
	RangeMsec rmsecBak = recHold.rmsecHeadTail;
	int  typeRangeBak  = recHold.typeRange;
	recHold = {};		// �O�̂��ߌʂɏ�����
	recHold.msecSelect1st = -1;
	recHold.msecTrPoint   = -1;
//	recHold.rmsecHeadTail = {-1, -1};
	recHold.rmsecHeadTail = rmsecBak;
	recHold.typeRange     = typeRangeBak;
	m_msecTotalMax    = m_backupData.bak_msecTotalMax;
	m_levelUseLogo  = 0;
	m_flagSetupAdj  = 0;
	m_flagSetupAuto = 0;
	m_nscOutDetail = 0;
}

//=====================================================================
// ����ݒ�̕ۑ��E�ǂݏo��
//=====================================================================

//---------------------------------------------------------------------
// config�ݒ�
//---------------------------------------------------------------------
void JlsDataset::setConfig(ConfigVarType tp, int val){
	int nTp = static_cast<int>(tp);
	m_config[nTp] = val;
}

//---------------------------------------------------------------------
// config�ݒ�l�擾
//---------------------------------------------------------------------
int JlsDataset::getConfig(ConfigVarType tp){
	int nTp = static_cast<int>(tp);
	return m_config[nTp];
}

int JlsDataset::getConfigAction(ConfigActType acttp){
	int val;
	int ret = 0;
	switch(acttp){
		case ConfigActType::LogoDelEdge:		// ���S�[��CM���f
			val = getConfig(ConfigVarType::LogoRevise);
			ret = val % 10;
			break;
		case ConfigActType::LogoDelMid:			// ���S����15�b�P��CM��
			val = getConfig(ConfigVarType::LogoRevise);
			ret = ((val / 10 % 10) & 0x1)? 1 : 0;
			break;
		case ConfigActType::LogoDelWide:		// �L�惍�S�Ȃ��폜
			val = getConfig(ConfigVarType::LogoRevise);
			ret = ((val / 10 % 10) & 0x2)? 1 : 0;
			{								// AddUC=1�̎��͖���
				int tmp = getConfig(ConfigVarType::flagAddUC);
				if ((tmp % 10) & 0x1) ret = 0;
			}
			break;
		case ConfigActType::LogoUCRemain:		// ���S�Ȃ��s���������c��
			val = getConfig(ConfigVarType::flagAddUC);
			ret = val % 10;
			break;
		case ConfigActType::LogoUCGapCm:		// CM�P�ʂ���덷���傫���\�����c��
			val = getConfig(ConfigVarType::flagAddUC);
			ret = ((val / 10 % 10) & 0x1)? 1 : 0;
			if ( isUnuseLevelLogo() ) ret = 1;		// ���S�g�p���Ȃ��ꍇ�͏펞
			break;
		case ConfigActType::MuteNoSc:			// �V�[���`�F���W�Ȃ������ʒu��CM���f
			val = getConfig(ConfigVarType::typeNoSc);
			ret = val;
			if (val == 0){					// �������f
				ret = ( pdata->isUnuseLevelLogo() )? 1 : 2;
			}
			break;
		default:
			break;
	}
	return ret;
}


//=====================================================================
// �f�[�^�T�C�Y�擾
//=====================================================================

//---------------------------------------------------------------------
// ���S�f�[�^�T�C�Y�擾
//---------------------------------------------------------------------
int JlsDataset::sizeDataLogo(){
	return (int) m_logo.size();
}
//---------------------------------------------------------------------
// ����SC�f�[�^�T�C�Y�擾
//---------------------------------------------------------------------
int JlsDataset::sizeDataScp(){
	return (int) m_scp.size();
}

//---------------------------------------------------------------------
// ���S�f�[�^����H
//---------------------------------------------------------------------
bool JlsDataset::emptyDataLogo(){
	return m_logo.empty();
}

//---------------------------------------------------------------------
// ����SC�S�̃f�[�^�ԍ��擾�i�[���܂ނ��I��t���j
//---------------------------------------------------------------------
RangeNsc JlsDataset::getRangeNscTotal(bool flagNoEdge){
	int rev = ( flagNoEdge )? 1 : 0;
	RangeNsc rnsc;
	rnsc.st = rev;
	rnsc.ed = sizeDataScp() - 1 - rev;
	return rnsc;
}


//=====================================================================
// �P�f�[�^�Z�b�g�P�ʂ̏���
//=====================================================================

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʏ������i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::clearRecordLogo(DataLogoRecord &dt){
	dt.org_rise     = 0;
	dt.org_fall     = 0;
	dt.org_rise_l   = 0;
	dt.org_rise_r   = 0;
	dt.org_fall_l   = 0;
	dt.org_fall_r   = 0;
	dt.rise         = 0;
	dt.fall         = 0;
	dt.rise_l       = 0;
	dt.rise_r       = 0;
	dt.fall_l       = 0;
	dt.fall_r       = 0;
	dt.fade_rise    = 0;
	dt.fade_fall    = 0;
	dt.intl_rise    = 0;
	dt.intl_fall    = 0;
	dt.stat_rise    = LOGO_PRIOR_NONE;
	dt.stat_fall    = LOGO_PRIOR_NONE;
	dt.unit_rise    = LOGO_UNIT_NORMAL;
	dt.unit_fall    = LOGO_UNIT_NORMAL;
	dt.outtype_rise = LOGO_RESULT_NONE;
	dt.outtype_fall = LOGO_RESULT_NONE;
	dt.result_rise  = 0;
	dt.result_fall  = 0;
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʏ������i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::clearRecordScp(DataScpRecord &dt){
	dt.msec     = 0;
	dt.msbk     = 0;
	dt.msmute_s = -1;
	dt.msmute_e = -1;
	dt.still    = 0;
	dt.statpos  = SCP_PRIOR_NONE;
	dt.score    = 0;
	dt.chap     = SCP_CHAP_NONE;
	dt.arstat   = SCP_AR_UNKNOWN;
	dt.arext    = SCP_AREXT_NONE;
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʑ}���i�Ō�̈ʒu�j�i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::pushRecordLogo(DataLogoRecord &dt){
	m_logo.push_back(dt);						// add data
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʑ}���i�Ō�̈ʒu�j�i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::pushRecordScp(DataScpRecord &dt){
	m_scp.push_back(dt);						// add data
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʍ폜�i�Ō�̈ʒu�j�i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::popRecordLogo(){
	m_logo.pop_back();							// delete data
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʑ}���i�w��ʒu�j�i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::insertRecordLogo(DataLogoRecord &dt, Nlg nlg){
	if (nlg >= 0 && nlg <= sizeDataLogo()){
		m_logo.insert(m_logo.begin()+nlg, dt);
	}
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʑ}���i�w��ʒu�j�i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::insertRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc <= sizeDataScp()){
		m_scp.insert(m_scp.begin()+nsc, dt);
	}
}
//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʍ폜�i�w��ʒu�j�i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::deleteRecordScp(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp.erase(m_scp.begin()+nsc);
	}
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʎ擾�i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::getRecordLogo(DataLogoRecord &dt, Nsc nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		dt = m_logo[nlg];
	}
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʕύX�i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::setRecordLogo(DataLogoRecord &dt, Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		m_logo[nlg] = dt;
	}
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʎ擾�i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::getRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		dt = m_scp[nsc];
	}
}

//---------------------------------------------------------------------
// �P�f�[�^�Z�b�g�P�ʕύX�i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::setRecordScp(DataScpRecord &dt, Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc] = dt;
	}
}



//=====================================================================
// �P�v�f�P�ʂ̏���
//=====================================================================

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i���S�f�[�^�����オ��j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoRise(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		return m_logo[nlg].rise;
	}
	return 0;
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i���S�f�[�^����������j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoFall(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		return m_logo[nlg].fall;
	}
	return 0;
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i���S�f�[�^�G�b�W�j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecLogoNrf(Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			return m_logo[n].rise;
		}
		else{
			return m_logo[n].fall;
		}
	}
	return 0;
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i�s���m�̈���܂߂����S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::getMsecLogoNrfWide(int &msec_c, int &msec_l, int &msec_r, Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			msec_c = m_logo[n].rise;
			msec_l = m_logo[n].rise_l;
			msec_r = m_logo[n].rise_r;
		}
		else{
			msec_c = m_logo[n].fall;
			msec_l = m_logo[n].fall_l;
			msec_r = m_logo[n].fall_r;
		}
	}
	else{
		msec_c = 0;
		msec_l = 0;
		msec_r = 0;
	}
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i�s���m�̈���܂߂����S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::getWideMsecLogoNrf(WideMsec &wmsec, Nrf nrf){
	Nlg n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			wmsec.just  = m_logo[n].rise;
			wmsec.early = m_logo[n].rise_l;
			wmsec.late  = m_logo[n].rise_r;
		}
		else{
			wmsec.just  = m_logo[n].fall;
			wmsec.early = m_logo[n].fall_l;
			wmsec.late  = m_logo[n].fall_r;
		}
	}
	else{
		wmsec.just  = 0;
		wmsec.early = 0;
		wmsec.late  = 0;
	}
}

//---------------------------------------------------------------------
// �~���b�f�[�^�ύX�i���S�f�[�^�G�b�W�j
//---------------------------------------------------------------------
void JlsDataset::setMsecLogoNrf(Nrf nrf, Msec val){
	int n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			m_logo[n].rise = val;
			if (m_logo[n].rise_l > val){
				m_logo[n].rise_l = val;
			}
			if (m_logo[n].rise_r < val){
				m_logo[n].rise_r = val;
			}
		}
		else{
			m_logo[n].fall = val;
			if (m_logo[n].fall_l > val){
				m_logo[n].fall_l = val;
			}
			if (m_logo[n].fall_r < val){
				m_logo[n].fall_r = val;
			}
		}
	}
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i����SC�ʒu�j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScp(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].msec;
	}
	return 0;
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i����SC ���O�I���ʒu�j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScpBk(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].msbk;
	}
	return 0;
}

//---------------------------------------------------------------------
// �~���b�f�[�^�擾�i����SC �J�n�^���O�I���ʒu�̑I��t���j
//---------------------------------------------------------------------
Msec JlsDataset::getMsecScpEdge(Nsc nsc, LogoEdgeType edge){
	if (edge == LOGO_EDGE_FALL){
		return getMsecScpBk(nsc);
	}
	return getMsecScp(nsc);
}

//---------------------------------------------------------------------
// �~���b�f�[�^���J�n�I��������Ŏ擾�i����SC�ʒu�j
//---------------------------------------------------------------------
WideMsec JlsDataset::getWideMsecScp(Nsc nsc){
	Msec msecT  = getMsecScp(nsc);
	Msec msecBk = getMsecScpBk(nsc);
	WideMsec wmsec;
	wmsec.just  = msecT;	// ���S�͊�ʒu
	wmsec.early = msecBk;
	wmsec.late  = msecT;
	return wmsec;
}

//---------------------------------------------------------------------
// �͈̓~���b�f�[�^�擾�i�͈͖����V�[���`�F���W�ԍ�����j
//---------------------------------------------------------------------
RangeMsec JlsDataset::getRangeMsecFromRangeNsc(RangeNsc rnsc){
	RangeMsec rmsec;
	rmsec.st = pdata->getMsecScp(rnsc.st);
	rmsec.ed = pdata->getMsecScp(rnsc.ed);
	if (rnsc.st < 0) rmsec.st = -1;
	if (rnsc.ed < 0) rmsec.ed = -1;
	return rmsec;
}

//---------------------------------------------------------------------
// �摜�ω��Ȃ��t���O�擾
//---------------------------------------------------------------------
bool JlsDataset::getScpStill(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].still;
	}
	return false;
}

//---------------------------------------------------------------------
// ��؂��Ԏ擾
//---------------------------------------------------------------------
jlsd::ScpPriorType JlsDataset::getScpStatpos(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].statpos;
	}
	return SCP_PRIOR_NONE;
}

//---------------------------------------------------------------------
// �\�������p �X�R�A�擾
//---------------------------------------------------------------------
int JlsDataset::getScpScore(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].score;
	}
	return 0;
}

//---------------------------------------------------------------------
// �\�������p ��؂��Ԏ擾
//---------------------------------------------------------------------
jlsd::ScpChapType JlsDataset::getScpChap(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].chap;
	}
	return SCP_CHAP_NONE;
}

//---------------------------------------------------------------------
// �\�������p �\�����e�擾
//---------------------------------------------------------------------
jlsd::ScpArType JlsDataset::getScpArstat(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].arstat;
	}
	return SCP_AR_UNKNOWN;
}

//---------------------------------------------------------------------
// �\�������p �\�����e�擾
//---------------------------------------------------------------------
jlsd::ScpArExtType JlsDataset::getScpArext(Nsc nsc){
	if (nsc >= 0 && nsc < sizeDataScp()){
		return m_scp[nsc].arext;
	}
	return SCP_AREXT_NONE;
}

//---------------------------------------------------------------------
// ���S���ʎ擾
//---------------------------------------------------------------------
bool JlsDataset::getResultLogoAtNrf(Msec &msec, LogoResultType &outtype, Nrf nrf){
	if (nrf >= 0 && nrf/2 < sizeDataLogo()){
		if (nrf%2 == 0){
			msec = m_logo[nrf/2].result_rise;
			outtype = m_logo[nrf/2].outtype_rise;
		}
		else{
			msec = m_logo[nrf/2].result_fall;
			outtype = m_logo[nrf/2].outtype_fall;
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
// ��؂��Ԑݒ�
//---------------------------------------------------------------------
void JlsDataset::setScpStatpos(Nsc nsc, ScpPriorType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].statpos = val;
	}
}

//---------------------------------------------------------------------
// �\�������p �X�R�A�ݒ�
//---------------------------------------------------------------------
void JlsDataset::setScpScore(Nsc nsc, int val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].score = val;
	}
}

//---------------------------------------------------------------------
// �\�������p ��؂��Ԑݒ�
//---------------------------------------------------------------------
void JlsDataset::setScpChap(Nsc nsc, ScpChapType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].chap = val;
	}
}

//---------------------------------------------------------------------
// �\�������p �\�����e�ݒ�
//---------------------------------------------------------------------
void JlsDataset::setScpArstat(Nsc nsc, ScpArType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].arstat = val;
		m_scp[nsc].arext  = SCP_AREXT_NONE;		// �g��������������
	}
}

//---------------------------------------------------------------------
// �\�������p �\�����e�g���ݒ�
//---------------------------------------------------------------------
void JlsDataset::setScpArext(Nsc nsc, ScpArExtType val){
	if (nsc >= 0 && nsc < sizeDataScp()){
		m_scp[nsc].arext = val;
	}
}


//---------------------------------------------------------------------
// ���S���ʐݒ�
//---------------------------------------------------------------------
void JlsDataset::setResultLogoAtNrf(Msec msec, LogoResultType outtype, Nrf nrf){
	if (nrf >= 0 && nrf/2 < sizeDataLogo()){
		if (nrf%2 == 0){
			m_logo[nrf/2].result_rise = msec;
			m_logo[nrf/2].outtype_rise = outtype;
		}
		else{
			m_logo[nrf/2].result_fall = msec;
			m_logo[nrf/2].outtype_fall = outtype;
		}
	}
}



//=====================================================================
// �D��x�擾����
//=====================================================================

//---------------------------------------------------------------------
// ���S�f�[�^�̌��Ƃ��ėD��x�擾
//---------------------------------------------------------------------
jlsd::LogoPriorType JlsDataset::getPriorLogo(Nrf nrf){
	int n = nrf/2;
	if (n >= 0 && n < sizeDataLogo()){
		if (nrf%2 == 0){
			return m_logo[n].stat_rise;
		}
		else{
			return m_logo[n].stat_fall;
		}
	}
	return LOGO_PRIOR_NONE;
}

//---------------------------------------------------------------------
// ����SC�̌��Ƃ��ėD��x�擾
// Auto�R�}���h�ɂ�鐄���L���Ŏ擾�f�[�^��ύX
//---------------------------------------------------------------------
jlsd::ScpPriorType JlsDataset::getPriorScp(Nsc nsc){
	if (nsc >= 0 && nsc < (int) m_scp.size()){
		if (m_flagSetupAuto <= 1){
			return m_scp[nsc].statpos;
		}
		else{
			if (m_scp[nsc].chap >= SCP_CHAP_DECIDE){
				return SCP_PRIOR_DECIDE;
			}
			else if (m_scp[nsc].chap > SCP_CHAP_NONE){
				return SCP_PRIOR_LV1;
			}
			else if (m_scp[nsc].chap < SCP_CHAP_NONE){
				return SCP_PRIOR_DUPE;
			}
		}
	}
	return SCP_PRIOR_NONE;
}



//=====================================================================
// �O��f�[�^�擾�����i���S�j
//=====================================================================

//---------------------------------------------------------------------
// ���̃��S�ʒu�擾
// ���́F
//   nrf  : ���S�ԍ�*2 + fall����1
//   dr   : ���������i�O�� / �㑤�j
//   edge : ���S�[�i0:�����オ��G�b�W  1:����������G�b�W  2:���G�b�W�j
//   type : 0:���ׂ�  1:�L�����S
// �Ԃ�l�F ���̃��S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
Nrf JlsDataset::getNrfDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type){
	int size_logo = sizeDataLogo();
	int r = -1;
	bool flag_end = false;
	bool check_rise = isLogoEdgeRise(edge);
	bool check_fall = isLogoEdgeFall(edge);
	int i = nrf;
	int step = (dr == SEARCH_DIR_NEXT)? +1 : -1;
	while(flag_end == false){
		i += step;
		if (i >= 0 && i < size_logo*2){
			int nlg_i  = nlgFromNrf(i);
			int edge_i = edgeFromNrf(i);
			if (edge_i == LOGO_EDGE_RISE && check_rise){
				if (isValidLogoRise(nlg_i) || type == LOGO_SELECT_ALL){
					r = i;
					flag_end = true;
				}
			}
			else if (edge_i == LOGO_EDGE_FALL && check_fall){
				if (isValidLogoFall(nlg_i) || type == LOGO_SELECT_ALL){
					r = i;
					flag_end = true;
				}
			}
		}
		else{
			flag_end = true;
		}
	};
	return r;
}

// �P�O�̃��S�ʒu�擾
Nrf JlsDataset::getNrfPrevLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type){
	return getNrfDirLogo(nrf, SEARCH_DIR_PREV, edge, type);
}

// �P��̃��S�ʒu�擾
Nrf JlsDataset::getNrfNextLogo(Nrf nrf, LogoEdgeType edge, LogoSelectType type){
	return getNrfDirLogo(nrf, SEARCH_DIR_NEXT, edge, type);
}

//---------------------------------------------------------------------
// ���̃��S�ʒu�擾�i�������Ɨ�������̃Z�b�g�j
// ���́F
//   nfall : ���S�ԍ�*2 + fall����1
//   type  : 0:���ׂ�  1:�L�����S
// �Ԃ�l�F ���S�ʒu�擾���ʁi�擾��:true�j
//   nrise : ���̗���胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//   nfall : ���̗����胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
bool JlsDataset::getNrfNextLogoSet(Nrf &nrf_rise, Nrf &nrf_fall, LogoSelectType type){
	nrf_rise = getNrfNextLogo(nrf_fall, LOGO_EDGE_RISE, type);
	if (nrf_rise >= 0){
		nrf_fall = getNrfNextLogo(nrf_rise, LOGO_EDGE_FALL, type);
	}
	else{
		nrf_fall = -1;
	}
	if (nrf_fall < 0) return false;
	return true;
}


//---------------------------------------------------------------------
// ���̐����\�����S�����ʒu�擾�i�ŏI�o�͔�����͂��j
// ���́F
//   nsc  : �V�[���`�F���W�ԍ�
//   dr   : ���������i�O�� / �㑤�j
//   edge : ���S�[�i0:�����オ��G�b�W  1:����������G�b�W  2:���G�b�W�j
//   flag_border : Border�̈�����S�Ɋ܂߂�
//   flag_out    : �o�͗p�i0:�����\�z�p�\��  1:�o�͗p�\���j
// �Ԃ�l�F ���̃��S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirElgForAll(Nsc nsc, SearchDirType dr, LogoEdgeType edge, bool flag_border, bool flag_out){
	int size_scp = sizeDataScp();
	int r = -1;
	int i = nsc;
	if (dr == SEARCH_DIR_NEXT){			// arstat�͂Q�_�Ԃ̌㑤�����邽�߂̈ړ�
		i = getNscNextScpDecide(i,  SCP_END_EDGEIN);
	}
	//--- �t�G�b�W�����܂ŒT�� ---
	bool inlogo_base = isElgInScpForAll(i, flag_border, flag_out);
	{
		int inext = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
		int icheck = (dr == SEARCH_DIR_NEXT)? i : inext;
		while(((edge == LOGO_EDGE_RISE && dr == SEARCH_DIR_NEXT && inlogo_base == true) ||
			   (edge == LOGO_EDGE_RISE && dr == SEARCH_DIR_PREV && inlogo_base == false) ||
			   (edge == LOGO_EDGE_FALL && dr == SEARCH_DIR_NEXT && inlogo_base == false) ||
			   (edge == LOGO_EDGE_FALL && dr == SEARCH_DIR_PREV && inlogo_base == true))
			  && ( isElgDivScpForAll(icheck, flag_border, flag_out) == false )	// ���S������؂�
			  && i >= 0){
			i = inext;
			inext = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
			icheck = (dr == SEARCH_DIR_NEXT)? i : inext;
			inlogo_base = isElgInScpForAll(i, flag_border, flag_out);
		}
	}
	//--- �G�b�W������T�� ---
	if ((i > 0 && i < size_scp) ||
		(i == 0 && dr == SEARCH_DIR_NEXT) ||
		(i == size_scp && dr == SEARCH_DIR_PREV)){
		bool flag_end = false;
		while(flag_end == false){
			int ilast = i;
			i = getNscDirScpDecide(i, dr, SCP_END_EDGEIN);
			if (i > 0 && i < size_scp){
				bool inlogo_i = isElgInScpForAll(i, flag_border, flag_out);
				int  iset = i;
				if (dr == SEARCH_DIR_NEXT){
					iset = ilast;					// �Q�_�Ԃ̑O��
				}
				if (inlogo_i != inlogo_base){		// �G�b�W�ω�����Ίm��
					flag_end = true;
					r = iset;
				}
				else if ( isElgDivScpForAll(iset, flag_border, flag_out) ){	// ���S������؂�
					flag_end = true;
					r = iset;
				}
			}
			else{
				flag_end = true;
				if (inlogo_base == true){			// ���S���ŏI��������[�ݒ�
					if (dr == SEARCH_DIR_NEXT){
						r = size_scp-1;
					}
					else{
						r = 0;
					}
				}
			}
		}
	};
	return r;
}


//---------------------------------------------------------------------
// ���̐����\�����S�����ʒu�擾
// ���́F
//   nsc  : �V�[���`�F���W�ԍ�
//   edge : ���S�[�i0:�����オ��G�b�W  1:����������G�b�W  2:���G�b�W�j
// �Ԃ�l�F ���̃��S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
// �P�O�̈ʒu�擾
Nsc JlsDataset::getNscPrevElg(Nsc nsc, LogoEdgeType edge){
	return getNscDirElgForAll(nsc, SEARCH_DIR_PREV, edge, false, false);
}

// �P��̈ʒu�擾
Nsc JlsDataset::getNscNextElg(Nsc nsc, LogoEdgeType edge){
	return getNscDirElgForAll(nsc, SEARCH_DIR_NEXT, edge, false, false);
}

//---------------------------------------------------------------------
// ���̃��S�ʒu�擾�i�������Ɨ�������̃Z�b�g�j
// ���́F
//   nrf_fall : ���S�ԍ�*2 + fall����1
//   type     : 0:���ׂ�  1:�L�����S
// �Ԃ�l�F ���S�ʒu�擾���ʁi�擾��:true�j
//   nrf_rise : ���̗���胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//   nrf_fall : ���̗����胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
bool JlsDataset::getNrfptNext(NrfCurrent &logopt, LogoSelectType type){
	//--- initialize ---
	if (logopt.valid == false){
		logopt.nrfRise = -1;
		logopt.nrfFall = -1;
		logopt.valid = true;
	}
	//--- backup ---
	logopt.nrfLastRise = logopt.nrfRise;
	logopt.nrfLastFall = logopt.nrfFall;
	logopt.msecLastRise = logopt.msecRise;
	logopt.msecLastFall = logopt.msecFall;
	//--- rise ---
	logopt.nrfRise = getNrfNextLogo(logopt.nrfFall, LOGO_EDGE_RISE, type);
	if (logopt.nrfRise >= 0){
		logopt.nrfFall = getNrfNextLogo(logopt.nrfRise, LOGO_EDGE_FALL, type);
	}
	else{
		logopt.nrfFall = -1;
	}
	logopt.msecRise = getMsecLogoNrf(logopt.nrfRise);
	logopt.msecFall = getMsecLogoNrf(logopt.nrfFall);
	if (logopt.nrfFall < 0) return false;
	return true;
}


//---------------------------------------------------------------------
// ���̐����\�����S�����ʒu�擾�i�������Ɨ�������̃Z�b�g�j
// ���́F
//   nsc_fall : �O�̗���������V�[���`�F���W�ԍ�
// �Ԃ�l�F ���S�ʒu�擾���ʁi�擾��:true�j
//   nsc_rise : ���̗���胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//   nsc_fall : ���̗����胍�S�ʒu�i�Ȃ��ꍇ��-1�j
//---------------------------------------------------------------------
bool JlsDataset::getElgptNext(ElgCurrent &elg){
	bool flag1st = false;
	//--- initialize ---
	if (elg.valid == false){
		elg.nscFall = 0;
		elg.valid = true;
		flag1st = true;
	}
	//--- backup ---
	elg.nscLastRise  = elg.nscRise;
	elg.nscLastFall  = elg.nscFall;
	elg.msecLastRise = elg.msecRise;
	elg.msecLastFall = elg.msecFall;
	//--- rise ---
	if ( isElgDivScpForAll(elg.nscFall, elg.border, elg.outflag) == false ){	// ���S�������ʒu���`�F�b�N
		if (flag1st) elg.nscFall = -1;		// 0�ʒu����`�F�b�N���邽��
		elg.nscRise = getNscDirElgForAll(elg.nscFall, SEARCH_DIR_NEXT, LOGO_EDGE_RISE, elg.border, elg.outflag);
	}
	else{
		elg.nscRise = elg.nscFall;
	}
	elg.msecRise = getMsecScp(elg.nscRise);
	//--- fall ---
	if (elg.nscRise >= 0){
		elg.nscFall = getNscDirElgForAll(elg.nscRise, SEARCH_DIR_NEXT, LOGO_EDGE_FALL, elg.border, elg.outflag);
	}
	else{
		elg.nscFall = -1;
	}
	if (elg.outflag){					// �ŏI�o�͎��͐��m�ɗ���������ʒu
		elg.msecFall = getMsecScpBk(elg.nscFall);
	}
	else{								// �ʏ�̓V�[���`�F���W�ԍ��ŋ��ʈʒu�F��
		elg.msecFall = getMsecScp(elg.nscFall);
	}
	//--- end ---
	if (elg.nscFall < 0) return false;
	return true;
}

//---------------------------------------------------------------------
// ���S�̌��ʈʒu�i���̈ʒu�j���擾
// ���́F
//   nlg    : �������J�n���郍�S�ԍ��i�w��ʒu���܂ށj
// �o�́F
//   �Ԃ�l�F�擾�������S�̎��̃��S�ԍ��i-1�̎��͊Y���Ȃ��j
//   msec_rise : �����オ��ʒu�i�~���b�j
//   msec_rise : ����������ʒu�i�~���b�j
//   cont_next : ���̃��S���؂�ڂȂ��̘A����
//---------------------------------------------------------------------
Nlg JlsDataset::getResultLogoNext(Msec &msec_rise, Msec &msec_fall, bool &cont_next, Nlg nlg){
	int msec_val_cont = 80;					// ���ꃍ�S�Ƃ݂Ȃ��Ԋumsec

	int size_logo = sizeDataLogo();
	cont_next = false;
	msec_rise = -1;
	msec_fall = -1;
	//--- �����オ��G�b�W������ ---
	Nlg nlg_rise = nlg;
	bool flag_rise = false;
	while(nlg_rise >= 0 && nlg_rise < size_logo && flag_rise == false){
		DataLogoRecord dtlogo;
		getRecordLogo(dtlogo, nlg_rise);
		if (dtlogo.outtype_rise == LOGO_RESULT_DECIDE){				// �m��G�b�W
			flag_rise = true;
			msec_rise = dtlogo.result_rise;
		}
		if (flag_rise == false){
			nlg_rise ++;
		}
	}
	//--- ����������G�b�W������ ---
	Nlg nlg_fall = nlg_rise;
	bool flag_fall = false;
	bool flag_unit = false;
	if (flag_rise == false){
		nlg_fall = nlg;
	}
	while(nlg_fall >= 0 && nlg_fall < size_logo && flag_fall == false){
		DataLogoRecord dtlogo;
		getRecordLogo(dtlogo, nlg_fall);
		if (dtlogo.outtype_fall == LOGO_RESULT_DECIDE){				// �m��G�b�W
			flag_fall = true;
			msec_fall = dtlogo.result_fall;
			if (dtlogo.unit_fall){
				flag_unit = true;
			}
		}
		if (flag_fall == false){
			nlg_fall ++;
		}
	}
	//--- ���̃��S�����ꃍ�S���m�F ---
	if (flag_fall){
		Nlg  nlg_next = nlg_fall + 1;
		bool flag_next = false;
		while(nlg_next >= 0 && nlg_next < size_logo && flag_next == false){
			DataLogoRecord dtlogo;
			getRecordLogo(dtlogo, nlg_next);
			if (dtlogo.outtype_rise == LOGO_RESULT_DECIDE){				// �m��G�b�W
				flag_next = true;
				int msec_next = dtlogo.result_rise;
				if (msec_fall + msec_val_cont > msec_next){				// ���ꃍ�S�ŕ␳����
					if ( (extOpt.nLgExact & 0x02) != 0 ){
						msec_fall = cnv.getMsecAdjustFrmFromMsec(msec_next, -1);	// �؂�ڂȂ��ɕ␳
					}
					if (flag_unit == false){					// �Ɨ����S�łȂ���ΐ؂�ڂȂ�����
						if ( dtlogo.unit_rise == false ){
							cont_next = true;
						}
					}
				}
			}
			nlg_next ++;
		}
	}
	//--- ���� ---
	Nlg nlg_ret = -1;
	//--- ����擾�� ---
	if (flag_rise == true && flag_fall == true){
		nlg_ret = nlg_fall + 1;
	}
	else{
		//--- ���������肪�Ȃ��ꍇ ---
		if ((flag_rise == true || nlg == 0) && flag_fall == false){
			nlg_ret = size_logo;
			msec_fall = getMsecTotalMax();
		}
		//--- �ŏ����烍�S���S���Ȃ��ꍇ ---
		if (nlg == 0){
			msec_rise = 0;
		}
	}
	return nlg_ret;
}

// �o�̓��S�Ή�
//---------------------------------------------------------------------
// ���̃��S�ʒu�擾�i�������Ɨ�������̃Z�b�g�j
// ���́F
//   nrf_fall : ���S�ԍ�*2 + fall����1
//   type     : ALL=���ׂ�  VALID=�L�����S
//   final    : false=�ʏ�  true=�ŏI�o��
// �Ԃ�l�F ���S�ʒu�擾���ʁi�擾��=true�j
//   logopt   : ���S�ʒu���
//---------------------------------------------------------------------
bool JlsDataset::getNrfptOutNext(NrfCurrent &logopt, LogoSelectType type, bool final){
	//--- initialize ---
	if (logopt.valid == false){
		logopt.nrfRise = -1;
		logopt.nrfFall = -1;
		logopt.valid = true;
	}
	//--- backup ---
	logopt.nrfLastRise = logopt.nrfRise;
	logopt.nrfLastFall = logopt.nrfFall;
	logopt.msecLastRise = logopt.msecRise;
	logopt.msecLastFall = logopt.msecFall;
	//--- rise ---
	logopt.nrfRise = getNrfMsecOutNextLogo(logopt.msecRise, logopt.nrfFall, LOGO_EDGE_RISE, type, final);
	if (logopt.nrfRise >= 0){
		logopt.nrfFall = getNrfMsecOutNextLogo(logopt.msecFall, logopt.nrfRise, LOGO_EDGE_FALL, type, final);
		if ( logopt.nrfFall < 0 ){
			logopt.nrfFall  = sizeDataLogo()*2-1;
			logopt.msecFall = getMsecTotalMax();
		}
	}
	else{
		logopt.nrfFall = -1;
		return false;
	}
	return true;
}
// unit�ݒ���l�������o�͗p�̂P��̃��S�ʒu�擾
Nrf JlsDataset::getNrfMsecOutNextLogo(Msec& msecOut, Nrf nrf, LogoEdgeType edge, LogoSelectType type, bool final){
	int msec_val_cont = 80;					// ���ꃍ�S�Ƃ݂Ȃ��Ԋumsec

	int r = -1;
	Msec msecRef = ( nrf >= 0 )? getMsecLogoNrf(nrf) : 0;
	Nrf nrfNext = nrf;
	bool flag_end = false;
	while(flag_end == false){
		nrfNext = getNrfOutDirLogo(nrfNext, SEARCH_DIR_NEXT, edge, type, final);
		r = nrfNext;
		msecOut = getMsecLogoNrf(nrfNext);
		flag_end = true;
		if ( nrfNext < 0 || final == false ){
			break;
		}
		//--- �ŏI�o�͗p���� ---
		DataLogoRecord dtlogo1;
		getRecordLogo(dtlogo1, nlgFromNrf(nrfNext));
		bool flagRise = isLogoEdgeRiseFromNrf(nrfNext);
		msecOut = ( flagRise )? dtlogo1.result_rise : dtlogo1.result_fall;
		if ( msecOut < msecRef ){
			flag_end = false;
		}
		//--- ���̃��S�����ꃍ�S���m�F ---
		if ( flagRise == false ){
			Nrf nrfRise = getNrfOutDirLogo(nrfNext, SEARCH_DIR_NEXT, LOGO_EDGE_RISE, type, final);
			if ( nrfRise >= 0 ){
				DataLogoRecord dtlogo2;
				getRecordLogo(dtlogo2, nlgFromNrf(nrfRise));
				if ( dtlogo2.result_rise - dtlogo1.result_fall < msec_val_cont ){
					if ( (extOpt.nLgExact & 0x02) == 0 ){
						msecOut = cnv.getMsecAdjustFrmFromMsec(dtlogo2.result_rise, -1);	// �؂�ڂȂ��ɕ␳
					}
					if ( dtlogo1.unit_fall == false && dtlogo2.unit_rise == false ){
						flag_end = false;	// ���ꃍ�S�̂��ߎ�������
						nrfNext = nrfRise;
					}
				}
			}
		}
	}
	return r;
}
// �o�͗p�̎��̃��S�ʒu�擾
Nrf JlsDataset::getNrfOutDirLogo(Nrf nrf, SearchDirType dr, LogoEdgeType edge, LogoSelectType type, bool final){
	int size_logo = sizeDataLogo();
	int r = -1;
	bool flag_end = false;
	bool check_rise = isLogoEdgeRise(edge);
	bool check_fall = isLogoEdgeFall(edge);
	int i = nrf;
	int step = (dr == SEARCH_DIR_NEXT)? +1 : -1;
	while(flag_end == false){
		i += step;
		if (i >= 0 && i < size_logo*2){
			int nlg_i  = nlgFromNrf(i);
			int edge_i = edgeFromNrf(i);
			DataLogoRecord dtlogo;
			getRecordLogo(dtlogo, nlg_i);
			if (edge_i == LOGO_EDGE_RISE && check_rise){
				if ( (final == true && dtlogo.outtype_rise == LOGO_RESULT_DECIDE) ||
					 (final == false && (isValidLogoRise(nlg_i) || type == LOGO_SELECT_ALL)) ){
					r = i;
					flag_end = true;
				}
			}
			else if (edge_i == LOGO_EDGE_FALL && check_fall){
				if ( (final == true && dtlogo.outtype_fall == LOGO_RESULT_DECIDE) ||
					 (final == false && (isValidLogoFall(nlg_i) || type == LOGO_SELECT_ALL)) ){
					r = i;
					flag_end = true;
				}
			}
		}
		else{
			flag_end = true;
		}
	};
	return r;
}



//=====================================================================
// �O��f�[�^�擾�����i�����V�[���`�F���W�j
//=====================================================================

//---------------------------------------------------------------------
// ���̍\���ʒu���擾
// ���́F
//   nsc  : �V�[���`�F���W�ԍ�
//   dr   : ���������i�O�� / �㑤�j
//   chap_th : ��؂�Ƃ�����臒l
// �o�́F
//   �Ԃ�l�F ��O�\����؂�̈ʒu�ԍ��i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirScpChap(Nsc nsc, SearchDirType dr, ScpChapType chap_th){
	if (dr == SEARCH_DIR_PREV){
		return getNscPrevScpChap(nsc, chap_th);
	}
	return getNscNextScpChap(nsc, chap_th);
}

// �P�O�̈ʒu�擾
Nsc JlsDataset::getNscPrevScpChap(Nsc nsc, ScpChapType chap_th){
	ScpEndType edgetype = SCP_END_NOEDGE;
	return getNscPrevScpChapEdge(nsc, chap_th, edgetype);
}
Nsc JlsDataset::getNscPrevScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int nEdge = ( noedge == SCP_END_NOEDGE )? 1 : 0;
	int r = -1;
	if (nsc-1 < num_scpos){
		for(int i=nsc-1; i>=0+nEdge; i--){
			ScpChapType chap_i = m_scp[i].chap;
			if (chap_i >= chap_th){
				r = i;
				break;
			}
		}
	}
	return r;
}
// �P��̈ʒu�擾
Nsc JlsDataset::getNscNextScpChap(Nsc nsc, ScpChapType chap_th){
	ScpEndType edgetype = SCP_END_NOEDGE;
	return getNscNextScpChapEdge(nsc, chap_th, edgetype);
}
// �P��̈ʒu�擾
Nsc JlsDataset::getNscNextScpChapEdge(Nsc nsc, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int nEdge = ( noedge == SCP_END_NOEDGE )? 1 : 0;
	int r = -1;
	if (nsc+1 >= 0){
		for(int i=nsc+1; i<num_scpos-nEdge; i++){
			ScpChapType chap_i = m_scp[i].chap;
			if (chap_i >= chap_th || i == 0 || i == num_scpos-1){
				r = i;
				break;
			}
		}
	}
	return r;
}

//---------------------------------------------------------------------
// ���̍\���ʒu���擾�i2�\���ȏ��CM�͌����j
// ���́F
//   nsc  : �V�[���`�F���W�ԍ�
//   noedge : 0�t���[���ƍŏI�t���[���������ݒ�
// �o�́F
//   �Ԃ�l�F ���̍\����؂�ʒu�ԍ��i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscNextScpCheckCmUnit(Nsc nsc, ScpEndType noedge){
	int nsc_next = nsc;
	int nsc_cur;
	ScpArType arstat = SCP_AR_UNKNOWN;
	//--- 2�\���ȏ��CM�͍Ō�̈ʒu�܂ňړ� ---
	do{
		nsc_cur  = nsc_next;
		nsc_next = getNscNextScpChapEdge(nsc_cur, SCP_CHAP_DECIDE, noedge);
		arstat = getScpArstat(nsc_next);
	} while(nsc_next > 0 &&
			(arstat == SCP_AR_N_AUNIT ||
			(arstat == SCP_AR_N_BUNIT && nsc_cur == nsc)));
	//--- ���̈ʒu�ɐݒ� ---
	if (nsc_cur == nsc){
		nsc_cur = nsc_next;
	}
	return nsc_cur;
}
Nsc JlsDataset::getNscPrevScpCheckCmUnit(Nsc nsc, ScpEndType noedge){
	int nsc_prev = nsc;
	int nsc_cur;
	bool overEdge;
	ScpArType arstat = SCP_AR_UNKNOWN;
	//--- 2�\���ȏ��CM�͍ŏ��̈ʒu�܂ňړ� ---
	do{
		nsc_cur  = nsc_prev;
		overEdge = ( nsc_cur == sizeDataScp() && noedge == SCP_END_NOEDGE );
		ScpEndType adaptEdge = ( overEdge )? SCP_END_EDGEIN : noedge;
		nsc_prev = getNscPrevScpChapEdge(nsc_cur, SCP_CHAP_DECIDE, adaptEdge);
		arstat = getScpArstat(nsc_cur);
	} while( nsc_prev > 0 && (arstat == SCP_AR_N_AUNIT || overEdge) );

	// 2�\���ȏ��CM�͍ŏ���SCP_AR_N_BUNIT�A�ȍ~�̘A����SCP_AR_N_AUNIT

	//--- ���̈ʒu�ɐݒ� ---
	nsc_cur = nsc_prev;
	return nsc_cur;
}

//---------------------------------------------------------------------
// ���̍\���ʒu���擾
// ���́F
//   nsc  : �V�[���`�F���W�ԍ�
//   dr   : ���������i�O�� / �㑤�j
//   noedge : 0�t���[���ƍŏI�t���[���������ݒ�
// �o�́F
//   �Ԃ�l�F ���̍\����؂�ʒu�ԍ��i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscDirScpDecide(Nsc nsc, SearchDirType dr, ScpEndType noedge){
	if (dr == SEARCH_DIR_PREV){
		return getNscPrevScpDecide(nsc, noedge);
	}
	return getNscNextScpDecide(nsc, noedge);
}

// �P�O�̈ʒu�擾
int JlsDataset::getNscPrevScpDecide(int nsc, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	int nstart = nsc-1;
	if (nstart == num_scpos-1 && noedge == SCP_END_NOEDGE) nstart = num_scpos-2;
	for(int i=nstart; i>=noedge; i--){
		if (getPriorScp(i) >= SCP_PRIOR_DECIDE || i == 0 || i == num_scpos-1){
			r = i;
			break;
		}
	}
	return r;
}

// �P��̈ʒu�擾
Nsc JlsDataset::getNscNextScpDecide(Nsc nsc, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	int nstart = nsc+1;
	if (nstart == 0 && noedge == SCP_END_NOEDGE) nstart = 1;
	for(int i=nstart; i<num_scpos-noedge; i++){
		if (getPriorScp(i) >= SCP_PRIOR_DECIDE || i == 0 || i == num_scpos-1){
			r = i;
			break;
		}
	}
	return r;
}

// �\���p�̍\��
Nsc JlsDataset::getNscNextScpDisp(Nsc nsc, ScpEndType noedge){
	Msec msecFrom = -1;
	if ( isScpChapTypeDecideFromNsc(nsc) ){		// ���ʒu����؂�Ȃ�Q�ƌ��Ƃ���
		 msecFrom = getMsecScp(nsc);
	}
	int r = nsc;
	bool cont = false;		// �ʏ��1��ŏI��
	do{
		if ( isAutoModeUse() ){
			r = getNscNextScpCheckCmUnit(r, noedge);
		}
		else{
			r = getNscNextScpDecide(r, noedge);
		}
		cont = false;
		if ( r>=0 && r<sizeDataScp() && msecFrom>=0 ){
			if ( msecFrom >= getMsecScp(r) ){	// ���݈ʒu�ȉ��͔�΂�
				cont = true;
			}
		}
	} while( r>=0 && cont );
	return r;
}
Nsc JlsDataset::getNscPrevScpDisp(Nsc nsc, ScpEndType noedge){
	Msec msecFrom = -1;
	if ( isScpChapTypeDecideFromNsc(nsc) ){		// ���ʒu����؂�Ȃ�Q�ƌ��Ƃ���
		msecFrom = getMsecScp(nsc);
	}
	int rdet = -1;
	int r = nsc;
	bool cont = false;
	do{
		if ( isAutoModeUse() ){
			r = getNscPrevScpCheckCmUnit(r, noedge);
		}
		else{
			r = getNscPrevScpDecide(r, noedge);
		}
		cont = false;
		if ( r >= 0 ){
			if ( msecFrom < 0 ){	// �Q�ƌ��Ȃ�
				rdet = r;		// ���ʈʒu�ɂ���
				msecFrom = getMsecScp(r);
				cont = true;
			}else{
				if ( msecFrom <= getMsecScp(r) ){	// ���݈ʒu�ȏ�͔�΂�
					rdet = r;
					cont = true;
				}else if ( rdet < 0 ){
					rdet = r;
				}
			}
		}
	} while( r>=0 && cont );
	return rdet;
}

// �~���b�����؂肪�w�萔�O�̈ʒu�擾
Nsc JlsDataset::getNscPrevScpDispFromMsecCount(Msec msec, int nCount, bool clip){
	Msec msecSrc = msec - msecValExact;
	WideMsec wmsecTmp;
	wmsecTmp.just  = msecSrc;
	wmsecTmp.early = 0;
	wmsecTmp.late  = msecSrc;
	Nsc nscT = pdata->getNscFromWideMsecFull(wmsecTmp, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
	nscT += 1;
	for(int i=1; i<=nCount; i++){
		nscT = pdata->getNscPrevScpDisp(nscT, SCP_END_EDGEIN);
		if ( nscT < 0 ) break;
	}
	if ( clip && nscT < 0 ) nscT = 0;
	return nscT;
}
// �~���b�����؂肪�w�萔��̈ʒu�擾
Nsc JlsDataset::getNscNextScpDispFromMsecCount(Msec msec, int nCount, bool clip){
	Msec msecMax = getMsecTotalMax();
	Msec msecSrc = msec + msecValExact;
	WideMsec wmsecTmp;
	wmsecTmp.just  = msecSrc;
	wmsecTmp.early = msecSrc;
	wmsecTmp.late  = msecMax;
	Nsc nscT = pdata->getNscFromWideMsecFull(wmsecTmp, SCP_CHAP_DECIDE, SCP_END_EDGEIN);
	nscT -= 1;
	for(int i=1; i<=nCount; i++){
		nscT = pdata->getNscNextScpDisp(nscT, SCP_END_EDGEIN);
		if ( nscT < 0 ) break;
	}
	if ( clip && nscT < 0 ) nscT = sizeDataScp() - 1;
	return nscT;
}

//=====================================================================
// �ʒu�ɑΉ�����f�[�^�擾����
//=====================================================================

//---------------------------------------------------------------------
// �Ώۈʒu���܂܂�郍�S�ԍ����擾
// ���́F
//   msec_target : �Ώۈʒu
//   edge        : 0=�����オ��G�b�W  1=����������G�b�W  2=���G�b�W
// �o�́F
//   �Ԃ�l�F ���S�ԍ�*2 + fall����1�i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nrf JlsDataset::getNrfLogoFromMsec(Msec msec_target, LogoEdgeType edge){
	int size_logo = sizeDataLogo();
	int r = -1;
	for(int i=0; i<size_logo; i++){
		if ( isLogoEdgeRise(edge) ){
			int msec_s = m_logo[i].rise_l;
			int msec_e = m_logo[i].rise_r;
			if (msec_s <= msec_target && msec_target <= msec_e){
				r = i*2;
				break;
			}
		}
		if ( isLogoEdgeFall(edge) ){
			int msec_s = m_logo[i].fall_l;
			int msec_e = m_logo[i].fall_r;
			if (msec_s <= msec_target && msec_target <= msec_e){
				r = i*2+1;
				break;
			}
		}
	}
	return r;
}
Nrf JlsDataset::getNrfLogoFromMsecResult(Msec msec_target, LogoEdgeType edge, bool result){
	int size_logo = sizeDataLogo();
	int r = -1;
	for(int i=0; i<size_logo; i++){
		if ( isLogoEdgeRise(edge) ){
			if ( result ){
				if ( m_logo[i].outtype_rise == LOGO_RESULT_DECIDE ){
					int msec_s = m_logo[i].result_rise;
					if ( abs(msec_target - msec_s) <= msecValExact ){
						r = i*2;
						break;
					}
				}
			}else{
				int msec_s = m_logo[i].rise_l;
				int msec_e = m_logo[i].rise_r;
				if (msec_s <= msec_target && msec_target <= msec_e){
					r = i*2;
					break;
				}
			}
		}
		if ( isLogoEdgeFall(edge) ){
			if ( result ){
				if ( m_logo[i].outtype_fall == LOGO_RESULT_DECIDE ){
					int msec_s = m_logo[i].result_fall;
					if ( abs(msec_target - msec_s) <= msecValExact ){
						r = i*2+1;
						break;
					}
				}
			}else{
				int msec_s = m_logo[i].fall_l;
				int msec_e = m_logo[i].fall_r;
				if (msec_s <= msec_target && msec_target <= msec_e){
					r = i*2+1;
					break;
				}
			}
		}
	}
	return r;
}

//---------------------------------------------------------------------
// ���S�؂�ւ��ʒu�̖����V�[���`�F���W�擾
// ���́F
//   nrf_target   : �Ώۃ��S�ԍ�
//   msec_th      : �����͈́i-1�̎��͐����Ȃ��j
//   chap_th      : �ΏۂƂ���\����؂���臒l
// �o�́F
//   �Ԃ�l�F �Ώۈʒu�Ɉ�ԋ߂��\����؂�̈ʒu�ԍ��i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscFromNrf(Nrf nrf_target, Msec msec_th, ScpChapType chap_th, bool flat){
	if (nrf_target < 0 || nrf_target >= sizeDataLogo()*2){
		return -1;
	}
	//--- ���S�\���͈͎擾 ---
	int msec_lg_c = -1;
	int msec_lg_l = -1;
	int msec_lg_r = -1;;
	getMsecLogoNrfWide(msec_lg_c, msec_lg_l, msec_lg_r, nrf_target);
	//--- ��ԋ߂��ʒu�̎擾 ---
	int nsc_det  = -1;
	int msec_dif_det = -1;
	ScpChapType chap_det;
	int i = 0;
	bool over = false;
	do{
		i = getNscNextScpChap(i, chap_th);
		if (i > 0){
			int msec_i = getMsecScp(i);
			ScpChapType chap_i = getScpChap(i);
			if (flat){
				chap_i = SCP_CHAP_NONE;
			}
			else if (chap_i >= SCP_CHAP_DECIDE){		// �m��Ȃ瓯�����x���ɐݒ�
				chap_i = SCP_CHAP_DECIDE;
			}
			if (msec_i > msec_lg_r + msecValNear3){
				over = true;
			}
			else if (msec_i >= msec_lg_l - msecValNear3){
				int msec_dif_i = abs(msec_i - msec_lg_c);
				if (msec_dif_i <= msec_th || msec_th < 0){
					if (nsc_det < 0 ||
						chap_i > chap_det ||
						(chap_i == chap_det && msec_dif_i < msec_dif_det)){
						nsc_det = i;
						msec_dif_det = msec_dif_i;
						chap_det = chap_i;
					}
				}
			}
		}
	}while(i > 0 && over == false);
	return nsc_det;
}

//---------------------------------------------------------------------
// �Ώۈʒu�Ɉ�ԋ߂��ʒu������
// ���́F
//   msec_target  : �Ώۈʒu
//   msec_th      : �����͈́i-1�̎��͐����Ȃ��j
//   chap_th      : �ΏۂƂ���\����؂���臒l
//   noedge       : 0�t���[���ƍŏI�t���[���������ݒ�
// �o�́F
//   �Ԃ�l�F �Ώۈʒu�Ɉ�ԋ߂��\����؂�̈ʒu�ԍ��i-1�̎��͊Y���Ȃ��j
//---------------------------------------------------------------------
Nsc JlsDataset::getNscFromMsecFull(Msec msec_target, Msec msec_th, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	Msec msec_min = -1;
	Msec msec_r = -1;
	bool decide_min = false;
	for(int i=noedge; i<num_scpos-noedge; i++){
		int msec_i = m_scp[i].msec;
		ScpChapType chap_i = m_scp[i].chap;
		bool edge = ( i == 0 || i == num_scpos-1);
		if (chap_i >= chap_th || edge){
			//--- �Ώۈʒu����̍����ŏ��l�ӏ����擾 ---
			int msec_dif = abs(msec_target - msec_i);
			if (msec_dif <= msec_th || msec_th < 0){
				bool decide_flag = ( jlsd::isScpChapTypeDecide(chap_i) || edge );
				bool sameloc = ( abs(msec_i - msec_r) <= msecValExact );
				bool prefer = ( sameloc && !decide_min && decide_flag);
				if ( msec_dif < msec_min || msec_min < 0 || prefer ){
					r = i;
					msec_min = msec_dif;
					msec_r   = msec_i;
					decide_min = decide_flag;
				}
			}
//			//--- �Ώۈʒu���߂�����I�� ---
//			if ( msec_i > msec_target + msecValExact ){
//				break;
//			}
		}
	}
	return r;
}

// �\����؂���擾
Nsc JlsDataset::getNscFromMsecChap(Msec msec_target, Msec msec_th, ScpChapType chap_th){
	return getNscFromMsecFull(msec_target, msec_th, chap_th, SCP_END_NOEDGE);
}

// ����SC�ʒu���擾
Nsc JlsDataset::getNscFromMsecMgn(Msec msec_target, Msec msec_th, ScpEndType noedge){
	return getNscFromMsecFull(msec_target, msec_th, SCP_CHAP_DUPE, noedge);
}
// ����SC�ʒu���擾
Nsc JlsDataset::getNscFromMsecAll(Msec msec_target){
	return getNscFromMsecFull(msec_target, msecValNear2, SCP_CHAP_DUPE, SCP_END_NOEDGE);
}

// ����SC�ʒu���擾
Nsc JlsDataset::getNscFromMsecAllEdgein(Msec msec_target){
	return getNscFromMsecFull(msec_target, msecValNear2, SCP_CHAP_DUPE, SCP_END_EDGEIN);
}

// �͈͓��̖���SC�ʒu���擾
Nsc JlsDataset::getNscFromWideMsecFull(WideMsec wmsec_target, ScpChapType chap_th, ScpEndType noedge){
	int num_scpos = sizeDataScp();
	int r = -1;
	Msec msec_min = -1;
	for(int i=noedge; i<num_scpos-noedge; i++){
		Msec msec_i = m_scp[i].msec;
		ScpChapType chap_i = m_scp[i].chap;
		if (chap_i >= chap_th || i == 0 || i == num_scpos-1){
			//--- �Ώۈʒu����̍����ŏ��l�ӏ����擾 ---
			int msec_dif = abs(wmsec_target.just - msec_i);
			if ((wmsec_target.early <= msec_i || wmsec_target.early < 0) &&
				(msec_i <= wmsec_target.late  || wmsec_target.late  < 0)){
				if (msec_dif < msec_min || msec_min < 0){
					r = i;
					msec_min = msec_dif;
				}
			}
			//--- �Ώۈʒu���߂�����I�� ---
			if (msec_i >= wmsec_target.late){
				break;
			}
		}
	}
	return r;
}
// �͈͓��̖���SC�ʒu���擾
Nsc JlsDataset::getNscFromWideMsecByChap(WideMsec wmsec_target, ScpChapType chap_th){
	return getNscFromWideMsecFull(wmsec_target, chap_th, SCP_END_NOEDGE);
}

// ��ԋ߂�����SC�ʒu���擾�i�\���p�̍\����؂�̂݁j
Nsc JlsDataset::getNscFromMsecDisp(Msec msec_target, Msec msec_th, ScpEndType noedge){
	WideMsec wmsec;
	wmsec.just  = msec_target;
	wmsec.early = msec_target-msec_th;
	wmsec.late  = msec_target+msec_th;
	return getNscFromWideMsecDisp(wmsec, noedge);
}
// �͈͓��̖���SC�ʒu���擾�i�\���p�̍\����؂�̂݁j
Nsc JlsDataset::getNscFromWideMsecDisp(WideMsec wmsec_target, ScpEndType noedge){
	Term term = {};
	pdata->setTermEndtype(term, noedge);	// �[���܂߂Ď��{���邩
	pdata->setTermForDisp(term, true);		// �\���p�̍\��
	Nsc r = -1;
	Msec msecMin = -1;
	bool first = true;
	bool cont = getTermNext(term);
	while( cont ){
		Nsc nscNow = ( first )? term.nsc.st : term.nsc.ed;
		Msec msecNow = ( first )? term.msec.st : term.msec.ed;
		Msec msecDif = abs(msecNow - wmsec_target.just);
		if ((wmsec_target.early <= msecNow || wmsec_target.early < 0) &&
		    (msecNow <= wmsec_target.late  || wmsec_target.late  < 0)){
			if (msecDif < msecMin || msecMin < 0){
				r = nscNow;
				msecMin = msecDif;
			}
			//--- �Ώۈʒu���߂�����I�� ---
			if (msecNow >= wmsec_target.late){
				break;
			}
		}
		//--- ���̈ʒu�ݒ� ---
		if ( first ){
			first = false;
		}else{
			cont = getTermNext(term);
		}
	}
	return r;
}

//---------------------------------------------------------------------
// �J�n�I���ʒu�ɑΉ����閳���V�[���`�F���W�ԍ����擾
//---------------------------------------------------------------------
bool JlsDataset::getRangeNscFromRangeMsec(RangeNsc &rnsc, RangeMsec rmsec){
	//--- �ʒu���琳�m�ȃ��S�ԍ����擾���� ---
	bool det = true;
	rnsc.st = -1;
	rnsc.ed = -1;

	if (rmsec.st >= 0){
		rnsc.st = getNscFromMsecChap(rmsec.st, pdata->msecValLap2, SCP_CHAP_DECIDE);
		if (rnsc.st < 0){
			det = false;
		}
	}
	if (rmsec.ed >= 0){
		rnsc.ed = getNscFromMsecChap(rmsec.ed, pdata->msecValLap2, SCP_CHAP_DECIDE);
		if (rnsc.ed < 0){
			det = false;
		}
	}
	return det;
}





//=====================================================================
// ��Ԑݒ�
//=====================================================================

//---------------------------------------------------------------------
// ���S���x���ݒ�E�ǂݏo��
//---------------------------------------------------------------------
void JlsDataset::setLevelUseLogo(int level){
	m_levelUseLogo = level;
}
int JlsDataset::getLevelUseLogo(){
	return m_levelUseLogo;
}

//---------------------------------------------------------------------
// �]������K�v�ȏ�������
//---------------------------------------------------------------------
void JlsDataset::setFlagSetupAdj(bool flag){
	m_flagSetupAdj = flag;
}

//---------------------------------------------------------------------
// �����\�������̃��[�h�ݒ�
//---------------------------------------------------------------------
void JlsDataset::setFlagAutoMode(bool flag){
	if (flag){
		m_flagSetupAuto = 2;
	}
	else{
		m_flagSetupAuto = 1;
	}
}



//=====================================================================
// ��Ԕ���
//=====================================================================

//---------------------------------------------------------------------
// �ǂݍ��񂾃��S�����݂��邩���f
//---------------------------------------------------------------------
bool JlsDataset::isExistLogo(){
	return (pdata->extOpt.flagNoLogo > 0)? false : true;
}

//---------------------------------------------------------------------
// ���S�������݂��邩���f
//---------------------------------------------------------------------
bool JlsDataset::isUnuseLogo(){
	return (m_levelUseLogo == CONFIG_LOGO_LEVEL_UNUSE_ALL)? true : false;
}
bool JlsDataset::isUnuseLevelLogo(){
	return (m_levelUseLogo < CONFIG_LOGO_LEVEL_USE_LOW)? true : false;
}

//---------------------------------------------------------------------
// Auto�\����������Ԃ��m�F
//---------------------------------------------------------------------
bool JlsDataset::isSetupAdjInitial(){
	return (m_flagSetupAdj == 0)? true : false;
}

//---------------------------------------------------------------------
// Auto�\����������Ԃ��m�F
//---------------------------------------------------------------------
bool JlsDataset::isAutoModeInitial(){
	return (m_flagSetupAuto == 0)? true : false;
}

//---------------------------------------------------------------------
// Auto�\�����g�p���郂�[�h���m�F
//---------------------------------------------------------------------
bool JlsDataset::isAutoModeUse(){
	return (m_flagSetupAuto == 2)? true : false;
}

//---------------------------------------------------------------------
// �����S���g�킸�\���������S���L���ȃ��[�h���m�F
//---------------------------------------------------------------------
bool JlsDataset::isAutoLogoOnly(){
	return (m_flagSetupAuto > 0 && isUnuseLevelLogo())? true : false;
}

//---------------------------------------------------------------------
// �Ώۈʒu�̃��S���L�������f�i���S�����オ��j
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoRise(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		if (m_logo[nlg].outtype_rise == LOGO_RESULT_NONE || m_logo[nlg].outtype_rise == LOGO_RESULT_DECIDE){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// �Ώۈʒu�̃��S���L�������f�i���S����������j
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoFall(Nlg nlg){
	if (nlg >= 0 && nlg < sizeDataLogo()){
		if (m_logo[nlg].outtype_fall == LOGO_RESULT_NONE || m_logo[nlg].outtype_fall == LOGO_RESULT_DECIDE){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// �Ώۈʒu�̃��S���L�������f�i���S����������j
//---------------------------------------------------------------------
bool JlsDataset::isValidLogoNrf(Nrf nrf){
	Nlg nlg = nlgFromNrf(nrf);
	if (jlsd::isLogoEdgeRiseFromNrf(nrf)){
		return isValidLogoRise(nlg);
	}
	return isValidLogoFall(nlg);
}


//---------------------------------------------------------------------
// �������S�����ӏ������f
//---------------------------------------------------------------------
bool JlsDataset::isElgDivScpForAll(Nsc nsc, bool flag_border, bool flag_out){
	bool ret = false;
	if ( getScpChap(nsc) == SCP_CHAP_DUNIT ){	// �����\���w��
		int  nsc2     = getNscNextScpDecide(nsc, SCP_END_EDGEIN);
		bool inlogo_p = isElgInScpForAll(nsc,  flag_border, flag_out);
		bool inlogo_n = isElgInScpForAll(nsc2, flag_border, flag_out);
		if ( inlogo_p && inlogo_n ){	// �O��Ƃ��Ƀ��S�����̈�
			ret = true;
		}
	}
	return ret;
}

//---------------------------------------------------------------------
// ���O�������\���̃��S�����\�������f�i�����\�z�p�j
//---------------------------------------------------------------------
bool JlsDataset::isElgInScp(Nsc nsc){
	return isElgInScpForAll(nsc, false, false);
}

//---------------------------------------------------------------------
// ���O�������\���̃��S�����\�������f�i�I��t���j
//---------------------------------------------------------------------
bool JlsDataset::isElgInScpForAll(Nsc nsc, bool flag_border, bool flag_out){
	bool ret = false;
	int num_scpos = sizeDataScp();
	if (nsc > 0 && nsc < num_scpos){
		//--- �\����؂�łȂ���Ύ��̈ʒu�擾 ---
		if (isScpChapTypeDecide( getScpChap(nsc) ) == false){
			if (nsc < num_scpos-1){
				nsc = getNscNextScpDecide(nsc, SCP_END_EDGEIN);
			}
		}
		//--- ���S�����\�������f ---
		ret = isLabelLogoFromNsc(nsc, flag_border, flag_out);
	}
	return ret;
}

//---------------------------------------------------------------------
// �w��ʒu��Auto��؂肪�m���Ԃ����f
//---------------------------------------------------------------------
bool JlsDataset::isScpChapTypeDecideFromNsc(Nsc nsc){
	ScpChapType chap_nsc = getScpChap(nsc);
	return jlsd::isScpChapTypeDecide(chap_nsc);
}

//---------------------------------------------------------------------
// �w��ʒu�̖����\���������Ȃ������f
//---------------------------------------------------------------------
bool JlsDataset::isStillFromMsec(Msec msec_target){
	int num_scpos = sizeDataScp();
	bool det = true;
	bool over = false;
	int i = 1;
	while(i < num_scpos-1 && det == false && over == false){
		Msec msec_s = m_scp[i].msmute_s;
		Msec msec_e = m_scp[i].msmute_e;
		if (msec_s <= msec_target && msec_target <= msec_e){
			if (m_scp[i].still == 0){
				det = false;
			}
		}
		if (msec_s > msec_target){
			over = true;
		}
		i ++;
	}
	return det;
}

//---------------------------------------------------------------------
// �w��ʒu�������\�������f
//---------------------------------------------------------------------
bool JlsDataset::isSmuteFromMsec(Msec msec_target){
	int num_scpos = sizeDataScp();
	bool det = false;
	bool over = false;
	int i = 1;
	while(i < num_scpos-1 && det == false && over == false){
		int msec_s = m_scp[i].msmute_s;
		int msec_e = m_scp[i].msmute_e;
		if (msec_s <= msec_target && msec_target <= msec_e){
			det = true;
		}
		if (msec_s > msec_target){
			over = true;
		}
		i ++;
	}
	return det;
}

//---------------------------------------------------------------------
// �Q�n�_������������Ԃ̃f�[�^�����f
//---------------------------------------------------------------------
bool JlsDataset::isSmuteSameArea(Nsc nsc1, Nsc nsc2){
	int num_scpos = sizeDataScp();
	if (nsc1 > 0 && nsc1 < num_scpos-1 && nsc2 > 0 && nsc2 < num_scpos-1){
		if (m_scp[nsc1].msmute_s == m_scp[nsc2].msmute_s &&
			m_scp[nsc1].msmute_e == m_scp[nsc2].msmute_e){
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
// �͈͂�����
// ���́F
//    rmsec  : ���肷��͈�
// ���o�́F
//    wmsec  : �Ώۃf�[�^�i�͈͂Ȃ��̎��͏��������Ȃ��j
// �o��:
//   �Ԃ�l�F false=�͈͂Ȃ�  true=�͈͐ݒ�
//---------------------------------------------------------------------
bool JlsDataset::limitWideMsecFromRange(WideMsec& wmsec, RangeMsec rmsec){
	if (wmsec.late  < rmsec.st && wmsec.late  >= 0 && rmsec.st >= 0) return false;
	if (wmsec.early > rmsec.ed && wmsec.early >= 0 && rmsec.ed >= 0) return false;
	if (rmsec.st >= 0){
		if (wmsec.early < rmsec.st) wmsec.early = rmsec.st;
		if (wmsec.just  < rmsec.st) wmsec.just  = rmsec.st;
	}
	if (rmsec.ed >= 0){
		if (wmsec.late > rmsec.ed || wmsec.late < 0) wmsec.late  = rmsec.ed;
		if (wmsec.just > rmsec.ed || wmsec.just < 0) wmsec.just  = rmsec.ed;
	}
	return true;
}



//=====================================================================
// Term�\������
//=====================================================================

//---------------------------------------------------------------------
// �������ɒ[���܂߂邩�I��
//---------------------------------------------------------------------
void JlsDataset::setTermEndtype(Term &term, ScpEndType endtype){
	term.endfix = true;
	term.endtype = endtype;
}
//---------------------------------------------------------------------
// �����p�ł͂Ȃ��\���p�̍\���ɂ��邩�I��
//---------------------------------------------------------------------
void JlsDataset::setTermForDisp(Term &term, bool flag){
	term.disp = flag;
}
//---------------------------------------------------------------------
// ���̍\�����擾
//---------------------------------------------------------------------
bool JlsDataset::getTermNext(Term &term){
	bool ret = false;
	if (!term.valid){				// ������s
		term.valid = true;
		if (!term.endfix){
			term.endfix = true;
			term.endtype = SCP_END_NOEDGE;
		}
		if (term.ini > 0){		// �J�n�ʒu�ݒ莞
			term.nsc.ed = term.ini;
		}
		else{
			term.nsc.ed = -1;
			if ( term.disp ){
				term.nsc.ed = pdata->getNscNextScpDisp(term.nsc.ed, term.endtype);
			}else{
				term.nsc.ed = pdata->getNscNextScpDecide(term.nsc.ed, term.endtype);
			}
		}
		term.msec.ed = pdata->getMsecScp(term.nsc.ed);;
	}
	term.nsc.st = term.nsc.ed;
	term.msec.st = term.msec.ed;
	if ( term.disp ){
		term.nsc.ed = pdata->getNscNextScpDisp(term.nsc.ed, term.endtype);
	}else{
		term.nsc.ed = pdata->getNscNextScpDecide(term.nsc.ed, term.endtype);
	}
	term.msec.ed = pdata->getMsecScp(term.nsc.ed);
	if (term.nsc.ed >= 0){
		ret = true;
	}
	return ret;
}

//---------------------------------------------------------------------
// �O�̍\�����擾
//---------------------------------------------------------------------
bool JlsDataset::getTermPrev(Term &term){
	bool ret = false;
	if (!term.valid){				// ������s
		term.valid = true;
		if (!term.endfix){
			term.endfix = true;
			term.endtype = SCP_END_NOEDGE;
		}
		if (term.ini > 0){		// �J�n�ʒu�ݒ莞
			term.nsc.st = term.ini;
		}
		else{
			term.nsc.st = pdata->sizeDataLogo();
			if ( term.disp ){
				term.nsc.st = pdata->getNscPrevScpDisp(term.nsc.st, term.endtype);
			}else{
				term.nsc.st = pdata->getNscPrevScpDecide(term.nsc.st, term.endtype);
			}
		}
		term.msec.st = pdata->getMsecScp(term.nsc.st);
	}
	term.nsc.ed = term.nsc.st;
	term.msec.ed = term.msec.st;
	if ( term.disp ){
		term.nsc.st = pdata->getNscPrevScpDisp(term.nsc.st, term.endtype);
	}else{
		term.nsc.st = pdata->getNscPrevScpDecide(term.nsc.st, term.endtype);
	}
	term.msec.st = pdata->getMsecScp(term.nsc.st);
	if (term.nsc.st >= 0){
		ret = true;
	}
	return ret;
}

//---------------------------------------------------------------------
// �e�v�f�̐ݒ�E�擾
//---------------------------------------------------------------------
ScpArType JlsDataset::getScpArstat(Term term){
	return pdata->getScpArstat(term.nsc.ed);
}
ScpArExtType JlsDataset::getScpArext(Term term){
	return pdata->getScpArext(term.nsc.ed);
}
void JlsDataset::setScpArstat(Term term, ScpArType arstat){
	pdata->setScpArstat(term.nsc.ed, arstat);
}
void JlsDataset::setScpArext(Term term, ScpArExtType arext){
	pdata->setScpArext(term.nsc.ed, arext);
}

//---------------------------------------------------------------------
// CM�\��������
//---------------------------------------------------------------------
bool JlsDataset::isScpArstatCmUnit(Term term){
	ScpArType arstat_term = getScpArstat(term);
	if ((arstat_term == SCP_AR_N_UNIT)  ||
		(arstat_term == SCP_AR_N_AUNIT) ||
		(arstat_term == SCP_AR_N_BUNIT)){
		return true;
	}
	return false;
}

//---------------------------------------------------------------------
// �����͈͓��ɂ��邩����
//---------------------------------------------------------------------
bool JlsDataset::checkScopeTerm(Term term, RangeMsec scope){
	int msec_spc  = pdata->msecValSpc;
	return (term.msec.st >= scope.st - msec_spc && term.msec.ed <= scope.ed + msec_spc)? true : false;
}

bool JlsDataset::checkScopeRange(RangeMsec bounds, RangeMsec scope){
	int msec_spc  = pdata->msecValSpc;
	return (bounds.st >= scope.st - msec_spc && bounds.ed <= scope.ed + msec_spc)? true : false;
}


//=====================================================================
// �J�X�^�����S�쐬
//=====================================================================

//---------------------------------------------------------------------
// �w��ݒ�Ń��S�ʒu���X�g���쐬���ĕԂ��i�����f�[�^�͕ύX���Ȃ��j
//---------------------------------------------------------------------
void JlsDataset::trialClogo(vector<WideMsec>& listWmsec, LogoCustomType custom){
	makeClogoMain(listWmsec, custom);
}
//---------------------------------------------------------------------
// �w��ݒ�Ń��S�ʒu���X�g���쐬�E�i�[
//---------------------------------------------------------------------
void JlsDataset::makeClogo(LogoCustomType custom){
	makeClogoMain(m_listClogo, custom);
	m_customLogo = custom;		// �ݒ�l���ۊ�
}
// �w��ݒ�Ń��S�ʒu���X�g���쐬���鋤�ʏ���
void JlsDataset::makeClogoMain(vector<WideMsec>& listWmsec, LogoCustomType& custom){
	if ( custom.extLogo && (isAutoModeUse() == false) ){
		custom.extLogo = false;		// AutoMode�łȂ���ΐ������S�͎g��Ȃ�
	}
	if ( custom.extLogo ){
		makeClogoFromVirtual(listWmsec, custom);
	}else{
		makeClogoFromReal(listWmsec, custom);
	}
}
// �w��ݒ�i���ۂ̃��S�j�Ń��S�ʒu���X�g���쐬
void JlsDataset::makeClogoFromReal(vector<WideMsec>& listWmsec, LogoCustomType custom){
	listWmsec.clear();		// ���X�g������

	LogoSelectType lgsel = LogoSelectType::LOGO_SELECT_VALID;
	if ( custom.selectAll ){		// �ݒ�l
		lgsel = LogoSelectType::LOGO_SELECT_ALL;
	}
	bool final = custom.final;		// �ݒ�l
	//--- ���ۂ̃��S�ʒu�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu ---
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = getNrfptOutNext(logopt, lgsel, final);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			if ( final ){
				wmsecRise = { logopt.msecRise, logopt.msecRise, logopt.msecRise};
				wmsecFall = { logopt.msecFall, logopt.msecFall, logopt.msecFall};
			}else{
				pdata->getWideMsecLogoNrf(wmsecRise, logopt.nrfRise);
				pdata->getWideMsecLogoNrf(wmsecFall, logopt.nrfFall);
			}
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
		}
	}
}
// �w��ݒ�i�������S�j�Ń��S�ʒu���X�g���쐬
void JlsDataset::makeClogoFromVirtual(vector<WideMsec>& listWmsec, LogoCustomType custom){
	listWmsec.clear();		// ���X�g������

	//--- �����\���ω��_�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu ---
	ElgCurrent elgTmp = {};
	elgTmp.border  = custom.border;		// �ݒ�l
	elgTmp.outflag = custom.final;		// �ݒ�l
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getElgptNext(elgTmp);
		if ( flag_cont ){
			WideMsec wmsecRise = getWideMsecScp(elgTmp.nscRise);
			WideMsec wmsecFall = getWideMsecScp(elgTmp.nscFall);
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
		}
	}
}
//---------------------------------------------------------------------
// �J�X�^�����S���X�g���擾
//---------------------------------------------------------------------
//--- �ݒ�擾 ---
LogoCustomType JlsDataset::getClogoCustom(){
	return m_customLogo;
}
//--- ���X�g�擾 ---
vector<WideMsec> JlsDataset::getClogoList(){
	return m_listClogo;
}
//--- �T�C�Y�擾 ---
int JlsDataset::sizeClogoList(){
	return (int)m_listClogo.size();
}
//---------------------------------------------------------------------
// ���݈ʒu���J�X�^�����S�̉��ԖڂɈ�ԋ߂����擾
// �Ԃ�l�F
//   -2    : �f�[�^�Ȃ�
//   -1    : �ŏ��̗����オ�����O
//   0�ȏ� : ���S���X�g�ԍ��i�����F�����A��F������j
//  �ő�ԍ�+1  : �Ō�̗������������
//---------------------------------------------------------------------
int JlsDataset::getClogoNumNear(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
	if ( listWmsec.empty() ){
		return -2;
	}
	int loc = isLogoEdgeRise(edge)? 0 : 1;		// �ŏ��̑Ή��G�b�W���S�ԍ�
	int nmax = (int)listWmsec.size();
	bool flagCont = true;
	while( flagCont && (loc+2 < nmax) ){
		if ( listWmsec[loc+2].just <= msecLogo && listWmsec[loc].just < msecLogo ){
			loc += 2;
		}else{
			flagCont = false;
			bool flagInC = ( listWmsec[loc].late >= msecLogo )? true : false;
			bool flagInN = ( listWmsec[loc+2].early <= msecLogo )? true : false;
			if ( flagInC == true && flagInN == false ){
				// ���ݗ̈�Ŏ��̗̈�ł͂Ȃ�
			}else if ( flagInC == false && flagInN == true ){
				loc += 2;
			}else if ( listWmsec[loc+1].just < msecLogo ){
				loc += 2;
			}else if ( listWmsec[loc+1].just == msecLogo ){
				if ( listWmsec[loc+2].just - msecLogo < msecLogo - listWmsec[loc].just ){
					loc += 2;
				}
			}
		}
	}
	if ( loc == nmax-2 ){		// �Ō��fall���rise����
		if ( listWmsec[loc+1].just <= msecLogo && listWmsec[loc].just < msecLogo ){
			loc += 2;
		}
	}
	if ( loc == 1 ){			// �ŏ���rise�O��fall����
		if ( listWmsec[loc-1].just >= msecLogo && listWmsec[loc].just > msecLogo ){
			loc = -1;
		}
	}
	//--- ���G�b�W���o�Ή� ---
	if ( edge == LogoEdgeType::LOGO_EDGE_BOTH ){
		if ( loc < 0 ){
			if ( loc == -1 ) loc = 0;
		}else if ( loc >= nmax ){
			if ( loc == nmax ) loc = nmax-1;
		}else{
			Msec msecRef = listWmsec[loc].just;
			int step = ( msecLogo < msecRef )? -1 : +1;
			if ( 0 <= loc+step && loc+step < nmax ){
				if ( abs(listWmsec[loc+step].just - msecLogo) < abs(msecRef - msecLogo) ){
					loc = loc+step;
				}
			}
		}
	}
	return loc;
}
//---------------------------------------------------------------------
// ���݈ʒu����P�O�̃J�X�^�����S���擾�i���X�g�ԍ��j
//---------------------------------------------------------------------
int JlsDataset::getClogoNumPrev(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
	if ( listWmsec.empty() ){
		return -2;
	}
	Msec msecLogoRev = msecLogo - getClogoMsecMgn();
	int locRef = getClogoNumNear(msecLogoRev, edge);
	int nmax = (int)listWmsec.size();
	int step = ( edge == LogoEdgeType::LOGO_EDGE_BOTH )? -1 : -2;
	if ( locRef < 0 || locRef >= nmax+1 ){
		return locRef;
	}else if ( locRef == nmax ){
		if ( isLogoEdgeFall(edge) ) return nmax-1;
		return nmax-2;
	}else{
		WideMsec wmsecRef = listWmsec[locRef];
		if ( wmsecRef.late < msecLogoRev ){
			return locRef;
		}
		else if ( locRef+step < 0 ){
			return -1;
		}
	}
	return locRef+step;
}
//---------------------------------------------------------------------
// ���݈ʒu����P��̃J�X�^�����S���擾�i���X�g�ԍ��j
//---------------------------------------------------------------------
int JlsDataset::getClogoNumNext(Msec msecLogo, LogoEdgeType edge){
	vector<WideMsec>& listWmsec = m_listClogo;
	if ( listWmsec.empty() ){
		return -2;
	}
	Msec msecLogoRev = msecLogo + getClogoMsecMgn();
	int locRef = getClogoNumNear(msecLogoRev, edge);
	int nmax = (int)listWmsec.size();
	int step = ( edge == LogoEdgeType::LOGO_EDGE_BOTH )? +1 : +2;
	if ( locRef < -1 || locRef >= nmax ){
		return locRef;
	}else if ( locRef == -1 ){
		if ( jlsd::isLogoEdgeRise(edge) ) return 0;
		return 1;
	}else{
		WideMsec wmsecRef = listWmsec[locRef];
		if ( msecLogoRev < wmsecRef.early ){
			return locRef;
		}
		else if ( locRef+step >= nmax ){
			return nmax;
		}
	}
	return locRef+step;
}
//---------------------------------------------------------------------
// �w��񐔑O��
//---------------------------------------------------------------------
int JlsDataset::getClogoNumPrevCount(Msec msecLogo, int nCount){
	int loc = getClogoNumPrev(msecLogo, LOGO_EDGE_BOTH);
	loc -= nCount - 1;
	if ( loc < -1 ) loc = -1;
	return loc;
}
int JlsDataset::getClogoNumNextCount(Msec msecLogo, int nCount){
	int loc = getClogoNumNext(msecLogo, LOGO_EDGE_BOTH);
	if ( loc >= -1 ){
		loc += nCount - 1;
	}
	int nsize = sizeClogoList();
	if ( loc < -1 || loc > nsize ) loc = nsize;
	return loc;
}
//---------------------------------------------------------------------
// �~���b�P�ʂőΉ�����J�X�^�����S���擾
//---------------------------------------------------------------------
WideMsec JlsDataset::getClogoWmsecFromNum(int num){
	int nmax = sizeClogoList();
	if ( nmax == 0 ) return {-1, -1, -1};
	if ( num == -1 ) return {0, 0, 0};
	if ( num == nmax ){
		Msec ms = getMsecTotalMax();
		return {ms, ms, ms};
	}
	if ( 0 <= num && num < nmax ) return m_listClogo[num];
	return {-1, -1, -1};
}
Msec JlsDataset::getClogoMsecFromNum(int num){
	WideMsec wmsec = getClogoWmsecFromNum(num);
	return wmsec.just;
}
Msec JlsDataset::getClogoMsecNear(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumNear(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
Msec JlsDataset::getClogoMsecPrev(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumPrev(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
Msec JlsDataset::getClogoMsecNext(Msec msecLogo, LogoEdgeType edge){
	int num = getClogoNumNext(msecLogo, edge);
	return getClogoMsecFromNum(num);
}
//---------------------------------------------------------------------
// �J�X�^�����S�̃��S���擾
//---------------------------------------------------------------------
//--- �Ώۈʒu�����S�ؑ֒n�_������ ---
bool JlsDataset::isClogoMsecExist(Msec msecLogo, LogoEdgeType edge){
	for(int i=0; i<sizeClogoList(); i++){
		if ( getClogoMsecFromNum(i) == msecLogo ){
			if ( i%2 == 0 ){
				if ( jlsd::isLogoEdgeRise(edge) ) return true;
			}else{
				if ( jlsd::isLogoEdgeFall(edge) ) return true;
			}
		}
	}
	return false;
}
//--- ���ۂ̃��S�g�p���� ---
bool JlsDataset::isClogoReal(){
	return ( m_customLogo.extLogo == false );
}
//--- �ʎZ���S�ԍ��i���ۂ̃��S�j ---
Nrf JlsDataset::getClogoRealNrf(Msec msecLogo, LogoEdgeType edge){
	if ( m_customLogo.extLogo ) return -1;		// ���ۂ̃��S�ȊO�͖���
	if ( msecLogo < 0 ) return -1;
	// �O�̂��ߍL���ԍ��擾���邽�ߍŏI���͍ŏI�ȊO�Ɨ����m�F
	if ( m_customLogo.final ){
		Nrf nrfTmp = getNrfLogoFromMsecResult(msecLogo, edge, true);
		if ( nrfTmp >= 0 ) return nrfTmp;
	}
	return getNrfLogoFromMsecResult(msecLogo, edge, false);
}
//--- ���S�ɑΉ����閳���V�[���`�F���W�ʒu�ԍ� ---
Nsc JlsDataset::getClogoNsc(Msec msecLogo){
	Nsc nscTmp = getNscFromMsecAllEdgein(msecLogo);
	if ( nscTmp < 0 && isClogoReal() ){		// ���ۃ��S�͍L�߂ɖ���SC����
		nscTmp = getNscFromMsecMgn(msecLogo, msecValSpc, ScpEndType::SCP_END_EDGEIN);
	}
	return nscTmp;
}
Msec JlsDataset::getClogoMsecMgn(){
	if ( isClogoReal() ) return msecValSpc;
	return msecValExact;
}
//=====================================================================
// �f�[�^�}��
//=====================================================================

//---------------------------------------------------------------------
// ���S��؂�}��
//   msec_target : ���S��؂�}���ʒu
//   confirm : �m��������ɍs�����i0=�ݒ�̂�  1=�m�菈�������s�j
//   unit    : �}���ʒu�����œƗ��\���ɂ��邩�i0=�]�����S�ƍ���  1=�}�����S�����œƗ��\���j
//   edge    : �G�b�W�ɂ��ʒu�␳�i0=�����オ��G�b�W  1=����������G�b�W�j
// �o�́F
//   �Ԃ�l�F �}���ʒu�ԍ� �}�����Ȃ������ꍇ��-1
//---------------------------------------------------------------------
Nsc JlsDataset::insertDivLogo(Msec msec_target, bool confirm, bool unit, LogoEdgeType edge){
	bool overlap = true;
	return insertLogoEdge(msec_target, msec_target, overlap, confirm, unit, edge);
}

//---------------------------------------------------------------------
// ���S�f�[�^�}��
// ���́F
//   msec_st : ���S�f�[�^�}���J�n�ʒu
//   msec_ed : ���S�f�[�^�}���I���ʒu
//   overlap : �������S�Əd�����i0=����  1=�d�����j
//   confirm : �m��������ɍs�����i0=�ݒ�̂�  1=�m�菈�������s�j
//   unit    : �}���ʒu�����œƗ��\���ɂ��邩�i0=�]�����S�ƍ���  1=�}�����S�����œƗ��\���j
// �o�́F
//   �Ԃ�l�F �}���ʒu�ԍ� �}�����Ȃ������ꍇ��-1
//---------------------------------------------------------------------
Nsc JlsDataset::insertLogo(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit){
	return insertLogoEdge(msec_st, msec_ed, overlap, confirm, unit, LOGO_EDGE_RISE);
}

//---------------------------------------------------------------------
// ���S�f�[�^�}���i��؂�Ή��j
//   edge    : �G�b�W�ɂ��ʒu�␳�i0=�����オ��G�b�W  1=����������G�b�W�j
//---------------------------------------------------------------------
Nsc JlsDataset::insertLogoEdge(Msec msec_st, Msec msec_ed, bool overlap, bool confirm, bool unit, LogoEdgeType edge){
	int size_logo = sizeDataLogo();
	Msec wid_ovl = pdata->msecValSpc;

	if ( msec_st < 0 || msec_ed < 0 || msec_st > getMsecTotalMax() || msec_ed > getMsecTotalMax() ){
		return -1;		// �G���[�`�F�b�N
	}
	bool separator = false;
	if ( msec_ed <= msec_st ){		// �͈͂ł͂Ȃ���؂�w��
		separator = true;
		msec_ed = msec_st;
		if ( jlsd::isLogoEdgeRise(edge) ){		// start edge
			msec_st = cnv.getMsecAdjustFrmFromMsec(msec_st, -1);
			if ( msec_st < 0 ){
				msec_st = 0;
			}
		}else{
			msec_ed = cnv.getMsecAdjustFrmFromMsec(msec_ed, +1);
			if ( msec_ed > getMsecTotalMax() ){
				msec_ed =getMsecTotalMax();
			}
		}
	}

	// ���S�}���ӏ�������
	int num_ins = -1;
	for(int i=1; i<size_logo; i++){
		if (msec_st >= getMsecLogoFall(i-1) && msec_ed <= getMsecLogoRise(i)){
			num_ins = i;
		}
	}
	if (num_ins < 0){
		if (msec_ed <= getMsecLogoRise(0)){
			num_ins = 0;
		}
		else if (msec_st >= getMsecLogoFall(size_logo-1)){
			num_ins = size_logo;
		}
	}
	if (size_logo >= MAXSIZE_LOGO){
		num_ins = -1;
	}

	int total_ins = 1;
	// �}���ӏ��Ɠ����ʒu�̊m����ۑ��p
	Msec bak_result_rise, bak_result_fall;
	bool flag_bak_rise = false;
	bool flag_bak_fall = false;
	LogoResultType bak_outtype_rise = LOGO_RESULT_NONE;
	LogoResultType bak_outtype_fall = LOGO_RESULT_NONE;
	// overlap�����̒ǉ�����
	if (overlap && num_ins < 0 && size_logo < MAXSIZE_LOGO){
		for(int i=0; i<size_logo; i++){
			DataLogoRecord dt;
			getRecordLogo(dt, i);
			// �}���ӏ��Ɠ����ʒu�̊m����ۑ�
			if (abs(msec_st - dt.rise) <= wid_ovl){
				flag_bak_rise    = true;
				bak_outtype_rise = dt.outtype_rise;
				bak_result_rise  = dt.result_rise;
			}
			if (abs(msec_ed - dt.fall) <= wid_ovl){
				flag_bak_fall    = true;
				bak_outtype_fall = dt.outtype_fall;
				bak_result_fall  = dt.result_fall;
			}
			// �}�����S�ƈ�v����ꍇ
			if (msec_st >= dt.rise - wid_ovl && msec_st <= dt.rise + wid_ovl &&
				msec_ed >= dt.fall - wid_ovl && msec_ed <= dt.fall + wid_ovl){
				dt.rise         = msec_st;
				dt.rise_l       = msec_st;
				dt.rise_r       = msec_st;
				dt.fall         = msec_ed;
				dt.fall_l       = msec_ed;
				dt.fall_r       = msec_ed;
				dt.fade_rise   = 0;
				dt.fade_fall   = 0;
				dt.intl_rise   = 0;
				dt.intl_fall   = 0;
				dt.stat_rise   = LOGO_PRIOR_DECIDE;
				dt.stat_fall   = LOGO_PRIOR_DECIDE;
				dt.outtype_rise = LOGO_RESULT_NONE;
				dt.outtype_fall = LOGO_RESULT_NONE;
				if (confirm){
					dt.outtype_rise = LOGO_RESULT_DECIDE;
					dt.outtype_fall = LOGO_RESULT_DECIDE;
					dt.result_rise  = msec_st;
					dt.result_fall  = msec_ed;
				}
				setRecordLogo(dt, i);
				return i;		// �X�V�̂�
			}
			// �}�����S�̑O���̂݃��S�Əd�Ȃ�ꍇ
			if (msec_st > dt.rise + wid_ovl && msec_st < dt.fall &&
				msec_ed > dt.fall + wid_ovl){
				dt.fall         = msec_st;
				dt.fall_l       = msec_st;
				dt.fall_r       = msec_st;
				dt.fade_fall    = 0;
				dt.intl_fall    = 0;
				dt.stat_fall    = LOGO_PRIOR_DECIDE;
				dt.outtype_fall = LOGO_RESULT_NONE;
				setRecordLogo(dt, i);
				if (confirm){
					setResultLogoAtNrf(dt.fall, LOGO_RESULT_DECIDE, i*2+1);
				}
			}
			// �}�����S�̌㔼�̂݃��S�Əd�Ȃ�ꍇ
			if (msec_st < dt.rise + wid_ovl &&
				msec_ed > dt.rise && msec_ed < dt.fall - wid_ovl){
				dt.rise         = msec_ed;
				dt.rise_l       = msec_ed;
				dt.rise_r       = msec_ed;
				dt.fade_rise    = 0;
				dt.intl_rise    = 0;
				dt.stat_rise    = LOGO_PRIOR_DECIDE;
				dt.outtype_rise = LOGO_RESULT_NONE;
				setRecordLogo(dt, i);
				if (confirm){
					setResultLogoAtNrf(dt.rise, LOGO_RESULT_DECIDE, i*2);
				}
			}
			// �}�����S���Ƀ��S�S�̂�����ꍇ
			if (msec_st - wid_ovl <= dt.rise && msec_ed + wid_ovl >= dt.fall){
					dt.outtype_rise = LOGO_RESULT_ABORT;			// abort
					dt.outtype_fall = LOGO_RESULT_ABORT;			// abort
					dt.result_rise  = -1;
					dt.result_fall  = -1;
					setRecordLogo(dt, i);
					if (num_ins < 0){
						num_ins = i;
						total_ins = 0;		// �}�����S�͏㏑������
					}
			}
			// �}�����S�S�̂����S�͈͓��ɓ���ꍇ
			if (msec_st > dt.rise + wid_ovl && msec_ed < dt.fall - wid_ovl){
				num_ins = i;
				total_ins = 2;				// �}�����S���Q�ɂȂ�
			}
			// �}���ʒu
			if (num_ins < 0 && msec_st <= dt.rise){
				num_ins = i;
			}
		}
		if (num_ins < 0){
			num_ins = size_logo;
		}
	}
	if (size_logo + total_ins > MAXSIZE_LOGO){
		num_ins = -1;
	}

	// ���ʂ��i�[
	int retval = -1;
	if (num_ins >= 0){
		DataLogoRecord dt;
		if (total_ins == 2){		// �������S�r���Ƀ��S�}�����Č����S���Q��������ꍇ
			getRecordLogo(dt, num_ins);
			if ( separator && unit ){		// ��؂�̏ꍇ�͂�����-unit�ݒ�
				dt.unit_rise = LOGO_UNIT_DIVIDE;
			}
			dt.rise        = msec_ed;
			dt.rise_l      = msec_ed;
			dt.rise_r      = msec_ed;
			dt.fade_rise   = 0;
			dt.intl_rise   = 0;
			dt.stat_rise   = LOGO_PRIOR_DECIDE;
			if (confirm){
				dt.outtype_rise = LOGO_RESULT_DECIDE;
				dt.result_rise  = msec_ed;
			}
			else{
				dt.outtype_rise = LOGO_RESULT_NONE;
				dt.result_rise  = 0;
			}
			insertRecordLogo(dt, num_ins+1);

			getRecordLogo(dt, num_ins);
			if ( separator && unit ){		// ��؂�̏ꍇ�͂�����-unit�ݒ�
				dt.unit_fall    = LOGO_UNIT_DIVIDE;
			}
			dt.fall        = msec_st;
			dt.fall_l      = msec_st;
			dt.fall_r      = msec_st;
			dt.fade_fall   = 0;
			dt.intl_fall   = 0;
			dt.stat_fall   = LOGO_PRIOR_DECIDE;
			if (confirm){
				dt.outtype_fall = LOGO_RESULT_DECIDE;
				dt.result_fall  = msec_st;
			}
			else{
				dt.outtype_fall = LOGO_RESULT_NONE;
				dt.result_fall  = 0;
			}
			setRecordLogo(dt, num_ins);
			num_ins = num_ins + 1;
		}
		retval = num_ins;
		getRecordLogo(dt, num_ins);
		dt.rise        = msec_st;
		dt.rise_l      = msec_st;
		dt.rise_r      = msec_st;
		dt.fall        = msec_ed;
		dt.fall_l      = msec_ed;
		dt.fall_r      = msec_ed;
		dt.fade_rise   = 0;
		dt.fade_fall   = 0;
		dt.intl_rise   = 0;
		dt.intl_fall   = 0;
		dt.stat_rise   = LOGO_PRIOR_DECIDE;
		dt.stat_fall   = LOGO_PRIOR_DECIDE;
		if (confirm){
			dt.outtype_rise = LOGO_RESULT_DECIDE;
			dt.outtype_fall = LOGO_RESULT_DECIDE;
			dt.result_rise  = msec_st;
			dt.result_fall  = msec_ed;
		}
		else{
			dt.outtype_rise = LOGO_RESULT_NONE;
			dt.outtype_fall = LOGO_RESULT_NONE;
			dt.result_rise  = 0;
			dt.result_fall  = 0;
		}
		if (flag_bak_rise){
			dt.outtype_rise = bak_outtype_rise;
			dt.result_rise  = bak_result_rise;
		}
		if (flag_bak_fall){
			dt.outtype_fall = bak_outtype_fall;
			dt.result_fall  = bak_result_fall;
		}
		dt.unit_rise        = LOGO_UNIT_NORMAL;
		dt.unit_fall        = LOGO_UNIT_NORMAL;
		if (unit){
			dt.unit_rise    = LOGO_UNIT_DIVIDE;
			dt.unit_fall    = LOGO_UNIT_DIVIDE;
		}
		if ( separator == false ){		// 2�_�w���Ԃ̑}��
			if (total_ins == 0){
				setRecordLogo(dt, num_ins);
			}
			else{
				insertRecordLogo(dt, num_ins);
			}
		}
	}
	return retval;
}


//---------------------------------------------------------------------
// �V�[���`�F���W��}��
// ���́F
//   msec_dst_s  : �}���ʒu
//   msec_dst_bk : ���O�I���ʒu
//   nsc_mute    : �����ʒu�֌W�̏����R�s�[���閳��SC�ԍ��i-1�̎������j
//   stat_scpos_dst : �}���\���ɐݒ肷��D��x
// �o�́F
//   �Ԃ�l�F�}�������ꏊ�ԍ�
//---------------------------------------------------------------------
Nsc JlsDataset::insertScpos(Msec msec_dst_s, Msec msec_dst_bk, Nsc nsc_mute, ScpPriorType stat_scpos_dst){
	int num_scpos = sizeDataScp();
	if (num_scpos <= 1){		// �V�[���`�F���W��ǂݍ��݂ł��ĂȂ��ꍇ
		return -1;
	}

	// �}���ꏊ������
	Nsc nsc_ins = 1;
	while( getMsecScp(nsc_ins) < msec_dst_s && nsc_ins < num_scpos - 1){
		nsc_ins ++;
	}

	// �}���ꏊ���m��
	bool flag_ins = 0;
	if ( getMsecScp(nsc_ins) != msec_dst_s || nsc_ins == num_scpos - 1){
		flag_ins = true;
	}

	// �������݈ʒu�I��
	DataScpRecord  dtscp;
	if (flag_ins){
		clearRecordScp(dtscp);
		dtscp.msec = msec_dst_s;
		dtscp.msbk = msec_dst_bk;
		if (nsc_mute >= 0 && nsc_mute < num_scpos){
			DataScpRecord dttmp;
			getRecordScp(dttmp, nsc_mute);
			dtscp.msmute_s = dttmp.msmute_s;
			dtscp.msmute_e = dttmp.msmute_e;
		}
		else{
			dtscp.msmute_s = msec_dst_bk;
			dtscp.msmute_e = msec_dst_s;
		}
		dtscp.statpos = stat_scpos_dst;
		insertRecordScp(dtscp, nsc_ins);			// �}���ɂ��X�V
	}
	else{
		getRecordScp(dtscp, nsc_ins);
		dtscp.statpos = stat_scpos_dst;
		setRecordScp(dtscp, nsc_ins);				// ���������ɂ��X�V
	}
	return nsc_ins;
}

//---------------------------------------------------------------------
// �w��ʒu�i�~���b�j�̖���SC�ԍ����擾�iLogoExact�ݒ�ɂ�蒲���L���I���j
// �Ȃ���΋����I�ɍ쐬���Đݒ�
// ���́F
//   msec_in : �Ώۈʒu�i�~���b�j
//   edge    : �I���G�b�W�iLOGO_EDGE_RISE=�J�n���ALOGO_EDGE_FALL=�I�����j
//   exact   : false=�ʏ�  true=���m�Ȉʒu�w��
// �o�́F
//   �Ԃ�l�F�Ή�����ꏊ�ԍ�
// ���ӓ_�F
//   �f�[�^�}���ɂ��V�[���`�F���W�ԍ�(nsc)�ύX����
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsecExact(Msec msec_in, LogoEdgeType edge, bool exact){
	if ( exact ){
		return getNscForceExactFixMsec(msec_in, edge);
	}
	return getNscForceMsec(msec_in, edge);
}
//---------------------------------------------------------------------
// �s�v�ȏd�������V�[���`�F���W���폜
//---------------------------------------------------------------------
bool JlsDataset::restructScp(){
	if ( !isAutoModeUse() ) return false;	// Auto�����Ȃ���Ή������Ȃ�
	int num_scpos = sizeDataScp();
	Nsc  nscHold = -1;
	Msec msecHold = -1;
	bool decideHold = false;
	bool change = false;
	for(int i=num_scpos-1; i>=0; i--){
		Msec msecCur = getMsecScp(i);
		bool decideCur = jlsd::isScpChapTypeDecide(getScpChap(i));
		if ( i == 0 || i == num_scpos-1 ){
			decideCur = true;
		}
		bool update = false;
		if ( nscHold < 0 ){	// ����
			update = true;
		}else if ( abs(msecCur - msecHold) > msecValExact ){	// �O�Ɨ���Ă����玟
			update = true;
		}else if ( !decideHold ){	// �����ʒu�Ō㑤�����m��Ȃ�㑤�폜
			deleteRecordScp(nscHold);
			change = true;
			update = true;
		}else if ( !decideCur ){	// �����ʒu�Ō㑤�m��A�O�����m��Ȃ�O���폜
			deleteRecordScp(i);
			change = true;
			nscHold -= 1;			// �㑤��1�O�ɂ����
		}
		if ( update ){
			nscHold = i;
			msecHold = msecCur;
			decideHold = decideCur;
		}
	}
	return change;
}
//---------------------------------------------------------------------
// �w��ʒu�i�~���b�j�̖���SC�ԍ����擾�iLogoExact�ݒ�ɂ�蒲���L���I���j
// �Ȃ���΋����I�ɍ쐬���Đݒ�
// ���́F
//   msec_in : �Ώۈʒu�i�~���b�j
//   edge    : �I���G�b�W�iLOGO_EDGE_RISE=�J�n���ALOGO_EDGE_FALL=�I�����j
// �o�́F
//   �Ԃ�l�F�Ή�����ꏊ�ԍ�
// ���ӓ_�F
//   �f�[�^�}���ɂ��V�[���`�F���W�ԍ�(nsc)�ύX����
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsec(Msec msec_in, LogoEdgeType edge){
	if ( (extOpt.nLgExact & 0x02) != 0 ){		// ���S�ʒu�����Ȃ�
		return getNscForceExactFixMsec(msec_in, edge);
	}
	return getNscForceMsecOrg(msec_in, edge);
}

//---------------------------------------------------------------------
// �w��ʒu�i�~���b�j�̖���SC�ԍ����擾
// �Ȃ���΋����I�ɍ쐬���Đݒ�
// ���́F
//   msec_in : �Ώۈʒu�i�~���b�j
//   edge    : �I���G�b�W�iLOGO_EDGE_RISE=�J�n���ALOGO_EDGE_FALL=�I�����j
// �o�́F
//   �Ԃ�l�F�Ή�����ꏊ�ԍ�
// ���ӓ_�F
//   �f�[�^�}���ɂ��V�[���`�F���W�ԍ�(nsc)�ύX����
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceMsecOrg(Msec msec_in, LogoEdgeType edge){
	Msec msec_clr = msecValNear2;			// �d�Ȃ������̊m��ӏ�����������͈�
	int num_scpos = sizeDataScp();

	if (msec_in <= 0 ){
		return 0;
	}
	if ( msec_in >= getMsecTotalMax() ){
		return num_scpos-1;
	}

	Nsc nsc_nearest = 0;
	Msec difmsec_nearest = 0;
	Nsc nsc_mute = -1;
	//--- ��ԋ߂��������� ---
	int i = 1;
	bool flag_search = true;
	while(flag_search){
		DataScpRecord dtscp;
		getRecordScp(dtscp, i);
		Msec msec_i = getMsecScpEdge(i, edge);
		Msec difmsec_i = abs(msec_in - msec_i);
		//--- ��ԋ߂��ꍇ�̍X�V ---
		if (difmsec_nearest > difmsec_i || nsc_nearest == 0){
			//--- �d�Ȃ��Ă���ΏۊO�ƂȂ����ꏊ���O�� ---
			if (nsc_nearest > 0 && difmsec_nearest <= msec_clr){
				setScpChap(nsc_nearest, SCP_CHAP_DUPE);
			}
			nsc_nearest = i;
			difmsec_nearest = difmsec_i;
		}
		else if (difmsec_i <= msec_clr){		// �d�Ȃ��Ă��鏊���O��
			setScpChap(i, SCP_CHAP_DUPE);
		}
		//--- �������ԗ̈�̊m�F ---
		if (dtscp.msmute_s <= msec_i && msec_i <= dtscp.msmute_e){
			if (msec_i <= msec_in || nsc_nearest == i){
				nsc_mute = i;
			}
		}
		//--- ���̈ʒu�ݒ� ---
		i ++;
		if (i >= num_scpos-1 || msec_i >= msec_in + msec_clr){
			flag_search = false;
		}
	}
	//--- �}���ʒu��ݒ� ---
	Msec msec_in_s  = msec_in;
	Msec msec_in_bk = msec_in;
	if (edge == LOGO_EDGE_RISE){
		msec_in_bk = cnv.getMsecAdjustFrmFromMsec(msec_in, -1);
	}
	else{
		msec_in_s  = cnv.getMsecAdjustFrmFromMsec(msec_in, +1);
	}
	Nsc nsc_ret = nsc_nearest;
	//--- ��������SC�ӏ����m�F ---
	bool flag_new = true;
	if (nsc_nearest > 0){
		int msec_near_s  = getMsecScp(nsc_nearest);
		int msec_near_bk = getMsecScpBk(nsc_nearest);
		if (msec_near_bk <= msec_in_s && msec_in_bk <= msec_near_s){
			flag_new = false;
		}
	}
	//--- �V�K�ӏ��Ȃ�}�� ---
	if (flag_new){
		nsc_ret = insertScpos(msec_in_s, msec_in_bk, nsc_mute, SCP_PRIOR_DECIDE);
	}
	return nsc_ret;
}

//---------------------------------------------------------------------
// �w��ʒu�i�~���b�j�̖���SC�ԍ����擾�i�ʒu�����Ȃ��œ��͂𐳊m�ȌŒ�ʒu�Ƃ���j
// �Ȃ���΋����I�ɍ쐬���Đݒ�
// ���́F
//   msec_in : �Ώۈʒu�i�~���b�j
//   edge    : �I���G�b�W�iLOGO_EDGE_RISE=�J�n���ALOGO_EDGE_FALL=�I�����j
// �o�́F
//   �Ԃ�l�F�Ή�����ꏊ�ԍ�
// ���ӓ_�F
//   �f�[�^�}���ɂ��V�[���`�F���W�ԍ�(nsc)�ύX����
//---------------------------------------------------------------------
Nsc JlsDataset::getNscForceExactFixMsec(Msec msec_in, LogoEdgeType edge){
	Msec msec_clr = msecValNear2;			// �d�Ȃ������̊m��ӏ�����������͈�
	int num_scpos = sizeDataScp();

	if (msec_in <= 0 ){
		return 0;
	}
	if ( msec_in >= getMsecTotalMax() ){
		return num_scpos-1;
	}

	Nsc nsc_nearest = 0;
	Msec difmsec_nearest = 0;
	Nsc nsc_mute = -1;

	int frmIn = cnv.getFrmFromMsec(msec_in);	// ���͈ʒu�̃t���[��
	bool flagExactNearest = false;	// ������W����
	bool flagFcNearest = false;		// �ێ����W�͌Œ�
	Msec msecFcPrev = 0;		// �O���ɂ���Œ�ʒu
	Msec msecFcOver = 0;		// �㑤�ɂ���Œ�ʒu
	//--- ��ԋ߂��������� ---
	int i = 1;
	bool flag_search = true;
	while(flag_search){
		DataScpRecord dtscp;
		getRecordScp(dtscp, i);
		Msec msec_i = getMsecScpEdge(i, edge);
		Msec difmsec_i = abs(msec_in - msec_i);
		ScpChapType chap_i = getScpChap(i);
		bool flagChapFc = ( chap_i == SCP_CHAP_DFORCE || chap_i == SCP_CHAP_DUNIT)? true : false;
		//--- �ʒu�֌W ---
		int frmSt = cnv.getFrmFromMsec(dtscp.msec);
		int frmBk = cnv.getFrmFromMsec(dtscp.msbk);
		bool flagLocSame = false;
		if ( frmSt-2 <= frmIn && frmIn <= frmBk+2 ){
			if ( (frmBk <= frmIn || jlsd::isLogoEdgeRise(edge) == false) &&
			     (frmSt >= frmIn || jlsd::isLogoEdgeFall(edge) == false) ){
				flagLocSame = true;
			}
		}
		//--- ��ԋ߂��ꍇ�̍X�V ---
		if (difmsec_nearest > difmsec_i || nsc_nearest == 0){
			bool flagErase = false;
			bool flagChange;
			if ( flagLocSame ){			// ������W
				flagChange = true;
				flagExactNearest = true;
			}else if ( msecFcPrev >= msec_i && msecFcPrev > 0 ){	// �O�̌Œ���O
				flagChange = false;
			}else if ( msecFcOver <= msec_i && msecFcOver > 0 ){	// ��̌Œ����
				flagChange = false;
			}else if ( flagChapFc ){	// �ύX�s�i�Œ�j�ʒu
				flagChange = false;
				if ( msec_i < msec_in && (msecFcPrev < msec_i || msecFcPrev == 0) ){
					msecFcPrev = msec_i;
					if ( nsc_nearest > 0 && nsc_nearest < i ){	// �O�̌��͍폜����
						flagErase = true;
					}
				}else if ( msec_i > msec_in && (msecFcOver > msec_i || msecFcOver == 0) ){
					msecFcOver = msec_i;
				}
			}else{
				flagChange = true;
			}
			if ( flagChange || flagErase ){
				//--- �d�Ȃ��Ă���ΏۊO�ƂȂ����ꏊ���O�� ---
				if (nsc_nearest > 0 && difmsec_nearest <= msec_clr && flagFcNearest == false){
					setScpChap(nsc_nearest, SCP_CHAP_DUPE);
				}
				nsc_nearest = 0;	// ���Ȃ�
			}
			if ( flagChange ){
				nsc_nearest = i;
				difmsec_nearest = difmsec_i;
				flagFcNearest = flagChapFc;
			}
		}
		else if (difmsec_i <= msec_clr){		// �d�Ȃ��Ă��鏊���O��
			if ( flagChapFc == false ){			// �Œ�łȂ����
				setScpChap(i, SCP_CHAP_DUPE);
			}
		}
		//--- �������ԗ̈�̊m�F ---
		if (dtscp.msmute_s <= msec_i && msec_i <= dtscp.msmute_e){
			if (msec_i <= msec_in || nsc_nearest == i){
				nsc_mute = i;
			}
		}
		//--- ���̈ʒu�ݒ� ---
		i ++;
		if (i >= num_scpos-1 || msec_i >= msec_in + msec_clr){
			flag_search = false;
		}
	}
	//--- �}���ʒu��ݒ� ---
	Msec msec_in_s  = msec_in;
	Msec msec_in_bk = msec_in;
	if (edge == LOGO_EDGE_RISE){
		msec_in_bk = cnv.getMsecAdjustFrmFromMsec(msec_in, -1);
	}
	else{
		msec_in_s  = cnv.getMsecAdjustFrmFromMsec(msec_in, +1);
	}
	Nsc nsc_ret = nsc_nearest;
	//--- ��������SC�ӏ����m�F ---
	bool flag_new = true;
	if (nsc_nearest > 0){
		if ( flagExactNearest ){
			flag_new = false;
			//--- �������e���C�� ---
			DataScpRecord dtscp;
			getRecordScp(dtscp, nsc_nearest);
			int frmSt = cnv.getFrmFromMsec(dtscp.msec);
			int frmBk = cnv.getFrmFromMsec(dtscp.msbk);
			if ( jlsd::isLogoEdgeRise(edge) ){
				if ( frmSt != frmIn ){
					dtscp.msec = msec_in;
				}
			}
			if ( jlsd::isLogoEdgeFall(edge) ){
				if ( frmBk != frmIn ){
					dtscp.msbk = msec_in;
				}
			}
			setRecordScp(dtscp, nsc_nearest);
		}
	}
	//--- �V�K�ӏ��Ȃ�}�� ---
	if (flag_new){
		nsc_ret = insertScpos(msec_in_s, msec_in_bk, nsc_mute, SCP_PRIOR_DECIDE);
	}
	return nsc_ret;
}


//---------------------------------------------------------------------
// ���S����/�Ȃ��ύX
//---------------------------------------------------------------------
void JlsDataset::changeLogoOnOff(Nsc nsc, bool logoOn){
	ScpArType arStatOrg = getScpArstat(nsc);
	ScpArType arStatNew;
	switch( arStatOrg ){
		case SCP_AR_L_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_BUNIT;
			break;
		case SCP_AR_L_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		case SCP_AR_L_MIXED :
			arStatNew = (logoOn)? SCP_AR_L_MIXED : SCP_AR_N_OTHER;
			break;
		case SCP_AR_N_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_UNIT;
			break;
		case SCP_AR_N_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		case SCP_AR_N_AUNIT :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_AUNIT;
			break;
		case SCP_AR_N_BUNIT :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_BUNIT;
			break;
		case SCP_AR_B_UNIT :
			arStatNew = (logoOn)? SCP_AR_L_UNIT : SCP_AR_N_UNIT;
			break;
		case SCP_AR_B_OTHER :
			arStatNew = (logoOn)? SCP_AR_L_OTHER : SCP_AR_N_OTHER;
			break;
		default :
			arStatNew = SCP_AR_UNKNOWN;
			break;
	}
	// ���S���肩�烍�S�Ȃ��ɕύX�������͊��Ԃ�CM���f
	if ( jlsd::isScpArTypeLogo(arStatOrg) && logoOn == false){
		Nsc nscPrev = getNscPrevScpDecide(nsc, SCP_END_EDGEIN);
		if ( isCmLenNscToNsc(nscPrev, nsc) ){
			arStatNew = SCP_AR_N_UNIT;
		}else{
			arStatNew = SCP_AR_N_OTHER;
		}
	}
	setScpArstat(nsc, arStatNew);
	setScpArext(nsc, SCP_AREXT_NONE);
}
//---------------------------------------------------------------------
// �\���ύX���̕\���p�ɕ␳����
//---------------------------------------------------------------------
void JlsDataset::changeChapDispUnit(Nsc nscFrom, Nsc nscTo, bool cutDivUnit){
	//--- �Ώۍ\���̕␳ ---
	changeChapDispUnitSub(nscFrom, nscTo, cutDivUnit);
}
void JlsDataset::changeChapDispUnitWithSide(Nsc nscFrom, Nsc nscTo){
	//--- �P�ʍ\���������̃J�b�g�Ȃ� ---
	bool cutDivUnit = false;
	//--- �Ώۍ\���̕␳ ---
	changeChapDispUnitSub(nscFrom, nscTo, cutDivUnit);
	//--- �O��\���̕␳ ---
	if ( nscFrom >= 0 ){
		Nsc nscPrev = getNscPrevScpDisp(nscFrom, SCP_END_EDGEIN);
		changeChapDispUnitSub(nscPrev, nscFrom, cutDivUnit);
	}
	if ( nscTo >= 0 ){
		Nsc nscNext = getNscNextScpDisp(nscTo, SCP_END_EDGEIN);
		changeChapDispUnitSub(nscTo, nscNext, cutDivUnit);
	}
}
void JlsDataset::changeChapDispUnitSub(Nsc nscFrom, Nsc nscTo, bool cutDivUnit){
	if ( nscFrom < 0 ) return;

	bool flagDiv = false;		// �\���p�ɂȂ������r��
	bool flagUnit = false;		// 15�b�P�ʂƂȂ�CM����
	Nsc nscCur  = nscFrom;
	Nsc nscDisp = nscFrom;
	Nsc nscPrev = nscFrom;
	while( nscFrom <= nscCur && nscCur <= nscTo && nscCur >= 0 ){
		//--- ���̈ʒu���擾 ---
		if ( nscDisp <= nscCur ){
			nscPrev = nscDisp;
			nscDisp = getNscNextScpDisp(nscCur, SCP_END_EDGEIN);	// �\���p���ʒu
			flagUnit = isCmLenNscToNsc(nscPrev, nscDisp);
		}
		nscCur = getNscNextScpDecide(nscCur, SCP_END_EDGEIN);
		if ( nscCur < 0 || nscDisp < 0 ) break;

		//--- �\���p�ɂȂ���؂蕪�����J�b�g ---
		if ( nscCur != nscDisp && cutDivUnit ){	// 15/30�b�\����CM�����̃J�b�g����
			if ( nscCur < nscTo ){			// �O�̂��ߔ͈͓����̎��̂�
				setScpChap(nscCur, SCP_CHAP_NONE);
			}
			continue;
		}
		//--- �\����؂�̕␳ ---
		ScpArType arstat = pdata->getScpArstat(nscCur);
		ScpArType arstatRev;
		switch( arstat ){
			case SCP_AR_N_UNIT:
			case SCP_AR_N_OTHER:
			case SCP_AR_N_AUNIT:
			case SCP_AR_N_BUNIT:
				if ( !cutDivUnit && (nscCur != nscDisp || flagDiv) ){
					if ( !flagUnit ){
						arstatRev = SCP_AR_N_OTHER;
					}else if ( flagDiv ){
						arstatRev = SCP_AR_N_AUNIT;
					}else{
						arstatRev = SCP_AR_N_BUNIT;
					}
				}else if ( flagUnit ){
					arstatRev = SCP_AR_N_UNIT;
				}else{
					arstatRev = SCP_AR_N_OTHER;
				}
				break;
			default:
				arstatRev =arstat;
				break;
		}
		if ( arstatRev != arstat ){
			setScpArstat(nscCur, arstatRev);
		}
		flagDiv = ( nscCur != nscDisp );
	}
}
bool JlsDataset::isCmLenNscToNsc(Nsc nscFrom, Nsc nscTo){
	if ( nscFrom < 0 || nscTo < 0 ) return false;
	Msec msec1 = getMsecScp(nscFrom);
	Msec msec2 = getMsecScp(nscTo);
	int n = cnv.getSecFromMsec(abs(msec1 - msec2));
	if ( n > 0 && n <= 120 && (n % 15 == 0) ) return true;
	return false;
}

//=====================================================================
// �����\�����x������
//=====================================================================
//---------------------------------------------------------------------
// �R�����g��ނ��擾
//---------------------------------------------------------------------
bool JlsDataset::isLabelLogoFromNsc(Nsc nsc, bool flagBorder, bool flagOut){
	ComLabelType label = getLabelTypeFromNsc(nsc, flagOut);
	return isLabelLogo(label, flagBorder, flagOut);
}
ComLabelType JlsDataset::getLabelTypeFromNsc(Nsc nsc, bool flagOut){
	ScpArType arstat = getScpArstat(nsc);
	ScpArExtType arext = getScpArext(nsc);
	return getLabelTypeFromStat(arstat, arext, flagOut);
}
// ���S�����̔���
bool JlsDataset::isLabelLogo(ComLabelType label, bool flagBorder, bool flagOut){
	bool logoOn;
	switch( label ){
		case ComLabelType::Mix :
		case ComLabelType::Logo :
		case ComLabelType::AddNEdge :
		case ComLabelType::AddLEdge :
		case ComLabelType::AddTR :
		case ComLabelType::RawTR :
		case ComLabelType::CanTR :
		case ComLabelType::AddSP :
		case ComLabelType::AddEC :
			logoOn = true;
			break;
		case ComLabelType::Bd :
		case ComLabelType::Bd15s :
			logoOn = (flagBorder || flagOut);
			break;
		default :
			logoOn = false;
			break;
	}
	return logoOn;
}
// ��ނ��擾
ComLabelType JlsDataset::getLabelTypeFromStat(ScpArType arstat, ScpArExtType arext, bool flagOut){
	ComLabelType label;

	switch( arstat ){
		case SCP_AR_N_UNIT :
		case SCP_AR_N_AUNIT :
		case SCP_AR_N_BUNIT :
			label = ComLabelType::CM;	// ":CM"
			break;
		case SCP_AR_N_OTHER :
			label = ComLabelType::NoLogo;	// ":Nologo"
			break;
		case SCP_AR_B_UNIT  :
			label = ComLabelType::Bd15s;	// ":Border15s"
			break;
		case SCP_AR_B_OTHER :
			label = ComLabelType::Bd;	// ":Border"
			break;
		case SCP_AR_L_UNIT :
		case SCP_AR_L_OTHER :
			label = ComLabelType::Logo;	// ":L"
			break;
		case SCP_AR_L_MIXED :
			label = ComLabelType::Mix;	// ":Mix"
			break;
		default :
			label = ComLabelType::None;	// ":"
			break;
	}
	if ( flagOut == false ){
		return label;
	}
	switch( arext ){
		case SCP_AREXT_L_TRKEEP :
			label = ComLabelType::AddTR;	// ":Trailer(add)"
			break;
		case SCP_AREXT_L_ECCUT :
			label = ComLabelType::CutTR;	// ":Trailer(cut)"
			break;
		case SCP_AREXT_L_TRRAW :
			label = ComLabelType::RawTR;	// ":Trailer"
			break;
		case SCP_AREXT_L_TRCUT :
			if (getConfig(ConfigVarType::flagCutTR) == 1){
				label = ComLabelType::CutTR;	// ":Trailer(cut)"
			}
			else{
				label = ComLabelType::CanTR;	// ":Trailer(cut-cancel)"
			}
			break;
		case SCP_AREXT_L_SP :
			if (getConfig(ConfigVarType::flagCutSP) == 1){
				label = ComLabelType::CutSP;	// ":Sponsor(cut)"
			}
			else{
				label = ComLabelType::AddSP;	// ":Sponsor(add)"
			}
			break;
		case SCP_AREXT_L_EC :
			label = ComLabelType::AddEC;	// ":Endcard(add)"
			break;
		case SCP_AREXT_L_LGCUT :
			label = ComLabelType::CutLEdge;	// ":L-Edge(cut)"
			break;
		case SCP_AREXT_L_LGADD :
			label = ComLabelType::AddLEdge;	// ":L-Edge(add)"
			break;
		case SCP_AREXT_N_TRCUT :
			label = ComLabelType::CutNoLogo;	// ":Nologo(cut)"
			break;
		case SCP_AREXT_N_LGCUT :
			label = ComLabelType::CutNEdge;	// ":N-Edge(cut)"
			break;
		case SCP_AREXT_N_LGADD :
			label = ComLabelType::AddNEdge;	// ":N-Edge(add)"
			break;
		default:
			break;
	}
	return label;
}
// ������擾
string JlsDataset::getLabelStr(ComLabelType label){
	const char *pstr = "";
	switch( label ){
		case ComLabelType::CM :
			pstr = ":CM";
			break;
		case ComLabelType::NoLogo :
			pstr = ":Nologo";
			break;
		case ComLabelType::CutNoLogo :
			pstr = ":Nologo(cut)";
			break;
		case ComLabelType::CutNEdge :
			pstr = ":N-Edge(cut)";
			break;
		case ComLabelType::CutLEdge :
			pstr = ":L-Edge(cut)";
			break;
		case ComLabelType::CutTR :
			pstr = ":Trailer(cut)";
			break;
		case ComLabelType::CutSP :
			pstr = ":Sponsor(cut)";
			break;
		case ComLabelType::Bd :
			pstr = ":Border";
			break;
		case ComLabelType::Bd15s :
			pstr = ":Border15s";
			break;
		case ComLabelType::Mix :
			pstr = ":Mix";
			break;
		case ComLabelType::Logo :
			pstr = ":L";
			break;
		case ComLabelType::AddNEdge :
			pstr = ":N-Edge(add)";
			break;
		case ComLabelType::AddLEdge :
			pstr = ":L-Edge(add)";
			break;
		case ComLabelType::AddTR :
			pstr = ":Trailer(add)";
			break;
		case ComLabelType::RawTR :
			pstr = ":Trailer";
			break;
		case ComLabelType::CanTR :
			pstr = ":Trailer(cut-cancel)";
			break;
		case ComLabelType::AddSP :
			pstr = ":Sponsor(add)";
			break;
		case ComLabelType::AddEC :
			pstr = ":Endcard(add)";
			break;
		default :
			pstr = ":";
			break;
	}
	string strRet = pstr;
	return strRet;
}



//=====================================================================
// �\�����̃��S�\�����Ԃ̎擾
//=====================================================================

//---------------------------------------------------------------------
// �͈͓��ɂ��郍�S�\�����Ԃ̕b�����擾
//---------------------------------------------------------------------
Sec JlsDataset::getSecLogoComponent(Msec msec_s, Msec msec_e){
	//--- �����S�f�[�^���g��Ȃ��ꍇ�̃��S���Ԑݒ� ---
	if ( isAutoLogoOnly() ){
		return getSecLogoComponentFromElg(msec_s, msec_e);
	}
	return getSecLogoComponentFromLogo(msec_s, msec_e);
}

// �����S�f�[�^����
Sec JlsDataset::getSecLogoComponentFromLogo(Msec msec_s, Msec msec_e){
	NrfCurrent logopt = {};
	Msec sum_msec = 0;
	//--- �\���̒[�������̂Ă锻�ʊ��� ---
	Msec msec_dif_mid = abs(msec_e - msec_s) / 2;
	Msec msec_dif_min = abs(msec_e - msec_s) / 4;
	if (msec_dif_mid > 11500){
		msec_dif_mid = 11500;
	}
	if (msec_dif_min > 4500){
		msec_dif_min = 4500;
	}
	while( getNrfptNext(logopt, LOGO_SELECT_VALID) ){
		//--- �͈͓��Ƀ��S�\�����Ԃ�����ꍇ ---
		if (logopt.msecRise + msecValSpc < msec_e && logopt.msecFall > msec_s + msecValSpc){
			Msec tmp_s = (logopt.msecRise < msec_s)? msec_s : logopt.msecRise;
			Msec tmp_e = (logopt.msecFall > msec_e)? msec_e : logopt.msecFall;
			//--- �[�����̏���(rise) ---
			{
				WideMsec wmsec;
				getWideMsecLogoNrf(wmsec, logopt.nrfRise);
				if (logopt.msecRise < msec_s + msec_dif_mid && logopt.msecFall >= msec_e){
					if (wmsec.early <= msec_s + msecValSpc && tmp_s > msec_s){
						tmp_s = msec_s;
					}
				}
				if (logopt.msecRise > msec_e - msec_dif_min){
					tmp_s = tmp_e;
				}
				else if (logopt.msecRise > msec_e - msec_dif_mid && logopt.msecFall >= msec_e){
					if (wmsec.late >= msec_e - msec_dif_min){
						tmp_s = tmp_e;
					}
				}
			}
			//--- �[�����̏���(fall) ---
			{
				WideMsec wmsec;
				getWideMsecLogoNrf(wmsec, logopt.nrfFall);
				if (logopt.msecFall > msec_e - msec_dif_mid && logopt.msecRise <= msec_s){
					if (wmsec.late >= msec_e - msecValSpc && tmp_e < msec_e){
						tmp_e = msec_e;
					}
				}
				if (logopt.msecFall < msec_s + msec_dif_min){
					tmp_e = tmp_s;
				}
				else if (logopt.msecFall < msec_s + msec_dif_mid && logopt.msecRise <= msec_s){
					if (wmsec.early <= msec_s + msec_dif_min){
						tmp_s = tmp_e;
					}
				}
			}
			// ���S�\�����Ԃ�ǉ�
			if (tmp_s < tmp_e){
				sum_msec += (tmp_e - tmp_s);
			}
		}
	}
	// �b���ɕϊ����ă��^�[��
	return cnv.getSecFromMsec(sum_msec);
}

// �����\���̃��S��������
Sec JlsDataset::getSecLogoComponentFromElg(Msec msec_s, Msec msec_e){
	Msec sum_msec = 0;
	ElgCurrent elg = {};
	while( getElgptNext(elg) ){
		// �͈͓��Ƀ��S�\�����Ԃ�����ꍇ
		if (elg.msecRise + msecValSpc < msec_e && elg.msecFall > msec_s + msecValSpc){
			Msec tmp_s, tmp_e;
			if (elg.msecRise < msec_s){
				tmp_s = msec_s;
			}
			else{
				tmp_s = elg.msecRise;
			}
			if (elg.msecFall > msec_e){
				tmp_e = msec_e;
			}
			else{
				tmp_e = elg.msecFall;
			}
			// ���S�\�����Ԃ�ǉ�
			if (tmp_s < tmp_e){
				sum_msec += (tmp_e - tmp_s);
			}
		}
	}
	// �b���ɕϊ����ă��^�[��
	return cnv.getSecFromMsec(sum_msec);
}



//=====================================================================
// Trim�ʒu���ڐݒ�
//=====================================================================
void JlsDataset::setOutDirect(vector<Msec>& listMsec){
	m_listOutDirect = listMsec;
}

//=====================================================================
// ����Trim�쐬
//=====================================================================

//---------------------------------------------------------------------
// �J�b�g���ʍ쐬
//---------------------------------------------------------------------
void JlsDataset::outputResultTrimGen(){
	resultTrim.clear();
	if ( m_listOutDirect.empty() ){
		if ( isAutoModeUse() ){
			outputResultTrimGenAuto();
		}else{
			outputResultTrimGenManual();
		}
	}else{		// Trim�ʒu���ڐݒ�
		for(int i=0; i<(int)m_listOutDirect.size(); i++){
			resultTrim.push_back( m_listOutDirect[i] );
		}
	}
}

// �J�b�g���ʍ쐬�i�\���������Ȃ��ꍇ�j
void JlsDataset::outputResultTrimGenManual(){
	LogoSelectType lgsel = LogoSelectType::LOGO_SELECT_VALID;
	bool final = true;
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = getNrfptOutNext(logopt, lgsel, final);
		if ( flag_cont ){
			//--- ���ʃf�[�^�ǉ� ---
			resultTrim.push_back( logopt.msecRise );
			resultTrim.push_back( logopt.msecFall );
		}
	}
}


// �J�b�g���ʍ쐬�i�\����������ꍇ�j
void JlsDataset::outputResultTrimGenAuto(){
	ElgCurrent elg = {};
	elg.outflag = true;					// �ŏI�o�͗p�̐ݒ�
	while ( getElgptNext(elg) ){
		resultTrim.push_back( elg.msecRise );
		resultTrim.push_back( elg.msecFall );
	}
}



//=====================================================================
// �ڍ׏��o�͍쐬
//=====================================================================

//---------------------------------------------------------------------
// �ڍ׏�񌋉ʂ̓ǂݏo���ʒu���Z�b�g
//---------------------------------------------------------------------
void JlsDataset::outputResultDetailReset(){
	m_nscOutDetail = 0;
}

//---------------------------------------------------------------------
// �ڍ׏�񌋉ʂ̕�����쐬
// �o��
//   �Ԃ�l�F 0=����  1=�I��
//   strBuf�F �o�͕�����
//---------------------------------------------------------------------
bool JlsDataset::outputResultDetailGetLine(string &strBuf){
	int num_scpos = sizeDataScp();
	//--- ���݈ʒu���擾 ---
	Nsc i = m_nscOutDetail;
	if (i < 0 || i >= num_scpos-1){
		return 1;
	}
	Msec msec_from = getMsecScp(i);
	//--- ���̈ʒu���擾 ---
	Msec msec_next;
	do{
		i = getNscNextScpDisp(i, SCP_END_EDGEIN);		// ���̍\����؂�ʒu�擾�i�[���܂ށj
		msec_next = getMsecScp(i);
	} while(msec_from == msec_next && i >= 0);
	//--- �ʒu��ݒ� ---
	m_nscOutDetail = i;
	if (i >= 0){
		//--- �O��Ԋu�̊��Ԏ擾 ---
		Msec msec_dif = msec_next - msec_from;
		Sec  sec_dif  = cnv.getSecFromMsec( msec_dif  );
		int frm_from = cnv.getFrmFromMsec( msec_from );
		int frm_next = cnv.getFrmFromMsec( msec_next );
		int frm_dif  = frm_next - cnv.getFrmFromMsec(sec_dif*1000 + msec_from);
		Sec sec_logo = getSecLogoComponent(msec_from, msec_next);
		//--- �I���n�_���擾 ---
		Msec msec_to  = getMsecScpBk(i);
		int frm_to   = cnv.getFrmFromMsec( msec_to   );
		//--- �\�� ---
		strBuf = sformat("%6d %6d %4d %3d %4d ",
					frm_from, frm_to, sec_dif, frm_dif, sec_logo);
		//--- �\�����̂��擾 ---
		if (m_flagSetupAuto > 1){
			outputResultDetailGetLineLabel(strBuf, getScpArstat(i), getScpArext(i));
		}
		return 0;
	}
	return 1;		// EOF
}





//---------------------------------------------------------------------
// �ڍ׏�񌋉ʂ̕�����쐬�i�����������́j
//---------------------------------------------------------------------
void JlsDataset::outputResultDetailGetLineLabel(string &strBuf, ScpArType arstat, ScpArExtType arext){
	bool flagOut = true;
	ComLabelType label = getLabelTypeFromStat(arstat, arext, flagOut);
	strBuf += getLabelStr(label);
}



//=====================================================================
// ���O�\��
//=====================================================================
void JlsDataset::dispSysMesN(const string& msg, SysMesType typeMsg){
	bool flagDisp;
	switch( typeMsg ){
		case SysMesType::CutMrg :
			flagDisp = ( (pdata->extOpt.dispSysMes & 0x4) != 0 );
			break;
		case SysMesType::OutDirect :
			flagDisp = ( (pdata->extOpt.dispSysMes & 0x2) != 0 );
			break;
		case SysMesType::CallFile :
			flagDisp = ( (pdata->extOpt.dispSysMes & 0x1) != 0 );
			break;
		default :
			flagDisp = false;
			break;
	}
	if ( flagDisp ){
		lcout << msg << endl;
	}else{
		LSys.bufMemoIns(msg + "\n");
	}
}

//=====================================================================
// �f�o�b�O�p�\��
//=====================================================================

//---------------------------------------------------------------------
// �f�o�b�O�p�\���i���S�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::displayLogo(){
	int n = (int) m_logo.size();
	string buf;
	for(int i=0; i<n; i++){
		buf = sformat("%3d %6d %6d %6d %6d %6d %6d %d%d%d%d %d %d %d %d %d %d %6d %6d\n",
			i, cnv.getFrmFromMsec(m_logo[i].rise), cnv.getFrmFromMsec(m_logo[i].fall),
			cnv.getFrmFromMsec(m_logo[i].rise_l), cnv.getFrmFromMsec(m_logo[i].rise_r),
			cnv.getFrmFromMsec(m_logo[i].fall_l), cnv.getFrmFromMsec(m_logo[i].fall_r),
			m_logo[i].fade_rise, m_logo[i].fade_fall, m_logo[i].intl_rise, m_logo[i].intl_fall,
			m_logo[i].stat_rise, m_logo[i].stat_fall, m_logo[i].unit_rise, m_logo[i].unit_fall,
			m_logo[i].outtype_rise, m_logo[i].outtype_fall,
			cnv.getFrmFromMsec(m_logo[i].result_rise), cnv.getFrmFromMsec(m_logo[i].result_fall)
		);
		lcout << buf;
	};
}

//---------------------------------------------------------------------
// �f�o�b�O�p�\���i����SC�f�[�^�j
//---------------------------------------------------------------------
void JlsDataset::displayScp(){
	int n = (int) m_scp.size();
	string buf;
	for(int i=0; i<n; i++){
		buf = sformat("%3d %6d %6d %6d %6d %d %d (%8d %8d %8d %8d)%4d %2d %2d %2d)\n",
			i, cnv.getFrmFromMsec(m_scp[i].msec), cnv.getFrmFromMsec(m_scp[i].msbk),
			cnv.getFrmFromMsec(m_scp[i].msmute_s), cnv.getFrmFromMsec(m_scp[i].msmute_e),
			m_scp[i].still, m_scp[i].statpos,
			m_scp[i].msec, m_scp[i].msbk, m_scp[i].msmute_s, m_scp[i].msmute_e,
			m_scp[i].score, m_scp[i].chap, m_scp[i].arstat, m_scp[i].arext
		);
		lcout << buf;
	}
}

