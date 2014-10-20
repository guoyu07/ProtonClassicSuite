#include "formdisplaytree.h"
#include "formedittree.h"
#include "ui_formedittree.h"
#include "utils.h"
#include "pcx_tree.h"
#include "formtreeconsistency.h"
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>
#include <QInputDialog>
#include <QMdiSubWindow>
#include <QStandardPaths>
#include <QFileDialog>

FormEditTree::FormEditTree(QWidget *parent) : QWidget(parent), ui(new Ui::FormEditTree)
{
    ui->setupUi(this);
    model=nullptr;
    updateListOfTree();
}

FormEditTree::~FormEditTree()
{
    if(model!=nullptr)delete model;
    delete ui;
}


void FormEditTree::on_addTypeButton_clicked()
{
    if(model!=nullptr)
    {
        bool ok;
        QString text;

        do
        {
            text=QInputDialog::getText(this,tr("Nouveau type"), tr("Nom du type à ajouter : "),QLineEdit::Normal,"",&ok).simplified();

        }while(ok && text.isEmpty());

        if(ok)
        {
            model->addType(text);
        }
    }
}

void FormEditTree::updateListOfTree()
{
    ui->comboBox->clear();

    QList<QPair<unsigned int,QString> > lot=PCx_Tree::getListOfTrees();
    bool nonEmpty=!lot.isEmpty();
    setReadOnly(!nonEmpty);
    ui->deleteTreeButton->setEnabled(nonEmpty);
    ui->duplicateTreeButton->setEnabled(nonEmpty);
    ui->viewTreeButton->setEnabled(nonEmpty);
    QPair<unsigned int, QString> p;
    foreach(p,lot)
    {
        ui->comboBox->insertItem(0,p.second,p.first);
    }

    if(ui->comboBox->count()==0)
    {
        on_newTreeButton_clicked();
    }
    else
    {
        ui->comboBox->setCurrentIndex(0);
        on_comboBox_activated(0);
    }
}

void FormEditTree::setReadOnly(bool state)
{
    ui->deleteNodeButton->setDisabled(state);
    ui->addNodeButton->setDisabled(state);
    ui->addTypeButton->setDisabled(state);
    ui->deleteTypeButton->setDisabled(state);
    ui->finishTreeButton->setDisabled(state);
    ui->modifyNodeButton->setDisabled(state);
    ui->treeView->setDragEnabled(!state);
    ui->listTypesView->setDisabled(state);
}

void FormEditTree::on_deleteTreeButton_clicked()
{
    if(model!=nullptr)
    {
        if(QMessageBox::question(this,tr("Attention"),tr("Vous allez supprimer cet arbre. Cette action ne peut pas être annulée. En êtes-vous sûr ?"))==QMessageBox::No)
        {
            return;
        }

        unsigned int treeId=model->getTreeId();
        //qDebug()<<"Deleting tree "<<treeId;

        int result=PCx_Tree::deleteTree(treeId);

        if(result==1)
        {
            ui->listTypesView->setModel(nullptr);
            model->getTypesTableModel()->clear();
            model->clear();
            delete model;
            model=nullptr;

            updateListOfTree();

            emit(listOfTreeChanged());
        }
        else if(result==0)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Il existe des audits liés à cet arbre. Supprimez-les d'abord"));
            return;
        }
    }
}

void FormEditTree::onTypesChanged()
{
    if(model->updateTree()==false)
        die();
    ui->treeView->expandToDepth(1);
}

void FormEditTree::on_newTreeButton_clicked()
{
    bool ok;
    QString text;

    do
    {
        text=QInputDialog::getText(this->parentWidget(),tr("Nouvel arbre"), tr("Nom du nouvel arbre : "),QLineEdit::Normal,"",&ok).simplified();

    }while(ok && text.isEmpty());

    if(ok)
    {
        QSqlDatabase::database().transaction();
        if(PCx_Tree::addTree(text)<=0)
        {
            QSqlDatabase::database().rollback();
            return;
        }
        QSqlDatabase::database().commit();
        updateListOfTree();
        emit(listOfTreeChanged());
    }
}

void FormEditTree::on_deleteTypeButton_clicked()
{
    if(model!=nullptr)
    {
        if(ui->listTypesView->model()->rowCount()==1)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Les noeuds doivent tous être typés"));
            return;
        }
        QModelIndex index=ui->listTypesView->currentIndex();
        //qDebug()<<"Selected index = "<<index;
        if(index.row()>-1)
        {
            model->deleteType(model->getTypesTableModel()->record(index.row()).field("id").value().toInt());
        }
    }
}

void FormEditTree::on_addNodeButton_clicked()
{
    if(model==nullptr)return;
    QModelIndexList selection=ui->treeView->selectionModel()->selectedIndexes();

    if(!selection.isEmpty())
    {
        QModelIndex indexType=ui->listTypesView->currentIndex();
        if(indexType.row()<0)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Sélectionnez le type de l'élément à ajouter dans la zone de droite"));
            return;
        }

        unsigned int selectedTypeId=model->getTypesTableModel()->record(indexType.row()).field("id").value().toUInt();

        //qDebug()<<"Selected type ID : "<<selectedTypeId;

        bool ok;
        QString text;

        do
        {
            text=QInputDialog::getText(this,tr("Nouveau noeud"), tr("Donnez un nom au nouveau noeud, son type sera <b>%1</b> : ").arg(indexType.data().toString()),QLineEdit::Normal,"",&ok).simplified();

        }while(ok && text.isEmpty());

        if(ok)
        {
            model->addNode(selectedTypeId,text,selection[0]);
        }

    }
    else
    {
        QMessageBox::warning(this,tr("Attention"),tr("Sélectionnez d'abord le noeud père dans l'arbre"));
        return;
    }
}

void FormEditTree::on_modifyNodeButton_clicked()
{
    if(model==nullptr)return;

    QModelIndexList selection=ui->treeView->selectionModel()->selectedIndexes();

    if(!selection.isEmpty())
    {
        unsigned int selectedId=selection[0].data(PCx_TreeModel::NodeIdUserRole).toUInt();
        //qDebug()<<"Selected Node ID : "<<selectedId;

        //No modification for the root
        if(selectedId==1)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Vous ne pouvez pas modifier la racine de l'arbre"));
            return;
        }

        QModelIndex indexType=ui->listTypesView->currentIndex();
        if(indexType.row()<0)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Sélectionnez le nouveau type du noeud à modifier dans la zone de droite"));
            return;
        }

        unsigned int selectedTypeId=model->getTypesTableModel()->record(indexType.row()).field("id").value().toUInt();

        bool ok;
        QString text;

        do
        {
            text=QInputDialog::getText(this,tr("Modifier noeud"), tr("Nouveau nom du noeud, son type sera <b>%1</b> : ").arg(indexType.data().toString()),QLineEdit::Normal,"",&ok).simplified();

        }while(ok && text.isEmpty());

        if(ok)
        {
            model->updateNode(selection[0],text,selectedTypeId);
        }
    }
    else
    {
        QMessageBox::warning(this,tr("Attention"),tr("Sélectionnez le noeud à modifier"));
        return;
    }
}

void FormEditTree::on_deleteNodeButton_clicked()
{
    if(model==nullptr)return;


    QModelIndexList selection=ui->treeView->selectionModel()->selectedIndexes();

    if(!selection.isEmpty())
    {
        //Assume only single selection
        unsigned int selectedId=selection[0].data(PCx_TreeModel::NodeIdUserRole).toUInt();
        //qDebug()<<"Selected Node ID : "<<selectedId;

        //No modification for the root
        if(selectedId==1)
        {
            QMessageBox::warning(this,tr("Attention"),tr("Vous ne pouvez pas supprimer la racine de l'arbre"));
            return;
        }

        if(QMessageBox::question(this,tr("Attention"),tr("Voulez-vous vraiment supprimer le noeud <b>%1</b> ainsi que tous ses descendants ?").arg(selection[0].data().toString()))==QMessageBox::No)
        {
            return;
        }
        model->deleteNode(selection[0]);
    }
}

void FormEditTree::on_finishTreeButton_clicked()
{
    if(model!=nullptr)
    {
        if(QMessageBox::question(this,tr("Attention"),tr("Voulez-vous vraiment terminer cet arbre ? Une fois terminé, vous ne pourrez plus le modifier."))==QMessageBox::No)
        {
            return;
        }
        if(model->finishTree()==false)
        {
            die();
        }
        emit(listOfTreeChanged());
        updateListOfTree();
    }
}

void FormEditTree::on_viewTreeButton_clicked()
{
    FormDisplayTree *ddt=new FormDisplayTree(model->getTreeId(),this);
    ddt->setAttribute(Qt::WA_DeleteOnClose);
    QMdiSubWindow *mdiSubWin=(QMdiSubWindow *)this->parentWidget();
    QMdiArea *mdiArea=mdiSubWin->mdiArea();
    mdiArea->addSubWindow(ddt);
    ddt->show();
}

void FormEditTree::on_comboBox_activated(int index)
{
    if(index==-1)return;
    if(model!=nullptr)
    {
        QItemSelectionModel *m=ui->listTypesView->selectionModel();
        ui->listTypesView->setModel(nullptr);
        delete m;
        delete model;
    }

    //Read-write types
    model=new PCx_TreeModel(ui->comboBox->currentData().toUInt());

    QItemSelectionModel *m=ui->treeView->selectionModel();
    ui->treeView->setModel(model);
    delete m;
    ui->treeView->expandToDepth(1);

    m=ui->listTypesView->selectionModel();
    Q_ASSERT(model->getTypes()->getTableModel()!=nullptr);
    ui->listTypesView->setModel(model->getTypesTableModel());
    ui->listTypesView->setModelColumn(1);
    delete m;

    connect(model,SIGNAL(typesUpdated()),this,SLOT(onTypesChanged()));
    setReadOnly(model->isFinished());
}

void FormEditTree::on_treeView_activated(const QModelIndex &index)
{
    Q_UNUSED(index);
    //Only edit unfinished trees
    if(!model->isFinished())
        on_modifyNodeButton_clicked();
}

void FormEditTree::on_duplicateTreeButton_clicked()
{
    if(model!=nullptr)
    {
        bool ok;
        QString text;

        do
        {
            text=QInputDialog::getText(this,tr("Dupliquer arbre"), tr("Nom de l'arbre dupliqué : "),QLineEdit::Normal,"",&ok).simplified();

        }while(ok && text.isEmpty());

        if(ok)
        {
            model->duplicateTree(text);
            updateListOfTree();
        }
    }
}

void FormEditTree::on_randomTreeButton_clicked()
{
    bool ok;
    QString text;
    unsigned int nbNodes;

    do
    {
        text=QInputDialog::getText(this,tr("Nouvel arbre aléatoire"), tr("Nom de l'arbre aléatoire : "),QLineEdit::Normal,"",&ok).simplified();

    }while(ok && text.isEmpty());

    if(!ok)
        return;

    nbNodes=QInputDialog::getInt(this,tr("Nouvel arbre aléatoire"),tr("Nombre de noeuds"),20,2,PCx_Tree::MAXNODES,1,&ok);

    if(ok)
    {
        PCx_Tree::createRandomTree(text,nbNodes);
        updateListOfTree();
    }
}

void FormEditTree::on_pushButtonExpandAll_clicked()
{
    ui->treeView->expandAll();
}

void FormEditTree::on_pushButtonCollapseAll_clicked()
{
    ui->treeView->collapseAll();
    ui->treeView->expandToDepth(0);
}



void FormEditTree::on_importTreeButton_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QString fileName = fileDialog.getOpenFileName(this, tr("Ouvrir un arbre"), "",tr("Arbres au format XLSX (*.xlsx)"));

    if(fileName.isEmpty())
        return;

    bool ok;
    QString text;

    do
    {
        text=QInputDialog::getText(this,tr("Importer arbre"), tr("Donnez un nom à cet arbre : "),QLineEdit::Normal,"",&ok).simplified();
    }while(ok && text.isEmpty());

    if(!ok)
        return;

    if(PCx_Tree::treeNameExists(text))
    {
        QMessageBox::warning(this,tr("Attention"),tr("Il existe déjà un arbre portant ce nom !"));
        return;
    }

    if(PCx_Tree::importTreeFromXLSX(fileName,text)>0)
    {
        updateListOfTree();
        emit(listOfTreeChanged());
    }
}

void FormEditTree::on_consistencyButton_clicked()
{
    FormTreeConsistency *form=new FormTreeConsistency(model->getTreeId(),this);
    form->setAttribute(Qt::WA_DeleteOnClose);
    QMdiSubWindow *mdiSubWin=(QMdiSubWindow *)this->parentWidget();
    QMdiArea *mdiArea=mdiSubWin->mdiArea();
    mdiArea->addSubWindow(form);
    form->show();
}

QSize FormEditTree::sizeHint() const
{
    return QSize(600,450);
}
