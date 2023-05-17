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
	var.setPdata(pdata);
}

//=====================================================================
// �R�}���h���ʂ͈̔͌���
//=====================================================================

//---------------------------------------------------------------------
// �R�}���h���ʂ͈̔͌���
//---------------------------------------------------------------------
void JlsScriptLimit::limitCommonRange(JlsCmdSet& cmdset){
	var.initVar(cmdset);				// �N���X���ϐ��������ݒ�
	limitCustomLogo();					// �ݒ�ɍ��킹�����S�쐬
	limitHeadTail();					// �S�͈̔͐ݒ�
	limitWindow();						// -F�n�I�v�V�����ݒ�
	updateCommonRange(cmdset);			// cmdset.limit���X�V
}

//---------------------------------------------------------------------
// �͈͂̍Đݒ�
//---------------------------------------------------------------------
void JlsScriptLimit::resizeRangeHeadTail(JlsCmdSet& cmdset, RangeMsec rmsec){
	//--- �͈͂�ݒ� ---
	limitHeadTailImm(rmsec);			// �S�͈̔͐ݒ�
	limitWindow();						// -F�n�I�v�V�����ƍ��킹���͈͂��ēx����
	updateCommonRange(cmdset);			// cmdset.limit���X�V
}

//--- cmdset�ɋ��ʐݒ���e���X�V ---
void JlsScriptLimit::updateCommonRange(JlsCmdSet& cmdset){
	cmdset.limit.setHeadTail( var.getHeadTail() );
	cmdset.limit.setFrameRange( var.getFrameRange() );
}

//---------------------------------------------------------------------
// �ݒ�ɍ��킹�����S���쐬
//---------------------------------------------------------------------
void JlsScriptLimit::limitCustomLogo(){
	//--- �ݒ�l ---
	LogoCustomType custom = {};
	custom.extLogo = var.opt.tack.virtualLogo;
	custom.selectAll = LOGO_SELECT_VALID;		// �o�̓��X�g�͒ʏ�͗L���̂�
	custom.final   = var.opt.getOptFlag(OptType::FlagFinal);
	custom.border  = false;
	if ( var.opt.tack.ignoreAbort ){		// ���SAbort��Ԃł����s����R�}���h�̏ꍇ
		custom.selectAll = LOGO_SELECT_ALL;			// �S���S�����X�g�ɏo��
	}
	//--- ���S���쐬�E�i�[ ---
	pdata->makeClogo(custom);
}

//---------------------------------------------------------------------
// HEADTIME/TAILTIME��`�ɂ��t���[���ʒu����
// �o�́F
//    var.setHeadTail()
//---------------------------------------------------------------------
void JlsScriptLimit::limitHeadTail(){
	RangeMsec rmsec;
	rmsec.st = pdata->recHold.rmsecHeadTail.st;
	if (rmsec.st == -1){
		rmsec.st = 0;
	}
	rmsec.ed = pdata->recHold.rmsecHeadTail.ed;
	if (rmsec.ed == -1){
		rmsec.ed = pdata->getMsecTotalMax();
	}
	limitHeadTailImm(rmsec);
}

//--- ���ڐ��l�ݒ� ---
void JlsScriptLimit::limitHeadTailImm(RangeMsec rmsec){
	var.setHeadTail(rmsec);
}

//---------------------------------------------------------------------
// -F�n�I�v�V�����ɂ��t���[���ʒu����
// �o�́F
//    var.setFrameRange()
//---------------------------------------------------------------------
void JlsScriptLimit::limitWindow(){
	//--- �t���[�������l��ݒ� ---
	Msec msec_opt_left  = var.opt.getOpt(OptType::MsecFrameL);
	Msec msec_opt_right = var.opt.getOpt(OptType::MsecFrameR);
	Msec msec_limit_left  = msec_opt_left;
	Msec msec_limit_right = msec_opt_right;
	//--- -FR�I�v�V�����̃t���[�����������A�t���[�������l���擾 ---
	OptType type_frame = (OptType) var.opt.getOpt(OptType::TypeFrame);
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
	if ( var.opt.isSetOpt(OptType::TypeFrame) == false ){
		bool fullFrame = var.opt.tack.fullFrameA;
		if ( pdata->recHold.typeRange == 1 ){		// �ϐ�RANGETYPE=1�ݒ莞
			fullFrame = var.opt.tack.fullFrameB;	// �ŏ����̃^�C�v
		}
		if ( !fullFrame ){		// ��ɑS�̂̏ꍇ�͏���
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
	bool flag_midext = ( (var.opt.getOpt(OptType::TypeFrameSub) & 0x1) != 0)? true : false;
	// -Fhead,-Ftail,-Fmid�Ńt���[���w�莞�̃t���[���v�Z
	if (type_frame == OptType::FrFhead ||
		type_frame == OptType::FrFtail ||
		type_frame == OptType::FrFmid){
		//--- head/tail�擾 ---
		RangeMsec wmsec_headtail = var.getHeadTail();
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
	var.setFrameRange(rmsecLimit);
}


//=====================================================================
// �L���ȃ��S�ʒu���X�g���擾
//=====================================================================

//---------------------------------------------------------------------
// -N�n�I�v�V�����ɑΉ�����L�����S���X�g���쐬
// �o�́F
//   �Ԃ�l�F ���X�g��
//    cmdset.limit.addLogoList*()
//---------------------------------------------------------------------
int JlsScriptLimit::limitLogoList(JlsCmdSet& cmdset){
	//--- �L�����S���X�g�������� ---
	cmdset.limit.clearLogoList();
	//--- -N�n�I�v�V�����Ō��肵���L�����S���X�g�擾 ---
	getLogoListStd(cmdset);
	//--- ���ڈʒu�ݒ�i-from�n�I�v�V�������j������ꍇ�̒ǉ����� ---
	getLogoListDirect(cmdset);
	//--- �L�����S����Ԃ� ---
	return cmdset.limit.sizeLogoList();
}
//--- -N�n�I�v�V�����Ō��肵�����S�ʒu���X�g���擾 ---
void JlsScriptLimit::getLogoListStd(JlsCmdSet& cmdset){
	//--- ���̓��S�擾 ---
	vector<Msec> listMsecLogoIn;		// ���͔ԍ��ɑΉ��������S�i�[�p
	int locStart;	// �͈͓��擪�̃��S���X�g�ԍ�
	int locEnd;		// �͈͓��Ō�̃��S���X�g�ԍ�
	if ( getLogoListStdData(listMsecLogoIn, locStart, locEnd) == false ){
		return;
	}
	//--- �g�p���S�擾 ---
	vector<bool> listUseLogoIn(listMsecLogoIn.size(), false);
	{
		//--- �ݒ��� ---
		LogoEdgeType edgeSel = var.opt.selectEdge;			// �R�}���h��S/E/B�I��
		int maxRise = (locEnd / 2) - ((locStart+1) / 2) + 1;	// rise�i�����j�̃��X�g��
		int maxFall = ((locEnd + 1) / 2) - (locStart / 2);		// fall�i��j�̃��X�g��
		int curRise = 0;
		int curFall = 0;
		//--- ���S�ԍ������ԂɎg�p���S���m�F ---
		for(int i = locStart; i <= locEnd; i++){
			Msec msecNow = listMsecLogoIn[i];
			LogoEdgeType edgeNow;
			if ( i % 2 == 0 ){
				edgeNow = LOGO_EDGE_RISE;
				curRise ++;
			}else{
				edgeNow = LOGO_EDGE_FALL;
				curFall ++;
			}
			if ( pdata->isClogoMsecExist(msecNow, edgeNow) == false ){
				continue;		// �o�͂ɂȂ����S�͎g�p���Ȃ�
			}
			if ( (edgeNow == LOGO_EDGE_RISE) && isLogoEdgeRise(edgeSel) ){	// rise�G�b�W�m�F
				if ( isLogoListStdNumUse(curRise, maxRise) ){
					listUseLogoIn[i] = true;		// �g�p���郍�S
				}
			}
			if ( (edgeNow == LOGO_EDGE_FALL) && isLogoEdgeFall(edgeSel) ){	// fall�G�b�W�m�F
				if ( isLogoListStdNumUse(curFall, maxFall) ){
					listUseLogoIn[i] = true;		// �g�p���郍�S
				}
			}
		}
	}
	//--- �g�p���S���i�[ ---
	for(int i=0; i<(int)listMsecLogoIn.size(); i++){
		if ( listUseLogoIn[i] ){
			LogoEdgeType edgeNow = ( i % 2 == 0 )? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
			cmdset.limit.addLogoListStd(listMsecLogoIn[i], edgeNow);
		}
	}
}
// ���݃��S�ԍ����I�v�V�����w�胍�S�ԍ����`�F�b�N
bool JlsScriptLimit::isLogoListStdNumUse(int curNum, int maxNum){
	if ( var.opt.sizeLgOpt() == 0 ){		// �w��Ȃ���Ώ������f�͑S���L������
		return true;
	}
	string strLgNum = var.opt.getLgOptAll();
	return pdata->cnv.isStrMultiNumIn(strLgNum, curNum, maxNum);
}
//--- �ݒ�l�ɍ��킹�����͈ʒu���擾 ---
bool JlsScriptLimit::getLogoListStdData(vector<Msec>& listMsecLogoIn, int& locStart, int& locEnd){
	bool typeLogo  = LOGO_SELECT_ALL;
	bool flagLimit = false;
	//--- �I�v�V�����ɂ�郍�S�ݒ�i���̓��S�ԍ��ƕK�v���j ---
	switch( (OptType)var.opt.getOpt(OptType::TypeNumLogo) ){
		case OptType::LgN:						// -N�I�v�V����
		case OptType::LgNFXlogo:				// -NFXlogo�I�v�V����
			typeLogo  = LOGO_SELECT_ALL;
			flagLimit = false;
			break;
		case OptType::LgNR:						// -NR�I�v�V����
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		case OptType::LgNlogo:					// -Nlogo�I�v�V����
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = true;
			break;
		case OptType::LgNauto:					// -Nauto�I�v�V����
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = true;
			break;
		case OptType::LgNFlogo:					// -NFlogo�I�v�V����
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		case OptType::LgNFauto:					// -NFauto�I�v�V����
			typeLogo  = LOGO_SELECT_VALID;
			flagLimit = false;
			break;
		default:
			break;
	}
	//--- �Ή����郍�S�f�[�^�擾 ---
	LogoCustomType custom = pdata->getClogoCustom();	// �o�̓��S�ݒ�擾
	custom.selectAll = ( typeLogo == LOGO_SELECT_ALL );
	vector<WideMsec> listWmsec;
	pdata->trialClogo(listWmsec, custom);		// �Ή����郍�S�쐬
	listMsecLogoIn.clear();
	for(int i=0; i<(int)listWmsec.size(); i++){
		listMsecLogoIn.push_back(listWmsec[i].just);
	}
	//--- ���X�g�̗L���͈͂����� ---
	RangeMsec rmsecHeadTail = {-1, -1};
	if ( flagLimit ){
		rmsecHeadTail = var.getHeadTail();
	}
	bool validList = getLogoListStdDataRange(locStart, locEnd, listMsecLogoIn, rmsecHeadTail);
	//--- �J�n�ƏI����K���Z�b�g�ɂ���ꍇ ---
	if ( var.opt.getOpt(OptType::FlagPair) > 0 ){
		if ( locStart > 0 && (locStart % 2 != 0) ){
			locStart -= 1;
		}
		if ( locEnd > 0 && (locEnd % 2 == 0) ){
			locEnd += 1;
		}
	}
	return validList;
}
// ���X�g�̗L���͈͂�����
bool JlsScriptLimit::getLogoListStdDataRange(int& st, int& ed, vector<Msec>& listMsec, RangeMsec rmsec){
	st = -1;
	ed = -1;
	if ( listMsec.empty() ){
		return false;
	}
	Msec msecMgn = pdata->getClogoMsecMgn();
	bool st1st = true;
	for(int i=0; i < (int)listMsec.size(); i++){
		Msec msecSel = listMsec[i];
		if ( i % 2 == 0 ){		// ���S�J�n��
			msecSel += msecMgn;
		}else{					// ���S�I����
			msecSel -= msecMgn;
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
//---------------------------------------------------------------------
// ���ڈʒu�w��i-from�n�I�v�V�����j����
//---------------------------------------------------------------------
void JlsScriptLimit::getLogoListDirect(JlsCmdSet& cmdset){
	bool exist = false;
	//--- �I�v�V�������璼�ڃt���[���ʒu�w����擾 ---
	vector<Msec> listMsecDirect;
	if ( var.opt.isSetStrOpt(OptType::ListFromAbs) ){			// -fromabs
		exist = true;
		string strList = var.opt.getStrOpt(OptType::ListFromAbs);
		pdata->cnv.getListValMsecM1(listMsecDirect, strList);
	}
	else if ( var.opt.isSetStrOpt(OptType::ListFromHead) ){			// -fromhead
		exist = true;
		RangeMsec rmsecHeadTail = var.getHeadTail();
		string strList = var.opt.getStrOpt(OptType::ListFromHead);
		vector<Msec> listTmp;
		pdata->cnv.getListValMsecM1(listTmp, strList);
		for(int i=0; i<(int)listTmp.size(); i++){
			Msec msec = rmsecHeadTail.st + listTmp[i];
			if ( listTmp[i] < 0 ){		// �����ݒ�
				msec = listTmp[i];
			}
			listMsecDirect.push_back(msec);
		}
	}
	else if ( var.opt.isSetStrOpt(OptType::ListFromTail) ){			// -fromtail
		exist = true;
		RangeMsec rmsecHeadTail = var.getHeadTail();
		string strList = var.opt.getStrOpt(OptType::ListFromTail);
		vector<Msec> listTmp;
		pdata->cnv.getListValMsecM1(listTmp, strList);
		for(int i=0; i<(int)listTmp.size(); i++){
			Msec msec = rmsecHeadTail.ed - listTmp[i];
			if ( listTmp[i] < 0 ){		// �����ݒ�
				msec = listTmp[i];
			}
			listMsecDirect.push_back(msec);
		}
	}
	else{
		bool useAbsS = false;
		bool useAbsE = false;
		string strList;
		if ( var.opt.isSetStrOpt(OptType::ListAbsSetFD) ){			// -AbsSetFD
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFD);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFE) ){		// -AbsSetFE
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFE);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFX) ){		// -AbsSetFX
			useAbsS = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetFX);
		}
		else if ( var.opt.isSetStrOpt(OptType::ListAbsSetXF) ){		// -AbsSetXF
			useAbsE = true;
			strList = var.opt.getStrOpt(OptType::ListAbsSetXF);
		}
		if ( useAbsS || useAbsE ){
			vector<Msec> listTmp;
			pdata->cnv.getListValMsecM1(listTmp, strList);
			for(int i=0; i<(int)listTmp.size(); i++){
				if ( (useAbsS && (i % 2 == 0)) || (useAbsE && (i % 2 == 1)) ){
					listMsecDirect.push_back(listTmp[i]);
				}
			}
		}
	}

	//--- �w�胊�X�g����̎��͖����ݒ� ---
	if ( exist && listMsecDirect.empty() ){
		cmdset.limit.addLogoListDirectDummy(true);
	}
	//--- ���ڃt���[���ʒu�w�肪���݂��鎞�͌��ƂȂ����S�ʒu�ƃZ�b�g�ŕۊ� ---
	int nsizeDir = (int)listMsecDirect.size();
	if ( nsizeDir > 0 ){
		//--- �{���̃��S�ʒu���擾 ---
		vector<Msec> listMsecLogo;		// ���S�ʒu�i�S�́j
		for(int i=0; i<pdata->sizeClogoList(); i++){
			listMsecLogo.push_back( pdata->getClogoMsecFromNum(i) );
		}
		//--- �e�ʒu��ۊ� ---
		for(int i=0; i<nsizeDir; i++){
			//--- ���S�ʒu��ۊ� ---
			Msec msecFrom = listMsecDirect[i];
			LogoEdgeType edgeFrom = var.opt.selectEdge;		// �R�}���h��S/E/B�I��
			cmdset.limit.addLogoListDirect(msecFrom, edgeFrom);	// ���X�g�ɒǉ�
			//--- �e�ʒu���ꂼ���ԋ߂�����S�ʒu���擾 ---
			int locNearest = getLogoListNearest(cmdset, listMsecLogo, msecFrom);
			if ( locNearest >= 0 ){		// ��ԋ߂����S�����݂��������S��
				Msec msecLogo = listMsecLogo[locNearest];
				LogoEdgeType edgeLogo = ( locNearest % 2 == 0 )? LOGO_EDGE_RISE : LOGO_EDGE_FALL;
				int locDir = cmdset.limit.sizeLogoList() - 1;	// �ǉ������ʒu
				cmdset.limit.attachLogoListOrg(locDir, msecLogo, edgeLogo);
			}
		}
	}
	//--- �����\���ʂ̊J�n�ʒu ---
	getLogoListDirectCom(cmdset);
}
// �����\���ʂ̊J�n�ʒu
void JlsScriptLimit::getLogoListDirectCom(JlsCmdSet& cmdset){
	vector<Msec> listMsec;
	vector<LogoEdgeType> listEdge;
	//--- �����\����؂�ɂ��ʒu ---
	ScrOptCRecord optC = {};
	getLogoListDirectComOpt(optC);

	if ( optC.exist ){
		Term term = {};
		pdata->setTermEndtype(term, SCP_END_EDGEIN);	// �[���܂߂Ď��{
		pdata->setTermForDisp(term, true);		// �\���p�̍\��
		RangeMsec rmsecHold;
		bool hold = false;
		bool cont = true;
		while( cont || hold ){
			//--- ���̈ʒu�ǂݍ��� ---
			if ( cont ){
				cont = pdata->getTermNext(term);
			}
			//--- �ړI�̍\�������� ---
			bool valid = false;
			RangeMsec rmsecNow = {-1, -1};
			if ( cont ){
				valid = isLogoListDirectComValid(term.nsc.ed, optC);
				rmsecNow = {term.msec.st, term.msec.ed};
			}
			//--- �������� ---
			if ( valid ){
				if ( !hold ){
					rmsecHold = rmsecNow;	// �V�K�i�[
					hold = true;
					valid = false;
				}else if ( var.opt.getOptFlag(OptType::FlagMerge) &&
						   rmsecHold.ed == rmsecNow.st ){
					rmsecHold.ed = rmsecNow.ed;	// ����
					valid = false;
				}
			}
			//--- �X�V ---
			if ( hold && (valid || !cont) ){	// �f�[�^�ǉ��܂��͍Ō�I������hold�f�[�^���X�V
				LogoEdgeType edgeBase = var.opt.selectEdge;	// �R�}���h��S/E/B�I��
				if ( jlsd::isLogoEdgeRise(edgeBase) ){
					listMsec.push_back(rmsecHold.st);
					listEdge.push_back(LOGO_EDGE_RISE);
				}
				if ( jlsd::isLogoEdgeFall(edgeBase) ){
					listMsec.push_back(rmsecHold.ed);
					listEdge.push_back(LOGO_EDGE_FALL);
				}
				if ( valid ){
					rmsecHold = rmsecNow;
				}else{
					hold = false;
				}
			}
		}
		//--- �w�胊�X�g����̎��͖����ݒ� ---
		if ( listMsec.empty() ){
			cmdset.limit.addLogoListDirectDummy(true);
		}
	}
	//--- ���ڃt���[���ʒu�w�肪���݂��鎞�̓Z�b�g�ŕۊ� ---
	if ( listMsec.empty() == false ){
		for(int i=0; i<(int)listMsec.size(); i++){
			cmdset.limit.addLogoListDirect(listMsec[i], listEdge[i]);	// ���X�g�ǉ�
		}
	}
}
//--- �I�v�V�����ݒ��Ԏ擾 ---
void JlsScriptLimit::getLogoListDirectComOpt(ScrOptCRecord& optC){
	int n;
	n = var.opt.getOpt(OptType::FnumFromAllC);		// -fromC
	if ( n == 1 ){
		optC.exist = true;
		optC.Tra = true;
		optC.Trr = true;
		optC.Trc = true;
		optC.Sp  = true;
		optC.Ec  = true;
		optC.Bd  = true;
		optC.Mx  = true;
		optC.Aea = true;
		optC.Aec = true;
		optC.Cm  = true;
		optC.Nl  = true;
		optC.L   = true;
	}
	else if ( n == 2 ){
		optC.exist = true;
	}

	n = var.opt.getOpt(OptType::FnumFromTr);			// -fromTR
	optC.exist |= getLogoListDirectComOptSub(optC.Tra, n);
	optC.exist |= getLogoListDirectComOptSub(optC.Trr, n);

	n = var.opt.getOpt(OptType::FnumFromSp);			// -fromSP
	optC.exist |= getLogoListDirectComOptSub(optC.Sp, n);

	n = var.opt.getOpt(OptType::FnumFromEc);			// -fromEC
	optC.exist |= getLogoListDirectComOptSub(optC.Ec, n);

	n = var.opt.getOpt(OptType::FnumFromBd);			// -fromBD
	optC.exist |= getLogoListDirectComOptSub(optC.Bd, n);

	n = var.opt.getOpt(OptType::FnumFromTra);			// -fromTRa
	optC.exist |= getLogoListDirectComOptSub(optC.Tra, n);

	n = var.opt.getOpt(OptType::FnumFromTrr);			// -fromTRr
	optC.exist |= getLogoListDirectComOptSub(optC.Trr, n);

	n = var.opt.getOpt(OptType::FnumFromCm);			// -fromCM
	optC.exist |= getLogoListDirectComOptSub(optC.Cm, n);

	n = var.opt.getOpt(OptType::FnumFromNl);			// -fromNL
	optC.exist |= getLogoListDirectComOptSub(optC.Nl, n);

	n = var.opt.getOpt(OptType::FnumFromL);				// -fromL
	optC.exist |= getLogoListDirectComOptSub(optC.L, n);
}
bool JlsScriptLimit::getLogoListDirectComOptSub(bool& data, int n){
	if ( n == 1 ){
		data = true;
		return true;
	}else if ( n == 2 ){
		data = false;
		return true;
	}
	return false;
}
// �����\�����I�v�V�����w��̗L���Ώۂ����f
bool JlsScriptLimit::isLogoListDirectComValid(Nsc nscCur, ScrOptCRecord optC){
	if ( optC.C ){
		return true;
	}
	bool flagOut = true;
	ComLabelType label = pdata->getLabelTypeFromNsc(nscCur, flagOut);
	bool valid = false;
	if ( optC.Tra ){
		switch( label ){
			case ComLabelType::AddTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Trr ){
		switch( label ){
			case ComLabelType::RawTR :
			case ComLabelType::CanTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Trc ){
		switch( label ){
			case ComLabelType::CutTR :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Sp ){
		switch( label ){
			case ComLabelType::AddSP :
			case ComLabelType::CutSP :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Ec ){
		switch( label ){
			case ComLabelType::AddEC :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Bd ){
		switch( label ){
			case ComLabelType::Bd :
			case ComLabelType::Bd15s :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Mx ){
		switch( label ){
			case ComLabelType::Mix :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Aea ){
		switch( label ){
			case ComLabelType::AddNEdge :
			case ComLabelType::AddLEdge :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Aec ){
		switch( label ){
			case ComLabelType::CutNEdge :
			case ComLabelType::CutLEdge :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Cm ){
		switch( label ){
			case ComLabelType::CM :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.Nl ){
		switch( label ){
			case ComLabelType::NoLogo :
			case ComLabelType::CutNoLogo :
				valid = true;
				break;
			default:
				break;
		}
	}
	if ( optC.L ){
		switch( label ){
			case ComLabelType::Logo :
				valid = true;
				break;
			default:
				break;
		}
	}
	return valid;
}

//---------------------------------------------------------------------
// �t���[�����ڎw��ʒu�Ɉ�ԋ߂�����S�ʒu���擾�i-from�n�I�v�V�����p�j
// ���́F
//    listMsec: ���S�ʒu���X�g�i�S�́j
//    msecFrom: �t���[�����ڎw��ʒu
// �o�́F
//   �Ԃ�l�F ���̓��X�g���ň�ԋ߂��ʒu
//---------------------------------------------------------------------
int JlsScriptLimit::getLogoListNearest(JlsCmdSet& cmdset, vector<Msec> listMsec, Msec msecFrom){
	//--- ���O�m�F ---
	int locNearest = -1;						// ���̓��X�g���ň�ԋ߂��ʒu
	cmdset.limit.forceLogoListStd(true);		// �L�����S���X�g�擾�ɋ����ݒ�
	//--- �L�����S���X�g�̊e�ʒu�̒������ԋ߂��ʒu������ ---
	Msec msecDif = 0;
	bool flagArea = false;
	int  nsize = cmdset.limit.sizeLogoList();		// �L�����肳�ꂽ���S����
	for(int i=0; i<nsize; i++){
		Msec         msecRef = cmdset.limit.getLogoListMsec(i);		// �L�����S���X�g���̃��S����
		LogoEdgeType edgeRef = cmdset.limit.getLogoListEdge(i);
		//--- �e���S��Ԃ��擾���O����܂߂����S�ʒu���擾 ---
		for(int j=0; j<(int)listMsec.size(); j++){
			if ( msecRef == listMsec[j] ){
				//--- �ݒ���ʒu�Ƌ������� ---
				WideMsec wmsec;
				wmsec.early = ( j > 0 )? listMsec[j-1] : 0;
				wmsec.just  = msecRef;
				wmsec.late  = ( j < (int)listMsec.size()-1 )? listMsec[j+1] : pdata->getMsecTotalMax();
				Msec msecTmpDif = abs(msecFrom - wmsec.just);
				//--- ���S��ԓ������� ---
				bool flagTmpArea = false;
				if ( jlsd::isLogoEdgeRise(edgeRef) ){		// rise edge
					if ( wmsec.early <= msecFrom && msecFrom < wmsec.late ){
						flagTmpArea = true;
					}
				}else{		// fall edge
					if ( wmsec.early < msecFrom && msecFrom <= wmsec.late ){
						flagTmpArea = true;
					}
				}
				//--- ��ԋ߂��ʒu�͍X�V ---
				if ( msecDif > msecTmpDif || locNearest < 0 || (flagTmpArea && !flagArea) ){
					//--- ���S��ԓ� �܂��� ���S��Ԗ����� ---
					if ( flagTmpArea || !flagArea ){
						locNearest = j;					// ���X�g�̈ʒu
						msecDif  = msecTmpDif;
						flagArea = flagTmpArea;
					}
				}
			}
		}
	}
	cmdset.limit.forceLogoListStd(false);		// �L�����S���X�g�擾�̋����ݒ������
	return locNearest;
}


//=====================================================================
// �^�[�Q�b�g�I��
//=====================================================================

//---------------------------------------------------------------------
// �Ώۃ��S�ɂ��Đ���������������đΏۈʒu�擾
//---------------------------------------------------------------------
bool JlsScriptLimit::selectTargetByLogo(JlsCmdSet& cmdset, int nlist){
	//--- �f�[�^������ ---
	cmdset.limit.clearLogoBase();
	cmdset.limit.clearTargetData();
	var.clearRangeDst();
	//--- ����S���m�� ---
	bool exeflag = baseLogo(cmdset, nlist);
	//--- �����Ώ۔͈͂�ݒ�i����S�ʒu���x�[�X�j ---
	if (exeflag){
		exeflag = targetRangeByLogo(cmdset);
	}
	//--- �^�[�Q�b�g�Ɉ�ԋ߂��V�[���`�F���W�ʒu���擾 ---
	if (exeflag){
		targetPoint(cmdset);
	}
	return exeflag;
}

//---------------------------------------------------------------------
// �Ώ۔͈͂�����A����������������đΏۈʒu�擾
//---------------------------------------------------------------------
void JlsScriptLimit::selectTargetByRange(JlsCmdSet& cmdset, WideMsec wmsec){
	//--- �f�[�^������ ---
	cmdset.limit.clearTargetData();
	var.clearRangeDst();
	//--- �����Ώ۔͈͂𒼐ڐ��l�Őݒ� ---
	targetRangeByImm(cmdset, wmsec);
	//--- ��ԋ߂��V�[���`�F���W�ʒu���擾 ---
	targetPoint(cmdset);
}

//=====================================================================
// ���S�ʒu���X�g���̎w�胍�S�Ŋ���S�f�[�^���쐬
//=====================================================================

//---------------------------------------------------------------------
// ����S�f�[�^���쐬
// ���́F
//    nlist: �L�����S���X�g����I������ԍ�
// �o�́F
//   �Ԃ�l�F ���񖞂������S��񔻒�ifalse=���񖞂����Ȃ� true=���񖞂������S���擾�j
//    cmdset.limit.setLogoB*
//---------------------------------------------------------------------
bool JlsScriptLimit::baseLogo(JlsCmdSet& cmdset, int nlist){
	//--- ���S�̊�ʒu�֘A���擾 ---
	bool exeflag = getBaseLogo(cmdset, nlist);

	//--- ��ʒu�������𖞂������m�F ---
	if (exeflag){
		exeflag = checkBaseLogo(cmdset);	
	}
	return exeflag;
}
// ����S���擾
bool JlsScriptLimit::getBaseLogo(JlsCmdSet& cmdset, int nlist){
	//--- �R�}���h�ݒ���擾 ---
	Msec         msecBsrc = cmdset.limit.getLogoListMsec(nlist);		// �ύX�����S�ʒu
	Msec         msecBorg = cmdset.limit.getLogoListOrgMsec(nlist);		// �{���̊���S�ʒu
	LogoEdgeType edgeBase = cmdset.limit.getLogoListEdge(nlist);
	//--- ���X�g�ԍ��ݒ� ---
	var.setLogoBaseListNum(nlist);

	if ( var.opt.getOptFlag(OptType::FlagSftLogo) ){	// ��ʒu��{���̈ʒu��(-SftLogo)
		msecBsrc = msecBorg;
	}
	if ( msecBsrc < 0 ) return false;	// �����ʒu�m�F

	//--- ����̊�Ƃ��ă��S����ݒ� ---
	if ( pdata->isClogoReal() ){
		Nrf nrfBase = pdata->getClogoRealNrf(msecBorg, edgeBase);
		var.setLogoBaseNrf(nrfBase, edgeBase);		// ���ۃ��S�Ŋ�ݒ�
		cmdset.limit.setLogoBaseNrf(nrfBase, edgeBase);	// ���ʗ̈�ɐݒ�
	}else{
		Nsc nscBase = pdata->getClogoNsc(msecBorg);
		var.setLogoBaseNsc(nscBase, edgeBase);		// �������S�Ŋ�ݒ�
		cmdset.limit.setLogoBaseNsc(nscBase, edgeBase);	// ���ʗ̈�ɐݒ�
	}
	Msec msecBmod = msecBorg;
	if ( msecBmod < 0 ){
		msecBmod = pdata->getClogoMsecNear(msecBsrc, edgeBase);	// ���݂��Ȃ����͕ύX��Ɉ�ԋ߂��ʒu
	}
	var.setLogoBsrcMsec(msecBsrc);		// �ύX�����S�ʒu��ݒ�
	var.setLogoBorgMsec(msecBmod);		// �{���̊���S�ʒu��ݒ�i���݂��Ȃ����͋߂��ɕύX�j

	//--- �^�[�Q�b�g�I��̈�̌v�Z�Ɏg�p���郍�S����ݒ� ---
	WideMsec wmsecTg;
	wmsecTg.just  = msecBsrc;
	wmsecTg.early = msecBsrc;
	wmsecTg.late  = msecBsrc;
	LogoEdgeType edgeTg = edgeBase;
	bool flagBase = ( msecBsrc == msecBorg );
	getBaseLogoForTg(wmsecTg, edgeTg, cmdset, flagBase);	// �I�v�V�������ɂ��␳
	if ( wmsecTg.late < 0 ){
		return false;	// �����ʒu�m�F
	}
	var.setLogoBtgWmsecEdge(wmsecTg, edgeTg);	// �^�[�Q�b�g�v�Z�p�̊�ʒu��ݒ�
	return true;
}
// �^�[�Q�b�g�I��̈�̌v�Z�Ɏg�p���郍�S����␳
void JlsScriptLimit::getBaseLogoForTg(WideMsec& wmsecTg, LogoEdgeType& edgeTg, JlsCmdSet& cmdset, bool flagBase){
	//--- �R�}���h�ݒ���擾 ---
	Msec msecMgn  = pdata->getClogoMsecMgn();
	Msec msecBsrc = wmsecTg.just;
	if ( msecBsrc < 0 ){
		return;
	}
	//--- ���S�P�ʈړ��I�v�V���� ---
	int numSft = 0;
	if ( var.opt.getOptFlag(OptType::FlagFromLast) ){	// �P�O�̃��S�����(-fromlast)
		numSft = -1;
	}
	if ( numSft == 0 && !flagBase ){	// ���S�͈̓`�F�b�N�Ȃ���Ή������Ȃ�
		return;
	}
	//--- ���S�ʒu�`�F�b�N ---
	int locLogo = pdata->getClogoNumNear(msecBsrc, edgeTg);	// ����S�ʒu
	if ( flagBase ){
		wmsecTg = pdata->getClogoWmsecFromNum(locLogo+numSft);
	}else{
		if ( numSft < 0 ){
			locLogo = pdata->getClogoNumPrev(msecBsrc-msecMgn, LOGO_EDGE_BOTH);
		}
		wmsecTg = pdata->getClogoWmsecFromNum(locLogo);
	}
	//--- ���S�͈̓`�F�b�N ---
	bool flagLogoWide = false;
	if ( var.opt.getOptFlag(OptType::FlagWide) ){	// �\���͈͂Ō���(-wide)
		flagLogoWide = true;
	}else if ( pdata->isClogoReal() == false ){	// ���ۂ̃��S�ł͂Ȃ����������^�����蕝
		flagLogoWide = true;
	}
	if ( !flagLogoWide ){
		wmsecTg.early = wmsecTg.just;
		wmsecTg.late  = wmsecTg.just;
	}
}
// ����S�ʒu�ɑΉ���������ݒ���m�F
bool JlsScriptLimit::checkBaseLogo(JlsCmdSet& cmdset){
	//--- �R�}���h�ݒ���擾 ---
	bool flagRealLogo = pdata->isClogoReal();
	Msec msecLogoBsrc = var.getLogoBsrcMsec();		// �ݒ���ʒu
	Nrf nrf_base = var.getLogoBaseNrf();			// ����S�ʒu
//	Nsc nsc_base = var.getLogoBaseNsc();
	bool exeflag = true;
	//--- ���S�ʒu�𒼐ڐݒ肷��R�}���h�ɕK�v�ȃ`�F�b�N ---
	if ( flagRealLogo ){
		//--- �m�茟�o�ς݃��S���m�F ---
		Msec           msec_tmp   = -1;
		LogoResultType outtype_rf = LOGO_RESULT_DECIDE;	// ����S���Ȃ����͊m�莞�̏���
		if ( nrf_base >= 0 ){
			pdata->getResultLogoAtNrf(msec_tmp, outtype_rf, nrf_base);
		}
		//--- �m�胍�S�ʒu�����o����R�}���h�� ---
		bool igncomp = cmdset.arg.tack.ignoreComp;
		bool ignabort = cmdset.arg.tack.ignoreAbort;
		if (outtype_rf == LOGO_RESULT_NONE || (outtype_rf == LOGO_RESULT_DECIDE && igncomp)){
		}
		else if ( outtype_rf == LOGO_RESULT_ABORT && ignabort ){	// Abort�����Ŏ��s����ꍇ
		}
		else{
			exeflag = false;
		}
		//--- select�p�m���⑶�ݎ��͏��� ---
		if (var.opt.cmdsel == CmdType::Select &&
			var.opt.getOptFlag(OptType::FlagReset) == false &&
			pdata->getPriorLogo(nrf_base) >= LOGO_PRIOR_DECIDE){
			exeflag = false;
		}
	}
	//--- �����ɍ��������� ---
	if (exeflag){
		//--- �t���[���͈̓`�F�b�N�i�ύX�����S�ʒu�Ŋm�F�j ---
		if ( var.opt.isSetOpt(OptType::TypeFrameSub) &&	// �t���[���͈̓I�v�V��������
			(var.opt.getOpt(OptType::TypeFrameSub) & 0x2) == 0 ){	// -FT�n�ł͂Ȃ�
			RangeMsec rmsecFrame = var.getFrameRange();	// �t���[���͈�
			//--- ���S���͈͓����m�F ---
			if ((rmsecFrame.st > msecLogoBsrc && rmsecFrame.st >= 0) ||
				(rmsecFrame.ed < msecLogoBsrc && rmsecFrame.ed >= 0)){
				exeflag = false;
			}
		}
		//--- �I�v�V�����Ɣ�r(-LenP, -LenN)�i�{���̊���S�Ŋm�F�j ---
		if (exeflag){
			bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
			WideMsec wmsecLg;
			var.getWidthLogoFromBase(wmsecLg, 1, flagWide);	// �O��͗א�(1)
			RangeMsec lenP;
			RangeMsec lenN;
			lenP.st = var.opt.getOpt(OptType::MsecLenPMin);
			lenP.ed = var.opt.getOpt(OptType::MsecLenPMax);
			lenN.st = var.opt.getOpt(OptType::MsecLenNMin);
			lenN.ed = var.opt.getOpt(OptType::MsecLenNMax);
			exeflag = checkBaseLogoLength(wmsecLg, lenP, lenN);
		}
		//--- �I�v�V�����Ɣ�r(-LenPE, -LenNE)�i�{���̊���S�Ŋm�F�j ---
		if (exeflag){
			bool flagWide = false;		// �e�_���S�ʒu�Őݒ�
			WideMsec wmsecLgE;
			var.getWidthLogoFromBase(wmsecLgE, 2, flagWide);	// �O��͓��G�b�W�א�(2)
			RangeMsec lenPE;
			RangeMsec lenNE;
			lenPE.st = var.opt.getOpt(OptType::MsecLenPEMin);
			lenPE.ed = var.opt.getOpt(OptType::MsecLenPEMax);
			lenNE.st = var.opt.getOpt(OptType::MsecLenNEMin);
			lenNE.ed = var.opt.getOpt(OptType::MsecLenNEMax);
			exeflag = checkBaseLogoLength(wmsecLgE, lenPE, lenNE);
		}
	}
	//--- ���S�ʒu����-SC�n�I�v�V����������ꍇ�̊m�F�i�ύX�����S�ʒu�Ŋm�F�j ---
	if ( !var.opt.tack.floatBase && !var.opt.tack.shiftBase && exeflag){
		LogoEdgeType edgeBase = var.getLogoBsrcEdge();
		exeflag = var.isScpEnableAtMsec(msecLogoBsrc, edgeBase, TargetCatType::From);
	}
	return exeflag;
}
//--- �O�ネ�S�Ԃ̒����ɂ�鐧�� ---
bool JlsScriptLimit::checkBaseLogoLength(WideMsec wmsecLg, RangeMsec lenP, RangeMsec lenN){
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

//=====================================================================
// �^�[�Q�b�g�͈͂��擾
//=====================================================================

//---------------------------------------------------------------------
// �����Ώ۔͈͂�ݒ�i����S�ʒu���x�[�X�j
// �o�́F
//   �Ԃ�l�F���񖞂����͈͊m�F�i0:�Y���Ȃ�  1:�Ώ۔͈͎擾�j
//   var.addRangeDst()
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
bool JlsScriptLimit::targetRangeByLogo(JlsCmdSet& cmdset){
	//--- �^�[�Q�b�g�͈͐ݒ�p�̊�ʒu�擾 ---
	WideMsec wmsecBase = var.getLogoBtgWmsec();	// �^�[�Q�b�g�p���S�ʒu�i�������܁j�擾

	//--- ��ʒu����^�[�Q�b�g�͈͍쐬 ---
	if ( var.opt.tack.shiftBase ){		// �V�t�g��ʒu�i-shift�I�v�V�����j
		addTargetRangeByLogoShift(wmsecBase);
	}else{				// �ʏ�̂P�̊�ʒu
		addTargetRangeData(wmsecBase);
	}
	//--- �^�[�Q�b�g���ݒ� ---
	bool fromLogo = true;			// ���S�ʒu�x�[�X�̃^�[�Q�b�g
	updateTargetRange(cmdset, fromLogo);

	return ( var.sizeRangeDst() > 0 );
}
//---------------------------------------------------------------------
// �����Ώ۔͈͂̊�ʒu�𒼐ڐ��l�ݒ�
// �o�́F
//   var.addRangeDst()
//   cmdset.limit.setTargetRange()
//---------------------------------------------------------------------
void JlsScriptLimit::targetRangeByImm(JlsCmdSet& cmdset, WideMsec wmsec){
	//--- ��ʒu����^�[�Q�b�g�͈͍쐬 ---
	addTargetRangeData(wmsec);
	//--- �^�[�Q�b�g���ݒ� ---
	bool fromLogo = false;			// ���S�ʒu��ł͂Ȃ����ڎw��R�}���h
	updateTargetRange(cmdset, fromLogo);
}

// �^�[�Q�b�g�����͈͂̊�{����ݒ�
void JlsScriptLimit::updateTargetRange(JlsCmdSet& cmdset, bool fromLogo){
	WideMsec wmsec = var.getRangeDstWide();
	cmdset.limit.setTargetRange(wmsec, fromLogo);
}
// �V�t�g�������ɑΉ��������S��ʒu��ݒ�
void JlsScriptLimit::addTargetRangeByLogoShift(WideMsec wmsecBase){
	//--- �\�[�g�p ---
	struct data_t {
		WideMsec wmsec;
		Msec gap;
		bool operator<( const data_t& right ) const {
			return gap < right.gap;
		}
		bool operator==( const data_t& right ) const {
			return gap == right.gap;
		}
	};

	//--- �����͈͐ݒ� ---
	WideMsec wmsecScope = wmsecBase;
	wmsecScope.just  += var.opt.getOpt(OptType::MsecSftC);
	wmsecScope.early += var.opt.getOpt(OptType::MsecSftL);
	wmsecScope.late  += var.opt.getOpt(OptType::MsecSftR);
	//--- ������ʒu�̌����J�n ---
	vector<data_t> listTarget;
	//--- �����V�|���`�F���W�̌������ԂɃ`�F�b�N ---
	bool flagNoEdge = var.opt.getOptFlag(OptType::FlagNoEdge);
	RangeNsc rnsc = pdata->getRangeNscTotal(flagNoEdge);
	for(int j=rnsc.st; j<=rnsc.ed; j++){
		Msec msecNow   = pdata->getMsecScp(j);
		Msec msecNowBk = pdata->getMsecScpBk(j);
		if ( wmsecScope.early <= msecNow && msecNowBk <= wmsecScope.late ){	// �����͈͓�
			//--- -shift�ɂ�郍�S�ʒu����-SC�n�I�v�V�������m�F ---
			bool valid = var.isScpEnableAtMsec(msecNow, LOGO_EDGE_RISE, TargetCatType::Shift);
			if ( valid ){		// ����S�ʒu�Ƃ��ėL��
				Msec msecRel = msecNow - wmsecBase.just;
				WideMsec wmsecTmp;
				wmsecTmp.just  = wmsecBase.just  + msecRel;
				wmsecTmp.early = wmsecBase.early + msecRel;
				wmsecTmp.late  = wmsecBase.late  + msecRel;
				data_t dat;
				dat.wmsec = wmsecTmp;
				dat.gap   = abs(msecRel);
				listTarget.push_back(dat);		// �����X�g�ɒǉ�
			}
		}
	}
	if ( listTarget.size() > 0 ){
		std::sort(listTarget.begin(), listTarget.end());	// �߂����Ƀ\�[�g
	}else{
		if ( var.opt.tack.forcePos ){		// ��ʒu�������ݒ肷��ꍇ
			data_t dat;
			dat.wmsec = wmsecBase;
			dat.gap   = 0;
			listTarget.push_back(dat);		// �����X�g�ɒǉ�
		}
	}
	//--- �����𖞂�������ʒu��ݒ� ---
	for(int i=0; i<(int)listTarget.size(); i++){
		addTargetRangeData(listTarget[i].wmsec);
	}
}
//---------------------------------------------------------------------
// �^�[�Q�b�g�����͈͂�ݒ�i�����ǉ����Ă������Ɖ\�j
//---------------------------------------------------------------------
void JlsScriptLimit::addTargetRangeData(WideMsec wmsecBase){
	//--- -DList�擾 ---
	vector<Msec> listMsec;
	if ( var.opt.isSetStrOpt(OptType::ListTgDst) ){		// -DList
		string strList = var.opt.getStrOpt(OptType::ListTgDst);
		pdata->cnv.getListValMsecM1(listMsec, strList);
	}
	//--- End�n�Ƃ��Ďg�����ʒu�𐳊m�ɐݒ� ---
	WideMsec wmsecFrom = {wmsecBase.just, wmsecBase.just, wmsecBase.just};
	if ( var.opt.getOptFlag(OptType::FlagFixPos) == false ||
		 var.opt.tack.immFrom == false ){	// -fromabs���̒��ڃt���[���ԍ��w��ł͂Ȃ�
		Nsc nscTmp = pdata->getNscFromMsecMgn(wmsecBase.just, pdata->msecValExact, SCP_END_NOEDGE);
		if ( nscTmp >= 0 ){
			wmsecFrom.early = pdata->getMsecScpBk(nscTmp);
			wmsecFrom.late  = pdata->getMsecScp(nscTmp);
		}
	}
	//--- �^�[�Q�b�g�����͈͂�ݒ� ---
	int nsize = (int)listMsec.size();
	if ( nsize > 0 ){		// �������I�v�V�����w��i-DList�j
		for(int i=0; i<nsize; i++){
			WideMsec wmsecOpt;
			wmsecOpt.just  = listMsec[i] + wmsecBase.just;
			wmsecOpt.early = listMsec[i] + wmsecBase.early;
			wmsecOpt.late  = listMsec[i] + wmsecBase.late;
			WideMsec wmsecFind;
			if ( findTargetRange(wmsecFind, wmsecOpt, wmsecFrom.just) ){
				var.addRangeDst(wmsecFind, wmsecFrom);
			}
		}
	}else{		// �ʏ�̒P��ݒ�
		WideMsec wmsecFind;
		if ( findTargetRange(wmsecFind, wmsecBase, wmsecFrom.just) ){
			var.addRangeDst(wmsecFind, wmsecFrom);
		}
	}
}
// �^�[�Q�b�g�I��̈�̌v�Z�Ɏg�p���郍�S����␳
bool JlsScriptLimit::findTargetRange(WideMsec& wmsecFind, WideMsec wmsecBase, Msec msecFrom){
	WideMsec wmsecAnd = {-1, -1, -1};		// �ʏ�͖��g�p
	bool exeflag = findTargetRangeSetBase(wmsecFind, wmsecAnd, wmsecBase, msecFrom);
	if ( exeflag ){
		exeflag = findTargetRangeLimit(wmsecFind, wmsecAnd);
	}
	return exeflag;
}
// �^�[�Q�b�g�ʒu�ύX�I�v�V�����𔽉f
bool JlsScriptLimit::findTargetRangeSetBase(WideMsec& wmsecFind, WideMsec& wmsecAnd, WideMsec wmsecBase, Msec msecFrom){
	//--- ��{�ݒ� ---
	wmsecFind = wmsecBase;
	bool flagAnd = var.opt.getOptFlag(OptType::FlagDstAnd);	// -DstAnd
	if ( flagAnd ){
		wmsecAnd = {msecFrom, msecFrom, msecFrom};	// And�ݒ莞�p
	}
	//--- ���ڈʒu���X�g�w�� ---
	if ( var.opt.isSetStrOpt(OptType::ListDstAbs) ){	// -DstAbs
		vector<Msec> listMsec;
		string strList = var.opt.getStrOpt(OptType::ListDstAbs);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			int nlist = var.getLogoBaseListNum();
			if ( nlist < 0 ) nlist = 0;
			if ( nlist >= nsize ) nlist = nsize-1;
			wmsecFind.just  = listMsec[nlist];
			wmsecFind.early = listMsec[nlist];
			wmsecFind.late  = listMsec[nlist];
			if ( flagAnd ){
				wmsecAnd = wmsecFind;
			}
		}
	}
	else if ( var.opt.isSetStrOpt(OptType::ListAbsSetFD) ){	// -AbsSetFD
		vector<Msec> listMsec;
		string strList = var.opt.getStrOpt(OptType::ListAbsSetFD);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			if ( nsize < 2 ) return false;		// �K��2�f�[�^�Z�b�g
			int nlist = var.getLogoBaseListNum();
			int ndst = nlist * 2 + 1;
			if ( ndst <= 0 ) ndst = 1;
			if ( ndst >= nsize ) ndst = nsize-1;
			wmsecFind.just  = listMsec[nlist];
			wmsecFind.early = listMsec[nlist];
			wmsecFind.late  = listMsec[nlist];
			if ( flagAnd ){
				wmsecAnd = wmsecFind;
			}
		}
	}
	if ( wmsecFind.just < 0 ){
		return false;
	}
	//--- �Q�ƈʒu�ݒ� ---
	WideMsec wmsecNext = ( flagAnd )? wmsecAnd : wmsecFind;		// And�ݒ蕪��
	{
		Msec msecSrc = wmsecNext.just;
		bool existNextL = var.opt.isSetOpt(OptType::NumDstNextL);	// -DstNextL
		bool existPrevL = var.opt.isSetOpt(OptType::NumDstPrevL);	// -DstPrevL
		if ( existNextL || existPrevL ){
			int numArg = 0;
			if ( existNextL ) numArg = var.opt.getOpt(OptType::NumDstNextL);
			if ( existPrevL ) numArg = var.opt.getOpt(OptType::NumDstPrevL) * -1;
			int locLogo;
			if ( numArg > 0 ){
				locLogo = pdata->getClogoNumNextCount(msecSrc, abs(numArg));
			}else if ( numArg < 0 ){
				locLogo = pdata->getClogoNumPrevCount(msecSrc, abs(numArg));
			}else{
				locLogo = pdata->getClogoMsecNear(msecSrc, LOGO_EDGE_BOTH);
			}
			wmsecNext = pdata->getClogoWmsecFromNum(locLogo);
		}
	}
	{
		Msec msecSrc = wmsecNext.just;
		bool existNextC = var.opt.isSetOpt(OptType::NumDstNextC);	// -DstNextC
		bool existPrevC = var.opt.isSetOpt(OptType::NumDstPrevC);	// -DstPrevC
		bool selNextCom = var.opt.tack.comFrom && !var.opt.tack.existDstOpt && !var.opt.tack.immFrom;
		if ( existNextC || existPrevC || selNextCom ){
			int numArg = 0;
			if ( existNextC ){
				numArg = var.opt.getOpt(OptType::NumDstNextC);
			}else if ( existPrevC ){
				numArg = var.opt.getOpt(OptType::NumDstPrevC) * -1;
			}else if ( var.opt.selectEdge == LOGO_EDGE_RISE ){
				numArg = 1;
			}else if ( var.opt.selectEdge == LOGO_EDGE_FALL ){
				numArg = -1;
			}
			Nsc nscT;
			if ( numArg > 0 ){
				nscT = pdata->getNscNextScpDispFromMsecCount(msecSrc, abs(numArg), true);
			}else if ( numArg < 0 ){
				nscT = pdata->getNscPrevScpDispFromMsecCount(msecSrc, abs(numArg), true);
			}else{
				nscT = pdata->getNscFromMsecDisp(msecSrc, pdata->getMsecTotalMax(), SCP_END_EDGEIN);
			}
			wmsecNext = pdata->getWideMsecScp(nscT);
		}
	}
	//--- And�ݒ蕪�� ---
	if ( flagAnd ){
		wmsecAnd = wmsecNext;
		wmsecAnd.early -= pdata->msecValSpc;
		wmsecAnd.late  += pdata->msecValSpc;
	}else{
		wmsecFind = wmsecNext;
	}
	return true;
}
// �R�}���h�w��̃^�[�Q�b�g�͈͂����͈̔͂ɒǉ�
bool JlsScriptLimit::findTargetRangeLimit(WideMsec& wmsecFind, WideMsec& wmsecAnd){
	bool exeflag = true;
	//--- �R�}���h�w��͈̔͂��t���[���͈͂ɒǉ� ---
	wmsecFind.just  += var.opt.wmsecDst.just;
	wmsecFind.early += var.opt.wmsecDst.early;
	wmsecFind.late  += var.opt.wmsecDst.late;
	//--- �R�}���h�͈͂�-1�ݒ莞�̕ϊ� ---
	if ( var.opt.wmsecDst.early == -1 ){
		wmsecFind.early = 0;
	}
	if ( var.opt.wmsecDst.late == -1 ){
		wmsecFind.late = pdata->getMsecTotalMax();
	}
	//--- ���S�����Ɍ��肷��Select�R�}���h�p�͈̔� ---
	if ( exeflag && var.opt.cmdsel == CmdType::Select ){
		if ( var.getLogoBsrcMsec() == var.getLogoBorgMsec() ){
			WideMsec wmsecBtg = var.getLogoBtgWmsec();	// ���S�ʒu�ƌ��������擾
			RangeMsec rmsecExt;
			rmsecExt.st = wmsecBtg.early + var.opt.getOpt(OptType::MsecLogoExtL);
			rmsecExt.ed = wmsecBtg.late  + var.opt.getOpt(OptType::MsecLogoExtR);
			exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecExt);
		}
	}
	//--- �O��̃��S�ʒu�ȓ��ɔ͈͌��肷��ꍇ ---
	if ( exeflag && var.opt.tack.limitByLogo ){
		bool flagWide = true;		// �\������͈͂Ō���
		WideMsec wmsecPN;
		var.getWidthLogoFromBaseForTarget(wmsecPN, 1, flagWide);	// �^�[�Q�b�g�p���S�őO��͗א�(1)
		RangeMsec rmsecPN = { wmsecPN.early, wmsecPN.late };
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecPN);
	}
	//--- �t���[���w��͈͓��Ɍ��� ---
	if ( exeflag ){
		RangeMsec rmsecWindow = var.getFrameRange();
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecWindow);
	}
	//--- -DstAnd�ɂ����� ---
	if ( exeflag && wmsecAnd.just >= 0 ){
		RangeMsec rmsecFind = {wmsecFind.early, wmsecFind.late};
		wmsecFind = wmsecAnd;
		exeflag = pdata->limitWideMsecFromRange(wmsecFind, rmsecFind);
	}
	//--- �͈͂����݂��Ȃ���Ζ����� ---
	if ( wmsecFind.early > wmsecFind.late ){
		exeflag = false;
	}
	return exeflag;
}

//=====================================================================
// �^�[�Q�b�g�ʒu���擾
//=====================================================================

//---------------------------------------------------------------------
// �^�[�Q�b�g�Ɉ�ԋ߂��V�[���`�F���W�ʒu���擾
// �o�́F
//   cmdset.list.setTargetOutEdge() : �o�͎��̃��S�G�b�W
//   cmdset.list.setResult*()       : �I���V�[���`�F���W�ʒu
//---------------------------------------------------------------------
void JlsScriptLimit::targetPoint(JlsCmdSet& cmdset){
	//--- �����ɍ����ʒu������ ---
	seekTargetPoint(cmdset);

	//--- �ʒu�o�͎��p�̃��S�G�b�W�ɂ��␳ ---
	setTargetPointOutEdge(cmdset);
}

//---------------------------------------------------------------------
// �ʒu���o�͎��̃��S�G�b�W�ɂ��␳
//---------------------------------------------------------------------
void JlsScriptLimit::setTargetPointOutEdge(JlsCmdSet& cmdset){
	//--- �W���G�b�W�ݒ� ---
	LogoEdgeType edgeBase = cmdset.limit.getLogoBaseEdge();
	LogoEdgeType edgeDst = edgeBase;
	LogoEdgeType edgeEnd = edgeBase;
	//--- �f�[�^�擾 ---
	TargetLocInfo tgDst = cmdset.limit.getResultDstCurrent();
	TargetLocInfo tgEnd = cmdset.limit.getResultEndCurrent();
	//--- �ʒu�ɂ��ݒ� ---
	if ( cmdset.arg.cmdsel == CmdType::MkLogo  ||	// MkLogo�R�}���h�͏]���ʂ�J�n�|�I���ʒu
		 cmdset.arg.getOptFlag(OptType::FlagHoldBoth) ||
		 cmdset.arg.getOptFlag(OptType::FlagEdgeB) ){
		if ( tgDst.msec < tgEnd.msec ){
			edgeDst = LOGO_EDGE_RISE;
			edgeEnd = LOGO_EDGE_FALL;
		}else{
			edgeDst = LOGO_EDGE_FALL;
			edgeEnd = LOGO_EDGE_RISE;
		}
	}
	//--- �Œ�G�b�W�ݒ� ---
	if ( cmdset.arg.getOptFlag(OptType::FlagEdgeS) ){
		edgeDst = LOGO_EDGE_RISE;
		edgeEnd = LOGO_EDGE_RISE;
	}
	if ( cmdset.arg.getOptFlag(OptType::FlagEdgeE) ){
		edgeDst = LOGO_EDGE_FALL;
		edgeEnd = LOGO_EDGE_FALL;
	}
	//--- �����߂� ---
	tgDst.edge = edgeDst;
	tgEnd.edge = edgeEnd;
	cmdset.limit.setResultDst(tgDst);
	cmdset.limit.setResultEnd(tgEnd);
}
//---------------------------------------------------------------------
// �����ɍ����ʒu������
//---------------------------------------------------------------------
void JlsScriptLimit::seekTargetPoint(JlsCmdSet& cmdset){
	//--- �����J�n�O�̏��� ---
	prepTargetPoint(cmdset);
	//--- �����ݒ� ---
	var.initSeekVar(cmdset);
	//--- �����V�[���`�F���W�ʒu�����ԂɌ��� ---
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	if ( !fixpos ){
		RangeNsc rnsc = var.seek.rnscScp;
		for(int j=rnsc.st; j<=rnsc.ed; j++){
			bool lastNsc = ( j == rnsc.ed );
			seekTargetPointFromScp(cmdset, j, lastNsc);
		}
	}
	//--- ���o�ł��Ȃ������ꍇ�̏��� ---
	if ( var.seek.tgDst.tp == TargetScpType::None ){
		bool flagForce = var.opt.tack.forcePos || fixpos;
		var.selRangeDstNum(0);	// �擪���I��
		var.seek.tgDst.nsc  = -1;
		var.seek.tgDst.msec = var.getRangeDstJust();
		var.seek.tgDst.msbk = var.seek.tgDst.msec;
		bool zone = var.isRangeToDst(var.getRangeDstFrom(), var.seek.tgDst.msec);
		if ( !zone ){
			var.seek.tgDst.tp = TargetScpType::Invalid;
		}else if ( flagForce ){
			if ( pdata->isRangeInTotalMax(var.seek.tgDst.msec) ){
				var.seek.tgDst.tp = TargetScpType::Force;
			}else{
				var.seek.tgDst.tp = TargetScpType::Invalid;
			}
		}else if ( fixpos ){
			var.seek.tgDst.tp = TargetScpType::Direct;
		}else{
			var.seek.tgDst.tp = TargetScpType::Invalid;
		}
		if ( var.seek.tgDst.tp != TargetScpType::Invalid ){
			seekTargetPointEnd(cmdset, var.seek.tgDst.msec, flagForce);	// End�n�_������
		}else{
			//var.seek.tgDst.tp   = TargetScpType::Invalid;
			var.seek.tgEnd.tp   = TargetScpType::Invalid;
		}
	}
	//--- �����V�[���`�F���W�`�F�b�N�Ȃ��ꍇ�̐����\���G�b�W�␳ ---
	if ( fixpos ){
		if ( pdata->isClogoReal() == false ){
			WideMsec wmsec = var.getLogoBtgWmsec();
			Msec msecRevBk = wmsec.just - wmsec.early;
			var.seek.tgDst.msbk = var.seek.tgDst.msec - msecRevBk;	// �I���ʒu�p�̕␳
		}
	}
	//--- �t���[���P�ʂ̌��� ---
	var.seek.tgDst.msec = pdata->cnv.getMsecAlignFromMsec(var.seek.tgDst.msec);
	var.seek.tgDst.msbk = pdata->cnv.getMsecAlignFromMsec(var.seek.tgDst.msbk);
	var.seek.tgEnd.msec = pdata->cnv.getMsecAlignFromMsec(var.seek.tgEnd.msec);
	var.seek.tgEnd.msbk = pdata->cnv.getMsecAlignFromMsec(var.seek.tgEnd.msbk);
	//--- ���ʒu�m�菈�� ---
	cmdset.limit.setResultDst(var.seek.tgDst);
	cmdset.limit.setResultEnd(var.seek.tgEnd);
}
//--- �w�薳���V�[���`�F���W�ʒu����̑Ώی��� ---
void JlsScriptLimit::seekTargetPointFromScp(JlsCmdSet& cmdset, Nsc nscNow, bool lastNsc){
	Msec         msecNow = pdata->getMsecScp(nscNow);
	ScpPriorType statNow = pdata->getPriorScp(nscNow);
	bool exist     = ( var.seek.tgDst.tp != TargetScpType::None );
	//--- �����𖞂����Ȃ���Ύ��̈ʒu�� ---
	if ( lastNsc && var.seek.flagNextTail ){	// NextTail�ōŏI�ʒu��else�̏����֌W�Ȃ�
	}else{
		if ( var.isScpEnableAtNsc(TargetCatType::Dst, nscNow) == false ){
			return;
		}
	}
	//--- �\���̗D�揇�ʂɂ�锻�� ---
	bool condPrior = false;
	if ( exist && !var.opt.getOpt(OptType::FlagFlat) ){
		if ( cmdset.arg.cmdsel == CmdType::Select ){
		}
		else if ( statNow > var.seek.statDst ){
			condPrior = true;		// ���̌������D��
		}
		else if ( statNow < var.seek.statDst && !lastNsc ){	// �ŏI�ʒu�͊m�舵���Œ��f���Ȃ�
			return;		// �D�揇�ʂ��Ⴂ���͂����Ńt�B���^
		}
	}
	//--- ���S����̏��g�p��(NextTail�R�}���h�p�j ---
	bool flagLogoNow = false;
	if ( var.seek.selectLogoRise ){
		int nsize = pdata->sizeClogoList();
		for(int k=0; k<nsize; k+=2){		// �����G�b�W�̂�
			Msec msecLogo = pdata->getClogoMsecFromNum(k);
			if ( abs(msecNow - msecLogo) <= pdata->msecValSpc ){
				if ( pdata->isAutoModeUse() == false ||
					 pdata->isScpChapTypeDecideFromNsc(nscNow) ){	// �m���؂莞�̂ݗD��
					flagLogoNow = true;			// ���S�����ʒu
				}
				else if ( pdata->isClogoReal() &&
					(pdata->getNscFromMsecMgn(msecNow, pdata->msecValSpc, ScpEndType::SCP_END_EDGEIN) < 0 ||
					 var.opt.getOptFlag(OptType::FlagFlat)) ){
					flagLogoNow = true;			// ���ۃ��S�g�p�ŋ߂��Ɋm���؂肪�Ȃ��������S�����ʒu
				}
			}
		}
		if ( !flagLogoNow && var.seek.flagOnLogo && !lastNsc ){
			return;		// �D�揇�ʂ��Ⴂ���͂����Ńt�B���^
		}
		if ( (flagLogoNow || lastNsc) && !var.seek.flagOnLogo ){
			condPrior = true;		// ���S�D�掞�͑��̌������D��
		}
	}
	//--- �eDst�I�v�V���������Ɣ�r���čœK�ʒu���擾 ---
	bool det = false;
	int nsize = var.sizeRangeDst();
	for(int k=0; k<nsize; k++){
		var.selRangeDstNum(k);	// ���I��
		if ( var.isRangeToDst(var.getRangeDstFrom(), msecNow) ){
			Msec msecGap = abs(msecNow - var.getRangeDstJust());
			if ( k < var.seek.numListDst || !exist ){	// �����^�[�Q�b�g���͐�D��
				det = true;
			}else if ( k > var.seek.numListDst ){	// ��̃^�[�Q�b�g��⏈��
				det = condPrior;
			}else if ( msecGap < var.seek.gapDst ){	// ���̏����������ł���Β��S�ɋ߂����D��
				det = true;
			}
			if ( det ){
				if ( seekTargetPointEnd(cmdset, msecNow, false) ){	// End�ʒu����
					var.seek.tgDst.tp   = TargetScpType::ScpNum;
					var.seek.tgDst.nsc  = nscNow;
					var.seek.tgDst.msec = msecNow;
					var.seek.tgDst.msbk = pdata->getMsecScpBk(nscNow);
					var.seek.numListDst = k;
					var.seek.statDst    = statNow;
					var.seek.gapDst     = msecGap;
					var.seek.flagOnLogo = flagLogoNow;
				}
			}
		}
	}
}
//--- -End�n�I�v�V�����ɊY������ʒu������ ---
bool JlsScriptLimit::seekTargetPointEnd(JlsCmdSet& cmdset, Msec msecDst, bool force){
	//--- �I�v�V�����ݒ��� ---
	bool needRange = var.isPrepEndRangeExist();	// �͈͐ݒ�
	bool needRefer = var.isPrepEndReferExist();	// �O��END�ʒu�ݒ�
	bool needMulti = var.isPrepEndMultiBase();	// ����from�ʒu
	bool fixEnd = ( !needRange && !needRefer && !needMulti );
	//--- -End�Œ�ʒu�m�莞�̐ݒ� ---
	if ( fixEnd && !force ){
		if ( var.seek.tgEnd.tp != TargetScpType::None ){	// �ݒ�ς݂Ȃ疈�񓯂��Ȃ̂ŏȗ�
			return ( var.seek.tgEnd.tp != TargetScpType::Invalid );
		}
	}
	//--- �Œ�ʒu�����̐ݒ� ---
	TargetLocInfo tgTry = var.getPrepEndAbs();
	if ( var.isPrepEndFromAbs() ){
		if ( needMulti ){	// END�������̊�ʒu�đI��
			var.getRangeDstFromForScp(tgTry.msec, tgTry.msbk, tgTry.nsc);
		}
	}else{	// End�Œ�ʒu�w��ł͂Ȃ����͌��ʈʒu���Q�ƈʒu�Ƃ���
		tgTry.tp = TargetScpType::None;
		tgTry.nsc = -1;
		tgTry.msec = msecDst;
		tgTry.msbk = msecDst;
	}
	Msec msecBefore = tgTry.msec;	// -EndAnd�g�p���̂��߂ɕۊ�
	//--- ����END�ʒu�̎Q�Ɛݒ� ---
	if ( needRefer ){
		seekTargetPointEndRefer(tgTry, cmdset, tgTry.msec);
	}
	//--- �����V�[���`�F���W�ʒu��͈͌������Ȃ��ꍇ�̊m�菈�� ---
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	if ( !needRange || tgTry.tp == TargetScpType::Invalid ){
		bool zone = var.isRangeToEndZone(msecDst, tgTry.msec);	// Zone�m�F
		bool valid = zone;
		if ( valid ){		// �����V�[���`�F���W��End�ʒu����������
			valid = var.isScpEnableAtMsec(tgTry.msec, LOGO_EDGE_RISE, TargetCatType::End);
		}
		if ( force ){
			if ( zone && pdata->isRangeInTotalMax(tgTry.msec) ){
				if ( tgTry.tp != TargetScpType::ScpNum ){
					tgTry.tp = TargetScpType::Force;
				}
			}else{
				tgTry.tp = TargetScpType::Invalid;
			}
		}else if ( !valid ){
			tgTry.tp = TargetScpType::Invalid;
		}else if ( fixpos ){
			tgTry.tp = TargetScpType::Direct;
		}
		valid = ( tgTry.tp != TargetScpType::Invalid );
		if ( valid || var.seek.tgEnd.tp == TargetScpType::None ){
			var.seek.tgEnd = tgTry;		// End�ʒu�X�V
		}
		return valid;
	}
	//--- �͈͂��疳���V�[���`�F���W�ʒu��I�� ---
	bool exist = false;
	if ( !fixpos ){
		Msec msecIn = tgTry.msec;	// End�����̋N�_
		Nsc  nscAnd = -1;			// -EndNext* + -EndAnd �ɂ�����
		if ( var.opt.getOptFlag(OptType::FlagEndAnd) ){		// -EndAnd
			msecIn = msecBefore;
			nscAnd = tgTry.nsc;
		}
		exist = seekTargetPointEndScp(cmdset, msecIn, msecDst, nscAnd);
	}
	//--- -End�n�ʒu�ɊY���Ȃ��ꍇ�̏��� ---
	if ( !exist ){
		if ( force || var.seek.tgEnd.tp == TargetScpType::None ){
			Msec msecLen = var.getPrepEndRangeForceLen();
			var.seek.tgEnd.tp   = TargetScpType::Invalid;
			var.seek.tgEnd.nsc  = -1;
			var.seek.tgEnd.msec = tgTry.msec + msecLen;
			var.seek.tgEnd.msbk = tgTry.msbk + msecLen;
			var.seek.numListEnd = 0;
			var.seek.statEnd    = ScpPriorType::SCP_PRIOR_NONE;
			var.seek.gapEnd     = 0;
			bool zone = var.isRangeToEndZone(msecDst, var.seek.tgEnd.msec);	// Zone�m�F
			if ( zone ){
				if ( force && pdata->isRangeInTotalMax(var.seek.tgEnd.msec) ){
					var.seek.tgEnd.tp = TargetScpType::Force;
				}else if ( fixpos ){
					var.seek.tgEnd.tp = TargetScpType::Direct;
				}
			}
		}
	}
	return ( var.seek.tgEnd.tp != TargetScpType::Invalid );
}
//--- ����END�ʒu�̎Q�Ɛݒ� ---
void JlsScriptLimit::seekTargetPointEndRefer(TargetLocInfo& tgEnd, JlsCmdSet& cmdset, Msec msecIn){
	Nsc  nscEnd = -1;
	Msec msecEnd = msecIn;
	Msec msbkEnd = msecIn;
	bool exist = false;
	{
		bool existNextL = cmdset.arg.isSetOpt(OptType::NumEndNextL);	// -EndNextL
		bool existPrevL = cmdset.arg.isSetOpt(OptType::NumEndPrevL);	// -EndPrevL
		if ( existNextL || existPrevL ){
			exist = true;
			int numArg = 0;
			if ( existNextL ) numArg = cmdset.arg.getOpt(OptType::NumEndNextL);
			if ( existPrevL ) numArg = cmdset.arg.getOpt(OptType::NumEndPrevL) * -1;
			int locLogo;
			if ( numArg > 0 ){
				locLogo = pdata->getClogoNumNextCount(msecEnd, abs(numArg));
			}else if ( numArg < 0 ){
				locLogo = pdata->getClogoNumPrevCount(msecEnd, abs(numArg));
			}else{
				locLogo = pdata->getClogoMsecNear(msecEnd, LOGO_EDGE_BOTH);
			}
			WideMsec wmsecLogo = pdata->getClogoWmsecFromNum(locLogo);
			msecEnd = wmsecLogo.late;
			msbkEnd = wmsecLogo.early;
			nscEnd = pdata->getClogoNsc(msecEnd);
		}
	}
	{
		bool existNextC = cmdset.arg.isSetOpt(OptType::NumEndNextC); 	// -EndNextC
		bool existPrevC = cmdset.arg.isSetOpt(OptType::NumEndPrevC); 	// -EndPrevC
		if ( existNextC || existPrevC ){
			exist = true;
			int numArg = 0;
			if ( existNextC ) numArg = cmdset.arg.getOpt(OptType::NumEndNextC);
			if ( existPrevC ) numArg = cmdset.arg.getOpt(OptType::NumEndPrevC) * -1;
			if ( numArg > 0 ){
				nscEnd = pdata->getNscNextScpDispFromMsecCount(msecEnd, abs(numArg), true);
			}else if ( numArg < 0 ){
				nscEnd = pdata->getNscPrevScpDispFromMsecCount(msecEnd, abs(numArg), true);
			}else{
				nscEnd = pdata->getNscFromMsecDisp(msecEnd, pdata->getMsecTotalMax(), SCP_END_EDGEIN);
			}
		}
	}
	if ( !exist ) return;
	//--- �X�V ---
	if ( nscEnd >= 0 ){
		tgEnd.tp   = TargetScpType::ScpNum;
		tgEnd.nsc  = nscEnd;
		tgEnd.msec = pdata->getMsecScp(nscEnd);
		tgEnd.msbk = pdata->getMsecScpBk(nscEnd);
	}else{
		tgEnd.tp   = TargetScpType::Direct;
		tgEnd.nsc  = -1;
		tgEnd.msec = msecEnd;
		tgEnd.msbk = msbkEnd;
		if ( msecEnd < 0 ){
			tgEnd.tp   = TargetScpType::Invalid;
		}
	}
}
//--- END�ʒu��͈͂��疳���V�[���`�F���W�ʒu��I�� ---
bool JlsScriptLimit::seekTargetPointEndScp(JlsCmdSet& cmdset, Msec msecIn, Msec msecDst, Nsc nscAnd){
	//--- �e�����V�[���`�F���W�ʒu�ŏ����ɍ�����ԋ߂������擾 ---
	bool exist = false;
	int nsize = var.sizePrepEndRange();
	int nCheckMax = nsize - 1;		// �ŏ��͑S�I�v�V��������
	RangeNsc rnsc = var.seek.rnscScp;
	for(int j=rnsc.st; j<=rnsc.ed; j++){
		//--- �����𖞂����Ȃ���Ύ��̈ʒu�� ---
		if ( var.isScpEnableAtNsc(TargetCatType::End, j) == false ){
			continue;
		}
		if ( nscAnd >= 0 && nscAnd != j ){		// -EndAnd�̎��͑Ώۈʒu(nscAnd)�Ɍ���
			continue;
		}
		ScpPriorType statNow = pdata->getPriorScp(j);
		bool condPrior = false;
		if ( exist && !var.opt.getOptFlag(OptType::FlagFlat) ){
			if ( statNow < var.seek.statEnd ){	// �\���̗D�揇�ʂ������Ⴂ�ʒu�̓`�F�b�N�ȗ�
				continue;
			}else if ( statNow > var.seek.statEnd ){	// �D�揇�ʂ�������
				condPrior = true;
			}
		}
		//--- �eEnd�I�v�V���������Ɣ�r���čœK�ʒu���擾 ---
		Msec msecNow = pdata->getMsecScp(j);
		bool det = false;
		for(int k=0; k<=nCheckMax; k++){
			WideMsec wmsecEnd = var.getPrepEndRangeWithOffset(k, msecIn);
			if ( var.isRangeToEnd(msecDst, msecNow, wmsecEnd) ){
				Msec msecGap = abs(msecNow - wmsecEnd.just);
				if ( k < nCheckMax || !exist || condPrior ){
					det = true;
				}else if ( msecGap < var.seek.gapEnd ){
					det = true;
				}
				if ( det ){
					exist   = true;
					nCheckMax = k;		// ���o�σI�v�V��������̌��͈ȍ~��������
					var.seek.tgEnd.tp   = TargetScpType::ScpNum;
					var.seek.tgEnd.nsc  = j;
					var.seek.tgEnd.msec = msecNow;
					var.seek.tgEnd.msbk = pdata->getMsecScpBk(j);
					var.seek.numListEnd = k;
					var.seek.statEnd    = statNow;
					var.seek.gapEnd     = msecGap;
					break;
				}
			}
		}
	}
	return exist;
}
//---------------------------------------------------------------------
// �����J�n�O�̏���
//---------------------------------------------------------------------
void JlsScriptLimit::prepTargetPoint(JlsCmdSet& cmdset){
	//--- End�ʒu���� ---
	prepTargetPointEnd(cmdset);
}
//--- END�ʒu�ݒ� ---
void JlsScriptLimit::prepTargetPointEnd(JlsCmdSet& cmdset){
	//--- ������ ---
	var.clearPrepEnd();
	//--- �}�[�W���ݒ� ---
	Msec msecMgnSpc   = pdata->msecValSpc;		// �����p�f�t�H���g�}�[�W��
	if ( cmdset.arg.isSetOpt(OptType::MsecEmargin) ){	// �}�[�W���ݒ肠��ΕύX
		msecMgnSpc   = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
	}
	//--- EndSft�w�� ---
	bool existEndSft = false;
	WideMsec wmsecEndSft;
	if ( cmdset.arg.isSetOpt(OptType::MsecEndSftC) ){			// -EndSft
		existEndSft = true;
		wmsecEndSft.just  = cmdset.arg.getOpt(OptType::MsecEndSftC);
		wmsecEndSft.early = cmdset.arg.getOpt(OptType::MsecEndSftL);
		wmsecEndSft.late  = cmdset.arg.getOpt(OptType::MsecEndSftR);
	}
	//--- End�͈͎w�� ---
	bool existEndLen = false;
	if ( cmdset.arg.isSetOpt(OptType::MsecEndlenC) &&			// -EndLen�ݒ肠��
	    ( cmdset.arg.getOpt(OptType::MsecEndlenC) != 0 ||		// -EndLen 0�ȊO
	      cmdset.arg.getOptFlag(OptType::FlagReset) == false )){		// -reset�Ȃ�
		existEndLen = true;
		WideMsec wmsec;
		wmsec.just  = cmdset.arg.getOpt(OptType::MsecEndlenC);
		wmsec.early = cmdset.arg.getOpt(OptType::MsecEndlenL);
		wmsec.late  = cmdset.arg.getOpt(OptType::MsecEndlenR);
		if ( existEndSft ){
			wmsec.just  += wmsecEndSft.just;
			wmsec.early += wmsecEndSft.early;
			wmsec.late  += wmsecEndSft.late;
		}
		var.addPrepEndRange(wmsec);	// End�ݒ胊�X�g�ǉ�
	}
	//--- End�͈̓��X�g�w�� ---
	if ( cmdset.arg.isSetStrOpt(OptType::ListTgEnd) ){	// -EndList
		existEndLen = true;
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListTgEnd);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			for(int i=0; i<(int)listMsec.size(); i++){
				WideMsec wmsec;
				if ( existEndSft ){
					wmsec.just  = listMsec[i] + wmsecEndSft.just;
					wmsec.early = listMsec[i] + wmsecEndSft.early;
					wmsec.late  = listMsec[i] + wmsecEndSft.late;
				}else{
					wmsec.just  = listMsec[i];
					wmsec.early = listMsec[i] - msecMgnSpc;
					wmsec.late  = listMsec[i] + msecMgnSpc;
				}
				var.addPrepEndRange(wmsec);	// End�ݒ胊�X�g�ǉ�
			}
		}
	}
	//--- EndSft�w��i�ۑ��j ---
	if ( existEndSft && !var.isPrepEndRangeExist() ){
		var.addPrepEndRange(wmsecEndSft);	// EndSft�ݒ胊�X�g�ǉ�
	}
	//--- End�O��ʒu�w�� ---
	bool existEndRefer = false;
	if ( cmdset.arg.isSetOpt(OptType::NumEndNextL) ||	// -EndNextL
		 cmdset.arg.isSetOpt(OptType::NumEndNextC) ){	// -EndNextC
		existEndRefer = true;
	}
	var.setPrepEndRefer(existEndRefer);
	//--- �Œ��END�ʒu�擾 ---
	TargetLocInfo tgEnd;
	bool multiBase;
	bool fromAbs = prepTargetPointEndAbs(tgEnd, multiBase, cmdset);
	if ( !existEndLen && !existEndRefer ){
		fromAbs = true;	// ����-End�n�I�v�V�����Ȃ���Ί�ʒu��End�ɂ���
	}
	var.setPrepEndAbs(fromAbs, multiBase, tgEnd);
}
//--- �Œ��END�ʒu�擾 ---
bool JlsScriptLimit::prepTargetPointEndAbs(TargetLocInfo& tgEnd, bool& multiBase, JlsCmdSet& cmdset){
	//--- �}�[�W���ݒ� ---
	Msec msecMgnSpc   = pdata->msecValSpc;		// �����p�f�t�H���g�}�[�W��
	Msec msecMgnExact = pdata->msecValExact;	// ���ڃt���[���ʒu�w�莞�͍ŏ����͈̔�
	if ( cmdset.arg.isSetOpt(OptType::MsecEmargin) ){	// �}�[�W���ݒ肠��ΕύX
		msecMgnSpc   = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
		msecMgnExact = abs(cmdset.arg.getOpt(OptType::MsecEmargin));
	}
	//--- �Œ�END�ʒu ---
	bool useBase = false;
	tgEnd.nsc  = -1;
	tgEnd.msec = -1;
	tgEnd.msbk = -1;
	bool fromAbs = false;
	Msec msecMgn = msecMgnSpc;
	if ( cmdset.arg.isSetStrOpt(OptType::ListEndAbs) ){	// -EndAbs
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListEndAbs);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			int nlist = var.getLogoBaseListNum();
			if ( nlist < 0 ) nlist = 0;
			if ( nlist >= nsize ) nlist = nsize-1;
			tgEnd.msec = listMsec[nlist];
		}
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetStrOpt(OptType::ListAbsSetFE) ){	// -AbsSetFE
		vector<Msec> listMsec;
		string strList = cmdset.arg.getStrOpt(OptType::ListAbsSetFE);
		if ( pdata->cnv.getListValMsecM1(listMsec, strList) ){
			int nsize = (int) listMsec.size();
			if ( nsize < 2 ) return false;		// �K��2�f�[�^�Z�b�g
			int nlist = var.getLogoBaseListNum();
			int ned = nlist * 2 + 1;
			if ( ned <= 0 ) ned = 1;
			if ( ned >= nsize ) ned = nsize-1;
			tgEnd.msec = listMsec[ned];
		}
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndHead) ){	// -EndHead
		tgEnd.msec = cmdset.limit.getHead();
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndTail) ){	// -EndTail
		tgEnd.msec = cmdset.limit.getTail();
		fromAbs = true;
	}
	else if ( cmdset.arg.isSetOpt(OptType::FlagEndHold) ){	// -EndHold
		string strVal = cmdset.arg.getStrOpt(OptType::StrValPosR);	// $POSHOLD
		pdata->cnv.getStrValMsecM1(tgEnd.msec, strVal, 0);
		tgEnd.msbk = tgEnd.msec;
		msecMgn = msecMgnExact;
		fromAbs = true;
	}
	else if ( cmdset.arg.getOptFlag(OptType::FlagEndBaseL) ){	// -EndBaseL
		fromAbs = true;
		Nrf nrfBase = cmdset.limit.getLogoBaseNrf();
		Nrf nscBase = cmdset.limit.getLogoBaseNsc();
		if ( nrfBase >= 0 ){
			tgEnd.msec = pdata->getMsecLogoNrf(nrfBase);
			tgEnd.msbk = tgEnd.msec;
			tgEnd.nsc  = pdata->getClogoNsc(tgEnd.msec);
		}else if ( nscBase >= 0 ){
			tgEnd.nsc  = cmdset.limit.getLogoBaseNsc();
			tgEnd.msec = pdata->getMsecScp(tgEnd.nsc);
			tgEnd.msbk = pdata->getMsecScpBk(tgEnd.nsc);
		}else{
			msecMgn    = msecMgnExact;
			useBase    = true;
			multiBase  = var.isRangeDstMultiFrom();
		}
	}
	else{
		if ( cmdset.arg.getOptFlag(OptType::FlagEndBase) ){	// -EndBase
			fromAbs = true;
		}
		msecMgn    = msecMgnExact;
		useBase    = true;
		multiBase  = var.isRangeDstMultiFrom();
	}
	//--- ��ʒu(from)�w�莞 ---
	if ( useBase ){
		var.getRangeDstFromForScp(tgEnd.msec, tgEnd.msbk, tgEnd.nsc);
	}
	//--- �␳ ---
	if ( tgEnd.msbk < 0 ){
		tgEnd.msbk = tgEnd.msec;
	}
	bool fixpos = var.opt.getOptFlag(OptType::FlagFixPos);
	tgEnd.exact = fixpos;
	if ( tgEnd.nsc >= 0 ){
		if ( fixpos ){
			tgEnd.tp = TargetScpType::Force;
		}else{
			tgEnd.tp = TargetScpType::ScpNum;
		}
	}else if ( !pdata->isRangeInTotalMax(tgEnd.msec) ){
		if ( fixpos ){
			tgEnd.tp = TargetScpType::Direct;
		}else{
			tgEnd.tp = TargetScpType::Invalid;
		}
	}else if ( fixpos ){
		tgEnd.tp = TargetScpType::Force;
	}else{
		tgEnd.nsc = pdata->getNscFromMsecMgn(tgEnd.msec, msecMgn, SCP_END_EDGEIN);
		if ( tgEnd.nsc >= 0 ){
			tgEnd.tp = TargetScpType::ScpNum;
		}else if ( var.opt.tack.forcePos ){
			tgEnd.tp = TargetScpType::Force;
		}else{
			tgEnd.tp = TargetScpType::Direct;
		}
	}
	if ( tgEnd.nsc >= 0 ){
		tgEnd.msec = pdata->getMsecScp(tgEnd.nsc);
		tgEnd.msbk = pdata->getMsecScpBk(tgEnd.nsc);
	}
	return fromAbs;
}

