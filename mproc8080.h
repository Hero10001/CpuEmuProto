#ifndef MPROC8080_H
#define MPROC8080_H

#include <assert.h>
#include <QObject>
#include <ram.h>
#include <prom.h>

class Mproc8080 : public QObject
{
    Q_OBJECT
public:
    Ram *mem;
//    PRom *pro;


    explicit Mproc8080(QObject *parent = 0);
    ~Mproc8080();
private:
    bool par[0xFF];//parity lookup

    uchar a[12];
    uchar *f;
    uchar *b,*c,*d,*e,*h,*l;
    ushort *bc,*de,*hl,*sp,*pc;

    uchar*  pReg(uchar s);
    ushort* pRP(uchar s);
    uchar    setflags(ushort in);//cheks which flags need to be set and returns anwser

    void   inline incPC(){(*pc)++;}
    uchar  inline readM(ushort pos){return *mem->read((size_t)pos);}
    uchar  inline readP(ushort pos){return *mem->read((size_t)pos);}//probly no prom in an  8080
    uchar  inline setFL(uchar flag){(*f)=(*f)|flag;}
    bool   inline chkFL(uchar flag){return ((*f)&flag)>0;}
    bool   inline chkAC(uchar aa,uchar bb,uchar out){return (((aa^bb)&0x10)!= out&0x10);}

    void mov(uchar opcode);
    void mvi(uchar opcode);
    void lxi(uchar opcode);
    void lda();
    void sta();
    void lhld();
    void shld();
    void ldax(uchar opcode);
    void stax(uchar opcode);
    void xchg();
    void add(uchar opcode);
    void adi();
    void adc(uchar opcode);
    void aci();
    void sub(uchar opcode);
    void sui();
    void sbb(uchar opcode);
    void sbi();
    void inr(uchar opcode);
    void dcr(uchar opcode);
    void inx(uchar opcode);
    void dcx(uchar opcode);
    void dad(uchar opcode);
    void ana(uchar opcode);
    void ani();
    void ora(uchar opcode);
    void ori();
    void xra(uchar opcode);
    void xri(uchar opcode);
    void cmp(uchar opcode);
    void cpi();
signals:

public slots:
};

#endif // MPROC8080_H
