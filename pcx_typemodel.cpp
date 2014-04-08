#include "pcx_typemodel.h"
#include "utility.h"
#include <QtGui>
#include <QtSql>
#include <QMessageBox>

PCx_TypeModel::PCx_TypeModel(unsigned int treeId, QObject *parent):QObject(parent)
{
    this->treeId=treeId;
    loadFromDatabase(treeId);
    loadSqlTableModel();
    loadSqlQueryModel();
    qDebug()<<"Loaded PCx_TypeModel with "<<typesTableModel->rowCount()<<" rows";
}

void PCx_TypeModel::loadSqlQueryModel()
{
    Q_ASSERT(treeId>0);
    typesQueryModel=new QSqlQueryModel();
    typesQueryModel->setQuery(QString("select * from types_%1").arg(treeId));
}

bool PCx_TypeModel::loadSqlTableModel()
{
    Q_ASSERT(treeId>0);
    typesTableModel=new QSqlTableModel();
    typesTableModel->setTable(QString("types_%1").arg(treeId));
    typesTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    typesTableModel->select();
    connect(typesTableModel,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(onTypesModelDataChanged(const QModelIndex &, const QModelIndex &)));
    return true;
}

QStringList PCx_TypeModel::getListOfDefaultTypes()
{
    QStringList listOfTypes;
    listOfTypes<<tr("Maire adjoint")<<tr("Conseiller")<<tr("Division")<<tr("Service");
    return listOfTypes;

}

/*Check if the new type does not already exists in the table
 * Assumes that only one row is modified at once
 */
bool PCx_TypeModel::onTypesModelDataChanged(const QModelIndex &topLeft, const QModelIndex & bottomRight)
{
    bool oldState;
    QString newType=topLeft.data().toString();
    if(validateType(newType)==false)
    {
        //In order to prevent reentrant "datachanged" signal with revertAll
        oldState=typesTableModel->blockSignals(true);
        typesTableModel->revertAll();
        typesTableModel->blockSignals(oldState);
        return false;
    }

    else
    {
        typesTableModel->submitAll();
        loadFromDatabase(treeId);
        emit typesUpdated();
        return true;
    }
}

bool PCx_TypeModel::validateType(const QString &newType)
{
    if(newType.isEmpty() || newType.contains(QRegExp("( )+")))
    {
        QMessageBox::warning(NULL,tr("Attention"),tr("Vous ne pouvez pas utiliser un type vide !"));
        return false;
    }
    else if(nomTypes.contains(newType,Qt::CaseInsensitive))
    {
        QMessageBox::warning(NULL,tr("Attention"),tr("Le type <b>%1</b> existe déjà !").arg(newType));
        return false;
    }
    return true;
}

PCx_TypeModel::~PCx_TypeModel()
{
    typesTableModel->clear();
    delete typesTableModel;
}

bool PCx_TypeModel::addType(const QString &type)
{
    if(validateType(type)==false)
        return false;

    else
    {
        QSqlQuery q;
        q.prepare(QString("insert into types_%1 (nom) values(:nom)").arg(treeId));
        q.bindValue(":nom",type);
        q.exec();
        if(q.numRowsAffected()!=1)
        {
            qCritical()<<q.lastError().text();
            die();
        }

        loadFromDatabase(treeId);
        typesTableModel->select();
    }
    return true;
}

bool PCx_TypeModel::deleteType(const QString &type)
{
    if(type.isNull() || type.isEmpty())return false;
    QSqlQuery query;
    int typeId=-1;

    query.prepare(QString("select id from types_%1 where nom=:nom limit 1").arg(treeId));
    query.bindValue(":nom",type);
    query.exec();

    if(!query.isActive())
    {
        qCritical()<<query.lastError();
        die();
    }

    if(query.next())
    {
        typeId=query.value(0).toInt();
        qDebug()<<"typeId = "<<typeId<<"Text = "<<type;
    }
    else return false;

    return deleteType(typeId);
}

bool PCx_TypeModel::deleteType(unsigned int id)
{
    QSqlQuery query;

    query.prepare(QString("select count(*) from arbre_%1 where type=:type").arg(treeId));
    query.bindValue(":type",id);
    query.exec();

    if(!query.isActive())
    {
        qCritical()<<query.lastError();
        die();
    }

    if(query.next())
    {
        qDebug()<<"Nombre de noeuds de type "<<id<<" = "<<query.value(0).toInt();
        if(query.value(0).toInt()>0)
        {
            QMessageBox::warning(NULL,tr("Attention"),tr("Il existe des noeuds de ce type dans l'arbre. Supprimez-les d'abord."));
            return false;
        }

    }
    else return false;

    query.prepare(QString("delete from types_%1 where id=:id").arg(treeId));
    query.bindValue(":id",id);
    query.exec();

    if(query.numRowsAffected()!=1)
    {
        qCritical()<<query.lastError().text();
        die();
    }
    loadFromDatabase(treeId);
    typesTableModel->select();
    return true;
}

bool PCx_TypeModel::loadFromDatabase(unsigned int treeId)
{
    Q_ASSERT(treeId>0);
    idTypesToNom.clear();
    nomTypes.clear();
    QSqlQuery query(QString("select * from types_%1").arg(treeId));
    if(!query.isActive())
    {
        qCritical()<<query.lastError();
        die();
    }

    while(query.next())
    {
        idTypesToNom.insert(query.value(0).toInt(),query.value(1).toString());
        nomTypes.append(query.value(1).toString());
    }
    return true;
}