#ifndef PCX_PREVISIONITEMCRITERIA_H
#define PCX_PREVISIONITEMCRITERIA_H

#include "pcx_audit.h"
#include <QComboBox>
#include <QAbstractListModel>

class PCx_PrevisionItemCriteria
{
public:

    enum PREVISIONOPERATOR
    {
        MINIMUM,
        MAXIMUM,
        AVERAGE,
        REGLIN,
        LASTVALUE,
        PERCENT,
        FIXEDVALUE,
        FIXEDVALUEFROMPROPORTIONALREPARTITION
    };

    PCx_PrevisionItemCriteria();
    PCx_PrevisionItemCriteria(const QString &serializedCriteria);
    PCx_PrevisionItemCriteria(PREVISIONOPERATOR previsionOperator, PCx_Audit::ORED ored, qint64 previsionOperand=0);
    ~PCx_PrevisionItemCriteria();

    qint64 getOperand() const{return previsionOperand;}
    PCx_Audit::ORED getOredTarget() const{return previsionOredTarget;}
    PREVISIONOPERATOR getOperator() const{return previsionOperator;}

    qint64 compute(unsigned int auditId, MODES::DFRFDIRI mode, unsigned int nodeId) const;

    QString getCriteriaLongDescription() const;

    QString serialize() const;
    bool unserialize(const QString &criteriaString);

    static void fillComboBoxWithOperators(QComboBox *combo);



private:


    qint64 getMinimumOf(unsigned int auditId, MODES::DFRFDIRI mode, unsigned int nodeId) const;
    qint64 getMaximumOf(unsigned int auditId, MODES::DFRFDIRI mode, unsigned int nodeId) const;
    qint64 getAverageOf(unsigned int auditId, MODES::DFRFDIRI mode, unsigned int nodeId) const;
    qint64 getReglinOf(unsigned int auditId, MODES::DFRFDIRI mode,unsigned int nodeId) const;
    qint64 getLastValueOf(unsigned int auditId, MODES::DFRFDIRI mode,unsigned int nodeId) const;
    qint64 getPercentOf(unsigned int auditId, MODES::DFRFDIRI mode,unsigned int nodeId) const;
    qint64 getFixedValue() const{return previsionOperand;}


    PREVISIONOPERATOR previsionOperator;
    PCx_Audit::ORED previsionOredTarget;
    qint64 previsionOperand;
};

#endif // PCX_PREVISIONITEMCRITERIA_H