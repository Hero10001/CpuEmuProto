#include "ram.h"

Ram::Ram(size_t cellNum, size_t cellSize, QObject *parent) : QObject(parent)
{
    this->cellNum=cellNum;
    this->cellSize=cellSize;
    mem= new BYTE[cellNum*cellSize];
}

BYTE *Ram::read(size_t pos)
{
    BYTE* out;
    if(pos<cellNum){
        out= new uchar[cellSize];
        memcpy(out,(mem+(pos*cellSize)),cellSize);
    }
    else
    out = NULL;

    return out;
}

void Ram::write(size_t pos, BYTE *pVal)
{
    if(pos>=cellNum)
        return;

    memcpy((mem+(pos*cellSize)),pVal,cellSize);
}

void Ram::dump()
{
    QFile f("dumpRam.txt");
    f.open(QFile::WriteOnly);
    f.write((const char*)mem,(cellNum*cellSize));
}

Ram::~Ram()
{
    delete[] this->mem;
}

