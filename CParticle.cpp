#include <cmath>
#include "CParticle.h"


// 根據重力與時間計算 delta time 之間的位移差量 再轉成螢幕的位移
// 假設 2.5 秒讓一個分子從螢幕最上方移動到最下方, 也就是移動 720 PIXEL
// 720 PIXEL = 0.5*9.8*2.5*2.5 m => 1M = 23.5102 PIXEL
// 因為Y 軸往下為負, 所以重力要加上負號, 代表方向

#define FALLING_TIME 2.5f
#define MAX_HEIGHT 720.0f
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*((t)+0.5f*(dt)))  //已經經過 t 秒後，再經過 dt 時間內落下距離


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
	//pngName 貼圖名稱
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
