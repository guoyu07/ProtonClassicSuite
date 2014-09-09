#ifndef PCX_AUDIT_H
#define PCX_AUDIT_H

#include "pcx_treemodel.h"
#include "pcx_auditmanage.h"
#include <QDateTime>


class PCx_Audit
{

public:


    explicit PCx_Audit(unsigned int auditId,bool loadTreeModel=true);
    virtual ~PCx_Audit();



    //Getters
    unsigned int getAuditId() const{return auditId;}
    QString getAuditName() const{return auditName;}
    static QString getAuditName(unsigned int auditId);
    QString getAttachedTreeName() const{return attachedTreeName;}
    PCx_TreeModel *getAttachedTreeModel() const{return attachedTree;}
    unsigned int getAttachedTreeId() const{return attachedTreeId;}

    bool isFinished() const{return finished;}
    QString isFinishedString() const{return finishedString;}
    QList<unsigned int> getYears() const{return years;}
    QString getYearsString() const{return yearsString;}
    QStringList getYearsStringList() const{return yearsStringList;}
    QDateTime getCreationTimeUTC()const{return creationTimeUTC;}
    QDateTime getCreationTimeLocal()const{return creationTimeLocal;}

    bool finishAudit();
    bool unFinishAudit();

    virtual bool setLeafValues(unsigned int leafId, PCx_AuditManage::DFRFDIRI mode, unsigned int year, QHash<PCx_AuditManage::ORED, double> vals);
    qint64 getNodeValue(unsigned int nodeId, PCx_AuditManage::DFRFDIRI mode, PCx_AuditManage::ORED ored, unsigned int year) const;

    virtual bool clearAllData(PCx_AuditManage::DFRFDIRI mode);

    int duplicateAudit(const QString &newName,QList<unsigned int> years,
                                bool copyDF=true,bool copyRF=true, bool copyDI=true, bool copyRI=true) const;


    QString getHTMLAuditStatistics() const;
    QList<unsigned int> getNodesWithNonNullValues(PCx_AuditManage::DFRFDIRI mode, unsigned int year) const;
    QList<unsigned int> getNodesWithAllNullValues(PCx_AuditManage::DFRFDIRI mode, unsigned int year) const;
    QList<unsigned int> getNodesWithAllZeroValues(PCx_AuditManage::DFRFDIRI mode, unsigned int year) const;


    int readDataFromTSV(const QString &fileName, PCx_AuditManage::DFRFDIRI mode);




signals:

public slots:


protected:

    unsigned int auditId;
    QString auditName;
    unsigned int attachedTreeId;
    PCx_TreeModel *attachedTree;

    bool loadTreeModel;

    QString attachedTreeName;

    QList<unsigned int> years;
    QString yearsString;
    QStringList yearsStringList;

    bool finished;
    QString finishedString;

    QDateTime creationTimeUTC;
    QDateTime creationTimeLocal;

    bool updateParent(const QString &tableName, unsigned int annee, unsigned int nodeId);


private:
    PCx_Audit(const PCx_Audit &c);
    PCx_Audit &operator=(const PCx_Audit &);
};


#endif // PCX_AUDIT_H
