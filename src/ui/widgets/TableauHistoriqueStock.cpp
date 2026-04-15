#include "TableauHistoriqueStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>

TableauHistoriqueStock::TableauHistoriqueStock(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent),
      m_gestionnaire(gestionnaire)
{
    qDebug() << "[TABLEAU HISTORIQUE] Initialisation";
    initializeUI();
}

TableauHistoriqueStock::~TableauHistoriqueStock()
{
}

void TableauHistoriqueStock::initializeUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // ====== FILTERS ======
    QHBoxLayout* filterLayout = new QHBoxLayout();

    m_filterType = new QComboBox();
    m_filterType->addItem("Tous les types");
    m_filterType->addItem("ENTREE");
    m_filterType->addItem("SORTIE");
    m_filterType->addItem("RETOUR");

    m_dateFrom = new QDateEdit();
    m_dateFrom->setDate(QDate::currentDate().addDays(-30));
    m_dateFrom->setCalendarPopup(true);

    m_dateTo = new QDateEdit();
    m_dateTo->setDate(QDate::currentDate());
    m_dateTo->setCalendarPopup(true);

    QPushButton* btnFiltrer = new QPushButton("🔍 Filtrer");
    connect(btnFiltrer, &QPushButton::clicked, this, &TableauHistoriqueStock::onFiltrerParDate);

    filterLayout->addWidget(new QLabel("Type:"));
    filterLayout->addWidget(m_filterType);
    filterLayout->addWidget(new QLabel("Du:"));
    filterLayout->addWidget(m_dateFrom);
    filterLayout->addWidget(new QLabel("Au:"));
    filterLayout->addWidget(m_dateTo);
    filterLayout->addWidget(btnFiltrer);
    filterLayout->addStretch();

    layout->addLayout(filterLayout);

    // ====== TABLE ======
    m_table = new QTableWidget();
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Type", "Produit", "Raison", "Quantité", "Avant", "Après", "Utilisateur", "Date"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    layout->addWidget(m_table);
    setLayout(layout);
}

void TableauHistoriqueStock::chargerDonnees()
{
    qDebug() << "[TABLEAU HISTORIQUE] Chargement des données";

    m_donneesCourantes = m_gestionnaire->obtenirMouvementsRecents();
    remplirTableau(m_donneesCourantes);
}

void TableauHistoriqueStock::remplirTableau(const QList<Mouvement>& mouvements)
{
    m_table->setRowCount(0);

    for (int i = 0; i < mouvements.count(); ++i) {
        const auto& mvt = mouvements[i];

        m_table->insertRow(i);

        // Type
        QTableWidgetItem* itemType = new QTableWidgetItem(mvt.type);
        itemType->setTextAlignment(Qt::AlignCenter);
        QColor couleur = (mvt.type == "ENTREE") ? QColor(76, 175, 80) :
                        (mvt.type == "SORTIE") ? QColor(33, 150, 243) :
                        QColor(255, 152, 0);
        itemType->setBackground(couleur);
        itemType->setForeground(Qt::white);
        m_table->setItem(i, 0, itemType);

        // Produit
        m_table->setItem(i, 1, new QTableWidgetItem(mvt.nomUtilisateur)); // TODO: Récupérer nomProduit

        // Raison
        m_table->setItem(i, 2, new QTableWidgetItem(mvt.raison));

        // Quantité
        QTableWidgetItem* itemQte = new QTableWidgetItem(QString::number(mvt.quantiteDelta));
        itemQte->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 3, itemQte);

        // Avant
        // TODO: Implémenter quantitéAvant

        // Après
        QTableWidgetItem* itemApres = new QTableWidgetItem(QString::number(mvt.quantiteApres));
        itemApres->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 5, itemApres);

        // Utilisateur
        m_table->setItem(i, 6, new QTableWidgetItem(mvt.nomUtilisateur));

        // Date
        m_table->setItem(i, 7, new QTableWidgetItem(mvt.dateCreation));
    }

    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);

    qDebug() << "[TABLEAU HISTORIQUE] Affichage de" << mouvements.count() << "mouvements";
}

void TableauHistoriqueStock::onFiltrerParType(int index)
{
    QString type = m_filterType->currentText();
    qDebug() << "[TABLEAU HISTORIQUE] Filtrage par type:" << type;

    QList<Mouvement> filtered;

    if (type == "Tous les types") {
        filtered = m_donneesCourantes;
    } else {
        for (const auto& mvt : m_donneesCourantes) {
            if (mvt.type == type) {
                filtered.append(mvt);
            }
        }
    }

    remplirTableau(filtered);
}

void TableauHistoriqueStock::onFiltrerParDate()
{
    qDebug() << "[TABLEAU HISTORIQUE] Filtrage par date";
    // TODO: Implémenter filtrage par date
    chargerDonnees();
}