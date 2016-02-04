#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__
#include "Definitions.h"
#include "rapidjson/document.h"
#include "cocos2d.h"
//#include "CCVector.h"

class GameScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    struct obstacle
    {
        float speed;
        float theta1;
        float theta2;
        int ringNum;
        char *color;
    };
    
    virtual bool init();
    cocos2d::Size visibleSize;
    cocos2d::DrawNode* goal;
    cocos2d::DrawNode* pathNode;
    cocos2d::DrawNode* snake[SNAKE_LENGTH];
    cocos2d::Node* rotationPoint;
    cocos2d::Node* obstacleRotationPoint;
    cocos2d::DrawNode* blocks;
    struct obstacle *obstacles;
    int noOfLevels;
    int r=0;
    int levelNo=0;
    int controlable=0;      // flag to check if user can controll the ball or not
    int rMax=0;             // max radius of circle

    int secondCount=0;    // second han value in the timer
    int minuteCount=0;   //minute hand clock in the timer
    float obstacleSpeed=0;
    cocos2d::Label *timer;
    float distance;
    float theta=0;
    float ballTime;
    float ballRadius;
    float ballInitTheta;
    int exitButtonWidth;
    int exitButtonHeight;
    int ballDirection;
    int obstacleCount;
    cocos2d::Color4F ballColor;
    rapidjson::Document document;
//    cocos2d::Size visibleSize;
//    cocos2d::Vec2 origin;
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);
   #if COMPILE_FOR_MOBILE == 1
    bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event* event);
  // #else
   // void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
   #endif
 
    void update(float dt);
    void actionComplete();
    void updateClock(float dt);
    void loadScene();
    void removeResources();
    cocos2d::Color4F convertHexToRBG(std::string hexString);
    void generateJSON(std::string jsonFile);
    void parseJSON();
    ~GameScene();
    
    
};

#endif // __HELLOWORLD_SCENE_H__
