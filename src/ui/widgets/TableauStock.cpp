#include "TableauStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDebug>

TableauStock::TableauStock(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireStock>())
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauStock::~TableauStock()
{
}

void TableauStock::initialiserColonnes()
{
    setColumnCount(6);
    setHorizontalHeaderLabels({"Produit", "Catégorie", "Stock actuel", "Stock minimum", "État", "Statut"});
    
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}

void TableauStock::chargerDonnees()
{
    remplirTableau();
}

void TableauStock::rafraichir()
{
    clearContents();
    setRowCount(0);
    remplirTableau();
}

void TableauStock::remplirTableau()
{
    // À implémenter avec les données de GestionnaireStock
    
    // Exemple de données fictives
    insertRow(0);
    setItem(0, 0, new QTableWidgetItem("Vin Rouge Shiraz"));
    setItem(0, 1, new QTableWidgetItem("Vin Rouge"));
    setItem(0, 2, new QTableWidgetItem("150"));
    setItem(0, 3, new QTableWidgetItem("50"));
    setItem(0, 4, new QTableWidgetItem("OK"));
    setItem(0, 5, new QTableWidgetItem("Actif"));
    
    insertRow(1);
    setItem(1, 0, new QTableWidgetItem("Bière Blonde"));
    setItem(1, 1, new QTableWidgetItem("Bière"));
    setItem(1, 2, new QTableWidgetItem("15"));
    setItem(1, 3, new QTableWidgetItem("30"));
    setItem(1, 4, new QTableWidgetItem("⚠ Bas"));
    setItem(1, 5, new QTableWidgetItem("Alerte"));
    
    mettreEnEvidence();
}

void TableauStock::mettreEnEvidence()
{
    for (int row = 0; row < rowCount(); ++row) {
        QString statut = item(row, 4)->text();
        
        if (statut.contains("Bas")) {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#fff3cd"));
            }
        } else if (statut.contains("Rupture")) {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#f8d7da"));
            }
        }
    }
}

void TableauStock::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauStock::afficherDetailsStock);
        menu.addAction("Exporter CSV", this, &TableauStock::exporterEnCSV);
        menu.exec(mapToGlobal(pos));
    });
}

void TableauStock::afficherDetailsStock()
{
    if (currentRow() >= 0) {
        QString produit = item(currentRow(), 0)->text();
        qDebug() << "Affichage détails pour:" << produit;
    }
}

void TableauStock::exporterEnCSV()
{
    // À implémenter
    qDebug() << "Export CSV";
}