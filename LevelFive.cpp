#include "LevelFive.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "LevelThree.h"//切換到哪裡LevelFour
#include "LevelFour.h"
USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*((t)+0.5f*(dt)))  //已經經過 t 秒後，再經過 dt 時間內落下距離
#define NUMBER_PARTICLES 100
#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;

LevelFive::~LevelFive()
{

//#ifdef BOX2D_DEBUG
//	if (_DebugDraw != NULL) delete _DebugDraw;
//#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	/*SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");*/
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* LevelFive::createScene()
{
	return LevelFive::create();
}

// on "init" you need to initialize your instance
bool LevelFive::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new (std::nothrow) b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("level05.csb"); 

	//火花
	_bApplyImpulse = false;
	_contactListener._bCreateSpark= false;
	_NumOfSparks = 10;
	_tdelayTime = 20;

	//車子
	_contactListener._bDeleteCar = false;

	//切換場景
	_bSwitchScene = false;
	_bSwitchPreviousScene = false;
	//
	_contactListener._bSpeedSensor01 = false;
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);
	
#endif
	/*auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(false);*/
	addChild(_csbRoot, 1);

	createStaticBoundary();
	setupButton();
	setStaticEdge();
	setDoor();
	setDoor02();
	setDoor03();
	setDoor04();
	setDoor05();
	setDoor06();
	setDoor07();
	arriveGoal();
	setDeleteEdge();
//#ifdef BOX2D_DEBUG
//	//DebugDrawInit
//	_DebugDraw = nullptr;
//	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
//	//設定DebugDraw
//	_b2World->SetDebugDraw(_DebugDraw);
//	//選擇繪製型別
//	uint32 flags = 0;
//	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
//	flags += GLESDebugDraw::e_pairBit;
//	flags += GLESDebugDraw::e_jointBit;
//	flags += GLESDebugDraw::e_centerOfMassBit;
//	flags += GLESDebugDraw::e_aabbBit;
//	//設定繪製類型
//	_DebugDraw->SetFlags(flags);
//#endif
	//加入 CLevelFiveContactListener
	_b2World->SetContactListener(&_contactListener);
	//
	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(LevelFive::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(LevelFive::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(LevelFive::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(LevelFive::update));

	return true;
}


void LevelFive::setupButton() {
	//新車子
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rectFrame_btn"));
	_rectButton = CButton::create();
	_rectButton->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_rectButton->setScale(btnSprite->getScale());
	this->addChild(_rectButton, 3);
	btnSprite->setVisible(false);


	//切換場景
	auto sceneBtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("switchScene_btn"));
	_switchSceneButton = CButton::create();
	_switchSceneButton->setButtonInfo("rightarrow.png", "rightarrowon.png", sceneBtnSprite->getPosition());
	_switchSceneButton->setScale(sceneBtnSprite->getScale());
	this->addChild(_switchSceneButton, 3);
	sceneBtnSprite->setVisible(false);
	//切換上一個場景
	auto previousSceneBtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("switchScenePrevious_btn"));
	_switchPreviousSceneButton = CButton::create();
	_switchPreviousSceneButton->setButtonInfo("leftarrow.png", "leftarrowon.png", previousSceneBtnSprite->getPosition());
	_switchPreviousSceneButton->setScale(previousSceneBtnSprite->getScale());
	this->addChild(_switchPreviousSceneButton, 3);
	previousSceneBtnSprite->setVisible(false);

}
void LevelFive::setupBall() {
	auto appearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("appearPoint"));
	Point appearPoint = appearSprite->getPosition();
	appearSprite->setVisible(false);
	//球
	auto ballSprite = Sprite::createWithSpriteFrameName("dount02.png");
	ballSprite->setScale(0.6f);
	this->addChild(ballSprite, 2);
	Size ballSize = ballSprite->getContentSize();

	_ballDynamicBody = { nullptr };
	b2BodyDef ballBodyDef;
	ballBodyDef.type = b2_dynamicBody;
	ballBodyDef.userData = ballSprite;
	ballBodyDef.position.Set(appearPoint.x / PTM_RATIO, appearPoint.y / PTM_RATIO);
	_ballDynamicBody = _b2World->CreateBody(&ballBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = (ballSize.width - 4) * 0.35f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 2.0f;
	fixtureDef.friction = 0.1f;
	_ballDynamicBody->CreateFixture(&fixtureDef);

}
void LevelFive::setStaticEdge() {

	// 產生 EdgeShape 的 body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &edgeShape;

	std::ostringstream ostr;
	std::string objname;
	for (int i = 0; i < 11; i++) {
		ostr.str("");
		ostr << "bar"; ostr.width(2); ostr.fill('0'); ostr << i + 1; objname = ostr.str();
		auto edgeSprite = _csbRoot->getChildByName(objname);

		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();  // 相對於中心點的位置

		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep1, lep2, wep1, wep2; // EdgeShape 的兩個端點 // world edge point1 & point2 
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;  // local edge point1 & point2
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // 先設定 X 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix); //  modelMatrix = rotMatrix*modelMatrix
		modelMatrix.m[3] = loc.x; //設定 Translation，自己的加上父親的, (pntLoc = csbRoot->getPosition())
		modelMatrix.m[7] = loc.y; //設定 Translation，自己的加上父親的

			// 產生兩個端點
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		// bottom edge
		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}
}
void LevelFive::setDoor()
{
	// 取得並設定 doorPoint 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint01"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.1f / PTM_RATIO, size.height * 0.1f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door01"));
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scaleX = circleSprite->getScaleX();	
	float scaleY = circleSprite->getScaleY();	

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO* scaleX, size.height * 0.5f / PTM_RATIO* scaleY);
	
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = false;
	dynamicBody->CreateFixture(&fixtureDef);

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(1.0f / PTM_RATIO,0 ));
	_b2World->CreateJoint(&JointDef);

}
void LevelFive::setDoor02()
{
	// 取得並設定 doorPoint 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint02"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.1f / PTM_RATIO, size.height * 0.1f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door02"));
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scaleX = circleSprite->getScaleX();
	float scaleY = circleSprite->getScaleY();

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = false;
	dynamicBody->CreateFixture(&fixtureDef);

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(1.0f / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef);
}
void LevelFive::setDoor03() {
	// 取得並設定 doorPoint 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint03"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.1f / PTM_RATIO, size.height * 0.1f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door03"));
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scaleX = circleSprite->getScaleX();
	float scaleY = circleSprite->getScaleY();

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = false;
	dynamicBody->CreateFixture(&fixtureDef);

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(1.0f / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef);
}
void LevelFive::setDoor04() {
	// 取得並設定 doorPoint 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint04"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.1f / PTM_RATIO, size.height * 0.1f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door04"));
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scaleX = circleSprite->getScaleX();
	float scaleY = circleSprite->getScaleY();

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = false;
	dynamicBody->CreateFixture(&fixtureDef);

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);
}
void LevelFive::setDoor05() {
	// 取得並設定 doorPoint 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint05"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.1f / PTM_RATIO, size.height * 0.1f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door05"));
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scaleX = circleSprite->getScaleX();
	float scaleY = circleSprite->getScaleY();

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = false;
	dynamicBody->CreateFixture(&fixtureDef);

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);
}
void LevelFive::setDoor06() {
	// 取得並設定 doorPoint03 為【靜態物體】
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint06"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	float scaleX = frameSprite->getScaleX();
	float scaleY = frameSprite->getScaleY();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = 5 / PTM_RATIO * scaleX;
	/*b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO* scaleX, size.height * 0.5f / PTM_RATIO* scaleY);*/
	b2FixtureDef fixtureDef;

	fixtureDef.shape = &circleShape;
	staticBody->CreateFixture(&fixtureDef);

	// 取得並設定 door01 為【動態物體】
	auto boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door06"));
	loc = boxSprite->getPosition();
	size = boxSprite->getContentSize();
	scaleX = boxSprite->getScaleX();
	scaleY = boxSprite->getScaleY();
	b2PolygonShape boxShape;

	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = boxSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;

	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	dynamicBody->CreateFixture(&fixtureDef);

	////
	b2RevoluteJoint* RevJoint = { nullptr };
	b2RevoluteJointDef RJoint;
	RJoint.Initialize(staticBody, dynamicBody, staticBody->GetWorldCenter());
	RevJoint = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
}
void  LevelFive::setDoor07() {
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("sensor02"));
	Point loc = sensorSprite->getPosition();
	Size  size = sensorSprite->getContentSize();
	float scale = sensorSprite->getScale();
	b2BodyDef sensorBodyDef;
	sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	sensorBodyDef.type = b2_staticBody;

	b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox(size.width * 0.5f * scale / PTM_RATIO, size.height * 0.5f * scale / PTM_RATIO);

	b2FixtureDef SensorFixtureDef;
	SensorFixtureDef.shape = &sensorShape;
	SensorFixtureDef.isSensor = true;	// 設定為 Sensor
	SensorFixtureDef.density = 50000; // 故意設定成這個值，方便碰觸時候的判斷
	SensorBody->CreateFixture(&SensorFixtureDef);
}
void LevelFive::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	
	

}
void LevelFive::arriveGoal() {
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("sensor"));
	Point loc = sensorSprite->getPosition();
	Size  size = sensorSprite->getContentSize();
	float scale = sensorSprite->getScale();
	sensorSprite->setVisible(false);
	b2BodyDef sensorBodyDef;
	sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	sensorBodyDef.type = b2_staticBody;

	b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox(size.width * 0.5f * scale / PTM_RATIO, size.height * 0.5f * scale / PTM_RATIO);

	b2FixtureDef SensorFixtureDef;
	SensorFixtureDef.shape = &sensorShape;
	SensorFixtureDef.isSensor = true;	// 設定為 Sensor
	SensorFixtureDef.density = 10000; // 故意設定成這個值，方便碰觸時候的判斷
	SensorBody->CreateFixture(&SensorFixtureDef);

}
void LevelFive::setDeleteEdge() {


	// 產生 EdgeShape 的 body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);
	// 產生長方形靜態物體所需要的 rectShape
	b2PolygonShape rectShape;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.isSensor = true;	// 設定為 Sensor
	fixtureDef.density = 20000; // 故意設定成這個值，方便碰觸時候的判斷

	for (size_t i = 0; i < 5; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		std::ostringstream ostr;
		std::string objname;
		ostr << "edge"; ostr.width(2); ostr.fill('0'); ostr << i + 1; objname = ostr.str();
		auto const rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = rectSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

		// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };

		rectShape.Set(vecs, 4);
		body->CreateFixture(&fixtureDef);
	}
}

//void LevelFive::readBlocksCSBFile(const std::string& csbfilename)
//{
//	auto csbRoot = CSLoader::createNode(csbfilename);
//	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
//	addChild(csbRoot, 1);
//	char tmp[20] = "";
//	for (size_t i = 1; i <= 3; i++)
//	{
//		// 產生所需要的 Sprite file name int plist 
//		sprintf(tmp, "block1_%02d", i); 
//	}
//}
//
//void LevelFive::readSceneFile(const std::string &csbfilename)
//{
//	auto csbRoot = CSLoader::createNode(csbfilename);
//	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
//	addChild(csbRoot, 1);
//	char tmp[20] = "";
//	for (size_t i = 1; i <= 3; i++)
//	{
//		// 產生所需要的 Sprite file name int plist 
//		sprintf(tmp, "XXX_%02d", i);
//	}
//}

void LevelFive::update(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);
	
	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
			ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}	
	}
	//
	// 產生火花
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//產生完關閉
		// 判斷延遲的時間是否滿足
		if (_bSparking) { //可以噴發，實現這次的噴發
			_tdelayTime = 0; // 時間重新設定，
			_bSparking = false; // 開始計時
			for (int i = 0; i < _NumOfSparks; i++) {
				// 建立 Spark Sprite 並與目前的物體結合
				auto sparkSprite = Sprite::createWithSpriteFrameName("flare.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//產生小方塊資料
				auto flagSprite = _csbRoot->getChildByName("goal");
				Point _createLoc;
				_createLoc = flagSprite->getPosition();
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(_createLoc.x / PTM_RATIO, _createLoc.y / PTM_RATIO);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = sparkSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture* RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//給力量
				RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 30 + rand() % 30), b2Vec2(_createLoc.x, _createLoc.y), true);
			}
#ifdef CREATED_REMOVED
			g_totCreated += _contactListener._NumOfSparks;
			CCLOG("Creating %4d Particles", g_totCreated);
#endif
		}
	}
	if (!_bSparking) {
		_tdelayTime += dt;
		if (_tdelayTime >= 20.0f) {
			_tdelayTime = 0; // 歸零
			_bSparking = true; // 可進行下一次的噴發
		}
	}
	//碰到感應器加速
	if (_contactListener._bSpeedSensor01) {
		_contactListener._bSpeedSensor01 = false;
		_ballDynamicBody->ApplyLinearImpulse(b2Vec2(20, 20), _ballDynamicBody->GetWorldCenter(), true);
	}
	
	
	//刪除碰到三角形的車子
	if (_contactListener._bDeleteCar) {
		_bSmoke = false;//關掉煙
		_contactListener._bDeleteCar = false;
		if (_contactListener._iBodyNum == 1) {
			Sprite* spriteData = (Sprite*)(_contactListener.contactBodyB->GetUserData());
			this->removeChild(spriteData);//鋼體的Sprite
			_b2World->DestroyBody(_contactListener.contactBodyB); // 釋放目前的 body
		}
		else if(_contactListener._iBodyNum == 2) {
			Sprite* spriteData = (Sprite*)(_contactListener.contactBodyA->GetUserData());
			this->removeChild(spriteData);
			_b2World->DestroyBody(_contactListener.contactBodyA);
		}
	}
	//切換場景
	if (_bSwitchScene) {
		this->unschedule(schedule_selector(LevelFive::update));
		Director::getInstance()->replaceScene(LevelThree::createScene());//切換的場景
	}
	if (_bSwitchPreviousScene) {
		this->unschedule(schedule_selector(LevelFour::update));
		Director::getInstance()->replaceScene(LevelFour::createScene());//切換的場景
	}
}

bool LevelFive::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesBegin(touchLoc)) {
		//產生車車
		setupBall();
		
	}
	_switchSceneButton->touchesBegin(touchLoc);
	_switchPreviousSceneButton->touchesBegin(touchLoc);
	// For Mouse Joiint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理
		// 判斷點的位置是否落在動態物體一定的範圍
		if (_ballDynamicBody != NULL) {
			if (body->GetUserData() != _ballDynamicBody->GetUserData()) {

				Sprite* spriteObj = static_cast<Sprite*>(body->GetUserData());
				Size objSize = spriteObj->getContentSize();
				float fdist = MAX_2(objSize.width, objSize.height) / 2.0f;
				float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
				float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
				float tpdist = x * x + y * y;
				if (tpdist < fdist * fdist)
				{
					_bTouchOn = true;
					b2MouseJointDef mouseJointDef;
					mouseJointDef.bodyA = _bottomBody;
					mouseJointDef.bodyB = body;
					mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
					mouseJointDef.collideConnected = true;
					mouseJointDef.maxForce = 1000.0f * body->GetMass();
					_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // 新增 Mouse Joint
					body->SetAwake(true);
					break;
				}
			}
		}
		
	}
	return true;
}

void  LevelFive::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);

	_switchSceneButton->touchesMoved(touchLoc);
	_switchPreviousSceneButton->touchesMoved(touchLoc);
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

}

void  LevelFive::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesEnded(touchLoc);
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
	if (_switchSceneButton->touchesEnded(touchLoc)) {
		_bSwitchScene = true;
		log("switchScene");
	}
	if (_switchPreviousSceneButton->touchesEnded(touchLoc)) {
		_bSwitchPreviousScene = true;
		log("switchScene-p");
	}
}


CLevelFiveContactListener::CLevelFiveContactListener()
{

}
void CLevelFiveContactListener::BeginContact(b2Contact* contact)
{
	contactBodyA = contact->GetFixtureA()->GetBody();
	contactBodyB = contact->GetFixtureB()->GetBody();

	// check 是否為落下的球經過 sensor1 ，只要經過就立刻讓他彈出去
	if (contactBodyA->GetFixtureList()->GetDensity() == 10000.0f) { // 代表 sensor1
		_bCreateSpark = true;
		log("in-a");
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 10000.0f) {// 代表 sensor1
		_bCreateSpark = true;
		log("in-b");
	}
	if (contactBodyA->GetFixtureList()->GetDensity() == 20000.0f) { // 代表 sensor1
		if (contactBodyB->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 1;//刪B
			
			log("in-2a");
		}
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 20000.0f) {// 代表 sensor1
		if (contactBodyA->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 2;//刪A
;			log("in-2a");
		}
	}
	if (contactBodyA->GetFixtureList()->GetDensity() == 50000.0f) { // 代表 sensor1
		_bSpeedSensor01 = true;	
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 50000.0f) {// 代表 sensor1
		_bSpeedSensor01 = true;
	}
	
}

//#ifdef BOX2D_DEBUG
////改寫繪製方法
//void LevelFive::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
//{
//	Director* director = Director::getInstance();
//
//	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
//	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
//	_b2World->DrawDebugData();
//	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
//}
//#endif