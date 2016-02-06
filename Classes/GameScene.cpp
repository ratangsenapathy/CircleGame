#include "GameScene.h"
#include "MainMenuScene.h"
#include "GameOverScene.h"
//#include "Levels.h"


#define COCOS2D_DEBUG 1

USING_NS_CC;
using namespace rapidjson;



GameScene::~GameScene()
{
  
    delete []obstacles;
    rotationPoint->removeAllChildrenWithCleanup(true);
    obstacleRotationPoint->removeAllChildrenWithCleanup(true);
    this->removeAllChildrenWithCleanup(true);
    Director::getInstance()->getTextureCache()->removeUnusedTextures();
    //_eventDispatcher->removeAllEventListeners();

    this->unscheduleUpdate();
}

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
   
    
    // 'layer' is an autorelease object
    
    auto layer = GameScene::create();

    // add layer as a child to scene
    scene->addChild(layer);
    
    
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance

bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
   
    std::string fullPath = "res/Levels.json";
    std::string jsonFile = FileUtils::getInstance()->getStringFromFile(fullPath.c_str());
    levelNo = UserDefault::getInstance()->getIntegerForKey("LevelNo");
    generateJSON(jsonFile);
    
    loadScene();
    
    return true;
}

void GameScene::loadScene()
{
    
    
    obstacleSpeed =10;
    
    secondCount=0; minuteCount=0;
    theta=0;
    controlable=0;
    distance=rMax;
    r=0;
    
    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();
    
    
    
#if COMPILE_FOR_MOBILE == 1
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    
    listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
#endif
    
    goal = DrawNode::create();
    goal->drawDot(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y), 5, Color4F(100,0,0,1));
    this->addChild(goal,1);          // drawing the goal
    
    
    rotationPoint = Node::create();
    rotationPoint->setPosition(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    this->addChild(rotationPoint, 2);
    
    
    //Setting the exit button
    auto exitLabel = Label::createWithTTF("Exit","fonts/Marker Felt.ttf",15);
    exitButtonWidth=exitLabel->getContentSize().width;
    exitButtonHeight=exitLabel->getContentSize().height;
    exitLabel->setPosition(Point(visibleSize.width-exitButtonWidth+origin.x,visibleSize.height-exitButtonHeight+origin.y));
    this->addChild(exitLabel);
    
    
    //setting the clock
    timer = Label::createWithTTF("00:00","fonts/Marker Felt.ttf",10);
    timer->setPosition(Point(timer->getContentSize().width+origin.x,visibleSize.height-timer->getContentSize().height+origin.y));
    this->schedule(schedule_selector(GameScene::updateClock),1.0f);  //scedule to call upDateClock function every 1.0 sec
    this->addChild(timer);
    
    obstacleRotationPoint = Node::create();
    obstacleRotationPoint->setPosition(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    this->addChild(obstacleRotationPoint, 3);
    
    
    //float theta=0;
    
    snake[0] = DrawNode::create();
    snake[0]->drawDot(Vec2(0,0),ballRadius,ballColor);
    //theta+=2*M_PI/150;
    //this->addChild(snake[0],2);
    
    rotationPoint->addChild(snake[0]);
    // fixedPoint->addChild(snake[0]);
    
    
    //loop to draw the concentric circles
    
    for(r=15;r<=rMax;r+=15)
    {
        for(theta=0;theta<=2*M_PI;theta+=2*M_PI/r){
            pathNode = DrawNode::create();
            pathNode->drawDot(Vec2(r*cos(theta)+origin.x+visibleSize.width/2,r*sin(theta)+origin.y+visibleSize.height/2),1,Color4F(0,0,10,1));
            //pathNode->autorelease();
            this->addChild(pathNode,1);
            //this->removeChild(pathNode);
        }
    }
    
    
    for(int i=0;i<obstacleCount;i++)
    {
        
        int index=0;
        int base=obstacles[i].ringNum*15;
        int vertexUpperCount= (int)ceil((obstacles[i].theta2*M_PI/180-obstacles[i].theta1*M_PI/180)/(2*M_PI/(base+15)));
        
        int vertexLowerCount= (int)ceil((obstacles[i].theta2*M_PI/180-obstacles[i].theta1*M_PI/180)/(2*M_PI/(base!=0?base:15)));
        
        Point *vertices = new Point[vertexLowerCount+vertexUpperCount+1];
        
        //Loops to draw obstacles
        float lower=obstacles[i].theta1*M_PI/180;
        float upper=obstacles[i].theta2*M_PI/180;
        
        for(theta=upper;theta>=lower;theta-=2*M_PI/(base+15)){
            
            vertices[index++]=Vec2((base+15)*cos(theta),(base+15)*sin(theta));
            
        }
        for(theta=lower;theta<=upper;theta+=2*M_PI/(base!=0?base:15)){
            
            vertices[index++]=Vec2(base*cos(theta),base*sin(theta));
            
        }
        
        Color4F obstacleColor = convertHexToRBG(obstacles[i].color);
        DrawNode* polygon = DrawNode::create();
        
        polygon->drawPolygon(vertices,index, obstacleColor, 1, obstacleColor);
        obstacleRotationPoint->addChild(polygon);
        lower =upper;
        
        delete []vertices;
        vertices=NULL;
        
        
    }
    
    
    snake[0]->runAction(Sequence::create(Place::create(Vec2(snake[0]->getPosition().x+rMax,snake[0]->getPosition().y)),CallFunc::create(CC_CALLBACK_0(GameScene::actionComplete,this)),NULL)); // set ball position
    
    distance=rMax;
    // auto rotateBy = RotateBy::create(0.25f,360/distance);
    // rotationPoint->runAction(RepeatForever::create(rotateBy));
    
    auto obstacleRotate = RotateBy::create(0.5f,obstacleSpeed*-1);
    obstacleRotationPoint->runAction(RepeatForever::create(obstacleRotate));
    
    controlable=0;
    this->scheduleUpdate();
}

void GameScene::generateJSON(std::string jsonFile)
{
    
    document.Parse<0>(jsonFile.c_str());
    parseJSON();
}

void GameScene::parseJSON()
{
    
    if(document.HasMember("levels"))
    {
        
        const rapidjson::Value& jsonLevels = document["levels"];
        noOfLevels=jsonLevels.Size();
        rMax = jsonLevels[(SizeType)levelNo]["numOfRings"].GetInt()*15;
        ballTime = 1.0/jsonLevels[(SizeType)levelNo]["ball"]["speed"].GetDouble();
        ballRadius = jsonLevels[(SizeType)levelNo]["ball"]["radius"].GetDouble();
        std::string hexString = jsonLevels[(SizeType)levelNo]["ball"]["color"].GetString();
        
        ballColor = convertHexToRBG(hexString);
        ballDirection = jsonLevels[(SizeType)levelNo]["ball"]["direction"].GetInt();
        ballInitTheta = jsonLevels[(SizeType)levelNo]["ball"]["initTheta"].GetDouble();
        //CCLOG("Test=%d",test);
        const rapidjson::Value& jsonObstacles = jsonLevels[(SizeType)levelNo]["obstacles"];
        obstacleCount=jsonObstacles.Size();
        obstacles = new struct obstacle[obstacleCount];
        
        for (SizeType i = 0; i < obstacleCount; i++)
        {
            //obstacles[i].speed = jsonObstacles[i]["speed"].GetDouble();
            obstacles[i].theta1 = jsonObstacles[i]["theta1"].GetDouble();
            obstacles[i].theta2 = jsonObstacles[i]["theta2"].GetDouble();
            obstacles[i].ringNum= jsonObstacles[i]["ringNum"].GetInt();
            obstacles[i].color = (char*)jsonObstacles[i]["color"].GetString();
            
        }
        
        
    }
    
}

Color4F GameScene::convertHexToRBG(std::string hexString)
{
    int hexValue;
    std::stringstream ss;
    ss << std::hex << hexString.substr(1);
    ss >> hexValue;
    float red = ((hexValue >> 16) & 0xFF) /255.0;  // Extract the RR byte
    float green = ((hexValue >> 8) & 0xFF)/255.0;   // Extract the GG byte
    float blue = ((hexValue) & 0xFF)/255.0;
    return Color4F(red,green,blue,1);
}

#if COMPILE_FOR_MOBILE == 1
bool GameScene::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{   // check if exit button region was clicked
    //_eventDispatcher->removeAllEventListeners();
    //auto scene = GameScene::createScene(levelNo);
   // auto scene = GameScene::createScene();
   // Director::getInstance()->replaceScene(scene);
   // return true;

    if((touch->getLocation().x>=(visibleSize.width-2*exitButtonWidth)) && (touch->getLocation().y>=(visibleSize.height-1.5*exitButtonHeight)))
    {           auto scene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(2, scene));
        //removeResources();

        return true;
    }
    if(controlable==1)  //  otherwise move ball inwards if controllable
    {
        controlable=0; // while moving inwards ball should not be controllable
        distance-=15;
        auto rotateBy = RotateBy::create(ballTime,0);
        rotationPoint->runAction(RepeatForever::create(rotateBy));
        
        // CCLOG("Cor=%f,%f",snake[0]->getPosition().x,snake[0]->getPosition().y);
        snake[0]->runAction(Sequence::create(MoveTo::create(ballTime*2,Vec2(snake[0]->getPosition().x-15,snake[0]->getPosition().y)),CallFunc::create(CC_CALLBACK_0(GameScene::actionComplete,this)),NULL));
        
    }
    
    return true;
}
//#else
/*
void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    if((touch->getLocation().x>=(visibleSize.width-2*exitButtonWidth)) && (touch->getLocation().y>=(visibleSize.height-1.5*exitButtonHeight)))
    {
        auto scene = MainMenuScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(2, scene));
        return true;
    }
    if(controlable==1)
    {
        controlable=0;
        distance-=15;
        auto rotateBy = RotateBy::create(objectTime,0);
        rotationPoint->runAction(RepeatForever::create(rotateBy));
        
        // CCLOG("Cor=%f,%f",snake[0]->getPosition().x,snake[0]->getPosition().y);
        snake[0]->runAction(Sequence::create(MoveTo::create(objectTime*2,Vec2(snake[0]->getPosition().x-15,snake[0]->getPosition().y)),
                                             CallFunc::create(CC_CALLBACK_0(GameScene::actionComplete,this)),NULL));
        
        
    }
    
}
*/
#endif

//function to update the clock
void GameScene::updateClock(float dt)
{
    secondCount++;
    if(secondCount == 60) {secondCount=0; minuteCount++;}
   // std::stringstream stream;
   // stream << secondCount;
   char secondText[3];  char minuteText[3];
    char timeText[6];
    if(secondCount<10)
    {
        sprintf(secondText,"0%d",secondCount);
    }
    else
        sprintf(secondText,"%d",secondCount);
    
    if(minuteCount<10)
    {
        sprintf(minuteText,"0%d",minuteCount);
    }
    else
        sprintf(minuteText,"%d",minuteCount);

    sprintf(timeText,"%s:%s",minuteText,secondText);
    timer->setString(timeText);
    
}

//after a move into the next path or outside after collision the ball should be made controllab;e againa and speed adjusted

void GameScene::actionComplete()
{
    if(distance==0)
    {
       
        
        if(levelNo+1==noOfLevels)
        {   //removeResources();
            auto scene = GameOverScene::createScene();
            Director::getInstance()->replaceScene(scene);
            return;
        }
        else{
            
            
            //auto scene = GameScene::createScene(levelNo);
            //auto scene = GameScene::createScene();
            //Director::getInstance()->replaceScene(scene);
            removeResources();
            UserDefault::getInstance()->setIntegerForKey("LevelNo", ++levelNo);
            parseJSON();
            loadScene();
            return;
        }
    }
         auto rotateBy = RotateBy::create(ballTime,360/distance);
    rotationPoint->runAction(RepeatForever::create(rotateBy));
controlable=1;
}

void GameScene::removeResources()
{
    timer->unschedule(schedule_selector(GameScene::updateClock));
    delete []obstacles;
    if(schedule_selector(GameScene::updateClock))
    rotationPoint->removeAllChildrenWithCleanup(true);
    obstacleRotationPoint->removeAllChildrenWithCleanup(true);
    this->removeAllChildrenWithCleanup(true);
    Director::getInstance()->getTextureCache()->removeUnusedTextures();
    //_eventDispatcher->removeAllEventListeners();
    _eventDispatcher->removeAllEventListeners();
    this->unscheduleUpdate();
}

//function updates every frame
void GameScene::update(float dt){
 Point snakePosition1 = rotationPoint->convertToWorldSpace(snake[0]->getPosition());
    int rotationValue = (360-(int)(obstacleRotationPoint->getRotation()) % 360);
    rotationValue=rotationValue==360?0:rotationValue;
    
  //  CCLOG("%d",360-(int)(obstacleRotationPoint->getRotation()) % 360);
//CCLOG("Position=%f,%f",snakePosition1.x,snakePosition1.y);
if(controlable==1){
  for(int i=0;i<obstacleCount;i++)
   {              // CCLOG("Distance=%f",distance);
         if(obstacles[i].ringNum*15==distance || obstacles[i].ringNum*15+15==distance)
           {
                      Point snakePosition = rotationPoint->convertToWorldSpace(snake[0]->getPosition());

 		        //Size visibleSize = Director::getInstance()->getVisibleSize();
               // Vec2 origin = Director::getInstance()->getVisibleOrigin();
                int originY=visibleSize.height/2+origin.y;
                int snakeY=snakePosition.y;
                int curTheta = (int)((acos((float)(snakePosition.x-rotationPoint->getPosition().x)/(float)distance))*180/M_PI);

                  if(snakeY<originY)
                           curTheta=360-curTheta;
                    //  CCLOG("Distance=%f, Theta=%f",distance,curTheta*180/M_PI);
                     // float thetaone=levels[levelNo].blocks[i].theta1*180/M_PI;
		     // float thetatwo=levels[levelNo].blocks[i].theta2*180/M_PI;
		     // CCLOG("Bottom,Top: %f, %f",thetaone,thetatwo);
               int lower =((int)((obstacles[i].theta1)+rotationValue)) % 360;
               int upper =((int)((obstacles[i].theta2)+rotationValue)) % 360;
		 if(curTheta>=lower && curTheta<=upper)       // check is collision occurs
                 {
                             controlable=0;
                         int diff= rMax - distance;
                             distance=rMax;
      auto rotateBy = RotateBy::create(0.25f,0);
      rotationPoint->runAction(RepeatForever::create(rotateBy));

           //CCLOG("Cor=%f,%f",snake[0]->getPosition().x,snake[0]->getPosition().y);
                     snake[0]->runAction(Sequence::create(MoveTo::create(ballTime*2,Vec2(snake[0]->getPosition().x+diff,snake[0]->getPosition().y)),CallFunc::create(CC_CALLBACK_0(GameScene::actionComplete,this)),NULL));
                           break;
                 }
           }
   }

  }
    
}
