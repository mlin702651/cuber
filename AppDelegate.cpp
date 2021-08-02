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

#include <cstdlib>
#include <ctime>
#include "AppDelegate.h"
//#include "mainscene.h"

#define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1  // audio engine 兩個只能二選一

#define BOX2D_EXAMPLE 4




#if BOX2D_EXAMPLE == 1
#include "StaticDynamicScene.h"
#elif  BOX2D_EXAMPLE == 2
#include "FixtureCollisionScene.h"
#elif  BOX2D_EXAMPLE == 3
#include "JointScene.h"
#elif  BOX2D_EXAMPLE == 4
#include "LevelOne.h"
#elif  BOX2D_EXAMPLE == 5
#include "LevelTwo.h"
#elif  BOX2D_EXAMPLE == 6
#include "LevelThree.h"
#elif  BOX2D_EXAMPLE == 7
#include "LevelFour.h"
#elif  BOX2D_EXAMPLE == 8
#include "LevelFive.h"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 720);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);
static cocos2d::Size screenResolutionSize = cocos2d::Size(1280, 720);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("Box2D Demo", cocos2d::Rect(0, 0, screenResolutionSize.width, screenResolutionSize.height),1.0f);
#else
        glview = GLViewImpl::create("my2dx");
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(false);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    auto frameSize = glview->getFrameSize();

    // if the frame's height is larger than the height of medium size.
    //if (frameSize.height > mediumResolutionSize.height)
    //{        
    //    director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    //}
    //// if the frame's height is larger than the height of small size.
    //else if (frameSize.height > smallResolutionSize.height)
    //{        
    //    director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    //}
    //// if the frame's height is smaller than the height of medium size.
    //else
    //{        
    //    director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    //}

    register_all_packages();

    // 設定亂數生成器

    srand(time(NULL));
    // create a scene. it's an autorelease object
       
    // create a scene. it's an autorelease object
#if BOX2D_EXAMPLE == 1
    auto scene = StaticDynamic::createScene();
#elif  BOX2D_EXAMPLE == 2
    auto scene = FixtureCollision::createScene();
#elif  BOX2D_EXAMPLE == 3
    auto scene = JointScene::createScene();
//#elif  BOX2D_EXAMPLE == 4
//    auto scene = Level01Scene::createScene();
#elif  BOX2D_EXAMPLE == 4
    auto scene = LevelOne::createScene();
#elif  BOX2D_EXAMPLE == 5
    auto scene = LevelTwo::createScene();
#elif  BOX2D_EXAMPLE == 6
    auto scene = LevelThree::createScene();
#elif  BOX2D_EXAMPLE == 7
    auto scene = LevelFour::createScene();
#elif  BOX2D_EXAMPLE == 8
    auto scene = LevelFive::createScene();
#endif

    director->runWithScene(scene); // 設定 director 從這個 scene 開始執行
    return true;

}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}
