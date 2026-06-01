#ifndef DOODLEPADWIDGET_HPP
#define DOODLEPADWIDGET_HPP

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QStack>
#include <QListWidget>
#include <QComboBox>
#include <QColor>

// -----------------------------
// Brush Modes
// -----------------------------
enum BrushMode {
    NormalBrush,
    GlowBrush,
    SprayBrush,
    SmudgeBrush
};

// -----------------------------
// Blend Modes
// -----------------------------
enum BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    Additive
};

// -----------------------------
// Layer Structure
// -----------------------------
struct Layer {
    QPixmap pixmap;
    bool visible = true;
    QString name;
    BlendMode mode = Normal;
};

// -----------------------------
// DoodlePadWidget Class
// -----------------------------
class DoodlePadWidget : public QWidget {
    Q_OBJECT

public:
    explicit DoodlePadWidget(QWidget *parent = nullptr);

    // ---- Public so MainWindow menus can call them ----
    void chooseColor();
    void useEraser();
    void clearCanvas();
    void saveImage();
    void undo();
    void redo();
    void setBrushMode(BrushMode mode) { brushMode = mode; erasing = false; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // ---- Internal helpers ----
    void drawLineTo(const QPoint &endPoint);
    void addLayer();
    void deleteLayer();
    void toggleLayerVisibility();
    void layerSelectionChanged();
    void refreshLayerList();
    void penSizeChanged(int size);
    void pushUndo();

private:
    // ---- Layer system ----
    QList<Layer> layers;
    int currentLayer = 0;

    QListWidget *layerList = nullptr;
    QComboBox *blendBox = nullptr;

    // ---- Drawing state ----
    bool drawing = false;
    bool erasing = false;
    QPoint lastPoint;
    QColor penColor = Qt::black;
    int penSize = 5;

    // ---- Brush mode (THIS is the correct place!) ----
    BrushMode brushMode = NormalBrush;

    // ---- Undo/Redo ----
    QStack<QPixmap> undoStack;
    QStack<QPixmap> redoStack;
};

#endif // DOODLEPADWIDGET_HPP
