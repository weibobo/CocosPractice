#ifndef LightningSimulator_hpp
#define LightningSimulator_hpp

#include "cocos2d.h"
#include "2d/CCSprite.h"
#include <vector>

/*
 */

USING_NS_CC;
using namespace std;

class Segment{
public:
    Segment(const Vec2& start, const Vec2& end, int g, int b);
    Vec2 posStart;
    Vec2 posEnd;
    Vec2 posMid;
    Vec2 dirNormal;
    int gen = 0;
    int branch = 0;
};

class LightningSimulator : public cocos2d::Sprite
{
public:
    LightningSimulator();
    virtual ~LightningSimulator();
    static LightningSimulator* create(const std::string &filename, const Vec2& start, const Vec2& end);
    virtual bool init(const std::string &filename, const Vec2& start, const Vec2& end);
    
    virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    
    void genLightning();

    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
protected:
    cocos2d::CustomCommand _customCommand;
    
    void genSegments();
    void genMeshes();
private:
    TrianglesCommand::Triangles _meshes = {nullptr, nullptr, 0, 0};
    GLuint _buffersVBO[2];
    
    Vec2 _posStart;
    Vec2 _posEnd;
    
    vector<Segment> _segmentList;
    
    float _segOffsetFactor = 0.2f;
    float _splitPercent = 0.5f;
    int _genDepth = 5;
    /*
     | Texture
     | - transparent - opacity - transparent - |
     | transparent:opacity = 7:1
     */
    float _growScale = 8.0f;
};

#endif /* LightningSimulator_hpp */
