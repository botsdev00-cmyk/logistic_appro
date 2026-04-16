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
#include <QMessageBox>

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
    m_filterType->addItem("AJUSTEMENT");
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
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "Type", "Produit", "SKU", "Catégorie", "Quantité", "Utilisateur", "Raison", "Date", "Source"
    });

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setColumnWidth(0, 80);      // Type
    m_table->setColumnWidth(1, 150);     // Produit
    m_table->setColumnWidth(2, 80);      // SKU
    m_table->setColumnWidth(3, 100);     // Catégorie
    m_table->setColumnWidth(4, 80);      // Quantité
    m_table->setColumnWidth(5, 120);     // Utilisateur
    m_table->setColumnWidth(6, 120);     // Raison
    m_table->setColumnWidth(7, 150);     // Date
    m_table->setColumnWidth(8, 80);      // Source

    layout->addWidget(m_table);
    setLayout(layout);
    
    qDebug() << "[TABLEAU HISTORIQUE] ✓ UI initialisée";
}

void TableauHistoriqueStock::chargerDonnees()
{
    qDebug() << "[TABLEAU HISTORIQUE] Chargement des données";

    if (!m_gestionnaire) {
        qWarning() << "[TABLEAU HISTORIQUE] ❌ Gestionnaire non initialisé!";
        return;
    }

    // ✅ Récupérer les mouvements
    m_donneesCourantes = m_gestionnaire->obtenirMouvementsRecents();
    qDebug() << "[TABLEAU HISTORIQUE] Mouvements chargés:" << m_donneesCourantes.count();
    
    // ✅ Remplir le tableau
    remplirTableau(m_donneesCourantes);
}

// ============================================================================
// REMPLIR TABLEAU - LA MÉTHODE MANQUANTE!
// ============================================================================

void TableauHistoriqueStock::remplirTableau(const QList<Mouvement>& mouvements)
{
    qDebug() << "[TABLEAU HISTORIQUE] Remplissage du tableau avec" << mouvements.count() << "mouvements";

    m_table->setRowCount(0);

    if (mouvements.isEmpty()) {
        qDebug() << "[TABLEAU HISTORIQUE] ⚠️ Aucun mouvement à afficher!";
        return;
    }

    for (int i = 0; i < mouvements.count(); ++i) {
        const auto& mvt = mouvements[i];

        m_table->insertRow(i);

        // ✅ Colonne 0: Type de mouvement
        QTableWidgetItem* itemType = new QTableWidgetItem(mvt.type);
        itemType->setTextAlignment(Qt::AlignCenter);
        itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
        
        // Couleur selon type
        if (mvt.type == "ENTREE") {
            itemType->setBackground(QColor("#C8E6C9"));  // Vert
        } else if (mvt.type == "SORTIE") {
            itemType->setBackground(QColor("#FFCCBC"));  // Orange
        } else if (mvt.type == "RETOUR") {
            itemType->setBackground(QColor("#B3E5FC"));  // Bleu
        } else if (mvt.type == "AJUSTEMENT") {
            itemType->setBackground(QColor("#FFF9C4"));  // Jaune
        }
        
        m_table->setItem(i, 0, itemType);

        // ✅ Colonne 1: Nom du produit
        QTableWidgetItem* itemProduit = new QTableWidgetItem(mvt.nomProduit);
        itemProduit->setFlags(itemProduit->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, itemProduit);

        // ✅ Colonne 2: Code SKU
        QTableWidgetItem* itemSKU = new QTableWidgetItem(mvt.codeSKU);
        itemSKU->setTextAlignment(Qt::AlignCenter);
        itemSKU->setFlags(itemSKU->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 2, itemSKU);

        // ✅ Colonne 3: Catégorie (si disponible, sinon vide)
        QTableWidgetItem* itemCategorie = new QTableWidgetItem("");  // À améliorer si info disponible
        itemCategorie->setFlags(itemCategorie->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 3, itemCategorie);

        // ✅ Colonne 4: Quantité
        QTableWidgetItem* itemQuantite = new QTableWidgetItem(QString::number(mvt.quantiteDelta));
        itemQuantite->setTextAlignment(Qt::AlignCenter);
        itemQuantite->setFont(QFont("Arial", 10, QFont::Bold));
        itemQuantite->setFlags(itemQuantite->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 4, itemQuantite);

        // ✅ Colonne 5: Utilisateur
        QTableWidgetItem* itemUtilisateur = new QTableWidgetItem(mvt.nomUtilisateur);
        itemUtilisateur->setFlags(itemUtilisateur->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 5, itemUtilisateur);

        // ✅ Colonne 6: Raison
        QTableWidgetItem* itemRaison = new QTableWidgetItem(mvt.raison);
        itemRaison->setFlags(itemRaison->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 6, itemRaison);

        // ✅ Colonne 7: Date
        QTableWidgetItem* itemDate = new QTableWidgetItem(mvt.dateCreation);
        itemDate->setFlags(itemDate->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 7, itemDate);

        // ✅ Colonne 8: Source
        QTableWidgetItem* itemSource = new QTableWidgetItem(mvt.source.isEmpty() ? "—" : mvt.source);
        itemSource->setTextAlignment(Qt::AlignCenter);
        itemSource->setFlags(itemSource->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 8, itemSource);

        qDebug() << "[LIGNE" << i << "]" << mvt.type << "-" << mvt.nomProduit 
                 << "(" << mvt.codeSKU << ") - Qté:" << mvt.quantiteDelta 
                 << "- Par:" << mvt.nomUtilisateur;
    }

    // Redimensionner automatiquement
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);

    qDebug() << "[TABLEAU HISTORIQUE] ✓ Tableau rempli avec" << mouvements.count() << "mouvements";
}

// ============================================================================
// SLOTS DE FILTRAGE
// ============================================================================

void TableauHistoriqueStock::onFiltrerParType(int index)
{
    qDebug() << "[TABLEAU HISTORIQUE] Filtrage par type - Index:" << index;

    if (!m_gestionnaire) return;

    QString typeSelectionne = m_filterType->currentText();

    if (typeSelectionne == "Tous les types") {
        remplirTableau(m_donneesCourantes);
    } else {
        QList<Mouvement> filtered;
        for (const auto& mvt : m_donneesCourantes) {
            if (mvt.type == typeSelectionne) {
                filtered.append(mvt);
            }
        }
        remplirTableau(filtered);
        qDebug() << "[TABLEAU HISTORIQUE] Filtrés:" << filtered.count() << "mouvements du type" << typeSelectionne;
    }
}

void TableauHistoriqueStock::onFiltrerParDate()
{
    qDebug() << "[TABLEAU HISTORIQUE] Filtrage par date";

    if (!m_gestionnaire) return;

    QDate dateDebut = m_dateFrom->date();
    QDate dateFin = m_dateTo->date();

    qDebug() << "[TABLEAU HISTORIQUE] Filtre date:" << dateDebut.toString("dd/MM/yyyy") 
             << "→" << dateFin.toString("dd/MM/yyyy");

    // Récupérer les mouvements filtrés par date
    auto mouvements = m_gestionnaire->obtenirMouvementsParDateRange(dateDebut, dateFin);
    
    qDebug() << "[TABLEAU HISTORIQUE] Résultat:" << mouvements.count() << "mouvements trouvés";
    
    remplirTableau(mouvements);
}