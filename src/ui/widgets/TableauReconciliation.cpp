#include "TableauReconciliation.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QFont>
#include <QColor>

TableauReconciliation::TableauReconciliation(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent), m_gestionnaire(gestionnaire)
{
    qDebug() << "[TABLEAU RECONCILIATION] Initialisation";
    initializeUI();
    chargerDonnees();
}

TableauReconciliation::~TableauReconciliation()
{
}

void TableauReconciliation::initializeUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_table = new QTableWidget();
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "Produit", "Stock Mouvements", "Stock Soldes", "Différence", "Statut", "Action"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setColumnWidth(0, 200);
    m_table->setColumnWidth(1, 130);
    m_table->setColumnWidth(2, 130);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 80);
    m_table->setColumnWidth(5, 100);

    layout->addWidget(m_table);
    setLayout(layout);
}

void TableauReconciliation::chargerDonnees()
{
    qDebug() << "[TABLEAU RECONCILIATION] Chargement des données";

    if (!m_gestionnaire) {
        qWarning() << "[TABLEAU RECONCILIATION] ❌ Gestionnaire non initialisé!";
        return;
    }

    auto resultats = m_gestionnaire->verifierIntegriteStock();
    qDebug() << "[TABLEAU RECONCILIATION] Résultats chargés:" << resultats.count();
    
    remplirTableau(resultats);
}

void TableauReconciliation::remplirTableau(const QList<ReconciliationResult>& resultats)
{
    m_table->setRowCount(0);

    qDebug() << "[TABLEAU RECONCILIATION] Remplissage du tableau avec" << resultats.count() << "produits";

    int ok = 0, erreurs = 0;

    for (int i = 0; i < resultats.count(); ++i) {
        const auto& res = resultats[i];

        if (res.isConsistent) {
            ok++;
        } else {
            erreurs++;
        }

        m_table->insertRow(i);

        // Produit
        QTableWidgetItem* itemProduit = new QTableWidgetItem(res.produitNom);
        itemProduit->setFlags(itemProduit->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 0, itemProduit);

        // Stock Mouvements
        QTableWidgetItem* itemMvt = new QTableWidgetItem(QString::number(res.stockFromMovements));
        itemMvt->setTextAlignment(Qt::AlignCenter);
        itemMvt->setFlags(itemMvt->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, itemMvt);

        // Stock Soldes
        QTableWidgetItem* itemSolde = new QTableWidgetItem(QString::number(res.stockInSoldes));
        itemSolde->setTextAlignment(Qt::AlignCenter);
        itemSolde->setFlags(itemSolde->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 2, itemSolde);

        // Différence
        QTableWidgetItem* itemDiff = new QTableWidgetItem(QString::number(res.difference));
        itemDiff->setTextAlignment(Qt::AlignCenter);
        itemDiff->setFont(QFont("Arial", 10, QFont::Bold));
        
        if (res.isConsistent) {
            itemDiff->setBackground(QColor(200, 230, 201)); // Vert
            itemDiff->setForeground(QColor(0, 128, 0));
        } else {
            itemDiff->setBackground(QColor(255, 205, 210)); // Rouge
            itemDiff->setForeground(QColor(192, 0, 0));
        }
        
        itemDiff->setFlags(itemDiff->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 3, itemDiff);

        // Statut
        QString statut = res.isConsistent ? "✓ OK" : "❌ ERREUR";
        QTableWidgetItem* itemStatut = new QTableWidgetItem(statut);
        itemStatut->setTextAlignment(Qt::AlignCenter);
        
        if (res.isConsistent) {
            itemStatut->setBackground(QColor(200, 230, 201));
        } else {
            itemStatut->setBackground(QColor(255, 205, 210));
        }
        
        itemStatut->setFlags(itemStatut->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 4, itemStatut);

        // Action
        QString action = res.isConsistent ? "—" : "Réparer";
        QTableWidgetItem* itemAction = new QTableWidgetItem(action);
        itemAction->setTextAlignment(Qt::AlignCenter);
        itemAction->setFlags(itemAction->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 5, itemAction);
    }

    m_table->resizeColumnsToContents();
    qDebug() << "[TABLEAU RECONCILIATION] ✓ Tableau rempli - OK:" << ok << "Erreurs:" << erreurs;
}