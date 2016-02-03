#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QtWidgets>

typedef struct
{
    double accX;
    double accY;
    double accZ;

    double gyrX;
    double gyrY;
    double gyrZ;

    double magX;
    double magY;
    double magZ;
} tele_unit;

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    ~Parser();

signals:
    void parsedInfo(const tele_unit &telemetria);
    void textInfo(const QString &str);

public slots:
    void getRaw(const QByteArray &raw);

private:
    tele_unit info;
    QFile *out;
    QString text;

};

#endif // PARSER_H
