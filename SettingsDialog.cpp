#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "shortcutsdialog.h"
#include "settingsmanager.h"
#include <QSerialPortInfo>
#include <QColor>
#include <QFileDialog>
#include <QShortcut>
#include <QKeySequence>
#include <QSettings>
#include <QColorDialog>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>



static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent, SettingsManager* sm) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    sm_(sm)
{
    ui->setupUi(this);
    // schovej tab2 hned po startu
        hiddenTabWidget_ = ui->tabWidget->widget(1);   // uložíme widget tab2
        hiddenTabIndex_ = 1;
        ui->tabWidget->removeTab(1);

        // tajná klávesová zkratka Ctrl+Alt+S
        QShortcut* sc = new QShortcut(QKeySequence("Ctrl+Alt+S"), this);
        connect(sc, &QShortcut::activated, this, &SettingsDialog::unlockHiddenTab);

    if (sm_) {
        importButton_ = ui->buttonBox->addButton(tr("Import Config"), QDialogButtonBox::ActionRole);
        exportButton_ = ui->buttonBox->addButton(tr("Export Config"), QDialogButtonBox::ActionRole);
        connect(importButton_, &QPushButton::clicked, this, &SettingsDialog::onImportConfig);
        connect(exportButton_, &QPushButton::clicked, this, &SettingsDialog::onExportConfig);
    }
    // změna vybraného sériového portu
        connect(ui->serialPortInfoListBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &SettingsDialog::showPortInfo);
    // přepnutí filtru “digit port only”
        connect(ui->checkBox_digit_port_only,
                &QCheckBox::toggled,
                this,
                &SettingsDialog::fillPortsInfo);

    populate();

    fillPortsInfo();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/*void SettingsDialog::load_Settings()
{
    QSettings  setting("Digitizer","app");
    this->port_name = setting.value("port_name","COM1").toString();
    qDebug() << "port name loaded " << port_name ;
    this->directory_save_dxf = setting.value("directory_save_dxf","c:/").toString();
    this->directory_save_data = setting.value("directory_save_data","c:/").toString();
        //ui->serialPortInfoListBox->setCurrentText(this->port_name);
    this->version = setting.value("version","serial").toString();
    qDebug() << "version from reg " << version ;
    setting.beginGroup("main_window");
    main_window_position = setting.value("main_window_position").toRect();
    save_main_window_position_on_exit = setting.value("save_main_window_position_on_exit",true).toBool();
    //qDebug() << "save_main_window_position_on_exit:" << setting.value("save_main_window_position_on_exit",true).toBool();
    setting.endGroup();
    setting.beginGroup("measure_window");
    measure_window_position = setting.value("measure_window_position").toRect();
    save_measure_window_position_on_exit = setting.value("save_measure_window_position_on_exit",true).toBool();
    qDebug() << "save_measure_window_position_on_exit:" << setting.value("save_measure_window_position_on_exit",true).toBool();
    setting.endGroup();
    arms_color = QColor(setting.value("arms_color").toString());
    //qDebug() <<"arms color  load" << arms_color;
    arms_pen->setColor(arms_color);
        //ui->arms_color->setAutoFillBackground(true);
    QPalette pal = ui->arms_color->palette();
    pal.setColor(QPalette::Window, arms_color);
        //ui->arms_color->setPalette(pal);
    setting.beginGroup("shortcuts");
    shortcuts->set_shortcut_addpoint(QString(setting.value("addpoint","\\").toString()));
    shortcuts->set_shortcut_addpolyline(QString(setting.value("addpolyline","*").toString()));
    shortcuts->set_shortcut_measure(QString(setting.value("measure","-").toString()));
    shortcuts->set_shortcut_back(QString(setting.value("back","1").toString()));
    shortcuts->set_shortcut_continous(QString(setting.value("continous","Backspace").toString()));
    shortcuts->set_shortcut_exportdxf(QString(setting.value("exportdxf","Enter").toString()));
    shortcuts->set_shortcut_clear(QString(setting.value("clear","4").toString()));
    shortcuts->set_shortcut_zoom(QString(setting.value("zoom","+").toString()));


    setting.endGroup();


    alfa_offset = double(setting.value("alfa_offset",0).toDouble ());
    beta_offset = double(setting.value("beta_offset",0).toDouble ());
    this->units = setting.value("units","mm").toString();
        //ui->unit_select->setCurrentText(this->units);

    if (units=="mm")
    {
        units_scale=1.0;
    }
    else
    {
        units_scale=25.4;
    }
    qDebug() << "unit scale " << units_scale;

    arm1_length = double(setting.value("arm1_length",600).toDouble ());
    arm2_length = double(setting.value("arm2_length",500).toDouble ());
    auto_step = double(setting.value("auto_step",10).toDouble ());
    changeunits(this->units);

    this->language = setting.value("language","English").toString();
        //ui->ComboBox_language->setCurrentText(language);

        //fillPortsInfo();
}
*/
void SettingsDialog::save_Arms(void)
{
    QSettings  settings("Digitizer","app");
    QString arm1_length_string = QString::number(arm1_length);
    settings.setValue("arm1_length",arm1_length_string  );

    QString arm2_length_string = QString::number(arm2_length);
    settings.setValue("arm2_length",arm2_length_string  );

}
/*
void SettingsDialog::save_Settings()
{
    QSettings  settings("Digitizer","app");

    settings.setValue("port_name",this->port_name);
    settings.setValue("directory_save_dxf",this->directory_save_dxf);
    settings.setValue("directory_save_data",this->directory_save_data);
    settings.setValue("version",this->version);
    settings.beginGroup("main_window");
    settings.setValue("save_main_window_position_on_exit",this->save_main_window_position_on_exit);
    qDebug() << "save_main_window_position_on_exit:" << this->save_main_window_position_on_exit;
    settings.endGroup();
    settings.beginGroup("measure_window");
    settings.setValue("save_measure_window_position_on_exit",this->save_measure_window_position_on_exit);
    qDebug() << "save_measure_window_position_on_exit:"  << this->save_measure_window_position_on_exit;
    settings.endGroup();
    settings.setValue("arms_color",QColor(arms_color).name());

    QString arm1_length_string = QString::number(arm1_length);
    //qDebug() << "arm1_length save" << arm1_length ;
    settings.setValue("arm1_length",arm1_length_string  );

    QString arm2_length_string = QString::number(arm2_length);
    settings.setValue("arm2_length",arm2_length_string  );

    QString auto_step_string = QString::number(auto_step);
    settings.setValue("auto_step",auto_step_string  );

    QString alfa_offset_string = QString::number(alfa_offset);
    settings.setValue("alfa_offset",alfa_offset_string  );

    QString beta_offset_string = QString::number(beta_offset);
    settings.setValue("beta_offset",beta_offset_string  );

    settings.setValue("units",this->units);


    settings.setValue("language",this->language);

    settings.beginGroup("shortcuts");
    settings.setValue("addpoint",shortcuts->get_shortcut_addpoint());
    settings.setValue("addpolyline",shortcuts->get_shortcut_addpolyline());
    settings.setValue("measure",shortcuts->get_shortcut_measure());
    settings.setValue("back",shortcuts->get_shortcut_back());
    settings.setValue("continous",shortcuts->get_shortcut_continous());
    settings.setValue("exportdxf",shortcuts->get_shortcut_exportdxf());
    settings.setValue("clear",shortcuts->get_shortcut_clear());
    settings.setValue("zoom",shortcuts->get_shortcut_zoom());
    settings.endGroup();

    qDebug() << "saved" ;

}
*/
void SettingsDialog::save_SettingsExitMain (void)
{
    qDebug() << "ukladam pozici okna" ;
    QSettings  setting("Digitizer","app");
    setting.beginGroup("main_window");
    if (this->save_main_window_position_on_exit)
    {
        qDebug() << "ukladam pozici okna main je zaskrt" ;
        setting.setValue("main_window_position",main_window_position);
    }
    setting.endGroup();
}

void SettingsDialog::save_SettingsExitMeasure (void)
{
    qDebug() << "ukladam pozici okna measure" ;
    QSettings  setting("Digitizer","app");
    setting.beginGroup("measure_window");
    if (this->save_measure_window_position_on_exit)
    {
        qDebug() << "ukladam pozici okna measure je zaskrt" ;
        setting.setValue("measure_window_position",measure_window_position);
    }
    setting.endGroup();
}

void SettingsDialog::retranslate()
{
    qDebug() <<"SettingsDialog::retranslate() " ;
    ui->retranslateUi(this);
    ui->ComboBox_language->setCurrentText(language);

    fillPortsInfo();

}

void SettingsDialog::changeunits(const QString& jednotky)
{
    //qDebug() << "necum " << jednotky;
    ui->label_arm1_units->setText(jednotky);
    ui->label_arm2_units->setText(jednotky);
    ui->label_step_units->setText(jednotky);

    if (jednotky=="mm")
    {
        units_scale=1.0;
        ui->doubleSpinBox_arm1_length->setDecimals(2);
        ui->doubleSpinBox_arm2_length->setDecimals(2);
        ui->auto_step->setDecimals(2);
        ui->auto_step->setSingleStep(1);
    }
    else
    {
        units_scale=25.4;
        ui->doubleSpinBox_arm1_length->setDecimals(3);
        ui->doubleSpinBox_arm2_length->setDecimals(3);
        ui->auto_step->setDecimals(3);
        ui->auto_step->setSingleStep(0.1);
    }
    qDebug() << "units_scale " << units_scale;
    ui->doubleSpinBox_arm1_length->setValue(this->arm1_length/units_scale);
    ui->doubleSpinBox_arm2_length->setValue(this->arm2_length/units_scale);
    ui->auto_step->setValue(this->auto_step/units_scale);

}

void SettingsDialog::showPortInfo(int idx)
{

    qDebug()<<"showportinfo " << idx;
    if (idx == -1)
        return;

    const QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    //m_ui->serialPortInfoListBox->setCurrentIndex(2);
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));


    QString pid=list.count() > 6 ? list.at(6) : tr(blankString);
    //qDebug() << "pid" << pid;
    if (pid=="5740")
    {
        ui->versionLabel->setText("serial");
    }
    else if (pid=="7523")
    {
        ui->versionLabel->setText("modbus");
    }
    else
    {
         ui->versionLabel->setText("none");
    }

}

void SettingsDialog::fillPortsInfo()
{
    qDebug() << "fill port info" ;
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    QString portName;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        portName = info.portName();
        list << info.portName()
             << (!description.isEmpty() ? description : tr(blankString))
             << (!manufacturer.isEmpty() ? manufacturer : tr(blankString))
             << (!serialNumber.isEmpty() ? serialNumber : tr(blankString))
             << info.portName()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
        if (ui->checkBox_digit_port_only->isChecked())
        {
                if ( info.productIdentifier() == 0x7523 or info.productIdentifier() == 0x5740) {
                    ui->serialPortInfoListBox->addItem(list.first(), list);

                }
        }
        else
        {
            ui->serialPortInfoListBox->addItem(list.first(), list);
        }

    }

}

void SettingsDialog::on_choose_arms_color_clicked()
{
    const QColor start = tmp_settings.arms_color.isValid() ? tmp_settings.arms_color : QColor(Qt::white);
    QColor c = QColorDialog::getColor(start, this, tr("Choose color"), QColorDialog::ShowAlphaChannel);
    if (!c.isValid()) {
        qDebug() << "Color pick canceled";
        return;
    }
    // ulož do modelu dialogu

    tmp_settings.arms_color = c;
    // náhled (předpokládám, že ui->arms_color je QLabel/QFrame/QWidget)
    ui->arms_color->setStyleSheet(
        QString("background-color:%1; border:1px solid #888;")
            .arg(c.name(QColor::HexArgb)));
}


void SettingsDialog::on_buttonBox_rejected()
{
    qDebug() << "reject";
    this->language=this->language_tmp;
    emit signal_retranslate();
    reject();

}


void SettingsDialog::on_ComboBox_language_activated(const QString &arg1)
{
    qDebug() << "on_ComboBox_language_activated " << arg1 ;
    this->language_tmp=this->language;
    this->language=arg1;
    emit signal_retranslate();
}

void SettingsDialog::on_ShortCuts_clicked()
{
    qDebug() << "short clicked";
    ShortCutsDialog dlg(this);
    dlg.setShortcuts(tmp_settings.shortcuts);      // naplnit z aktuálních settings
    if (dlg.exec() == QDialog::Accepted) { // po OK převzít změny
        tmp_settings.shortcuts = dlg.shortcuts();
        // nic dalšího není potřeba – uloží se až při OK tohoto SettingsDialogu
    }
}



void SettingsDialog::setSettings(const Settings& s) {
    tmp_settings = s;
    populate();
}

Settings SettingsDialog::result() const {
    Settings out = tmp_settings;
    // … pokud čteš hodnoty z UI až tady, propsat do out
    return out;
}

void SettingsDialog::populate() {
    // TODO: Optionally enumerate available ports via QSerialPortInfo
    ui->auto_step->setValue(tmp_settings.auto_step);
    ui->unit_select->setCurrentText(unitsToString(tmp_settings.units));
    ui->doubleSpinBox_arm1_length->setValue(tmp_settings.arm1_length);
    ui->doubleSpinBox_arm2_length->setValue(tmp_settings.arm2_length);

    // náhled barvy
        ui->arms_color->setAttribute(Qt::WA_StyledBackground, true);
        ui->arms_color->setMinimumSize(32, 20);

        const QColor c = tmp_settings.arms_color.isValid() ? tmp_settings.arms_color : QColor(Qt::white);
        ui->arms_color->setStyleSheet(
            QStringLiteral("background-color:%1; border:1px solid #888; border-radius:6px;")
                .arg(c.name(QColor::HexArgb)));

        qDebug() << "populate color =" << c.name(QColor::HexArgb);
 //UI
        ui->checkBox_main_position_save_on_exit->setChecked(tmp_settings.save_main_window_position_on_exit);
        ui->checkBox_measure_position_save_on_exit->setChecked(tmp_settings.save_measure_window_position_on_exit);
        ui->language->setText(tmp_settings.language);
//---------SERIAL---------
        ui->serialPortInfoListBox->setCurrentText(tmp_settings.serial.portName);
        ui->comboBox_datasource->setCurrentIndex(static_cast<int>(tmp_settings.datasource));
//----dxf dir
        ui->lineEdit_dxf_dir->setText(tmp_settings.directory_save_dxf);


}


/*void SettingsDialog::buildUi()
{
    populate();
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::pullFromUi);
}*/

void SettingsDialog::pullFromUi()
{
   qDebug()<<"accept v settings dialog" << ui->unit_select->currentText();
   tmp_settings.auto_step=ui->auto_step->value();
   tmp_settings.units=stringToUnits(ui->unit_select->currentText());
   tmp_settings.arm1_length=ui->doubleSpinBox_arm1_length->value();
   tmp_settings.arm2_length=ui->doubleSpinBox_arm2_length->value();
   //tmp_settings.arms_color=  QColor(arms_color).name();

   //UI
   tmp_settings.save_main_window_position_on_exit=ui->checkBox_main_position_save_on_exit->isChecked();
   qDebug()<<"pull save_main_window_position_on_exit " << tmp_settings.save_main_window_position_on_exit;
   tmp_settings.save_measure_window_position_on_exit=ui->checkBox_measure_position_save_on_exit->isChecked();
   tmp_settings.language=ui->language->text();

   //------------------SERIAL------------
   tmp_settings.serial.portName = ui->serialPortInfoListBox->currentText();
   tmp_settings.datasource = static_cast<DataSource>(ui->comboBox_datasource->currentIndex());

   //---dxf
   tmp_settings.directory_save_dxf = ui->lineEdit_dxf_dir->text();
}

void SettingsDialog::on_buttonBox_accepted()
{
    pullFromUi();
    this->accept();
}


void SettingsDialog::on_button_browse_dxf_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select DXF Export Directory"),
        ui->lineEdit_dxf_dir->text().isEmpty() ? QDir::homePath() : ui->lineEdit_dxf_dir->text()
    );

    if (!dir.isEmpty()) {
        ui->lineEdit_dxf_dir->setText(dir);
        tmp_settings.directory_save_dxf = dir;  // hned uložit do dočasných settings
    }
}

void SettingsDialog::onImportConfig()
{
    if (!sm_) return;

    const QString defDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Import settings from JSON"),
        QDir(defDir).filePath("settings.json"),
        tr("JSON files (*.json)"));
    if (path.isEmpty()) return;

    QString err;
    if (!sm_->importJson(path, &err)) {
        QMessageBox::warning(this, tr("Import failed"),
                             tr("Cannot import settings:\n%1").arg(err));
        return;
    }
    setSettings(sm_->currentSettings());
    QMessageBox::information(this, tr("Import settings"),
                             tr("Settings imported from %1")
                                 .arg(QDir::toNativeSeparators(path)));
}

void SettingsDialog::onExportConfig()
{
    if (!sm_) return;

    pullFromUi();

    const QString defDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Export settings to JSON"),
        QDir(defDir).filePath("settings.json"),
        tr("JSON files (*.json)"));
    if (path.isEmpty()) return;

    QString err;
    if (!sm_->exportJson(path, tmp_settings, &err)) {
        QMessageBox::warning(this, tr("Export failed"),
                             tr("Cannot export settings:\n%1").arg(err));
        return;
    }
    QMessageBox::information(this, tr("Export settings"),
                             tr("Settings exported to %1")
                                 .arg(QDir::toNativeSeparators(path)));
}

void SettingsDialog::unlockHiddenTab()
{
    if (hiddenTabWidget_ && hiddenTabIndex_ != -1) {
        ui->tabWidget->insertTab(hiddenTabIndex_, hiddenTabWidget_, tr("Security"));
        ui->tabWidget->setCurrentIndex(hiddenTabIndex_);
        hiddenTabIndex_ = -1; // už není schovaný
    }
}
