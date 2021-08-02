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

	

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new (std::nothrow) b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("level01.csb");

#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
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

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(Level01Scene::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(Level01Scene::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(Level01Scene::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level01Scene::update));

	return true;
}


void Level01Scene::update(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	//// ���o _b2World ���Ҧ��� body �i��B�z
	//// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	//for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	//{
	//	// �H�U�O�H Body ���]�t Sprite ��ܬ���
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

	//���l
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

	//���l
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
	for (int i = 0; i < 3; i++) {
		ostr.str("");
		ostr << "bar0" << i+1; objname = ostr.str();
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
void Level01Scene::setColorBar() {
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

	
	auto edgeSprite = _csbRoot->getChildByName("door01");
	/*edgeSprite->setColor(CarColor[0]);*/
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

	// ���o�ó]�w frame01_pri �e�عϥܬ��i�R�A����j
	loc.y += 200;
	log("%f", loc.y);
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);


	

	//���ͫ��� Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);

	
}
void Level01Scene::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);

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


bool Level01Scene::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesBegin(touchLoc);
	// For Mouse Joiint 

	//for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	//{
	//	if (body->GetUserData() == NULL) continue; // �R�A���餣�B�z
	//	// �P�_�I����m�O�_���b�ʺA����@�w���d��
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
	//		_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // �s�W Mouse Joint
	//		body->SetAwake(true);
	//		break;
	//	}
	//}
	
	return true;
}
void Level01Scene::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
	
}
void Level01Scene::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesEnded(touchLoc)) { // ����@�Ө���
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
//��gø�s��k
void Level01Scene::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif



