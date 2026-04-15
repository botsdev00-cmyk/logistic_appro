#ifndef TABLEAUHISTORIQUESTOCK_H
#define TABLEAUHISTORIQUESTOCK_H

#include <QWidget>
#include <QList>
#include "../../business/managers/GestionnaireStock.h"

class QTableWidget;
class QComboBox;
class QDateEdit;

class TableauHistoriqueStock : public QWidget
{
    Q_OBJECT

public:
    explicit TableauHistoriqueStock(GestionnaireStock* gestionnaire, QWidget* parent = nullptr);
    ~TableauHistoriqueStock();

    void chargerDonnees();

private slots:
    void onFiltrerParType(int index);
    void onFiltrerParDate();

private:
    void initializeUI();
    void remplirTableau(const QList<Mouvement>& mouvements);

    GestionnaireStock* m_gestionnaire;
    QTableWidget* m_table;
    QComboBox* m_filterType;
    QDateEdit* m_dateFrom;
    QDateEdit* m_dateTo;
    QList<Mouvement> m_donneesCourantes;
};

#endif // TABLEAUHISTORIQUESTOCK_H