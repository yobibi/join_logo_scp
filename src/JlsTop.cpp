// �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "JlsIF.hpp"

//---------------------------------------------------------------------
// join_logo_scp
// �Ԃ�l�F
//  0 : ����I��
//  1 : �������s���Ȃ��ŏI��
//  2 : �ݒ�E�t�@�C���G���[
//  3 : ���s�G���[
//---------------------------------------------------------------------
int main(int argc, char *argv[])
{
	JlsIF jls;
	int ret = jls.start(argc, argv);		// �J�n

	return ret;
}
