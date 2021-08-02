#ifndef __CPARTICLE_H__
#define __CPARTICLE_H__

#include "cocos2d.h"

USING_NS_CC;

class CParticle
{
private:
	cocos2d::Sprite* _Particle;	// 分子本體
	cocos2d::Point  _OldPos;		// 分子前一個位置//現在沒有用到 可以做tail
	cocos2d::Point  _Pos;			// 分子目前的位置
	cocos2d::Point  _Direction;	// 分子目前的運動方向，單位向量

	int iRotateOn;

	float _fExistTimer;
	// 顯示與否
	bool _bVisible;

public:
	CParticle();

	void setParticle(Point loc, cocos2d::Scene& stage);
	bool update(float dt);
	
};

#endif