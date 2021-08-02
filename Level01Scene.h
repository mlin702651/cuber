#ifndef __LEVEL01_SCENE_H__
#define __LEVEL01_SCENE_H__


#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CLight.h"
#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class Level01Scene : public cocos2d::Scene
{
public:

	~Level01Scene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	// for MouseJoint
	b2Body* _bottomBody; // 底部的 edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;

	//產生車子的按鈕
	CButton* _rectButton;
	CButton* _redButton;
	CButton* _blueButton;
	CButton* _orangeButton;

	int colorType;
	// Box2D Examples
	//void readBlocksCSBFile(const std::string &);
	//void readSceneFile(const std::string &);
	void createStaticBoundary();
	void setupButton();
	void setupCar();
	void setStaticEdge();
	void setColorBar();
	void setCarColor();
	void setDoor();
#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void update(float dt);

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

	// implement the "static create()" method manually
	CREATE_FUNC(Level01Scene);
};

#endif 