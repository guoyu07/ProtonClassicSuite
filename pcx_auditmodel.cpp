#include "pcx_auditmodel.h"
#include "utils.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

PCx_AuditModel::PCx_AuditModel(unsigned int auditId, QObject *parent, bool readOnly) :
    QObject(parent)
{
    attachedTree=NULL;
    modelDF=NULL;
    modelDI=NULL;
    modelRI=NULL;
    modelRF=NULL;
    this->auditId=auditId;
    loadFromDb(auditId,readOnly);
}

PCx_AuditModel::~PCx_AuditModel()
{
    if(modelDF!=NULL)
    {
        modelDF->clear();
        delete modelDF;
    }
    if(modelRF!=NULL)
    {
        modelRF->clear();
        delete modelRF;
    }
    if(modelDI!=NULL)
    {
        modelDI->clear();
        delete modelDI;
    }
    if(modelRI!=NULL)
    {
        modelRI->clear();
        delete modelRI;
    }
    delete attachedTree;
}

bool PCx_AuditModel::addNewAudit(const QString &name, QList<unsigned int> years, unsigned int attachedTreeId)
{
    Q_ASSERT(!years.isEmpty() && !name.isEmpty() && attachedTreeId>0);

    //Removes duplicates and sort years
    QSet<unsigned int> yearsSet=years.toSet();
    QList<unsigned int> yearsList=yearsSet.toList();
    qSort(yearsList);

    QString yearsString;
    foreach(unsigned int annee,yearsList)
    {
        yearsString.append(QString::number(annee));
        yearsString.append(',');
    }
    yearsString.chop(1);

    qDebug()<<"years string = "<<yearsString;

    QSqlQuery q;
    q.prepare("select count(*) from index_audits where nom=:nom");
    q.bindValue(":nom",name);
    q.exec();

    if(q.next())
    {
        if(q.value(0).toInt()>0)
        {
            QMessageBox::warning(NULL,QObject::tr("Attention"),QObject::tr("Il existe déjà un audit portant ce nom !"));
            return false;
        }
    }
    else
    {
        qCritical()<<q.lastError().text();
        die();
    }

    q.exec(QString("select count(*) from index_arbres where id='%1'").arg(attachedTreeId));
    if(q.next())
    {
        if(q.value(0).toInt()==0)
        {
            qCritical()<<"Ajout audit sur arbre inexistant !";
            die();
        }
    }
    else
    {
        qCritical()<<q.lastError().text();
        die();
    }

    QSqlDatabase::database().transaction();
    q.prepare("insert into index_audits (nom,id_arbre,annees) values (:nom,:id_arbre,:annees)");
    q.bindValue(":nom",name);
    q.bindValue(":id_arbre",attachedTreeId);
    q.bindValue(":annees",yearsString);
    q.exec();

    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }

    QVariant lastId=q.lastInsertId();
    if(!lastId.isValid())
    {
        qCritical()<<"Problème d'id, vérifiez la consistance de la base";
        QSqlDatabase::database().rollback();
        die();
    }
    unsigned int uLastId=lastId.toUInt();

    q.exec(QString("create table audit_DF_%1(id integer primary key autoincrement, id_node integer not null, annee integer not null, ouverts real, realises real, engages real, disponibles real)").arg(uLastId));

    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        //q.exec(QString("delete from index_audits where id=%1").arg(uLastId));
        die();
    }

    q.exec(QString("create table audit_RF_%1(id integer primary key autoincrement, id_node integer not null, annee integer not null, ouverts real, realises real, engages real, disponibles real)").arg(uLastId));

    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        //q.exec(QString("delete from index_audits where id=%1").arg(uLastId));
        //q.exec(QString("drop table audit_DF_%1").arg(uLastId));
        die();
    }

    q.exec(QString("create table audit_DI_%1(id integer primary key autoincrement, id_node integer not null, annee integer not null, ouverts real, realises real, engages real, disponibles real)").arg(uLastId));

    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        //q.exec(QString("delete from index_audits where id=%1").arg(uLastId));
        //q.exec(QString("drop table audit_DF_%1").arg(uLastId));
        //q.exec(QString("drop table audit_RF_%1").arg(uLastId));
        die();
    }

    q.exec(QString("create table audit_RI_%1(id integer primary key autoincrement, id_node integer not null, annee integer not null, ouverts real, realises real, engages real, disponibles real)").arg(uLastId));

    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        //q.exec(QString("delete from index_audits where id=%1").arg(uLastId));
        //q.exec(QString("drop table audit_DF_%1").arg(uLastId));
        //q.exec(QString("drop table audit_RF_%1").arg(uLastId));
        //q.exec(QString("drop table audit_DI_%1").arg(uLastId));
        die();
    }

    //Populate tables with years for each node of the attached tree

    QList<unsigned int> nodes=PCx_TreeModel::getNodesId(attachedTreeId);
    qDebug()<<"Nodes ids = "<<nodes;

    foreach(unsigned int node,nodes)
    {
        foreach(unsigned int annee,yearsList)
        {
            q.prepare(QString("insert into audit_DF_%1(id_node,annee) values (:idnode,:annee)").arg(uLastId));
            q.bindValue(":idnode",node);
            q.bindValue(":annee",annee);
            q.exec();
            if(q.numRowsAffected()==-1)
            {
                qCritical()<<q.lastError().text();
                QSqlDatabase::database().rollback();
                die();
            }

            q.prepare(QString("insert into audit_RF_%1(id_node,annee) values (:idnode,:annee)").arg(uLastId));
            q.bindValue(":idnode",node);
            q.bindValue(":annee",annee);
            q.exec();
            if(q.numRowsAffected()==-1)
            {
                qCritical()<<q.lastError().text();
                QSqlDatabase::database().rollback();
                die();
            }

            q.prepare(QString("insert into audit_DI_%1(id_node,annee) values (:idnode,:annee)").arg(uLastId));
            q.bindValue(":idnode",node);
            q.bindValue(":annee",annee);
            q.exec();
            if(q.numRowsAffected()==-1)
            {
                qCritical()<<q.lastError().text();
                QSqlDatabase::database().rollback();
                die();
            }

            q.prepare(QString("insert into audit_RI_%1(id_node,annee) values (:idnode,:annee)").arg(uLastId));
            q.bindValue(":idnode",node);
            q.bindValue(":annee",annee);
            q.exec();
            if(q.numRowsAffected()==-1)
            {
                qCritical()<<q.lastError().text();
                QSqlDatabase::database().rollback();
                die();
            }
        }
    }
    QSqlDatabase::database().commit();

    return true;
}

bool PCx_AuditModel::deleteAudit(unsigned int auditId)
{
    Q_ASSERT(auditId>0);
    QSqlQuery q(QString("select count(*) from index_audits where id='%1'").arg(auditId));
    q.exec();
    if(q.next())
    {
        if(q.value(0).toInt()==0)
        {
            qCritical()<<"Audit inexistant !";
            return false;
        }
    }
    else
    {
        qCritical()<<q.lastError().text();
        die();
    }

    QSqlDatabase::database().transaction();
    q.exec(QString("delete from index_audits where id='%1'").arg(auditId));
    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }
    q.exec(QString("drop table audit_DF_%1").arg(auditId));
    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }
    q.exec(QString("drop table audit_RF_%1").arg(auditId));
    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }
    q.exec(QString("drop table audit_DI_%1").arg(auditId));
    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }
    q.exec(QString("drop table audit_RI_%1").arg(auditId));
    if(q.numRowsAffected()==-1)
    {
        qCritical()<<q.lastError().text();
        QSqlDatabase::database().rollback();
        die();
    }
    QSqlDatabase::database().commit();
    qDebug()<<"Audit "<<auditId<<" deleted";
    return true;
}



bool PCx_AuditModel::finishAudit()
{
    auditInfos.finished=true;
    return PCx_AuditModel::finishAudit(auditId);
}

QSqlTableModel *PCx_AuditModel::getTableModel(const QString &mode) const
{
    if(0==mode.compare("DF",Qt::CaseInsensitive))
    {
        return modelDF;
    }
    if(0==mode.compare("RF",Qt::CaseInsensitive))
    {
        return modelRF;
    }
    if(0==mode.compare("DI",Qt::CaseInsensitive))
    {
        return modelDI;
    }
    if(0==mode.compare("RI",Qt::CaseInsensitive))
    {
        return modelRI;
    }
    return NULL;
}

QSqlTableModel *PCx_AuditModel::getTableModel(DFRFDIRI mode) const
{
    switch(mode)
    {
    case DF:return modelDF;
    case RF:return modelRF;
    case DI:return modelDI;
    case RI:return modelRI;
    case GLOBAL:return NULL;
    }
    return NULL;
}

bool PCx_AuditModel::loadFromDb(unsigned int auditId,bool readOnly)
{
    Q_ASSERT(auditId>0);
    QSqlQuery q;
    auditInfos.updateInfos(auditId);
    if(auditInfos.valid)
    {
        unsigned int attachedTreeId=auditInfos.attachedTreeId;
        qDebug()<<"Attached tree ID = "<<attachedTreeId;

        if(attachedTree!=NULL)
        {
            delete attachedTree;
        }
        attachedTree=new PCx_TreeModel(attachedTreeId);
        if(readOnly==false)
        {
            if(modelDF!=NULL)
            {
                modelDF->clear();
                delete modelDF;
            }
            if(modelRF!=NULL)
            {
                modelRF->clear();
                delete modelRF;
            }
            if(modelDI!=NULL)
            {
                modelDI->clear();
                delete modelDI;
            }
            if(modelRI!=NULL)
            {
                modelRI->clear();
                delete modelRI;
            }
            modelDF=new QSqlTableModel();
            modelDF->setTable(QString("audit_DF_%1").arg(auditId));
            modelDF->setHeaderData(COL_ANNEE,Qt::Horizontal,"");
            modelDF->setHeaderData(COL_OUVERTS,Qt::Horizontal,tr("Ouverts"));
            modelDF->setHeaderData(COL_REALISES,Qt::Horizontal,tr("Réalisés"));
            modelDF->setHeaderData(COL_ENGAGES,Qt::Horizontal,tr("Engagés"));
            modelDF->setHeaderData(COL_DISPONIBLES,Qt::Horizontal,tr("Disponibles"));
            modelDF->setEditStrategy(QSqlTableModel::OnFieldChange);
            modelDF->select();

            modelDI=new QSqlTableModel();
            modelDI->setTable(QString("audit_DI_%1").arg(auditId));
            modelDI->setHeaderData(COL_ANNEE,Qt::Horizontal,"");
            modelDI->setHeaderData(COL_OUVERTS,Qt::Horizontal,tr("Ouverts"));
            modelDI->setHeaderData(COL_REALISES,Qt::Horizontal,tr("Réalisés"));
            modelDI->setHeaderData(COL_ENGAGES,Qt::Horizontal,tr("Engagés"));
            modelDI->setHeaderData(COL_DISPONIBLES,Qt::Horizontal,tr("Disponibles"));
            modelDI->setEditStrategy(QSqlTableModel::OnFieldChange);
            modelDI->select();

            modelRI=new QSqlTableModel();
            modelRI->setTable(QString("audit_RI_%1").arg(auditId));
            modelRI->setHeaderData(COL_ANNEE,Qt::Horizontal,"");
            modelRI->setHeaderData(COL_OUVERTS,Qt::Horizontal,tr("Ouverts"));
            modelRI->setHeaderData(COL_REALISES,Qt::Horizontal,tr("Réalisés"));
            modelRI->setHeaderData(COL_ENGAGES,Qt::Horizontal,tr("Engagés"));
            modelRI->setHeaderData(COL_DISPONIBLES,Qt::Horizontal,tr("Disponibles"));
            modelRI->setEditStrategy(QSqlTableModel::OnFieldChange);
            modelRI->select();

            modelRF=new QSqlTableModel();
            modelRF->setTable(QString("audit_RF_%1").arg(auditId));
            modelRF->setHeaderData(COL_ANNEE,Qt::Horizontal,"");
            modelRF->setHeaderData(COL_OUVERTS,Qt::Horizontal,tr("Ouverts"));
            modelRF->setHeaderData(COL_REALISES,Qt::Horizontal,tr("Réalisés"));
            modelRF->setHeaderData(COL_ENGAGES,Qt::Horizontal,tr("Engagés"));
            modelRF->setHeaderData(COL_DISPONIBLES,Qt::Horizontal,tr("Disponibles"));
            modelRF->setEditStrategy(QSqlTableModel::OnFieldChange);
            modelRF->select();

            connect(modelDF,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(onModelDataChanged(const QModelIndex &, const QModelIndex &)));
            connect(modelRF,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(onModelDataChanged(const QModelIndex &, const QModelIndex &)));
            connect(modelDI,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(onModelDataChanged(const QModelIndex &, const QModelIndex &)));
            connect(modelRI,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(onModelDataChanged(const QModelIndex &, const QModelIndex &)));
        }
    }
    else
    {
        qCritical()<<"Invalid audit ID";
        die();
    }
    return true;
}

bool PCx_AuditModel::propagateToAncestors(const QModelIndex &node)
{
    QSqlTableModel *model=(QSqlTableModel *)node.model();
    int row=node.row();
    unsigned int nodeId=model->index(row,COL_IDNODE).data().toUInt();
    unsigned int annee=model->index(row,COL_ANNEE).data().toUInt();
    QString tableName=model->tableName();
    qDebug()<<"Propagate from node "<<nodeId<<" in "<<annee<<" on "<<tableName;

    QSqlDatabase::database().transaction();
    if(updateParent(tableName,annee,nodeId))
    {
        QSqlDatabase::database().commit();
        return true;
    }
    else
    {
        qCritical()<<"ERROR DURING PROPAGATING VALUES TO ROOTS, CANCELLING";
        QSqlDatabase::database().rollback();
        return false;
    }

}

bool PCx_AuditModel::updateParent(const QString &tableName, unsigned int annee, unsigned int nodeId)
{
    unsigned int parent=attachedTree->getParentId(nodeId);
    qDebug()<<"Parent of "<<nodeId<<" = "<<parent;
    QList<unsigned int> listOfChildren=attachedTree->getChildren(parent);
    qDebug()<<"Children of "<<nodeId<<" = "<<listOfChildren;
    QSqlQuery q;
    QStringList l;
    foreach (unsigned int childId, listOfChildren)
    {
        l.append(QString::number(childId));
    }

    q.exec(QString("select * from %1 where id_node in(%2) and annee=%3").arg(tableName).arg(l.join(',')).arg(annee));
    if(!q.isActive())
    {
        qCritical()<<q.lastError().text();
        die();
    }

    double sumOuverts=0.0;
    double sumRealises=0.0;
    double sumEngages=0.0;
    double sumDisponibles=0.0;

    //To check if we need to insert 0.0 or NULL
    bool nullOuverts=true;
    bool nullEngages=true;
    bool nullRealises=true;
    bool nullDisponibles=true;


    while(q.next())
    {
        qDebug()<<"Node ID = "<<q.value("id_node")<< "Ouverts = "<<q.value("ouverts")<<" Realises = "<<q.value("realises")<<" Engages = "<<q.value("engages")<<" Disponibles = "<<q.value("disponibles");
        if(!q.value("ouverts").isNull())
        {
            sumOuverts+=q.value("ouverts").toDouble();
            nullOuverts=false;
        }

        if(!q.value("realises").isNull())
        {
            sumRealises+=q.value("realises").toDouble();
            nullRealises=false;
        }
        if(!q.value("engages").isNull())
        {
            sumEngages+=q.value("engages").toDouble();
            nullEngages=false;
        }
        if(!q.value("disponibles").isNull())
        {
            sumDisponibles+=q.value("disponibles").toDouble();
            nullDisponibles=false;
        }
    }

    q.prepare(QString("update %1 set ouverts=:ouverts,realises=:realises,engages=:engages,disponibles=:disponibles where annee=:annee and id_node=:id_node").arg(tableName));
    if(nullOuverts)
        q.bindValue(":ouverts",QVariant(QVariant::Double));
    else
        q.bindValue(":ouverts",sumOuverts);

    if(nullRealises)
        q.bindValue(":realises",QVariant(QVariant::Double));
    else
        q.bindValue(":realises",sumRealises);

    if(nullEngages)
        q.bindValue(":engages",QVariant(QVariant::Double));
    else
        q.bindValue(":engages",sumEngages);

    if(nullDisponibles)
        q.bindValue(":disponibles",QVariant(QVariant::Double));
    else
        q.bindValue(":disponibles",sumDisponibles);

    q.bindValue(":annee",annee);
    q.bindValue(":id_node",parent);
    q.exec();
    if(q.numRowsAffected()<=0)
    {
        qCritical()<<q.lastError().text();
        return false;
    }

    if(parent>1)
    {
        updateParent(tableName,annee,parent);
    }
    return true;
}

QString PCx_AuditModel::modetoTableString(DFRFDIRI mode) const
{
    switch(mode)
    {
    case DF:
        return "DF";
    case RF:
        return "RF";
    case DI:
        return "DI";
    case RI:
        return "RI";
    default:
        qCritical()<<"Invalid mode specified !";
    }
    return QString();
}



QString PCx_AuditModel::modeToCompleteString(DFRFDIRI mode) const
{
    switch(mode)
    {
    case DF:
        return tr("Dépenses de fonctionnement");
    case RF:
        return tr("Recettes de fonctionnement");
    case DI:
        return tr("Dépenses d'investissement");
    case RI:
        return tr("Recettes d'investissement");
    default:
        qCritical()<<"Invalid mode specified !";
    }
    return QString();
}

QString PCx_AuditModel::OREDtoTableString(ORED ored) const
{
    switch(ored)
    {
    case ouverts:
        return "ouverts";
    case realises:
        return "realises";
    case engages:
        return "engages";
    case disponibles:
        return "disponibles";
    default:
        qCritical()<<"Invalid ORED specified !";
    }
    return QString();
}

QString PCx_AuditModel::OREDtoCompleteString(ORED ored) const
{
    switch(ored)
    {
    case ouverts:
        return tr("prévu");
    case realises:
        return tr("réalisé");
    case engages:
        return tr("engagé");
    case disponibles:
        return tr("disponible");
    default:
        qCritical()<<"Invalid ORED specified !";
    }
    return QString();
}

//Warning, be called twice (see isDirty)
void PCx_AuditModel::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex & bottomRight)
{
    Q_UNUSED(bottomRight);
    QSqlTableModel *model=(QSqlTableModel *)topLeft.model();

    qDebug()<<"Audit Data changed for model "<<model->tableName()<<": topleft column = "<<topLeft.column()<<" topleft row = "<<topLeft.row()<<"bottomRight column = "<<bottomRight.column()<<" bottomRight row = "<<bottomRight.row();
    qDebug()<<"Model dirty : "<<model->isDirty();
    int row=topLeft.row();

    QVariant vOuverts=model->index(row,COL_OUVERTS).data();
    QVariant vRealises=model->index(row,COL_REALISES).data();
    QVariant vEngages=model->index(row,COL_ENGAGES).data();

    double ouverts=vOuverts.toDouble();
    double realises=vRealises.toDouble();
    double engages=vEngages.toDouble();

    if(!vRealises.isNull() && !vOuverts.isNull() && !vEngages.isNull())
    {
        QVariant disponibles=ouverts-(realises+engages);
        QModelIndex indexDispo=model->index(row,COL_DISPONIBLES);
        model->setData(indexDispo,disponibles);
    }

    //Only propagate after the database has been updated (called for refreshing the view, see https://bugreports.qt-project.org/browse/QTBUG-30672
    if(!model->isDirty())
    {
        propagateToAncestors(topLeft);
    }

}


QList<QPair<unsigned int, QString> > PCx_AuditModel::getListOfAudits(ListAuditsMode mode)
{
    QList<QPair<unsigned int,QString> > listOfAudits;
    QDateTime dt;

    QSqlQuery query("select * from index_audits order by datetime(le_timestamp)");

    while(query.next())
    {
        QString item;
        dt=QDateTime::fromString(query.value("le_timestamp").toString(),"yyyy-MM-dd hh:mm:ss");
        dt.setTimeSpec(Qt::UTC);
        QDateTime dtLocal=dt.toLocalTime();
        QPair<unsigned int, QString> p;
        if(query.value("termine").toBool()==true)
        {
            //Finished audit
            item=QString("%1 - %2").arg(query.value("nom").toString()).arg(dtLocal.toString(Qt::SystemLocaleShortDate));
            if(mode!=UnFinishedAuditsOnly)
            {
                p.first=query.value("id").toUInt();
                p.second=item;
                listOfAudits.append(p);
            }
        }
        else if(mode!=FinishedAuditsOnly)
        {
            //Unfinished audit
            item=QString("%1 - %2").arg(query.value("nom").toString()).arg(dtLocal.toString(Qt::SystemLocaleShortDate));
            p.first=query.value("id").toUInt();
            p.second=item;
            listOfAudits.append(p);
        }
    }
    return listOfAudits;
}

bool PCx_AuditModel::finishAudit(unsigned int id)
{
    Q_ASSERT(id>0);
    QSqlQuery q;
    q.prepare("select count(*) from index_audits where id=:id");
    q.bindValue(":id",id);
    q.exec();
    if(q.next())
    {
        if(q.value(0).toInt()==0)
        {
            qCritical()<<"Terminer Audit inexistant !";
            return false;
        }
    }
    else
    {
        qCritical()<<q.lastError().text();
        die();
    }
    q.prepare("update index_audits set termine=1 where id=:id");
    q.bindValue(":id",id);
    q.exec();

    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError().text();
        die();
    }
    return true;
}

QString PCx_AuditModel::generateHTMLHeader() const
{
    return QString("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n<html>\n<head><title>Audit %1</title>\n<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>\n<style type='text/css'>\n%2\n</style>\n</head>\n<body>\n"
                               "<h3>Audit %1</h3>\n").arg(auditInfos.name.toHtmlEscaped()).arg(getCSS());

}

QString PCx_AuditModel::generateHTMLReportForNode(quint8 bitFieldPagesOfTables, quint16 bitFieldTables, quint16 bitFieldGraphics, unsigned int selectedNode, DFRFDIRI mode,
                                                  QCustomPlot *plot, unsigned int favoriteGraphicsWidth, unsigned int favoriteGraphicsHeight,double scale,QTextDocument *document,
                                                  const QString &absoluteImagePath, const QString &relativeImagePath,QProgressDialog *progress) const
{
    Q_ASSERT(selectedNode>0 && plot!=NULL);

    QString output;

    //Either group of tables, or individual tables
    if(bitFieldPagesOfTables!=0)
    {
        if(bitFieldPagesOfTables & TABRECAP)
            output.append(getTabRecap(selectedNode,mode));

        if(bitFieldPagesOfTables & TABEVOLUTION)
            output.append(getTabEvolution(selectedNode,mode));

        if(bitFieldPagesOfTables & TABEVOLUTIONCUMUL)
            output.append(getTabEvolutionCumul(selectedNode,mode));

        if(bitFieldPagesOfTables & TABBASE100)
            output.append(getTabBase100(selectedNode,mode));

        if(bitFieldPagesOfTables & TABJOURSACT)
            output.append(getTabJoursAct(selectedNode,mode));

        if(bitFieldPagesOfTables & TABRESULTS)
            output.append(getTabResults(selectedNode));
    }

    else if(bitFieldTables!=0)
    {
        if(bitFieldTables & T1)
            output.append(getT1(selectedNode,mode)+"<br>");
        if(bitFieldTables & T2)
            output.append(getT2(selectedNode,mode)+"<br>");
        if(bitFieldTables & T2BIS)
            output.append(getT2bis(selectedNode,mode)+"<br>");
        if(bitFieldTables & T3)
            output.append(getT3(selectedNode,mode)+"<br>");
        if(bitFieldTables & T3BIS)
            output.append(getT3bis(selectedNode,mode)+"<br>");
        if(bitFieldTables & T4)
            output.append(getT4(selectedNode,mode)+"<br>");
        if(bitFieldTables & T5)
            output.append(getT5(selectedNode,mode)+"<br>");
        if(bitFieldTables & T6)
            output.append(getT6(selectedNode,mode)+"<br>");
        if(bitFieldTables & T7)
            output.append(getT7(selectedNode,mode)+"<br>");
        if(bitFieldTables & T8)
            output.append(getT8(selectedNode,mode)+"<br>");
        if(bitFieldTables & T9)
            output.append(getT9(selectedNode,mode)+"<br>");
        if(bitFieldTables & T10)
            output.append(getT10(selectedNode)+"<br>");
        if(bitFieldTables & T11)
            output.append(getT11(selectedNode)+"<br>");
        if(bitFieldTables & T12)
            output.append(getT12(selectedNode)+"<br>");
    }

    if(bitFieldGraphics!=0)
    {
        //Graphics, a little too verbose
        //getGx draw the plot in the hidden QCustomPlot widget, which can be exported to pixmap and inserted into html with <img>

        //inline mode
        if(document!=NULL)
        {
            if(bitFieldGraphics & G1)
            {
                output.append("<div align='center' class='g'><b>"+getG1(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G1' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));

            }
            if(bitFieldGraphics & G2)
            {
                output.append("<div align='center' class='g'><b>"+getG2(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & G3)
            {
                output.append("<div align='center' class='g'><b>"+getG3(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G3' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & G4)
            {
                output.append("<div align='center' class='g'><b>"+getG4(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G4' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & G5)
            {
                output.append("<div align='center' class='g'><b>"+getG5(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G5' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & G6)
            {
                output.append("<div align='center' class='g'><b>"+getG6(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G6' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & G7)
            {
                output.append("<div align='center' class='g'><b>"+getG7(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G7' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & G8)
            {
                output.append("<div align='center' class='g'><b>"+getG8(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G8' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & G9)
            {
                output.append("<div align='center' class='g'><b>"+getG9(selectedNode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
        }

        //Linked images mode. Images are saved into imagePath
        else
        {
            /*Sleep to test progressDialog
            QTime dieTime = QTime::currentTime().addMSecs(5000 );
            while( QTime::currentTime() < dieTime )
            {
                QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
            }*/

            QSettings settings;
            QString imageFormat=settings.value("output/imageFormat","png").toString();
            const char *imgFormat="png";
            int quality=-1;
            if(imageFormat=="png")
            {
                imgFormat="png";
                quality=-1;
            }

            //FIXME : save to JPG quality 100 as a workaround because saving to PNG is very slow
            else if(imageFormat=="jpg")
            {
                imgFormat="jpeg";
                quality=96;
            }
            else
            {
                qCritical()<<"Invalid image format";
                die();
            }
            QString suffix="."+imageFormat;

            int progressValue=0;
            if(progress!=NULL)
            {
                progressValue=progress->value();
            }

            if(absoluteImagePath.isEmpty()|| relativeImagePath.isEmpty())
            {
                qCritical()<<"Please pass an absolute and relative path to store images";
                return QString();
            }

            if(bitFieldGraphics & G1)
            {
                output.append("<div align='center' class='g'><b>"+getG1(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G1' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & G2)
            {
                output.append("<div align='center' class='g'><b>"+getG2(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }


            if(bitFieldGraphics & G3)
            {
                output.append("<div align='center' class='g'><b>"+getG3(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G3' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));

                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & G4)
            {
                output.append("<div align='center' class='g'><b>"+getG4(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G4' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            QCoreApplication::processEvents( QEventLoop::AllEvents,100);

            if(bitFieldGraphics & G5)
            {
                output.append("<div align='center' class='g'><b>"+getG5(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G5' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }


            if(bitFieldGraphics & G6)
            {
                output.append("<div align='center' class='g'><b>"+getG6(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G6' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & G7)
            {
                output.append("<div align='center' class='g'><b>"+getG7(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G7' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & G8)
            {
                output.append("<div align='center' class='g'><b>"+getG8(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G8' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & G9)
            {
                output.append("<div align='center' class='g'><b>"+getG9(selectedNode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G9' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }
        }
    }

    //NOTE : For vectorized graphics
    //cursor.insertText(QString(QChar::ObjectReplacementCharacter), QCPDocumentObject::generatePlotFormat(ui->plot, 600, 400));

    return output;
}

