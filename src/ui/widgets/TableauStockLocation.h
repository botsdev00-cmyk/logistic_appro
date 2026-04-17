#ifndef TABLEAUSTOCKLOCATION_H
#define TABLEAUSTOCKLOCATION_H

#include <QWidget>
#include <QList>
#include "../../business/managers/GestionnaireStock.h"

class QTableWidget;

class TableauStockLocation : public QWidget
{
    Q_OBJECT

public:
    explicit TableauStockLocation(GestionnaireStock* gestionnaire, QWidget* parent = nullptr);
    ~TableauStockLocation();

    void chargerDonnees();

private:
    void initializeUI();
    void remplirTableau(const QList<StockParLocation>& stocks);

    GestionnaireStock* m_gestionnaire;
    QTableWidget* m_table;
};

#endif // TABLEAUSTOCKLOCATION_H