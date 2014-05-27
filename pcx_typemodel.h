#ifndef PCX_TYPEMODEL_H
#define PCX_TYPEMODEL_H
#include <QtGui>
#include <QtSql>

class PCx_TypeModel:public QObject
{
    Q_OBJECT
public:
    explicit PCx_TypeModel(unsigned int treeId, QObject *parent=0);
    ~PCx_TypeModel();

    QString getNomType(unsigned int id) const{return idTypesToNom[id];}
    const QStringList &getNomTypes() const {return nomTypes;}

    bool addType(const QString &type);
    bool deleteType(const QString &type);
    bool deleteType(unsigned int id);

    unsigned int getTreeId() const{return treeId;}
    QSqlTableModel *getTableModel(){return typesTableModel;}

    static QStringList getListOfDefaultTypes();

signals:
    void typesUpdated(void);


private slots:
    bool onTypesModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:

    bool validateType(const QString &newType);
    bool loadFromDatabase(unsigned int treeId);
    bool loadSqlTableModel();

    QHash<unsigned int,QString> idTypesToNom;
    unsigned int treeId;
    QStringList nomTypes;
    QSqlTableModel *typesTableModel;
};

#endif // PCX_TYPEMODEL_H
