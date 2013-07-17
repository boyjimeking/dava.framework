/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_LANDSCAPE_NODE_H__
#define __DAVAENGINE_LANDSCAPE_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/LandscapeCursor.h"
#include "Render/Highlevel/RenderObject.h"

#include "FileSystem/FilePath.h"

namespace DAVA
{

class Scene;
class Image;
class Texture;
class RenderDataObject;
class Shader;
class SceneFileV2;
class Heightmap;
    
template<class T>
class LandQuadTreeNode
{
public:
    LandQuadTreeNode()
    {
        childs = 0;
        parent = 0;
        for (int32 k = 0; k < 4; ++k)
            neighbours[k] = 0;
    }
    ~LandQuadTreeNode()
    {
        SafeDeleteArray(childs);
    }
    
    void AllocChilds()
    {
		SafeDeleteArray(childs);
        childs = new LandQuadTreeNode[4];
    }
    
    LandQuadTreeNode * childs;  // It's array of 4 child nodes
    LandQuadTreeNode * parent;
    LandQuadTreeNode * neighbours[4]; 
    T data;
};
    
template <class T>
class LinearQuadTree
{
public:
    
    
    

};

/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
    Landscape node is always axial aligned for simplicity of frustum culling calculations
    Keep in mind that landscape orientation cannot be changed using localTransform and worldTransform matrices. 
 */ 

class Landscape : public RenderObject
{
public:	
    enum 
    {
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
    };
    
	Landscape();
	virtual ~Landscape();
    
    /**
        \brief Set lod coefficients for dynamic roam landscape
        Default values: (60, 120, 240, 480)
        Every next value should be almost twice higher than previous to avoid gaps between levels
     */
    void SetLods(const Vector4 & lods);
    

    enum eTiledShaderMode
    {
        TILED_MODE_TILEMASK = 0,
        TILED_MODE_TEXTURE,
        TILED_MODE_MIXED,
        TILED_MODE_TILE_DETAIL_MASK, 
        
        TILED_MODE_COUNT
    };

    
    /**
     \brief Change rendering mode. 
     \param[in] renderingMode rendering mode of landscape.
     */
    void SetTiledShaderMode(eTiledShaderMode _tiledShaderMode);
    
    /**
     \brief Get rendering mode. 
     \returns rendering mode of landscape.
     */
    inline eTiledShaderMode GetTiledShaderMode();

    
    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    virtual void BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & landscapeBox);
    
    enum eTextureLevel
    {
        TEXTURE_COLOR = 0,  // in case of BLENDED_SHADER in alpha channel it can be tile mask for TILED_TEXTURES
        TEXTURE_TILE_MASK,
        TEXTURE_TILE0,   
        TEXTURE_TILE1,
        TEXTURE_TILE2,
        TEXTURE_TILE3,
        // TEXTURE_BUMP,
        
        TEXTURE_DETAIL, 
        
        TEXTURE_TILE_FULL, 
        
        TEXTURE_COUNT
    };

	class LandscapeVertex
	{
	public:
		Vector3 position;
		Vector2 texCoord;
	};
    
    /**
        \brief Set texture for the specific texture level
        
     To render landscape you need to set textures.  
        For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
        For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
        For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
          
        \param[in] level level of texture you want to set
        \param[in] textureName name of texture you want to open and set to specific level
     */
    void SetTexture(eTextureLevel level, const FilePath & textureName);

    
    /**
     \brief Set texture for the specific texture level
     
     To render landscape you need to set textures.  
     For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
     For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
     For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
     
     \param[in] level level of texture you want to set
     \param[in] texture you want to set to specific level
     */
    void SetTexture(eTextureLevel level, Texture * texture);

	/**
        \brief Get texture that was previously set in SetTexture.
        \param[in] level 
        \returns current texture
	 */
	virtual Texture * GetTexture(eTextureLevel level);
    
	/**
        \brief Get texture name that was previously set in SetTexture.
        \param[in] level level of texture you want to get name
        \returns current texture name
	 */
    const FilePath & GetTextureName(eTextureLevel level);

	/**
        \brief Set texture name for export.
        \param[in] level level of texture you want to set name
        \param[in] newTextureName new texture name
	 */
    void SetTextureName(eTextureLevel level, const FilePath &newTextureName);
    
    
	/**
        \brief Set tiling for specific texture level.
        This function gives you can control of tiling for specific landscape level.
     */    
    void SetTextureTiling(eTextureLevel level, const Vector2 & tiling);

    /**
        \brief Get tiling for specified texture level.
        \returns Tiling for specified texture level.
     */
    const Vector2 & GetTextureTiling(eTextureLevel level); 

    void SetTileColor(eTextureLevel level, const Color & color);
    const Color & GetTileColor(eTextureLevel level);

    /**
        \brief Overloaded draw function to draw landscape
     */
	virtual void Draw(Camera * camera);

	/**
        \brief Get landscape mesh geometry.
        Unoptimized lod0 mesh is returned.
        \param[out] vertices landscape vertices
        \param[out] indices landscape indices
	 */
	void GetGeometry(Vector<LandscapeVertex> & vertices, Vector<int32> & indices);
    
    /**
        \brief Function to receive pathname of heightmap object
        \returns pathname of heightmap
     */
    const FilePath & GetHeightmapPathname();

    
    void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    // TODO: Need comment here
	bool PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal = 0) const;
	Vector3 GetPoint(int16 x, int16 y, uint16 height);

	void CursorEnable();
	void CursorDisable();
	void SetCursorTexture(Texture * texture);
	void SetBigTextureSize(float32 bigSize);
	void SetCursorPosition(const Vector2 & position);
	void SetCursorScale(float32 scale);

    Heightmap *GetHeightmap();
    virtual void SetHeightmap(Heightmap *height);
    
    virtual void UpdateFullTiledTexture();
    FilePath SaveFullTiledTexture();
    Texture *CreateFullTiledTexture();
    
    void SetFog(bool _fogEnabled);
    bool IsFogEnabled() const;
    void SetFogDensity(float32 _fogDensity);
    float32 GetFogDensity() const;
    void SetFogColor(const Color & _fogColor);
    const Color & GetFogColor() const;

    LandscapeCursor *GetCursor();
    
	virtual RenderObject * Clone(RenderObject *newObject);


protected:	
    
    class LandscapeQuad
    {
    public:
        LandscapeQuad()
        {
            x = y = size = lod = 0;
            rdoQuad = -1;
            frame = 0;
        }
        
        int16   x, y;
        //int16   xbuf, ybuf;
        int16   size;
        int8    lod;
        int16   rdoQuad;
        AABBox3 bbox;
        uint32  frame;
    };
   
    static const int32 RENDER_QUAD_WIDTH = 129;
    static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;
    static const int32 INDEX_ARRAY_COUNT = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6;
    

    void RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels);
    LandQuadTreeNode<LandscapeQuad> * FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize);
    void FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth);

    void BindMaterial(int32 lodLayer);
    void UnbindMaterial();
    
    void DrawQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod);
    void Draw(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void DrawFans();

    Texture * CreateTexture(eTextureLevel level, const FilePath & textureName);
    
    int16 AllocateRDOQuad(LandscapeQuad * quad);
    void ReleaseAllRDOQuads();

	int GetMaxLod(LandscapeQuad& quad);
	
    Vector<LandscapeVertex *> landscapeVerticesArray;
    Vector<RenderDataObject *> landscapeRDOArray;
    
    uint16 * indices;
    Texture * textures[TEXTURE_COUNT];
    Vector<FilePath> textureNames;
    
    int32 lodLevelsCount;
    float32 lodDistance[8]; //
    float32 lodSqDistance[8];
    
    LandQuadTreeNode<LandscapeQuad> quadTreeHead;

    List<LandQuadTreeNode<LandscapeQuad>*> fans;
    
    int32 allocatedMemoryForQuads;
    
    Vector3 cameraPos;
    Frustum *frustum;
    
    ePrimitiveType primitypeType;

    void InitShaders();
    void ReleaseShaders();
    
    int32 uniformCameraPosition;
    int32 uniformTextures[TEXTURE_COUNT];
    int32 uniformTextureTiling[TEXTURE_COUNT];
    Vector2 textureTiling[TEXTURE_COUNT];
    int32 uniformTileColor[TEXTURE_COUNT];
    Color tileColor[TEXTURE_COUNT];
    
    int32 uniformFogDensity;
    int32 uniformFogColor;
    int32 uniformFogDensityFT;
    int32 uniformFogColorFT;

    
    Shader * tileMaskShader;
    Shader * fullTiledShader;

	LandscapeCursor * cursor;
        
    int16 queueRdoQuad;
    int32 queueRenderCount;
    uint16 * queueDrawIndices;
    
    void FlushQueue();
    void ClearQueue();
    
    bool BuildHeightmap();
    void BuildLandscape();
    Heightmap *heightmap;
    FilePath heightmapPath;
    
	static const uint32 TEXTURE_TILE_FULL_SIZE = 2048;
    
    Vector<LandQuadTreeNode<LandscapeQuad> *>lod0quads;
    Vector<LandQuadTreeNode<LandscapeQuad> *>lodNot0quads;

    int32 prevLodLayer;
    
    int32 flashQueueCounter;
    
//    eTiledShaderMode tiledShaderMode;
    uint32 tiledShaderMode;
    
    int32 nearLodIndex;
    int32 farLodIndex;
    
    
    bool    isFogEnabled;
    float32 fogDensity;
    Color   fogColor;
    
    
public:
    
    INTROSPECTION_EXTEND(Landscape, RenderObject,
//        MEMBER(heightmapPath, "Heightmap Path", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        COLLECTION(textureNames, "Texture Names", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
         
        MEMBER(tiledShaderMode, "Tiled Shader Mode", I_SAVE | I_VIEW | I_EDIT)

        MEMBER(isFogEnabled, "Is Fog Enabled", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(fogDensity, "Fog Density", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(fogColor, "Fog Color", I_SAVE | I_VIEW | I_EDIT)
    );
};

    
inline Landscape::eTiledShaderMode Landscape::GetTiledShaderMode()
{
    return (eTiledShaderMode)tiledShaderMode;
}
    
};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__





