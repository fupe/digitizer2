#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColor>
#include "settings.h"

class QPen;
class QPushButton;
class SettingsManager;
class SerialManager;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr, SettingsManager* sm = nullptr, SerialManager* serial = nullptr);
    ~SettingsDialog();
    bool save_main_window_position_on_exit;
    bool save_measure_window_position_on_exit;
    QString port_name;  //*
    QRect main_window_position; //*
    QPen *arms_pen; //*
    QColor arms_color; //*
    double arm1_length;//*
    double arm2_length;//*
    double auto_step;//*
    double alfa_offset;//*
    double beta_offset;//*
    QString version;   //*
    bool document_modified = false;
    bool document_saved = false;
    QString units;  //*
    double units_scale;
    QString directory_save_dxf; //*
    QString directory_save_data;//*
    //shortcuts
    //ShortCutsDialog *shortcuts=nullptr;




    //measure

    QRect measure_window_position;


    //void load_Settings (void);
    //void save_Settings (void);
    void save_SettingsExitMain (void);
    void save_SettingsExitMeasure (void);
    void save_Arms(void);
    void retranslate (void);
    void changeunits (const QString&);
    void setSettings(const Settings& s); // předání kopie k editaci
    Settings result() const;             // vrátí zeditovanou kopii


public slots:


private slots:

    void unlockHiddenTab();
    void showPortInfo(int idx);
    //void buildUi(void);
    void pullFromUi (void);
    void on_choose_arms_color_clicked();

    void on_buttonBox_rejected();

    void on_ComboBox_language_currentIndexChanged(const QString &arg1);

    void on_ShortCuts_clicked();

    void populate();

    void on_buttonBox_accepted();

    void on_button_browse_dxf_clicked();

    void onImportConfig();
    void onExportConfig();
    void on_button_browse_simul_clicked();
    void on_pushButton_logging_enagle_toggled(bool checked);
    void on_unit_select_currentIndexChanged(const QString &text);

private:
    Ui::SettingsDialog *ui;
    void fillPortsInfo();
    void assignLanguageCodes();
    Settings tmp_settings ;// lokální pracovní kopie
    Settings orig_settings_ ;// původní nastavení pro reset
    Units currentUnits_ = Units::Millimeters;
    int hiddenTabIndex_ = -1;  //index skryte zalozky
    QWidget* hiddenTabWidget_ = nullptr; //ukazatel skryte zalozky
    SettingsManager* sm_ = nullptr;
    QPushButton* importButton_ = nullptr;
    QPushButton* exportButton_ = nullptr;
    SerialManager* serialManager_ = nullptr;

signals:
    void signal_scene(void);
    void signal_reconnect(void);
    void signal_retranslate (void);
    //void documentModifiedChanged(bool newValue);

};

#endif // MYSETTINGS_H
