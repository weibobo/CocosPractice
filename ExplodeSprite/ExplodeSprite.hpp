#ifndef ExplodeSprite_hpp
#define ExplodeSprite_hpp

#include "cocos2d.h"
#include "2d/CCSprite.h"

/*
 auto node = ExplodeSprite::create("fish2.png");
 node->setPosition(Vec2(winSize.width/2,winSize.height/2));
 addChild(node);
 */

USING_NS_CC;
using namespace std;

class ExplodeFrag : public cocos2d::Sprite
{
public:
    static ExplodeFrag* createWithTexture(cocos2d::Texture2D *texture)
    {
        ExplodeFrag *sprite = new (std::nothrow) ExplodeFrag();
        if (sprite && sprite->initWithTexture(texture))
        {
            sprite->autorelease();
            return sprite;
        }
        CC_SAFE_DELETE(sprite);
        return nullptr;
    }
    
    float _speed = 0.0f;
};

class ExplodeSprite : public cocos2d::Sprite
{
public:
    static ExplodeSprite* create(const std::string &filename, float gridLen = 3.0f, float duration = 2.5f);
    virtual bool init(const std::string &filename, float gridLen, float duration);
    
    virtual void update(float t) override;
    
    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    
    void setGridScale(float scale) { _gridScale = scale; }
    float getGridScale() const { return _gridScale; }
protected:
    void restore();
private:
    float _gridLen = 0.0f;
    float _gridScale = 3.0f;
    float _duration = 0.0f;
    float _curTime = 0.0f;
    bool _isDisappeared = false;
};

#endif /* ExplodeSprite_hpp */
