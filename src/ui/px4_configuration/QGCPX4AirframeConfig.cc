#include <QMessageBox>
#include <QDebug>

#include "QGCPX4AirframeConfig.h"
#include "ui_QGCPX4AirframeConfig.h"

#include "UASManager.h"
#include "UAS.h"

QGCPX4AirframeConfig::QGCPX4AirframeConfig(QWidget *parent) :
    QWidget(parent),
    mav(NULL),
    selectedId(-1),
    ui(new Ui::QGCPX4AirframeConfig)
{
    ui->setupUi(this);

    // Fill the lists here manually in accordance with the list from:
    // https://github.com/PX4/Firmware/blob/master/ROMFS/px4fmu_common/init.d/rcS

    ui->planeComboBox->addItem(tr("Multiplex Easystar 1/2"), 100);
    ui->planeComboBox->addItem(tr("Hobbyking Bixler 1/2"), 101);

    connect(ui->planePushButton, SIGNAL(clicked()), this, SLOT(planeSelected()));
    connect(ui->planeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(planeSelected(int)));

    ui->flyingWingComboBox->addItem(tr("Bormatec Camflyer Q"), 30);
    ui->flyingWingComboBox->addItem(tr("Phantom FPV"), 31);

    connect(ui->flyingWingPushButton, SIGNAL(clicked()), this, SLOT(flyingWingSelected()));
    connect(ui->flyingWingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(flyingWingSelected(int)));

    ui->quadXComboBox->addItem(tr("Standard 10\" Quad"), 1);
    ui->quadXComboBox->addItem(tr("DJI F330 8\" Quad"), 10);
    ui->quadXComboBox->addItem(tr("Turnigy Talon v2 X550 Quad"), 666);

    connect(ui->quadXPushButton, SIGNAL(clicked()), this, SLOT(quadXSelected()));
    connect(ui->quadXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(quadXSelected(int)));

    connect(ui->quadPlusPushButton, SIGNAL(clicked()), this, SLOT(quadPlusSelected()));
    connect(ui->quadPlusComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(quadPlusSelected(int)));

    connect(ui->hexaXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(hexaXSelected(int)));

    connect(ui->hexaPlusComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(hexaPlusSelected(int)));

    connect(ui->octoXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(octoXSelected(int)));

    connect(ui->octoPlusComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(octoPlusSelected(int)));

    connect(ui->hComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(hSelected(int)));

    ui->hComboBox->addItem(tr("TBS Discovery"), 15);
    ui->hComboBox->addItem(tr("H Custom"), 16);

    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(applyAndReboot()));

    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS()));

    setActiveUAS(UASManager::instance()->getActiveUAS());
}

void QGCPX4AirframeConfig::parameterChanged(int uas, int component, QString parameterName, QVariant value)
{
    Q_UNUSED(uas);
    Q_UNUSED(component);

    if (parameterName.contains("SYS_AUTOSTART"))
    {
        int index = value.toInt();
        if (index > 0) {
            setAirframeID(index);
            ui->statusLabel->setText(tr("Onboard start script ID: #%1").arg(index));
        } else {
            ui->statusLabel->setText(tr("System not configured for autostart."));
        }
    }
}

void QGCPX4AirframeConfig::setActiveUAS(UASInterface* uas)
{
    if (mav)
    {
        disconnect(mav, SIGNAL(parameterChanged(int,int,QString,QVariant)), this, SLOT(parameterChanged(int,int,QString,QVariant)));
        mav = NULL;
    }

    if (!uas)
        return;

    mav = uas;
    paramMgr = mav->getParamManager();

    connect(mav, SIGNAL(parameterChanged(int,int,QString,QVariant)), this, SLOT(parameterChanged(int,int,QString,QVariant)));
}

void QGCPX4AirframeConfig::uncheckAll()
{
    ui->planePushButton->setChecked(false);
    ui->flyingWingPushButton->setChecked(false);
    ui->quadXPushButton->setChecked(false);
    ui->quadPlusPushButton->setChecked(false);
    ui->hexaXPushButton->setChecked(false);
    ui->hexaPlusPushButton->setChecked(false);
    ui->octoXPushButton->setChecked(false);
    ui->octoPlusPushButton->setChecked(false);
    ui->hPushButton->setChecked(false);
}

void QGCPX4AirframeConfig::setAirframeID(int id)
{
    selectedId = id;
    qDebug() << "setAirframeID" << selectedId;
    ui->statusLabel->setText(tr("Ground start script ID: #%1").arg(selectedId));

    // XXX too much boilerplate code here - this widget is really just
    // a quick hack to get started
    uncheckAll();

    if (id > 0 && id < 15) {
        ui->quadXPushButton->setChecked(true);
        ui->statusLabel->setText(tr("Selected quad X (ID: #%1)").arg(selectedId));
    }
    else if (id >= 15 && id < 20)
    {
        ui->hPushButton->setChecked(true);
    }
    else if (id >= 30 && id < 50)
    {
        ui->flyingWingPushButton->setChecked(true);
        ui->statusLabel->setText(tr("Selected flying wing (ID: #%1)").arg(selectedId));
    }
    else if (id >= 100 && id < 150)
    {
        ui->planePushButton->setChecked(true);
        ui->statusLabel->setText(tr("Selected plane (ID: #%1)").arg(selectedId));
    }
}

void QGCPX4AirframeConfig::applyAndReboot()
{
    // Guard against the case of an edit where we didn't receive all params yet
    if (selectedId <= 0)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("No airframe selected"));
        msgBox.setInformativeText(tr("Please select an airframe first."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        (void)msgBox.exec();

        return;
    }

    if (!mav)
        return;

    if (paramMgr->countOnboardParams() == 0 &&
            paramMgr->countPendingParams() == 0)
    {
        paramMgr->requestParameterListIfEmpty();
        QGC::SLEEP::msleep(100);
    }

    // Guard against the case of an edit where we didn't receive all params yet
    if (paramMgr->countPendingParams() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Parameter sync with UAS not yet complete"));
        msgBox.setInformativeText(tr("Please wait a few moments and retry"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        (void)msgBox.exec();

        return;
    }

    QList<int> components = paramMgr->getComponentForParam("SYS_AUTOSTART");

    // Guard against multiple components responding - this will never show in practice
    if (components.count() != 1) {
        QMessageBox msgBox;
        msgBox.setText(tr("Invalid system setup detected"));
        msgBox.setInformativeText(tr("None or more than one component advertised to provide the main system configuration option. This is an invalid system setup - please check your autopilot."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        (void)msgBox.exec();

        return;
    }

    qDebug() << "Setting comp" << components.first() << "SYS_AUTOSTART" << (qint32)selectedId;

    paramMgr->setPendingParam(components.first(),"SYS_AUTOSTART", (qint32)selectedId);

    //need to set autoconfig in order for PX4 to pick up the selected airframe params
    setAutoConfig(true);

    // Send pending params and then write them to persistent storage when done
    paramMgr->sendPendingParameters(true);

    // Reboot
    //TODO right now this relies upon the above send & persist finishing before the reboot command is received...
    QGC::SLEEP::msleep(3000);
    mav->executeCommand(MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 1, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
}

void QGCPX4AirframeConfig::setAutoConfig(bool enabled)
{
    if (!mav)
        return;
    paramMgr->setPendingParam(0, "SYS_AUTOCONFIG", (qint32) ((enabled) ? 1 : 0));
}

void QGCPX4AirframeConfig::flyingWingSelected()
{
    flyingWingSelected(ui->flyingWingComboBox->currentIndex());
}

void QGCPX4AirframeConfig::flyingWingSelected(int index)
{
    int system_index = ui->flyingWingComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::planeSelected()
{
    planeSelected(ui->planeComboBox->currentIndex());
}

void QGCPX4AirframeConfig::planeSelected(int index)
{
    int system_index = ui->planeComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}


void QGCPX4AirframeConfig::quadXSelected()
{
    quadXSelected(ui->quadXComboBox->currentIndex());
}

void QGCPX4AirframeConfig::quadXSelected(int index)
{
    int system_index = ui->quadXComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::quadPlusSelected()
{
    quadPlusSelected(ui->quadPlusComboBox->currentIndex());
}

void QGCPX4AirframeConfig::quadPlusSelected(int index)
{
    int system_index = ui->quadPlusComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::hexaXSelected()
{
    hexaXSelected(ui->hexaXComboBox->currentIndex());
}

void QGCPX4AirframeConfig::hexaXSelected(int index)
{
    int system_index = ui->hexaXComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::hexaPlusSelected()
{
    hexaPlusSelected(ui->hexaPlusComboBox->currentIndex());
}

void QGCPX4AirframeConfig::hexaPlusSelected(int index)
{
    int system_index = ui->hexaPlusComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::octoXSelected()
{
    octoXSelected(ui->octoXComboBox->currentIndex());
}

void QGCPX4AirframeConfig::octoXSelected(int index)
{
    int system_index = ui->octoXComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::octoPlusSelected()
{
    octoPlusSelected(ui->octoPlusComboBox->currentIndex());
}

void QGCPX4AirframeConfig::octoPlusSelected(int index)
{
    int system_index = ui->octoPlusComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}

void QGCPX4AirframeConfig::hSelected()
{
    hSelected(ui->hComboBox->currentIndex());
}

void QGCPX4AirframeConfig::hSelected(int index)
{
    int system_index = ui->hComboBox->itemData(index).toInt();
    setAirframeID(system_index);
}


QGCPX4AirframeConfig::~QGCPX4AirframeConfig()
{
    delete ui;
}
