#include "TableauRepartition.h"
#include "../../business/managers/GestionnaireRepartition.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>

TableauRepartition::TableauRepartition(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireRepartition>())
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauRepartition::~TableauRepartition()
{
}

void TableauRepartition::initialiserColonnes()
{
    setColumnCount(6);
    setHorizontalHeaderLabels({"Équipe", "Route", "Date", "Montant attendu", "Statut", "Actions"});
    
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}

void TableauRepartition::chargerDonnees()
{
    remplirTableau();
}

void TableauRepartition::rafraichir()
{
    clearContents();
    setRowCount(0);
    remplirTableau();
}

void TableauRepartition::remplirTableau()
{
    // Données fictives
    insertRow(0);
    setItem(0, 0, new QTableWidgetItem("Équipe Alpha"));
    setItem(0, 1, new QTableWidgetItem("Route Nord"));
    setItem(0, 2, new QTableWidgetItem("2026-04-10"));
    setItem(0, 3, new QTableWidgetItem("5000 FCFA"));
    setItem(0, 4, new QTableWidgetItem("En cours"));
    setItem(0, 5, new QTableWidgetItem("Détails | Articles"));
    
    insertRow(1);
    setItem(1, 0, new QTableWidgetItem("Équipe Bravo"));
    setItem(1, 1, new QTableWidgetItem("Route Sud"));
    setItem(1, 2, new QTableWidgetItem("2026-04-09"));
    setItem(1, 3, new QTableWidgetItem("3500 FCFA"));
    setItem(1, 4, new QTableWidgetItem("Complétée"));
    setItem(1, 5, new QTableWidgetItem("Détails | Articles"));
    
    mettreEnEvidence();
}

void TableauRepartition::filtrerParStatut(const QString& statut)
{
    for (int row = 0; row < rowCount(); ++row) {
        QString itemStatut = item(row, 4)->text();
        bool visible = statut.isEmpty() || itemStatut.contains(statut, Qt::CaseInsensitive);
        setRowHidden(row, !visible);
    }
}

void TableauRepartition::mettreEnEvidence()
{
    for (int row = 0; row < rowCount(); ++row) {
        QString statut = item(row, 4)->text();
        
        if (statut == "En cours") {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#d1ecf1"));
            }
        } else if (statut == "Complétée") {
            for (int col = 0; col < columnCount(); ++col) {
                item(row, col)->setBackground(QColor("#d4edda"));
            }
        }
    }
}

void TableauRepartition::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauRepartition::afficherDetailsRepartition);
        menu.addAction("Afficher articles", this, &TableauRepartition::afficherArticles);
        menu.addAction("Changer statut", this, &TableauRepartition::changerStatut);
        menu.exec(mapToGlobal(pos));
    });
}

void TableauRepartition::afficherDetailsRepartition()
{
    if (currentRow() >= 0) {
        QString equipe = item(currentRow(), 0)->text();
        qDebug() << "Détails répartition:" << equipe;
    }
}

void TableauRepartition::changerStatut()
{
    if (currentRow() >= 0) {
        qDebug() << "Changer statut";
    }
}

void TableauRepartition::afficherArticles()
{
    if (currentRow() >= 0) {
        qDebug() << "Afficher articles";
    }
}