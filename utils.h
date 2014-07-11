#ifndef UTILS_H
#define UTILS_H

#include <QString>

//Currencies are stored as fixed point, multiplied with this coeff
#define FIXEDPOINTCOEFF 1000
#define DEFAULTNUMDECIMALS 2

//Maximum acceptable number
//Will be stored multiplied by FIXEDPOINTCOEFF into an int64
//Must be < MAX_DBL to fit in QDoubleSpinBox
#define MAX_NUM 4294967295u

enum FORMATMODE
{
    FORMATMODENORMAL,
    FORMATMODETHOUSANDS,
    FORMATMODEMILLIONS
};



void die(int retcode=1);

void updateFormatModeAndDecimals();

QString newDb();
bool loadDb(const QString &databaseName);
void initCurrentDb(void);
QString formatDouble(double num,int decimals=-1, bool forcedUnits=false);
QString formatCurrency(qint64 num, int decimals=-1, bool forcedUnits=false);
QString generateUniqueFileName(const QString &suffix="");

#endif // UTILS_H
