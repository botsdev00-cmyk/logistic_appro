#ifndef PANELALERTES_H
#define PANELALERTES_H

#include <QWidget>
#include <QList>
#include "../../business/managers/GestionnaireStock.h"

class QTableWidget;

class PanelAlertes : public QWidget
{
    Q_OBJECT

public:
    explicit PanelAlertes(GestionnaireStock* gestionnaire, QWidget* parent = nullptr);
    ~PanelAlertes();

    void chargerDonnees();

private:
    void initializeUI();
    void remplirTableau(const QList<Alerte>& alertes);

    GestionnaireStock* m_gestionnaire;
    QTableWidget* m_table;
};

#endif // PANELALERTES_H