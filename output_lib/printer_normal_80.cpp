#include "printer_normal_80.h"
#include "printer_0.h"

////注册
//static Printer_Normal_virtual_Register<Printer_Normal_80> r(QList<QStandardItem*>() << new QStandardItem("Printer_Normal_80"));
////
Printer_Normal_80::Printer_Normal_80()
{
}

Printer_Normal_80::~Printer_Normal_80()
{

}
blockIns Printer_Normal_80::tran(const blockIns &bins, int xinye_beep)
{
    Q_UNUSED(xinye_beep)
    return bins;
}

QString Printer_Normal_80::name()
{
    return "GOODPARTNER";
}
