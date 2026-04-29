#ifndef TABLEAU_RETOURS_EN_ATTENTE_H
#define TABLEAU_RETOURS_EN_ATTENTE_H

#include <QTableWidget>
#include <QList>
#include "../../core/entities/RetourStock.h"

class TableauRetoursEnAttente : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableauRetoursEnAttente(QWidget* parent = nullptr);

    void setRetoursEnAttente(const QList<RetourStock>& retours);

signals:
    void ligneRetourClicked(const RetourStock& retour);
};

#endif // TABLEAU_RETOURS_EN_ATTENTE_H