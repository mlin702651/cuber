#include "Level01Scene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#ifndef MAX_CIRCLE_OBJECTS
#define MAX_CIRCLE_OBJECTS  11
#endif
Color3B CarColor[3] = { Color3B(208,45,45), Color3B(77,204,42), Color3B(14,201,220) };
Vec2 appearPos =Vec2(1000.0f / PTM_RATIO, 1000.0f / PTM_RATIO);

using namespace cocostudio::timeline;

Level01Scene::~Level01Scene()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	//	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level01Scene::createScene()
{
	return Level01Scene::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool Level01Scene::init()
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
	_csbRoot = CSLoader::createNode("level01.csb");

#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	createStaticBoundary();
	setupButton();
	setupCar();
	setStaticEdge();
	/*setColorBar();*/
	setDoor();
#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level01Scene::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level01Scene::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level01Scene::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level01Scene::update));

	return true;
}


void Level01Scene::update(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	//// 取得 _b2World 中所有的 body 進行處理
	//// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	//for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	//{
	//	// 以下是以 Body 有包含 Sprite 顯示為例
	//	if (body->GetUserData() != NULL)
	//	{
	//		Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
	//		ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
	//		ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
	//	}
	//}




}

void Level01Scene::setupButton() {
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rectFrame_btn"));
	_rectButton = CButton::create();
	_rectButton->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_rectButton->setScale(btnSprite->getScale());
	this->addChild(_rectButton, 3);
	btnSprite->setVisible(false);
}
void Level01Scene::setupCar() {
	auto appearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("appearPoint"));
	Point appearPoint= appearSprite->getPosition();

	//車子
	auto carSprite = Sprite::createWithSpriteFrameName("car.png");
	Point carLoc = appearPoint;
	Size carSize = carSprite->getContentSize();
	b2Body* carDynamicBody = { nullptr };
	b2BodyDef carBodyDef;
	carBodyDef.type = b2_dynamicBody;
	carBodyDef.userData = carSprite;
	carBodyDef.position.Set(appearPoint.x / PTM_RATIO, appearPoint.y / PTM_RATIO);
	carDynamicBody = _b2World->CreateBody(&carBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(carSize.width * 0.3f / PTM_RATIO, carSize.height * 0.3f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	carDynamicBody->CreateFixture(&fixtureDef);

	//輪子
	Sprite* wheelSprite[2] = { nullptr };
	Point wheelLoc[2];
	Size wheelSize;
	b2Body* wheelDynamicBody[2] = { nullptr };
	b2BodyDef wheelBodyDef[2];

	for (int i = 0; i < 2; i++) {
		/*std::ostringstream ostr;
		std::string objname;
		ostr << "wheel_"; ostr << i + 1; objname = ostr.str();*/
		wheelSprite[i] = Sprite::createWithSpriteFrameName("wheel.png");
		/*wheelSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));*/
		wheelSize = wheelSprite[i]->getContentSize();
		wheelBodyDef[i].type = b2_dynamicBody;
		wheelBodyDef[i].userData = wheelSprite[i];
		wheelBodyDef[i].position.Set((carLoc.x - 10) / PTM_RATIO, (carLoc.y - 10) / PTM_RATIO);
		wheelDynamicBody[i] = _b2World->CreateBody(&wheelBodyDef[i]);
	}

	b2CircleShape circleShape;
	circleShape.m_radius = (wheelSize.width - 8) * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	wheelDynamicBody[0]->CreateFixture(&fixtureDef);
	wheelDynamicBody[1]->CreateFixture(&fixtureDef);

	b2RevoluteJointDef revjoint;

	revjoint.bodyA = carDynamicBody;
	revjoint.localAnchorA.Set(-0.7, -0.7);
	revjoint.bodyB = wheelDynamicBody[0];
	_b2World->CreateJoint(&revjoint);
	revjoint.bodyA = carDynamicBody;
	revjoint.localAnchorA.Set(0.9, -0.7);
	revjoint.bodyB = wheelDynamicBody[1];
	_b2World->CreateJoint(&revjoint);
	
}
void Level01Scene::setStaticEdge() {

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
	for (int i = 0; i < 3; i++) {
		ostr.str("");
		ostr << "bar0" << i+1; objname = ostr.str();
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
void Level01Scene::setColorBar() {
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

	
	auto edgeSprite = _csbRoot->getChildByName("door01");
	/*edgeSprite->setColor(CarColor[0]);*/
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
	/*fixtureDef.filter.categoryBits = 1 << 1;*/
	log("%d",fixtureDef.filter.categoryBits);
	body->CreateFixture(&fixtureDef);
	
}
void Level01Scene::setDoor()
{
	auto doorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door01"));
	Point loc = doorSprite->getPosition();
	Size size = doorSprite->getContentSize();
	float scale = doorSprite->getScale();
	b2PolygonShape rectShape;
	rectShape.SetAsBox((size.width - 4) * 0.5f / PTM_RATIO, (size.height - 4) * 0.5f / PTM_RATIO);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = doorSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	dynamicBody->CreateFixture(&fixtureDef);

	// 取得並設定 frame01_pri 畫框圖示為【靜態物體】
	loc.y += 200;
	log("%f", loc.y);
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);


	

	//產生垂直 Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);

	
}
void Level01Scene::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

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


bool Level01Scene::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesBegin(touchLoc);
	// For Mouse Joiint 

	//for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	//{
	//	if (body->GetUserData() == NULL) continue; // 靜態物體不處理
	//	// 判斷點的位置是否落在動態物體一定的範圍
	//	Sprite* spriteObj = static_cast<Sprite*>(body->GetUserData());
	//	Size objSize = spriteObj->getContentSize();
	//	float fdist = MAX_2(objSize.width, objSize.height) / 2.0f;
	//	float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
	//	float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
	//	float tpdist = x * x + y * y;
	//	if (tpdist < fdist * fdist)
	//	{
	//		_bTouchOn = true;
	//		b2MouseJointDef mouseJointDef;
	//		mouseJointDef.bodyA = _bottomBody;
	//		mouseJointDef.bodyB = body;
	//		mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
	//		mouseJointDef.collideConnected = true;
	//		mouseJointDef.maxForce = 1000.0f * body->GetMass();
	//		_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // 新增 Mouse Joint
	//		body->SetAwake(true);
	//		break;
	//	}
	//}
	
	return true;
}
void Level01Scene::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
	
}
void Level01Scene::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesEnded(touchLoc)) { // 釋放一個車車
		setupCar();
	}
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
}


#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level01Scene::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif



