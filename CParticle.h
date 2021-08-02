#ifndef __CPARTICLE_H__
#define __CPARTICLE_H__

#include "cocos2d.h"

USING_NS_CC;

class CParticle
{
private:
	cocos2d::Sprite* _Particle;	// ���l����
	cocos2d::Point  _OldPos;		// ���l�e�@�Ӧ�m//�{�b�S���Ψ� �i�H��tail
	cocos2d::Point  _Pos;			// ���l�ثe����m
	cocos2d::Point  _Direction;	// ���l�ثe���B�ʤ�V�A���V�q

	int iRotateOn;

	float _fExistTimer;
	// ��ܻP�_
	bool _bVisible;

public:
	CParticle();

	void setParticle(Point loc, cocos2d::Scene& stage);
	bool update(float dt);
	
};

#endif