#include "LevelOne.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "LevelTwo.h"//���������

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*((t)+0.5f*(dt)))  //�w�g�g�L t ���A�A�g�L dt �ɶ������U�Z��
#define NUMBER_PARTICLES 100
#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;

LevelOne::~LevelOne()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	/*SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");*/
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* LevelOne::createScene()
{
	return LevelOne::create();
}

// on "init" you need to initialize your instance
bool LevelOne::init()
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

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new (std::nothrow) b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("level01.csb"); 

	//����
	_bApplyImpulse = false;
	_contactListener._bCreateSpark= false;
	_NumOfSparks = 10;
	_tdelayTime = 20;

	//���l
	_contactListener._bDeleteCar = false;

	//��������
	_bSwitchScene = false;
	//���l����

#ifndef BOX2D_DEBUG
	 //�]�w��ܭI���ϥ�
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
	arriveGoal();
	setDeleteEdge();
#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif
	//�[�J CLevelOneContactListener
	_b2World->SetContactListener(&_contactListener);
	//
	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(LevelOne::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(LevelOne::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(LevelOne::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(LevelOne::update));

	return true;
}


void LevelOne::setupButton() {
	//�s���l
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rectFrame_btn"));
	_rectButton = CButton::create();
	_rectButton->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_rectButton->setScale(btnSprite->getScale());
	this->addChild(_rectButton, 3);
	btnSprite->setVisible(false);


	//��������
	auto sceneBtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("switchScene_btn"));
	_switchSceneButton = CButton::create();
	_switchSceneButton->setButtonInfo("rightarrow.png", "rightarrowon.png", sceneBtnSprite->getPosition());
	_switchSceneButton->setScale(sceneBtnSprite->getScale());
	this->addChild(_switchSceneButton, 3);
	sceneBtnSprite->setVisible(false);


}
void LevelOne::setupCar() {
	auto appearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("appearPoint"));
	Point appearPoint = appearSprite->getPosition();
	appearSprite->setVisible(false);
	//���l
	auto carSprite = Sprite::createWithSpriteFrameName("car.png");
	carSprite->setScale(0.6f);
	this->addChild(carSprite, 2);
	Size carSize = carSprite->getContentSize();
	
	carDynamicBody = { nullptr };
	b2BodyDef carBodyDef;
	carBodyDef.type = b2_dynamicBody;
	carBodyDef.userData = carSprite;
	carBodyDef.position.Set(appearPoint.x / PTM_RATIO, appearPoint.y / PTM_RATIO);
	carDynamicBody = _b2World-> CreateBody(&carBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(carSize.width * 0.3f / PTM_RATIO, carSize.height * 0.3f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	carDynamicBody->CreateFixture(&fixtureDef);
	
	//���l
	Sprite* wheelSprite[2] = { nullptr };
	Point wheelLoc[2];
	Size wheelSize;
	b2Body* wheelDynamicBody[2] = { nullptr };
	b2BodyDef wheelBodyDef[2];

	for (int i = 0; i < 2; i++) {
		wheelSprite[i] = Sprite::createWithSpriteFrameName("wheel.png");
		wheelSprite[i]->setScale(0.7f);
		this->addChild(wheelSprite[i], 2);
		wheelSize = wheelSprite[i]->getContentSize();
		wheelBodyDef[i].type = b2_dynamicBody;
		wheelBodyDef[i].userData = wheelSprite[i];
		wheelBodyDef[i].position.Set((appearPoint.x - 10) / PTM_RATIO, (appearPoint.y - 10) / PTM_RATIO);
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
	revjoint.localAnchorA.Set(-0.9, -0.7);
	revjoint.bodyB = wheelDynamicBody[0];
	_b2World->CreateJoint(&revjoint);
	revjoint.bodyA = carDynamicBody;
	revjoint.localAnchorA.Set(0.7, -0.7);
	revjoint.bodyB = wheelDynamicBody[1];
	_b2World->CreateJoint(&revjoint);

}
void LevelOne::setStaticEdge() {

	// ���� EdgeShape �� body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	std::ostringstream ostr;
	std::string objname;
	for (int i = 0; i < 8; i++) {
		ostr.str("");
		ostr << "bar0" << i + 1; objname = ostr.str();
		auto edgeSprite = _csbRoot->getChildByName(objname);

		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();  // �۹�󤤤��I����m

		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I // world edge point1 & point2 
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;  // local edge point1 & point2
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix); //  modelMatrix = rotMatrix*modelMatrix
		modelMatrix.m[3] = loc.x; //�]�w Translation�A�ۤv���[�W���˪�, (pntLoc = csbRoot->getPosition())
		modelMatrix.m[7] = loc.y; //�]�w Translation�A�ۤv���[�W���˪�

			// ���ͨ�Ӻ��I
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		// bottom edge
		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}
}
void LevelOne::setDoor()
{
	// ���o�ó]�w doorPoint ���i�R�A����j
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO, size.height * 0.5f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// ���o�ó]�w door01 ���i�ʺA����j
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

	//���ͫ��� Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);

}
void LevelOne::setDoor02()
{
	// ���o�ó]�w doorPoint �e�عϥܬ��i�R�A����j
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint02"));
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO, size.height * 0.5f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true;
	staticBody->CreateFixture(&fixtureDef);

	// ���o�ó]�w door01 ���i�ʺA����j
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

	//���ͫ��� Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);
}
void LevelOne::setDoor03() {
	// ���o�ó]�w doorPoint03 ���i�R�A����j
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("doorPoint03"));
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
	circleShape.m_radius = 5 / PTM_RATIO* scaleX;
	/*b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO* scaleX, size.height * 0.5f / PTM_RATIO* scaleY);*/
	b2FixtureDef fixtureDef;

	fixtureDef.shape = &circleShape;
	staticBody->CreateFixture(&fixtureDef);

	// ���o�ó]�w door01 ���i�ʺA����j
	auto boxSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door03"));
	loc = boxSprite->getPosition();
	size = boxSprite->getContentSize();
	scaleX = boxSprite->getScaleX();
	scaleY = boxSprite->getScaleY();
	b2PolygonShape boxShape;
	
	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO * scaleX, size.height * 0.5f / PTM_RATIO * scaleY);

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData =boxSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &boxShape;
	
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	dynamicBody->CreateFixture(&fixtureDef);

	////
	b2RevoluteJoint* RevJoint = { nullptr };
	b2RevoluteJointDef RJoint;
	RJoint.Initialize(staticBody, dynamicBody, dynamicBody->GetWorldCenter());
	RevJoint = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
	//
	dynamicBody->ApplyLinearImpulse(b2Vec2(0, 8), dynamicBody->GetWorldCenter()+b2Vec2(2,0), true);

}

void LevelOne::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
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
void LevelOne::arriveGoal() {
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
	SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
	SensorFixtureDef.density = 10000; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
	SensorBody->CreateFixture(&SensorFixtureDef);

}
void LevelOne::setDeleteEdge() {


	// ���� EdgeShape �� body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);
	// ���ͪ�����R�A����һݭn�� rectShape
	b2PolygonShape rectShape;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.isSensor = true;	// �]�w�� Sensor
	fixtureDef.density = 20000; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_

	for (size_t i = 0; i < 18; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		std::ostringstream ostr;
		std::string objname;
		ostr << "edge"; ostr.width(2); ostr.fill('0'); ostr << i + 1; objname = ostr.str();
		auto const rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = rectSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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


void LevelOne::update(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);
	
	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
			ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}	
	}
	//
	// ���ͤ���
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//���ͧ�����
		// �P�_���𪺮ɶ��O�_����
		if (_bSparking) { //�i�H�Q�o�A��{�o�����Q�o
			_tdelayTime = 0; // �ɶ����s�]�w�A
			_bSparking = false; // �}�l�p��
			for (int i = 0; i < _NumOfSparks; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto sparkSprite = Sprite::createWithSpriteFrameName("flare.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//���ͤp������
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

				//���O�q
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
			_tdelayTime = 0; // �k�s
			_bSparking = true; // �i�i��U�@�����Q�o
		}
	}
	//���l�᭱����
	if (_bSmoke) {
		
		// �P�_���𪺮ɶ��O�_����
		if (_fSmokeTimer > 0.4f) { //�i�H�Q�o�A��{�o�����Q�o
			_fSmokeTimer = 0;
			for (int i = 0; i < 1; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto sparkSprite = Sprite::createWithSpriteFrameName("flare.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				sparkSprite->setScale(0.75);
				this->addChild(sparkSprite, 5);
				//���ͤp������
				
				Point _createLoc;
				b2Vec2 _carLoc;
				_carLoc = carDynamicBody->GetPosition();
				_createLoc.x = _carLoc.x;
				_createLoc.y = _carLoc.y;
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(_createLoc.x - 1.5, _createLoc.y-0.5);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = sparkSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				RectBody->SetGravityScale(-0.4);
				b2Fixture* RectFixture = RectBody->CreateFixture(&RectFixtureDef);
	
				////���O�q
				RectBody->ApplyForce(b2Vec2(-5, 0), b2Vec2(_createLoc.x, _createLoc.y), true);
			}
		}
		else {
			_fSmokeTimer += dt;
		}
	}
	
	
	//�R���I��T���Ϊ����l
	if (_contactListener._bDeleteCar) {
		_bSmoke = false;//������
		_contactListener._bDeleteCar = false;
		if (_contactListener._iBodyNum == 1) {
			Sprite* spriteData = (Sprite*)(_contactListener.contactBodyB->GetUserData());
			this->removeChild(spriteData);//���骺Sprite
			_b2World->DestroyBody(_contactListener.contactBodyB); // ����ثe�� body
		}
		else if(_contactListener._iBodyNum == 2) {
			Sprite* spriteData = (Sprite*)(_contactListener.contactBodyA->GetUserData());
			this->removeChild(spriteData);
			_b2World->DestroyBody(_contactListener.contactBodyA);
		}
	}
	//��������
	if (_bSwitchScene) {
		this->unschedule(schedule_selector(LevelOne::update));
		Director::getInstance()->replaceScene(LevelTwo::createScene());//����������
	}
}

bool LevelOne::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesBegin(touchLoc)) {
		//���ͨ���
		setupCar();
		//���}���᭱����
		_bSmoke = true;
	}
	_switchSceneButton->touchesBegin(touchLoc);
	// For Mouse Joiint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // �R�A���餣�B�z
		// �P�_�I����m�O�_���b�ʺA����@�w���d��
		if (carDynamicBody != NULL) {
			if (body->GetUserData() != carDynamicBody->GetUserData()) {

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
					_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // �s�W Mouse Joint
					body->SetAwake(true);
					break;
				}
			}
		}
		
	}
	return true;
}

void  LevelOne::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);

	_switchSceneButton->touchesMoved(touchLoc);
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

}

void  LevelOne::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
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
}


CLevelOneContactListener::CLevelOneContactListener()
{

}
void CLevelOneContactListener::BeginContact(b2Contact* contact)
{
	contactBodyA = contact->GetFixtureA()->GetBody();
	contactBodyB = contact->GetFixtureB()->GetBody();

	// check �O�_�����U���y�g�L sensor1 �A�u�n�g�L�N�ߨ����L�u�X�h
	if (contactBodyA->GetFixtureList()->GetDensity() == 10000.0f) { // �N�� sensor1
		_bCreateSpark = true;
		log("in-a");
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 10000.0f) {// �N�� sensor1
		_bCreateSpark = true;
		log("in-b");
	}
	if (contactBodyA->GetFixtureList()->GetDensity() == 20000.0f) { // �N�� sensor1
		if (contactBodyB->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 1;//�RB
			
			log("in-2a");
		}
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 20000.0f) {// �N�� sensor1
		if (contactBodyA->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 2;//�RA
;			log("in-2a");
		}
	}
	
}

#ifdef BOX2D_DEBUG
//��gø�s��k
void LevelOne::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif