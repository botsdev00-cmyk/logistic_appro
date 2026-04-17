#include "TableauStockLocation.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QFont>

TableauStockLocation::TableauStockLocation(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent), m_gestionnaire(gestionnaire)
{
    qDebug() << "[TABLEAU STOCK LOCATION] Initialisation";
    initializeUI();
    chargerDonnees();
}

TableauStockLocation::~TableauStockLocation()
{
}

void TableauStockLocation::initializeUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_table = new QTableWidget();
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "Produit", "WAREHOUSE", "IN_TRANSIT", "RETURNED", "TOTAL", "Statut"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setColumnWidth(0, 200);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 80);
    m_table->setColumnWidth(5, 100);

    layout->addWidget(m_table);
    setLayout(layout);
}

void TableauStockLocation::chargerDonnees()
{
    qDebug() << "[TABLEAU STOCK LOCATION] Chargement des données";

    if (!m_gestionnaire) {
        qWarning() << "[TABLEAU STOCK LOCATION] ❌ Gestionnaire non initialisé!";
        return;
    }

    auto stocks = m_gestionnaire->obtenirTousStocksParLocation();
    qDebug() << "[TABLEAU STOCK LOCATION] Stocks chargés:" << stocks.count();
    
    remplirTableau(stocks);
}

void TableauStockLocation::remplirTableau(const QList<StockParLocation>& stocks)
{
    m_table->setRowCount(0);

    qDebug() << "[TABLEAU STOCK LOCATION] Remplissage du tableau avec" << stocks.count() << "produits";

    for (int i = 0; i < stocks.count(); ++i) {
        const auto& stock = stocks[i];

        m_table->insertRow(i);

        // Produit
        QTableWidgetItem* itemProduit = new QTableWidgetItem(stock.produitNom);
        itemProduit->setFlags(itemProduit->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 0, itemProduit);

        // WAREHOUSE
        QTableWidgetItem* itemWH = new QTableWidgetItem(QString::number(stock.warehouse));
        itemWH->setTextAlignment(Qt::AlignCenter);
        itemWH->setBackground(stock.warehouse > 0 ? QColor(200, 230, 201) : QColor(255, 205, 210));
        itemWH->setFlags(itemWH->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, itemWH);

        // IN_TRANSIT
        QTableWidgetItem* itemIT = new QTableWidgetItem(QString::number(stock.inTransit));
        itemIT->setTextAlignment(Qt::AlignCenter);
        itemIT->setBackground(stock.inTransit > 0 ? QColor(255, 243, 224) : QColor(255, 205, 210));
        itemIT->setFlags(itemIT->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 2, itemIT);

        // RETURNED
        QTableWidgetItem* itemRet = new QTableWidgetItem(QString::number(stock.returned));
        itemRet->setTextAlignment(Qt::AlignCenter);
        itemRet->setBackground(stock.returned > 0 ? QColor(200, 212, 255) : QColor(255, 205, 210));
        itemRet->setFlags(itemRet->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 3, itemRet);

        // TOTAL
        QTableWidgetItem* itemTotal = new QTableWidgetItem(QString::number(stock.total));
        itemTotal->setTextAlignment(Qt::AlignCenter);
        itemTotal->setFont(QFont("Arial", 10, QFont::Bold));
        itemTotal->setFlags(itemTotal->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 4, itemTotal);

        // Statut
        QString statut = stock.total > 0 ? "✓ OK" : "⚠️ VIDE";
        QTableWidgetItem* itemStatut = new QTableWidgetItem(statut);
        itemStatut->setTextAlignment(Qt::AlignCenter);
        itemStatut->setFlags(itemStatut->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 5, itemStatut);
    }

    m_table->resizeColumnsToContents();
    qDebug() << "[TABLEAU STOCK LOCATION] ✓ Tableau rempli";
}