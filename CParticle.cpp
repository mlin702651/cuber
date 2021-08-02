#include <cmath>
#include "CParticle.h"


// �ھڭ��O�P�ɶ��p�� delta time �������첾�t�q �A�ন�ù����첾
// ���] 2.5 �����@�Ӥ��l�q�ù��̤W�貾�ʨ�̤U��, �]�N�O���� 720 PIXEL
// 720 PIXEL = 0.5*9.8*2.5*2.5 m => 1M = 23.5102 PIXEL
// �]��Y �b���U���t, �ҥH���O�n�[�W�t��, �N���V

#define FALLING_TIME 2.5f
#define MAX_HEIGHT 720.0f
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*((t)+0.5f*(dt)))  //�w�g�g�L t ���A�A�g�L dt �ɶ������U�Z��


USING_NS_CC;

CParticle::CParticle()
{
	_fExistTimer = 0;
}


bool CParticle::update(float dt)
{
	log("CParticle::update");
	if (_fExistTimer > 2) {
		_Particle->setVisible(false);
	}
	else {
		_fExistTimer += dt;
		_Pos.y += 1;
	}
	return true;
}
void CParticle::setParticle(Point loc,cocos2d::Scene& stage) {
	//pngName �K�ϦW��
	_Particle = Sprite::createWithSpriteFrameName("flare.png");
	_Pos = loc;
	_Particle->setPosition(loc.x, loc.y);
	_Particle->setOpacity(255);
	_Particle->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
	_bVisible = false;
	_Particle->setVisible(false);
	BlendFunc blendfunc = { GL_SRC_ALPHA, GL_ONE };
	_Particle->setBlendFunc(blendfunc);
	stage.addChild(_Particle, 10);
}
