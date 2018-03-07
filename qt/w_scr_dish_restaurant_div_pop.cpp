﻿#include "w_scr_dish_restaurant_div_pop.h"
#include "ui_w_scr_dish_restaurant_div_pop.h"
#include "ftableview_standmodel_sql_none.h"
#include "restauranttableview.h"
#include "public_sql.h"
#include "frontviewdialog_virtual.h"
#include "n_func.h"
#include "lds_query.h"
#include <QPainter>
#include <QSqlError>
#include "w_scr_dish_main.h"

/**
说明
九宫格选中状态分两种，1：model_check 为 ture， 2：selectindexs contain index
在调用函数返回时，可以同时返回这两种的选择状态
*/

w_scr_dish_restaurant_div_pop::w_scr_dish_restaurant_div_pop(QWidget *parent):
    QDialog(parent),
    ui(new Ui::w_scr_dish_restaurant_div_pop)
{
    ui->setupUi(this);
    //1
    initFTable(ui->tableView);
    model = (ftableview_standmodel_sql_none*)ui->tableView->model();
    delegate9box = (restraurant9BoxDelegate *)ui->tableView->itemDelegate();
    //2
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 3; x++){
            model->item(y, x)->setFlags(Qt::ItemIsEnabled);
            model->item(y, x)->setData(restraurantDelegate::index2chbillno(y*3+x), Model_sqr);
        }
    }
    //4
    connect(ui->pushButton_cancel,SIGNAL(clicked()),this,SLOT(tocancel()));
    connect(ui->tableView,SIGNAL(signalclick(QModelIndex)),this,SIGNAL(signal_div_click(QModelIndex)));
    connect(ui->tableView,SIGNAL(signalclick(QModelIndex)),this,SLOT(toSendChecklist()));
    connect(this, SIGNAL(signal_div_click(QModelIndex)),this,SLOT(accept()));

    ui->pushButton_ok->hide();
    ui->label->setText("");
    this->setWindowTitle(MESSAGE_TITLE_VOID);
}

w_scr_dish_restaurant_div_pop::~w_scr_dish_restaurant_div_pop()
{
    delete ui;
}

void w_scr_dish_restaurant_div_pop::update_by_ch_billnos_ch_tableno(const QMap<int, QString> &ch_billno_map, const QString &ch_tableno, bool show_num_cost)
{
    this->ch_billno_map = ch_billno_map;
    lds_query query;

    for(int y = 0; y < 3; y++) {
        for(int x = 0; x < 3; x++) {
            model->item(x, y)->setData("", Model_ch_billno);
            model->item(x, y)->setData("", Model_check);

            model->item(x, y)->setData("", Qt::UserRole + 2/*int_person*/);
            model->item(x, y)->setData("", Qt::UserRole + 3/*num_cost*/);

            //吧台
            int int_div_id = restraurantDelegate::index2chbillno(x * 3 + y);
            query.execSelect(QString("SELECT ch_billno from cey_bt_table_bar where ch_tableno = '%1' and  int_div_id = %2")
                             .arg(ch_tableno)
                             .arg(int_div_id)
                             );
            model->item(x, y)->setData(query.next(), Qt::UserRole + 8/*div_bar*/);
        }
    }
    //
    QMapIterator<int, QString> mapitr(ch_billno_map);
    while(mapitr.hasNext()) {
        mapitr.next();
        QString ch_billno = mapitr.value();
        int sqr = restraurantDelegate::chbillno2index(mapitr.key());
        model->item(sqr / 3, sqr % 3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        model->item(sqr / 3, sqr % 3)->setData(ch_billno, Model_ch_billno);
        if(show_num_cost) {
            query.execSelect(QString("select int_person, num_cost from cey_u_table where ch_billno = '%1' ")
                             .arg(ch_billno));
            query.next();
            model->item(sqr / 3, sqr % 3)->setData(query.recordValue("int_person").toInt(), Qt::UserRole + 2);
            model->item(sqr / 3, sqr % 3)->setData(query.recordValue("num_cost").toFloat(), Qt::UserRole + 3);
        }
    }

    //22
    if(!ch_tableno.isEmpty()) {
        query.execSelect(QString(" select vch_tablename from cey_bt_table where ch_tableno = '%1' ")
                         .arg(ch_tableno));
        query.next();
        ui->pushButton_ch_tableno->setText(QString("%1:%2(%3)")
                                           .arg(tr("当前桌台"))
                                           .arg(query.recordValue("vch_tablename").toString())
                                           .arg(ch_tableno));
    }

    ui->tableView->setCurrentIndex(QModelIndex());
}

void w_scr_dish_restaurant_div_pop::update_by_ch_billnos(const QMap<int, QString> &ch_billno_map)
{
    update_by_ch_billnos_ch_tableno(ch_billno_map, "", false);
}

QModelIndexList w_scr_dish_restaurant_div_pop::get_cur_check_index_list()// == 0 返回空
{
    QModelIndexList selectindex = ui->tableView->selectionModel()->selectedIndexes();
    QModelIndexList indexlist;
    for(int row = 0; row < 3; row ++) {
        for(int column = 0; column < 3; column ++) {
            QModelIndex index = model->index(row, column);
            if(selectindex.contains(index)
                    || index.data(Model_check).toBool()) {
                if(false == index.data(Qt::UserRole + 7/*disable*/).toBool()){
                    indexlist.append(index);
                }
            }
        }
    }
    return indexlist;
}

QModelIndex w_scr_dish_restaurant_div_pop::getModeIndex(int row, int column)
{
    return model->index(row, column);
}

QString w_scr_dish_restaurant_div_pop::get_ch_billno_by_index(const QModelIndex &index)
{
    return index.data(Model_ch_billno).toString();
}

QString w_scr_dish_restaurant_div_pop::get_sqr_by_curindex()
{
    QModelIndex index = ui->tableView->currentIndex();
    if(index.isValid())
        return index.data(Model_sqr).toString();
    return "";
}

void w_scr_dish_restaurant_div_pop::enableMutilSelected()
{
    ui->tableView->setSelectionMode(QTableView::MultiSelection);
}


bool w_scr_dish_restaurant_div_pop::getMaxChbillnoAdd1_insert_cey_u_master(const QString &k, QString &ret_ch_billno)
{
    ret_ch_billno = getMaxChbillnoAdd1_cey_u_master(k);
    return cey_u_master_insert(ret_ch_billno);
}

QString w_scr_dish_restaurant_div_pop::getMaxChbillnoAdd1_cey_u_master(const QString &k)
{
    lds_query query;
    QDateTime dt = n_func::f_get_sysdatetime();
    QString kyyMMddFF = k + dt.toString("yyMMdd")+lds::LOCAL_TERMINAL_INDEX_FF;

    query.execSelect(QString("select max(ch_billno) from cey_u_table where ch_billno like '%1%' ").arg(kyyMMddFF));

    query.next();
    int id = query.recordValue(0).toString().mid(9).toInt() + 1;
    QString ch_billno = kyyMMddFF + QString().sprintf("%05d",id);

    return ch_billno;
}

bool w_scr_dish_restaurant_div_pop::cey_u_master_insert(const QString &ch_billno)
{
    lds_query query;
    bool f = cey_u_master_insert(query, ch_billno);
    qDebug() << query.recordError();
    return f;
}

bool w_scr_dish_restaurant_div_pop::cey_u_master_insert(lds_query &query, const QString &ch_billno)
{
    return
            query.execInsert("cey_u_master",qrtVariantPairList()
                             << qrtVariantPair("ch_billno", ch_billno)<< qrtVariantPair("ch_state", 1)<< qrtVariantPair("dt_operdate", n_func::f_get_sysdatetime())<< qrtVariantPair("vch_operID", public_sql::gs_operid))
            && query.execInsertDuplicateUpdate("cey_u_master_help",
                                               "ch_billno",
                                               qrtVariantPairList()<< qrtVariantPair("ch_billno", ch_billno)<< qrtVariantPair("date", n_func::f_get_sysdatetime().date())
                                               );
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_update(const QString &ch_tableno, const QString &ch_billno, int sqr, QString *errstring)
{
    lds_query query;
    bool f = cey_bt_table_update(query, ch_tableno, ch_billno, sqr);
    if(false == f)
        if(errstring)
            *errstring = query.recordError();
    return f;
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_update(lds_query &query, const QString &ch_tableno, const QString &ch_billno, int sqr)
{
    bool f = false;
    if(w_scr_dish_main::Q000 == ch_tableno) {
        f = query.execUpdate("cey_sys_lan_terminal", "ch_billno", ch_billno, qrtEqual("vch_terminal", lds::terminalCode));

        return f;
    }
    //中餐
    if(sqr == 0) {
        f = query.execUpdate("cey_bt_table","ch_billno", ch_billno, qrtEqual("ch_tableno", ch_tableno));
    } else {
        f = query.execUpdate("cey_bt_table","ch_billno_"+QString::number(sqr), ch_billno, qrtEqual("ch_tableno", ch_tableno));
    }
    return f;
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_lock(const QString &ch_tableno, int sqr)
{
    lds_query query;
    bool f = false;
    if(sqr == 0) {
        f = query.execUpdate("cey_bt_table","ch_lockflag", "Y",qrtEqual("ch_tableno", ch_tableno));
    } else {
        f = query.execUpdate("cey_bt_table","ch_lockflag_"+QString::number(sqr), "Y",qrtEqual("ch_tableno", ch_tableno));
    }

    return f;
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_unlock(const QString &ch_tableno, int sqr, QString *errstring)//0 - 8
{
    lds_query query;
    if(sqr == 0) {
        return query.execUpdate("cey_bt_table", "ch_lockflag",
                                "N",qrtEqual("ch_tableno", ch_tableno), errstring);
    }

    return query.execUpdate("cey_bt_table", "ch_lockflag_"+QString::number(sqr),
                            "N",qrtEqual("ch_tableno", ch_tableno), errstring);
}

//该餐桌已经被锁定,请确定
bool w_scr_dish_restaurant_div_pop::cey_bt_table_has_locked(const QString &ch_tableno, int sqr)
{
    lds_query query;
    bool f = false;
    if(sqr == 0) {
        query.execSelect(QString("SELECT ch_lockflag from cey_bt_table where ch_tableno = '%1' ")
                             .arg(ch_tableno));
        //
        query.next();
        f = ("Y" == query.recordValue(0).toString());
    } else {
        query.execSelect(QString("SELECT ch_lockflag_%2 from cey_bt_table  where ch_tableno = '%1' ")
                             .arg(ch_tableno)
                             .arg(sqr));
        //
        query.next();
        f = ("Y" == query.recordValue(0).toString());
    }
    return f;
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_has_locked(QWidget *parent, const QString &ch_tableno, int sqr)
{
    if(cey_bt_table_has_locked(ch_tableno, sqr)) {
        lds_messagebox::warning(parent, MESSAGE_TITLE_VOID, tr("该餐桌已经被锁定,请确定"));
        return true;
    }
    return false;
}

int w_scr_dish_restaurant_div_pop::cey_bt_table_div_sqr(const QString &ch_tableno, const QString &ch_billno)
{
    if(w_scr_dish_main::Q000 == ch_tableno) {
        return 0;
    }

    lds_query query;
    query.execSelect(QString("SELECT "
                             " ch_billno  , ch_billno_1,ch_billno_2, "
                             " ch_billno_3, ch_billno_4,ch_billno_5, "
                             " ch_billno_6, ch_billno_7,ch_billno_8  "
                             " from cey_bt_table where ch_tableno = '%1' ")
                     .arg(ch_tableno)
                     );
    if(query.next()) {
        for(int k = 0; k < 9; k++) {
            if(query.recordValue(k).toString() == ch_billno) {
                return k;
            }
        }
    }
    return -1;
}

QString w_scr_dish_restaurant_div_pop::getMaxChpaynoAdd1_cey_u_checkout_master(const QDateTime &dt)
{
    lds_query query;
    QString kyyMMddFF = "P" + dt.toString("yyMMdd") + lds::LOCAL_TERMINAL_INDEX_FF;
    query.execSelect(QString("select Max(ch_payno) from cey_u_checkout_master where   ch_payno like '%1%' ").arg(kyyMMddFF));
    query.next();
    int id = query.recordValue(0).toString().mid(9).toInt() + 1;
    QString ch_payno = kyyMMddFF + QString().sprintf("%05d",id);

    return  ch_payno;
}

QString w_scr_dish_restaurant_div_pop::getMaxChpaynoAdd1_cey_u_checkout_master()
{
    return getMaxChpaynoAdd1_cey_u_checkout_master(n_func::f_get_sysdatetime());
}

QString w_scr_dish_restaurant_div_pop::cey_bt_table_ch_billno_get(const QString &ch_tableno, int sqr)
{
    QString div;
    if(sqr == 0) {
        div = "";
    } else {
        div = "_"+QString::number(sqr);
    }
    lds_query query;
    query.execSelect(QString("SELECT ch_billno%2 from cey_bt_table where ch_tableno = '%1' ")
                     .arg(ch_tableno)
                     .arg(div));
    query.next();

    return query.recordValue(0).toString();
}

bool w_scr_dish_restaurant_div_pop::cey_u_orderdish_dish_existed(const QString &ch_billno)
{
    lds_query query;
    query.execSelect(QString("SELECT ch_billno from cey_u_orderdish where ch_billno = '%1' limit 1")
                     .arg(ch_billno));
    return query.next();
}

bool w_scr_dish_restaurant_div_pop::cey_bt_table_bar_ch_billno_existed(const QString &ch_tableno, int sqr)
{
    lds_query query;
    query.execSelect(QString("select ch_billno from cey_bt_table_bar where ch_tableno = '%1' and int_div_id = %2 ")
                     .arg(ch_tableno)
                     .arg(sqr));
    return query.next();
}

void w_scr_dish_restaurant_div_pop::initFTable(ftableview *tableView)
{
    ftableview_standmodel_sql_none *model = new ftableview_standmodel_sql_none(tableView);

    restraurant9BoxDelegate *delegate9box = new restraurant9BoxDelegate(tableView);

    delegate9box->preprinttablemap = &public_sql::preprinttablemap;
    tableView->setModel(model);
    tableView->setItemDelegate(delegate9box);
    tableView->updateFixed_fillData_addAnimation(3, 3, 100, 100);
    tableView->setSelectionMode(QTableView::SingleSelection);
    tableView->setSelectionBehavior(QTableView::SelectItems);
    tableView->setStyleSheet("border:0;background:transparent;selection-background-color: transparent;");

}

void w_scr_dish_restaurant_div_pop::enableSinglSelectedWhenMutil()
{
    if(ui->tableView->selectionMode() == QTableView::MultiSelection) {
        ui->tableView->enableSelectOneWhenMultiSelected();
    }
}

void w_scr_dish_restaurant_div_pop::enableItemFlagSelectedWithAll()
{
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 3; x++){
            model->item(y, x)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }
    }
}

void w_scr_dish_restaurant_div_pop::enableItemFlagSelectedWith_nochbillno()
{
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 3; x++){
            if(model->item(y, x)->data(Model_ch_billno).toString().isEmpty()) {
                model->item(y, x)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            } else {
                model->item(y, x)->setFlags(Qt::ItemIsEnabled);
            }
        }
    }
}

void w_scr_dish_restaurant_div_pop::enableItemFlagSelectedWith_chbillno_bar()
{
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 3; x++){
            //
            model->item(y, x)->setFlags(Qt::ItemIsEnabled);
            //
            if(model->item(y, x)->data(Model_ch_billno).toString().length() > 0) {
                model->item(y, x)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            }
            //
            if(model->item(y, x)->data(Qt::UserRole + 8/*div_bar*/).toBool()) {
                model->item(y, x)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            }
        }
    }
}

void w_scr_dish_restaurant_div_pop::setcurindex_by_sqr(int sqr)
{
    ui->tableView->setCurrentIndex(QModelIndex());

    for(int row = 0; row < 3; row ++){
        for(int column = 0; column < 3; column ++){
            if(model->data(model->index(row, column), Model_sqr).toInt() == sqr){
                ui->tableView->setCurrentIndex(model->index(row, column));
                return;
            }
        }
    }
}

QString w_scr_dish_restaurant_div_pop::get_ch_billno_by_sqr(QString sqr)
{
    for(int row = 0; row < 3; row ++){
        for(int column = 0; column < 3; column ++){
            if(model->data(model->index(row, column), Model_sqr).toString() == sqr){
                return model->data(model->index(row, column), Model_ch_billno).toString();
            }
        }
    }
    return "";
}

void w_scr_dish_restaurant_div_pop::setFlagWith_KAITAI_YUDA()
{
    QPixmap p(50, 50);
    QPainter painter;
    painter.begin(&p);
    painter.fillRect(QRect(0, 0, 50, 50), restraurantDelegate::gettableState(tableState_KAITAI).colorname1);
    QPolygonF polygon(QVector<QPointF>() << QPointF(50, 0) << QPointF(50, 50) << QPointF(0, 50));
    QPainterPath path;
    path.addPolygon(polygon);
    painter.fillPath(path, restraurantDelegate::color_light(restraurantDelegate::gettableState(tableState_YUDA).colorname1, 2));
    painter.end();

    ui->pushButton_ch_tableno->setIcon(p);
}

void w_scr_dish_restaurant_div_pop::setShowDesc(const QString &text)
{
    ui->label->setText(text);
    ui->label->setStyleSheet("color:#ff650b");
}

void w_scr_dish_restaurant_div_pop::setDesc(const QString &desc)
{
    ui->label_desc->setText(desc);
}

void w_scr_dish_restaurant_div_pop::tocancel()
{
    this->reject();
}

void w_scr_dish_restaurant_div_pop::toSendChecklist()
{
    //checklist
    QString checklist;
    QModelIndexList indexlist = get_cur_check_index_list();
    foreach(const QModelIndex &index, indexlist){
        checklist += index.data(Model_sqr).toString() + ",";
    }
    emit signal_div_check(checklist);
}
//=========================
w_scr_dish_restaurant_div_pop_only_table::w_scr_dish_restaurant_div_pop_only_table(bool isTransparent, QWidget *parent)
    : w_scr_dish_restaurant_div_pop(parent)
{
    if(isTransparent)
        this->setStyleSheet("background:transparent;");
    iscombine = false;
    //1
    ui->tableView->updateFixed(3, 3, 50, 50);
    resize(ui->tableView->size());
    lds::hideWidget(ui->frame);
    lds::hideWidget(ui->pushButton_ch_tableno);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);

    disconnect(this, SIGNAL(signal_div_click(QModelIndex)),this,SLOT(accept()));
}

void w_scr_dish_restaurant_div_pop_only_table::trycombine()
{
    iscombine = false;
    if(restraurantDelegate::ch_billno_is_div(ch_billno_map)){

    } else {
        if(ch_billno_map.isEmpty()) {
            qDebug() << "ch_billno_map is empty";
        }
        QString ch_billno = ch_billno_map.value(0);
        iscombine = true;
        ui->tableView->hideColumn(0);
        ui->tableView->hideColumn(2);
        ui->tableView->hideRow(0);
        ui->tableView->hideRow(2);
        ui->tableView->setRowHeight(1, 50 * 3);
        ui->tableView->setColumnWidth(1, 50 * 3);

        lds_query query;
        query.execSelect(QString("select y.* from  cey_u_table x, cey_bt_table y  where x.ch_tableno = y.ch_tableno and x.ch_billno = '%1' ")
                         .arg(ch_billno));
        if(query.next()){
            model->setData(model->index(1, 1), query.recordValue("vch_tablename").toString(), Qt::UserRole + 5/*vch_tablename*/);
            model->setData(model->index(1, 1), query.recordValue("ch_tableno").toString(), Qt::UserRole + 6/*ch_tableno*/);
        }
        //
        delegate9box->tryCombine();
    }
}

//===================
w_scr_dish_restaurant_div::w_scr_dish_restaurant_div(const QMap<int, QString> &ch_billno_map, const QString &ch_tableno, QWidget *parent)
    : w_scr_dish_restaurant_div_pop(parent)
{

    this->update_by_ch_billnos_ch_tableno(ch_billno_map, ch_tableno);

    disconnect(this, SIGNAL(signal_div_click(QModelIndex)),this,SLOT(accept()));

    ui->pushButton_ok->show();
    connect(ui->pushButton_ok,SIGNAL(clicked()),this,SLOT(accept()));
}
