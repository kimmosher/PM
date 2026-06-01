#include "DoodlePadWidget.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QListWidget>
#include <QColorDialog>
#include <QFileDialog>
#include <QRandomGenerator>
#include <QtMath>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

DoodlePadWidget::DoodlePadWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);

    // ---- Main Layout ----
    auto *mainLayout = new QHBoxLayout(this);

    // ---- Left: Canvas + Toolbar ----
    auto *canvasLayout = new QVBoxLayout;
    mainLayout->addLayout(canvasLayout, 1);

    // ---- Toolbar ----
    auto *toolbar = new QHBoxLayout;

    // QPushButton *colorBtn = new QPushButton("Color");
    // connect(colorBtn, &QPushButton::clicked, this, &DoodlePadWidget::chooseColor);
    // toolbar->addWidget(colorBtn);

    // QPushButton *eraserBtn = new QPushButton("Eraser");
    // connect(eraserBtn, &QPushButton::clicked, this, &DoodlePadWidget::useEraser);
    // toolbar->addWidget(eraserBtn);

    // QPushButton *undoBtn = new QPushButton("Undo");
    // connect(undoBtn, &QPushButton::clicked, this, &DoodlePadWidget::undo);
    // toolbar->addWidget(undoBtn);

    // QPushButton *redoBtn = new QPushButton("Redo");
    // connect(redoBtn, &QPushButton::clicked, this, &DoodlePadWidget::redo);
    // toolbar->addWidget(redoBtn);

    // QPushButton *clearBtn = new QPushButton("Clear");
    // connect(clearBtn, &QPushButton::clicked, this, &DoodlePadWidget::clearCanvas);
    // toolbar->addWidget(clearBtn);

    // QPushButton *saveBtn = new QPushButton("Save");
    // connect(saveBtn, &QPushButton::clicked, this, &DoodlePadWidget::saveImage);
    // toolbar->addWidget(saveBtn);

    // QPushButton *glowBtn = new QPushButton("Glow");
    // connect(glowBtn, &QPushButton::clicked, this, [this]() {
    //     brushMode = GlowBrush;
    //     erasing = false;
    // });
    // toolbar->addWidget(glowBtn);

    // QPushButton *smudgeBtn = new QPushButton("Smudge");
    // connect(smudgeBtn, &QPushButton::clicked, this, [this]() {
    //     brushMode = SmudgeBrush;
    //     erasing = false;
    // });
    // toolbar->addWidget(smudgeBtn);
    
    // QPushButton *sprayBtn = new QPushButton("Spray");
    // connect(sprayBtn, &QPushButton::clicked, this, [this]() {
    //     brushMode = SprayBrush;
    //     erasing = false;
    // });
    // toolbar->addWidget(sprayBtn);

    


    QSlider *sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setRange(1, 40);
    sizeSlider->setValue(penSize);
    connect(sizeSlider, &QSlider::valueChanged, this, &DoodlePadWidget::penSizeChanged);
    toolbar->addWidget(sizeSlider);

    canvasLayout->addLayout(toolbar);

    // ---- Right: Layer Panel ----
    auto *layerPanel = new QVBoxLayout;
    mainLayout->addLayout(layerPanel);

    layerList = new QListWidget;
    connect(layerList, &QListWidget::currentRowChanged,
            this, &DoodlePadWidget::layerSelectionChanged);
    layerPanel->addWidget(layerList);

    QPushButton *addLayerBtn = new QPushButton("Add Layer");
    connect(addLayerBtn, &QPushButton::clicked, this, &DoodlePadWidget::addLayer);
    layerPanel->addWidget(addLayerBtn);

    QPushButton *delLayerBtn = new QPushButton("Delete Layer");
    connect(delLayerBtn, &QPushButton::clicked, this, &DoodlePadWidget::deleteLayer);
    layerPanel->addWidget(delLayerBtn);

    QPushButton *toggleBtn = new QPushButton("Toggle Visibility");
    connect(toggleBtn, &QPushButton::clicked, this, &DoodlePadWidget::toggleLayerVisibility);
    layerPanel->addWidget(toggleBtn);

    // ---- Blend Mode Dropdown ----
blendBox = new QComboBox();
blendBox->addItems({"Normal", "Multiply", "Screen", "Overlay", "Additive"});

connect(blendBox, &QComboBox::currentIndexChanged, this, [this](int index) {
    layers[currentLayer].mode = static_cast<BlendMode>(index);
    update();
});

layerPanel->addWidget(blendBox);
    // ---- Initial Layer ----
    Layer base;
    base.pixmap = QPixmap(1600, 1200);
    base.pixmap.fill(Qt::white);
    base.visible = true;
    base.name = "Layer 1";
    layers.append(base);

    currentLayer = 0;
    refreshLayerList();
}

void DoodlePadWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    for (const Layer &layer : layers)
        if (layer.visible)
            painter.drawPixmap(0, 0, layer.pixmap);
}

void DoodlePadWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        drawing = true;
        lastPoint = event->position().toPoint();
        pushUndo();
    }
}

void DoodlePadWidget::mouseMoveEvent(QMouseEvent *event) {
    if (drawing && (event->buttons() & Qt::LeftButton))
        drawLineTo(event->position().toPoint());
}

void DoodlePadWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        drawing = false;
}

void DoodlePadWidget::drawLineTo(const QPoint &endPoint) {
    if (currentLayer < 0 || currentLayer >= layers.size())
        return;

    QPainter painter(&layers[currentLayer].pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (brushMode == GlowBrush) {
        for (int i = 0; i < 6; ++i) {
            QColor glow = penColor;
            glow.setAlpha(180 - i * 30);
            int size = penSize + i * 3;
            painter.setPen(QPen(glow, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(lastPoint, endPoint);
        }
    }
    else if (brushMode == SprayBrush) {
        int density = penSize * 4;

        for (int i = 0; i < density; ++i) {
            double angle = QRandomGenerator::global()->bounded(628318) / 100000.0;
            double radius = (QRandomGenerator::global()->bounded(100000) / 100000.0)
                            * (penSize * 1.5);

            int dx = static_cast<int>(std::cos(angle) * radius);
            int dy = static_cast<int>(std::sin(angle) * radius);

            QPoint p = endPoint + QPoint(dx, dy);

            QColor dot = penColor;
            dot.setAlpha(QRandomGenerator::global()->bounded(80, 180));

            painter.setPen(QPen(dot, 1, Qt::SolidLine, Qt::RoundCap));
            painter.drawPoint(p);
        }
    }
    else if (brushMode == SmudgeBrush) {

    int radius = penSize * 2;
    int diameter = radius * 2;

    // Sample the area around the cursor
    QImage area = layers[currentLayer].pixmap.copy(
        endPoint.x() - radius,
        endPoint.y() - radius,
        diameter,
        diameter
    ).toImage();

    // Blur the sampled area (simple box blur)
    QImage blurred = area;
    QPainter blurPainter(&blurred);
    blurPainter.setRenderHint(QPainter::Antialiasing, true);
    blurPainter.setOpacity(0.5);
    blurPainter.drawImage(0, 0, area);
    blurPainter.end();

    // Paint the blurred area back onto the canvas
    painter.drawImage(
        endPoint.x() - radius,
        endPoint.y() - radius,
        blurred
    );
}

    else { // Normal brush
        QColor useColor = erasing ? Qt::white : penColor;
        painter.setPen(QPen(useColor, penSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(lastPoint, endPoint);
    }

    lastPoint = endPoint;
    update();
}

void DoodlePadWidget::addLayer() {
    Layer l;
    l.pixmap = QPixmap(layers[0].pixmap.size());
    l.pixmap.fill(Qt::transparent);
    l.visible = true;
    l.name = QString("Layer %1").arg(layers.size() + 1);

    layers.append(l);
    refreshLayerList();
}

void DoodlePadWidget::deleteLayer() {
    if (layers.size() <= 1)
        return;

    layers.removeAt(currentLayer);
    currentLayer = qBound(0, currentLayer - 1, layers.size() - 1);
    refreshLayerList();
}

void DoodlePadWidget::toggleLayerVisibility() {
    layers[currentLayer].visible = !layers[currentLayer].visible;
    refreshLayerList();
    update();
}

void DoodlePadWidget::layerSelectionChanged() {
    int row = layerList->currentRow();
    if (row >= 0 && row < layers.size())
        currentLayer = row;
}

void DoodlePadWidget::refreshLayerList() {
    layerList->clear();
    for (const Layer &l : layers) {
        QString text = l.visible ? l.name : l.name + " (hidden)";
        layerList->addItem(text);
    }
    layerList->setCurrentRow(currentLayer);
}

void DoodlePadWidget::clearCanvas() {
    pushUndo();
    layers[currentLayer].pixmap.fill(Qt::transparent);
    update();
}

void DoodlePadWidget::penSizeChanged(int size) {
    penSize = size;
}

void DoodlePadWidget::chooseColor() {
    QColor chosen = QColorDialog::getColor(penColor, this);
    if (chosen.isValid()) {
        penColor = chosen;
        erasing = false;
    }
}

void DoodlePadWidget::useEraser() {
    erasing = true;
}

void DoodlePadWidget::pushUndo() {
    if (currentLayer < 0 || currentLayer >= layers.size())
        return;

    undoStack.push(layers[currentLayer].pixmap);
    redoStack.clear();
}

void DoodlePadWidget::undo() {
    if (!undoStack.isEmpty()) {
        redoStack.push(layers[currentLayer].pixmap);
        layers[currentLayer].pixmap = undoStack.pop();
        update();
    }
}

void DoodlePadWidget::redo() {
    if (!redoStack.isEmpty()) {
        undoStack.push(layers[currentLayer].pixmap);
        layers[currentLayer].pixmap = redoStack.pop();
        update();
    }
}

void DoodlePadWidget::saveImage() {
    QString file = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Files (*.png)");
    if (file.isEmpty())
        return;

    QPixmap final(layers[0].pixmap.size());
    final.fill(Qt::white);

    QPainter painter(&final);
    for (const Layer &l : layers)
        if (l.visible)
            painter.drawPixmap(0, 0, l.pixmap);

    final.save(file, "PNG");
}
