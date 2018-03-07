#include "waimai_pay.h"
#include "ui_waimai_pay.h"
#include <QSqlRecord>
#include "lds_messagebox.h"
#include "other_pay_view.h"
#include "bank_card.h"
#include "lds_query.h"
#include <QDateTime>
#include "n_func.h"
#include "take_cashier.h"
#include <QSqlError>
#include "backheader.h"
#include "lds_messagebox.h"
#include "public_printby_ch_billno.h"
#include "w_scr_dish_restaurant_div_pop.h"

waimai_pay::waimai_pay(const QString &xiaofei,const QString &zhekou,const QString &zengsong,const QString &moling,const QString &shuishou,const QString &billno,const QString &waimaihao,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::waimai_pay)
{
    ui->setupUi(this);

    _xiaofei = xiaofei;
    _zhekou = zhekou;
    _zengsong = zengsong;
    _moling = moling;
    _shuishou = shuishou;
    _billno = billno;
    _waimaihao = waimaihao.trimmed();

    kafujine = 0;
    shifoujiazhang = _waimaihao;
    btn_t_f = false;
    init_control();
    init_data();
}

waimai_pay::~waimai_pay()
{
    delete ui;
}

void waimai_pay::init_control()
{
    ui->lineEdit_consum->setEnabled(false);
    ui->lineEdit_discount->setEnabled(false);
    ui->lineEdit_present->setEnabled(false);
    ui->lineEdit_realreceive->setEnabled(false);
    ui->lineEdit_blotout->setEnabled(false);
    ui->lineEdit_int_rate->setEnabled(false);
    ui->lineEdit_change->setEnabled(false);
    ui->lineEdit_9->setFocus();

    table_model = new QStandardItemModel();
    QStringList list;
    list <<tr("行号")<<tr("付款方式")<<tr("实付现金")<<tr("付款金额")<<tr("凭证号");
    table_model->setHorizontalHeaderLabels(list);
    ui->tableView->setModel(table_model);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
}

void waimai_pay::init_data()
{
    ui->lineEdit_consum->setValue(_xiaofei.toDouble());
    ui->lineEdit_discount->setValue(_zhekou.toDouble()); ui->lineEdit_discount->enableShowNegativeValue();
    ui->lineEdit_present->setValue(_zengsong.toDouble()); ui->lineEdit_present->enableShowNegativeValue();
    ui->lineEdit_blotout->setValue(_moling.toDouble()); ui->lineEdit_blotout->enableShowNegativeValue();
    ui->lineEdit_int_rate->setValue(_shuishou.toDouble());

    double yingshou = 0;
    yingshou = _xiaofei.toDouble() - _zhekou.toDouble() - _zengsong.toDouble() - _moling.toDouble() + _shuishou.toDouble();
    ui->lineEdit_realreceive->setValue(yingshou);

    ui->lineEdit_9->setValue(yingshou);

    ui->lineEdit_change->setValue(0);
    ui->label_12->setValue(0);
    ui->label_14->setValue(yingshou);
}

void waimai_pay::on_pushButton_21_clicked()//取消
{
    btn_t_f = false;
    this->close();
}

void waimai_pay::on_pushButton_19_clicked()//代金券
{
    float label_14 = 0;
    label_14 = ui->label_14->value();
    if(ui->label_14->value() == 0) {
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("已足额付款无需继续付款"));
        return;
    }
    QString timer;
    bank_card dialog(QString::number(ui->label_14->value()), this);
    dialog.setWindowTitle(tr("代金券"));
    lds_roundeddialog_rect_align(&dialog).exec();
    if(dialog.btn_t_f == true)  {
        kafujine = dialog.shishoujine;
        fukuanpingzheng = dialog.pingzhenghao;
        if(ui->label_14->value() <= dialog.shishoujine)  {
            ui->lineEdit_9->setValue(0);
            float l_12 = 0;
            l_12 = ui->label_14->value() + ui->label_12->value();
            ui->label_12->setValue((l_12));
            ui->label_14->setValue(0);
        }   else
        {
            float yue = 0;
            yue = ui->label_14->value() - dialog.shishoujine;
            ui->lineEdit_9->setValue((yue));
            ui->label_12->setValue((dialog.shishoujine + ui->label_12->value()));
            ui->label_14->setValue((yue));
        }
        timer = dialog.time;
    } else
    {
        return;
    }

    int row = table_model->rowCount();
    table_model->setRowCount(row + 1);
    float t_shifuxianjin = 0;
    float t_fukuanjie = 0;
    if(label_14 <= kafujine) {//付清
        t_shifuxianjin = label_14;
        t_fukuanjie = kafujine;
    } else//未付清
    {
        t_shifuxianjin = kafujine;
        t_fukuanjie = kafujine;
    }

    table_model->setItem(row,0,new QStandardItem(QString::number(row+1))); //序号
    table_model->setItem(row,1,new QStandardItem(tr("代金券")));
    table_model->setItem(row,2,new QStandardItem(QString().sprintf("%0.2f",t_shifuxianjin)));
    table_model->setItem(row,3,new QStandardItem(QString().sprintf("%0.2f",t_fukuanjie)));
    table_model->setItem(row,4,new QStandardItem(fukuanpingzheng));
    table_model->setItem(row,5,new QStandardItem("04"));
    table_model->setItem(row,6,new QStandardItem(dialog.vch_memeo));
    table_model->setItem(row,7,new QStandardItem(timer));
    ui->tableView->selectRow(row);

    ui->tableView->setColumnHidden(5,true);
    ui->tableView->setColumnHidden(6,true);
    ui->tableView->setColumnHidden(7,true);
}

void waimai_pay::on_pushButton_20_clicked()//其他付款
{
    float label_14 = 0;
    label_14 = ui->label_14->value();

    if(ui->label_14->value() == 0) {
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("已足额付款无需继续付款"));
        
        return;
    }

    other_pay_view pay_dialog("2", this);
    pay_dialog.setWindowTitle(_TEXT_SLOT(this));
    lds_roundeddialog_rect_align(&pay_dialog).exec();
    if(pay_dialog.btn_t_f == true)  {
        QString timer;
        bank_card dialog(QString::number(ui->label_14->value()), this);
        dialog.setWindowTitle(pay_dialog.payname[pay_dialog.curindex_id]);
        lds_roundeddialog_rect_align(&dialog).exec();
        if(dialog.btn_t_f == true)   {
            kafujine = dialog.shishoujine;
            fukuanpingzheng = dialog.pingzhenghao;
            if(ui->label_14->value() <= dialog.shishoujine)   {
                ui->lineEdit_9->setValue(0);
                float l_12 = 0;
                l_12 = ui->label_14->value() + ui->label_12->value();
                ui->label_12->setValue((l_12));
                ui->label_14->setValue(0);
            }   else
            {
                float yue = 0;
                yue = ui->label_14->value() - dialog.shishoujine;
                ui->lineEdit_9->setValue((yue));
                ui->label_12->setValue((dialog.shishoujine + ui->label_12->value()));
                ui->label_14->setValue((yue));
            }
            timer = dialog.time;
        }   else
        {
            return;
        }

        lds_query model;
        model.execSelect(QString("select ch_paymodeno,ch_incomeflag from cey_bt_paymode where ch_paymodeno = '%1' ")
                         .arg(pay_dialog.payfanshi[pay_dialog.curindex_id]));
        model.next();
        QString paytype;
        model.next();
        {
            paytype = model.value(0).toString();
        }

        int row = table_model->rowCount();
        table_model->setRowCount(row + 1);
        float t_shifuxianjin = 0;
        float t_fukuanjie = 0;
        if(label_14 <= kafujine) {//付清
            t_shifuxianjin = label_14;
            t_fukuanjie = kafujine;
        }   else//未付清
        {
            t_shifuxianjin = kafujine;
            t_fukuanjie = kafujine;
        }

        table_model->setItem(row,0,new QStandardItem(QString::number(row+1))); //序号
        table_model->setItem(row,1,new QStandardItem(pay_dialog.payname[pay_dialog.curindex_id]));
        table_model->setItem(row,2,new QStandardItem(QString().sprintf("%0.2f",t_shifuxianjin)));
        table_model->setItem(row,3,new QStandardItem(QString().sprintf("%0.2f",t_fukuanjie)));
        table_model->setItem(row,4,new QStandardItem(fukuanpingzheng));
        table_model->setItem(row,5,new QStandardItem(paytype));
        table_model->setItem(row,6,new QStandardItem(dialog.vch_memeo));
        table_model->setItem(row,7,new QStandardItem(timer));
        ui->tableView->selectRow(row);
        ui->tableView->setColumnHidden(5,true);
        ui->tableView->setColumnHidden(6,true);
        ui->tableView->setColumnHidden(7,true);
    }

}

void waimai_pay::on_pushButton_13_clicked()//买单
{
    if(shifoujiazhang != _waimaihao)  {
        return;
    }
    if(ui->lineEdit_9->value() + ui->label_12->value() < ui->lineEdit_realreceive->value()) {
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("对不起,您的付款金额不够"));
        
        return;
    }

    QSqlQueryModel m_billno;
    QString ch_payno;
    //    QString danhao;
    m_billno.setQuery("Select NOW()");
    QDateTime ldt_datetime;
    ldt_datetime.setTime(m_billno.record(0).value(0).toTime());
    ldt_datetime.setDate(m_billno.record(0).value(0).toDate());
    QString time = m_billno.record(0).value(0).toDateTime().toString("yyyy-MM-dd hh:mm:ss");

    ch_payno = w_scr_dish_restaurant_div_pop::getMaxChpaynoAdd1_cey_u_checkout_master(ldt_datetime);
    lds_query query;
    lds_query::tran_saction();
    QString zhuohao = "Q000";

    //付款明细
    if(query.execInsert("cey_u_checkout_master",qrtVariantPairList()
                        << qrtVariantPair("ch_payno", ch_payno)
                        << qrtVariantPair("ch_billno", _billno)
                        << qrtVariantPair("ch_tableno", zhuohao)
                        << qrtVariantPair("num_cost", ui->lineEdit_consum->value())
                        << qrtVariantPair("num_discount", ui->lineEdit_discount->value())

                        << qrtVariantPair("num_present", ui->lineEdit_present->value())
                        << qrtVariantPair("num_service", 0)
                        << qrtVariantPair("num_lowcost", 0)
                        << qrtVariantPair("num_blotout", ui->lineEdit_blotout->value())
                        << qrtVariantPair("num_run", 0)

                        << qrtVariantPair("vch_run_operID", "")
                        << qrtVariantPair("ch_state", "Y")
                        << qrtVariantPair("vch_operID", public_sql::gs_operid)
                        << qrtVariantPair("dt_operdate", time)
                        << qrtVariantPair("vch_operID2", QVariant())

                        << qrtVariantPair("dt_operdate2", QVariant())
                        << qrtVariantPair("num_rate", ui->lineEdit_int_rate->value())
                        ))
    {

    }
    else
    {
        lds_query::roll_back();
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("插入cey_u_checkout_master失败"));
        return;
    }


    //现金付款
    if(ui->label_14->value() != 0)  {
        float aa = 0;
        aa = ui->lineEdit_9->value() - ui->lineEdit_change->value();
        if(!query.execInsert("cey_u_checkout_detail" ,qrtVariantPairList()
                             << qrtVariantPair("ch_payno", ch_payno)
                             << qrtVariantPair("ch_paymodeno", "00")
                             << qrtVariantPair("num_payamount", ui->lineEdit_9->value())
                             << qrtVariantPair("num_change", ui->lineEdit_change->value())
                             << qrtVariantPair("num_realamount", QString().sprintf("%0.2f",aa))

                             << qrtVariantPair("num_face", 0)
                             << qrtVariantPair("vch_operID", public_sql::gs_operid)
                             << qrtVariantPair("dt_operdate", time)
                             << qrtVariantPair("num_card_remain", 0)
                             << qrtVariantPair("vch_voucherno", QVariant())

                             << qrtVariantPair("num_face_Num", 0)))
        {
            lds_query::roll_back();
            lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("cey_u_checkout_detail失败"));
            
            return;
        }
    }

    if(ui->lineEdit_blotout->value() != 0) {//存在自动抹零
        if(query.execInsert("cey_u_checkout_detail",qrtVariantPairList()
                            << qrtVariantPair("ch_payno", ch_payno)
                            << qrtVariantPair("ch_paymodeno", "YY")
                            << qrtVariantPair("num_payamount", ui->lineEdit_blotout->value())
                            << qrtVariantPair("num_change", 0)
                            << qrtVariantPair("num_realamount", ui->lineEdit_blotout->value())

                            << qrtVariantPair("num_face", 0)
                            << qrtVariantPair("vch_operID", public_sql::gs_operid)
                            << qrtVariantPair("dt_operdate", time)
                            << qrtVariantPair("num_card_remain", 0)
                            << qrtVariantPair("vch_voucherno", QVariant())

                            << qrtVariantPair("num_face_Num", 0)))
        {

        }
        else
        {
            lds_query::roll_back();
            lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("cey_u_checkout_detail失败"));
            
            return;
        }
    }


    for(int i = 0; i < table_model->rowCount(); i++)
    {
        QString pingzheng = table_model->item(i,4)->text();
        if(pingzheng.trimmed().isEmpty()){
            pingzheng = "null";
        }

        if(query.execInsert("cey_u_checkout_detail", qrtVariantPairList()
                            << qrtVariantPair("ch_payno", ch_payno)<< qrtVariantPair("ch_paymodeno", table_model->item(i,5)->text())<< qrtVariantPair("num_payamount", table_model->item(i,2)->text())<< qrtVariantPair("num_change", 0)<< qrtVariantPair("num_realamount", table_model->item(i,3)->text())
                            << qrtVariantPair("num_face", 0)<< qrtVariantPair("vch_operID", public_sql::gs_operid)<< qrtVariantPair("dt_operdate", time)<< qrtVariantPair("num_card_remain", 0)<< qrtVariantPair("vch_voucherno", pingzheng)
                            << qrtVariantPair("num_face_Num", 0)))
        {

        }
        else
        {
            lds_query::roll_back();
            lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("cey_u_checkout_detail失败"));
            
            return;
        }
    }

    //付款成功
    if(query.execUpdate("cey_u_table","ch_payno", ch_payno,qrtEqual("ch_billno", _billno))) {
    }
    else
    {
        lds_query::roll_back();
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("更新cey_u_table失败"));

        return;
    }
    if(!query.execUpdate("cey_u_orderdish",qrtVariantPairList()
                          << qrtVariantPair("ch_payno", ch_payno),
                         QString("ch_togono = '%1' ").arg(_waimaihao))) {

        lds_query::roll_back();
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("更新cey_u_orderdish失败"));
        return;
    }

    if(query.execUpdate("cey_u_togo",qrtVariantPairList()
                         << qrtVariantPair("ch_payno", ch_payno) << qrtVariantPair("ch_state", "Y"),
                        qrtEqual("ch_togono", _waimaihao)))
    {

    }
    else
    {
        lds_query::roll_back();
        lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("更新cey_u_togo失败"));

        return;
    }

    QString waimai_weixin_info;

    query.execSelect(QString("SELECT vch_yun_sn FROM hddpos.cey_u_togo where ch_togono = '%1'; ").arg(_waimaihao));
    query.next();
    QString vch_yun_sn = query.recordValue("vch_yun_sn").toString();
    if(vch_yun_sn.isEmpty()
            ||(!vch_yun_sn.isEmpty()
               &&w_sys_manage_cloudsync::up_weixin_change_takkeoutstate(this
                                                                        ,vch_yun_sn
                                                                        ,"21"//交易结束-交易成功
                                                                        )
               )
            ){
        if(!vch_yun_sn.isEmpty()) waimai_weixin_info=(tr("[微信")+tr("交易结束-交易成功!]"));
        lds_query::com_mit();

        pay_print();

        shifoujiazhang.clear();
        btn_t_f = true;
        lds_messagebox::information(this, MESSAGE_TITLE_VOID,tr("外卖交易成功") +waimai_weixin_info);
        this->close();
        return;
    }
    lds_messagebox::warning(this, MESSAGE_TITLE_VOID,tr("外卖交易失败"));
    lds_query::roll_back();
    return;
}

void waimai_pay::on_lineEdit_9_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    {
        float xianjin = 0;
        xianjin = ui->lineEdit_9->value();
        float yingshou = 0;
        yingshou = ui->label_14->value();
        if(xianjin > yingshou)   {
            ui->lineEdit_change->setValue(xianjin - yingshou);
        }
        else    {
            ui->lineEdit_change->setValue(0);
        }
    }
}

void waimai_pay::print_lost(const QString &str)
{
    ;
    lds_messagebox::warning(this, MESSAGE_TITLE_VOID,str);

}

void waimai_pay::pay_print()
{
    QString path  = lds::sysconf->value("system_setting/printerpath").toString();
    //开钱箱
    if(ui->lineEdit_9->value() != 0  //现金不为0
            && lds::sysconf->value("system_setting/qianxiang", true).toBool()){
        blockIns ins;
        ins += pieceIns(lds::sysconf->value("system_setting/qianxiangzhiling").toByteArray(), "");
        backheader::printer.synchPrint(ins,  path);
    }
    //根据单号
    public_printby_ch_billno printer(_billno);
    printer.toprintwaimai_pay(printer.defpath(), n_func::f_get_sysparm("bp_togo_count", "1").toInt());
}

void waimai_pay::on_pushButton_11_clicked()//<<
{
    QPushButton *b = qobject_cast<QPushButton *>(this->sender());
    if(b){
        ui->lineEdit_9->setValue(b->text().toDouble());
        ui->lineEdit_9->selectAll();
    }
}

void waimai_pay::on_pushButton_7_clicked()//100
{
    QPushButton *b = qobject_cast<QPushButton *>(this->sender());
    if(b){
        ui->lineEdit_9->setValue(b->text().toDouble());
        ui->lineEdit_9->selectAll();
    }
}

void waimai_pay::on_pushButton_4_clicked()//50
{
    QPushButton *b = qobject_cast<QPushButton *>(this->sender());
    if(b){
        ui->lineEdit_9->setValue(b->text().toDouble());
        ui->lineEdit_9->selectAll();
    }
}
