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
#ifndef TST_PROTONCLASSICSUITEUNITTESTS_H
#define TST_PROTONCLASSICSUITEUNITTESTS_H

#include "pcx_audit.h"
#include <QtTest>


class ProtonClassicSuiteUnitTests : public QObject
{
    Q_OBJECT

    void fillTestAudit(PCx_Audit *audit);

    QByteArray hashString(const QString &str);

public:
    ProtonClassicSuiteUnitTests();


private Q_SLOTS:
    void cleanup();
    void initTestCase();
    void cleanupTestCase();
    void testCaseForTreeManagement();
    void testCaseForTreeNodes();
    void testCaseForTreeNodesOrder();
    void testCaseForTreeDuplication();
    void testCaseForTreeImportExport();
    void testCaseForTreeToDot();

    void testCaseForNumbersConversion();

    void testCaseForAuditManagement();
    void testCaseForAuditNodes();
    void testCaseForAuditDuplication();
    void testCaseForAuditImportExport();
    void testCaseForPCATables();
    void testCaseForPCAGraphics();
    void testCaseForPCAQueries();
    void benchmarkReport();

    void testCaseForReportingManagement();
    void testCaseForReportingNodes();

    void testCaseForPCBPrevision();
    void testCaseForPCBPrevisionItemCriteria();
    void testCaseForPCBPrevisionItem();
    void testCaseForPCBPrevisionItemDispatch();
};



#endif // TST_PROTONCLASSICSUITEUNITTESTS_H

