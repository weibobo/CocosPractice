//
//  RippleSprite.cpp
//  SGame
//
//  Created by ct on 2017/6/6.
//
//

#include "RippleSprite.hpp"
USING_NS_CC;

RippleSprite::~RippleSprite()
{
    for (auto i = 0; i< _nRow; i++)
    {
        delete _pSrcBuffer[i];
        delete _pDstBuffer[i];
    }
    delete _pSrcBuffer;
    delete _pDstBuffer;
    
    delete _meshes.verts;
    delete _meshes.indices;
    delete _pTexCoordsOrigin;
    
    glDeleteBuffers(2, _buffersVBO);
}

RippleSprite* RippleSprite::create(const std::string& filename, float gridLen)
{
    RippleSprite *sprite = new (std::nothrow) RippleSprite();
    if (sprite && sprite->init(filename, gridLen))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

void RippleSprite::initMesh(float gridLen)
{
    _fGridLen = gridLen;
    
    auto size = this->getContentSize();
    _nRow = floor(size.height / gridLen) + 1;
    _nCol = floor(size.width / gridLen) + 1;
    
    int cellSize = (_nRow + 1) * (_nCol + 1);
    _meshes.verts = new V3F_C4B_T2F[cellSize];
    _meshes.indices = new unsigned short[cellSize * 6];
    _meshes.vertCount = cellSize * 4;
    _meshes.indexCount = cellSize * 6;
    
    _pTexCoordsOrigin = new Tex2F[cellSize];
    
    for(int i = 0; i<_nRow + 1; i++){
        for(int j = 0; j<_nCol + 1; j++){
            int index = i * (_nCol + 1) + j;
            float x = _fGridLen * j;
            float y = size.height - _fGridLen * i;
            float s = x / size.width;
            float t = 1 - y / size.height;
            
            auto& rect = _meshes.verts[index];
            rect.vertices = Vec3(x, y, .0f);
            rect.colors = Color4B(255, 255, 255, 255);
            rect.texCoords = Tex2F(s, t);
            
            _pTexCoordsOrigin[index] = Tex2F(s, t);
            
            if (i != _nRow && j != _nCol)
            {
                int lt = index;
                int rt = lt + 1;
                int ld = lt + (_nCol + 1);
                int rd = ld + 1;
                
                auto addr = &_meshes.indices[lt * 6];
                *(addr + 0) = lt;
                *(addr + 1) = ld;
                *(addr + 2) = rd;
                *(addr + 3) = lt;
                *(addr + 4) = rd;
                *(addr + 5) = rt;
            }
        }
    }
}

void RippleSprite::initVBO()
{
    glGenBuffers(2, &_buffersVBO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * _meshes.indexCount, _meshes.verts, GL_STATIC_DRAW);
    
    // vertices
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, vertices));
    
    // colors
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, colors));
    
    // tex coords
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, texCoords));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_meshes.indices[0]) * _meshes.indexCount, _meshes.indices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool RippleSprite::init(const std::string &filename, float gridLen)
{
    if (!this->initWithFile(filename))
    {
        return false;
    }
    
    this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR));
    
    this->initMesh(gridLen);
    this->initVBO();
    
    // Create Depth Buffer
    _pSrcBuffer = new float*[_nRow + 1];
    _pDstBuffer = new float*[_nRow + 1];
    for(int i=0; i<_nRow+1; i++)
    {
        _pSrcBuffer[i] = new float[_nCol + 1];
        _pDstBuffer[i] = new float[_nCol + 1];
        
        memset(_pSrcBuffer[i], 0, sizeof(float) * _nCol + 1);
        memset(_pDstBuffer[i], 0, sizeof(float) * _nCol + 1);
    }
    
    // Register Touch Event
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    
    listener->onTouchBegan = CC_CALLBACK_2(RippleSprite::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(RippleSprite::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(RippleSprite::onTouchEnded, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}


bool RippleSprite::onTouchBegan(Touch *touch, Event *unused_event)
{
    this->newPoint(touch->getLocation(), _fDepth, _fRadius);
    
    return true;
}

void RippleSprite::onTouchMoved(Touch *touch, Event *unused_event)
{
    this->newPoint(touch->getLocation(), _fDepth, _fRadius);
}

void RippleSprite::onTouchEnded(Touch *touch, Event *unused_event)
{
}

void RippleSprite::update(float dt){
    for(int i=1; i<_nRow; i++){
        for(int j=1; j<_nCol; j++){
            float up       = _pSrcBuffer[i-1][j];
            float down     = _pSrcBuffer[i+1][j];
            float left     = _pSrcBuffer[i][j-1];
            float right    = _pSrcBuffer[i][j+1];
            float mid      = _pDstBuffer[i][j];

            float energy = (up + down + left + right) / 2 - mid;
            energy -= energy / _fAttenuation;
            _pDstBuffer[i][j] = energy;
        
            float uOffset = (up - down) * _fMapWidth;
            float vOffset = (left - right) * _fMapWidth;
            
            auto index = i*(_nCol + 1) + j;
            auto& tcCurrent = _meshes.verts[index].texCoords;
            auto& tcOrigin = _pTexCoordsOrigin[index];
            tcCurrent.u = tcOrigin.u + uOffset;
            tcCurrent.v = tcOrigin.v + vOffset;
        }
    }
    swap(_pSrcBuffer, _pDstBuffer);
    
    // Reload
    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * _meshes.vertCount, _meshes.verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RippleSprite::newPoint(const Vec2& posWorld, float depth, float radius)
{
    auto size = this->getContentSize();
    auto posLocal = this->convertToNodeSpace(posWorld);
    
    float x = posLocal.x;
    float y = size.height - posLocal.y;
    
    float minX = x - radius;
    float maxX = x + radius;
    float minY = y - radius;
    float maxY = y + radius;
    
    const int margin = 1;
    auto minRow = MAX(margin, floorf(minY / _fGridLen));
    auto maxRow = MIN(_nRow - margin, ceilf(maxY / _fGridLen));
    auto minCol = MAX(margin, floorf(minX / _fGridLen));
    auto maxCol = MIN(_nCol - margin, ceilf(maxX/_fGridLen));
    
    for(int i=minRow; i<=maxRow; i++){
        for(int j=minCol; j<=maxCol; j++){
            auto& v = _meshes.verts[i*(_nCol + 1) + j].vertices;
            float dis = Vec2(x, y).distance(Vec2(v.x, size.height - v.y));
            if(dis < radius)
            {
                _pSrcBuffer[i][j] -= depth * (0.5+0.5*cosf(dis*M_PI/radius));
            }
        }
    }
}

void RippleSprite::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
#if CC_USE_CULLING
    // Don't do calculate the culling if the transform was not updated
    auto visitingCamera = Camera::getVisitingCamera();
    auto defaultCamera = Camera::getDefaultCamera();
    if (visitingCamera == defaultCamera) {
        _insideBounds = ((flags & FLAGS_TRANSFORM_DIRTY)|| visitingCamera->isViewProjectionUpdated()) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
    }
    else
    {
        _insideBounds = renderer->checkVisibility(transform, _contentSize);
    }
    
    if(_insideBounds)
#endif
    {
        _customCommand.init(_globalZOrder, transform, flags);
        _customCommand.func = CC_CALLBACK_0(RippleSprite::onDraw, this, transform, flags);
        renderer->addCommand(&_customCommand);
    }
}

void RippleSprite::onDraw(const Mat4 &transform, uint32_t flags)
{
    GL::bindTexture2D(this->getTexture()->getName());
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    this->getGLProgramState()->apply(transform);
    
    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    
    // vertices
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, vertices));
    
    // colors
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, colors));
    
    // tex coords
    glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof( V3F_C4B_T2F, texCoords));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDrawElements(GL_TRIANGLES, (GLsizei) _meshes.indexCount, GL_UNSIGNED_SHORT, _meshes.indices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
