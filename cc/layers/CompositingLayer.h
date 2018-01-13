#ifndef cc_LayerForCompositing_h
#define cc_LayerForCompositing_h

#include "third_party/WebKit/Source/wtf/Vector.h"
#include "third_party/WebKit/public/platform/WebCanvas.h"
#include "third_party/skia/include/core/SkColor.h"

class SkBitmap;
struct SkRect;

namespace blink {
class IntRect;
class IntSize;
}

namespace cc {

struct DrawToCanvasProperties;
class CompositingTile;
struct TileActionInfo;
class TileActionInfoVector;
class SkBitmapRefWrap;
class LayerTreeHost;
class TilesAddr;

class CompositingLayer {
public:
    CompositingLayer(int id);
    virtual ~CompositingLayer();

    enum CCType {
        NormalCCLayer,
        ImageCCLayer,
    };
    virtual CCType ccType() const { return NormalCCLayer; }

    int id() const;

    void insertChild(CompositingLayer* child, size_t index);
    void removeFromParent();

    void setParent(CompositingLayer* layer);
    CompositingLayer* parent() const { return m_parent; }
    void replaceChild(CompositingLayer* reference, CompositingLayer* newLayer);
    int indexOfChild(CompositingLayer* child) const;
    void removeChildOrDependent(CompositingLayer* child);
    void removeAllChildren();

    WTF::Vector<CompositingLayer*>& children() { return m_children; }

    DrawToCanvasProperties* drawToCanvasProperties() { return m_prop; }
    void updataDrawProp(DrawToCanvasProperties* m_prop);

    bool masksToBounds() const;
    bool drawsContent() const;
    const blink::IntSize& bounds() const;
    bool opaque() const;
    float opacity() const;
    SkColor backgroundColor() const;
    bool isDoubleSided() const;
    bool useParentBackfaceVisibility() const;

    void updataTile(int newIndexNumX, int newIndexNumY, DrawToCanvasProperties* prop);
    void cleanupUnnecessaryTile(const WTF::Vector<TileActionInfo*>& tiles);

    virtual void drawToCanvas(LayerTreeHost* host, blink::WebCanvas* canvas, const blink::IntRect& clip);

    void blendToTiles(TileActionInfoVector* willRasteredTiles, const SkBitmap* bitmap, const SkRect& dirtyRect);
    void blendToTilesByTiles(TileActionInfoVector* willRasteredTiles);
    
    void drawToCanvasChildren(LayerTreeHost* host, SkCanvas* canvas, const SkRect& clip, int deep);

    size_t tilesSize() const;

    SkColor getBackgroundColor() const;

protected:
    friend class DoClipLayer;
    void blendToTile(CompositingTile* tile, const SkBitmap* bitmap, const SkRect& dirtyRect, SkColor* solidColor, bool isSolidColorCoverWholeTile);

    void drawDebugLine(SkCanvas& canvas, CompositingTile* tile);
        
    int m_id;
    DrawToCanvasProperties* m_prop;

    TilesAddr* m_tilesAddr;
    int m_numTileX;
    int m_numTileY;

    WTF::Vector<CompositingLayer*> m_children;
    CompositingLayer* m_parent;
};

class CompositingImageLayer : public CompositingLayer {
public:
    CompositingImageLayer(int id);
    virtual ~CompositingImageLayer() override;
    virtual void drawToCanvas(LayerTreeHost* host, blink::WebCanvas* canvas, const blink::IntRect& clip) override;
    void setImage(SkBitmapRefWrap* bitmap);
    virtual CCType ccType() const override { return CompositingLayer::ImageCCLayer;    }
    
private:
    SkBitmapRefWrap* m_bitmap;
};

}

#endif // cc_LayerForCompositing_