#include "pcx_previsionitem.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QElapsedTimer>


PCx_PrevisionItem::PCx_PrevisionItem(PCx_Prevision *prevision, MODES::DFRFDIRI mode, unsigned int nodeId, unsigned int year)
    :prevision(prevision),mode(mode),nodeId(nodeId),year(year)
{

}

PCx_PrevisionItem::~PCx_PrevisionItem()
{

}

void PCx_PrevisionItem::loadFromDb()
{
    QSqlQuery q;
    computedValue=0;
    itemsToAdd.clear();
    itemsToSubstract.clear();
    computedValue=0;
    label=QString();

    q.prepare(QString("select * from prevision_%1_%2 where id_node=:id_node and year=:year").arg(MODES::modeToTableString(mode))
              .arg(prevision->getPrevisionId()));
    q.bindValue(":id_node",nodeId);
    q.bindValue(":year",year);
    if(!q.exec())
    {
        qCritical()<<q.lastError();
        die();
    }
    if(q.next())
    {
        label=q.value("label").toString();
        computedValue=q.value("computedPrevision").toLongLong();
        QStringList items=q.value("prevision_operators_to_add").toString().split(';',QString::SkipEmptyParts);
        foreach(const QString &item,items)
        {
            PCx_PrevisionItemCriteria criteria(item);
            itemsToAdd.append(criteria);
        }

        items.clear();
        items=q.value("prevision_operators_to_substract").toString().split(';',QString::SkipEmptyParts);


        foreach(const QString &item,items)
        {
            PCx_PrevisionItemCriteria criteria(item);
            itemsToSubstract.append(criteria);
        }
    }
}

QString PCx_PrevisionItem::getNodePrevisionHTMLReport() const
{
    QString output;

    if(!prevision->getAttachedTree()->isLeaf(nodeId))
    {
        output=QString("Critères appliqués aux descendants de <b>%1</b> (%2)").arg(prevision->getAttachedTree()->getNodeName(nodeId).toHtmlEscaped())
                .arg(MODES::modeToCompleteString(mode));
        QList<unsigned int> descendants=prevision->getAttachedTree()->getDescendantsId(nodeId);
        foreach(unsigned int descendant,descendants)
        {
            PCx_PrevisionItem tmpItem(prevision,mode,descendant,year);
            tmpItem.loadFromDb();
            if(!tmpItem.getItemsToAdd().isEmpty() || !tmpItem.getItemsToSubstract().isEmpty())
            {
                output.append(QString("<p><a href='#node_%3'><b>%1 : %2€</b></a></p>").arg(prevision->getAttachedTree()->getNodeName(descendant).toHtmlEscaped())
                              .arg(NUMBERSFORMAT::formatFixedPoint(tmpItem.getSummedPrevisionItemValue()))
                              .arg(descendant));
                output.append("<ul>"+tmpItem.dataAsHTML()+"</ul>");
            }
        }
    }

    else
    {
        if(!itemsToAdd.isEmpty()||!itemsToSubstract.isEmpty())
        {
            output.append(QString("<p><b>%1 : %2€</b></p>").arg(prevision->getAttachedTree()->getNodeName(nodeId).toHtmlEscaped())
                          .arg(NUMBERSFORMAT::formatFixedPoint(getSummedPrevisionItemValue())));

            output.append("<ul>"+dataAsHTML()+"</ul>");
        }
    }
    return output;
}

qint64 PCx_PrevisionItem::getSummedPrevisionItemValue() const
{
    qint64 total=0;

    if(prevision->getAttachedTree()->isLeaf(nodeId))
    {
        return computedValue;
    }

    QList<unsigned int> childrenLeaves=prevision->getAttachedTree()->getLeavesId(nodeId);

    /* Fastest but possibly limited by the number of statements in 'IN' sql
    QStringList childrenLeavesIdString;
    foreach(unsigned int leaf,childrenLeaves)
    {
        childrenLeavesIdString.append(QString::number(leaf));
    }

    QSqlQuery q(QString("select sum(computedPrevision) from prevision_%1_%2 where year=%3 and id_node in (%4)")
                .arg(MODES::modeToTableString(mode))
                .arg(prevision->getPrevisionId())
                .arg(year)
                .arg(childrenLeavesIdString.join(',')));

    if(q.next())
    {
        total=q.value(0).toLongLong();
    }
    else
    {
        qCritical()<<q.lastError();
        die();
    }
*/
    QString reqString=QString("select computedPrevision from prevision_%1_%2 where year=%3 and id_node=")
            .arg(MODES::modeToTableString(mode))
            .arg(prevision->getPrevisionId())
            .arg(year);



    foreach(unsigned int leaf,childrenLeaves)
    {
        QSqlQuery q(reqString+QString::number(leaf));

        if(q.next())
        {
            total+=q.value(0).toLongLong();
        }
    }
    return total;
}

qint64 PCx_PrevisionItem::getPrevisionItemValue() const
{
    qint64 total=0;

    foreach(const PCx_PrevisionItemCriteria &criteria, itemsToAdd)
    {
        total+=criteria.compute(prevision->getAttachedAuditId(),mode,nodeId);
    }
    foreach(const PCx_PrevisionItemCriteria &criteria, itemsToSubstract)
    {
        total-=criteria.compute(prevision->getAttachedAuditId(),mode,nodeId);
    }
    return total;
}

void PCx_PrevisionItem::insertCriteriaToAdd(PCx_PrevisionItemCriteria criteria,bool compute)
{
    itemsToAdd.append(criteria);
    if(compute)
        computedValue=getPrevisionItemValue();
}


void PCx_PrevisionItem::dispatchCriteriaItemsToChildrenLeaves()
{
    QList<unsigned int> descendantsId;
    QList<unsigned int> leavesId;

    descendantsId=prevision->getAttachedTree()->getDescendantsId(nodeId);
    leavesId=prevision->getAttachedTree()->getLeavesId(nodeId);

    //Dispatch items to children leaves

    QSqlDatabase::database().transaction();
    foreach(unsigned int descendant,descendantsId)
    {
        PCx_PrevisionItem tmpItem(prevision,mode,descendant,year);

        if(leavesId.contains(descendant))
        {
            foreach(const PCx_PrevisionItemCriteria &criteria,itemsToAdd)
            {
                tmpItem.insertCriteriaToAdd(criteria,false);
            }
            foreach(const PCx_PrevisionItemCriteria &criteria,itemsToSubstract)
            {
                tmpItem.insertCriteriaToSub(criteria,false);
            }
            tmpItem.computedValue=tmpItem.getPrevisionItemValue();
        }
        tmpItem.saveDataToDb();
    }

    //Remove current items
    if(!leavesId.contains(nodeId))
        deleteAllCriteria();
    saveDataToDb();

    //Remove items from ancestors

    QList<unsigned int> ancestors=prevision->getAttachedTree()->getAncestorsId(nodeId);
    foreach(unsigned int ancestor,ancestors)
    {
        PCx_PrevisionItem tmpModel(prevision,mode,ancestor,year);
        tmpModel.deleteAllCriteria();
        tmpModel.saveDataToDb();
    }
    QSqlDatabase::database().commit();

}


void PCx_PrevisionItem::dispatchComputedValueToChildrenLeaves() const
{
    QList<unsigned int> descendantsId=prevision->getAttachedTree()->getDescendantsId(nodeId);
    QList<unsigned int> leavesId=prevision->getAttachedTree()->getLeavesId(nodeId);


    unsigned int lastYear=prevision->getAttachedAudit()->getYears().last();
    qint64 total=prevision->getAttachedAudit()->getNodeValue(nodeId,mode,PCx_Audit::ORED::REALISES,lastYear);
    if(total==0)
    {
        qWarning()<<QObject::tr("Ne peut pas répartir sur la base d'un réalisé nul en %1 !").arg(lastYear);
        return;
    }


    QSqlDatabase::database().transaction();
    if(itemsToAdd.isEmpty()&&itemsToSubstract.isEmpty())
    {
        foreach(unsigned int descendant,descendantsId)
        {
            PCx_PrevisionItem tmpItem(prevision,mode,descendant,year);
            tmpItem.deleteAllCriteria();
            tmpItem.saveDataToDb();
        }
    }

    else
    {
        foreach(unsigned int descendant,descendantsId)
        {
            PCx_PrevisionItem tmpItem(prevision,mode,descendant,year);

            if(leavesId.contains(descendant))
            {
                qint64 val=prevision->getAttachedAudit()->getNodeValue(descendant,mode,PCx_Audit::ORED::REALISES,lastYear);
                if(val!=-MAX_NUM)
                {
                    double percent=(double)val/total;
                    double newVal=NUMBERSFORMAT::fixedPointToDouble(computedValue)*percent;
                    PCx_PrevisionItemCriteria criteria(PCx_PrevisionItemCriteria::PREVISIONOPERATOR::FIXEDVALUEFROMPROPORTIONALREPARTITION,PCx_Audit::ORED::OUVERTS,NUMBERSFORMAT::doubleToFixedPoint(newVal));
                    tmpItem.insertCriteriaToAdd(criteria);
                    tmpItem.saveDataToDb();
                }
                else
                {
                    PCx_PrevisionItemCriteria criteria(PCx_PrevisionItemCriteria::PREVISIONOPERATOR::FIXEDVALUEFROMPROPORTIONALREPARTITION,PCx_Audit::ORED::OUVERTS,0);
                    tmpItem.insertCriteriaToAdd(criteria);
                    tmpItem.saveDataToDb();
                }
            }
            else
            {
                tmpItem.saveDataToDb();
            }
        }
    }
    //Remove items from ancestors
    QList<unsigned int> ancestors=prevision->getAttachedTree()->getAncestorsId(nodeId);
    foreach(unsigned int ancestor,ancestors)
    {
        PCx_PrevisionItem tmpItem(prevision,mode,ancestor,year);
        tmpItem.saveDataToDb();
    }
    QSqlDatabase::database().commit();
}

void PCx_PrevisionItem::insertCriteriaToSub(PCx_PrevisionItemCriteria criteria,bool compute)
{
    itemsToSubstract.append(criteria);
    if(compute)
        computedValue=getPrevisionItemValue();
}

bool PCx_PrevisionItem::deleteCriteria(QModelIndexList selectedIndexes,bool compute)
{
    //TODO : only single selection supported
    if(selectedIndexes.isEmpty())
        return true;
    if(selectedIndexes.size()>1)
    {
        qDebug()<<"Multiselect not implemented";
    }
    const QModelIndex &index=selectedIndexes[0];

    if(index.row()<itemsToAdd.size())
    {
        itemsToAdd.removeAt(index.row());
    }
    else
    {
        itemsToSubstract.removeAt(index.row()-itemsToAdd.size());
    }
    if(compute)
        computedValue=getPrevisionItemValue();
    //  saveDataToDb();

    return true;
}

void PCx_PrevisionItem::deleteAllCriteria()
{
    itemsToAdd.clear();
    itemsToSubstract.clear();
    computedValue=0;
    //  saveDataToDb();
}



QString PCx_PrevisionItem::dataAsHTML() const
{
    QString output;
    unsigned int auditId=prevision->getAttachedAuditId();

    foreach(const PCx_PrevisionItemCriteria &criteria,itemsToAdd)
    {
        output.append(QString("<li style='background-color:lightgreen'>%1 : %2€</li>")
                      .arg(criteria.getCriteriaLongDescription().toHtmlEscaped())
                      .arg(NUMBERSFORMAT::formatFixedPoint(criteria.compute(auditId,mode,nodeId))));
    }
    foreach(const PCx_PrevisionItemCriteria &criteria,itemsToSubstract)
    {
        output.append(QString("<li style='background-color:red'>%1 : %2€</li>")
                      .arg(criteria.getCriteriaLongDescription().toHtmlEscaped())
                      .arg(NUMBERSFORMAT::formatFixedPoint(criteria.compute(auditId,mode,nodeId))));
    }
    return output;

}



void PCx_PrevisionItem::saveDataToDb() const
{
    QSqlQuery q;
    QStringList items;
    foreach(const PCx_PrevisionItemCriteria &criteria, itemsToAdd)
    {
        items.append(criteria.serialize());
    }
    QString addString=items.join(';');
    items.clear();
    foreach(const PCx_PrevisionItemCriteria &criteria, itemsToSubstract)
    {
        items.append(criteria.serialize());
    }
    QString subString=items.join(';');

    q.prepare(QString("insert into prevision_%1_%2 (id_node,year,prevision_operators_to_add, prevision_operators_to_substract,computedPrevision) values (:idnode,:year,:prevopadd,:prevopsub,:computed)")
              .arg(MODES::modeToTableString(mode))
              .arg(prevision->getPrevisionId()));
    q.bindValue(":idnode",nodeId);
    q.bindValue(":year",year);
    q.bindValue(":prevopadd",addString);
    q.bindValue(":prevopsub",subString);
    q.bindValue(":computed",computedValue);
    if(!q.exec())
    {
        qCritical()<<q.lastError();
        die();
    }
    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError();
        die();
    }

}


QString PCx_PrevisionItem::getLongDescription() const
{
    QString out;
    QStringList listItems;
    if(itemsToAdd.size()==0 && itemsToSubstract.size()==0)
        return QObject::tr("aucune prévision");
    foreach(const PCx_PrevisionItemCriteria &criteria,itemsToAdd)
    {
        listItems.append(criteria.getCriteriaLongDescription());
    }
    out.append(listItems.join('+'));

    listItems.clear();

    foreach(const PCx_PrevisionItemCriteria &criteria,itemsToSubstract)
    {
        listItems.append(criteria.getCriteriaLongDescription());
    }
    if(listItems.size()>0)
    {
        out.append('-');
        out.append(listItems.join('-'));
    }
    return out;
}

