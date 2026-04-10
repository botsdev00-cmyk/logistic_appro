#include "TableauClient.h"
#include "../../business/managers/GestionnaireClient.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>

TableauClient::TableauClient(QWidget* parent)
    : QTableWidget(parent),
      m_gestionnaire(std::make_unique<GestionnaireClient>())
{
    initialiserColonnes();
    creerContextMenu();
    setStyleSheet(
        "QTableWidget { background-color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }"
    );
}

TableauClient::~TableauClient()
{
}

void TableauClient::initialiserColonnes()
{
    setColumnCount(7);
    setHorizontalHeaderLabels({"Nom", "Route", "Téléphone", "Email", "Conditions paiement", "Solde crédit", "Statut"});
    
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
}

void TableauClient::chargerDonnees()
{
    remplirTableau();
}

void TableauClient::rafraichir()
{
    clearContents();
    setRowCount(0);
    remplirTableau();
}

void TableauClient::remplirTableau()
{
    // Données fictives - à remplacer par appel à GestionnaireClient
    insertRow(0);
    setItem(0, 0, new QTableWidgetItem("Boutique du Centre"));
    setItem(0, 1, new QTableWidgetItem("Route Sud"));
    setItem(0, 2, new QTableWidgetItem("+243 123 456 789"));
    setItem(0, 3, new QTableWidgetItem("info@boutique.com"));
    setItem(0, 4, new QTableWidgetItem("Cash"));
    setItem(0, 5, new QTableWidgetItem("0 FCFA"));
    setItem(0, 6, new QTableWidgetItem("Actif"));
    
    insertRow(1);
    setItem(1, 0, new QTableWidgetItem("Supermarché Nord"));
    setItem(1, 1, new QTableWidgetItem("Route Nord"));
    setItem(1, 2, new QTableWidgetItem("+243 987 654 321"));
    setItem(1, 3, new QTableWidgetItem("super@nord.com"));
    setItem(1, 4, new QTableWidgetItem("Crédit 7 jours"));
    setItem(1, 5, new QTableWidgetItem("50000 FCFA"));
    setItem(1, 6, new QTableWidgetItem("Actif"));
}

void TableauClient::filtrerParRoute(const QString& route)
{
    for (int row = 0; row < rowCount(); ++row) {
        QString itemRoute = item(row, 1)->text();
        bool visible = route.isEmpty() || itemRoute.contains(route, Qt::CaseInsensitive);
        setRowHidden(row, !visible);
    }
}

void TableauClient::rechercher(const QString& terme)
{
    for (int row = 0; row < rowCount(); ++row) {
        bool trouve = false;
        for (int col = 0; col < columnCount(); ++col) {
            if (item(row, col)->text().contains(terme, Qt::CaseInsensitive)) {
                trouve = true;
                break;
            }
        }
        setRowHidden(row, !trouve);
    }
}

void TableauClient::creerContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction("Afficher détails", this, &TableauClient::afficherDetailsClient);
        menu.addAction("Éditer", this, &TableauClient::editerClient);
        menu.addSeparator();
        menu.addAction("Supprimer", this, &TableauClient::supprimerClient);
        menu.addSeparator();
        menu.addAction("Exporter CSV", this, &TableauClient::exporterEnCSV);
        menu.exec(mapToGlobal(pos));
    });
}

void TableauClient::afficherDetailsClient()
{
    if (currentRow() >= 0) {
        QString client = item(currentRow(), 0)->text();
        qDebug() << "Affichage détails pour:" << client;
    }
}

void TableauClient::editerClient()
{
    if (currentRow() >= 0) {
        QString client = item(currentRow(), 0)->text();
        qDebug() << "Édition du client:" << client;
    }
}

void TableauClient::supprimerClient()
{
    if (currentRow() >= 0) {
        QString client = item(currentRow(), 0)->text();
        qDebug() << "Suppression du client:" << client;
    }
}

void TableauClient::exporterEnCSV()
{
    qDebug() << "Export CSV clients";
}