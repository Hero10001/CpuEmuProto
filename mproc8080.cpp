#include "mproc8080.h"

//Flag masks
#define carry    0x1
#define parity   0x4
#define auxcarry 0x10
//#define interupt 0x20
#define zero     0x40
#define sign     0x80

//Source // Dest // RegPair // Condition // Restart masks
#define Source   0x07
#define Dest     0x38
#define RP       0x30
#define CCC      0x38
#define NNN      0x38

//Source Dest field
#define A        0x07
#define B        0x00
#define C        0x01
#define D        0x02
#define E        0x03
#define H        0x04
#define L        0x05
#define M        0x06
//RegisterPair field
#define BC       0x00
#define DE       0x01
#define HL       0x02
#define SP       0x03
//Condition field
#define NZ       0x00
#define Z        0x01
#define NC       0x02
#define Cc       0x03//((Cc==c) carry
#define PO       0x04
#define PE       0x05
#define P        0x06
#define Mc       0x07 //(Mc==m): minus
//Function values without regaddr or conditions
#define LDA      0x3A
#define STA      0x32
#define LHLD     0x2A
#define SHLD     0x22
#define XCHG     0xEB
#define ADI      0xC6
#define ACI      0xCE
#define SUI      0xD6
#define SBI      0xDE
#define DAA      0x27 //not implemented
#define ANI      0xE6
#define ORI      0xF6
#define XRI      0xEE
#define CPI      0xFE
#define RLC      0x07
#define RRC      0x0F
#define RAL      0x17
#define RAR      0x1F
#define CMA      0x2F
#define CMC      0x3F
#define STC      0x37
#define JMP      0xC3
#define JMP1     0xCB //#
#define CALL     0xCD
#define RET      0xC9
#define RET1     0xD9//#
#define PCHL     0xE9
#define XTHL     0xE3
#define SPHL     0xF9
#define IN       0xDB
#define OUT      0xD3
#define EI       0xFB
#define DI       0xF3
#define HLT      0x76
#define NOP      0x00
#define NOP1     0x10//#
#define NOP2     0x20//#
#define NOP3     0x30//#
#define NOP4     0x08//#
#define NOP5     0x18//#
#define NOP6     0x28//#
#define NOP7     0x38//#
//#superfluous
//Function values with RegPair field (set to 0b00)
#define LXI      0x01
#define LDAX     0x0A//*
#define STAX     0x02//*
#define INX      0x03
#define DCX      0x0B
#define DAD      0x09
#define PUSH     0xC5
#define POP      0xC1
//*only work with RP=00 or RP=01 otherwise opcode==LDA/STA
//Function values with Dest field only (set to 0b000)
#define MVI      0x06
#define INR      0x08
#define DCR      0x09
//Function values with Src field only (set to 0b000)
#define ADD      0x80
#define ADC      0x88
#define SUB      0x90
#define SBB      0x98
#define ANA      0xA0
#define ORA      0xB0
#define XRA      0xA8
#define CMP      0xB8
//Function values with Condition field only (set to 0b000)
#define JCCC     0xC2
#define CCCC     0xC4
#define RCCC     0xC0
//Function values with Src and dest field (set to 0b000)
#define MOV      0x40

//Macro for detecting if enough timeunits in acummulator
#define CHKTIME(t) if(this->accum<t) return false

Mproc8080::Mproc8080(QObject *parent) : QObject(parent)
{
    //init parity lookup
    memset(par,false,0xFF);
    //algorithyms are for pussys
    par[0x03]=true;
    par[0x0C]=true;
    par[0x30]=true;
    par[0xC0]=true;
    par[0x0F]=true;
    par[0x33]=true;
    par[0xC3]=true;
    par[0xCC]=true;
    par[0xF0]=true;
    par[0x3C]=true;
    par[0x3F]=true;
    par[0xCF]=true;
    par[0xF3]=true;
    par[0xFC]=true;
    par[0xFF]=true;

    //init registers
    //a=uchar[12];
    memset(a,0,12);
    memset(ports,0,0xFF);
    //set values
    inte=true;
    accum=0;
    //set Flags
    f=a+1;
    *f=2;
    //define registerpairs
    bc=(ushort*)a+2;
    de=(ushort*)a+4;
    hl=(ushort*)a+6;
    sp=(ushort*)a+8;
    pc=(ushort*)a+10;
    //define rgisters
    b=(uchar*)bc;c=b+1;
    d=(uchar*)de;e=d+1;
    h=(uchar*)hl;l=h+1;
}



Mproc8080::~Mproc8080()
{

}
uchar *Mproc8080::pReg(uchar s)
{
    switch(s)
    {
    case A:
        return a;
        break;
    case B:
        return b;
        break;
    case C:
        return c;
        break;
    case D:
        return d;
        break;
    case E:
        return e;
        break;
    case H:
        return h;
        break;
    case L:
        return l;
        break;
    default:
        assert(false);
        return NULL;//useless just to shut the compiler up
    }
}

ushort *Mproc8080::pRP(uchar s)
{
    switch(s)
    {
    case BC:
        return bc;
        break;
    case DE:
        return de;
        break;
    case HL:
        return hl;
        break;
    case SP:
        return sp;
        break;
    default:
        assert(false);
        return NULL;//useless just to shut the compiler up
    }
}

uchar Mproc8080::setflags(ushort in)
{

    uchar out=(in&0xFF);//slice the hb off

    (*f)=0x2; //reset flags
    if((in>>8)>0)
        setFL(carry); //set carry if carry is needed
    if((out>>0x7)>0)
        setFL(sign);
    if(out==0)
        setFL(zero);
    if(par[out])
        setFL(parity);
    return out;
}

bool Mproc8080::condition(uchar con)
{
    switch(con)
    {
    case NZ:
        return !chkFL(zero);
    case Z:
        return chkFL(zero);
    case NC:
        return !chkFL(carry);
    case Cc:
        return chkFL(carry);
    case PO:
        return !chkFL(parity);
    case PE:
        return chkFL(parity);
    case P:
        return !chkFL(sign);
    case Mc:
        return chkFL(sign);
    default:
        assert(false);
        return false;
    }
}

bool Mproc8080::callFunction(uchar opcode)
{
    //functions without fields
    switch(opcode)
    {
    case LDA:
        CHKTIME(13);
        accum -= 13;
        lda();
        return true;
    case STA:
        CHKTIME(13);
        accum -= 13;
        sta();
        return true;
    case LHLD:
        CHKTIME(16);
        accum -= 16;
        lhld();
        return true;
    case SHLD:
        CHKTIME(16);
        accum -= 16;
        shld();
        return true;
    case XCHG:
        CHKTIME(5);
        accum -= 5;
        xchg();
        return true;
    case ADI:
        CHKTIME(7);
        accum -= 7;
        adi();
        return true;
    case ACI:
        CHKTIME(7);
        accum -= 7;
        aci();
        return true;
    case SUI:
        CHKTIME(7);
        accum -= 7;
        sui();
        return true;
    case SBI:
        CHKTIME(7);
        accum -= 7;
        sbi();
        return true;
    case DAA:
        assert(false);
        return true;
    case ANI:
        CHKTIME(7);
        accum -= 7;
        ani();
        return true;
    case ORI:
        CHKTIME(7);
        accum -= 7;
        ori();
        return true;
    case XRI:
        CHKTIME(7);
        accum -= 7;
        xri();
        return true;
    case CPI:
        CHKTIME(7);
        accum -= 7;
        cpi();
        return true;
    case RLC:
        CHKTIME(4);
        accum -= 4;
        rlc();
        return true;
    case RRC:
        CHKTIME(4);
        accum -= 4;
        rrc();
        return true;
    case RAL:
        CHKTIME(4);
        accum -= 4;
        ral();
        return true;
    case RAR:
        CHKTIME(4);
        accum -= 4;
        rar();
        return true;
    case CMA:
        CHKTIME(4);
        accum -= 4;
        cma();
        return true;
    case CMC:
        CHKTIME(4);
        accum -= 4;
        cmc();
        return true;
    case STC:
        CHKTIME(4);
        accum -= 4;
        stc();
        return true;
    case JMP:
    case JMP1:
        CHKTIME(10);
        accum -= 10;
        jmp();
        return true;
    case CALL:
        CHKTIME(17);
        accum -= 17;
        call();
        return true;
    case RET:
    case RET1:
        CHKTIME(10);
        accum -= 10;
        ret();
        return true;
    case PCHL:
        CHKTIME(5);
        accum -= 5;
        pchl();
        return true;
    case XTHL:
        CHKTIME(18);
        accum -= 18;
        xthl();
        return true;
    case SPHL:
        CHKTIME(5);
        accum -= 5;
        sphl();
        return true;
    case IN:
        CHKTIME(10);
        accum -= 10;
        in();
        return true;
    case OUT:
        CHKTIME(10);
        accum -= 10;
        out();
        return true;
    case EI:
        CHKTIME(4);
        accum -= 4;
        ei();
        return true;
    case DI:
        CHKTIME(4);
        accum -= 4;
        di();
        return true;
    case HLT:
        CHKTIME(7);
        accum -= 7;
        //TODO program HALT logic
        return true;
    case NOP:
    case NOP1:
    case NOP2:
    case NOP3:
    case NOP4:
    case NOP5:
    case NOP6:
    case NOP7:
        CHKTIME(4);
        accum -= 4;
        //Nothing
        return true;
    }

    //functions with RP field
    switch(opcode&(~RP))
    {
    case LXI:
        CHKTIME(10);
        accum -= 10;
        lxi(opcode);
        return true;
    case LDAX:
        CHKTIME(7);
        accum -= 7;
        ldax(opcode);
        return true;
    case STAX:
        CHKTIME(7);
        accum -= 7;
        stax(opcode);
        return true;
    case INX:
        CHKTIME(5);
        accum -= 5;
        inx(opcode);
        return true;
    case DCX:
        CHKTIME(5);
        accum -= 5;
        dcx(opcode);
        return true;
    case DAD:
        CHKTIME(10);
        accum -= 10;
        dad(opcode);
        return true;
    case PUSH:
        CHKTIME(11);
        accum -= 11;
        push(opcode);
        return true;
    case POP:
        CHKTIME(10);
        accum -= 10;
        pop(opcode);
        return true;
    }

    //functions with dest field
    switch(opcode&(~Dest))
    {
    case MVI:
        if(((opcode&Dest)>>3)==M)//diffrent time for Memaccess
        {
            CHKTIME(10);
            accum -= 10;
            mvi(opcode);
            return true;
        }
        CHKTIME(7);
        accum -= 7;
        mvi(opcode);
        return true;
    case INR:
        if(((opcode&Dest)>>3)==M)
        {
            CHKTIME(10);
            accum -= 10;
            inr(opcode);
            return true;
        }
        CHKTIME(5);
        accum -= 5;
        inr(opcode);
        return true;
    case DCR:
        if(((opcode&Dest)>>3)==M)
        {
            CHKTIME(10);
            accum -= 10;
            dcr(opcode);
            return true;
        }
        CHKTIME(5);
        accum -= 5;
        dcr(opcode);
        return true;
    }

    //functions with src field
    switch(opcode&(~Source))
    {
    case ADD:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            add(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        add(opcode);
        return true;
    case ADC:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            adc(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        adc(opcode);
        return true;
    case SUB:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            sub(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        sub(opcode);
        return true;
    case SBB:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            sbb(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        sbb(opcode);
        return true;
    case ANA:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            ana(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        ana(opcode);
        return true;
    case ORA:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            ora(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        ora(opcode);
        return true;
    case XRA:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            xra(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        xra(opcode);
        return true;
    case CMP:
        if((opcode&Source)==M)
        {
            CHKTIME(7);
            accum -= 7;
            cmp(opcode);
            return true;
        }
        CHKTIME(4);
        accum -= 4;
        cmp(opcode);
        return true;
    }

    //functions with src & dest field
    switch(opcode&(~(Source|Dest)))
    {
    case ADD:
        if(((opcode&Source)==M)||(((opcode&Dest)>>3)==M))
        {
            CHKTIME(7);
            accum -= 7;
            mov(opcode);
            return true;
        }
        CHKTIME(5);
        accum -= 5;
        mov(opcode);
        return true;
    }

    //functions with Condition field
    bool  con=condition(((opcode&CCC)>>3));//checks con beforehand cuz time differs
    switch(opcode&(~CCC))
    {
    case JCCC:
        CHKTIME(10);
        accum -= 10;
        jccc(opcode);
        return true;
    case CCCC:
        if(con)
        {
            CHKTIME(17);
            accum -= 17;
            cccc(opcode);
            return true;
        }
        CHKTIME(11);
        accum -= 11;
        cccc(opcode);
        return true;
    case RCCC:
        if(con)
        {
            CHKTIME(10);
            accum -= 10;
            rccc(opcode);
            return true;
        }
        CHKTIME(5);
        accum -= 5;
        rccc(opcode);
        return true;
    default:
        assert(false);
        return false;
    }
}

void Mproc8080::mov(uchar opcode)
{
    //needs to be handled AFTER HLT cuz(MOVMM->HLT)
    uchar source = opcode&Source;
    uchar destination= (opcode&Dest)>>3;

    assert(destination==M&&source==M);

    if(source==M)
        *pReg(destination) = readM(*hl);
    else if(destination==M)
        mem->write(*hl,pReg(source));
    else
        *pReg(destination)=*pReg(source);

}

void Mproc8080::mvi(uchar opcode)
{
    incPC();
    uchar destination = (opcode&Dest)>>3;
    *pReg(destination)=readM(*pc);
}

void Mproc8080::lxi(uchar opcode)
{
    ushort* destination = pRP(((opcode&RP)>>4));

    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    ushort src = (hb<<8)|lb;
    *destination=src;
}

void Mproc8080::lda()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    ushort memAddress =(hb<<8)|lb ;

    *a=readM(memAddress);
}

void Mproc8080::sta()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    ushort memAddress =(hb<<8)|lb ;
    mem->write(memAddress,a);
}

void Mproc8080::lhld()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    ushort memAddress =(hb<<8)|lb ;

    *h=readM(memAddress);
    *l=readM(memAddress+1);
}

void Mproc8080::shld()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    ushort memAddress =(hb<<8)|lb ;
    mem->write(memAddress,(+h));
    mem->write(memAddress+1,(+h));
}

void Mproc8080::ldax(uchar opcode)
{
    ushort src = *pRP(((opcode&RP)>>4));
    *a=readM(src);
}

void Mproc8080::stax(uchar opcode)
{
    ushort* src = pRP(((opcode&RP)>>4));
    mem->write((*src),a);
}

void Mproc8080::xchg()
{
    ushort tmp=*de;
    *de=*hl;
    *hl=tmp;
}

void Mproc8080::add(uchar opcode)
{
    uchar src = opcode&Source;
    ushort aa,bb,out;
    aa=(ushort)(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);
}

void Mproc8080::adi()
{
    incPC();
    ushort aa,bb,out;
    aa=(ushort)(*a);
    bb=readM(readM((*pc)));

    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);
}

void Mproc8080::adc(uchar opcode)
{
    uchar src = opcode&Source;
    ushort aa,bb,out;
    aa=(ushort)(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    if(chkFL(carry))
        out=aa+bb+1;
    else
        out=aa+bb;

    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);
}

void Mproc8080::aci()
{
    incPC();
    ushort aa,bb,out;
    aa=(ushort)(*a);
    bb=readM(readM((*pc)));

    if(chkFL(carry))
        out=aa+bb+1;
    else
        out=aa+bb;

    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);
}

void Mproc8080::sub(uchar opcode)
{
    uchar src = opcode&Source;
    ushort aa,bb,out;
    aa=(ushort)(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    bb=(~bb)+1;
    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::sui()
{
    incPC();
    ushort aa,bb,out;
    aa=(ushort)(*a);
    bb=readM(readM((*pc)));

    bb=(~bb)+1;
    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::sbb(uchar opcode)
{
    uchar src = opcode&Source;
    ushort aa,bb,out;
    aa=(ushort)(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    if(chkFL(carry))
        bb=(~(bb+1))+1;
    else
        bb=(~bb)+1;

    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::sbi()
{
    incPC();
    ushort aa,bb,out;
    aa=(ushort)(*a);
    bb=readM(readM((*pc)));

    if(chkFL(carry))
        bb=(~(bb+1))+1;
    else
        bb=(~bb)+1;

    out=aa+bb;
    *a=setflags(out);//slice the hb off && set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::inr(uchar opcode)
{
    uchar destination= (opcode&Dest)>>3;
    uchar bb;
    bool carry_old=chkFL(carry);

    if(destination==M)
    {
        bb=readM((*hl));
        bb=bb+1;
        mem->write((*hl),&bb);

    }
    else
    {
        bb=*pReg(destination);
        bb=bb+1;
        *pReg(destination)=bb;
    }
    setflags((ushort)bb);

    if(chkAC((bb-1),1,bb))
        setFL(auxcarry);

    if(carry_old!=chkFL(carry))//if carry was changed
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::dcr(uchar opcode)
{
    uchar destination= (opcode&Dest)>>3;
    uchar bb;
    bool carry_old=chkFL(carry);

    if(destination==M)
    {
        bb=readM((*hl));
        bb=bb+((~1)+1);
        mem->write((*hl),&bb);

    }
    else
    {
        bb=*pReg(destination);
        bb=bb+((~1)+1);
        *pReg(destination)=bb;
    }
    setflags((ushort)bb);

    if(chkAC((bb+1),((~1)+1),bb))
        setFL(auxcarry);

    if(carry_old!=chkFL(carry))//if carry was changed
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::inx(uchar opcode)
{
    ushort *dest=pRP(opcode&RP);
    (*dest)=(*dest)+1;
}

void Mproc8080::dcx(uchar opcode)
{
    ushort *dest=pRP(opcode&RP);
    (*dest)=(*dest)-1;
}

void Mproc8080::dad(uchar opcode)
{
    ushort *dest=pRP(opcode&RP);
    uint   ans=(*dest)+(*hl);
    (*hl) =ushort(ans&0xFFFF); //halfs the int
    if(ans>0xFFFF)
        setFL(carry);
}

void Mproc8080::ana(uchar opcode)
{
    uchar src = opcode&Source;
    uchar aa,bb,out;
    aa=(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    out= aa&bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::ani()
{
    incPC();
    uchar bb = readM((*pc));
    uchar aa,out;
    aa=(*a);

    out= aa&bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::ora(uchar opcode)
{
    uchar bb = opcode&Source;
    uchar aa,out;
    aa=(*a);

    if(bb==M)
        bb=readM((*hl));
    else
        bb=*pReg(bb);

    out= aa|bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::ori()
{
    incPC();
    uchar bb = readM((*pc));
    uchar aa,out;
    aa=(*a);

    out= aa|bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::xra(uchar opcode)
{
    uchar src = opcode&Source;
    uchar aa,bb,out;
    aa=(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    out= aa^bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::xri()
{
    incPC();
    uchar bb = readM((*pc));
    uchar aa,out;
    aa=(*a);

    out= aa^bb;
    setflags(out);
    (*a)=out;
}

void Mproc8080::cmp(uchar opcode)
{
    uchar src = opcode&Source;
    ushort aa,bb,out;
    aa=(ushort)(*a);

    if(src==M)
        bb=readM((*hl));
    else
        bb=*pReg(src);

    if(chkFL(carry))
        bb=(~(bb+1))+1;
    else
        bb=(~bb)+1;

    out=aa+bb;
    setflags(out);// set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::cpi()
{
    incPC();
    ushort aa,bb,out;
    aa=(ushort)(*a);
    bb=readM(readM((*pc)));

    if(chkFL(carry))
        bb=(~(bb+1))+1;
    else
        bb=(~bb)+1;

    out=aa+bb;
    setflags(out);// set Flags

    if(chkAC(aa,bb,out))
        setFL(auxcarry);

    if(chkFL(carry))
        (*f)=(*f)^carry; //invert carry
}

void Mproc8080::rlc()
{
    if(chkFL(carry)&&(((*a)&0x80)==0)) //set carry according to  high bit in aa
        (*f)=(*f)^carry; //invert carry
    else
        setFL(carry);
    //rotate
    (*a)= ((*a) << 1) | ((*a) >> 7);
}

void Mproc8080::rrc()
{
    if(chkFL(carry)&&(((*a)&0x80)==0)) //set carry according to  high bit in aa
        (*f)=(*f)^carry; //invert carry
    else
        setFL(carry);
    //rotate
    (*a)= ((*a) << 7) | ((*a) >> 1);
}

void Mproc8080::ral()
{
    bool carryST=chkFL(carry);

    if(chkFL(carry)&&(((*a)&0x80)==0)) //set carry according to  high bit in aa
        (*f)=(*f)^carry; //invert carry
    else
        setFL(carry);

    if(carryST)
        (*a)=((*a)<<1)|0x1;
    else
        (*a)=(*a)<<1;

}

void Mproc8080::rar()
{
    bool carryST=chkFL(carry);

    if(chkFL(carry)&&(((*a)&0x1)==0)) //set carry according to  low bit in aa
        (*f)=(*f)^carry; //invert carry
    else
        setFL(carry);

    if(carryST)
        (*a)=((*a)>>1)|0x80;
    else
        (*a)=(*a)>>1;
}

void Mproc8080::cma()
{
    (*a)=~(*a);
}

void Mproc8080::cmc()
{
    (*f)=(*f)^carry; //invert carry
}

void Mproc8080::stc()
{
    setFL(carry);
}

void Mproc8080::jmp()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    (*pc)=(hb<<8)|lb;

}

void Mproc8080::jccc(uchar opcode)
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    uchar con=(opcode&CCC)>>3;

    if(condition(con))
        (*pc)=(hb<<8)|lb;
}

void Mproc8080::call()
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));


    //push pc on stack
    incPC();
    uchar *addr=(uchar*)pc;
    (*sp)--;
    mem->write((*sp),(addr+1));
    (*sp)--;
    mem->write((*sp),addr);

    (*pc)=(hb<<8)|lb;
}

void Mproc8080::cccc(uchar opcode)
{
    incPC();
    ushort lb = readM((*pc));
    incPC();
    ushort hb = readM((*pc));
    uchar con=(opcode&CCC)>>3;
    if(!condition(con))
        return;

    //push pc on stack
    incPC();
    uchar *addr=(uchar*)pc;
    (*sp)--;
    mem->write((*sp),(addr+1));
    (*sp)--;
    mem->write((*sp),addr);


    (*pc)=(hb<<8)|lb;
}

void Mproc8080::ret()
{
    uchar hb=readM((*sp));
    (*sp)++;
    uchar lb=readM((*sp));
    (*sp)++;
    (*pc)=hb|lb;
}

void Mproc8080::rccc(uchar opcode)
{
    uchar con=(opcode&CCC)>>3;
    if(!condition(con))
        return;
    uchar hb=readM((*sp));
    (*sp)++;
    uchar lb=readM((*sp));
    (*sp)++;
    (*pc)=hb|lb;
}

void Mproc8080::rst(uchar opcode)
{
    ushort nnn=0;
    nnn+=(opcode&NNN);
    //push pc on stack
    incPC();
    uchar *addr=(uchar*)pc;
    (*sp)--;
    mem->write((*sp),(addr+1));
    (*sp)--;
    mem->write((*sp),(addr));

    (*pc)=nnn;
}

void Mproc8080::pchl()
{
    (*pc)=(*hl);
}


void Mproc8080::push(uchar opcode)
{
    uchar src = (opcode&RP)>>4;

    if(src==SP)
    {
        (*sp)--;
        mem->write((*sp),a);
        (*sp)--;
        mem->write((*sp),f);
    }
    else
    {
        uchar *addr=(uchar*)pRP(src);
        (*sp)--;
        mem->write((*sp),addr);
        (*sp)--;
        mem->write((*sp),(addr+1));
    }
}

void Mproc8080::pop(uchar opcode)
{
    uchar src = (opcode&RP)>>4;

    if(src==SP)
    {
        (*f)=readM((*sp));
        (*sp)++;
        (*a)=readM((*sp));
        (*sp)++;
    }
    else
    {
        uchar *addr=(uchar*)pRP(src);
        (*(addr+1))=readM((*sp));
        (*sp)++;
        (*addr)=readM((*sp));
        (*sp)++;
    }
}

void Mproc8080::xthl()
{
    uchar l_old =(*l);
    uchar h_old =(*h);

    (*l)=readM(*sp);
    (*h)=readM((*sp)+1);

    mem->write((*sp),&l_old);
    mem->write((*sp)+1,&h_old);
}

void Mproc8080::sphl()
{
    (*sp)=(*hl);
}

void Mproc8080::in()
{
    incPC();
    uchar addr=readM((*pc));
    (*a)=ports[addr];
}

void Mproc8080::out()
{
    incPC();
    uchar addr=readM((*pc));
    ports[addr]=(*a);
}

void Mproc8080::ei()
{
    inte=true;
}

void Mproc8080::di()
{
    inte=false;
}


