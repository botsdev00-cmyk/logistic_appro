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
#include <QFont>

TableauHistoriqueStock::TableauHistoriqueStock(GestionnaireStock* gestionnaire, QWidget* parent)
    : QWidget(parent),
      m_gestionnaire(gestionnaire)
{
    qDebug() << "[TABLEAU HISTORIQUE] Initialisation";
    initializeUI();
    chargerDonnees();  // ✅ Charger les données au démarrage
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
    connect(m_filterType, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TableauHistoriqueStock::onFiltrerParType);

    m_dateFrom = new QDateEdit();
    m_dateFrom->setDate(QDate::currentDate().addDays(-30));
    m_dateFrom->setCalendarPopup(true);

    m_dateTo = new QDateEdit();
    m_dateTo->setDate(QDate::currentDate());
    m_dateTo->setCalendarPopup(true);

    QPushButton* btnFiltrer = new QPushButton("🔍 Filtrer par date");
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
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "Type", "Produit", "SKU", "Raison", "Quantité", "Utilisateur", "Date", "Source"
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

    if (!m_gestionnaire) {
        qWarning() << "[TABLEAU HISTORIQUE] ❌ Gestionnaire non initialisé!";
        return;
    }

    m_donneesCourantes = m_gestionnaire->obtenirMouvementsRecents();
    qDebug() << "[TABLEAU HISTORIQUE] Mouvements chargés:" << m_donneesCourantes.count();
    
    remplirTableau(m_donneesCourantes);
}

void TableauHistoriqueStock::remplirTableau(const QList<Mouvement>& mouvements)
{
    m_table->setRowCount(0);

    qDebug() << "[TABLEAU HISTORIQUE] Remplissage du tableau avec" << mouvements.count() << "mouvements";

    for (int i = 0; i < mouvements.count(); ++i) {
        const auto& mvt = mouvements[i];

        m_table->insertRow(i);

        // ✅ Type avec couleur
        QTableWidgetItem* itemType = new QTableWidgetItem(mvt.type);
        itemType->setTextAlignment(Qt::AlignCenter);
        itemType->setFont(QFont("Arial", 10, QFont::Bold));
        
        QColor couleur = (mvt.type == "ENTREE") ? QColor(76, 175, 80) :
                        (mvt.type == "SORTIE") ? QColor(33, 150, 243) :
                        QColor(255, 152, 0);  // RETOUR
        itemType->setBackground(couleur);
        itemType->setForeground(Qt::white);
        m_table->setItem(i, 0, itemType);

        // ✅ CORRECTION: Produit (pas Utilisateur!)
        QTableWidgetItem* itemProduit = new QTableWidgetItem(mvt.nomProduit);
        itemProduit->setFlags(itemProduit->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, itemProduit);

        // ✅ SKU
        QTableWidgetItem* itemSKU = new QTableWidgetItem(mvt.codeSKU);
        itemSKU->setFlags(itemSKU->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 2, itemSKU);

        // ✅ Raison
        QTableWidgetItem* itemRaison = new QTableWidgetItem(mvt.raison);
        itemRaison->setFlags(itemRaison->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 3, itemRaison);

        // ✅ Quantité
        QTableWidgetItem* itemQte = new QTableWidgetItem(QString::number(mvt.quantiteDelta));
        itemQte->setTextAlignment(Qt::AlignCenter);
        itemQte->setFont(QFont("Arial", 10, QFont::Bold));
        itemQte->setFlags(itemQte->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 4, itemQte);

        // ✅ Utilisateur
        QTableWidgetItem* itemUtilisateur = new QTableWidgetItem(mvt.nomUtilisateur);
        itemUtilisateur->setFlags(itemUtilisateur->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 5, itemUtilisateur);

        // ✅ Date
        QTableWidgetItem* itemDate = new QTableWidgetItem(mvt.dateCreation);
        itemDate->setTextAlignment(Qt::AlignCenter);
        itemDate->setFlags(itemDate->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 6, itemDate);

        // ✅ Source
        QTableWidgetItem* itemSource = new QTableWidgetItem(mvt.source);
        itemSource->setFlags(itemSource->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 7, itemSource);
        
        qDebug() << "[MOUVEMENT]" << mvt.type << "-" << mvt.nomProduit 
                 << "Qté:" << mvt.quantiteDelta << "par" << mvt.nomUtilisateur;
    }

    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);

    qDebug() << "[TABLEAU HISTORIQUE] ✓ Tableau rempli avec" << mouvements.count() << "mouvements";
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
    qDebug() << "[TABLEAU HISTORIQUE] Filtrage par date:" 
             << m_dateFrom->date() << "à" << m_dateTo->date();
    
    QDate dateDebut = m_dateFrom->date();
    QDate dateFin = m_dateTo->date();
    
    auto mouvements = m_gestionnaire->obtenirMouvementsParDateRange(dateDebut, dateFin);
    remplirTableau(mouvements);
}