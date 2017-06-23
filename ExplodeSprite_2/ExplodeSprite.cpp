#include "ExplodeSprite.hpp"
USING_NS_CC;

ExplodeSprite* ExplodeSprite::create(const std::string& filename, float gridLen, float duration)
{
    ExplodeSprite *sprite = new (std::nothrow) ExplodeSprite();
    if (sprite && sprite->init(filename, gridLen, duration))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool ExplodeSprite::init(const std::string &filename, float gridLen, float duration)
{
    if (!this->initWithFile(filename))
    {
        return false;
    }
    _gridLen = gridLen;
    _duration = duration;
    
    auto size = this->getContentSize();
    int row = floor(size.height / _gridLen);
    int col = floor(size.width / _gridLen);
    _moveDis = size.height * 3 / 2;
    
    for(auto i = 0; i < row; i++)
    {
        for(auto j = 0; j < col; j++)
        {
            auto cell = ExplodeFrag::createWithTexture(this->getTexture());
            cell->setTextureRect(Rect(j * _gridLen, i * _gridLen + _gridLen, _gridLen, _gridLen));
            cell->setAnchorPoint(Vec2(0, 1));
            cell->setPosition(j * _gridLen, size.height - i * _gridLen);
            cell->setTag(i * (col + 1) + j);
            
            auto pos = cell->getPosition();
            auto dst = rand() % (int) size.height + _moveDis;
            auto dis = dst - (size.height - pos.y);
            cell->_speed = dis / _duration;
            
            this->addChild(cell);
        }
    }
    
    this->setOpacity(0);
    
    // Register Touch Event
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    
    listener->onTouchBegan = CC_CALLBACK_2(ExplodeSprite::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(ExplodeSprite::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(ExplodeSprite::onTouchEnded, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

bool ExplodeSprite::onTouchBegan(Touch *touch, Event *unused_event)
{
    return true;
}

void ExplodeSprite::onTouchMoved(Touch *touch, Event *unused_event)
{

}

void ExplodeSprite::onTouchEnded(Touch *touch, Event *unused_event)
{
    if (_curTime > 0 || _isDisappeared)
    {
        this->restore();
    }
    else
    {
        this->scheduleUpdate();
    }
}

void ExplodeSprite::restore()
{
    this->unscheduleUpdate();
    _isDisappeared = false;
    _curTime = 0;
    
    auto size = this->getContentSize();
    int col = floor(size.width / _gridLen);
    
    for(auto& child : this->getChildren())
    {
        child->setScale(1);
        child->setOpacity(255);
        auto tag = child->getTag();
        auto i = floor(tag / (col + 1));
        auto j = floor(tag % (col + 1));
        child->setPosition(j * _gridLen, size.height - i * _gridLen);
    }
}

void ExplodeSprite::update(float dt){
    auto size = this->getContentSize();
    
    for(auto& node : this->getChildren())
    {
        auto child = static_cast<ExplodeFrag*>(node);
        
        auto pos = child->getPosition();
        auto dir = Vec2(0, -1);
        
        auto dis = child->_speed * dt;
        auto vec = dir * dis;
        auto posTarget = pos + vec;
        
        auto pos2top = size.height - posTarget.y;
        
        child->setPosition(posTarget);
        child->setScale(MAX(0, (1 - pos2top / _moveDis)) * _gridScale);
        child->setOpacity(MAX(0, 255 - (pos2top / _moveDis) * 255));
    }
    
    _curTime += dt;
    if (_curTime > _duration)
    {
        _curTime = 0;
        _isDisappeared = true;
        this->unscheduleUpdate();
        return;
    }
}
