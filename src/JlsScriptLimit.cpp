//
// ���s�X�N���v�g�R�}���h�̈�����������^�[�Q�b�g���i��
//
#include "stdafx.h"
#include "CommonJls.hpp"
#include "JlsScriptLimit.hpp"
#include "JlsCmdSet.hpp"
#include "JlsDataset.hpp"

///////////////////////////////////////////////////////////////////////
//
// ��������ɂ��^�[�Q�b�g�I��N���X
//
///////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------
// ������
//---------------------------------------------------------------------
JlsScriptLimit::JlsScriptLimit(JlsDataset *pdata){
	this->pdata  = pdata;
}

//=====================================================================
// �R�}���h���ʂ͈̔͌���
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h���ʂ͈̔͌���
//---------------------------------------------------------------------
void JlsScriptLimit::limitCommonRange(JlsCmdSet& cmdset){
	limitHeadTail(cmdset);						// �S�͈̔͐ݒ�
	limitWindow(cmdset);						// -F�n�I�v�V�����ݒ�
	limitListForTarget(cmdset);					// -TLhold�ɂ��͈͐ݒ�
}

//---------------------------------------------------------------------
// �͈͂̍Đݒ�
//---------------------------------------------------------------------
void JlsScriptLimit::resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec){
	//--- �͈͂�ݒ� ---
	limitHeadTailImm(cmdset, rmsec);			// �S�͈̔͐ݒ�
	limitWindow(cmdset);						// -F�n�I�v�V�����ƍ��킹���͈͂��ēx����
}

//---------------------------------------------------------------------
// HEADTIME/TAILTIME��`�ɂ��t���[���ʒu����
// �o�́F
//    cmdset.limit.setHeadTail()
//---------------------------------------------------------------------
void JlsScriptLimit::limitHeadTail(JlsCmdSet& cmdset){
	RangeMsec rmsec;
	rmsec.st = pdata->recHold.rmsecHeadTail.st;
	if (rmsec.st == -1){
		rmsec.st = 0;
	}
	rmsec.ed = pdata->recHold.rmsecHeadTail.ed;
	if (rmsec.ed == -1){
		rmsec.ed = pdata->getMsecTotalMax();
	}
	cmdset.limit.setHeadTail(rmsec);
}

//--- ���ڐ��l�ݒ� ---
void JlsScriptLimit::limitHeadTailImm(JlsCmdSet& cmdset, RangeMsec rmsec){
	cmdset.limit.setHeadTail(rmsec);
}

//---------------------------------------------------------------------
// -F�n�I�v�V�����ɂ��t���[���ʒu����
// �o�́F
//    cmdset.limit.setFrameRange()
//---------------------------------------------------------------------
void JlsScriptLimit::limitWindow(JlsCmdSet& cmdset){
	//--- �t���[�������l��ݒ� ---
	Msec msec_opt_left  = cmdset.arg.getOpt(OptType::MsecFrameL);
	Msec msec_opt_right = cmdset.arg.getOpt(OptType::MsecFrameR);
	Msec msec_limit_left  = msec_opt_left;
	Msec msec_limit_right = msec_opt_right;
	//--- -FR�I�v�V�����̃t���[�����������A�t���[�������l���擾 ---
	OptType type_frame = (OptType) cmdset.arg.getOpt(OptType::TypeFrame);
	if (type_frame == OptType::FrFR){
		int nrf_1st_rise = pdata->getNrfNextLogo(-1, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		if (nrf_1st_rise >= 0){
			int msec_tmp = pdata->getMsecLogoNrf(nrf_1st_rise);
			if (msec_limit_left != -1){
				msec_limit_left += msec_tmp;
			}
			if (msec_limit_right != -1){
				msec_limit_right += msec_tmp;
			}
		}
	}
	//--- -F�n��`�Ȃ��ꍇ�ŁAHEADTIME/TAILTIME������ꍇ ---
	if ( cmdset.arg.isSetOpt(OptType::TypeFrame) == false ){
		if ( cmdset.arg.tack.fullFrame == false ){		// ��ɑS�̂̏ꍇ�͏���
			RangeMsec rmsec = pdata->recHold.rmsecHeadTail;
			if ( rmsec.st >= 0 ){	// -HEADTIME��`����ꍇ
				msec_limit_left = rmsec.st;
			}
			if ( rmsec.ed >= 0 ){	// -TAILTIME��`����ꍇ
				msec_limit_right = rmsec.ed;
			}
		}
	}
	//--- ���Ԓl�������̎擾 ---
	bool flag_midext = ( (cmdset.arg.getOpt(OptType::TypeFrameSub) & 0x1) != 0)? true : false;
	// -Fhead,-Ftail,-Fmid�Ńt���[���w�莞�̃t���[���v�Z
	if (type_frame == OptType::FrFhead ||
		type_frame == OptType::FrFtail ||
		type_frame == OptType::FrFmid){
		//--- head/tail�擾 ---
		RangeMsec wmsec_headtail = cmdset.limit.getHeadTail();
		Msec msec_head = wmsec_headtail.st;
		Msec msec_tail = wmsec_headtail.ed;
		//--- ���Ԓn�_�̎擾 ---
		// �ŏ��̃��S�J�n����Ō�̃��S�I���̒��Ԓn�_���擾
		Nrf nrf_1st_rise = pdata->getNrfNextLogo(-1, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
		Nrf nrf_end_fall = pdata->getNrfPrevLogo(pdata->sizeDataLogo()*2, LOGO_EDGE_FALL, LOGO_SELECT_VALID);
		//--- �J�n�n�_���� ---
		Msec msec_window_start = 0;
		Msec msec_window_midst = 0;
		if (nrf_1st_rise >= 0)  msec_window_midst = pdata->getMsecLogoNrf(nrf_1st_rise);
		if (msec_window_midst < msec_head)  msec_window_midst = msec_head;
		if (msec_window_start < msec_head)  msec_window_start = msec_head;
		//--- �I���n�_���� ---
		Msec msec_window_mided = pdata->getMsecTotalMax();
		Msec msec_window_end   = pdata->getMsecTotalMax();
		if (nrf_end_fall >= 0) msec_window_mided = pdata->getMsecLogoNrf(nrf_end_fall);
		if (msec_window_mided > msec_tail) msec_window_mided = msec_tail;
		if (msec_window_end > msec_tail) msec_window_end = msec_tail;
		//--- ���Ԓn�_���� ---
		Msec msec_window_md = (msec_window_midst +msec_window_mided) / 2;
		//--- �t���[�������͈͂�ݒ� ---
		if (type_frame == OptType::FrFhead){
			msec_limit_left  = msec_window_start + msec_opt_left;
			msec_limit_right = msec_window_start + msec_opt_right;
			if ( msec_opt_left  == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_right == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_right = min(msec_limit_right, msec_window_md);
			}
		}
		else if (type_frame == OptType::FrFtail){
			msec_limit_left  = msec_window_end - msec_opt_right;
			msec_limit_right = msec_window_end - msec_opt_left;
			if ( msec_opt_right == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_left  == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_left = max(msec_limit_left, msec_window_md);
			}
		}
		else if (type_frame == OptType::FrFmid){
			msec_limit_left  = msec_window_start + msec_opt_left;
			msec_limit_right = msec_window_end   - msec_opt_right;
			if ( msec_opt_left  == -1 ){
				msec_limit_left  = msec_window_start;
			}
			if ( msec_opt_right == -1 ){
				msec_limit_right = msec_window_end;
			}
			if ( !flag_midext ){
				msec_limit_left  = min(msec_limit_left,  msec_window_md);
				msec_limit_right = max(msec_limit_right, msec_window_md);
			}
		}
	}
	//--- ���ʊi�[ ---
	RangeMsec rmsecLimit = {msec_limit_left, msec_limit_right};
	cmdset.limit.setFrameRange(rmsecLimit);
}


//---------------------------------------------------------------------
// -TgtLimit�I�v�V�����őΏۈʒu�����肷��ꍇ�̈ʒu���X�g�쐬
// �o�́F
//    cmdset.limit.addTargetList
//---------------------------------------------------------------------
void JlsScriptLimit::limitListForTarget(JlsCmdSet& cmdset){
	//--- �I�v�V�����Ȃ���Ή������Ȃ� ---
	if (cmdset.arg.isSetOpt(OptType::MsecTgtLimL) == false) return;

	string strList = cmdset.arg.getStrOpt(OptType::StrValListR);	// $LISTHOLD
	if ( strList.empty() == false ){
		cmdset.limit.clearTargetList();		// ���X�g������
		int pos = 0;
		string dstr;
		while ( (pos = pdata->cnv.getStrWord(dstr, strList, pos)) > 0 ){
			int val;
			if (pdata->cnv.getStrValMsecM1(val, dstr, 0) > 0){
				if (val != -1){
					Msec arg_l = (Msec) cmdset.arg.getOpt(OptType::MsecTgtLimL);
					Msec arg_r = (Msec) cmdset.arg.getOpt(OptType::MsecTgtLimR);
					RangeMsec rmsec = {arg_l, arg_r};
					if (arg_l != -1) rmsec.st += val;
					if (arg_r != -1) rmsec.ed += val;
					if (arg_l != -1 || arg_r != -1){
						cmdset.limit.addTargetList(rmsec);
					}
				}
			}
			while (strList[pos] == ',') pos++;
		}
	}
}


//=====================================================================
// ���S�ʒu��񃊃X�g���擾
//=====================================================================

//---------------------------------------------------------------------
// -N -NR -LG�I�v�V�����ɑΉ�����L�����S���X�g���쐬
// �o�́F
//   �Ԃ�l�F ���X�g��
//    cmdset.limit.addLogoList()
//---------------------------------------------------------------------
int JlsScriptLimit::limitLogoList(JlsCmdSet& cmdset){
	//--- ���S��ޑI�� ---
	LogoSelectType typeLogoSel = LOGO_SELECT_ALL;
	bool flagLimitRange = false;
	switch( (OptType)cmdset.arg.getOpt(OptType::TypeNumLogo) ){
		case OptType::LgN:						// -N�I�v�V����
			typeLogoSel = LOGO_SELECT_ALL;
			flagLimitRange = false;
			break;
		case OptType::LgNR:						// -NR�I�v�V����
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = false;
			break;
		case OptType::LgNlogo:					// -Nlogo�I�v�V����
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = true;
			break;
		case OptType::LgNauto:					// -Nauto�I�v�V����
			typeLogoSel = LOGO_SELECT_VALID;
			flagLimitRange = true;
			break;
		default:
			break;
	}

	//--- ���S�ʒu���X�g���擾 ---
	vector<WideMsec> listWmsecLogo;
	int locStart;
	int locEnd;
	bool validList = false;
	{
		//--- ���X�g�擾 ---
		ScrLogoInfoCmdRecord infoCmd = {};
		infoCmd.typeLogo = typeLogoSel;
		getLogoInfoList(listWmsecLogo, cmdset, infoCmd);

		//--- ���X�g�̗L���͈͂����� ---
		RangeMsec rmsecHeadTail = {-1, -1};
		if ( flagLimitRange ){
			rmsecHeadTail = cmdset.limit.getHeadTail();
		}
		validList = limitLogoListRange(locStart, locEnd, listWmsecLogo, rmsecHeadTail);
		//--- �J�n�ƏI����K���Z�b�g�ɂ���ꍇ ---
		if ( cmdset.arg.getOpt(OptType::FlagPair) > 0 ){
			if ( locStart > 0 && (locStart % 2 != 0) ){
				locStart -= 1;
			}
			if ( locEnd > 0 && (locEnd % 2 == 0) ){
				locEnd += 1;
			}
		}
	}

	//--- -N�n�I�v�V�����̏����ɍ������S�ʒu������E�i�[ ---
	if ( validList ){
		//--- �ݒ��� ---
		LogoEdgeType edgeSel = cmdset.arg.selectEdge;			// �R�}���h��S/E/B�I��
		int maxRise = (locEnd / 2) - ((locStart+1) / 2) + 1;	// rise�i�����j�̃��X�g��
		int maxFall = ((locEnd + 1) / 2) - (locStart / 2);		// fall�i��j�̃��X�g��
		int curRise = 0;
		int curFall = 0;
		//--- ���S�ԍ������ԂɊm�F ---
		for(int i = locStart; i <= locEnd; i++){
			bool flagLocRise = ( i % 2 == 0 )? true : false;
			if ( flagLocRise && isLogoEdgeRise(edgeSel) ){		// rise�G�b�W�m�F
				curRise ++;
				bool result = limitLogoListSub(cmdset.arg, curRise, maxRise);
				if ( result ){
					cmdset.limit.addLogoList(listWmsecLogo[i].just, LOGO_EDGE_RISE);
				}
			}
			bool flagLocFall = ( i % 2 == 1 )? true : false;
			if ( flagLocFall && isLogoEdgeFall(edgeSel) ){		// fall�G�b�W�m�F
				curFall ++;
				bool result = limitLogoListSub(cmdset.arg, curFall, maxFall);
				if ( result ){
					cmdset.limit.addLogoList(listWmsecLogo[i].just, LOGO_EDGE_FALL);
				}
			}
		}
	}
	return cmdset.limit.sizeLogoList();
}
// ���݃��S�ԍ����I�v�V�����w�胍�S�ԍ����`�F�b�N
bool JlsScriptLimit::limitLogoListSub(JlsCmdArg& cmdarg, int curNum, int maxNum){
	bool result = false;
	int numlist = (int) cmdarg.sizeLgOpt();
	if (numlist == 0){				// �w��Ȃ���Ώ������f�͑S���L������
		result = true;
	}
	for(int m=0; m<numlist; m++){
		string strVal = cmdarg.getLgOpt(m);
		int rloc = (int)strVal.find("..");
		if ( rloc == (int)string::npos ){		// �ʏ�̐��l
			int val = stoi(strVal);
			if ( (val == 0) || (val == curNum) || (maxNum + val + 1 == curNum) ){
				result = true;
			}
		}else{								// �͈͎w��
			string strSt = strVal.substr(0, rloc);
			string strEd = strVal.substr(rloc+2);
			int valSt = stoi(strSt);
			int valEd = stoi(strEd);
			if ( valSt < 0 ){
				valSt = maxNum + valSt + 1;
			}
			if ( valEd < 0 ){
				valEd = maxNum + valEd + 1;
			}
			if ( (valSt <= curNum) && (curNum <= valEd) ){
				result = true;
			}
		}
	}
	return result;
}
// ���X�g�̗L���͈͂�����
bool JlsScriptLimit::limitLogoListRange(int& st, int& ed, vector<WideMsec>& listWmsec, RangeMsec rmsec){
	st = -1;
	ed = -1;
	if ( listWmsec.empty() ){
		return false;
	}
	Msec msecSpc = pdata->msecValSpc;
	bool st1st = true;
	for(int i=0; i < (int)listWmsec.size(); i++){
		Msec msecSel = listWmsec[i].just;
		if ( i % 2 == 0 ){		// ���S�J�n��
			msecSel += msecSpc;
		}else{					// ���S�I����
			msecSel -= msecSpc;
		}
		if ( msecSel >= rmsec.st || rmsec.st < 0 ){
			if ( st1st ){
				st = i;
				st1st = false;
			}
		}
		if ( msecSel <= rmsec.ed || rmsec.ed < 0 ){
			ed = i;
		}
	}
	if ( st > ed || st < 0 || ed < 0){		// �͈͑��݂��Ȃ��ꍇ
		st = -1;
		ed = -1;
		return false;
	}
	return true;
}


//=====================================================================
// �w�胍�S�̐����K�p
//=====================================================================

//---------------------------------------------------------------------
// �Ώۃ��S�ɂ��Đ���������������đΏۈʒu�擾
//---------------------------------------------------------------------
bool JlsScriptLimit::selectTargetByLogo(JlsCmdSet& cmdset, int nlist){
	//--- ���S�����𖞂�������S��I�� ---
	bool exeflag = limitTargetLogo(cmdset, nlist);
	//--- �����Ώ۔͈͂�ݒ�i����S�ʒu���x�[�X�j ---
	if (exeflag){
		exeflag = limitTargetRangeByLogo(cmdset);
	}
	//--- �^�[�Q�b�g�Ɉ�ԋ߂��V�[���`�F���W�ʒu���擾 ---
	if (exeflag){
		getTargetPoint(cmdset);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// �Ώ۔͈͂�����A����������������đΏۈʒu�擾
//---------------------------------------------------------------------
void JlsScriptLimit::selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec, bool force){
	//--- �����Ώ۔͈͂𒼐ڐ��l�Őݒ� ---
	limitTargetRangeByImm(cmdset, wmsec, force);
	//--- ��ԋ߂��V�[���`�F���W�ʒu���擾 ---
	getTargetPoint(cmdset);
}


//---------------------------------------------------------------------
// ����S��I��
// ���́F
//    nlist: �L�����S���X�g����I������ԍ�
// �o�́F
//   �Ԃ�l�F ���񖞂������S��񔻒�ifalse=���񖞂����Ȃ� true=���񖞂������S���擾�j
//    cmdset.limit.setLogoBaseNrf()
//    cmdset.limit.setLogoBaseNsc()
//---------------------------------------------------------------------
bool JlsScriptLimit::limitTargetLogo(JlsCmdSet& cmdset, int nlist){
	bool exeflag = limitTargetLogoGet(cmdset, nlist);
	if (exeflag){
		exeflag = limitTargetLogoCheck(cmdset);
	}
	return exeflag;
}

// ����S�ʒu���擾
bool JlsScriptLimit::limitTargetLogoGet(JlsCmdSet& cmdset, int nlist){
	//--- �R�}���h�ݒ���擾 ---
	Msec msecTarget  = cmdset.limit.getLogoListMsec(nlist);
	LogoEdgeType edgeSel = cmdset.limit.getLogoListEdge(nlist);
	//--- ���X�g���Ȃ���ΏI�� ---
	if ( msecTarget < 0 ) return false;
	//--- ��ʒu���擾���Đݒ�i�擾�ɂ͖������܂߂��S���S�Ō����j ---
	ScrLogoInfoCmdRecord infoCmd = {};
	infoCmd.typeLogo = LOGO_SELECT_ALL;	// �S���S
	infoCmd.typeSetBase = ScrLogoSetBase::BaseLoc;		// ��ʒu��ݒ肷�铮��
	infoCmd.edgeSel = edgeSel;			// ��ʒu�̃G�b�W�i�����^������j�I��
	infoCmd.msecSel = msecTarget;		// ��ʒu�̃~���b���i�Ή����郍�S�ԍ���T���j
	vector<WideMsec> listTmp;			// �s�g�p���ݒ�
	bool det = getLogoInfoList(listTmp, cmdset, infoCmd);	// ��ʒu���擾���Đݒ�
	//--- �L�����S�݂̂Ń��S���X�g���쐬���� ---
	if ( det ){
		infoCmd.typeLogo = LOGO_SELECT_VALID;	// �L�����S
		infoCmd.typeSetBase = ScrLogoSetBase::ValidList;	// ���S���X�g�̐ݒ�
		getLogoInfoList(listTmp, cmdset, infoCmd);	// �L�����S���X�g�̐ݒ�
	}
	return det;
}

// ����S�ʒu�ɑΉ���������ݒ���m�F
bool JlsScriptLimit::limitTargetLogoCheck(JlsCmdSet& cmdset){
	//--- �R�}���h�ݒ���擾 ---
	Nrf nrf_base = cmdset.limit.getLogoBaseNrf();
	Nsc nsc_base = cmdset.limit.getLogoBaseNsc();
	LogoEdgeType edge_base = cmdset.limit.getLogoBaseEdge();
	bool exeflag = false;
	//--- ���S�ʒu�𒼐ڐݒ肷��R�}���h�ɕK�v�ȃ`�F�b�N ---
	if (nrf_base >= 0){
		exeflag = true;
		//--- �m�茟�o�ς݃��S���m�F ---
		Msec msec_tmp;
		LogoResultType outtype_rf;
		pdata->getResultLogoAtNrf(msec_tmp, outtype_rf, nrf_base);
		//--- �m�胍�S�ʒu�����o����R�}���h�� ---
		bool igncomp = cmdset.arg.tack.ignoreComp;
		if (outtype_rf == LOGO_RESULT_NONE || (outtype_rf == LOGO_RESULT_DECIDE && igncomp)){
		}
		else{
			exeflag = false;
		}
		//--- select�p�m���⑶�ݎ��͏��� ---
		if (cmdset.arg.cmdsel == CmdType::Select &&
			cmdset.arg.getOpt(OptType::FlagReset) == 0 &&
			pdata->getPriorLogo(nrf_base) >= LOGO_PRIOR_DECIDE){
			exeflag = false;
		}
	}
	else if (nsc_base >= 0){
		exeflag = true;
	}
	//--- �����ɍ��������� ---
	if (exeflag){
		//--- �t���[���͈̓`�F�b�N ---
		if ( cmdset.arg.isSetOpt(OptType::TypeFrameSub) &&	// �t���[���͈̓I�v�V��������
			(cmdset.arg.getOpt(OptType::TypeFrameSub) & 0x2) == 0 ){	// -FT�n�ł͂Ȃ�
			WideMsec wmsecLg;
			bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
			cmdset.limit.getLogoWmsecBase(wmsecLg, 0, flagWide);	// �O��͓������S(0)
			RangeMsec rmsecFrame = cmdset.limit.getFrameRange();	// �t���[���͈�
			//--- ���S���͈͓����m�F ---
			if ((rmsecFrame.st > wmsecLg.just && rmsecFrame.st >= 0) ||
				(rmsecFrame.ed < wmsecLg.just && rmsecFrame.ed >= 0)){
				exeflag = false;
			}
		}
		//--- �I�v�V�����Ɣ�r(-LenP, -LenN) ---
		if (exeflag){
			bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
			WideMsec wmsecLg;
			cmdset.limit.getLogoWmsecBase(wmsecLg, 1, flagWide);	// �O��͗א�(1)
			RangeMsec lenP;
			RangeMsec lenN;
			lenP.st = cmdset.arg.getOpt(OptType::MsecLenPMin);
			lenP.ed = cmdset.arg.getOpt(OptType::MsecLenPMax);
			lenN.st = cmdset.arg.getOpt(OptType::MsecLenNMin);
			lenN.ed = cmdset.arg.getOpt(OptType::MsecLenNMax);
			exeflag = limitTargetLogoCheckLength(wmsecLg, lenP, lenN);
		}
		//--- �I�v�V�����Ɣ�r(-LenPE, -LenNE) ---
		if (exeflag){
			bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
			WideMsec wmsecLgE;
			cmdset.limit.getLogoWmsecBase(wmsecLgE, 2, flagWide);	// �O��͓��G�b�W�א�(2)
			RangeMsec lenPE;
			RangeMsec lenNE;
			lenPE.st = cmdset.arg.getOpt(OptType::MsecLenPEMin);
			lenPE.ed = cmdset.arg.getOpt(OptType::MsecLenPEMax);
			lenNE.st = cmdset.arg.getOpt(OptType::MsecLenNEMin);
			lenNE.ed = cmdset.arg.getOpt(OptType::MsecLenNEMax);
			exeflag = limitTargetLogoCheckLength(wmsecLgE, lenPE, lenNE);
		}
	}
	//--- ���S�ʒu����-SC�n�I�v�V����������ꍇ�̊m�F ---
	if (cmdset.arg.tack.floatBase == false && exeflag){
		WideMsec wmsecLg;
		bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
		cmdset.limit.getLogoWmsecBase(wmsecLg, 0, flagWide);	// �O��͓������S(0)
		bool chk_base = true;
		bool chk_rel  = false;
		exeflag = checkOptScpFromMsec(cmdset.arg, wmsecLg.just, edge_base, chk_base, chk_rel);
	}
	return exeflag;
}
//--- �O�ネ�S�Ԃ̒����ɂ�鐧�� ---
bool JlsScriptLimit::limitTargetLogoCheckLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN){
	//--- �O�ネ�S�܂ł̒��� ---
	Msec msecDifPrev = wmsecLg.just - wmsecLg.early;
	Msec msecDifNext = wmsecLg.late - wmsecLg.just;
	//--- �[�����̏��� ---
	if ( wmsecLg.early < 0 && wmsecLg.just >= 0 ){
		msecDifPrev = wmsecLg.just;
	}
	if ( wmsecLg.late < 0 && wmsecLg.just >= 0 ){
		msecDifNext = pdata->getMsecTotalMax() - wmsecLg.just;
	}
	//--- -LenP/-LenPE ��r ---
	bool exeflag = true;
	if ( lenP.st >= 0 ){
		if ( msecDifPrev < lenP.st || msecDifPrev < 0 ){
			exeflag = false;
		}
	}
	if ( lenP.ed >= 0 ){
		if ( msecDifPrev > lenP.ed || msecDifPrev < 0 ){
			exeflag = false;
		}
	}
	//--- -LenN/-LenNE ��r ---
	if ( lenN.st >= 0 ){
		if ( msecDifNext < lenN.st || msecDifNext < 0 ){
			exeflag = false;
		}
	}
	if ( lenN.ed >= 0 ){
		if ( msecDifNext > lenN.ed || msecDifNext < 0 ){
			exeflag = false;
		}
	}
	return exeflag;
}
//---------------------------------------------------------------------
// �����Ώ۔͈͂�ݒ�i����S�ʒu���x�[�X�j
// �o�́F
//   �Ԃ�l�F���񖞂����͈͊m�F�i0:�Y���Ȃ�  1:�Ώ۔͈͎擾�j
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
bool JlsScriptLimit::limitTargetRangeByLogo(JlsCmdSet& cmdset){
	bool exeflag = true;
	//--- ��Ƃ��郍�S�f�[�^�̈ʒu�͈͂�ǂݍ��� ---
	WideMsec wmsec_lg_org;
	{
		Nrf nrf_base = cmdset.limit.getLogoBaseNrf();
		if (nrf_base >= 0){			// ���ۂ̃��S��̏ꍇ
			int numSft = 0;
			if (cmdset.arg.getOpt(OptType::FlagFromLast) > 0){	// �P�O�̃��S����鎞�͋t�G�b�W
				numSft = -1;
			}
			bool flagWide = false;
			if ( cmdset.arg.getOpt(OptType::FlagWide) > 0 ){	// �\���͈͂Ō���
				flagWide = true;
			}
			cmdset.limit.getLogoWmsecBaseShift(wmsec_lg_org, 0, flagWide, numSft);
			if ( wmsec_lg_org.just < 0 ){
				wmsec_lg_org = {};
			}
		}
		else{				// �����\�������S��������ꍇ
			bool flagWide = true;		// �����^������̈Ⴂ���z��
			cmdset.limit.getLogoWmsecBase(wmsec_lg_org, 0, flagWide);
		}
	}
	//--- �I�v�V�����ɂ��ʒu�I�� ---
	WideMsec wmsec_base;
	{
		if ( cmdset.arg.isSetOpt(OptType::MsecFromAbs) ){
			Msec msec_tmp = cmdset.arg.getOpt(OptType::MsecFromAbs);
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_tmp < 0) exeflag = false;				// ���̒l�̎��̓R�}���h�𖳌��Ƃ���
		}
		else if ( cmdset.arg.isSetOpt(OptType::MsecFromHead) ){
			Msec msec_opt = cmdset.arg.getOpt(OptType::MsecFromHead);
			Msec msec_tmp = cmdset.limit.getHead() + msec_opt;
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_opt < 0) exeflag = false;				// ���̒l�̎��̓R�}���h�𖳌��Ƃ���
		}
		else if ( cmdset.arg.isSetOpt(OptType::MsecFromTail) ){
			Msec msec_opt = cmdset.arg.getOpt(OptType::MsecFromTail);
			Msec msec_tmp = cmdset.limit.getTail() - msec_opt;
			wmsec_base = {msec_tmp, msec_tmp, msec_tmp};
			if (msec_opt < 0) exeflag = false;				// ���̒l�̎��̓R�}���h�𖳌��Ƃ���
		}
		else{
			wmsec_base = wmsec_lg_org;
		}
	}
	//--- �R�}���h�w��͈̔͂��t���[���͈͂ɒǉ� ---
	WideMsec wmsec_find;
	{
		//--- �R�}���h�w��͈̔͂�ǉ� ---
		wmsec_find.just  = wmsec_base.just  + cmdset.arg.wmsecDst.just;	// set point to find
		wmsec_find.early = wmsec_base.early + cmdset.arg.wmsecDst.early;
		wmsec_find.late  = wmsec_base.late  + cmdset.arg.wmsecDst.late;
		//--- shift�R�}���h�ʒu���f ---
		if (cmdset.arg.isSetOpt(OptType::MsecSftC) != 0){
			wmsec_find.just  += cmdset.arg.getOpt(OptType::MsecSftC);
			wmsec_find.early += cmdset.arg.getOpt(OptType::MsecSftL);
			wmsec_find.late  += cmdset.arg.getOpt(OptType::MsecSftR);
		}
		//--- �R�}���h�͈͂�-1�ݒ莞�̕ϊ� ---
		if ( cmdset.arg.wmsecDst.early == -1 ){
			wmsec_find.early = 0;
		}
		if ( cmdset.arg.wmsecDst.late == -1 ){
			wmsec_find.late = pdata->getMsecTotalMax();
		}
		//--- ���S�����Ɍ��肷��Select�R�}���h�p�͈̔� ---
		if (cmdset.arg.cmdsel == CmdType::Select){
			Msec msec_ext_l = wmsec_lg_org.early + cmdset.arg.getOpt(OptType::MsecLogoExtL);
			Msec msec_ext_r = wmsec_lg_org.late  + cmdset.arg.getOpt(OptType::MsecLogoExtR);
			if (wmsec_find.early > msec_ext_r || wmsec_find.late < msec_ext_l){
				exeflag = false;
			}
			else{
				if (wmsec_find.early < msec_ext_l)  wmsec_find.early = msec_ext_l;
				if (wmsec_find.just  < msec_ext_l)  wmsec_find.just  = msec_ext_l;
				if (wmsec_find.late  > msec_ext_r)  wmsec_find.late  = msec_ext_r;
				if (wmsec_find.just  > msec_ext_r)  wmsec_find.just  = msec_ext_r;
			}
		}
		//--- �O��̃��S�ʒu�ȓ��ɔ͈͌��肷��ꍇ ---
		if (cmdset.arg.tack.limitByLogo){
			bool flagWide = true;		// �\������͈͂Ō���
			WideMsec wmsec_lgpn;
			cmdset.limit.getLogoWmsecBase(wmsec_lgpn, 1, flagWide);	// �O��͗א�(1)
			if (wmsec_lgpn.early >= 0){
				if (wmsec_find.early < wmsec_lgpn.early) wmsec_find.early = wmsec_lgpn.early;
				if (wmsec_find.just  < wmsec_lgpn.early) wmsec_find.just  = wmsec_lgpn.early;
			}
			if (wmsec_lgpn.late >= 0){
				if (wmsec_find.late > wmsec_lgpn.late) wmsec_find.late = wmsec_lgpn.late;
				if (wmsec_find.just > wmsec_lgpn.late) wmsec_find.just = wmsec_lgpn.late;
			}
		}
	}
	//--- �t���[���w��͈͓��Ɍ��� ---
	if (exeflag){
		RangeMsec rmsec_window = cmdset.limit.getFrameRange();
		exeflag = pdata->limitWideMsecFromRange(wmsec_find, rmsec_window);
	}
	//--- �͈͂����݂��Ȃ���Ζ����� ---
	if (wmsec_find.early > wmsec_find.late){
		exeflag = false;
	}
	//--- ���ʂ��i�[ ---
	Msec msec_force = -1;
	if ((cmdset.arg.getOpt(OptType::FlagForce) > 0) ||
		(cmdset.arg.getOpt(OptType::FlagNoForce) > 0)){
		msec_force = wmsec_find.just;
	}
	bool from_logo = true;
	cmdset.limit.setTargetRange(wmsec_find, msec_force, from_logo);

	return exeflag;
}

//--- ���ڐ��l�ݒ� ---
void JlsScriptLimit::limitTargetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec, bool force){
	Msec msec_force = -1;
	if ( force ){
		if ( wmsec.just >= 0 && wmsec.just <= pdata->getMsecTotalMax() ){
			msec_force = wmsec.just;
		}
	}
	bool from_logo = false;
	cmdset.limit.setTargetRange(wmsec, msec_force, from_logo);
}



//=====================================================================
// �^�[�Q�b�g�ʒu���擾
//=====================================================================

//---------------------------------------------------------------------
// �^�[�Q�b�g�Ɉ�ԋ߂��V�[���`�F���W�ʒu���擾
// �o�́F
//   cmdset.list.setResultTarget() : �I���V�[���`�F���W�ʒu
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPoint(JlsCmdSet& cmdset){
	//--- �͈͂��擾 ---
	WideMsec wmsec_target = cmdset.limit.getTargetRangeWide();
	LogoEdgeType edge_sel = cmdset.limit.getLogoBaseEdge();

	Nsc nsc_scpos_tag = -1;
	Nsc nsc_scpos_end = -1;
	int flag_noedge = cmdset.arg.getOpt(OptType::FlagNoEdge);

	//--- -SC, -NoSC���I�v�V�����ɑΉ�����V�[���`�F���W�L������ ---
	getTargetPointSetScpEnable(cmdset);

	//--- NextTail�R�}���h�p ---
	bool flag_nexttail = false;
	bool flag_logorise = false;
	if (cmdset.arg.cmdsel == CmdType::NextTail){
		flag_nexttail = true;
		flag_noedge = 0;
		edge_sel = LOGO_EDGE_RISE;
		if (cmdset.arg.selectEdge == LOGO_EDGE_RISE){
			flag_logorise = true;
		}
	}
	//--- ��ԋ߂��V�[���`�F���W�ʒu���擾 ---
	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + flag_noedge;
	int jsize = size_scp - flag_noedge;

	int val_difmin = 0;
	ScpPriorType stat_scpos = SCP_PRIOR_DUPE;
	//--- ���S����̏��擾�p(Next�R�}���h�p�j ---
	Nrf nrf_logo = 0;
	Msec msec_logo = 0;
	bool flag_logo = false;
	//--- ��ԋ߂��ӏ��̒T�� ---
	for(int j=jfrom; j<jsize; j++){
		Msec         msec_now = pdata->getMsecScp(j);
		ScpPriorType stat_now = pdata->getPriorScp(j);
		Msec         msec_now_edge = pdata->getMsecScpEdge(j, edge_sel);
		Msec         val_dif = abs(msec_now_edge - wmsec_target.just);
		//--- �Ώۉӏ��̃I�v�V��������m�F ---
		if ( (cmdset.limit.getScpEnable(j) && cmdset.limit.isTargetListed(msec_now)) || 
			 (j == size_scp-1 && flag_nexttail) ){
			//--- ���S����̏��g�p��(NextTail�R�}���h�p�j ---
			bool flag_now_logo = false;
			if (flag_logorise){
				while (msec_logo + pdata->msecValSpc < msec_now && nrf_logo >= 0){
					nrf_logo = pdata->getNrfNextLogo(nrf_logo, LOGO_EDGE_RISE, LOGO_SELECT_VALID);
					if (nrf_logo >= 0){
						msec_logo = pdata->getMsecLogoNrf(nrf_logo);
					}
				}
				if (abs(msec_logo - msec_now) <= pdata->msecValSpc){
					if ( pdata->isAutoModeUse() == false ||
						 pdata->isScpChapTypeDecideFromNsc(j) ){	// �m���؂莞�̂ݗD��
						flag_now_logo = true;
					}
				}
				if (j == size_scp-1) flag_now_logo = true;			// �ŏI�ʒu�̓��S����
				if (flag_now_logo == false && flag_logo == true){	// ���ƌ��ʒu�̗D���Ԕ��f
					stat_now = SCP_PRIOR_DUPE;
				}
			}
			//--- �ŏ������̈ʒu��T�� ---
			if (val_difmin > val_dif || nsc_scpos_tag < 0){
				if (msec_now >= wmsec_target.early && msec_now <= wmsec_target.late){
					//--- ����Ԃ̊m�F ---
					bool chk_stat = false;
					if (stat_now >= stat_scpos || cmdset.arg.getOpt(OptType::FlagFlat) > 0){
						chk_stat = true;
					}
					else if (cmdset.arg.cmdsel == CmdType::Select){
						chk_stat = true;
					}
					else if (j == size_scp-1){						// �ŏI�ʒu�͊m�舵��
						chk_stat = true;
					}
					if (chk_stat){
						//--- -EndLen�I�v�V�����m�F ---
						bool chk_end = getTargetPointEndResult(nsc_scpos_end, cmdset.arg, msec_now);
						if (chk_end){			// End�ʒu���������Ė������̏ꍇ�̂ݏ���
							val_difmin = val_dif;
							nsc_scpos_tag = j;
							stat_scpos = stat_now;
							flag_logo = flag_now_logo;
						}
					}
				}
			}
		}
	}
	//--- ���o�ł��Ȃ������ꍇ��force�K�p�����force�ʒu����End�n�_�m�F ---
	if (nsc_scpos_tag < 0){
		Msec msec_force   = cmdset.limit.getTargetRangeForce();
		if (msec_force >= 0){
			getTargetPointEndResult(nsc_scpos_end, cmdset.arg, msec_force);
		}
	}
	//--- �ʒu�o�͎��p�̃��S�G�b�W�I�� ---
	getTargetPointOutEdge(cmdset);
	//--- ���ʂ��i�[ ---
	cmdset.limit.setResultTarget(nsc_scpos_tag, nsc_scpos_end);
}

//---------------------------------------------------------------------
// �ʒu���o�͎��̃��S�G�b�W�I��
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPointOutEdge(JlsCmdSet& cmdset){
	LogoEdgeType outEdge = cmdset.limit.getLogoBaseEdge();
	if ( cmdset.arg.getOpt(OptType::FlagEdgeS) ){
		outEdge = LOGO_EDGE_RISE;
	}
	if ( cmdset.arg.getOpt(OptType::FlagEdgeE) ){
		outEdge = LOGO_EDGE_FALL;
	}
	cmdset.limit.setTargetOutEdge(outEdge);
}

//---------------------------------------------------------------------
// -End�n�I�v�V�����ɑΉ�����V�[���`�F���W�ʒu�擾
// ���́F
//   msec_base  : ��ƂȂ�t���[��
// �o�́F
//   �Ԃ�l�F��������K�p���ʁi0=���s  1=�����j
//   nsc_scpos_end : �i��������𖞂��������̂ݍX�V�j�Ή�����V�[���`�F���W�ʒu
//---------------------------------------------------------------------
bool JlsScriptLimit::getTargetPointEndResult(int& nsc_scpos_end, JlsCmdArg& cmdarg, int msec_base){
	//--- -EndLen�I�v�V�����m�F ---
	int nend = -2;
	if ( cmdarg.isSetOpt(OptType::MsecEndlenC) &&
	     cmdarg.getOpt(OptType::MsecEndlenC) != 0){
		nend = getTargetPointEndlen(cmdarg, msec_base);
	}
	else{
		nend = getTargetPointEndArg(cmdarg, msec_base);
	}
	if ( nend >= 0 ){
		nsc_scpos_end = nend;
	}
	if ( nend == -1 ) return false;		// -End�n�I�v�V�������݂����o���s
	return true;
}

//---------------------------------------------------------------------
// -EndLen�I�v�V�����ɑΉ�����V�[���`�F���W�ʒu�擾
// ���́F
//   msec_base  : ��ƂȂ�t���[��
// �Ԃ�l�F
//   -1    : �Y���Ȃ�
//   0�ȏ� : ��v����V�[���`�F���W�ԍ�
//---------------------------------------------------------------------
Nsc JlsScriptLimit::getTargetPointEndlen(JlsCmdArg& cmdarg, int msec_base){
	Msec msec_endlen_c = msec_base + cmdarg.getOpt(OptType::MsecEndlenC);
	Msec msec_endlen_l = msec_base + cmdarg.getOpt(OptType::MsecEndlenL);
	Msec msec_endlen_r = msec_base + cmdarg.getOpt(OptType::MsecEndlenR);

	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + cmdarg.getOpt(OptType::FlagNoEdge);
	int jsize = size_scp - cmdarg.getOpt(OptType::FlagNoEdge);

	Nsc  nsc_scpos_end = -1;
	Msec val_difmin = 0;
	ScpPriorType stat_scpos = SCP_PRIOR_NONE;
	for(int j=jfrom; j<jsize; j++){
		int msec_now = pdata->getMsecScp(j);
		ScpPriorType stat_now = pdata->getPriorScp(j);
		int val_dif = abs(msec_now - msec_endlen_c);
		if (val_difmin > val_dif || nsc_scpos_end < 0){
			if (msec_now >= msec_endlen_l && msec_now <= msec_endlen_r){
				if (stat_now >= stat_scpos || cmdarg.getOpt(OptType::FlagFlat) > 0){
					val_difmin = val_dif;
					nsc_scpos_end = j;
					stat_scpos = stat_now;
				}
			}
		}
	}
	return nsc_scpos_end;
}

//---------------------------------------------------------------------
// End�n�I�v�V��������̈ʒu�擾�i-EndLen�ȊO�j
// ���́F
//   msec_base  : ��ƂȂ�t���[��
// �Ԃ�l�F
//   -2    : �R�}���h�Ȃ�
//   -1    : �w��ɑΉ�����ʒu�͊Y���Ȃ�
//   0�ȏ� : ��v����V�[���`�F���W�ԍ�
//---------------------------------------------------------------------
Nsc JlsScriptLimit::getTargetPointEndArg(JlsCmdArg& cmdarg, int msec_base){
	int  errnum = -2;
	Msec msec_target = 0;
	Msec msec_th = pdata->msecValExact;
	string cstr;

	//--- �I�v�V�������@�擾 ---
	if ( cmdarg.isSetOpt(OptType::MsecEndAbs) ){			// -EndAbs
		msec_target = cmdarg.getOpt(OptType::MsecEndAbs);
		if (msec_target >= 0){
			errnum = 0;
		}
		else{
			errnum = -1;
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndHead) ){	// -EndHead
		Msec val = pdata->recHold.rmsecHeadTail.st;
		errnum = 0;
		if (val != -1){
			msec_target = val;
		}
		else{
			msec_target = 0;
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndTail) ){	// -EndTail
		Msec val = pdata->recHold.rmsecHeadTail.ed;
		errnum = 0;
		if (val != -1){
			msec_target = val;
		}
		else{
			msec_target = pdata->getMsecTotalMax();
		}
	}
	else if ( cmdarg.isSetOpt(OptType::FlagEndHold) ){	// -EndHold
		string strVal = cmdarg.getStrOpt(OptType::StrValPosR);	// $POSHOLD
		int pos = pdata->cnv.getStrValMsecM1(msec_target, strVal, 0);
		if ( pos >= 0 && msec_target >= 0 ){
			errnum = 0;
		}
		else{
			errnum = -1;
		}
	}
	//--- �I�v�V�����ݒ� ---
	Nsc nsc_scpos_end;
	if (errnum == 0){
		nsc_scpos_end = pdata->getNscFromMsecFull(
							msec_target, msec_th, SCP_CHAP_NONE, SCP_END_EDGEIN);
	}
	else{
		nsc_scpos_end = errnum;
	}
	return nsc_scpos_end;
}

//---------------------------------------------------------------------
// -SC, -NoSC���I�v�V�����ɑΉ�����V�[���`�F���W�L������i�S�����V�[���`�F���W�ʒu�Ŋm�F�j
// �o�́F
//   cmdset.limit.setScpEnable()
//---------------------------------------------------------------------
void JlsScriptLimit::getTargetPointSetScpEnable(JlsCmdSet& cmdset){
	//--- �X�V���f ---
	int size_scp = pdata->sizeDataScp();
	int size_enable = cmdset.limit.sizeScpEnable();
	if (size_scp == size_enable) return;	// �����Ȃ���ΑO�񂩂�ύX�Ȃ�

	//--- ���΃R�}���h�͏�Ƀ`�F�b�N�B�ʏ�R�}���h�͐ݒ�ɂ��`�F�b�N ---
	bool chk_base = false;
	bool chk_rel  = true;
	if (cmdset.arg.tack.floatBase){
		chk_base = true;
	}
	//--- �S�����V�[���`�F���W�ʒu�Ń`�F�b�N ---
	vector <bool> list_enable;
	for(int m=0; m<size_scp; m++){
		int msec_base = pdata->getMsecScp(m);
		bool result = checkOptScpFromMsec(cmdset.arg, msec_base, LOGO_EDGE_RISE, chk_base, chk_rel);
		list_enable.push_back(result);
	}
	cmdset.limit.setScpEnable(list_enable);
}



//=====================================================================
// ���������Ŏg�p
//=====================================================================

//---------------------------------------------------------------------
// ���S�̗����^������ʒu���̃��X�g���擾
// ���́F
//   infoCmd  : �擾����f�[�^�̐ݒ�
// �o�́F
//   �Ԃ�l�F�擾�������f�[�^���݁ifalse=���݂��Ȃ��Ature=���݁j
//   listWmsecLogo : �e���S�̈ʒu���X�g
//---------------------------------------------------------------------
bool JlsScriptLimit::getLogoInfoList(vector<WideMsec>& listWmsecLogo, JlsCmdSet& cmdset, ScrLogoInfoCmdRecord infoCmd){
	int loc;
	bool det = false;
	if ( cmdset.arg.tack.virtualLogo == false ){
		vector<Nrf>  listNrfLogo;
		//--- ���ۂ̃��S�ʒu�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu���X�g�擾 ---
		getLogoInfoListLg(listWmsecLogo, listNrfLogo, infoCmd.typeLogo);
		//--- ���X�g�i�[�ʒu�ɑΉ����郍�S�ԍ����擾 ---
		loc = getLogoInfoListFind(listWmsecLogo, infoCmd.msecSel, infoCmd.edgeSel);
		//--- �^�[�Q�b�g��Base�ʒu�Ƃ��Đݒ肷��ꍇ�̏��� ---
		if ( infoCmd.typeSetBase == ScrLogoSetBase::BaseLoc ){
			if ( loc >= 0 && loc < (int)listWmsecLogo.size() ){
				if ( listWmsecLogo[loc].just == infoCmd.msecSel ){
					det = true;
					cmdset.limit.setLogoBaseNrf(listNrfLogo[loc], infoCmd.edgeSel);
				}
			}
		}else{
			det = ( listWmsecLogo.empty() )? false : true;
		}
	}else{
		vector<Nsc>  listNscLogo;
		//--- �����\���ω��_�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu���X�g�擾 ---
		bool flagOut = ( cmdset.arg.getOpt(OptType::FlagFinal) != 0 )? true : false;	// -final�I�v�V����
		getLogoInfoListElg(listWmsecLogo, listNscLogo, flagOut);
		//--- ���X�g�i�[�ʒu�ɑΉ����郍�S�ԍ����擾 ---
		loc = getLogoInfoListFind(listWmsecLogo, infoCmd.msecSel, infoCmd.edgeSel);
		//--- �^�[�Q�b�g��Base�ʒu�Ƃ��Đݒ肷��ꍇ�̏��� ---
		if ( infoCmd.typeSetBase == ScrLogoSetBase::BaseLoc ){
			if ( loc >= 0 && loc < (int)listWmsecLogo.size() ){
				if ( listWmsecLogo[loc].just == infoCmd.msecSel ){
					det = true;
					cmdset.limit.setLogoBaseNsc(listNscLogo[loc], infoCmd.edgeSel);
				}
			}
		}else{
			det = ( listWmsecLogo.empty() )? false : true;
		}
	}
	//--- ���S���X�g��ۊ� ---
	if ( infoCmd.typeSetBase == ScrLogoSetBase::ValidList ){
		cmdset.limit.setLogoWmsecList(listWmsecLogo, loc);
	}
	return det;
}

//---------------------------------------------------------------------
// ���݈ʒu�����S���X�g�̉��Ԗڂ��擾
// �Ԃ�l�F
//   -2    : �f�[�^�Ȃ�
//   -1    : �ŏ��̗����オ�����O
//   0�ȏ� : ���S���X�g�ԍ��i�����F�����A��F������j
//---------------------------------------------------------------------
int JlsScriptLimit::getLogoInfoListFind(vector<WideMsec>& listWmsec, Msec msecLogo, LogoEdgeType edge){
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
	return loc;
}
//---------------------------------------------------------------------
// ���ۂ̃��S�ʒu�̃��X�g���擾
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoInfoListLg(vector<WideMsec>& listWmsec, vector<Nrf>& listNrf, LogoSelectType type){
	listWmsec.clear();		// ���X�g������
	listNrf.clear();		// ���X�g������

	//--- ���ۂ̃��S�ʒu�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu ---
	NrfCurrent logopt = {};
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getNrfptNext(logopt, type);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			pdata->getWideMsecLogoNrf(wmsecRise, logopt.nrfRise);
			pdata->getWideMsecLogoNrf(wmsecFall, logopt.nrfFall);
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
			listNrf.push_back(logopt.nrfRise);
			listNrf.push_back(logopt.nrfFall);
		}
	}
}
//---------------------------------------------------------------------
// ���S���������\���ʒu�̃��X�g���擾
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoInfoListElg(vector<WideMsec>& listWmsec, vector<Nsc>& listNsc, bool outflag){
	listWmsec.clear();		// ���X�g������
	listNsc.clear();		// ���X�g������

	//--- �����\���ω��_�����S�ԍ��Ƃ���ꍇ�̃��S�ʒu ---
	ElgCurrent elg = {};
	elg.outflag = outflag;
	bool flag_cont = true;
	while( flag_cont ){
		flag_cont = pdata->getElgptNext(elg);
		if ( flag_cont ){
			WideMsec wmsecRise;
			WideMsec wmsecFall;
			Msec msecRiseT  = pdata->getMsecScp(elg.nscRise);
			Msec msecRiseBk = pdata->getMsecScpBk(elg.nscRise);
			Msec msecFallT  = pdata->getMsecScp(elg.nscFall);
			Msec msecFallBk = pdata->getMsecScpBk(elg.nscFall);
			wmsecRise.just  = msecRiseT;	// ���S�͗�����ʒu�Ō���
			wmsecRise.early = msecRiseBk;	// ������G�b�W�͏�����O�̉\��
			wmsecRise.late  = msecRiseT;
			wmsecFall.just  = msecFallT;	// ���S�͗������ʒu�Ō���
			wmsecFall.early = msecFallBk;
			wmsecFall.late  = msecFallT;
			listWmsec.push_back(wmsecRise);
			listWmsec.push_back(wmsecFall);
			listNsc.push_back(elg.nscRise);
			listNsc.push_back(elg.nscFall);
		}
	}
}

//---------------------------------------------------------------------
// -SC, -NoSC�n�I�v�V�����ɑΉ�����V�[���`�F���W�L������
// ���́F
//   msec_base  : ��ƂȂ�t���[��
//   edge      : 0:start edge  1:end edge
//   chk_base  : �ʏ�R�}���h�̔�����{(false=���Ȃ� true=����)
//   chk_rel   : ���Έʒu�R�}���h�̔�����{(false=���Ȃ� true=����)
// �Ԃ�l�F
//   false : ��v����
//   true  : ��v�m�F
//---------------------------------------------------------------------
bool JlsScriptLimit::checkOptScpFromMsec(JlsCmdArg& cmdarg, int msec_base, LogoEdgeType edge, bool chk_base, bool chk_rel){
	int size_scp = pdata->sizeDataScp();
	int jfrom = 0 + cmdarg.getOpt(OptType::FlagNoEdge);
	int jsize = size_scp - cmdarg.getOpt(OptType::FlagNoEdge);
	bool result = true;
	int numlist = cmdarg.sizeScOpt();
	for(int k=0; k<numlist; k++){
		//--- ���Έʒu�R�}���h���菈�� ---
		OptType sctype = cmdarg.getScOptType(k);
		if (cmdarg.isScOptRelative(k)){			// ���Έʒu�����R�}���h
			if (chk_rel == false){				// ���Έʒu�`�F�b�N���Ȃ����͒��~
				sctype = OptType::ScNone;
			}
		}
		else{									// �ʏ�ݒ�
			if (chk_base == false){				// �ʏ�ݒ�̃`�F�b�N�łȂ����͒��~
				sctype = OptType::ScNone;
			}
		}
		//--- �Ώۂł���΃`�F�b�N ---
		if (sctype != OptType::ScNone){
			DataScpRecord dt;
			Nsc nsc_scpos_sc   = -1;
			Nsc nsc_smute_all  = -1;
			Nsc nsc_smute_part = -1;
			Nsc nsc_chap_auto  = -1;
			Msec lens_min = cmdarg.getScOptMin(k);
			Msec lens_max = cmdarg.getScOptMax(k);
			for(int j=jfrom; j<jsize; j++){
				pdata->getRecordScp(dt, j);
				int msec_now;
				if ( isLogoEdgeRise(edge) ){
					msec_now = dt.msec;
				}
				else{
					msec_now = dt.msbk;
				}
				if ((msec_now - msec_base >= lens_min || lens_min == -1) &&
					(msec_now - msec_base <= lens_max || lens_min == -1)){
					nsc_scpos_sc = j;
					// for -AC option
					ScpChapType chap_now = dt.chap;
					if (chap_now >= SCP_CHAP_DECIDE || chap_now == SCP_CHAP_CDET){
						nsc_chap_auto = j;
					}
				}
				// �����n
				int msec_smute_s = dt.msmute_s;
				int msec_smute_e = dt.msmute_e;
				if (msec_smute_s < 0 || msec_smute_e < 0){
					msec_smute_s = msec_now;
					msec_smute_e = msec_now;
				}
				// for -SMA option �i������񂪂���ꍇ�̂݌��o�j
				if ((msec_smute_s - msec_base <= lens_min) &&
					(msec_smute_e - msec_base >= lens_max)){
					nsc_smute_all = j;
				}
				//for -SM option
				if ((msec_smute_s - msec_base <= lens_max || lens_max == -1) &&
					(msec_smute_e - msec_base >= lens_min || lens_min == -1)){
					nsc_smute_part = j;
				}
			}
			if (nsc_scpos_sc < 0 && sctype == OptType::ScSC){	// -SC
				result = false;
			}
			else if (nsc_scpos_sc >= 0 && sctype == OptType::ScNoSC){	// -NoSC
				result = false;
			}
			else if (nsc_smute_part < 0 && sctype == OptType::ScSM){	// -SM
				result = false;
			}
			else if (nsc_smute_part >= 0 && sctype == OptType::ScNoSM){	// -NoSM
				result = false;
			}
			else if (nsc_smute_all < 0 && sctype == OptType::ScSMA){	// -SMA
				result = false;
			}
			else if (nsc_smute_all >= 0 && sctype == OptType::ScNoSMA){	// -NoSMA
				result = false;
			}
			else if (nsc_chap_auto < 0 && sctype == OptType::ScAC){	// -AC
				result = false;
			}
			else if (nsc_chap_auto >= 0 && sctype == OptType::ScNoAC){	// -NoAC
				result = false;
			}
		}
		if (result == false){
			break;
		}
	}
	return result;
}

