#ifndef FORMQUERIES_H
#define FORMQUERIES_H

#include "pcx_auditmodel.h"
#include "pcx_report.h"
#include "pcx_queryvariation.h"
#include "pcx_queriesmodel.h"
#include <QWidget>
#include <QTextDocument>

namespace Ui {
class FormQueries;
}

class FormQueries : public QWidget
{
    Q_OBJECT

public:
    explicit FormQueries(QWidget *parent = 0);
    ~FormQueries();

private slots:
    void on_comboBoxListAudits_activated(int index);

    void on_pushButtonExecReq1_clicked();

    void on_comboBoxAugDim_activated(int index);

    void on_pushButtonSaveReq1_clicked();

    void on_pushButtonDelete_clicked();

    void on_pushButtonExecFromList_clicked();

    void on_listView_activated(const QModelIndex &index);

    void on_pushButtonSave_clicked();

private:
    Ui::FormQueries *ui;
    void updateListOfAudits();
    PCx_AuditModel *model;
    PCx_Report *report;
    PCx_QueriesModel *queriesModel;
    QTextDocument *doc;
    QString currentHtmlDoc;

    bool getParamsReq1(unsigned int &typeId, PCx_AuditModel::ORED &ored, PCx_AuditModel::DFRFDIRI &dfrfdiri,
                       PCx_QueryVariation::INCREASEDECREASE &increase, PCx_QueryVariation::PERCENTORPOINTS &percent,
                       PCx_QueryVariation::OPERATORS &oper, double &val, unsigned int &year1, unsigned int &year2);
    QString execQueries(QModelIndexList items);
};

#endif // FORMQUERIES_H
