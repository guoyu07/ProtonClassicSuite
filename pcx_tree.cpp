#include "pcx_tree.h"
#include "pcx_treemanage.h"
#include "utils.h"
#include "pcx_typemodel.h"
#include "xlsxdocument.h"
#include <QMessageBox>
#include <QDateTime>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QUuid>
#include <QElapsedTimer>
#include <QFileInfo>


PCx_Tree::PCx_Tree(unsigned int treeId, bool typesReadOnly)
{
    types=new PCx_TypeModel(treeId,typesReadOnly);

    if(loadFromDatabase(treeId)==false)
    {
        qCritical()<<"Unable to load tree"<<treeId;
        die();
    }
}

PCx_Tree::~PCx_Tree()
{
    delete types;
    types=nullptr;
}


unsigned int PCx_Tree::addNode(unsigned int pid, unsigned int typeId, const QString &name)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(pid>0);
    Q_ASSERT(typeId>0);

    if(getNumberOfNodes()>=MAXNODES)
    {
        QMessageBox::warning(nullptr,QObject::tr("Attention"),QObject::tr("Nombre maximum de noeuds atteint !"));
        return 0;
    }

    QSqlQuery q;

    //NOTE : Check for duplicate nodes in tree
    if(nodeExists(name,typeId))
    {
        QMessageBox::warning(nullptr,QObject::tr("Attention"),QObject::tr("Il existe déjà un noeud portant ce nom dans l'arbre' !"));
        return 0;
    }

    q.prepare(QString("insert into arbre_%1 (nom,pid,type) values (:nom, :pid, :type)").arg(treeId));
    q.bindValue(":nom",name);
    q.bindValue(":pid",pid);
    q.bindValue(":type",typeId);
    q.exec();

    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError();
        return 0;
    }

    return q.lastInsertId().toUInt();
}


bool PCx_Tree::updateNode(unsigned int nodeId, const QString &newName, unsigned int newType)
{
    Q_ASSERT(!newName.isNull());
    Q_ASSERT(!newName.isEmpty());
    Q_ASSERT(newType>0);

    QSqlQuery q;

    if(nodeExists(newName,newType))
    {
        QMessageBox::warning(nullptr,QObject::tr("Attention"),QObject::tr("Il existe déjà un noeud portant ce nom !"));
        return false;
    }

    if(!types->isTypeIdValid(newType))
    {
        qCritical()<<"Invalid type !";
        return false;
    }

    q.prepare(QString("update arbre_%1 set nom=:nom, type=:type where id=:id").arg(treeId));
    q.bindValue(":nom",newName);
    q.bindValue(":type",newType);
    q.bindValue(":id",nodeId);
    q.exec();

    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError();
        return false;
    }

    return true;
}



QList<unsigned int> PCx_Tree::getNodesId() const
{
    return PCx_TreeManage::getNodesId(this->treeId);
}


QStringList PCx_Tree::getListOfCompleteNodeNames() const
{
    QStringList nodeNames;
    QSqlQuery q;
    if(!q.exec(QString("select nom,type from arbre_%1 where id>1").arg(treeId)))
    {
        qCritical()<<q.lastError();
        return QStringList();
    }
    while(q.next())
    {
        nodeNames.append(QString("%1 %2").arg(types->idTypeToName(q.value("type").toUInt())).arg(q.value("nom").toString()));
    }
    return nodeNames;
}


QStringList PCx_Tree::getListOfNodeNames() const
{
    QStringList nodeNames;
    QSqlQuery q;
    if(!q.exec(QString("select distinct nom from arbre_%1 where id>1").arg(treeId)))
    {
        qCritical()<<q.lastError();
        return QStringList();
    }
    while(q.next())
    {
        nodeNames.append(q.value("nom").toString());
    }
    return nodeNames;
}

unsigned int PCx_Tree::getNumberOfNodes() const
{
    return PCx_TreeManage::getNumberOfNodes(treeId);
}


QList<unsigned int> PCx_Tree::getLeavesId() const
{
    QList<unsigned int> leaves;
    QList<unsigned int> nodes;
    nodes=getNodesId();
    foreach (unsigned int node, nodes) {
        if(isLeaf(node))
        {
            leaves.append(node);
        }
    }
    //qDebug()<<"Leaves for tree "<<treeId<< " = "<<leaves;
    return leaves;
}


QList<unsigned int> PCx_Tree::getNonLeavesId() const
{
    QList<unsigned int> nonleaves;
    QList<unsigned int> nodes;
    nodes=getNodesId();
    foreach (unsigned int node, nodes) {
        if(!isLeaf(node))
        {
            nonleaves.append(node);
        }
    }
    qDebug()<<"Non-leaves for tree "<<treeId<< " = "<<nonleaves;
    return nonleaves;
}

QSet<unsigned int> PCx_Tree::getNodesWithSharedName() const
{
    QSqlQuery q(QString("select * from arbre_%1").arg(treeId));
    QSet<unsigned int> nodes;
    QHash<QString,QPair<unsigned int,unsigned int> > nameToTypeAndId;
    if(!q.isActive())
    {
        qCritical()<<q.lastError();
        return QSet<unsigned int>();
    }
    while(q.next())
    {
        QString name=q.value("nom").toString().toLower();
        if(nameToTypeAndId.contains(name))
        {
            nodes.insert(q.value("id").toUInt());
            nodes.insert(nameToTypeAndId.value(name).second);
        }
        else
        {
            QPair<unsigned int,unsigned int>typeAndId;
            typeAndId.first=q.value("type").toUInt();
            typeAndId.second=q.value("id").toUInt();
            nameToTypeAndId.insert(name,typeAndId);
        }
    }
    return nodes;
}

bool PCx_Tree::isLeaf(unsigned int nodeId) const
{
    Q_ASSERT(nodeId>0);
    QSqlQuery q;
    q.prepare(QString("select count(*) from arbre_%1 where pid=:nodeid").arg(treeId));
    q.bindValue(":nodeid",nodeId);
    q.exec();
    if(q.next())
    {
        if(q.value(0).toInt()==0)
            return true;
        else return false;
    }
    else
    {
        qCritical()<<q.lastError();
        die();
    }
    return false;
}

unsigned int PCx_Tree::getTreeDepth() const
{
    unsigned int maxDepth=0;
    QList<unsigned int> nodes=getNodesId();
    unsigned int depth,pid,curNode;
    foreach(unsigned int node, nodes)
    {
        if(node==1)
            continue;
        depth=0;

        curNode=node;
        do
        {
            pid=getParentId(curNode);
            depth++;
            curNode=pid;
        }while(pid!=1);

        if(depth>maxDepth)
            maxDepth=depth;
    }
    return maxDepth;
}



QList<unsigned int> PCx_Tree::getIdsOfNodesWithThisType(unsigned int typeId) const
{
    Q_ASSERT(typeId>0);
    QSqlQuery q;
    QList<unsigned int> nodes;
    q.prepare(QString("select id from arbre_%1 where type=:typeId").arg(treeId));
    q.bindValue(":typeId",typeId);
    if(!q.exec())
    {
        qCritical()<<q.lastError();
        return QList<unsigned int>();
    }
    while(q.next())
    {
        nodes.append(q.value(0).toUInt());
    }
    return nodes;
}

unsigned int PCx_Tree::getNumberOfNodesWithThisType(unsigned int typeId) const
{
    Q_ASSERT(typeId>0);
    QSqlQuery q;
    q.prepare(QString("select count(*) from arbre_%1 where type=:typeId").arg(treeId));
    q.bindValue(":typeId",typeId);
    if(!q.exec())
    {
        qCritical()<<q.lastError();
        die();
    }
    if(!q.next())
        return 0;
    return q.value(0).toUInt();
}

QList<unsigned int> PCx_Tree::sortNodesBFS(QList<unsigned int> &nodes) const
{
    QList<unsigned int>sortedNodes;

    QList<unsigned int>toCheck;
    toCheck.append(1);

    while(!toCheck.isEmpty())
    {
        unsigned int node=toCheck.takeFirst();
        if(nodes.contains(node))
            sortedNodes.append(node);
        toCheck.append(getChildren(node));
    }
    return sortedNodes;
}


QList<unsigned int> PCx_Tree::sortNodesDFS(QList<unsigned int> &nodes,unsigned int currentNode) const
{
    QList<unsigned int> sortedNodes;

    if(nodes.contains(currentNode))
        sortedNodes.append(currentNode);

    QList<unsigned int> children=getChildren(currentNode);
    foreach(unsigned int childId,children)
    {
        sortedNodes.append(sortNodesDFS(nodes,childId));
    }
    return sortedNodes;
}

unsigned int PCx_Tree::getParentId(unsigned int nodeId) const
{
    QSqlQuery q;
    q.prepare(QString("select pid from arbre_%1 where id=:nodeid").arg(treeId));
    q.bindValue(":nodeid",nodeId);
    q.exec();
    if(q.next())
    {
        return q.value(0).toUInt();
    }
    else
    {
        qCritical()<<q.lastError();
    }
    return 0;
}

QList<unsigned int> PCx_Tree::getChildren(unsigned int nodeId) const
{
    QList<unsigned int> listOfChildren;
    QSqlQuery q;
    q.prepare(QString("select id from arbre_%1 where pid=:nodeid order by nom").arg(treeId));
    q.bindValue(":nodeid",nodeId);
    q.exec();

    while(q.next())
    {
        listOfChildren.append(q.value(0).toUInt());
    }
    return listOfChildren;
}



void PCx_Tree::updateNodePid(unsigned int nodeId, unsigned int newPid)
{
    Q_ASSERT(nodeId>1 && newPid > 0);
    QSqlQuery q;

    q.prepare(QString("update arbre_%1 set pid=:pid where id=:id").arg(treeId));
    q.bindValue(":pid",newPid);
    q.bindValue(":id",nodeId);
    q.exec();

    if(q.numRowsAffected()!=1)
    {
        qCritical()<<q.lastError();
        die();
    }
}

bool PCx_Tree::finishTree()
{
    Q_ASSERT(treeId>0);
    if(this->finished==false)
    {
        QSqlQuery q;
        q.prepare("update index_arbres set termine=1 where id=:id");
        q.bindValue(":id",treeId);
        q.exec();
        if(q.numRowsAffected()!=1)
        {
            qCritical()<<q.lastError();
            return false;
        }
        this->finished=true;
    }
    return true;
}





bool PCx_Tree::nodeExists(const QString &name, unsigned int typeId) const
{
    QSqlQuery q;

    q.prepare(QString("select count(*) from arbre_%1 where nom=:nom and type=:type").arg(treeId));
    q.bindValue(":nom",name);
    q.bindValue(":type",typeId);
    q.exec();

    if(q.next())
    {
        if(q.value(0).toInt()>0)
        {
            return true;
        }
    }
    else
    {
        qCritical()<<q.lastError();
        die();
    }
    return false;
}

int PCx_Tree::getNodeIdFromTypeAndNodeName(const QPair<QString, QString> &typeAndNodeName) const
{
    int typeId=types->nameToIdType(typeAndNodeName.first);
    if(typeId==-1)
        return -1;
    QSqlQuery q;
    QString nodeNameSpl=typeAndNodeName.second.simplified();
    q.prepare(QString("select id from arbre_%1 where nom=:nameId and type=:typeId").arg(treeId));
    q.bindValue(":nameId",nodeNameSpl);
    q.bindValue(":typeId",typeId);
    if(!q.exec())
    {
        qCritical()<<q.lastError();
        return -1;
    }
    if(!q.next())
    {
        return -1;
    }
    return q.value(0).toInt();
}


QPair<QString,QString> PCx_Tree::getTypeNameAndNodeName(unsigned int node) const
{
    Q_ASSERT(node>0);
    QSqlQuery q;
    q.prepare(QString("select type,nom from arbre_%1 where id=:id").arg(treeId));
    q.bindValue(":id",node);
    q.exec();

    QPair<QString,QString> typeNameAndNodeName;

    if(q.next())
    {
        if(node>1)
        {
            QString typeName=types->idTypeToName(q.value("type").toUInt());
            typeNameAndNodeName.first=typeName;
            typeNameAndNodeName.second=q.value("nom").toString();
        }
        //Root does not has type
        else
        {
            typeNameAndNodeName.first=QString();
            typeNameAndNodeName.second=q.value("nom").toString();
        }
        return typeNameAndNodeName;
    }
    else
    {
        qCritical()<<"Inexistant node"<<node;
    }
    return QPair<QString,QString>();

}


QString PCx_Tree::getNodeName(unsigned int node) const
{
    Q_ASSERT(node>0);
    QSqlQuery q;
    q.prepare(QString("select type,nom from arbre_%1 where id=:id").arg(treeId));
    q.bindValue(":id",node);
    q.exec();

    if(q.next())
    {
        if(node>1)
        {
            QString typeName=types->idTypeToName(q.value("type").toUInt());
            return QString("%1 %2").arg(typeName).arg(q.value("nom").toString());
        }
        //Root does not has type
        else
        {
            return q.value("nom").toString();
        }
    }
    else
    {
        qCritical()<<"Inexistant node"<<node;
    }
    return QString();
}


bool PCx_Tree::deleteNode(unsigned int nodeId)
{
    Q_ASSERT(nodeId>0);
    QList<unsigned int> listOfChildrens;
    QSqlQuery q;
    q.prepare(QString("select id from arbre_%1 where pid=:pid").arg(treeId));
    q.bindValue(":pid",nodeId);
    q.exec();

    if(!q.isActive())
    {
        qCritical()<<q.lastError();
        return false;
    }
    while(q.next())
    {
        listOfChildrens.append(q.value(0).toUInt());
    }

    if(listOfChildrens.size()==0)
    {
        q.prepare(QString("delete from arbre_%1 where id=:id").arg(treeId));
        q.bindValue(":id",nodeId);
        q.exec();

        if(q.numRowsAffected()!=1)
        {
            qCritical()<<q.lastError();
            die();
        }
        return true;
    }
    else
    {
        foreach(unsigned int child,listOfChildrens)
        {
            deleteNode(child);
        }
        deleteNode(nodeId);
    }
    return true;
}



QString PCx_Tree::toDot() const
{
    QList<unsigned int> nodes=getNodesId();

    QString out="graph g{\nrankdir=LR;\n";

    foreach(unsigned int node,nodes)
    {
        //Escape double quotes for dot format
        out.append(QString("%1 [label=\"%2\"];\n").arg(node).arg(getNodeName(node).replace('"',QString("\\\""))));
    }

    foreach(unsigned int node,nodes)
    {
        unsigned int pid=getParentId(node);

        if(pid!=0)
        {
            out.append(QString("\t%1--%2;\n").arg(pid).arg(node));
        }
    }
    out.append("}\n");
    return out;
}

bool PCx_Tree::toXLSX(const QString &fileName) const
{
    Q_ASSERT(!fileName.isEmpty());
    QList<unsigned int> nodes=getNodesId();

    QXlsx::Document xlsx;

    xlsx.write(1,1,"Type noeud");
    xlsx.write(1,2,"Nom noeud");
    xlsx.write(1,3,"Type père");
    xlsx.write(1,4,"Nom père");

    QPair<QString,QString>typeNameAndNodeName,pidTypeNameAndPidNodeName;

    int row=2;
    foreach(unsigned int node,nodes)
    {
        unsigned int pid=getParentId(node);
        typeNameAndNodeName=getTypeNameAndNodeName(node);

        if(pid>1)
        {
            xlsx.write(row,1,typeNameAndNodeName.first);
            xlsx.write(row,2,typeNameAndNodeName.second);
            pidTypeNameAndPidNodeName=getTypeNameAndNodeName(pid);
            xlsx.write(row,3,pidTypeNameAndPidNodeName.first);
            xlsx.write(row,4,pidTypeNameAndPidNodeName.second);
            row++;
        }
        else if(pid==1)
        {
            xlsx.write(row,1,typeNameAndNodeName.first);
            xlsx.write(row,2,typeNameAndNodeName.second);
            row++;
        }

    }
    return xlsx.saveAs(fileName);
}

bool PCx_Tree::loadFromDatabase(unsigned int treeId)
{
    Q_ASSERT(treeId>0);

    QSqlQuery query;

    query.prepare("SELECT nom,termine,le_timestamp from index_arbres where id=:id");
    query.bindValue(":id",treeId);
    if(!query.exec())
    {
        qCritical()<<query.lastError();
        return false;
    }
    if(query.next())
    {
        this->treeName=query.value("nom").toString();
        this->treeId=treeId;
        this->finished=query.value("termine").toBool();
        this->creationTime=query.value("le_timestamp").toString();
    }
    else
    {
        qWarning()<<"Missing Tree";
        return false;
    }
    return true;
}
