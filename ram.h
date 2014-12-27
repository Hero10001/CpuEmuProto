#ifndef RAM_H
#define RAM_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <cstring>

#define BYTE uchar

class Ram : public QObject
{
    Q_OBJECT
public:
    explicit Ram(size_t cellNum,size_t cellSize,QObject *parent = 0);
    BYTE* read(size_t pos);
    void write(size_t pos,BYTE *pVal);
    void dump();
    ~Ram();
private:
    BYTE *mem;
    size_t cellSize;
    size_t cellNum;


signals:

public slots:
};

#endif // RAM_H
