#include "TableauVentes.h"
#include "../../business/managers/GestionnaireSales.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>

TableauVentes::TableauVentes(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireSales>()),
      m_totalVentes(0.0),
      m_totalCash(0.0),
      m_totalCredit(0.0)
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauVentes::~TableauVentes()
{
}

void TableauVentes::initialiserColonnes()
{
    setColumnCount(7);
    setHorizontalHeaderLabels({"Produit", "Client", "Quantité", "Prix unitaire", "Type", "Montant", "Date"});
    
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}

void TableauVentes::chargerDonnees()
{
    remplirTableau();
}

void TableauVentes::rafraichir()
{
    clearContents();
    setRowCount(0);
    m_totalVentes = 0.0;
    m_totalCash = 0.0;
    m_totalCredit = 0.0;
    remplirTableau();
}

void TableauVentes::remplirTableau()
{
    // Données fictives
    insertRow(0);
    setItem(0, 0, new QTableWidgetItem("Vin Rouge Shiraz"));
    setItem(0, 1, new QTableWidgetItem("Boutique du Centre"));
    setItem(0, 2, new QTableWidgetItem("40"));
    setItem(0, 3, new QTableWidgetItem("12.50 FCFA"));
    setItem(0, 4, new QTableWidgetItem("Cash"));
    setItem(0, 5, new QTableWidgetItem("500.00 FCFA"));
    setItem(0, 6, new QTableWidgetItem("2026-04-10"));
    
    insertRow(1);
    setItem(1, 0, new QTableWidgetItem("Bière Blonde"));
    setItem(1, 1, new QTableWidgetItem("Supermarché Nord"));
    setItem(1, 2, new QTableWidgetItem("100"));
    setItem(1, 3, new QTableWidgetItem("3.50 FCFA"));
    setItem(1, 4, new QTableWidgetItem("Crédit"));
    setItem(1, 5, new QTableWidgetItem("350.00 FCFA"));
    setItem(1, 6, new QTableWidgetItem("2026-04-10"));
    
    calculerTotaux();
}

void TableauVentes::filtrerParClient(const QString& client)
{
    for (int row = 0; row < rowCount(); ++row) {
        QString itemClient = item(row, 1)->text();
        bool visible = client.isEmpty() || itemClient.contains(client, Qt::CaseInsensitive);
        setRowHidden(row, !visible);
    }
}

void TableauVentes::filtrerParType(const QString& type)
{
    for (int row = 0; row < rowCount(); ++row) {
        QString itemType = item(row, 4)->text();
        bool visible = type.isEmpty() || itemType.contains(type, Qt::CaseInsensitive);
        setRowHidden(row, !visible);
    }
}

void TableauVentes::calculerTotaux()
{
    m_totalVentes = 0.0;
    m_totalCash = 0.0;
    m_totalCredit = 0.0;
    
    for (int row = 0; row < rowCount(); ++row) {
        QString montantStr = item(row, 5)->text().replace(" FCFA", "");
        double montant = montantStr.toDouble();
        m_totalVentes += montant;
        
        QString type = item(row, 4)->text();
        if (type == "Cash") {
            m_totalCash += montant;
        } else {
            m_totalCredit += montant;
        }
    }
    
    qDebug() << "Total ventes:" << m_totalVentes;
    qDebug() << "Total cash:" << m_totalCash;
    qDebug() << "Total crédit:" << m_totalCredit;
}

void TableauVentes::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauVentes::afficherDetailsVente);
        menu.addSeparator();
        menu.addAction("Imprimer facture", this, &TableauVentes::imprimer);
        menu.addAction("Exporter CSV", this, &TableauVentes::exporterEnCSV);
        menu.exec(mapToGlobal(pos));
    });
}

void TableauVentes::afficherDetailsVente()
{
    if (currentRow() >= 0) {
        QString produit = item(currentRow(), 0)->text();
        qDebug() << "Détails vente:" << produit;
    }
}

void TableauVentes::imprimer()
{
    qDebug() << "Imprimer facture";
}

void TableauVentes::exporterEnCSV()
{
    qDebug() << "Export CSV ventes";
}