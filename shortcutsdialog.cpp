#include "shortcutsdialog.h"
#include "ui_shortcutsdialog.h"      // přesný název podle .ui
#include <QKeySequenceEdit>
#include <QPushButton>

ShortCutsDialog::ShortCutsDialog(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::ShortCutsDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Shortcuts"));

    // Reset
    connect(ui->pushButton_reset_to_default, &QPushButton::clicked,
            this, &ShortCutsDialog::resetToDefaults);
}

ShortCutsDialog::~ShortCutsDialog() {
    delete ui;
}

void ShortCutsDialog::setShortcuts(const Shortcuts& s) {
    tmp_ = s;
    populate();
}

Shortcuts ShortCutsDialog::shortcuts() const {
    return tmp_;
}

void ShortCutsDialog::populate() {
    // naplnění editorů ze slovníku
    ui->keySequenceEdit_addpoint   ->setKeySequence(tmp_.map.value(QStringLiteral("action.addPoint")));
    ui->keySequenceEdit_addpolyline->setKeySequence(tmp_.map.value(QStringLiteral("action.polyline")));
    ui->keySequenceEdit_measure    ->setKeySequence(tmp_.map.value(QStringLiteral("action.measure")));
    ui->keySequenceEdit_clear      ->setKeySequence(tmp_.map.value(QStringLiteral("action.clear")));
    ui->keySequenceEdit_continous  ->setKeySequence(tmp_.map.value(QStringLiteral("action.continous")));
    ui->keySequenceEdit_zoom       ->setKeySequence(tmp_.map.value(QStringLiteral("action.zoom")));
    ui->keySequenceEdit_back       ->setKeySequence(tmp_.map.value(QStringLiteral("action.back")));
    ui->keySequenceEdit_exportdxf  ->setKeySequence(tmp_.map.value(QStringLiteral("action.exportdxf")));
}

void ShortCutsDialog::pullFromUi() {
    // sebrání hodnot z UI zpět do modelu
    tmp_.map[QStringLiteral("action.addPoint")]   = ui->keySequenceEdit_addpoint   ->keySequence();
    tmp_.map[QStringLiteral("action.polyline")]   = ui->keySequenceEdit_addpolyline->keySequence();
    tmp_.map[QStringLiteral("action.measure")]    = ui->keySequenceEdit_measure    ->keySequence();
    tmp_.map[QStringLiteral("action.clear")]      = ui->keySequenceEdit_clear      ->keySequence();
    tmp_.map[QStringLiteral("action.continous")]  = ui->keySequenceEdit_continous  ->keySequence();
    tmp_.map[QStringLiteral("action.zoom")]       = ui->keySequenceEdit_zoom       ->keySequence();
    tmp_.map[QStringLiteral("action.back")]       = ui->keySequenceEdit_back       ->keySequence();
    tmp_.map[QStringLiteral("action.exportdxf")]  = ui->keySequenceEdit_exportdxf  ->keySequence();
}

void ShortCutsDialog::accept() {
    pullFromUi();
    QDialog::accept();
}

void ShortCutsDialog::resetToDefaults() {
    qDebug()<<"reset to defaults";
    const Shortcuts def = Shortcuts::defaults();
    ui->keySequenceEdit_addpoint   ->setKeySequence(def.map.value(QStringLiteral("action.addPoint")));
    ui->keySequenceEdit_addpolyline->setKeySequence(def.map.value(QStringLiteral("action.polyline")));
    ui->keySequenceEdit_measure    ->setKeySequence(def.map.value(QStringLiteral("action.measure")));
    ui->keySequenceEdit_clear      ->setKeySequence(def.map.value(QStringLiteral("action.clear")));
    ui->keySequenceEdit_continous  ->setKeySequence(def.map.value(QStringLiteral("action.continous")));
    ui->keySequenceEdit_zoom       ->setKeySequence(def.map.value(QStringLiteral("action.zoom"))); // nebo zoomAll
    ui->keySequenceEdit_back       ->setKeySequence(def.map.value(QStringLiteral("action.back")));
    ui->keySequenceEdit_exportdxf  ->setKeySequence(def.map.value(QStringLiteral("action.exportdxf")));
}
