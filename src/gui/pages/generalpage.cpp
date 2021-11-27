//
// Created by nils on 11/5/21.
//

#include "generalpage.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QJsonObject>

GeneralPage::GeneralPage(QWidget *parent) : ConfigurationPage(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(40);

    keepMemorySame = new QCheckBox(QStringLiteral("Keep initial and maximum memory allocations the same"));

    QLabel* initialMemoryLabel = new QLabel();
    initialMemory = new QSlider(Qt::Horizontal);
    initialMemory->setMinimum(1024);
    initialMemory->setMaximum(16384);
    initialMemory->setPageStep(1024);

    QLabel* maxMemoryLabel = new QLabel();
    maxMemory = new QSlider(Qt::Horizontal);
    maxMemory->setMinimum(1024);
    maxMemory->setMaximum(16384);
    maxMemory->setPageStep(1024);

    //Memory slider functionality
    connect(initialMemory, &QSlider::valueChanged, [initialMemoryLabel](int val){initialMemoryLabel->setText("Initial Memory:  " + QString::number(val) + " MiB");});
    connect(maxMemory, &QSlider::valueChanged, [maxMemoryLabel](int val){maxMemoryLabel->setText("Maximum Memory:  " + QString::number(val) + " MiB");});

    initialMemory->setValue(16384/4);
    maxMemory->setValue(16384/4);

    connect(keepMemorySame, &QCheckBox::toggled, this, &GeneralPage::keepMinMaxSameChanged);
    keepMemorySame->setChecked(true);

    //Memory slider group
    QVBoxLayout* memorySliderContainer = new QVBoxLayout();
    memorySliderContainer->setSpacing(6);

    memorySliderContainer->addWidget(initialMemoryLabel, 0, Qt::AlignHCenter);
    memorySliderContainer->addWidget(initialMemory);
    memorySliderContainer->addSpacing(30);
    memorySliderContainer->addWidget(maxMemoryLabel, 0, Qt::AlignHCenter);
    memorySliderContainer->addWidget(maxMemory);

    //Window resolution
    windowWidth = new QSpinBox();
    windowHeight = new QSpinBox();

    windowWidth->setMinimum(0);
    windowWidth->setMaximum(99999);
    windowWidth->setValue(640);

    windowHeight->setMinimum(0);
    windowHeight->setMaximum(99999);
    windowHeight->setValue(480);

    //Window resolution group
    QHBoxLayout* windowResContainer = new QHBoxLayout();
    windowResContainer->setSpacing(30);
    windowResContainer->addWidget(new QLabel(QStringLiteral("Window width")));
    windowResContainer->addWidget(windowWidth, 1);
    windowResContainer->addWidget(new QLabel(QStringLiteral("Window height")));
    windowResContainer->addWidget(windowHeight, 1);

    //Custom jre checkbox lineedit and button
    useCustomJre = new QCheckBox(QStringLiteral("Use custom jre"));

    jreLine = new QLineEdit();
    QPushButton* openFile = new QPushButton();
    openFile->setIcon(QIcon(":/res/icons/openfolder.svg"));

    jreLine->setDisabled(true);
    openFile->setDisabled(true);

    connect(useCustomJre, &QCheckBox::toggled, jreLine, &QLineEdit::setEnabled);
    connect(useCustomJre, &QCheckBox::toggled, openFile, &QPushButton::setEnabled);
    connect(jreLine, &QLineEdit::returnPressed, [this](){jreLine->clearFocus();});

    connect(openFile, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName();
        if(!fileName.isNull())
            jreLine->setText(fileName);
    });

    //Custom jre groups
    QGridLayout* customJreContainer = new QGridLayout();
    customJreContainer->setSpacing(6);
    customJreContainer->addWidget(useCustomJre, 0, 0, 1, -1, Qt::AlignHCenter);
    customJreContainer->addWidget(jreLine, 1, 0);
    customJreContainer->addWidget(openFile, 1, 1);

    //Jvm arguments
    QVBoxLayout* jvmArgsGroup = new QVBoxLayout();
    jvmArgsGroup->setSpacing(6);

    jvmArgs = new QPlainTextEdit();

    jvmArgsGroup->addWidget(new QLabel(QStringLiteral("JVM Arguments")), 0, Qt::AlignHCenter);
    jvmArgsGroup->addWidget(jvmArgs);


    mainLayout->addWidget(keepMemorySame, 0, Qt::AlignHCenter);
    mainLayout->addLayout(memorySliderContainer);
    mainLayout->addLayout(windowResContainer);
    mainLayout->addLayout(customJreContainer);
    mainLayout->addLayout(jvmArgsGroup, 1);

    setLayout(mainLayout);
}

QString GeneralPage::title() {
    return QStringLiteral("General");
}

QIcon GeneralPage::icon() {
    return QIcon(":/res/icons/cog.svg");
}

void GeneralPage::save(QJsonObject &jsonObject) {
    jsonObject["keepMemorySame"] = keepMemorySame->isChecked();
    jsonObject["initialMemory"] = initialMemory->value();
    jsonObject["maxMemory"] = maxMemory->value();

    jsonObject["windowWidth"] = windowWidth->value();
    jsonObject["windowHeight"] = windowHeight->value();

    jsonObject["useCustomJre"] = useCustomJre->isChecked();
    jsonObject["customJrePath"] = jreLine->text();

    jsonObject["jvmArgs"] = jvmArgs->toPlainText();
}

void GeneralPage::load(const QJsonObject &jsonObject) {
    keepMemorySame->setChecked(jsonObject["keepMemorySame"].toBool(true));
    initialMemory->setValue(jsonObject["initialMemory"].toInt(4096));
    maxMemory->setValue(jsonObject["maxMemory"].toInt(4096));

    windowWidth->setValue(jsonObject["windowWidth"].toInt(640));
    windowHeight->setValue(jsonObject["windowHeight"].toInt(480));

    useCustomJre->setChecked(jsonObject["useCustomJre"].toBool(false));
    jreLine->setText(jsonObject["customJrePath"].toString());

    jvmArgs->setPlainText(jsonObject["jvmArgs"].toString());
}


void GeneralPage::keepMinMaxSameChanged(bool checked) {
    if(checked){
        initialMemory->setValue(maxMemory->value());
        maxMemory->setDisabled(true);
        connect(initialMemory, &QSlider::valueChanged, maxMemory, &QSlider::setValue, Qt::UniqueConnection);
    }else{
        maxMemory->setEnabled(true);
        disconnect(initialMemory, &QSlider::valueChanged, maxMemory, &QSlider::setValue);
    }
}

int GeneralPage::getInitialMemory() {
    return initialMemory->value();
}

int GeneralPage::getMaxMemory() {
    return maxMemory->value();
}

QString GeneralPage::getJrePath() {
    return jreLine->text();
}

bool GeneralPage::isUsingCustomJre() {
    return useCustomJre->isChecked();
}

int GeneralPage::getWindowWidth() {
    return windowWidth->value();
}

int GeneralPage::getWindowHeight() {
    return windowHeight->value();
}

QString GeneralPage::getJvmArgs() {
    return jvmArgs->toPlainText();
}
