#include "TableauStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

TableauStock::TableauStock(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent),
      m_gestionnaire(gestionnaire)
{
    qDebug() << "[TABLEAU STOCK] Initialisation";
    initializeUI();
}

TableauStock::~TableauStock()
{
}

void TableauStock::initializeUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_table = new QTableWidget();
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "Produit", "SKU", "Catégorie", "Quantité", "Réservée", 
        "Disponible", "Prix Moyen", "Statut", "Actions"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    layout->addWidget(m_table);
    setLayout(layout);
}

void TableauStock::chargerDonnees()
{
    qDebug() << "[TABLEAU STOCK] Chargement des données";

    m_donneesCourantes = m_gestionnaire->obtenirTousLesStocks();
    remplirTableau(m_donneesCourantes);
}

void TableauStock::remplirTableau(const QList<StockInfo>& stocks)
{
    m_table->setRowCount(0);

    for (int i = 0; i < stocks.count(); ++i) {
        const auto& stock = stocks[i];

        m_table->insertRow(i);

        // Colonne: Produit
        QTableWidgetItem* itemProduit = new QTableWidgetItem(stock.produitNom);
        m_table->setItem(i, 0, itemProduit);

        // Colonne: SKU
        QTableWidgetItem* itemSKU = new QTableWidgetItem(stock.codeSKU);
        m_table->setItem(i, 1, itemSKU);

        // Colonne: Catégorie
        QTableWidgetItem* itemCategorie = new QTableWidgetItem(stock.categorie);
        m_table->setItem(i, 2, itemCategorie);

        // Colonne: Quantité totale
        QTableWidgetItem* itemTotal = new QTableWidgetItem(QString::number(stock.quantiteTotal));
        itemTotal->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 3, itemTotal);

        // Colonne: Quantité réservée
        QTableWidgetItem* itemReservee = new QTableWidgetItem(QString::number(stock.quantiteReservee));
        itemReservee->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 4, itemReservee);

        // Colonne: Quantité disponible
        QTableWidgetItem* itemDisponible = new QTableWidgetItem(QString::number(stock.quantiteDisponible));
        itemDisponible->setTextAlignment(Qt::AlignCenter);
        itemDisponible->setFont(QFont("Arial", 10, QFont::Bold));
        m_table->setItem(i, 5, itemDisponible);

        // Colonne: Prix moyen
        QTableWidgetItem* itemPrix = new QTableWidgetItem(QString::number(stock.prixMoyen, 'f', 2) + " €");
        itemPrix->setTextAlignment(Qt::AlignRight);
        m_table->setItem(i, 6, itemPrix);

        // Colonne: Statut
        QTableWidgetItem* itemStatut = new QTableWidgetItem(stock.statut);
        itemStatut->setTextAlignment(Qt::AlignCenter);
        itemStatut->setBackground(obtenirCouleurStatut(stock.statut));
        m_table->setItem(i, 7, itemStatut);

        // Colonne: Actions
        QWidget* widgetActions = new QWidget();
        QHBoxLayout* layoutActions = new QHBoxLayout(widgetActions);
        layoutActions->setContentsMargins(2, 2, 2, 2);

        QPushButton* btnDetail = new QPushButton("📄 Détail");
        btnDetail->setMaximumWidth(80);
        connect(btnDetail, &QPushButton::clicked, this, [this, i]() { onAfficherDetail(i); });

        QPushButton* btnHistorique = new QPushButton("📊 Historique");
        btnHistorique->setMaximumWidth(100);
        connect(btnHistorique, &QPushButton::clicked, this, [this, i]() { onAfficherHistorique(i); });

        layoutActions->addWidget(btnDetail);
        layoutActions->addWidget(btnHistorique);
        layoutActions->addStretch();

        m_table->setCellWidget(i, 8, widgetActions);
    }

    // Redimensionner les colonnes
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);

    qDebug() << "[TABLEAU STOCK] Affichage de" << stocks.count() << "produits";
}

void TableauStock::filtrer(const QString& critere)
{
    qDebug() << "[TABLEAU STOCK] Filtrage par critère:" << critere;

    auto filtered = m_gestionnaire->rechercherStocks(critere);
    remplirTableau(filtered);
}

void TableauStock::filtrerParStatut(const QString& statut)
{
    qDebug() << "[TABLEAU STOCK] Filtrage par statut:" << statut;

    QList<StockInfo> filtered;

    if (statut == "Tous les statuts") {
        filtered = m_donneesCourantes;
    } else {
        for (const auto& stock : m_donneesCourantes) {
            if (stock.statut == statut) {
                filtered.append(stock);
            }
        }
    }

    remplirTableau(filtered);
}

void TableauStock::onAfficherDetail(int row)
{
    const auto& stock = m_donneesCourantes[row];

    QString detail = QString(
        "DÉTAIL PRODUIT\n\n"
        "Nom: %1\n"
        "SKU: %2\n"
        "Catégorie: %3\n"
        "Type: %4\n\n"
        "STOCK\n"
        "Quantité totale: %5\n"
        "Quantité réservée: %6\n"
        "Quantité disponible: %7\n"
        "Stock minimum: %8\n\n"
        "VALEUR\n"
        "Prix moyen: %9 €\n"
        "Valeur stock: %10 €\n\n"
        "STATUT: %11"
    ).arg(stock.produitNom, stock.codeSKU, stock.categorie, stock.type)
     .arg(stock.quantiteTotal).arg(stock.quantiteReservee).arg(stock.quantiteDisponible)
     .arg(stock.stockMinimum).arg(stock.prixMoyen, 0, 'f', 2).arg(stock.valeurStock, 0, 'f', 2)
     .arg(stock.statut);

    QMessageBox::information(this, "Détail Stock", detail);
}

void TableauStock::onModifierStockMinimum(int row)
{
    // TODO: Implémenter modification du stock minimum
    QMessageBox::information(this, "Modification", "Modification du stock minimum");
}

void TableauStock::onAfficherHistorique(int row)
{
    const auto& stock = m_donneesCourantes[row];
    qDebug() << "[TABLEAU STOCK] Historique du produit:" << stock.produitNom;

    auto mouvements = m_gestionnaire->obtenirHistoriqueProduit(stock.produitId);

    QString historique = QString("HISTORIQUE - %1 (%2 mouvements)\n\n").arg(stock.produitNom).arg(mouvements.count());

    for (const auto& mvt : mouvements) {
        historique += QString("• %1 | Type: %2 | Raison: %3 | Qté: %4 | Par: %5\n")
            .arg(mvt.dateCreation, mvt.type, mvt.raison)
            .arg(mvt.quantiteDelta).arg(mvt.nomUtilisateur);
    }

    QMessageBox::information(this, "Historique", historique);
}

QColor TableauStock::obtenirCouleurStatut(const QString& statut)
{
    if (statut == "RUPTURE") return QColor(255, 100, 100); // Rouge
    if (statut == "CRITIQUE") return QColor(255, 193, 7);  // Orange
    if (statut == "FAIBLE") return QColor(255, 235, 59);   // Jaune
    return QColor(76, 175, 80);                            // Vert
}