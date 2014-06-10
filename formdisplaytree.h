#ifndef FORMDISPLAYTREE_H
#define FORMDISPLAYTREE_H

#include "pcx_treemodel.h"

namespace Ui {
class FormDisplayTree;
}

class FormDisplayTree : public QWidget
{
    Q_OBJECT

public:
    explicit FormDisplayTree(PCx_TreeModel *treeModel, QWidget *parent = 0);
    ~FormDisplayTree();

private slots:
    void on_printViewButton_clicked();

private:
    Ui::FormDisplayTree *ui;
    PCx_TreeModel *model;
};

#endif // FORMDISPLAYTREE_H