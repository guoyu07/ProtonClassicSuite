#ifndef PCX_QUERY_H
#define PCX_QUERY_H

#include <climits>
#include "xlsxdocument.h"
#include "pcx_audit.h"

class PCx_Query
{
public:

    enum QUERIESTYPES
    {
        VARIATION,
        MINMAX,
        RANK
    };

    static const unsigned int ALLTYPES=UINT_MAX;

    PCx_Query(PCx_Audit *model);
    PCx_Query(PCx_Audit *model,unsigned int typeId,PCx_AuditManage::ORED ored,PCx_AuditManage::DFRFDIRI dfrfdiri,
                                      unsigned int year1, unsigned int year2,const QString &name="");

    QPair<unsigned int,unsigned int> getYears() const {return QPair<unsigned int, unsigned int>(year1,year2);}
    virtual void setYears(unsigned int year1, unsigned int year2);

    void setORED(PCx_AuditManage::ORED ored){this->ored=ored;}
    PCx_AuditManage::ORED getORED() const{return ored;}

    void setDFRFDIRI(PCx_AuditManage::DFRFDIRI dfrfdiri){this->dfrfdiri=dfrfdiri;}
    PCx_AuditManage::DFRFDIRI getDFRFDIRI() const{return dfrfdiri;}

    unsigned int getTypeId() const{return typeId;}
    void setTypeId(unsigned int typeId){this->typeId=typeId;}

    void setModel(PCx_Audit *model){this->model=model;}
    const PCx_Audit *getModel() const{return model;}

    const QString &getName() const{return name;}
    void setName(const QString &name){this->name=name;}

    bool remove(unsigned int queryId);

    virtual bool save(const QString &name) const =0;
    virtual bool canSave(const QString &name) const =0;
    virtual bool load(unsigned int queryId)=0;
    static bool deleteQuery(unsigned int auditId,unsigned int queryId);

    virtual QString getDescription() const =0;
    virtual QString exec(QXlsx::Document *xlsxDocument=nullptr) const =0;


    static bool createTableQueries(unsigned int auditId);

    static QString getCSS();


protected:
    PCx_Audit *model;
    //If typeId==ALLTYPES, do not filter nodes
    unsigned int typeId;
    PCx_AuditManage::ORED ored;
    PCx_AuditManage::DFRFDIRI dfrfdiri;
    unsigned int year1,year2;
    QString name;
    unsigned int queryId;

};

#endif // PCX_QUERY_H
