#ifndef PCX_QUERYRANK_H
#define PCX_QUERYRANK_H

#include "pcx_query.h"

class PCx_QueryRank : public PCx_Query
{
public:

    enum GREATERSMALLER
    {
        SMALLER,
        GREATER
    };

    PCx_QueryRank();
    PCx_QueryRank(PCx_AuditModel *model,unsigned int queryId);

    PCx_QueryRank(PCx_AuditModel *model, unsigned int typeId, PCx_AuditModel::ORED ored, PCx_AuditModel::DFRFDIRI dfrfdiri,
                       GREATERSMALLER greaterOrSmaller, unsigned int number, unsigned int year1,unsigned int year2, const QString &name="");


    unsigned int getNumber()const{return number;}
    GREATERSMALLER getGreaterOrSmaller()const{return grSm;}

    void setNumber(unsigned int num){number=num;}
    void setGreaterOrSmaller(GREATERSMALLER grsm){grSm=grsm;}

    bool save(const QString &name) const;
    QString exec() const;
    bool load(unsigned int queryId);
    bool canSave(const QString &name) const;
    QString getDescription() const;

    static QString greaterSmallerToString(GREATERSMALLER grSm);


private:

    GREATERSMALLER grSm;
    unsigned int number;








};

#endif // PCX_QUERYRANK_H