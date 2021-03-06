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

#include "formauditexplore.h"
#include "ui_formauditexplore.h"
#include "utils.h"
#include <QScrollBar>
#include <QPrintDialog>
#include <QFileDialog>

//#include "QCustomPlot/qcpdocumentobject.h"

FormAuditExplore::FormAuditExplore(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormAuditExplore)
{
    model=nullptr;
    ready=false;
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1,1);

    doc=new QTextDocument();
    ui->textEdit->setDocument(doc);
    //NOTE : For vectorized graphics
    //interface = new QCPDocumentObject(this);
    //ui->textEdit->document()->documentLayout()->registerHandler(QCPDocumentObject::PlotTextFormat, interface);
    updateListOfAudits();
    referenceNode=1;
}

FormAuditExplore::~FormAuditExplore()
{
    delete doc;
    if(model!=nullptr)
    {
        delete model;
        delete report;
    }
    delete ui;
}

void FormAuditExplore::onListOfAuditsChanged()
{
    updateListOfAudits();
}

void FormAuditExplore::onSettingsChanged()
{
    if(model==nullptr)
        return;
    if(ui->treeView->currentIndex().isValid())
    {
        on_treeView_clicked(ui->treeView->currentIndex());
    }
    else
    {
        QModelIndex rootIndex=model->getAttachedTree()->index(0,0);
        on_treeView_clicked(rootIndex);
    }
}


void FormAuditExplore::updateListOfAudits()
{
    ui->comboListAudits->clear();

    QList<QPair<unsigned int,QString> >listOfAudits=PCx_Audit::getListOfAudits(PCx_Audit::ListAuditsMode::FinishedAuditsOnly);
    //do not update text browser if no audit are available

    if(listOfAudits.isEmpty())
    {
        QMessageBox::information(this,tr("Information"),tr("Créez et terminez un audit dans la fenêtre de gestion des audits"));
    }


    bool nonEmpty=!listOfAudits.isEmpty();
    this->setEnabled(nonEmpty);
    doc->setHtml(tr("<h1 align='center'><br><br><br><br><br>Remplissez un audit et n'oubliez pas de le terminer</h1>"));

    ready=nonEmpty;
    QPair<unsigned int, QString> p;
    foreach(p,listOfAudits)
    {
        ui->comboListAudits->insertItem(0,p.second,p.first);
    }
    ui->comboListAudits->setCurrentIndex(0);
    on_comboListAudits_activated(0);
}


void FormAuditExplore::updateTextBrowser()
{
    ui->saveButton->setEnabled(ready);
    if(!ready)
    {
        doc->setHtml(tr("<h1 align='center'><br><br><br><br><br>Remplissez un audit et n'oubliez pas de le terminer</h1>"));
        return;
    }

    unsigned int selectedNode=ui->treeView->selectionModel()->currentIndex().data(PCx_TreeModel::NodeIdUserRole).toUInt();

    getSelections();
    QScrollBar *sb=ui->textEdit->verticalScrollBar();
    int sbval=sb->value();

    doc->clear();

    QSettings settings;
    report->getGraphics().setGraphicsWidth(settings.value("graphics/width",PCx_Graphics::DEFAULTWIDTH).toInt());
    report->getGraphics().setGraphicsHeight(settings.value("graphics/height",PCx_Graphics::DEFAULTHEIGHT).toInt());

    //Always scale 1.0 when displaying
    report->getGraphics().setScale(1.0);

    QString output=model->generateHTMLHeader();
    output.append(model->generateHTMLAuditTitle());
    output.append(report->generateHTMLAuditReportForNode(selectedTabs,QList<PCx_Tables::PCATABLES>(),selectedGraphics,selectedNode,selectedMode,referenceNode,doc));
    output.append("</body></html>");
    doc->setHtml(output);
    sb->setValue(sbval);
}


void FormAuditExplore::getSelections()
{
    selectedMode=MODES::DFRFDIRI::GLOBAL;

    if(ui->radioButtonDF->isChecked())
        selectedMode=MODES::DFRFDIRI::DF;

    else if(ui->radioButtonRF->isChecked())
        selectedMode=MODES::DFRFDIRI::RF;

    else if(ui->radioButtonDI->isChecked())
        selectedMode=MODES::DFRFDIRI::DI;

    else if(ui->radioButtonRI->isChecked())
        selectedMode=MODES::DFRFDIRI::RI;

    else if(ui->radioButtonGlobal->isChecked())
        selectedMode=MODES::DFRFDIRI::GLOBAL;

    else
    {
        qCritical()<<"Unsupported case of radio button checking";
    }

    selectedGraphics.clear();
    selectedTabs.clear();

    if(selectedMode!=MODES::DFRFDIRI::GLOBAL)
    {
        if(ui->checkBoxPoidsRelatif->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCAOVERVIEW);

        if(ui->checkBoxEvolution->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCAEVOLUTION);

        if(ui->checkBoxEvolutionCumul->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCAEVOLUTIONCUMUL);

        if(ui->checkBoxBase100->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCABASE100);

        if(ui->checkBoxJoursAct->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCADAYOFWORK);

        if(ui->checkBoxRawHistoryData->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAHISTORY);

        if(ui->checkBoxOuvert->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG1);

        if(ui->checkBoxOuvertCumul->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG2);

        if(ui->checkBoxRealise->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG3);

        if(ui->checkBoxRealiseCumul->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG4);

        if(ui->checkBoxEngage->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG5);

        if(ui->checkBoxEngageCumul->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG6);

        if(ui->checkBoxDisponible->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG7);

        if(ui->checkBoxDisponibleCumul->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG8);

    }

    else
    {
        if(ui->checkBoxResults->isChecked())
            selectedTabs.append(PCx_Tables::PCAPRESETS::PCARESULTS);

        if(ui->checkBoxRecapGraph->isChecked())
            selectedGraphics.append(PCx_Graphics::PCAGRAPHICS::PCAG9);
    }
}

void FormAuditExplore::on_comboListAudits_activated(int index)
{
    if(index==-1||ui->comboListAudits->count()==0)return;
    unsigned int selectedAuditId=ui->comboListAudits->currentData().toUInt();
    if(!(selectedAuditId>0))
    {
        qFatal("Assertion failed");
    }
    //qDebug()<<"Selected audit ID = "<<selectedAuditId;

    if(model!=nullptr)
    {
        delete model;
        delete report;
    }

    referenceNode=1;

    model=new PCx_AuditWithTreeModel(selectedAuditId);
    report=new PCx_Report(model);

    QItemSelectionModel *m=ui->treeView->selectionModel();
    ui->treeView->setModel(model->getAttachedTree());
    delete m;
    ui->treeView->expandToDepth(1);
    QModelIndex rootIndex=model->getAttachedTree()->index(0,0);
    ui->treeView->setCurrentIndex(rootIndex);
    on_treeView_clicked(rootIndex);
    updateTextBrowser();
}

void FormAuditExplore::on_treeView_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
   // ui->groupBoxMode->setTitle(index.data().toString());

    updateTextBrowser();
}

void FormAuditExplore::on_radioButtonGlobal_toggled(bool checked)
{
    if(checked)
    {
        selectedMode=MODES::DFRFDIRI::GLOBAL;
        updateTextBrowser();

        ui->checkBoxPoidsRelatif->setEnabled(false);
        ui->checkBoxBase100->setEnabled(false);
        ui->checkBoxEvolution->setEnabled(false);
        ui->checkBoxEvolutionCumul->setEnabled(false);
        ui->checkBoxJoursAct->setEnabled(false);
        ui->checkBoxResults->setEnabled(true);

        ui->checkBoxRawHistoryData->setEnabled(false);
        ui->checkBoxOuvert->setEnabled(false);
        ui->checkBoxOuvertCumul->setEnabled(false);
        ui->checkBoxRealise->setEnabled(false);
        ui->checkBoxRealiseCumul->setEnabled(false);
        ui->checkBoxEngage->setEnabled(false);
        ui->checkBoxEngageCumul->setEnabled(false);
        ui->checkBoxDisponible->setEnabled(false);
        ui->checkBoxDisponibleCumul->setEnabled(false);
        ui->checkBoxRecapGraph->setEnabled(true);
    }
    else
    {
        ui->checkBoxPoidsRelatif->setEnabled(true);
        ui->checkBoxBase100->setEnabled(true);
        ui->checkBoxEvolution->setEnabled(true);
        ui->checkBoxEvolutionCumul->setEnabled(true);
        ui->checkBoxJoursAct->setEnabled(true);
        ui->checkBoxResults->setEnabled(false);

        ui->checkBoxRawHistoryData->setEnabled(true);
        ui->checkBoxOuvert->setEnabled(true);
        ui->checkBoxOuvertCumul->setEnabled(true);
        ui->checkBoxRealise->setEnabled(true);
        ui->checkBoxRealiseCumul->setEnabled(true);
        ui->checkBoxEngage->setEnabled(true);
        ui->checkBoxEngageCumul->setEnabled(true);
        ui->checkBoxDisponible->setEnabled(true);
        ui->checkBoxDisponibleCumul->setEnabled(true);
        ui->checkBoxRecapGraph->setEnabled(false);
    }
}


void FormAuditExplore::on_saveButton_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QString fileName = fileDialog.getSaveFileName(this, tr("Enregistrer en HTML"), "",tr("Fichiers HTML (*.html *.htm)"));
    if(fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if(fi.suffix().compare("html",Qt::CaseInsensitive)!=0 && fi.suffix().compare("htm",Qt::CaseInsensitive)!=0)
        fileName.append(".html");
    fi=QFileInfo(fileName);


    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Attention"),tr("Ouverture du fichier impossible : %1").arg(file.errorString()));
        return;
    }
    //Will reopen after computation
    file.close();
    file.remove();

    QString relativeImagePath=fi.fileName()+"_files";
    QString absoluteImagePath=fi.absoluteFilePath()+"_files";

    QFileInfo imageDirInfo(absoluteImagePath);

    if(!imageDirInfo.exists())
    {
        if(!fi.absoluteDir().mkdir(relativeImagePath))
        {
            QMessageBox::critical(this,tr("Attention"),tr("Création du dossier des images impossible"));
            return;
        }
    }
    else
    {
        if(!imageDirInfo.isWritable())
        {
            QMessageBox::critical(this,tr("Attention"),tr("Ecriture impossible dans le dossier des images"));
            return;
        }
    }

    unsigned int node=ui->treeView->selectionModel()->currentIndex().data(PCx_TreeModel::NodeIdUserRole).toUInt();
    getSelections();

    int maximumProgressValue=selectedGraphics.count();

    QProgressDialog progress(tr("Enregistrement en cours..."),0,0,maximumProgressValue);
    progress.setMinimumDuration(1000);

    progress.setWindowModality(Qt::ApplicationModal);
    progress.setValue(0);

    QSettings settings;

    report->getGraphics().setGraphicsWidth(settings.value("graphics/width",PCx_Graphics::DEFAULTWIDTH).toInt());
    report->getGraphics().setGraphicsHeight(settings.value("graphics/height",PCx_Graphics::DEFAULTHEIGHT).toInt());
    report->getGraphics().setScale(settings.value("graphics/scale",PCx_Graphics::DEFAULTSCALE).toDouble());

    //Generate report in non-embedded mode, saving images
    QString output=model->generateHTMLHeader();
    output.append(model->generateHTMLAuditTitle());
    output.append(report->generateHTMLAuditReportForNode(selectedTabs,QList<PCx_Tables::PCATABLES>(),selectedGraphics,node,selectedMode,referenceNode,nullptr,absoluteImagePath,relativeImagePath,&progress));
    output.append("</body></html>");


    QString settingStyle=settings.value("output/style","CSS").toString();
    if(settingStyle=="INLINE")
    {
        //Pass HTML through a temp QTextDocument to reinject css into tags (more compatible with text editors)
        QTextDocument formattedOut;
        formattedOut.setHtml(output);
        output=formattedOut.toHtml("utf-8");

        //Cleanup the output a bit
        output.replace(" -qt-block-indent:0;","");
    }

    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Attention"),tr("Ouverture du fichier impossible : %1").arg(file.errorString()));
        QDir dir(absoluteImagePath);
        dir.removeRecursively();
        return;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream<<output;
    stream.flush();
    file.close();
    progress.setValue(maximumProgressValue);
    if(stream.status()==QTextStream::Ok)
        QMessageBox::information(this,tr("Information"),tr("Le document <b>%1</b> a bien été enregistré. Les images sont stockées dans le dossier <b>%2</b>").arg(fi.fileName().toHtmlEscaped(),relativeImagePath.toHtmlEscaped()));
    else
        QMessageBox::critical(this,tr("Attention"),tr("Le document n'a pas pu être enregistré !"));

}

void FormAuditExplore::on_checkBoxResults_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxRecapGraph_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxOuvert_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxEngage_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxOuvertCumul_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxEngageCumul_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxRealise_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxDisponible_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxRealiseCumul_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxDisponibleCumul_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxPoidsRelatif_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_radioButtonDF_toggled(bool checked)
{
    if(checked)
    {
        updateTextBrowser();
    }
}

void FormAuditExplore::on_radioButtonRF_toggled(bool checked)
{
    if(checked)
    {
        updateTextBrowser();
    }

}

void FormAuditExplore::on_radioButtonDI_toggled(bool checked)
{
    if(checked)
    {
        updateTextBrowser();
    }
}

void FormAuditExplore::on_radioButtonRI_toggled(bool checked)
{
    if(checked)
    {
        updateTextBrowser();
    }
}

void FormAuditExplore::on_checkBoxEvolution_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxEvolutionCumul_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxBase100_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_checkBoxJoursAct_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}

void FormAuditExplore::on_pushButtonCollapseAll_clicked()
{
    ui->treeView->collapseAll();
    ui->treeView->expandToDepth(0);
}

void FormAuditExplore::on_pushButtonExpandAll_clicked()
{
    ui->treeView->expandAll();
}

void FormAuditExplore::on_checkBoxRawHistoryData_toggled(bool checked)
{
    Q_UNUSED(checked);
    updateTextBrowser();
}


void FormAuditExplore::on_treeView_doubleClicked(const QModelIndex &index)
{
    referenceNode=index.data(PCx_TreeModel::NodeIdUserRole).toUInt();
    QMessageBox::information(this,"Information",tr("Nouveau noeud de référence pour les calculs : %1").arg(model->getAttachedTree()->getNodeName(referenceNode).toHtmlEscaped()));
    updateTextBrowser();
}
