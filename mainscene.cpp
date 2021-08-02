/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "mainscene.h"


#ifdef Audio_Example
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#endif


USING_NS_CC;

using namespace cocostudio::timeline;
using namespace cocos2d::ui;
//using namespace CocosDenshion;

mainscene::~mainscene()
{

#ifdef Audio_Example 1
    AudioEngine::end();
#endif

    this->removeAllChildren();
    Director::getInstance()->getTextureCache()->removeUnusedTextures();

    //SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("scene101.plist");
    //SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("scene101bg.plist");
    //Director::getInstance()->getTextureCache()->removeUnusedTextures(); // 釋放用不到的TEXTURE
}


Scene* mainscene::createScene()
{
    return mainscene::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool mainscene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

// 載入 cocos Studio 輸出的 csb 檔
    auto rootNode = CSLoader::createNode("mainscenex.csb");
    addChild(rootNode);
    _uiView.setProperty("uiroot.csb", visibleSize, *this);
    _uiView.setModel(_ParticleControl);

/*
    // Particle Control System
    // 最好的方式是，以下的 Slider 根據這裡的設定值，顯示出正確的數值與位置
    _ParticleControl.init(*this);
    _ParticleControl._emitterPt = _EmitterSprite->getLoc();
    _ParticleControl._fDir = 90.0;
    _ParticleControl._iNumParticles = 100;
    _ParticleControl._iGenParticles = 0;
    _ParticleControl._fSpread = 180.0f;
    _ParticleControl._fVelocity = 2.5;	 // 分子的離開速度
    _ParticleControl._fLifeTime = 3.5f;	 // 分子的存活時間
    _ParticleControl._fSpin = 0;
    _ParticleControl._fGravity = 0;
    _ParticleControl._fElpasedTime = 0;
    _ParticleControl.setType(STAY_FOR_TWOSECONDS); // 分子運動的型態，預設為 0
    _ParticleControl._windDir = Point(0, 0); // 本範例沒有實作此項功能

    */
    auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
    listener->onTouchBegan = CC_CALLBACK_2(mainscene::onTouchBegan, this);		//加入觸碰開始事件
    listener->onTouchMoved = CC_CALLBACK_2(mainscene::onTouchMoved, this);		//加入觸碰移動事件
    listener->onTouchEnded = CC_CALLBACK_2(mainscene::onTouchEnded, this);		//加入觸碰離開事件

    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
    this->schedule(CC_SCHEDULE_SELECTOR(mainscene::update));

    return true;
}

void mainscene::update(float dt)
{
    _ParticleControl.update(dt);
}


bool mainscene::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
    Point touchLoc = pTouch->getLocation();
    _uiView.onTouchBegan(touchLoc);

    return true;
}

void mainscene::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
    Point touchLoc = pTouch->getLocation();
    _uiView.onTouchMoved(touchLoc);
}

void  mainscene::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
    Point touchLoc = pTouch->getLocation();
    _uiView.onTouchEnded(touchLoc);
}
