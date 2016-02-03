#include "parser.h"

Parser::Parser(QObject *parent) :
    QObject(parent)
{
    QTime time = QTime::currentTime();
    out = new QFile(QString("telemetria_%1_%2_%3").arg(time.hour())
            .arg(time.minute()).arg(time.second()));
    out->open(QIODevice::WriteOnly);

}


Parser::~Parser()
{
    out->close();
}

void Parser::getRaw(const QByteArray &raw)
{
    QByteArray temp = raw;

    tele_unit tele_tmp;
    if(quint8(temp.at(0)) == 0xAA)
    {
        qDebug() << "0xAA parser started";
        long long i = 1;
        while(1)
        {
            tele_tmp.accX = (quint16)temp.at(i)<<8;
            tele_tmp.accX = (quint16)temp.at(i++);
            tele_tmp.accY = (quint16)temp.at(i++)<<8;
            tele_tmp.accY = (quint16)temp.at(i++);
            tele_tmp.accZ = (quint16)temp.at(i++)<<8;
            tele_tmp.accZ = (quint16)temp.at(i++);

            tele_tmp.gyrX = (quint16)temp.at(i++)<<8;
            tele_tmp.gyrX = (quint16)temp.at(i++);
            tele_tmp.gyrY = (quint16)temp.at(i++)<<8;
            tele_tmp.gyrY = (quint16)temp.at(i++);
            tele_tmp.gyrZ = (quint16)temp.at(i++)<<8;
            tele_tmp.gyrZ = (quint16)temp.at(i++);

            tele_tmp.magX = (quint16)temp.at(i++)<<8;
            tele_tmp.magX = (quint16)temp.at(i++);
            tele_tmp.magY = (quint16)temp.at(i++)<<8;
            tele_tmp.magY = (quint16)temp.at(i++);
            tele_tmp.magZ = (quint16)temp.at(i++)<<8;
            tele_tmp.magZ = (quint16)temp.at(i++);

            i++;
            out->write(temp);

            emit parsedInfo(tele_tmp);
        }
    }
    else if(quint8(temp.at(0)) == 0xA5)
    {
        text = (QString)temp;
        emit textInfo(text);
    }
    else if(quint8(temp.at(0)) == 0xFF)
    {
        text = "Обмен окончен";
        emit textInfo(text);
    }
    else if(quint8(temp.at(0)) == 0xEE)
    {
        text = "Ошибка";
        emit textInfo(text);
    }
    /*else
    {
        text = (QString)temp;
        emit textInfo(text);
    }*/

}
