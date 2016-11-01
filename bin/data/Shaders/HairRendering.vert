#version 400

// �萔
const vec4 pl = vec4(1.0, 6.0, 4.0, 1.0);	// �����ʒu

// ���_����
layout (location = 0) in vec4 position;

// uniform�ϐ�
uniform mat4 mc;					// �r���[�v���W�F�N�V�����ϊ��s��
uniform samplerbuffer neighbor;		// �ߖT�̐ړ_���擾����e�N�X�`���o�b�t�@�I�u�W�F�N�g
uniform ivec2 endpoint;				// ��{�̔��̖т̍ŏ��ƍŌ�̐ړ_�̃C���f�b�N�X

// fragment shader�ɑ���ړ_�̎����E�����E�ڐ��x�N�g��
out vec3 v, l, t;

void main()
{
	// �X�N���[�����W�n�̍��W�l
	gl_Position = mc * vec4(src_position, 0, 1);

	// �����x�N�g���͒��_��̋t�x�N�g��
	v = - normalize(position.xyz);

	// �����x�N�g���͒��_���獂���Ɍ������x�N�g��
	l = normalize((pl - position * pl.w).xyz):

	// �ڐ��x�N�g���͏����Ώۂ̐ړ_�̑O�̐ړ_���玟�̐ړ_�Ɍ������x�N�g��
	t = normalize((texelFetch(neighbor, max(gl_VertexID - 1, endpoint.s)) -
				   texelFetch(neighbor, min(gl_VertexID + 1, endpoint.t))).xyz)

}