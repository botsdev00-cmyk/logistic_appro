#ifndef TABLEAURECONCILIATION_H
#define TABLEAURECONCILIATION_H

#include <QWidget>
#include <QList>
#include "../../business/managers/GestionnaireStock.h"

class QTableWidget;

class TableauReconciliation : public QWidget
{
    Q_OBJECT

public:
    explicit TableauReconciliation(GestionnaireStock* gestionnaire, QWidget* parent = nullptr);
    ~TableauReconciliation();

    void chargerDonnees();

private:
    void initializeUI();
    void remplirTableau(const QList<ReconciliationResult>& resultats);

    GestionnaireStock* m_gestionnaire;
    QTableWidget* m_table;
};

#endif // TABLEAURECONCILIATION_H