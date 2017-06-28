#include "LightningSimulator.hpp"
USING_NS_CC;

Segment::Segment(const Vec2& start, const Vec2& end, int g, int b)
: posStart(start)
, posEnd(end)
, gen(g)
, branch(b)
{
    auto dir = (posEnd - posStart).getNormalized();
    dirNormal = Vec2(dir.y, -dir.x);
    posMid = (posEnd + posStart) / 2;
}

LightningSimulator::LightningSimulator()
{
    glGenBuffers(2, &_buffersVBO[0]);
}

LightningSimulator::~LightningSimulator()
{
    glDeleteBuffers(2, _buffersVBO);
    
    CC_SAFE_DELETE_ARRAY(_meshes.verts);
    CC_SAFE_DELETE_ARRAY(_meshes.indices);
}

LightningSimulator* LightningSimulator::create(const std::string& filename, const Vec2& start, const Vec2& end)
{
    LightningSimulator *sprite = new (std::nothrow) LightningSimulator();
    if (sprite && sprite->init(filename, start, end))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool LightningSimulator::init(const std::string &filename, const Vec2& start, const Vec2& end)
{
    if (!this->initWithFile(filename))
    {
        return false;
    }
    
    _posStart = start;
    _posEnd = end;
    
    this->setBlendFunc(BlendFunc::ADDITIVE);
    this->getTexture()->setAntiAliasTexParameters();
    this->setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR));
    
    this->genSegments();
    
    // Register Touch Event
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    
    listener->onTouchBegan = CC_CALLBACK_2(LightningSimulator::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(LightningSimulator::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(LightningSimulator::onTouchEnded, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    this->genLightning();
    
    return true;
}

void LightningSimulator::genLightning()
{
    this->genSegments();
    this->genMeshes();
}

void LightningSimulator::genSegments()
{
    auto size = this->getContentSize();
    auto mid = Vec2(size.width / 2, size.height / 2);
    
    _segmentList.clear();
    _segmentList.push_back(Segment(_posStart + mid, _posEnd + mid, 0, 0));
    
    for(auto gen = 1; gen <= _genDepth; gen++)
    {
        vector<Segment> newSegmentList;
        for(auto seg = _segmentList.begin(); seg != _segmentList.end();)
        {
            auto length = (seg->posEnd - seg->posStart).getLength();
            auto maxOffset = length * _segOffsetFactor;
            auto offset = rand_0_1() * maxOffset - maxOffset / 2;
            auto newMid = seg->dirNormal * offset + seg->posMid;
            
            newSegmentList.push_back(Segment(seg->posStart, newMid, gen, seg->branch));
            newSegmentList.push_back(Segment(newMid, seg->posEnd, gen, seg->branch));
            
            if (rand_0_1() < _splitPercent)
            {
                // jitter
                auto dir = newMid - seg->posStart;
                auto degree = rand_0_1() * 6 - 6 / 2;
                dir.rotate(Vec2(0, 0), CC_DEGREES_TO_RADIANS(degree));
                dir *= 0.7;
                auto splitEnd = dir + newMid;
                newSegmentList.push_back(Segment(newMid, splitEnd, gen, seg->branch + 1));
            }

            
            seg = _segmentList.erase(seg);
        }
        _segmentList.assign(newSegmentList.begin(), newSegmentList.end());
    }
}

void LightningSimulator::genMeshes()
{
    CC_SAFE_DELETE_ARRAY(_meshes.verts);
    CC_SAFE_DELETE_ARRAY(_meshes.indices);
    
    auto segSize = _segmentList.size();
    _meshes.verts = new V3F_C4B_T2F[segSize * 4];
    _meshes.indices = new unsigned short[segSize * 6];
    _meshes.vertCount = segSize * 4;
    _meshes.indexCount = segSize * 6;
    
    //calculate quad for seg
    //      v0        v3
    //       * -------*
    //       |        |
    // start ---------> end
    //       | segW/2 |
    //       *--------*
    //      v1        v2
    for(auto i = 0; i < _segmentList.size(); i++)
    {
        auto& seg = _segmentList[i];
        auto normal = seg.dirNormal;
        float segW = 12 * pow(0.3, seg.branch);
        float opacity = 1.0 * pow(0.4, seg.branch);

        Vec2 p0 = seg.posStart - normal * segW / 2;
        Vec2 p1 = seg.posStart + normal * segW / 2;
        Vec2 p2 = seg.posEnd + normal * segW / 2;
        Vec2 p3 = seg.posEnd - normal * segW / 2;

        auto mid = seg.posMid;
        p0 = mid + (p0 - mid) * _growScale;
        p1 = mid + (p1 - mid) * _growScale;
        p2 = mid + (p2 - mid) * _growScale;
        p3 = mid + (p3 - mid) * _growScale;
        
        auto& v1 = _meshes.verts[i*4 + 0];
        auto& v2 = _meshes.verts[i*4 + 1];
        auto& v3 = _meshes.verts[i*4 + 2];
        auto& v4 = _meshes.verts[i*4 + 3];
        
        v1.vertices = Vec3(p0.x, p0.y, 0);
        v2.vertices = Vec3(p1.x, p1.y, 0);
        v3.vertices = Vec3(p2.x, p2.y, 0);
        v4.vertices = Vec3(p3.x, p3.y, 0);
        
        v1.colors = Color4B(255, 255, 255, 255 * opacity);
        v2.colors = Color4B(255, 255, 255, 255 * opacity);
        v3.colors = Color4B(255, 255, 255, 255 * opacity);
        v4.colors = Color4B(255, 255, 255, 255 * opacity);
        
        v1.texCoords.u = 0;
        v1.texCoords.v = 0;
        v2.texCoords.u = 0;
        v2.texCoords.v = 1;
        v3.texCoords.u = 1;
        v3.texCoords.v = 1;
        v4.texCoords.u = 1;
        v4.texCoords.v = 0;
        
        _meshes.indices[i*6 + 0] = i*4 + 0;
        _meshes.indices[i*6 + 1] = i*4 + 1;
        _meshes.indices[i*6 + 2] = i*4 + 2;
        _meshes.indices[i*6 + 3] = i*4 + 0;
        _meshes.indices[i*6 + 4] = i*4 + 2;
        _meshes.indices[i*6 + 5] = i*4 + 3;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * _meshes.indexCount, _meshes.verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_meshes.indices[0]) * _meshes.indexCount, _meshes.indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool LightningSimulator::onTouchBegan(Touch *touch, Event *unused_event)
{
    return true;
}

void LightningSimulator::onTouchMoved(Touch *touch, Event *unused_event)
{
}

void LightningSimulator::onTouchEnded(Touch *touch, Event *unused_event)
{
}

void LightningSimulator::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
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
        _customCommand.func = CC_CALLBACK_0(LightningSimulator::onDraw, this, transform, flags);
        renderer->addCommand(&_customCommand);
    }
}

void LightningSimulator::onDraw(const Mat4 &transform, uint32_t flags)
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
