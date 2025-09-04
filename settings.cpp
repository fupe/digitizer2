// settings.cpp
#include "settings.h"
#include <QKeySequence>

Shortcuts Shortcuts::defaults()
{
    Shortcuts s;
    s.map[QStringLiteral("action.addPoint")]     = QKeySequence::fromString(QStringLiteral("Ctrl+O"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.polyline")]     = QKeySequence::fromString(QStringLiteral("Ctrl+S"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.circle")]       = QKeySequence::fromString(QStringLiteral("Ctrl+f"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.measure")]      = QKeySequence::fromString(QStringLiteral("P"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.newfile")]        = QKeySequence::fromString(QStringLiteral("L"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.continous")]    = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.zoom")]         = QKeySequence::fromString(QStringLiteral("L"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.back")]         = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.exportdxf")]    = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    return s;
}


