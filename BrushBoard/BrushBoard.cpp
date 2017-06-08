
#include "BrushBoard.hpp"
USING_NS_CC;

BrushBoard::BrushBoard(int w, int h, const cocos2d::Color4B& color, int radius)
: _w(w)
, _h(h)
, _color(color)
, _radius(radius)
{
    _pData = new unsigned char[_w * _h * 4];
    _pSubData = new unsigned char[2 * _radius * 2 * _radius * 4];
    _pMaskData = new unsigned char[2 * _radius * 2 * _radius];
    
    loadMaskShape();
    clearWithColor(_pData, _w, _h, _color);
}

BrushBoard::~BrushBoard(){
    delete [] _pData;
    delete [] _pSubData;
    delete [] _pMaskData;
}

BrushBoard* BrushBoard::create(int w, int h, const cocos2d::Color4B& color, int radius){
    BrushBoard* pRet = new BrushBoard(w, h, color, radius);
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

bool BrushBoard::init()
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
    
    listener->onTouchBegan = CC_CALLBACK_2(BrushBoard::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BrushBoard::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BrushBoard::onTouchEnded, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

bool BrushBoard::onTouchBegan(Touch *touch, Event *unused_event)
{
    Vec2 touchLocation = touch->getLocation(); // Get the touch position
    touchLocation = this->getParent()->convertToNodeSpace(touchLocation);
    Rect bBox = getBoundingBox();
    return bBox.containsPoint(touchLocation);
}

void BrushBoard::onTouchMoved(Touch *touch, Event *unused_event)
{
    Vec2 touchLocation = touch->getLocation(); // Get the touch position
    touchLocation = this->getParent()->convertToNodeSpace(touchLocation);
    this->newCircle(touchLocation);
}

void BrushBoard::onTouchEnded(Touch *touch, Event *unused_event)
{
}

void BrushBoard::loadMaskShape()
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
                _pMaskData[offset] = 0;
            }
            else
            {
                _pMaskData[offset] = _color.a;
            }
        }
    }
}

void BrushBoard::newCircle(cocos2d::Vec2 center)
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
            _pData[mainOffset] = MIN(oldAlpha, newAlpha);
            
            _pSubData[subOffset - 3] = _pData[mainOffset - 3];
            _pSubData[subOffset - 2] = _pData[mainOffset - 2];
            _pSubData[subOffset - 1] = _pData[mainOffset - 1];
            _pSubData[subOffset - 0] = _pData[mainOffset - 0];
        }
    }
    
    _texture->updateWithData(_pSubData, basex, basey, w, h);
}

void BrushBoard::clearWithColor(unsigned char *data, int w, int h, cocos2d::Color4B color){
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            data[(i * w + j) * 4 + 0] = color.r;
            data[(i * w + j) * 4 + 1] = color.g;
            data[(i * w + j) * 4 + 2] = color.b;
            data[(i * w + j) * 4 + 3] = color.a;
        }
    }
}
