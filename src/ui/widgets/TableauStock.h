#ifndef TABLEAUSTOCK_H
#define TABLEAUSTOCK_H

#include <QWidget>
#include <QUuid>
#include <QList>
#include "../../business/managers/GestionnaireStock.h"  // ✅ AJOUT: Pour StockInfo

class QTableWidget;
class QTableWidgetItem;
class QPushButton;

class TableauStock : public QWidget
{
    Q_OBJECT

public:
    explicit TableauStock(GestionnaireStock* gestionnaire, QWidget* parent = nullptr);
    ~TableauStock();

    void chargerDonnees();
    void filtrer(const QString& critere);
    void filtrerParStatut(const QString& statut);

private slots:
    void onAfficherDetail(int row);
    void onModifierStockMinimum(int row);
    void onAfficherHistorique(int row);

private:
    void initializeUI();
    void remplirTableau(const QList<StockInfo>& stocks);
    QColor obtenirCouleurStatut(const QString& statut);

    GestionnaireStock* m_gestionnaire;
    QTableWidget* m_table;
    QList<StockInfo> m_donneesCourantes;
};

#endif // TABLEAUSTOCK_H