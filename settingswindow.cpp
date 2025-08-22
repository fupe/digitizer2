#include "settingswindow.h"
#include "shortcutsdialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
// #include <QSerialPortInfo> // if you want to enumerate ports

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent) {
    buildUi();
}

void SettingsWindow::buildUi() {
    auto* lay = new QFormLayout(this);

    comboPort_     = new QComboBox(this);
    comboBaud_     = new QComboBox(this);
    comboUnits_    = new QComboBox(this);
    spinAnimMs_    = new QSpinBox(this);
    chkAA_         = new QCheckBox(QStringLiteral("Antialiasing"), this);
    btnShortcuts_  = new QPushButton(QStringLiteral("Zkratky…"), this);

    comboUnits_->addItems({QStringLiteral("mm"), QStringLiteral("inch")});
    comboBaud_->addItems({QStringLiteral("9600"), QStringLiteral("19200"),
                          QStringLiteral("38400"), QStringLiteral("57600"),
                          QStringLiteral("115200")});
    spinAnimMs_->setRange(50, 2000);

    lay->addRow(QStringLiteral("Port:"),        comboPort_);
    lay->addRow(QStringLiteral("Baud:"),        comboBaud_);
    lay->addRow(QStringLiteral("Jednotky:"),    comboUnits_);
    lay->addRow(QStringLiteral("Animace (ms):"),spinAnimMs_);
    lay->addRow(chkAA_);
    lay->addRow(btnShortcuts_);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addRow(bb);

    connect(bb, &QDialogButtonBox::accepted, this, [this]{ pullFromUi(); accept(); });
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnShortcuts_, &QPushButton::clicked, this, &SettingsWindow::onShortcutsClicked);
}

void SettingsWindow::setSettings(const Settings& s) {
    tmp_ = s;
    populate();
}

void SettingsWindow::populate() {
    // TODO: Optionally enumerate available ports via QSerialPortInfo
    if (comboPort_->findText(tmp_.portName) < 0 && !tmp_.portName.isEmpty())
        comboPort_->addItem(tmp_.portName);
    comboPort_->setCurrentText(tmp_.portName);
    comboBaud_->setCurrentText(QString::number(tmp_.baudRate));
    comboUnits_->setCurrentText(unitsToString(tmp_.units));
}

void SettingsWindow::pullFromUi() {
    tmp_.portName    = comboPort_->currentText();
    tmp_.baudRate    = comboBaud_->currentText().toInt();
    tmp_.units       = stringToUnits(comboUnits_->currentText());
   }

Settings SettingsWindow::settings() const {
    return tmp_;
}

void SettingsWindow::onShortcutsClicked() {
    ShortcutsDialog dlg(this);
    dlg.setShortcuts(tmp_.shortcuts);
    if (dlg.exec() == QDialog::Accepted)
        tmp_.shortcuts = dlg.shortcuts();
}
