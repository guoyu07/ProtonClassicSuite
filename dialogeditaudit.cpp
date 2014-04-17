#include "dialogeditaudit.h"
#include "ui_dialogeditaudit.h"
#include "pcx_auditmodel.h"
#include "pcx_auditinfos.h"
#include "auditdatadelegate.h"

DialogEditAudit::DialogEditAudit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DialogEditAudit)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1,1);
    auditModel=NULL;

    delegateDF=new auditDataDelegate(ui->tableViewDF);
    delegateRF=new auditDataDelegate(ui->tableViewRF);
    delegateDI=new auditDataDelegate(ui->tableViewDI);
    delegateRI=new auditDataDelegate(ui->tableViewRI);

    ui->tableViewDF->setItemDelegate(delegateDF);
    ui->tableViewRF->setItemDelegate(delegateRF);
    ui->tableViewDI->setItemDelegate(delegateDI);
    ui->tableViewRI->setItemDelegate(delegateRI);

    updateListOfAudits();
}

DialogEditAudit::~DialogEditAudit()
{
    delete ui;
    if(auditModel!=NULL)
    {
        delete auditModel;
    }
    delete(delegateDF);
    delete(delegateRF);
    delete(delegateDI);
    delete(delegateRI);
}

void DialogEditAudit::updateListOfAudits()
{
    ui->comboListAudits->clear();

    QHash<int,QString> listOfAudits=PCx_AuditModel::getListOfAudits(false);
    foreach(int auditId,listOfAudits.keys())
    {
        ui->comboListAudits->insertItem(0,listOfAudits[auditId],auditId);
    }
    ui->comboListAudits->setCurrentIndex(0);
    on_comboListAudits_activated(0);
}

void DialogEditAudit::on_comboListAudits_activated(int index)
{
    if(index==-1||ui->comboListAudits->count()==0)return;
    unsigned int selectedAuditId=ui->comboListAudits->currentData().toUInt();
    qDebug()<<"Selected audit ID = "<<selectedAuditId;
    if(auditModel!=NULL)
    {
        delete auditModel;
    }
    auditModel=new PCx_AuditModel(selectedAuditId);
    ui->treeView->setModel(auditModel->getAttachedTreeModel());
    ui->treeView->expandToDepth(1);



    ui->tableViewDF->setModel(auditModel->getModelDF());
    ui->tableViewDF->hideColumn(0);
    ui->tableViewDF->hideColumn(1);
    ui->tableViewRF->setModel(auditModel->getModelRF());
    ui->tableViewRF->hideColumn(0);
    ui->tableViewRF->hideColumn(1);
    ui->tableViewDI->setModel(auditModel->getModelDI());
    ui->tableViewDI->hideColumn(0);
    ui->tableViewDI->hideColumn(1);
    ui->tableViewRI->setModel(auditModel->getModelRI());
    ui->tableViewRI->hideColumn(0);
    ui->tableViewRI->hideColumn(1);

    //Roots
    auditModel->getModelDF()->setFilter(QString("id_node=1"));
    auditModel->getModelRF()->setFilter(QString("id_node=1"));
    auditModel->getModelDI()->setFilter(QString("id_node=1"));
    auditModel->getModelRI()->setFilter(QString("id_node=1"));
    ui->label->setText(auditModel->getAttachedTreeModel()->index(0,0).data().toString());

    ui->tableViewDF->setEnabled(false);
    ui->tableViewRF->setEnabled(false);
    ui->tableViewDI->setEnabled(false);
    ui->tableViewRI->setEnabled(false);
}

void DialogEditAudit::on_treeView_activated(const QModelIndex &index)
{
    unsigned int selectedNode=index.data(Qt::UserRole+1).toUInt();
    Q_ASSERT(selectedNode>0);

    auditModel->getModelDF()->setFilter(QString("id_node=%1").arg(selectedNode));
    auditModel->getModelRF()->setFilter(QString("id_node=%1").arg(selectedNode));
    auditModel->getModelDI()->setFilter(QString("id_node=%1").arg(selectedNode));
    auditModel->getModelRI()->setFilter(QString("id_node=%1").arg(selectedNode));
    bool isLeaf=auditModel->getAttachedTreeModel()->isLeaf(selectedNode);

    ui->tableViewDF->setEnabled(isLeaf);
    ui->tableViewRF->setEnabled(isLeaf);
    ui->tableViewDI->setEnabled(isLeaf);
    ui->tableViewRI->setEnabled(isLeaf);
    ui->label->setText(index.data().toString());
}
