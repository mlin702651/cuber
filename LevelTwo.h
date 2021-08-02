#ifndef __LEVELTWO_SCENE_H__
#define __LEVELTWO_SCENE_H__

//#define BOX2D_DEBUG 1

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
class CLevelTwoContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _targetSprite; // 用於判斷是否
	bool _bCreateSpark;		// 產生火花
	bool _bApplyImpulse;	// 產生瞬間的衝力
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	bool _bDeleteCar;

	bool _door04Sensor;
	//刪除子彈
	bool _bDeleteBullet;
	//碰撞的兩個物體
	b2Body* contactBodyA;
	b2Body* contactBodyB;
	b2Body* deleteBody;
	int _iBodyNum;
	CLevelTwoContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	/*virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);*/
};
class LevelTwo : public cocos2d::Scene
{
public:

	~LevelTwo();
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
	//火花
	/*bool _bCreateSpark;*/
	float _tdelayTime; // 用於火花的產生，不要事件進入太多而導致一下產生過多的火花
	bool  _bSparking;  // true: 可以噴出火花，false: 不行	
	int  _NumOfSparks;
	bool _bApplyImpulse;

	//碰撞
	CLevelTwoContactListener _contactListener;

	//產生球的按鈕
	CButton* _rectButton;
	b2Body* _ballDynamicBody;

	//切換場景
	CButton* _switchSceneButton;
	bool _bSwitchScene;
	CButton* _switchPreviousSceneButton;
	bool _bSwitchPreviousScene;

	//第一個機關
	CButton* _door01Button;
	b2Body* _bulletDynamicBody;
	
	// Box2D Examples
	//void readBlocksCSBFile(const std::string &);
	//void readSceneFile(const std::string &);
	void createStaticBoundary();
	void setupButton();
	void setStaticEdge();
	void setDoor01();
	void setDoor02();
	void setDoor03();
	void setDoor04();
	void arriveGoal();
	void setDeleteEdge();
	void setupBall();
	void setupBullet();
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
	CREATE_FUNC(LevelTwo);
};

#endif // __JointScene_SCENE_H__

