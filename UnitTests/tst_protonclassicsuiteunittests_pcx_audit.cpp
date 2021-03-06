/*
* ProtonClassicSuite
* 
* Copyright Thibault et Gilbert Mondary, Laboratoire de Recherche pour le Développement Local (2006--)
* 
* labo@gipilab.org
* 
* Ce logiciel est un programme informatique servant à cerner l'ensemble des données budgétaires
* de la collectivité territoriale (audit, reporting infra-annuel, prévision des dépenses à venir)
* 
* 
* Ce logiciel est régi par la licence CeCILL soumise au droit français et
* respectant les principes de diffusion des logiciels libres. Vous pouvez
* utiliser, modifier et/ou redistribuer ce programme sous les conditions
* de la licence CeCILL telle que diffusée par le CEA, le CNRS et l'INRIA 
* sur le site "http://www.cecill.info".
* 
* En contrepartie de l'accessibilité au code source et des droits de copie,
* de modification et de redistribution accordés par cette licence, il n'est
* offert aux utilisateurs qu'une garantie limitée. Pour les mêmes raisons,
* seule une responsabilité restreinte pèse sur l'auteur du programme, le
* titulaire des droits patrimoniaux et les concédants successifs.
* 
* A cet égard l'attention de l'utilisateur est attirée sur les risques
* associés au chargement, à l'utilisation, à la modification et/ou au
* développement et à la reproduction du logiciel par l'utilisateur étant 
* donné sa spécificité de logiciel libre, qui peut le rendre complexe à 
* manipuler et qui le réserve donc à des développeurs et des professionnels
* avertis possédant des connaissances informatiques approfondies. Les
* utilisateurs sont donc invités à charger et tester l'adéquation du
* logiciel à leurs besoins dans des conditions permettant d'assurer la
* sécurité de leurs systèmes et ou de leurs données et, plus généralement, 
* à l'utiliser et l'exploiter dans les mêmes conditions de sécurité. 
* 
* Le fait que vous puissiez accéder à cet en-tête signifie que vous avez 
* pris connaissance de la licence CeCILL, et que vous en avez accepté les
* termes.
*
*/
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "utils.h"
#include "pcx_audit.h"
#include "tst_protonclassicsuiteunittests.h"

void ProtonClassicSuiteUnitTests::testCaseForAuditManagement()
{

    for(unsigned int i=0;i<sizeof(PCx_Audit::ORED);i++)
    {
        QVERIFY(!PCx_Audit::OREDtoCompleteString((PCx_Audit::ORED)i).isEmpty());
        QVERIFY(!PCx_Audit::OREDtoTableString((PCx_Audit::ORED)i).isEmpty());
        QCOMPARE(PCx_Audit::OREDFromTableString(PCx_Audit::OREDtoTableString((PCx_Audit::ORED)i)),(PCx_Audit::ORED)i);
    }


    unsigned int treeId=PCx_Tree::createRandomTree("RANDOMTREE",20);
    PCx_Tree tree(treeId);
    tree.finishTree();

    QCOMPARE(PCx_Audit::auditIdExists(21412),false);
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits).isEmpty());
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly).isEmpty());
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly).isEmpty());
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::AllAudits));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::FinishedAuditsOnly));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly));

    QList<unsigned int> years={2010,2011,2012,2013};
    unsigned int auditId=PCx_Audit::addNewAudit("TESTAUDIT",years,treeId);
    QCOMPARE(PCx_Audit::auditIdExists(auditId),true);
    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits).isEmpty());
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly).isEmpty());
    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly).isEmpty());
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::AllAudits));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::FinishedAuditsOnly));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly));

    PCx_Audit audit(auditId);

    QCOMPARE(audit.getYears(),years);
    QCOMPARE(audit.getYearsString(),QString("2010 - 2013"));
    QStringList yearsStringList={"2010","2011","2012","2013"};
    QCOMPARE(audit.getYearsStringList(),yearsStringList);

    QVERIFY(!audit.isFinished());
    QCOMPARE(audit.isFinishedString(),QString("non"));
    audit.finishAudit();
    QVERIFY(audit.isFinished());
    QCOMPARE(audit.isFinishedString(),QString("oui"));

    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits).isEmpty());
    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly).isEmpty());
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly).isEmpty());

    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::AllAudits));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::FinishedAuditsOnly));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly));

    audit.unFinishAudit();
    QVERIFY(!audit.isFinished());
    QCOMPARE(audit.isFinishedString(),QString("non"));

    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits).isEmpty());
    QVERIFY(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly).isEmpty());
    QVERIFY(!PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly).isEmpty());
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::AllAudits),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::AllAudits));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::FinishedAuditsOnly));
    QCOMPARE(PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly),
             PCx_Audit::getListOfAuditsAttachedWithThisTree(treeId,PCx_Audit::ListAuditsMode::UnFinishedAuditsOnly));

    QVERIFY(PCx_Audit::auditNameExists("TESTAUDIT"));
    //Garbage audit name
    QVERIFY(!PCx_Audit::auditNameExists("FREFRTGR"));

    QCOMPARE(audit.getAttachedTreeId(),treeId);
    QCOMPARE(audit.getAttachedTree()->getTreeId(),treeId);
    QCOMPARE(audit.getAuditId(),auditId);
    QCOMPARE(audit.getAuditName(),QString("TESTAUDIT"));

    PCx_Audit::deleteAudit(auditId);
    PCx_Tree::deleteTree(treeId);
}

void ProtonClassicSuiteUnitTests::testCaseForNumbersConversion()
{
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(12345.678),qint64(12345678));
    QCOMPARE(NUMBERSFORMAT::formatDouble(1234.567,-1,true),QString("1\u00A0234,57"));
    QCOMPARE(NUMBERSFORMAT::formatFixedPoint(1234567,-1,true),QString("1\u00A0234,57"));
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(12345678),12345.678);

    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM)),(double)MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*10)),(double)MAX_NUM*10);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*100)),(double)MAX_NUM*100);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*1000)),(double)MAX_NUM*1000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*10000)),(double)MAX_NUM*10000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*100000)),(double)MAX_NUM*100000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(MAX_NUM*1000000)),(double)MAX_NUM*1000000);

    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM)),(double)-MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*10)),(double)-MAX_NUM*10);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*100)),(double)-MAX_NUM*100);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*1000)),(double)-MAX_NUM*1000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*10000)),(double)-MAX_NUM*10000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*100000)),(double)-MAX_NUM*100000);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(NUMBERSFORMAT::doubleToFixedPoint(-MAX_NUM*1000000)),(double)-MAX_NUM*1000000);

    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM)),MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*10)),MAX_NUM*10);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*100)),MAX_NUM*100);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*1000)),MAX_NUM*1000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*10000)),MAX_NUM*10000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*100000)),MAX_NUM*100000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*1000000)),MAX_NUM*1000000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(MAX_NUM*10000000)),MAX_NUM*10000000);

    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM)),-MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*10)),-MAX_NUM*10);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*100)),-MAX_NUM*100);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*1000)),-MAX_NUM*1000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*10000)),-MAX_NUM*10000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*100000)),-MAX_NUM*100000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*1000000)),-MAX_NUM*1000000);
    QCOMPARE(NUMBERSFORMAT::doubleToFixedPoint(NUMBERSFORMAT::fixedPointToDouble(-MAX_NUM*10000000)),-MAX_NUM*10000000);
}


void ProtonClassicSuiteUnitTests::testCaseForAuditNodes()
{
    unsigned int treeId=PCx_Tree::addTree("TREEFORTEST");
    PCx_Tree tree(treeId);
    unsigned int nodeA=tree.addNode(1,1,"nodeA");
    unsigned int nodeB=tree.addNode(nodeA,1,"nodeB");
    tree.addNode(nodeA,1,"nodeC");
    tree.addNode(1,1,"nodeD");
    tree.addNode(nodeB,1,"nodeE");
    tree.finishTree();
    unsigned int auditId=PCx_Audit::addNewAudit("RANDOMAUDIT",QList<unsigned int>{2010,2011},treeId);

    PCx_Audit audit(auditId);
    for(unsigned int i=MODES::DFRFDIRI::DF;i<sizeof(MODES::DFRFDIRI);i++)
    {
        for(int j=2010;j<=2011;j++)
        {
            QCOMPARE(audit.getNodesWithAllNullValues((MODES::DFRFDIRI)i,j).count(),6);
            QCOMPARE(audit.getNodesWithAllZeroValues((MODES::DFRFDIRI)i,j).count(),0);
            QCOMPARE(audit.getNodesWithNonNullValues((MODES::DFRFDIRI)i,j).count(),0);
        }
    }
    unsigned int firstLeaf=audit.getAttachedTree()->getLeavesId().first();
    unsigned int lastLeaf=audit.getAttachedTree()->getLeavesId().last();

    audit.setLeafValues(firstLeaf,MODES::DFRFDIRI::DF,2010,QMap<PCx_Audit::ORED,double>{{PCx_Audit::ORED::OUVERTS,100.4},
                                                                                         {PCx_Audit::ORED::REALISES,50.3},{PCx_Audit::ORED::ENGAGES,10.0}});
    audit.setLeafValues(lastLeaf,MODES::DFRFDIRI::DF,2010,QMap<PCx_Audit::ORED,double>{{PCx_Audit::ORED::OUVERTS,200.6},
                                                                                        {PCx_Audit::ORED::REALISES,60.2},{PCx_Audit::ORED::ENGAGES,20.0}});

    audit.setLeafValues(firstLeaf,MODES::DFRFDIRI::DF,2011,QMap<PCx_Audit::ORED,double>{{PCx_Audit::ORED::OUVERTS,(double)MAX_NUM},
                                                                                        {PCx_Audit::ORED::REALISES,(double)MAX_NUM},{PCx_Audit::ORED::ENGAGES,(double)MAX_NUM}});
    audit.setLeafValues(lastLeaf,MODES::DFRFDIRI::DF,2011,QMap<PCx_Audit::ORED,double>{{PCx_Audit::ORED::OUVERTS,(double)-MAX_NUM},
                                                                                        {PCx_Audit::ORED::REALISES,(double)-MAX_NUM},{PCx_Audit::ORED::ENGAGES,(double)-MAX_NUM}});

    //Test correct set and get
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::OUVERTS,2010)),100.4);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::REALISES,2010)),50.3);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::ENGAGES,2010)),10.0);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::OUVERTS,2011)),(double)MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(lastLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::OUVERTS,2011)),(double)-MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::REALISES,2011)),(double)MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(lastLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::REALISES,2011)),(double)-MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::ENGAGES,2011)),(double)MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(lastLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::ENGAGES,2011)),(double)-MAX_NUM);

    //Test disponible computation
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::DISPONIBLES,2010)),100.4-(50.3+10.0));
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(firstLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::DISPONIBLES,2011)),(double)-MAX_NUM);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(lastLeaf,MODES::DFRFDIRI::DF,PCx_Audit::ORED::DISPONIBLES,2011)),(double)MAX_NUM);
    //Test propagation to the root
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::OUVERTS,2010)),100.4+200.6);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::REALISES,2010)),50.3+60.2);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::ENGAGES,2010)),30.0);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::DISPONIBLES,2010)),160.5);

    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::OUVERTS,2011)),0.0);
    QCOMPARE(NUMBERSFORMAT::fixedPointToDouble(audit.getNodeValue(1,MODES::DFRFDIRI::DF,PCx_Audit::ORED::DISPONIBLES,2011)),0.0);


    QString stats=audit.getHTMLAuditStatistics();
    //Removes the first three lines as they contains dynamic creation date/time
    QStringList statsSplitted=stats.split("\n");
    statsSplitted.removeFirst();
    statsSplitted.removeFirst();
    statsSplitted.removeFirst();

    stats=statsSplitted.join("\n");
    stats=stats.simplified();


    QByteArray expectedStats="df2257644478406d063b186531e7564c8bccda1e3c8f9cede2e4852cb3a39776";
    QByteArray hash=hashString(stats);


    QCOMPARE(hash.toHex(),expectedStats);


    audit.clearAllData(MODES::DFRFDIRI::DF);
    audit.clearAllData(MODES::DFRFDIRI::RF);
    audit.clearAllData(MODES::DFRFDIRI::DI);
    audit.clearAllData(MODES::DFRFDIRI::RI);
    for(unsigned int i=MODES::DFRFDIRI::DF;i<sizeof(MODES::DFRFDIRI);i++)
    {
        for(int j=2010;j<=2011;j++)
        {
            QCOMPARE(audit.getNodesWithAllNullValues((MODES::DFRFDIRI)i,j).count(),6);
            QCOMPARE(audit.getNodesWithAllZeroValues((MODES::DFRFDIRI)i,j).count(),0);
            QCOMPARE(audit.getNodesWithNonNullValues((MODES::DFRFDIRI)i,j).count(),0);
        }
    }

    audit.fillWithRandomData(MODES::DFRFDIRI::DF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::DI,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RI,false);
    for(unsigned int i=MODES::DFRFDIRI::DF;i<sizeof(MODES::DFRFDIRI);i++)
    {
        for(int j=2010;j<=2011;j++)
        {
            QCOMPARE(audit.getNodesWithAllNullValues((MODES::DFRFDIRI)i,j).count(),0);
            QCOMPARE(audit.getNodesWithAllZeroValues((MODES::DFRFDIRI)i,j).count(),0);
            QCOMPARE(audit.getNodesWithNonNullValues((MODES::DFRFDIRI)i,j).count(),6);
        }
    }

    PCx_Audit::deleteAudit(auditId);
    PCx_Tree::deleteTree(treeId);
}

void ProtonClassicSuiteUnitTests::testCaseForAuditDuplication()
{
    //QSKIP("Skipped");
    unsigned int treeId=PCx_Tree::createRandomTree("RANDOMTREE",20);
    PCx_Tree tree(treeId);
    tree.finishTree();
    unsigned int auditId=PCx_Audit::addNewAudit("RANDOMAUDIT",QList<unsigned int>{2010,2011},treeId);

    PCx_Audit audit(auditId);

    audit.fillWithRandomData(MODES::DFRFDIRI::DF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::DI,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RI,false);

    unsigned int dupAuditId=audit.duplicateAudit("DUPAUDIT",QList<unsigned int>{2009,2010,2011,2012});
    PCx_Audit dupAudit(dupAuditId);
    QVERIFY(PCx_Audit::auditNameExists("DUPAUDIT"));

    QVERIFY(!dupAudit.isFinished());
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DF,2009).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RF,2009).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DI,2009).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RI,2009).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DF,2010).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RF,2010).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DI,2010).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RI,2010).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DF,2011).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RF,2011).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DI,2011).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RI,2011).count(),0);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DF,2012).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RF,2012).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::DI,2012).count(),20);
    QCOMPARE(dupAudit.getNodesWithAllNullValues(MODES::DFRFDIRI::RI,2012).count(),20);


    QList<unsigned int> nodes=tree.getNodesId();

    qint64 val1,val2;
    foreach(unsigned int node,nodes)
    {
        for(unsigned int mode=0;mode<sizeof(MODES::DFRFDIRI);mode++)
        {
            for(unsigned int ored=0;ored<sizeof(PCx_Audit::ORED);ored++)
            {
                for(int year=2010;year<=2011;year++)
                {
                    val1=audit.getNodeValue(node,(MODES::DFRFDIRI)mode,(PCx_Audit::ORED)ored,year);
                    val2=dupAudit.getNodeValue(node,(MODES::DFRFDIRI)mode,(PCx_Audit::ORED)ored,year);
                    QCOMPARE(val1,val2);
                }
            }
        }
    }
    PCx_Audit::deleteAudit(dupAuditId);
    PCx_Audit::deleteAudit(auditId);
    PCx_Tree::deleteTree(treeId);
}

void ProtonClassicSuiteUnitTests::testCaseForAuditImportExport()
{
    //QSKIP("Skipped");
    unsigned int treeId=PCx_Tree::createRandomTree("RANDOMTREE",30);
    PCx_Tree tree(treeId);
    tree.finishTree();
    unsigned int auditId=PCx_Audit::addNewAudit("RANDOMAUDIT",QList<unsigned int>{2010,2011},treeId);
    unsigned int importedAuditId=PCx_Audit::addNewAudit("IMPORTEDAUDIT",QList<unsigned int>{2010,2011},treeId);

    PCx_Audit audit(auditId);
    PCx_Audit importedAudit(importedAuditId);


    QString tmpFileNameSkel1=QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+generateUniqueFileName(".xlsx");
    audit.exportLeavesSkeleton(tmpFileNameSkel1);
    importedAudit.importDataFromXLSX(tmpFileNameSkel1,MODES::DFRFDIRI::DF);
    QCOMPARE(importedAudit.getNodesWithAllZeroValues(MODES::DFRFDIRI::DF,2010).count(),30);
    QCOMPARE(importedAudit.getNodesWithAllZeroValues(MODES::DFRFDIRI::DF,2011).count(),30);
    QFile::remove(tmpFileNameSkel1);
    importedAudit.clearAllData(MODES::DFRFDIRI::DF);

    audit.fillWithRandomData(MODES::DFRFDIRI::DF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RF,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::DI,false);
    audit.fillWithRandomData(MODES::DFRFDIRI::RI,false);

    QString tmpFileNameDF=QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+generateUniqueFileName(".xlsx");
    QString tmpFileNameRF=QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+generateUniqueFileName(".xlsx");
    QString tmpFileNameDI=QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+generateUniqueFileName(".xlsx");
    QString tmpFileNameRI=QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+generateUniqueFileName(".xlsx");

    audit.exportLeavesDataXLSX(MODES::DFRFDIRI::DF,tmpFileNameDF);
    audit.exportLeavesDataXLSX(MODES::DFRFDIRI::RF,tmpFileNameRF);
    audit.exportLeavesDataXLSX(MODES::DFRFDIRI::DI,tmpFileNameDI);
    audit.exportLeavesDataXLSX(MODES::DFRFDIRI::RI,tmpFileNameRI);

    importedAudit.importDataFromXLSX(tmpFileNameDF,MODES::DFRFDIRI::DF);
    importedAudit.importDataFromXLSX(tmpFileNameRF,MODES::DFRFDIRI::RF);
    importedAudit.importDataFromXLSX(tmpFileNameDI,MODES::DFRFDIRI::DI);
    importedAudit.importDataFromXLSX(tmpFileNameRI,MODES::DFRFDIRI::RI);


    QList<unsigned int> nodes=tree.getNodesId();

    qint64 val1,val2;
    foreach(unsigned int node,nodes)
    {
        for(unsigned int mode=0;mode<sizeof(MODES::DFRFDIRI);mode++)
        {
            for(unsigned int ored=0;ored<sizeof(PCx_Audit::ORED);ored++)
            {
                for(int year=2010;year<=2011;year++)
                {
                    val1=audit.getNodeValue(node,(MODES::DFRFDIRI)mode,(PCx_Audit::ORED)ored,year);
                    val2=importedAudit.getNodeValue(node,(MODES::DFRFDIRI)mode,(PCx_Audit::ORED)ored,year);
                    QCOMPARE(val1,val2);
                }
            }
        }
    }

    PCx_Audit::deleteAudit(auditId);
    PCx_Audit::deleteAudit(importedAuditId);
    PCx_Tree::deleteTree(treeId);
    QFile::remove(tmpFileNameDF);
    QFile::remove(tmpFileNameRF);
    QFile::remove(tmpFileNameDI);
    QFile::remove(tmpFileNameRI);
}
