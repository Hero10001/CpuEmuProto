#ifndef PROM_H
#define PROM_H
#
#include <QObject>
#include <QFile>
#include <QDataStream>
#include <cstring>

class PRom : public QObject
{
    Q_OBJECT
public:
    explicit PRom(size_t cellNum,size_t cellSize,QObject *parent = 0);
    uchar* read(size_t pos);
    void set(size_t pos,uchar *pVal);
    void init(uchar *pVal);
    void dump();
    ~PRom();
private:
    uchar *mem;
    size_t cellSize;
    size_t cellNum;

signals:

public slots:
};

#endif // PROM_H
