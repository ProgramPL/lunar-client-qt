//
// Created by nils on 11/4/21.
//

#include "mainwindow.h"

#include <QGridLayout>
#include <QListWidgetItem>
#include <QComboBox>
#include <QStatusBar>
#include <QScrollArea>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QMessageBox>

#include "pages/configurationpage.h"
#include "pages/generalpage.h"
#include "launch/launcher.h"

#define GET_PAGES(name) ConfigurationPage* name[] = \
{generalPage, agentsPage}

const QString MainWindow::configLocation = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/lunar-client-qt/config.json";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    setWindowTitle(QStringLiteral("Lunar Client Qt"));
    QWidget* centralWidget = new QWidget();

    QGridLayout* mainLayout = new QGridLayout();

    pageList = new QListWidget();
    pageStack = new QStackedWidget();

    pageStack->setContentsMargins(30, 10, 30, 10);

    pageList->setIconSize(QSize(32, 32));

    generalPage = new GeneralPage();
    agentsPage = new AgentsPage();

    GET_PAGES(pages);

    for(ConfigurationPage* page : pages){
        new QListWidgetItem(page->icon(), page->title(), pageList);
        pageStack->addWidget(page);
    }

    connect(pageList, &QListWidget::currentRowChanged, pageStack, &QStackedWidget::setCurrentIndex);

    pageList->setCurrentRow(0);
    pageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pageList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    pageList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    QFont font;
    font.setPointSize(11);
    pageList->setFont(font);

    versionSelect = new QComboBox();
    versionSelect->addItems({"1.7", "1.8", "1.12", "1.16", "1.17"});
    versionSelect->setCurrentIndex(1);

    launchNoCosmeticsButton = new QPushButton();
    launchNoCosmeticsButton->setMinimumHeight(45);
    connect(launchNoCosmeticsButton, &QPushButton::pressed, this, &MainWindow::launchNoCosmetics);

    launchOfflineButton = new QPushButton();
    launchOfflineButton->setMinimumHeight(45);
    connect(launchOfflineButton, &QPushButton::pressed, this, &MainWindow::launchOffline);

    connect(&offlineLauncher, &OfflineLauncher::error, this, &MainWindow::errorCallback);

    resetLaunchButtons();

    mainLayout->addWidget(pageList);
    mainLayout->addWidget(versionSelect, 1, 0);
    mainLayout->addWidget(launchNoCosmeticsButton, 2, 0);
    mainLayout->addWidget(launchOfflineButton, 3, 0);
    mainLayout->addWidget(pageStack, 0, 3, -1, 1);

    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
    resize(800, 600);

    load();
}

void MainWindow::resetLaunchButtons() {
    launchOfflineButton->setEnabled(true);
    launchOfflineButton->setText(QStringLiteral("Launch"));

    launchNoCosmeticsButton->setEnabled(true);
    launchNoCosmeticsButton->setText(QStringLiteral("Launch without \ncosmetics"));
}

void MainWindow::launchNoCosmetics() {
    launch(offlineLauncher, false);
}


void MainWindow::launchOffline() {
    launch(offlineLauncher);
}

void MainWindow::launch(Launcher& launcher, bool cosmetics){
    launcher.launch(Launcher::LaunchOptions{
       versionSelect->currentText(),
       !generalPage->isUsingCustomJre(),
       generalPage->isUsingCustomJre() ? generalPage->getJrePath() : QString(),
       generalPage->getJvmArgs(),
       agentsPage->getAgents(),
       cosmetics,
       generalPage->getInitialMemory(),
       generalPage->getMaxMemory(),
       generalPage->getWindowWidth(),
       generalPage->getWindowHeight(),
    });
}

void MainWindow::save() {
    QJsonObject saveObj;

    GET_PAGES(pages);

    for(auto page : pages){
        QJsonObject pageObj;
        page->save(pageObj);
        saveObj[page->title()] = pageObj;
    }

    saveObj["version"] = versionSelect->currentIndex();

    QString path = QFileInfo(configLocation).absolutePath();
    QDir dir;
    if(!dir.exists(path)){
        dir.mkdir(path);
    }

    QFile configFile(configLocation);

    configFile.open(QIODevice::WriteOnly);

    configFile.write(QJsonDocument(saveObj).toJson());

    configFile.close();
}


void MainWindow::load() {
    QFile configFile(configLocation);
    configFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonObject jsonObj = QJsonDocument::fromJson(configFile.readAll()).object();

    configFile.close();

    GET_PAGES(pages);

    for(auto page : pages){
        page->load(jsonObj[page->title()].toObject());
    }

    versionSelect->setCurrentIndex(jsonObj["version"].toInt(1));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    save();
    event->accept();
}

void MainWindow::errorCallback(const QString &message) {
    QMessageBox messageBox;
    messageBox.setText(message);
    messageBox.exec();
}