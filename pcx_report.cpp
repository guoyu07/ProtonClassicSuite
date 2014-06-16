#include "pcx_report.h"
#include "utils.h"

PCx_Report::PCx_Report(PCx_AuditModel *model)
{
    this->model=model;
    tables=new PCx_Tables(model);
    graphics=new PCx_Graphics(model);
}

PCx_Report::~PCx_Report()
{
    delete tables;
    delete graphics;
}

QString PCx_Report::getCSS() const
{
    //Original PCA stylesheet with slight incoherencies between t2, t2bis, t3 and t3bis
    QString css=
            "\nbody{font-family:sans-serif;font-size:9pt;background-color:white;color:black;}"
            "\n.req1{background-color:#FAB;padding:5px;}"
            "\n.req1normal{background-color:#FAB;}"
            "\n.req2{background-color:#BFA;padding:5px;}"
            "\n.req2normal{background-color:#BFA;}"
            "\n.req3{background-color:#BAF;padding:5px;}"
            "\n.req3normal{background-color:#BAF;}"
            "\nh1{color:#A00;}"
            "\nh2{color:navy;}"
            "\nh3{color:green;font-size:larger;}"
            "\ndiv.g{margin-left:auto;margin-right:auto;page-break-inside:avoid;}"
            "\ntable{color:navy;font-weight:400;font-size:8pt;page-break-inside:avoid;}"
            "\ntd.t1annee,td.t3annee{background-color:#b3b3b3;}"
            "\ntd.t1pourcent{background-color:#b3b3b3;color:#FFF;}"
            "\ntd.t1valeur,td.t2valeur,td.t3valeur,td.t4annee,td.t4valeur,td.t4pourcent,td.t5annee,td.t7annee{background-color:#e6e6e6;}"
            "\ntd.t2annee{background-color:#b3b3b3;color:green;}"
            "\ntd.t2pourcent,td.t3pourcent{background-color:#e6e6e6;color:#000;}"
            "\ntd.t5pourcent,td.t6pourcent{background-color:#b3b3b3;color:#000;}"
            "\ntd.t5valeur,td.t6valeur,td.t7valeur{background-color:#b3b3b3;color:#000;font-weight:400;}"
            "\ntd.t6annee{background-color:#e6e6e6;color:green;}"
            "\ntd.t7pourcent{background-color:#666;color:#FFF;}"
            "\ntd.t8pourcent{background-color:#e6e6e6;text-align:center;color:#000;}"
            "\ntd.t8valeur{background-color:#e6e6e6;font-weight:400;text-align:center;color:#000;}"
            "\ntd.t9pourcent{background-color:#666;text-align:center;color:#FFF;}"
            "\ntd.t9valeur{background-color:#666;color:#FFF;font-weight:400;text-align:center;}"
            "\ntr.t1entete,tr.t3entete,td.t9annee{background-color:#b3b3b3;text-align:center;}"
            "\ntr.t2entete,td.t8annee{background-color:#b3b3b3;color:green;text-align:center;}"
            "\ntr.t4entete,tr.t5entete,tr.t7entete,tr.t9entete{background-color:#e6e6e6;text-align:center;}"
            "\ntr.t6entete{background-color:#e6e6e6;color:green;text-align:center;}"
            "\ntr.t8entete{background-color:#e6e6e6;text-align:center;color:green;}\n";

    return css;
}


QString PCx_Report::generateHTMLHeader() const
{
    return QString("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n<html>\n<head><title>Audit %1</title>\n<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>\n<style type='text/css'>\n%2\n</style>\n</head>\n<body>\n"
                   "<h3>Audit %1</h3>\n").arg(model->getAuditInfos().name.toHtmlEscaped()).arg(getCSS());

}

QString PCx_Report::generateHTMLReportForNode(quint8 bitFieldPagesOfTables, quint16 bitFieldTables, quint16 bitFieldGraphics, unsigned int selectedNode, PCx_AuditModel::DFRFDIRI mode,
                                                  QCustomPlot *plot, unsigned int favoriteGraphicsWidth, unsigned int favoriteGraphicsHeight,double scale,QTextDocument *document,
                                                  const QString &absoluteImagePath, const QString &relativeImagePath,QProgressDialog *progress) const
{
    Q_ASSERT(selectedNode>0 && plot!=NULL);

    QString output;

    //Either group of tables, or individual tables
    if(bitFieldPagesOfTables!=0)
    {
        if(bitFieldPagesOfTables & PCx_Tables::TABRECAP)
            output.append(tables->getTabRecap(selectedNode,mode));

        if(bitFieldPagesOfTables & PCx_Tables::TABEVOLUTION)
            output.append(tables->getTabEvolution(selectedNode,mode));

        if(bitFieldPagesOfTables & PCx_Tables::TABEVOLUTIONCUMUL)
            output.append(tables->getTabEvolutionCumul(selectedNode,mode));

        if(bitFieldPagesOfTables & PCx_Tables::TABBASE100)
            output.append(tables->getTabBase100(selectedNode,mode));

        if(bitFieldPagesOfTables & PCx_Tables::TABJOURSACT)
            output.append(tables->getTabJoursAct(selectedNode,mode));

        if(bitFieldPagesOfTables & PCx_Tables::TABRESULTS)
            output.append(tables->getTabResults(selectedNode));
    }

    else if(bitFieldTables!=0)
    {
        if(bitFieldTables & PCx_Tables::T1)
            output.append(tables->getT1(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T2)
            output.append(tables->getT2(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T2BIS)
            output.append(tables->getT2bis(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T3)
            output.append(tables->getT3(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T3BIS)
            output.append(tables->getT3bis(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T4)
            output.append(tables->getT4(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T5)
            output.append(tables->getT5(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T6)
            output.append(tables->getT6(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T7)
            output.append(tables->getT7(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T8)
            output.append(tables->getT8(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T9)
            output.append(tables->getT9(selectedNode,mode)+"<br>");
        if(bitFieldTables & PCx_Tables::T10)
            output.append(tables->getT10(selectedNode)+"<br>");
        if(bitFieldTables & PCx_Tables::T11)
            output.append(tables->getT11(selectedNode)+"<br>");
        if(bitFieldTables & PCx_Tables::T12)
            output.append(tables->getT12(selectedNode)+"<br>");
    }

    if(bitFieldGraphics!=0)
    {
        //Graphics, a little too verbose
        //getGx draw the plot in the hidden QCustomPlot widget, which can be exported to pixmap and inserted into html with <img>

        //inline mode
        if(document!=NULL)
        {
            if(bitFieldGraphics & PCx_Graphics::G1)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG1(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G1' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));

            }
            if(bitFieldGraphics & PCx_Graphics::G2)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG2(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & PCx_Graphics::G3)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG3(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G3' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & PCx_Graphics::G4)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG4(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G4' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & PCx_Graphics::G5)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG5(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G5' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & PCx_Graphics::G6)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG6(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G6' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & PCx_Graphics::G7)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG7(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G7' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
            if(bitFieldGraphics & PCx_Graphics::G8)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG8(selectedNode,mode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' alt='G8' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }

            if(bitFieldGraphics & PCx_Graphics::G9)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG9(selectedNode,plot)+"</b><br>");
                QString name="mydata://"+QString::number(qrand());
                document->addResource(QTextDocument::ImageResource,QUrl(name),QVariant(plot->toPixmap(favoriteGraphicsWidth,favoriteGraphicsHeight,scale)));
                output.append(QString("<img width='%1' height='%2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(name));
            }
        }

        //Linked images mode. Images are saved into imagePath
        else
        {

            QSettings settings;
            QString imageFormat=settings.value("output/imageFormat","png").toString();
            const char *imgFormat="png";
            int quality=-1;
            if(imageFormat=="png")
            {
                imgFormat="png";
                quality=-1;
            }

            //FIXME : save to JPG quality 100 as a workaround because saving to PNG is very slow
            else if(imageFormat=="jpg")
            {
                imgFormat="jpeg";
                quality=96;
            }
            else
            {
                qCritical()<<"Invalid image format";
                die();
            }
            QString suffix="."+imageFormat;

            int progressValue=0;
            if(progress!=NULL)
            {
                progressValue=progress->value();
            }

            if(absoluteImagePath.isEmpty()|| relativeImagePath.isEmpty())
            {
                qCritical()<<"Please pass an absolute and relative path to store images";
                return QString();
            }

            if(bitFieldGraphics & PCx_Graphics::G1)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG1(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G1' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & PCx_Graphics::G2)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG2(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G2' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }


            if(bitFieldGraphics & PCx_Graphics::G3)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG3(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G3' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));

                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & PCx_Graphics::G4)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG4(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G4' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            QCoreApplication::processEvents( QEventLoop::AllEvents,100);

            if(bitFieldGraphics & PCx_Graphics::G5)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG5(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G5' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }


            if(bitFieldGraphics & PCx_Graphics::G6)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG6(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G6' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & PCx_Graphics::G7)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG7(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G7' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & PCx_Graphics::G8)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG8(selectedNode,mode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G8' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }

            if(bitFieldGraphics & PCx_Graphics::G9)
            {
                output.append("<div align='center' class='g'><b>"+graphics->getG9(selectedNode,plot)+"</b><br>");
                QString imageName=generateUniqueFileName(suffix);
                QString imageAbsoluteName=imageName;
                imageName.prepend(relativeImagePath+"/");
                imageAbsoluteName.prepend(absoluteImagePath+"/");

                if(!plot->saveRastered(imageAbsoluteName,favoriteGraphicsWidth,favoriteGraphicsHeight,scale,imgFormat,quality))
                {
                    qCritical()<<"Unable to save "<<imageAbsoluteName;
                    die();
                }
                output.append(QString("<img width='%1' height='%2' alt='G9' src='%3'></div><br>")
                              .arg(favoriteGraphicsWidth).arg(favoriteGraphicsHeight).arg(imageName));
                if(progress!=NULL)
                    progress->setValue(++progressValue);
            }
        }
    }

    //NOTE : For vectorized graphics
    //cursor.insertText(QString(QChar::ObjectReplacementCharacter), QCPDocumentObject::generatePlotFormat(ui->plot, 600, 400));

    return output;
}

