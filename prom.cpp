#include "prom.h"

PRom::PRom(size_t cellNum, size_t cellSize, QObject *parent) : QObject(parent)
{
    this->cellNum=cellNum;
    this->cellSize=cellSize;
    mem= new uchar[cellNum*cellSize];
    memset(mem,0xFF,(cellNum*cellSize));
}

uchar *PRom::read(size_t pos)
{
    uchar* out;
    if(pos<cellNum){
        out= new uchar[cellSize];
        memcpy(out,(mem+(pos*cellSize)),cellSize);
    }
    else
    out = NULL;

    return out;
}

void PRom::set(size_t pos, uchar *pVal)
{
    if(pos>=cellNum)
        return;
    uchar *old=new uchar[cellSize];
    memcpy(old,(mem+pos*cellSize),cellSize);

    for(uint i=0;i<cellSize;i++) // "fuse-logic" bits can only be set to 0
        old[i]=pVal[i]&old[i];

    memcpy((mem+(pos*cellSize)),old,cellSize);
    delete[] old;
}

void PRom::init(uchar *pVal)
{
    memcpy(mem,pVal,(cellNum*cellSize));
}

void PRom::dump()
{
    QFile f("dumpPRom.txt");
    f.open(QFile::WriteOnly);
    f.write((const char*)mem,(cellNum*cellSize));
}

PRom::~PRom()
{
    delete[] mem;
}

