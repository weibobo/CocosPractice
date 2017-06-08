
#include "BlackBoard.hpp"
USING_NS_CC;

BlackBoard::BlackBoard(int w, int h, const cocos2d::Color4B& color, int radius)
: _w(w)
, _h(h)
, _color(color)
, _radius(radius)
{
    _pData = new unsigned char[_w * _h * 4];
    _pSubData = new unsigned char[2 * _radius * 2 * _radius * 4];
    _pMaskData = new unsigned char[2 * _radius * 2 * _radius];
    
    loadMaskShape();
    memset(_pData, 0, _w * _h * 4);
}

BlackBoard::~BlackBoard(){
    delete [] _pData;
    delete [] _pSubData;
    delete [] _pMaskData;
}

BlackBoard* BlackBoard::create(int w, int h, const cocos2d::Color4B& color, int radius){
    BlackBoard* pRet = new BlackBoard(w, h, color, radius);
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool BlackBoard::init()
{
    if (!Node::init())
    {
        return false;
    }
    
    setContentSize(Size(_w, _h));
    
    // Bind Texture
    auto texture = new Texture2D();
    texture->initWithData(_pData, _w * _h * 4, Texture2D::PixelFormat::RGBA8888, _w, _h, Size(_w, _h));
    initWithTexture(texture);
    texture->release();
    
    // Register Touch Event
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    
    listener->onTouchBegan = CC_CALLBACK_2(BlackBoard::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BlackBoard::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BlackBoard::onTouchEnded, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

bool BlackBoard::onTouchBegan(Touch *touch, Event *unused_event)
{
    Vec2 touchLocation = touch->getLocation(); // Get the touch position
    touchLocation = this->getParent()->convertToNodeSpace(touchLocation);
    Rect bBox = getBoundingBox();
    return bBox.containsPoint(touchLocation);
}

void BlackBoard::onTouchMoved(Touch *touch, Event *unused_event)
{
    Vec2 touchLocation = touch->getLocation(); // Get the touch position
    touchLocation = this->getParent()->convertToNodeSpace(touchLocation);
    this->newCircle(touchLocation);
}

void BlackBoard::onTouchEnded(Touch *touch, Event *unused_event)
{
}

void BlackBoard::loadMaskShape()
{
    memset(_pMaskData, 0, 2 * _radius * 2 * _radius);
    
    int cx = _radius;
    int cy = _radius;
    
    for (int j = 0; j < _radius * 2; j++)
    {
        for (int i = 0; i < _radius * 2; i++)
        {
            float dx = i - cx;
            float dy = j - cy;
            
            int offset = j * _radius * 2 + i;
            float dist = sqrt(dx * dx + dy * dy);
            
            if (dist < _radius)
            {
                _pMaskData[offset] = 255;
            }
        }
    }
}

void BlackBoard::newCircle(cocos2d::Vec2 center)
{
    auto pLocal = this->convertToNodeSpace(center);
    
    int x = pLocal.x;
    int y = _h - pLocal.y;
    
    memset(_pSubData, 0, 2 * _radius * 2 * _radius * 4);
    
    int basex = MAX(x - _radius, 0);
    int basey = MAX(y - _radius, 0);
    
    int w = MIN(x + _radius, _w) - basex;
    int h = MIN(y + _radius, _h) - basey;
    
    w = w / (int)2 * 2;
    
    int ox = x  - _radius;
    int oy = y  - _radius;
    
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int dx = i + basex - ox;
            int dy = j + basey - oy;
            
            int subOffset = (j * w + i) * 4 + 3;
            int mainOffset = ((j + basey) * _w + (i + basex)) * 4 + 3;
            int maskOffset = dy * _radius * 2 + dx;
            
            auto oldAlpha = _pData[mainOffset];
            auto newAlpha = _pMaskData[maskOffset];
            auto alpha = MAX(oldAlpha, newAlpha);
            
            if (alpha > 0)
            {
                _pSubData[subOffset - 3] = _pData[mainOffset - 3] = _color.r;
                _pSubData[subOffset - 2] = _pData[mainOffset - 2] = _color.g;
                _pSubData[subOffset - 1] = _pData[mainOffset - 1] = _color.b;
                _pSubData[subOffset - 0] = _pData[mainOffset - 0] = alpha;
            }
        }
    }
    
    _texture->updateWithData(_pSubData, basex, basey, w, h);
}
