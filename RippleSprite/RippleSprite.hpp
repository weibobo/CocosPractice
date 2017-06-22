//
//  CustomSprite.hpp
//  SGame
//
//  Created by ct on 2017/6/6.
//
//

#ifndef RippleSprite_hpp
#define RippleSprite_hpp

#include "cocos2d.h"
#include "2d/CCSprite.h"

/*
 
 auto ripple = RippleSprite::create("frozen_small.png",8);
 ripple->setPosition(Vec2(winSize.width/2,winSize.height/2));
 ripple->scheduleUpdate();
 addChild(ripple);
 
 */

USING_NS_CC;
using namespace std;

class RippleSprite : public cocos2d::Sprite
{
public:
    virtual ~RippleSprite();
    static RippleSprite* create(const std::string &filename, float gridLen);
    virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
    virtual void update(float t) override;
    
    virtual bool init(const std::string &filename, float gridSideLen);
    void newPoint(const Vec2& posWorld, float depth, float radius);
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    
    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    
    void setDepth(float depth) { _fDepth = depth; }
    float getDepth() const { return _fDepth; }
    
    void setRadius(float radius) { _fRadius = radius; }
    float getRadius() const { return _fRadius; }
    
    void setAttenuation(float att) { _fAttenuation = att; }
    float getAttenuation() const { return _fAttenuation; }
    
    void setMapWidth(float width) { _fMapWidth = width; }
    float getMapWidth() const { return _fMapWidth; }
    
protected:
    cocos2d::CustomCommand _customCommand;
protected:
    void initMesh(float gridLen);
    void initVBO();
private:
    float _fGridLen = 0;
    int   _nRow = 0;
    int   _nCol = 0;
    
    TrianglesCommand::Triangles _meshes;
    Tex2F* _pTexCoordsOrigin = nullptr;
    
    GLuint _buffersVBO[2]; //0: vertex  1: indices
    
    float** _pSrcBuffer = nullptr;
    float** _pDstBuffer = nullptr;
    
    float _fDepth = 512;
    float _fRadius = 12;
    
    float _fAttenuation = 16;
    float _fMapWidth = 1.0 / 1048;
};

#endif /* RippleSprite_hpp */
