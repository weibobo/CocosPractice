
#ifndef BrushBoard_hpp
#define BrushBoard_hpp

#include "cocos2d.h"
#include "2d/CCSprite.h"

/*
 Usage:
    auto winSize = Director::getInstance()->getWinSize();
    auto layer = BrushBoard::create(winSize.width, winSize.height, Color4B(162, 87, 45, 255), 30);
    layer->setPosition(winSize.width / 2, winSize.height / 2);
    this->addChild(layer);
 */

class BrushBoard : public cocos2d::Sprite
{
public:
    BrushBoard(int w, int h, const cocos2d::Color4B& color, int radius);
    virtual ~BrushBoard();
    
    static BrushBoard* create(int w, int h, const cocos2d::Color4B& color, int radius);
    
    virtual bool init();
    
    void clearWithColor(unsigned char *data, int w, int h, cocos2d::Color4B color);
    void newCircle(cocos2d::Vec2 center);
    
    virtual void loadMaskShape();
    
    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
private:
    unsigned char *_pData = nullptr;
    unsigned char *_pSubData = nullptr;
    unsigned char *_pMaskData = nullptr;
    
    int _w = 0;
    int _h = 0;
    int _radius = 0;
    cocos2d::Color4B _color;
};

#endif /* BrushBoard_hpp */
