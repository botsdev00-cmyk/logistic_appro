#include "TableauCredit.h"
#include "../../business/managers/GestionnaireCredit.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QDate>

TableauCredit::TableauCredit(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireCredit>())
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauCredit::~TableauCredit()
{
}

void TableauCredit::initialiserColonnes()
{
    setColumnCount(7);
    setHorizontalHeaderLabels({"Client", "Montant", "Date d'échéance", "Jours retard", "Statut", "Date paiement", "Actions"});
    
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}

void TableauCredit::chargerDonnees()
{
    afficherTousLesCredits();
}

void TableauCredit::rafraichir()
{
    clearContents();
    setRowCount(0);
    chargerDonnees();
}

void TableauCredit::remplirTableau()
{
    // Données fictives
    insertRow(0);
    setItem(0, 0, new QTableWidgetItem("Supermarché Nord"));
    setItem(0, 1, new QTableWidgetItem("50000 FCFA"));
    setItem(0, 2, new QTableWidgetItem("2026-04-15"));
    setItem(0, 3, new QTableWidgetItem("0"));
    setItem(0, 4, new QTableWidgetItem("En attente"));
    setItem(0, 5, new QTableWidgetItem("-"));
    setItem(0, 6, new QTableWidgetItem("Payer | Détails"));
    
    insertRow(1);
    setItem(1, 0, new QTableWidgetItem("Distributeur Principal"));
    setItem(1, 1, new QTableWidgetItem("150000 FCFA"));
    setItem(1, 2, new QTableWidgetItem("2026-04-03"));
    setItem(1, 3, new QTableWidgetItem("7"));
    setItem(1, 4, new QTableWidgetItem("En retard"));
    setItem(1, 5, new QTableWidgetItem("-"));
    setItem(1, 6, new QTableWidgetItem("Payer | Détails"));
    
    mettreEnEvidence();
}

void TableauCredit::afficherCreditsEnRetard()
{
    for (int row = 0; row < rowCount(); ++row) {
        QString statut = item(row, 4)->text();
        bool visible = (statut == "En retard");
        setRowHidden(row, !visible);
    }
}

void TableauCredit::afficherTousLesCredits()
{
    remplirTableau();
}

void TableauCredit::mettreEnEvidence()
{
    for (int row = 0; row < rowCount(); ++row) {
        QString statut = item(row, 4)->text();
        
        if (statut == "En retard") {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#f8d7da"));
                item(row, col)->setForeground(QColor("#721c24"));
            }
        } else if (statut == "En attente") {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#fff3cd"));
            }
        }
    }
}

void TableauCredit::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauCredit::afficherDetailsCredit);
        menu.addAction("Marquer comme payé", this, &TableauCredit::marquerCommePayé);
        menu.addAction("Envoyer rappel", this, &TableauCredit::envoyerRappel);
        menu.addSeparator();
        menu.addAction("Exporter CSV", this, &TableauCredit::exporterEnCSV);
        menu.exec(mapToGlobal(pos));
    });
}

void TableauCredit::afficherDetailsCredit()
{
    if (currentRow() >= 0) {
        QString client = item(currentRow(), 0)->text();
        qDebug() << "Détails crédit:" << client;
    }
}

void TableauCredit::marquerCommePayé()
{
    if (currentRow() >= 0) {
        item(currentRow(), 4)->setText("Payé");
        qDebug() << "Crédit marqué comme payé";
    }
}

void TableauCredit::envoyerRappel()
{
    if (currentRow() >= 0) {
        QString client = item(currentRow(), 0)->text();
        qDebug() << "Rappel envoyé à:" << client;
    }
}

void TableauCredit::exporterEnCSV()
{
    qDebug() << "Export CSV crédits";
}