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
	cocos2d::Sprite* _targetSprite; // �Ω�P�_�O�_
	bool _bCreateSpark;		// ���ͤ���
	bool _bApplyImpulse;	// �����������ĤO
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	bool _bDeleteCar;

	bool _door04Sensor;
	//�R���l�u
	bool _bDeleteBullet;
	//�I������Ӫ���
	b2Body* contactBodyA;
	b2Body* contactBodyB;
	b2Body* deleteBody;
	int _iBodyNum;
	CLevelTwoContactListener();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
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
	b2Body* _bottomBody; // ������ edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;
	//����
	/*bool _bCreateSpark;*/
	float _tdelayTime; // �Ω���᪺���͡A���n�ƥ�i�J�Ӧh�ӾɭP�@�U���͹L�h������
	bool  _bSparking;  // true: �i�H�Q�X����Afalse: ����	
	int  _NumOfSparks;
	bool _bApplyImpulse;

	//�I��
	CLevelTwoContactListener _contactListener;

	//���Ͳy�����s
	CButton* _rectButton;
	b2Body* _ballDynamicBody;

	//��������
	CButton* _switchSceneButton;
	bool _bSwitchScene;
	CButton* _switchPreviousSceneButton;
	bool _bSwitchPreviousScene;

	//�Ĥ@�Ӿ���
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

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //Ĳ�I�����ƥ� 

	// implement the "static create()" method manually
	CREATE_FUNC(LevelTwo);
};

#endif // __JointScene_SCENE_H__

