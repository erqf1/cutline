#pragma once
#include <QImage>
#include <QRectF>
#include "model.h"

// Vorschaubilder einer Videodatei für die Timeline
struct ThumbSet {
    double step = 1.0;
    QList<QImage> imgs;
};

// Schnittstelle, über die Timeline und Overlays mit dem Hauptfenster reden.
class EditorHost {
public:
    virtual ~EditorHost() = default;
    virtual Project& pr() = 0;
    virtual double curTime() const = 0;
    virtual int selPiece() const = 0;
    virtual int selItem() const = 0;   // Item-ID, 0 = keins
    virtual int selAudio() const = 0;  // Ton-ID, 0 = keins
    virtual void selectPiece(int i) = 0;
    virtual void selectItem(int id) = 0;
    virtual void selectAudio(int id) = 0;
    virtual void pushUndo() = 0;
    virtual void modelEdited(bool keepTime = false) = 0;
    virtual void seek(double t) = 0;
    virtual double viewScale() const = 0;
    virtual int timelineViewportWidth() const = 0;
    virtual const ThumbSet* thumbs(int srcIndex) const = 0;
    virtual QImage frameImage() = 0;   // aktuelles Videobild (für echte Unschärfe in der Vorschau)
    virtual QRectF videoRect() const = 0;  // Bereich des Videos in Szenenkoordinaten
    virtual bool showGuides() const = 0;   // Rahmen/Griffe anzeigen (nicht im Vollbild)
};
