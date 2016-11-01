#include "ofxHairDraw.h"
#define BUFFER_OFFSET(bytes) ((GLubyte *)NULL + (bytes))

ofxHairDraw::ofxHairDraw(ofxHairModel &model)
:	m_model(model)
{
}

void ofxHairDraw::init(string filename)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (!shader.load(filename)) {
		cout << "failed loading shader" << endl;
	};

	// generate OpenGL Vertex-Buffer-Object & Index-Buffer-Object
	if (mVbo)  glDeleteBuffers(1, &mVbo);
	glGenBuffers(1, &mVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexIntermediate) * m_model.mNormalVertices.size(), &m_model.mNormalVertices[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Texture */
	/* I refered from this website to implement "Vertex Texture Fetch"
		OpenGL��GPU�X�L�j���O�y���_�e�N�X�`���ҁz
		http://nullorempry.jimdo.com/2012/07/02/opengl%E3%81%A7gpu%E3%82%B9%E3%82%AD%E3%83%8B%E3%83%B3%E3%82%B0-%E9%A0%82%E7%82%B9%E3%83%86%E3%82%AF%E3%82%B9%E3%83%81%E3%83%A3%E7%B7%A8/
	*/

	#define TEXTURE_SIZE 2
	boneNum = m_model.mGuideJoints.size();

	cout << "bone Num : " << boneNum << endl;

	unsigned int ids[TEXTURE_SIZE] = { 0, 0 }; // �e�N�X�`���̈�
	ofVec4f *init_data = new ofVec4f [boneNum]; // �{�[���̏����f�[�^�z��
	translations = new ofVec4f[boneNum];
	rotations = new ofQuaternion[boneNum];
	
	// ���W�̏�����
	for (int i = 0; i < boneNum; i++)
		init_data[i].set(ofVec4f(0.0f, 0.0f, 0.0f, 1.0f));

	for (int i = 0; i < TEXTURE_SIZE; i++)
	{
		if (ids[i])  glDeleteBuffers(1, &ids[i]);

		if (i == 0)
			glActiveTexture(GL_TEXTURE0);
		else if (i == 1)
			glActiveTexture(GL_TEXTURE1);

		glGenTextures(1, &ids[i]);
		glBindTexture(GL_TEXTURE_2D, ids[i]);

		/* �t�B���^�͕�Ԃ������Ȃ� && �~�b�v�}�b�v�������Ȃ� */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_NEAREST);

		/* ���b�s���O��Clamp���[�h(0~1�ȊO�͒[�������������΂�) */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);

		/* �~�b�v�}�b�v������Ȃ� */
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

		/* �f�[�^�`���̐ݒ� */
		glTexImage2D(
			GL_TEXTURE_2D,	// �e�N�X�`���^�[�Q�b�g
			0,				// �~�b�v�}�b�v���x��
			GL_RGBA32F,		// �e�N�X�`���̃s�N�Z���t�H�[�}�b�g
			boneNum, 1,		// �e�N�X�`���̏c����(�f�[�^�̗v�f��)
			0,				// �{�[�_�[
			GL_RGBA,		// �s�N�Z���̔z��`��
			GL_FLOAT,		// 1�s�N�Z���̃f�[�^�`��
			init_data);		// �s�N�Z���f�[�^�͋�
							
		/* Uniform�ԍ��̎擾 */
		if (i == 0) {
			locate_pos = glGetUniformLocation(shader.getProgram(), "BoneTranslationTexture");	// �{�[���ړ��e�N�X�`����Uniform�ϐ��̔ԍ�
			if (locate_pos >= 0)
				glUniform1i(locate_pos, 0);
			else
				fprintf(stderr, "���j�t�H�[���ϐ� BoneTranslationTexture ��������܂���\n");
		}
		else if (i == 1) {
			locate_rot = glGetUniformLocation(shader.getProgram(), "BoneRotationTexture");		// �{�[����]�e�N�X�`����Uniform�ϐ��̔ԍ�
			if (locate_rot >= 0)
				glUniform1i(locate_rot, 1);
			else
				fprintf(stderr, "���j�t�H�[���ϐ� BoneRotationTexture ��������܂���\n");
		}
	}

	posID = ids[0]; // �{�[�����s�ړ��l�̃e�N�X�`��
	rotID = ids[1]; // �{�[����]�l�̃e�N�X�`��
	cout << "Texture Trans location : " << locate_pos << " , " << posID << endl;
	cout << "Texture Rotate location : " << locate_rot << " , " << rotID << endl;

	delete init_data;
}

void ofxHairDraw::update(ofxHairModel &model, ofEasyCam &cam)
{
}

void ofxHairDraw::draw(ofxHairModel &model, ofEasyCam &cam)
{
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* layout update */
	const int iPositionSlot = 0;
	const int iColorSlot = 1;
	const int iBoneIndicesSlot = 2;
	const int iBoneWeightsSlot = 3;

	/* uniform�ϐ����Z�b�g�A�b�v����O�ɌĂ΂��K�v������ */
	glUseProgram(shader.getProgram());

	glEnableVertexAttribArray(iPositionSlot);
	glEnableVertexAttribArray(iColorSlot);
	glEnableVertexAttribArray(iBoneIndicesSlot);
	glEnableVertexAttribArray(iBoneWeightsSlot);

	// bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);

	glVertexAttribPointer(iPositionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(VertexIntermediate), 0);
	glVertexAttribPointer(iColorSlot, 3, GL_FLOAT, GL_FALSE, sizeof(VertexIntermediate), (void*)(sizeof(ofVec3f)));
	glVertexAttribPointer(iBoneIndicesSlot, 4, GL_FLOAT, GL_FALSE, sizeof(VertexIntermediate), (void*)(6 * sizeof(GLfloat)));
	glVertexAttribPointer(iBoneWeightsSlot, 4, GL_FLOAT, GL_FALSE, sizeof(VertexIntermediate), (void*)(10 * sizeof(GLfloat)));
	shader.setUniformMatrix4f("ModelViewProjectionMatrix", cam.getModelViewProjectionMatrix());

	/* For Rendering */
	if (this->hasSkin()) {
		shader.begin();
		this->getSkinFbo().begin();
		this->getSkinFbo().activateAllDrawBuffers();
	}

	/* �ړ����̍X�V */
	int i = 0;
	for (auto &s : model.strands) {
		for (auto &p : s.m_particles) {
			ofMatrix4x4 m = p.gMat0.getInverse() * p.gMat;
			translations[i] = m.getTranslation();
			rotations[i] = m.getRotate();
			i++;
		}
	}

	// �e�N�X�`�����j�b�g�̐؂�ւ��i���s�ړ��e�N�X�`����TextureUnit1�ɓo�^�j
	glActiveTexture(GL_TEXTURE1);
	if (locate_pos != -1) {
		glUniform1i(locate_pos, GL_TEXTURE1 - GL_TEXTURE0);

		// ���݂̃��j�b�g�e�N�X�`����L��������
		glEnable(GL_TEXTURE_2D);

		/* �{�[�����s�ړ��e�N�X�`���̍X�V */
		glBindTexture(GL_TEXTURE_2D, posID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, boneNum, 1, 0, GL_RGBA, GL_FLOAT, translations);

		// ���݂̃��j�b�g�e�N�X�`���𖳌�������
		glDisable(GL_TEXTURE_2D);
	}

	// �e�N�X�`�����j�b�g�̐؂�ւ� (��]�e�N�X�`����TextureUnit2�ɓo�^)
	glActiveTexture(GL_TEXTURE2);
	if (locate_rot != -1) {
		glUniform1i(locate_rot, GL_TEXTURE2 - GL_TEXTURE0);

		/* ���݂̃��j�b�g�e�N�X�`����L�������� */
		glEnable(GL_TEXTURE_2D);

		/* �{�[����]�e�N�X�`���̍X�V */
		glBindTexture(GL_TEXTURE_2D, rotID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, boneNum, 1, 0, GL_RGBA, GL_FLOAT, rotations);

		// ���݂̃��j�b�g�e�N�X�`���𖳌�������
		glDisable(GL_TEXTURE_2D);
	}

	/* �{�[���e�N�X�`���̃T�C�Y��n�� */
	shader.setUniform2i("BoneTextureSize", boneNum, 1);

	/* drawing */
	int res = 100;
	glEnableClientState(GL_VERTEX_ARRAY); //�L����
	for (int i = 0; i < m_model.mNormalVertices.size() / res; i++) {
		glDrawArrays(GL_LINE_STRIP, i*res, res);
	}
	glDisableClientState(GL_VERTEX_ARRAY);  //������


	/* Hair Simulation */
	shader.end();

	/* clean */
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(iPositionSlot);
	glDisableVertexAttribArray(iColorSlot);
	glDisableVertexAttribArray(iBoneIndicesSlot);
	glDisableVertexAttribArray(iBoneWeightsSlot);

	glUseProgram(0);
}

void ofxHairDraw::drawOld(ofxHairModel &model)
{
	if (bEdge) {
		glLineWidth(0.1f);
		for (auto s : model.strands) {
			if (s.bGuideHair && bGuideHair) {
				glBegin(GL_LINE_STRIP);
				for (auto p : s.m_particles) {
					if (bColor) {
						ofSetColor(p.color);
					}
					else {
						ofSetColor(p.collision_color);
					}
					glVertex3f(p.position.x, p.position.y, p.position.z);
				}
				glEnd();
			}

			if (!s.bGuideHair && bNormalHair) {
				glBegin(GL_LINE_STRIP);
				for (auto p : s.m_particles) {
					if (bColor) {
						ofSetColor(p.color);
					}
					else {
						ofSetColor(p.collision_color);
					}
					glVertex3f(p.position.x, p.position.y, p.position.z);
				}
				glEnd();
			}
		}

		if (bNode) {
			ofSetColor(255);
			glPointSize(3.0);
			glBegin(GL_POINTS);
			for (auto s : model.strands) {
				for (auto p : s.m_particles) {
					ofPoint pos = p.position;
					ofSetColor(p.color);
					if (bColor) {
						ofSetColor(p.collision_color);
					}
					glVertex3f(pos.x, pos.y, pos.z);
				}
			}
			glEnd();
		}
	}
}

ofxHairDraw& ofxHairDraw::setDrawHairParticles(bool v)
{
	bNode = v;
	return *this;
}

ofxHairDraw& ofxHairDraw::setDrawHairEdges(bool v)
{
	bEdge = v;
	return *this;
}

ofxHairDraw& ofxHairDraw::setDrawHairColor(bool v)
{
	bColor = v;
	return *this;
}

ofxHairDraw& ofxHairDraw::setDrawHairNormal(bool v)
{
	bNormalHair = v;
	return *this;
}


ofxHairDraw& ofxHairDraw::setDrawHairGuide(bool v)
{
	bGuideHair = v;
	return *this;
}

std::vector<ofVec3f> ofxHairDraw::getSkinVertices() const {
	ofFloatPixels pixels;
	skinFbo->readToPixels(pixels, 0);

	std::vector<ofVec3f> data(boneNum);
	std::memcpy(&data[0], pixels.getData(), sizeof(ofVec3f) * boneNum);

	return std::move(data);
}
