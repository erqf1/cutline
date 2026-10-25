#pragma once
#include <QImage>
#include <QStringList>
#include "model.h"

// Mitgelieferte Spezial-Schriften laden (einmal beim Start)
void registerSpecialFonts();
QStringList specialFonts();  // Familiennamen der mitgelieferten Schriften
QString defaultTextFont();

// Text-Element als Bild in genau dieser Größe (Vorschau und Export nutzen dasselbe Bild).
// Die Schrift wird so groß gewählt, dass der Text den Rahmen ausfüllt.
QImage renderTextImage(const Item& it, QSize size);

// Zeichnung oder Form als Bild in dieser Größe; canvasH = Höhe der Bildfläche in Pixeln (für die Strichstärke)
QImage renderGraphicImage(const Item& it, QSize size, double canvasH);
// Bild für Text, Zeichnung oder Form (Export und Vorschau)
QImage renderItemImage(const Item& it, QSize size, double canvasH);
