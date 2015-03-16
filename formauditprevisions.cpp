#include "formauditprevisions.h"
#include "ui_formauditprevisions.h"
#include "pcx_prevision.h"
#include "utils.h"
#include <QListWidgetItem>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include "pcx_graphics.h"
#include "formdisplayprevisionreport.h"

FormAuditPrevisions::FormAuditPrevisions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAuditPrevisions)
{
    previsionModel=nullptr;
    auditWithTreeModel=nullptr;
    currentPrevisionItemTableModel=nullptr;
    currentPrevisionItem=nullptr;
    graphics=nullptr;

    recentPrevisionItem=nullptr;
    recentPrevisionItemTableModel=nullptr;


    currentMode=MODES::DFRFDIRI::DF;
    currentNodeId=1;
    ui->setupUi(this);
    if(!ui->checkBoxDisplayLeafCriteria->isChecked())
    {
        ui->textBrowser->setHidden(true);
    }
    PCx_PrevisionItemCriteria::fillComboBoxWithOperators(ui->comboBoxOperators);
    ui->comboBoxORED->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::OUVERTS,true),PCx_Audit::ORED::OUVERTS);
    ui->comboBoxORED->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::REALISES,true),PCx_Audit::ORED::REALISES);
    ui->comboBoxORED->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::ENGAGES,true),PCx_Audit::ORED::ENGAGES);
    ui->comboBoxORED->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::DISPONIBLES,true),PCx_Audit::ORED::DISPONIBLES);

    ui->comboBoxOREDDispatchRef->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::OUVERTS,true),PCx_Audit::ORED::OUVERTS);
    ui->comboBoxOREDDispatchRef->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::REALISES,true),PCx_Audit::ORED::REALISES);
    ui->comboBoxOREDDispatchRef->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::ENGAGES,true),PCx_Audit::ORED::ENGAGES);
    ui->comboBoxOREDDispatchRef->addItem(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::DISPONIBLES,true),PCx_Audit::ORED::DISPONIBLES);
    ui->comboBoxOREDDispatchRef->setCurrentText(PCx_Audit::OREDtoCompleteString(PCx_Audit::ORED::REALISES,true));


    connect(ui->textBrowser,SIGNAL(anchorClicked(QUrl)),this,SLOT(onAnchorClicked(QUrl)));

    updateListOfPrevisions();
}


FormAuditPrevisions::~FormAuditPrevisions()
{
    delete ui;
    if(previsionModel!=nullptr)
    {
        delete previsionModel;
    }
    if(auditWithTreeModel!=nullptr)
    {
        delete auditWithTreeModel;
    }
    if(currentPrevisionItemTableModel!=nullptr)
    {
        delete currentPrevisionItemTableModel;
    }
    if(currentPrevisionItem!=nullptr)
    {
        delete currentPrevisionItem;
    }
    if(recentPrevisionItemTableModel!=nullptr)
    {
        delete recentPrevisionItemTableModel;
    }

    if(recentPrevisionItem!=nullptr)
    {
        delete recentPrevisionItem;
    }
    if(graphics!=nullptr)
    {
        delete graphics;
    }

}

void FormAuditPrevisions::onListOfPrevisionsChanged()
{
    updateListOfPrevisions();
}

void FormAuditPrevisions::onSettingsChanged()
{
    if(auditWithTreeModel==nullptr)
        return;
    if(ui->treeView->currentIndex().isValid())
    {
        on_treeView_clicked(ui->treeView->currentIndex());
    }
    else
    {
        QModelIndex rootIndex=auditWithTreeModel->getAttachedTree()->index(0,0);
        on_treeView_clicked(rootIndex);
    }
}

void FormAuditPrevisions::updateListOfPrevisions()
{
    ui->comboListPrevisions->clear();

    QList<QPair<unsigned int,QString> >listOfPrevisions=PCx_Prevision::getListOfPrevisions();
    bool nonEmpty=!listOfPrevisions.isEmpty();
    this->setEnabled(nonEmpty);

    QPair<unsigned int, QString> p;
    foreach(p,listOfPrevisions)
    {
        ui->comboListPrevisions->insertItem(0,p.second,p.first);
    }
    ui->comboListPrevisions->setCurrentIndex(0);
    on_comboListPrevisions_activated(0);
}

void FormAuditPrevisions::updatePrevisionItemTableModel()
{
    if(currentPrevisionItem!=nullptr)
    {
        delete currentPrevisionItem;
        currentPrevisionItem=nullptr;
    }
    currentPrevisionItem=new PCx_PrevisionItem(previsionModel,currentMode,currentNodeId,auditWithTreeModel->getYears().last()+1);
    currentPrevisionItem->loadFromDb();
    if(currentPrevisionItemTableModel!=nullptr)
    {
        currentPrevisionItemTableModel->setPrevisionItem(currentPrevisionItem);
    }
    else
    {
        currentPrevisionItemTableModel=new PCx_PrevisionItemTableModel(currentPrevisionItem,this);
        ui->tableViewCriteria->setModel(currentPrevisionItemTableModel);
    }

    if(recentPrevisionItem!=nullptr)
    {
        recentPrevisionItem->setNodeId(currentNodeId);
        recentPrevisionItem->setMode(currentMode);

        if(recentPrevisionItemTableModel!=nullptr)
            recentPrevisionItemTableModel->resetModel();
    }
    else
    {
        recentPrevisionItem=new PCx_PrevisionItem(previsionModel,currentMode,currentNodeId,auditWithTreeModel->getYears().last()+1);
    }
    if(recentPrevisionItemTableModel==nullptr)
    {
        recentPrevisionItemTableModel=new PCx_PrevisionItemTableModel(recentPrevisionItem,this);
        ui->tableViewRecentCriteria->setModel(recentPrevisionItemTableModel);
    }
}

void FormAuditPrevisions::updateLabels()
{
    const QString &nodeName=auditWithTreeModel->getAttachedTree()->getNodeName(currentNodeId);
    ui->labelNodeName->setText(nodeName);
    qint64 lastValueN=auditWithTreeModel->getNodeValue(currentNodeId,currentMode,PCx_Audit::ORED::OUVERTS,auditWithTreeModel->getYears().last());
    ui->labelValueN->setText(NUMBERSFORMAT::formatFixedPoint(lastValueN));
    ui->labelValueNplus1->setText(NUMBERSFORMAT::formatFixedPoint(currentPrevisionItem->getSummedPrevisionItemValue()));
    ui->labelValuePrevisionItem->setText(NUMBERSFORMAT::formatFixedPoint(recentPrevisionItem->getPrevisionItemValue()));
    ui->labelValueAppliedItems->setText(NUMBERSFORMAT::formatFixedPoint(currentPrevisionItem->getPrevisionItemValue()));
    ui->tableViewCriteria->resizeColumnToContents(0);
    ui->tableViewRecentCriteria->resizeColumnToContents(0);


    ui->plot->clearGraphs();
    ui->plot->clearItems();
    ui->plot->clearPlottables();

    graphics->getPCAHistory(currentNodeId,currentMode,{PCx_Audit::ORED::OUVERTS,PCx_Audit::ORED::REALISES},currentPrevisionItem,true);


}


void FormAuditPrevisions::on_comboListPrevisions_activated(int index)
{
    if(index==-1||ui->comboListPrevisions->count()==0)return;
    unsigned int selectedPrevisionId=ui->comboListPrevisions->currentData().toUInt();
    if(!(selectedPrevisionId>0))
    {
        qFatal("Assertion failed");
    }
    //qDebug()<<"Selected audit ID = "<<selectedAuditId;

    if(previsionModel!=nullptr)
    {
        delete previsionModel;
        previsionModel=nullptr;
    }
    if(auditWithTreeModel!=nullptr)
    {
        delete auditWithTreeModel;
        auditWithTreeModel=nullptr;
    }

    if(recentPrevisionItemTableModel!=nullptr)
    {
        delete recentPrevisionItemTableModel;
        recentPrevisionItemTableModel=nullptr;
    }
    if(recentPrevisionItem!=nullptr)
    {
        delete recentPrevisionItem;
        recentPrevisionItem=nullptr;
    }

    if(graphics!=nullptr)
    {
        delete graphics;
        graphics=nullptr;
    }

    previsionModel=new PCx_Prevision(selectedPrevisionId);
    auditWithTreeModel=new PCx_AuditWithTreeModel(previsionModel->getAttachedAuditId());
    graphics=new PCx_Graphics(previsionModel->getAttachedAudit(),ui->plot);

    QItemSelectionModel *m=ui->treeView->selectionModel();
    ui->treeView->setModel(auditWithTreeModel->getAttachedTree());
    delete m;
    ui->treeView->expandToDepth(1);
    QModelIndex rootIndex=auditWithTreeModel->getAttachedTree()->index(0,0);
    ui->treeView->setCurrentIndex(rootIndex);
    on_treeView_clicked(rootIndex);
}

void FormAuditPrevisions::on_treeView_clicked(const QModelIndex &index)
{
    currentNodeId=index.data(PCx_TreeModel::NodeIdUserRole).toUInt();
    if(auditWithTreeModel->getAttachedTree()->isLeaf(currentNodeId))
    {
        ui->pushButtonApplyToLeaves->setText(tr("Appliquer à la feuille active"));
        ui->pushButtonApplyToNode->setEnabled(false);
        ui->comboBoxOREDDispatchRef->setEnabled(false);
    }
    else
    {
        ui->pushButtonApplyToLeaves->setText(tr("Appliquer aux feuilles liées"));
        ui->pushButtonApplyToNode->setEnabled(true);
        ui->comboBoxOREDDispatchRef->setEnabled(true);
    }
  //  qDebug()<<auditWithTreeModel->getAttachedTree()->getLeavesId(currentNodeId);
    updatePrevisionItemTableModel();
    updateLabels();
    if(ui->checkBoxDisplayLeafCriteria->isChecked())
    {
        ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());
    }
}


void FormAuditPrevisions::on_comboBoxOperators_activated(int index)
{
    Q_UNUSED(index);
    switch(ui->comboBoxOperators->currentData().toInt())
    {
        case PCx_PrevisionItemCriteria::PREVISIONOPERATOR::FIXEDVALUE:
        ui->comboBoxORED->setEnabled(false);
        ui->doubleSpinBox->setEnabled(true);
        ui->doubleSpinBox->setSuffix("€");
        break;
        case PCx_PrevisionItemCriteria::PREVISIONOPERATOR::PERCENT:
        ui->comboBoxORED->setEnabled(true);
        ui->doubleSpinBox->setEnabled(true);
        ui->doubleSpinBox->setSuffix("%");
        break;
    default:
        ui->comboBoxORED->setEnabled(true);
        ui->doubleSpinBox->setEnabled(false);
    }
}

void FormAuditPrevisions::onAnchorClicked(QUrl url)
{
    QStringList nodeString=url.toString().split("_");
    if(nodeString.count()!=2)
    {
        qWarning()<<"Error parsing anchor";
        return;
    }
    unsigned int nodeId=nodeString.at(1).toUInt();

    QModelIndexList indexOfNode=auditWithTreeModel->getAttachedTree()->getIndexOfNodeId(nodeId);
    if(indexOfNode.count()==1)
    {
        const QModelIndex &index=indexOfNode.at(0);
        ui->treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
        ui->treeView->scrollTo(index);
        on_treeView_clicked(index);
    }

}


void FormAuditPrevisions::on_pushButtonAddCriteriaToAdd_clicked()
{
    PCx_PrevisionItemCriteria::PREVISIONOPERATOR prevop=(PCx_PrevisionItemCriteria::PREVISIONOPERATOR)ui->comboBoxOperators->currentData().toUInt();
    PCx_Audit::ORED ored=(PCx_Audit::ORED)ui->comboBoxORED->currentData().toUInt();
    qint64 operand=0;
    if(ui->doubleSpinBox->isEnabled())
    {
        operand=NUMBERSFORMAT::doubleToFixedPoint(ui->doubleSpinBox->value());
    }
    PCx_PrevisionItemCriteria criteria(prevop,ored,operand);
    recentPrevisionItem->insertCriteriaToAdd(criteria);
    recentPrevisionItemTableModel->resetModel();
    updateLabels();

}

void FormAuditPrevisions::on_pushButtonAddCriteriaToSubstract_clicked()
{
    PCx_PrevisionItemCriteria::PREVISIONOPERATOR prevop=(PCx_PrevisionItemCriteria::PREVISIONOPERATOR)ui->comboBoxOperators->currentData().toUInt();
    PCx_Audit::ORED ored=(PCx_Audit::ORED)ui->comboBoxORED->currentData().toUInt();
    qint64 operand=0;
    if(ui->doubleSpinBox->isEnabled())
    {
        operand=NUMBERSFORMAT::doubleToFixedPoint(ui->doubleSpinBox->value());
    }
    PCx_PrevisionItemCriteria criteria(prevop,ored,operand);
    recentPrevisionItem->insertCriteriaToSub(criteria);
    recentPrevisionItemTableModel->resetModel();
    updateLabels();

}

void FormAuditPrevisions::on_pushButtonDelCriteria_clicked()
{
    QItemSelectionModel *selectionModel=ui->tableViewRecentCriteria->selectionModel();
    if(selectionModel->selectedRows().isEmpty())
        return;
    recentPrevisionItem->deleteCriteria(selectionModel->selectedRows());
    recentPrevisionItemTableModel->resetModel();
    updateLabels();

}

void FormAuditPrevisions::on_radioButtonDF_toggled(bool checked)
{
    if(checked)
    {
        currentMode=MODES::DFRFDIRI::DF;
        updatePrevisionItemTableModel();
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());

    }
}

void FormAuditPrevisions::on_radioButtonRF_toggled(bool checked)
{
    if(checked)
    {
        currentMode=MODES::DFRFDIRI::RF;
        updatePrevisionItemTableModel();
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());

    }
}

void FormAuditPrevisions::on_radioButtonDI_toggled(bool checked)
{
    if(checked)
    {
        currentMode=MODES::DFRFDIRI::DI;
        updatePrevisionItemTableModel();
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());

    }
}

void FormAuditPrevisions::on_radioButtonRI_toggled(bool checked)
{
    if(checked)
    {
        currentMode=MODES::DFRFDIRI::RI;
        updatePrevisionItemTableModel();
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());

    }
}

void FormAuditPrevisions::on_pushButtonDeleteAll_clicked()
{
    recentPrevisionItem->deleteAllCriteria();
    recentPrevisionItemTableModel->resetModel();
    updateLabels();
}

void FormAuditPrevisions::on_pushButtonApplyToNode_clicked()
{
    if(recentPrevisionItem!=nullptr && currentPrevisionItem!=nullptr)
    {
        PCx_Audit::ORED oredDispatchRef=(PCx_Audit::ORED)ui->comboBoxOREDDispatchRef->currentData().toUInt();

        if(recentPrevisionItem->dispatchComputedValueToChildrenLeaves(oredDispatchRef)==false)
        {
            return;
        }
        recentPrevisionItem->saveDataToDb();
        currentPrevisionItem->loadFromDb();
        currentPrevisionItemTableModel->resetModel();
        emit previsionUpdated(previsionModel->getPrevisionId());
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());

    }
}

void FormAuditPrevisions::on_pushButtonApplyToLeaves_clicked()
{
    if(recentPrevisionItem!=nullptr && currentPrevisionItem!=nullptr)
    {
        recentPrevisionItem->saveDataToDb();
        currentPrevisionItem->loadFromDb();
        currentPrevisionItem->dispatchCriteriaItemsToChildrenLeaves();
        currentPrevisionItemTableModel->resetModel();
        emit previsionUpdated(previsionModel->getPrevisionId());
        updateLabels();
        if(ui->textBrowser->isVisible())
            ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());
    }
}

void FormAuditPrevisions::on_checkBoxDisplayLeafCriteria_toggled(bool checked)
{
    if(checked)
    {
        ui->textBrowser->setHidden(false);
        ui->textBrowser->setHtml(currentPrevisionItem->getPrevisionItemsOfDescendantsAsHTML());
    }
    else
    {
        ui->textBrowser->clear();
        ui->textBrowser->setHidden(true);
    }
}

QSize FormAuditPrevisions::sizeHint() const
{
    return QSize(850,500);
}

void FormAuditPrevisions::on_pushButtonDisplayReport_clicked()
{
    FormDisplayPrevisionReport *form=new FormDisplayPrevisionReport(currentPrevisionItem,this);
    form->setAttribute(Qt::WA_DeleteOnClose);
    QMdiSubWindow *mdiSubWin=(QMdiSubWindow *)this->parentWidget();
    QMdiArea *mdiArea=mdiSubWin->mdiArea();
    QMdiSubWindow *subWin=mdiArea->addSubWindow(form);
    subWin->setWindowIcon(QIcon(":/icons/icons/editPrevisions.png"));
    form->show();
}

void FormAuditPrevisions::on_pushButtonSaveBigReport_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QString fileName = fileDialog.getSaveFileName(this, tr("Enregistrer en HTML"), "",tr("Fichiers HTML (*.html *.htm)"));
    if(fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if(fi.suffix().compare("html",Qt::CaseInsensitive)!=0 && fi.suffix().compare("htm",Qt::CaseInsensitive)!=0)
        fileName.append(".html");
    currentPrevisionItem->savePrevisionItemReport(fileName,true);
}


void FormAuditPrevisions::on_checkBoxShowCriteriaToApply_toggled(bool checked)
{
    ui->frame->setVisible(checked);
}

void FormAuditPrevisions::on_pushButtonExpandAll_clicked()
{
    ui->treeView->expandAll();
}

void FormAuditPrevisions::on_pushButtonCollapseAll_clicked()
{
    ui->treeView->collapseAll();
    ui->treeView->expandToDepth(0);
}
