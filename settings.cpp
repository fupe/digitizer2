// settings.cpp
#include "settings.h"
#include <QKeySequence>

Shortcuts Shortcuts::defaults()
{
    Shortcuts s;
    s.map[QStringLiteral("action.addPoint")]     = QKeySequence::fromString(QStringLiteral("Ctrl+O"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.polyline")]     = QKeySequence::fromString(QStringLiteral("Ctrl+S"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.measure")]      = QKeySequence::fromString(QStringLiteral("P"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.clear")]        = QKeySequence::fromString(QStringLiteral("L"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.continous")]    = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.zoom")]         = QKeySequence::fromString(QStringLiteral("L"),      QKeySequence::PortableText);
    s.map[QStringLiteral("action.back")]         = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    s.map[QStringLiteral("action.exportdxf")]         = QKeySequence::fromString(QStringLiteral("Ctrl+0"), QKeySequence::PortableText);
    return s;
}


/*
 *  ui->keySequenceEdit_addpoint   ->setKeySequence(tmp_.map.value(QStringLiteral("action.addPoint")));
    ui->keySequenceEdit_addpolyline->setKeySequence(tmp_.map.value(QStringLiteral("action.polyline")));
    ui->keySequenceEdit_measure    ->setKeySequence(tmp_.map.value(QStringLiteral("action.measure")));
    ui->keySequenceEdit_clear      ->setKeySequence(tmp_.map.value(QStringLiteral("action.clear")));
    ui->keySequenceEdit_continous  ->setKeySequence(tmp_.map.value(QStringLiteral("action.continous")));
    ui->keySequenceEdit_zoom       ->setKeySequence(tmp_.map.value(QStringLiteral("action.zoom")));
    ui->keySequenceEdit_back       ->setKeySequence(tmp_.map.value(QStringLiteral("action.back")));
    ui->keySequenceEdit_exportdxf  ->setKeySequence(tmp_.map.value(QStringLiteral("action.exportdxf")));
    */
