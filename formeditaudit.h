#ifndef FORMEDITAUDIT_H
#define FORMEDITAUDIT_H

#include <QtSql>
#include "pcx_auditmodel.h"
#include "auditdatadelegate.h"

namespace Ui {
class FormEditAudit;
}

class FormEditAudit : public QWidget
{
    Q_OBJECT

public:
    explicit FormEditAudit(QWidget *parent = 0);
    ~FormEditAudit();

public slots:
    void onListOfAuditsChanged();


private slots:
    void on_comboListAudits_activated(int index);

    void on_treeView_clicked(const QModelIndex &index);

private:
    Ui::FormEditAudit *ui;
    void updateListOfAudits();
    PCx_AuditModel *auditModel;
    //Perhaps only would be sufficient ?
    auditDataDelegate *delegateDF,*delegateRF,*delegateDI,*delegateRI;
};

#endif // FORMEDITAUDIT_H