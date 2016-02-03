#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->monitor->setEnabled(false);

    setupPlot();

    HEXIsLoaded = false;

    port_serial = new QSerialPort(this);

    settings = new SettingsDialog;

    initActionsConnections();

    connect(port_serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(port_serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));


}

MainWindow::~MainWindow()
{
  delete port_serial;
  delete settings;
  delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    int extraWidth = width() - minimumWidth();
    int extraHeight = height() - minimumHeight();

    ui->labelHEXName->setGeometry(10,368+extraHeight,111,16);
    ui->line->setGeometry(120,10,20,351+extraHeight);

    ui->verticalScrollBar->setGeometry(906+extraWidth,10,16,351+extraHeight);
    ui->horizontalScrollBar->setGeometry(140,370+extraHeight,761+extraWidth,16);
    ui->plot->setGeometry(140,10,761+extraWidth,351+extraHeight);

    ui->sendButton->setGeometry(10,390+extraHeight,111,21);
    ui->commandLineEdit->setGeometry(128,390+extraHeight,803+extraWidth,21);
    ui->monitor->setGeometry(10,420+extraHeight,921+extraWidth,251);

}

void MainWindow::initActionsConnections()
{
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slotOpenDialog()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(slotSaveDialog()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfig, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), ui->monitor, SLOT(clear()));

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(openSerialPort()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(closeSerialPort()));
    connect(ui->calibrButton, SIGNAL(clicked()), this, SLOT(calibrate()));
    connect(ui->teleButton, SIGNAL(clicked()), this, SLOT(telemetria()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendCommandLine()));

}

void MainWindow::slotOpenDialog()
{
    char string[1024];
    tele_unit tmp_mem;
    QString fileName = QFileDialog::getOpenFileName(this,tr("Выберите файл телеметрии:"),//QString().fromLocal8Bit("Выберите файл телеметрии:"),
                                                    QString(),
                                                    QString().fromLocal8Bit("Intel HEX (*.hex)"));
    if(fileName.isEmpty())
    {
        statusBar()->showMessage(tr("Невозможно открыть. Укажите имя файла."));
        return;
    }

    QFile fileHEX(fileName);
    fileHEX.open(QIODevice::ReadOnly|QIODevice::Text);
    if(!fileHEX.isOpen())
    {
        statusBar()->showMessage(tr("Ошибка при открытии файла"));
        return;
    }

    if(fileHEX.size() == 0)
    {
        statusBar()->showMessage(tr("Файл пуст"));
        return;
    } else {

        HEXIsLoaded = true;
        ui->labelHEXName->setText(tr("File: ")+fileName);

        do
        {
            if(fileHEX.readLine(string,1024)<36)
                break;

            tmp_mem.accX = read_string_hex(&string[0], 4);
            tmp_mem.accY = read_string_hex(&string[4], 4);
            tmp_mem.accZ = read_string_hex(&string[8], 4);

            tmp_mem.gyrX = read_string_hex(&string[12], 4);
            tmp_mem.gyrY = read_string_hex(&string[16], 4);
            tmp_mem.gyrZ = read_string_hex(&string[20], 4);

            tmp_mem.accY = read_string_hex(&string[24], 4);
            tmp_mem.accY = read_string_hex(&string[28], 4);
            tmp_mem.accY = read_string_hex(&string[32], 4);

            buffer.append(tmp_mem);
        } while(1);



        return;
    }
}

void MainWindow::slotSaveDialog()
{
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
  QString fileName = dateTime.currentDateTime().toString()+".png";
  fileName.replace(" ", "");
  pm.save("./screenshots/"+fileName);
  qApp->quit();
}

void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    port_serial->setPortName("\\\\.\\COM5"); //p.name
    if (port_serial->open(QIODevice::ReadWrite)) {
        if (port_serial->setBaudRate(p.baudRate)
                && port_serial->setDataBits(p.dataBits)
                && port_serial->setParity(p.parity)
                && port_serial->setStopBits(p.stopBits)
                && port_serial->setFlowControl(p.flowControl)) {

            started = false;
            parser = new Parser(this);
            qRegisterMetaType<tele_unit>("tele_unit");
            ui->monitor->setEnabled(true);
            ui->connectButton->setEnabled(false);
            ui->disconnectButton->setEnabled(true);
            ui->actionConfig->setEnabled(false);
            ui->sendButton->setEnabled(true);
            ui->teleButton->setEnabled(true);
            ui->calibrButton->setEnabled(true);
            statusBar()->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

        } else {
            port_serial->close();
            QMessageBox::critical(this, tr("Ошибка"), port_serial->errorString());

            statusBar()->showMessage(tr("Невозможно открыть %s порт").arg(p.name));
        }
    } else {
        QMessageBox::critical(this, tr("ошибка"), port_serial->errorString());

        statusBar()->showMessage(tr("Ошибка настройки порта"));
    }

    connect(parser, SIGNAL(textInfo(QString)), ui->monitor, SLOT(insertPlainText(QString)));
    connect(parser,SIGNAL(parsedInfo(tele_unit)), this, SLOT(setupRealtimeDataDemo(tele_unit)));

}

void MainWindow::closeSerialPort()
{
    port_serial->close();
    disconnect(parser, SIGNAL(textInfo(QString)), ui->monitor, SLOT(insertPlainText(QString)));
    disconnect(parser,SIGNAL(parsedInfo(tele_unit)), this, SLOT(setupRealtimeDataDemo(tele_unit)));
    delete parser;

    started = false;
    ui->monitor->setEnabled(false);
    ui->connectButton->setEnabled(true);
    ui->teleButton->setEnabled(false);
    ui->calibrButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->disconnectButton->setEnabled(false);
    ui->actionConfig->setEnabled(true);
    statusBar()->showMessage(tr("Порт отключен"));
}

void MainWindow::writeData(const QByteArray &data)
{
    port_serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = port_serial->readAll();
    parser->getRaw(data);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Критичная ошибка"), port_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::setupPlot()
{
  ui->plot->addGraph();
  ui->plot->graph(0)->setPen(QPen(Qt::blue));
  ui->plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
  ui->plot->graph(0)->setAntialiasedFill(false);
  ui->plot->addGraph();
  ui->plot->graph(1)->setPen(QPen(Qt::red));
  ui->plot->addGraph();
  ui->plot->graph(2)->setPen(QPen(Qt::yellow));
  ui->plot->addGraph();
  ui->plot->graph(3)->setPen(QPen(Qt::green));
  ui->plot->addGraph();
  ui->plot->graph(4)->setPen(QPen(Qt::black));
  ui->plot->addGraph();
  ui->plot->graph(5)->setPen(QPen(Qt::darkBlue));
  ui->plot->addGraph();
  ui->plot->graph(6)->setPen(QPen(Qt::darkRed));
  ui->plot->addGraph();
  ui->plot->graph(7)->setPen(QPen(Qt::darkGray));
  ui->plot->addGraph();
  ui->plot->graph(8)->setPen(QPen(Qt::darkGreen));

}

void MainWindow::horzScrollBarChanged(int value)
{
  if (qAbs(ui->plot->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->plot->xAxis->setRange(value/100.0, ui->plot->xAxis->range().size(), Qt::AlignLeft);
    ui->plot->replot();
  }
}

void MainWindow::vertScrollBarChanged(int value)
{
  if (qAbs(ui->plot->yAxis->range().center()+value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->plot->yAxis->setRange(-value/100.0, ui->plot->yAxis->range().size(), Qt::AlignCenter);
    ui->plot->replot();
  }
}

void MainWindow::xAxisChanged(QCPRange range)
{
  ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
  ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::yAxisChanged(QCPRange range)
{
  ui->verticalScrollBar->setValue(qRound(-range.center()*100.0)); // adjust position of scroll bar slider
  ui->verticalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::setupRealtimeDataDemo(const tele_unit &telemetria)
{
    double key = 0;
    static double lastPointKey = 0;
    key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    tele_unit tmp = telemetria;
    qDebug() << "setaupRealTimeData";
    ui->plot->graph(0)->addData(key, tmp.accX);
    ui->plot->graph(1)->addData(key, tmp.accY);
    ui->plot->graph(2)->addData(key, tmp.accZ);

    ui->plot->graph(3)->addData(key, tmp.gyrX);
    ui->plot->graph(4)->addData(key, tmp.gyrY);
    ui->plot->graph(5)->addData(key, tmp.gyrZ);

    ui->plot->graph(6)->addData(key, tmp.magX);
    ui->plot->graph(7)->addData(key, tmp.magY);
    ui->plot->graph(8)->addData(key, tmp.magZ);

    lastPointKey = key;

    ui->plot->replot();
}

inline int MainWindow::read_string_hex(char *str, int len)
{
    int ret = 0, i;
    for(i = 0; i < len; i++)
    {
        ret <<= 4;
        if((str[i]>='0')&&(str[i]<='9'))
            ret += str[i] - '0';
        else if((str[i]>='A')&&(str[i]<='F'))
            ret += str[i] - 'A' + 0xA;
        else if((str[i]>='a')&&(str[i]<='f'))
            ret += str[i] - 'a' + 0xA;
        else
            return -1;
    }
    return ret;
}

void MainWindow::sendCommandLine()
{
    QString temp;
    temp = ui->commandLineEdit->text();
    ui->commandLineEdit->selectAll();
    ui->commandLineEdit->del();
    QByteArray buf_out;
    buf_out.append(temp);
    writeData(buf_out);
}

void MainWindow::calibrate()
{
    QString temp;
    QByteArray buf_out;

    ui->teleButton->setEnabled(false);
    ui->calibrButton->setEnabled(false);
    ui->sendButton->setEnabled(true);

    temp = "A5";
    buf_out.append(temp);
    writeData(buf_out);

}

void MainWindow::telemetria()
{
    QString temp;
    QByteArray buf_out;
    qDebug() << "Started = " << started;

    if(!started)
    {
        ui->teleButton->setText("Стоп");
        ui->calibrButton->setEnabled(false);
        ui->sendButton->setEnabled(true);
        started = !started;
        temp = "AA";
        buf_out.append(temp);
        writeData(buf_out);
    }
    else
    {
        ui->teleButton->setText("Телеметрия");
        ui->calibrButton->setEnabled(true);
        ui->sendButton->setEnabled(false);
        started = !started;
        temp = "FF";
        buf_out.append(temp);
        writeData(buf_out);

    }
}
