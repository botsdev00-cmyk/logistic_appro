#include "VueStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../widgets/TableauStock.h"
#include "../widgets/TableauHistoriqueStock.h"
#include "../widgets/PanelAlertes.h"
#include "../dialogs/BoiteDialogEntreeStock.h"
#include "../dialogs/BoiteDialogRetourStock.h"
#include "../../core/entities/EntreeStock.h"
#include "../../core/entities/RetourStock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

// ============================================================================
// CONSTRUCTEUR & DESTRUCTEUR
// ============================================================================

VueStock::VueStock(GestionnaireStock* gestionnaire, const QUuid& utilisateurId, QWidget* parent)
    : QWidget(parent),
      m_gestionnaire(gestionnaire),
      m_utilisateurId(utilisateurId)
{
    qDebug() << "[VUE STOCK] Initialisation pour utilisateur:" << utilisateurId;
    
    initializeUI();
    connectSignals();
    chargerDonnees();
    afficherAlertes();
}

VueStock::~VueStock()
{
    qDebug() << "[VUE STOCK] Destruction";
}

// ============================================================================
// INITIALISATION UI
// ============================================================================

void VueStock::initializeUI()
{
    qDebug() << "[VUE STOCK] Initialisation UI";
    
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    // ====== TOOLBAR ======
    QHBoxLayout* toolbar = new QHBoxLayout();

    m_btnAjouterEntree = new QPushButton("➕ Nouvelle Entrée");
    m_btnAjouterEntree->setMinimumWidth(140);
    m_btnAjouterRetour = new QPushButton("↩️ Nouveau Retour");
    m_btnAjouterRetour->setMinimumWidth(140);
    m_btnEntreesEnAttente = new QPushButton("⏳ Entrées en attente");
    m_btnEntreesEnAttente->setMinimumWidth(150);
    m_btnRetoursEnAttente = new QPushButton("⏳ Retours en attente");
    m_btnRetoursEnAttente->setMinimumWidth(150);
    m_btnActualiser = new QPushButton("🔄 Actualiser");
    m_btnActualiser->setMinimumWidth(100);
    m_btnExporter = new QPushButton("📊 Exporter");
    m_btnExporter->setMinimumWidth(100);
    m_btnSynchroniser = new QPushButton("🔗 Synchroniser");
    m_btnSynchroniser->setMinimumWidth(120);

    toolbar->addWidget(m_btnAjouterEntree);
    toolbar->addWidget(m_btnAjouterRetour);
    toolbar->addWidget(m_btnEntreesEnAttente);
    toolbar->addWidget(m_btnRetoursEnAttente);
    toolbar->addStretch();
    toolbar->addWidget(m_btnActualiser);
    toolbar->addWidget(m_btnExporter);
    toolbar->addWidget(m_btnSynchroniser);

    layoutPrincipal->addLayout(toolbar);

    // ====== SEARCH & FILTER ======
    QHBoxLayout* searchLayout = new QHBoxLayout();

    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Rechercher par produit, SKU...");
    m_searchBox->setMaximumWidth(300);

    m_filterStatus = new QComboBox();
    m_filterStatus->addItem("Tous les statuts");
    m_filterStatus->addItem("OK");
    m_filterStatus->addItem("FAIBLE");
    m_filterStatus->addItem("CRITIQUE");
    m_filterStatus->addItem("RUPTURE");
    m_filterStatus->setMaximumWidth(150);

    QLabel* labelValeur = new QLabel("Valeur totale du stock: ");
    m_labelValeurTotal = new QLabel("0,00 €");
    m_labelValeurTotal->setStyleSheet("font-weight: bold; color: #2196F3; font-size: 12px;");

    searchLayout->addWidget(new QLabel("🔍 Recherche:"));
    searchLayout->addWidget(m_searchBox);
    searchLayout->addSpacing(20);
    searchLayout->addWidget(new QLabel("Statut:"));
    searchLayout->addWidget(m_filterStatus);
    searchLayout->addStretch();
    searchLayout->addWidget(labelValeur);
    searchLayout->addWidget(m_labelValeurTotal);

    layoutPrincipal->addLayout(searchLayout);

    // ====== TABS ======
    m_tabWidget = new QTabWidget();
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #E0E0E0; } "
        "QTabBar::tab { padding: 8px 20px; margin-right: 2px; }"
    );

    // Tab 1: Stock actuel
    m_tableauStock = new TableauStock(m_gestionnaire);
    m_tabWidget->addTab(m_tableauStock, "📦 Stock Actuel");

    // Tab 2: Historique des mouvements
    m_tableauHistorique = new TableauHistoriqueStock(m_gestionnaire);
    m_tabWidget->addTab(m_tableauHistorique, "📋 Historique des Mouvements");

    // Tab 3: Alertes
    m_panelAlertes = new PanelAlertes(m_gestionnaire);
    m_tabWidget->addTab(m_panelAlertes, "⚠️ Alertes Stock");

    layoutPrincipal->addWidget(m_tabWidget, 1);

    setLayout(layoutPrincipal);
    
    qDebug() << "[VUE STOCK] ✓ UI initialisée";
}

// ============================================================================
// CONNEXION DES SIGNAUX
// ============================================================================

void VueStock::connectSignals()
{
    qDebug() << "[VUE STOCK] Connexion des signaux";

    connect(m_btnAjouterEntree, &QPushButton::clicked, this, &VueStock::onAjouterEntree);
    connect(m_btnAjouterRetour, &QPushButton::clicked, this, &VueStock::onAjouterRetour);
    connect(m_btnEntreesEnAttente, &QPushButton::clicked, this, &VueStock::onEntreesEnAttente);
    connect(m_btnRetoursEnAttente, &QPushButton::clicked, this, &VueStock::onRetoursEnAttente);
    connect(m_btnActualiser, &QPushButton::clicked, this, &VueStock::onActualiser);
    connect(m_btnExporter, &QPushButton::clicked, this, &VueStock::onExporterStock);
    connect(m_btnSynchroniser, &QPushButton::clicked, this, &VueStock::onSynchroniser);
    
    connect(m_searchBox, &QLineEdit::textChanged, this, &VueStock::onRechercherStock);
    connect(m_filterStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &VueStock::onFiltrerParStatut);

    qDebug() << "[VUE STOCK] ✓ Signaux connectés";
}

// ============================================================================
// CHARGEMENT DES DONNÉES
// ============================================================================

void VueStock::chargerDonnees()
{
    qDebug() << "[VUE STOCK] Chargement des données...";
    
    if (!m_gestionnaire) {
        qWarning() << "[VUE STOCK] Gestionnaire non initialisé!";
        return;
    }

    if (m_tableauStock) {
        m_tableauStock->chargerDonnees();
        qDebug() << "[VUE STOCK] ✓ Tableau stock chargé";
    }

    if (m_tableauHistorique) {
        m_tableauHistorique->chargerDonnees();
        qDebug() << "[VUE STOCK] ✓ Tableau historique chargé";
    }

    // Mise à jour valeur totale
    double valeurTotal = m_gestionnaire->obtenirValeurTotalStock();
    m_labelValeurTotal->setText(QString::number(valeurTotal, 'f', 2) + " €");

    qDebug() << "[VUE STOCK] ✓ Valeur totale:" << valeurTotal;
}

// ============================================================================
// AFFICHAGE DES ALERTES
// ============================================================================

void VueStock::afficherAlertes()
{
    if (!m_gestionnaire) return;

    auto alertes = m_gestionnaire->obtenirAlertes();
    qDebug() << "[VUE STOCK] Alertes actives:" << alertes.count();

    if (!alertes.isEmpty()) {
        auto critiques = m_gestionnaire->obtenirAlertesCritiques();
        // ...
        if (!critiques.isEmpty()) {
            QString message = QString("⚠️ ALERTES CRITIQUES STOCK\n\n");
            message += QString("Total: %1 alerte(s)\n\n").arg(critiques.count());

            // CORRECTION ICI : static_cast<int>(critiques.count())
            for (int i = 0; i < std::min(5, static_cast<int>(critiques.count())); ++i) {
                const auto& alerte = critiques[i];
                message += QString("• %1 (%2)\n  Qté: %3 (min: %4)\n  Action: %5\n\n")
                    .arg(alerte.produitNom, alerte.codeSKU)
                    .arg(alerte.quantiteDisponible).arg(alerte.stockMinimum)
                    .arg(alerte.action);
            }

            if (critiques.count() > 5) {
                message += QString("... et %1 autre(s)").arg(critiques.count() - 5);
            }

            // ✅ Afficher dans le premier onglet (alertes)
            m_tabWidget->setCurrentIndex(2);
        }
    }
}

// ============================================================================
// ACTUALISATION
// ============================================================================

void VueStock::onActualiser()
{
    qDebug() << "[VUE STOCK] Actualisation manuelle";
    chargerDonnees();
    afficherAlertes();
}

// ============================================================================
// SLOTS - ENTRÉES
// ============================================================================

void VueStock::onAjouterEntree()
{
    qDebug() << "[VUE STOCK] Ouvrir dialog nouvelle entrée";

    BoiteDialogEntreeStock dialog(m_gestionnaire, m_utilisateurId, this);
    if (dialog.exec() == QDialog::Accepted) {
        chargerDonnees();
        QMessageBox::information(this, "✓ Succès", "Entrée de stock créée avec succès");
    }
}

void VueStock::onEntreesEnAttente()
{
    qDebug() << "[VUE STOCK] Afficher entrées en attente";

    auto entrees = m_gestionnaire->obtenirEntreesEnAttente();
    
    if (entrees.isEmpty()) {
        QMessageBox::information(this, "Entrées en Attente", "Aucune entrée en attente");
        return;
    }

    QString message = QString("ENTRÉES EN ATTENTE (%1)\n\n").arg(entrees.count());
    
    for (int i = 0; i < std::min(10, static_cast<int>(entrees.count())); ++i) {
        const auto& entree = entrees[i];
        message += QString("• ID: %1\n  Quantité: %2\n  Créée: %3\n\n")
            .arg(entree.getEntreeStockId().toString().mid(0, 8))
            .arg(entree.getQuantite())
            .arg(entree.getDate().toString("dd/MM/yyyy HH:mm"));
    }

    if (entrees.count() > 10) {
        message += QString("... et %1 autre(s)").arg(entrees.count() - 10);
    }

    QMessageBox::information(this, "Entrées en Attente", message);
}

// ============================================================================
// SLOTS - RETOURS
// ============================================================================

void VueStock::onAjouterRetour()
{
    qDebug() << "[VUE STOCK] Ouvrir dialog nouveau retour";

    BoiteDialogRetourStock dialog(m_gestionnaire, m_utilisateurId, this);
    if (dialog.exec() == QDialog::Accepted) {
        chargerDonnees();
        QMessageBox::information(this, "✓ Succès", "Retour de stock créé avec succès");
    }
}

void VueStock::onRetoursEnAttente()
{
    qDebug() << "[VUE STOCK] Afficher retours en attente";

    auto retours = m_gestionnaire->obtenirRetoursEnAttente();
    
    if (retours.isEmpty()) {
        QMessageBox::information(this, "Retours en Attente", "Aucun retour en attente");
        return;
    }

    QString message = QString("RETOURS EN ATTENTE (%1)\n\n").arg(retours.count());
    
    for (int i = 0; i < std::min(10, static_cast<int>(retours.count())); ++i) {
        const auto& retour = retours[i];
        message += QString("• ID: %1\n  Quantité: %2\n  Créé: %3\n\n")
            .arg(retour.getRetourStockId().toString().mid(0, 8))
            .arg(retour.getQuantite())
            .arg(retour.getDate().toString("dd/MM/yyyy HH:mm"));
    }

    if (retours.count() > 10) {
        message += QString("... et %1 autre(s)").arg(retours.count() - 10);
    }

    QMessageBox::information(this, "Retours en Attente", message);
}

// ============================================================================
// SLOTS - RECHERCHE & FILTRES
// ============================================================================

void VueStock::onRechercherStock()
{
    QString critere = m_searchBox->text();
    qDebug() << "[VUE STOCK] Recherche:" << critere;

    if (m_tableauStock) {
        m_tableauStock->filtrer(critere);
    }
}

void VueStock::onFiltrerParStatut(int /*index*/)
{
    QString statut = m_filterStatus->currentText();
    qDebug() << "[VUE STOCK] Filtre statut:" << statut;

    if (m_tableauStock) {
        m_tableauStock->filtrerParStatut(statut);
    }
}

// ============================================================================
// SLOTS - ACTIONS
// ============================================================================

void VueStock::onExporterStock()
{
    qDebug() << "[VUE STOCK] Export stock en CSV/Excel";
    QMessageBox::information(this, "Export", "Fonctionnalité d'export (À implémenter)");
}

void VueStock::onSynchroniser()
{
    qDebug() << "[VUE STOCK] Synchronisation des soldes";
    
    if (!m_gestionnaire) {
        QMessageBox::critical(this, "Erreur", "Gestionnaire non initialisé");
        return;
    }

    if (m_gestionnaire->synchroniserStockSoldes()) {
        chargerDonnees();
        QMessageBox::information(this, "✓ Synchronisation", 
                                "Soldes synchronisés avec succès");
    } else {
        QMessageBox::critical(this, "Erreur", 
                             "Erreur lors de la synchronisation: " + 
                             m_gestionnaire->obtenirDernierErreur());
    }
}