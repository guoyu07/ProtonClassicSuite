#ifndef DIALOGEDITAUDIT_H
#define DIALOGEDITAUDIT_H

#include <QDialog>
#include <QtSql>
#include "pcx_auditmodel.h"
#include "auditdatadelegate.h"

namespace Ui {
class DialogEditAudit;
}

class DialogEditAudit : public QWidget
{
    Q_OBJECT

public:
    explicit DialogEditAudit(QWidget *parent = 0);
    ~DialogEditAudit();

private slots:
    void on_comboListAudits_activated(int index);

    void on_treeView_activated(const QModelIndex &index);

private:
    Ui::DialogEditAudit *ui;
    void updateListOfAudits();
    PCx_AuditModel *auditModel;
    auditDataDelegate *delegateDF,*delegateRF,*delegateDI,*delegateRI;

};

#endif // DIALOGEDITAUDIT_H
