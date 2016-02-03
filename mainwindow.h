#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

#include "parser.h"
#include "qcustomplot.h"

#define CALIBRATE   0xA5;
#define TELEMETRIA  0xAA;
#define OSHIBKA     0xEE;
#define END         0xFF;

namespace Ui {
class MainWindow;
}

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
  void setupPlot();
  void initActionsConnections();
  inline int read_string_hex(char *str, int len);

private slots:
  void resizeEvent(QResizeEvent *);

  void openSerialPort();
  void closeSerialPort();
  void readData();
  void writeData(const QByteArray &data);
  void handleError(QSerialPort::SerialPortError error);

  void horzScrollBarChanged(int value);
  void vertScrollBarChanged(int value);
  void xAxisChanged(QCPRange range);
  void yAxisChanged(QCPRange range);
  void setupRealtimeDataDemo(const tele_unit &telemetria);
  //void realtimeDataSlot();

  void slotOpenDialog();
  void slotSaveDialog();

  void calibrate();
  void telemetria();

  void sendCommandLine();

private:
    Ui::MainWindow *ui;

    QSerialPort *port_serial;
    SettingsDialog *settings;
    Parser *parser;

    QVector<tele_unit> teleData;
    QList<tele_unit> buffer;

    QDateTime dateTime;

    bool started;
    bool HEXIsLoaded;
};

#endif // MAINWINDOW_H
