#include "LevelFour.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "LevelFive.h"//������U�@��
#include "LevelTwo.h"//������W�@��
USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
#define NUMBER_PARTICLES 1000 // �w�]�@�����o 1000 �� Particles
using namespace cocostudio::timeline;

LevelFour::~LevelFour()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	/*SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");*/
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* LevelFour::createScene()
{
	return LevelFour::create();
}

// on "init" you need to initialize your instance
bool LevelFour::init()
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
	_csbRoot = CSLoader::createNode("level04.csb"); 
	
	//����
	_bApplyImpulse = false;
	_contactListener._bCreateSpark= false;
	_NumOfSparks = 8;
	_tdelayTime = 20;

	//���l
	_contactListener._bDeleteCar = false;

	//��������
	_bSwitchScene = false;
	_bSwitchPreviousScene = false;

	//�[�t
	_contactListener._bSpeedSensor01 = false;
	_contactListener._bSpeedSensor02 = false;

	//��ø���Ϳ���
	_drawPoint = Point (20, 100);
	//���U�����y
	_iAppearPointNum = 0;
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);
	createStaticBoundary();
	setupButton();
	setupBall();
	setStaticEdge();
	setDoor01();
	setDoor02();
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
	//�[�J CLevelFourContactListener
	_b2World->SetContactListener(&_contactListener);
	//
	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(LevelFour::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(LevelFour::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(LevelFour::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(LevelFour::update));

	return true;
}


void LevelFour::setupButton() {
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

	//�����W�@�ӳ���
	auto previousSceneBtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("switchScenePrevious_btn"));
	_switchPreviousSceneButton = CButton::create();
	_switchPreviousSceneButton->setButtonInfo("leftarrow.png", "leftarrowon.png", previousSceneBtnSprite->getPosition());
	_switchPreviousSceneButton->setScale(previousSceneBtnSprite->getScale());
	this->addChild(_switchPreviousSceneButton, 3);
	previousSceneBtnSprite->setVisible(false);
	//restart
	auto restartBtnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("restart_btn"));
	_restartButton = CButton::create();
	_restartButton->setButtonInfo("orange02.png", "orange05.png", restartBtnSprite->getPosition());
	_restartButton->setScale(restartBtnSprite->getScale());
	this->addChild(_restartButton, 3);
	restartBtnSprite->setVisible(false);
}

void LevelFour::setupBall() {
	auto appearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("appearPoint"));
	Point appearPoint = appearSprite->getPosition();
	appearSprite->setVisible(false);
	//�y
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
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	_ballDynamicBody->CreateFixture(&fixtureDef);

}
void LevelFour::setupBullet(int speed) {
	Point appearPoint;

	_iAppearPointNum = (_iAppearPointNum + 1) % 4;
	
	std::ostringstream ostr;
	std::string objname;
	ostr << "appearPoint"; ostr.width(2); ostr.fill('0'); ostr << _iAppearPointNum + 1; objname = ostr.str();

	auto appearSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
	appearPoint = appearSprite->getPosition();

	//�y
	_ballSprite = Sprite::createWithSpriteFrameName("bubble.png");
	_ballSprite->setScale(0.6f);
	this->addChild(_ballSprite, 2);
	Size ballSize = _ballSprite->getContentSize();

	_bulletDynamicBody = { nullptr };
	b2BodyDef ballBodyDef;
	ballBodyDef.type = b2_dynamicBody;
	ballBodyDef.userData = _ballSprite;
	ballBodyDef.position.Set(appearPoint.x / PTM_RATIO, appearPoint.y / PTM_RATIO);
	_bulletDynamicBody = _b2World->CreateBody(&ballBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = (ballSize.width - 4) * 0.28f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.2f;
	fixtureDef.friction = 0.1f;
	_bulletDynamicBody->CreateFixture(&fixtureDef);
	//���X�h
	_bulletDynamicBody->ApplyLinearImpulse(b2Vec2(0, -speed), _bulletDynamicBody->GetWorldCenter(), true);
	
}

void LevelFour::setDoor01() {
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
	SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
	SensorFixtureDef.density = 50000; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
	SensorBody->CreateFixture(&SensorFixtureDef);
	
}
void LevelFour::setDoor02() {
	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("sensor03"));
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
	SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
	SensorFixtureDef.density = 60000; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
	SensorBody->CreateFixture(&SensorFixtureDef);

}

void LevelFour::setStaticEdge() {

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
	for (int i = 0; i < 9; i++) {
		ostr.str("");
		ostr << "bar"; ostr.width(2); ostr.fill('0'); ostr << i + 1; objname = ostr.str();;
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
void LevelFour::createStaticBoundary()
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
void LevelFour::arriveGoal() {
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
void LevelFour::drawline() {
	
	auto ballSprite = Sprite::createWithSpriteFrameName("clock01.png");
	ballSprite->setScale(0.1f);
	this->addChild(ballSprite, 2);
	Size ballSize = ballSprite->getContentSize();

	b2Body* drawStaticBody;
	drawStaticBody = { nullptr };
	
	_drawBodyDef.type = b2_staticBody;
	_drawBodyDef.userData = ballSprite;
	_drawBodyDef.position.Set(_drawPoint.x / PTM_RATIO, _drawPoint.y / PTM_RATIO);
	drawStaticBody = _b2World->CreateBody(&_drawBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = (ballSize.width - 4) * 0.05f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 500.0f;
	fixtureDef.friction = 0.1f;
	drawStaticBody->CreateFixture(&fixtureDef);
}
void LevelFour::setDeleteEdge() {
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

	for (size_t i = 0; i < 33; i++)
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



void LevelFour::update(float dt)
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
				RectBody->ApplyForce(b2Vec2(rand() % 30 - 25, 30 + rand() % 30), b2Vec2(_createLoc.x, _createLoc.y), true);
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

	//�R���I��T���Ϊ����l
	if (_contactListener._bDeleteCar) {
		_contactListener._bDeleteCar = false;
			Sprite* spriteData = (Sprite*)(_contactListener.deleteBody->GetUserData());
			this->removeChild(spriteData);//���骺Sprite
			_b2World->DestroyBody(_contactListener.deleteBody);
	}
	//�R���l�u _bDeleteBullet
	if (_contactListener._bDeleteBullet) {
		_contactListener._bDeleteBullet = false;
		Sprite* spriteData = (Sprite*)(_contactListener.deleteBody->GetUserData());
		this->removeChild(spriteData);//���骺Sprite
		_b2World->DestroyBody(_contactListener.deleteBody);
	}
	//�I��P�����[�t
	if (_contactListener._bSpeedSensor01) {
		_contactListener._bSpeedSensor01 = false;
		_ballDynamicBody->ApplyLinearImpulse(b2Vec2(20,20), _ballDynamicBody->GetWorldCenter(), true); 
	}
	if (_contactListener._bSpeedSensor02) {
		_contactListener._bSpeedSensor02 = false;
		_ballDynamicBody->ApplyLinearImpulse(b2Vec2(-5, 40), _ballDynamicBody->GetWorldCenter(), true);
	}

	//��������
	if (_bSwitchScene) {
		this->unschedule(schedule_selector(LevelFour::update));
		Director::getInstance()->replaceScene(LevelFive::createScene());//����������
	}//Previous
	if (_bSwitchPreviousScene) {
		this->unschedule(schedule_selector(LevelFour::update));
		Director::getInstance()->replaceScene(LevelTwo::createScene());//����������
	}
	//��ø���Ϳ���
	if (!_contactListener._bPassSensor01) {
		if (_bIsDraw) {
			/*_bIsDraw = false;*/
			drawline();
		}
	}
	
	//���U�����y
	float waitingTime = 0;
	
		waitingTime = rand() % 1+0.3 ;
		if (_fFallingTimer > waitingTime) {
			setupBullet(3);
			_fFallingTimer = 0;
		}
		else _fFallingTimer += dt;
	
}

bool LevelFour::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	bool bOnRectButton = false;
	if (_rectButton->touchesBegin(touchLoc)) {
		//���Ͳy
		setupBall();
		bOnRectButton = true;
	}
	if (_restartButton->touchesBegin(touchLoc)) {
		
	}
	
	//����
	_switchSceneButton->touchesBegin(touchLoc);
	_switchPreviousSceneButton->touchesBegin(touchLoc);

	//��ø����
	if (!bOnRectButton) {
		_drawPoint = touchLoc;
		_bIsDraw = true;
	}

	return true;
}

void  LevelFour::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);
	_restartButton->touchesMoved(touchLoc);
	
	_switchSceneButton->touchesMoved(touchLoc);
	_switchPreviousSceneButton->touchesMoved(touchLoc);
	
	_drawPoint = touchLoc;
	_bIsDraw = true;

}

void  LevelFour::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesEnded(touchLoc);
	
	if (_restartButton->touchesEnded(touchLoc)) {
		for (b2Body* body = _b2World->GetBodyList(); body;)
		{
			if (body->GetFixtureList()->GetDensity() == 500) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite*)body->GetUserData();
					this->removeChild(spriteData);
				}
				b2Body* nextbody = body->GetNext();
				_b2World->DestroyBody(body);
				body = nextbody;
			}
			else  body = body->GetNext();
		}
		_contactListener._bPassSensor01 = false;
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


CLevelFourContactListener::CLevelFourContactListener()
{
	_bPassSensor01 = false;
}
void CLevelFourContactListener::BeginContact(b2Contact* contact)
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
	if (contactBodyA->GetFixtureList()->GetDensity() == 20000.0f) { //�R���Ҧ��I��T���Ϊ��F��
		if (contactBodyB->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 1;//�RB			
			log("in-2a");
			deleteBody = contactBodyB;
		}
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 20000.0f) {//�R���Ҧ��I��T���Ϊ��F��
		if (contactBodyA->GetFixtureList()->GetDensity() != 20000.0f) {
			_bDeleteCar = true;
			_iBodyNum = 2;//�RA
;			log("in-2b");
			deleteBody = contactBodyA;
		}
	}
	
	if (contactBodyA->GetFixtureList()->GetDensity() == 50000.0f) { // �N�� sensor1
		_bSpeedSensor01 = true;
		_bPassSensor01 = true;
		log("in-4a");
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 50000.0f) {// �N�� sensor1
		_bSpeedSensor01 = true;
		_bPassSensor01 = true;
		log("in-4b");
	}
	if (contactBodyA->GetFixtureList()->GetDensity() == 60000.0f) { // �N�� sensor1
		_bSpeedSensor02 = true;
		log("in-4a");
	}
	else if (contactBodyB->GetFixtureList()->GetDensity() == 60000.0f) {// �N�� sensor1
		_bSpeedSensor02 = true;
		log("in-4b");
	}
	
}

#ifdef BOX2D_DEBUG
//��gø�s��k
void LevelFour::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif