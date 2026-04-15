#include "PanelAlertes.h"
#include "../../business/managers/GestionnaireStock.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>

PanelAlertes::PanelAlertes(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent),
      m_gestionnaire(gestionnaire)
{
    qDebug() << "[PANEL ALERTES] Initialisation";
    initializeUI();
    chargerDonnees();
}

PanelAlertes::~PanelAlertes()
{
}

void PanelAlertes::initializeUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_table = new QTableWidget();
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({
        "Produit", "Quantité Disponible", "Stock Minimum", "Sévérité", "Actions"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    layout->addWidget(m_table);
    setLayout(layout);
}

void PanelAlertes::chargerDonnees()
{
    qDebug() << "[PANEL ALERTES] Chargement des alertes";

    auto alertes = m_gestionnaire->obtenirAlertes();
    remplirTableau(alertes);
}

void PanelAlertes::remplirTableau(const QList<Alerte>& alertes)
{
    m_table->setRowCount(0);

    for (int i = 0; i < alertes.count(); ++i) {
        const auto& alerte = alertes[i];

        m_table->insertRow(i);

        // Produit
        m_table->setItem(i, 0, new QTableWidgetItem(alerte.produitNom));

        // Quantité disponible
        QTableWidgetItem* itemQte = new QTableWidgetItem(QString::number(alerte.quantiteDisponible));
        itemQte->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, itemQte);

        // Stock minimum
        QTableWidgetItem* itemMin = new QTableWidgetItem(QString::number(alerte.stockMinimum));
        itemMin->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 2, itemMin);

        // Sévérité
        QTableWidgetItem* itemSeverite = new QTableWidgetItem(alerte.severite);
        itemSeverite->setTextAlignment(Qt::AlignCenter);

        if (alerte.severite == "RUPTURE") {
            itemSeverite->setBackground(QColor(255, 100, 100)); // Rouge
            itemSeverite->setForeground(Qt::white);
        } else if (alerte.severite == "CRITICAL") {
            itemSeverite->setBackground(QColor(255, 152, 0));   // Orange
            itemSeverite->setForeground(Qt::white);
        } else {
            itemSeverite->setBackground(QColor(255, 235, 59)); // Jaune
        }

        m_table->setItem(i, 3, itemSeverite);

        // Actions
        QTableWidgetItem* itemAction = new QTableWidgetItem("📋 Voir détail");
        itemAction->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 4, itemAction);
    }

    m_table->resizeColumnsToContents();

    qDebug() << "[PANEL ALERTES] Affichage de" << alertes.count() << "alertes";
}