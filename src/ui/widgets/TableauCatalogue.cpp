#include "TableauCatalogue.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QTabWidget>

// Constructeur par défaut
TableauCatalogue::TableauCatalogue(QWidget* parent)
    : QWidget(parent),
      m_gestionnaireCatalogue(nullptr),
      m_utilisateurId(QUuid()),
      m_tableauProduits(nullptr),
      m_tableauCategories(nullptr),
      m_champRecherche(nullptr),
      m_modeleTableau(nullptr),
      m_modeleCategories(nullptr),
      m_proxyModele(nullptr),
      m_proxyModeleCategories(nullptr)
{
    initializeUI();
}

// Constructeur avec gestionnaire et utilisateur
TableauCatalogue::TableauCatalogue(GestionnaireCatalogue* gestionnaire, const QUuid& utilisateurId, QWidget* parent)
    : QWidget(parent),
      m_gestionnaireCatalogue(gestionnaire),
      m_utilisateurId(utilisateurId),
      m_tableauProduits(nullptr),
      m_tableauCategories(nullptr),
      m_champRecherche(nullptr),
      m_modeleTableau(nullptr),
      m_modeleCategories(nullptr),
      m_proxyModele(nullptr),
      m_proxyModeleCategories(nullptr)
{
    qDebug() << "[TABLEAU CATALOGUE] Initialisation avec utilisateur ID:" << utilisateurId.toString();
    initializeUI();
    if (m_gestionnaireCatalogue) {
        rafraichir();
    }
}

TableauCatalogue::~TableauCatalogue()
{
}

void TableauCatalogue::setGestionnaireCatalogue(GestionnaireCatalogue* gestionnaire)
{
    m_gestionnaireCatalogue = gestionnaire;
    rafraichir();
}

void TableauCatalogue::setUtilisateurId(const QUuid& utilisateurId)
{
    m_utilisateurId = utilisateurId;
    qDebug() << "[TABLEAU CATALOGUE] Utilisateur ID défini:" << utilisateurId.toString();
}

void TableauCatalogue::initializeUI()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    // ====== BARRE DE CONTRÔLE ======
    QHBoxLayout* barreControle = new QHBoxLayout();
    barreControle->setContentsMargins(10, 10, 10, 10);

    QLabel* labelRecherche = new QLabel("Rechercher :");
    labelRecherche->setStyleSheet("font-weight: bold;");
    
    m_champRecherche = new QLineEdit();
    m_champRecherche->setPlaceholderText("SKU, Nom, Catégorie ou Code...");
    m_champRecherche->setMaximumWidth(350);
    connect(m_champRecherche, &QLineEdit::textChanged, this, &TableauCatalogue::onRecherche);

    QPushButton* btnAjouterCategorie = new QPushButton("+ Nouvelle Catégorie");
    btnAjouterCategorie->setMinimumWidth(150);
    btnAjouterCategorie->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 6px 12px;"
        "    border-radius: 3px;"
        "    border: none;"
        "}"
        "QPushButton:hover { background-color: #0b7dda; }"
    );
    connect(btnAjouterCategorie, &QPushButton::clicked, this, &TableauCatalogue::onAjouterCategorie);

    QPushButton* btnAjouter = new QPushButton("+ Nouveau Produit");
    btnAjouter->setMinimumWidth(150);
    btnAjouter->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 6px 12px;"
        "    border-radius: 3px;"
        "    border: none;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(btnAjouter, &QPushButton::clicked, this, &TableauCatalogue::onAjouterProduit);

    QPushButton* btnRafraichir = new QPushButton("🔄 Rafraîchir");
    btnRafraichir->setMinimumWidth(120);
    btnRafraichir->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 6px 12px;"
        "    border-radius: 3px;"
        "    border: none;"
        "}"
        "QPushButton:hover { background-color: #e68900; }"
    );
    connect(btnRafraichir, &QPushButton::clicked, this, &TableauCatalogue::rafraichir);

    barreControle->addWidget(labelRecherche);
    barreControle->addWidget(m_champRecherche);
    barreControle->addStretch();
    barreControle->addWidget(btnAjouterCategorie);
    barreControle->addWidget(btnAjouter);
    barreControle->addWidget(btnRafraichir);

    layoutPrincipal->addLayout(barreControle);

    // ====== ONGLETS (PRODUITS ET CATÉGORIES) ======
    QTabWidget* onglets = new QTabWidget();

    // --- ONGLET PRODUITS ---
    m_modeleTableau = new QStandardItemModel(this);
    m_modeleTableau->setHorizontalHeaderLabels(
        QStringList() << "SKU" << "Nom" << "Catégorie" << "Prix (€)" << "Stock Min" << "Statut" << "Actions"
    );

    m_proxyModele = new QSortFilterProxyModel(this);
    m_proxyModele->setSourceModel(m_modeleTableau);
    m_proxyModele->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModele->setFilterKeyColumn(-1);

    m_tableauProduits = new QTableView();
    m_tableauProduits->setModel(m_proxyModele);
    m_tableauProduits->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableauProduits->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableauProduits->setColumnWidth(0, 100);
    m_tableauProduits->setColumnWidth(1, 150);
    m_tableauProduits->setColumnWidth(2, 120);
    m_tableauProduits->setColumnWidth(3, 100);
    m_tableauProduits->setColumnWidth(4, 100);
    m_tableauProduits->setColumnWidth(5, 100);
    m_tableauProduits->setColumnWidth(6, 150);
    m_tableauProduits->setAlternatingRowColors(true);
    m_tableauProduits->setStyleSheet(
        "QTableView { gridline-color: #e0e0e0; }"
        "QTableView::item { padding: 5px; }"
        "QHeaderView::section { background-color: #f5f5f5; padding: 5px; border: 1px solid #ddd; }"
    );
    m_tableauProduits->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableauProduits, &QTableView::customContextMenuRequested,
            this, &TableauCatalogue::onMenuContextuel);

    onglets->addTab(m_tableauProduits, "Produits");

    // --- ONGLET CATÉGORIES ---
    m_modeleCategories = new QStandardItemModel(this);
    m_modeleCategories->setHorizontalHeaderLabels(
        QStringList() << "Code" << "Nom" << "Description" << "Ordre" << "Statut" << "Actions"
    );

    m_proxyModeleCategories = new QSortFilterProxyModel(this);
    m_proxyModeleCategories->setSourceModel(m_modeleCategories);
    m_proxyModeleCategories->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModeleCategories->setFilterKeyColumn(-1);

    m_tableauCategories = new QTableView();
    m_tableauCategories->setModel(m_proxyModeleCategories);
    m_tableauCategories->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableauCategories->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableauCategories->setColumnWidth(0, 80);
    m_tableauCategories->setColumnWidth(1, 120);
    m_tableauCategories->setColumnWidth(2, 200);
    m_tableauCategories->setColumnWidth(3, 80);
    m_tableauCategories->setColumnWidth(4, 80);
    m_tableauCategories->setColumnWidth(5, 150);
    m_tableauCategories->setAlternatingRowColors(true);
    m_tableauCategories->setStyleSheet(
        "QTableView { gridline-color: #e0e0e0; }"
        "QTableView::item { padding: 5px; }"
        "QHeaderView::section { background-color: #f5f5f5; padding: 5px; border: 1px solid #ddd; }"
    );
    m_tableauCategories->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableauCategories, &QTableView::customContextMenuRequested,
            this, &TableauCatalogue::onMenuContextuel);

    onglets->addTab(m_tableauCategories, "Catégories");

    layoutPrincipal->addWidget(onglets);
    setLayout(layoutPrincipal);
}

void TableauCatalogue::rafraichir()
{
    chargerProduits();
    chargerCategories();
}

void TableauCatalogue::chargerProduits()
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    m_modeleTableau->removeRows(0, m_modeleTableau->rowCount());

    QList<Produit> produits = m_gestionnaireCatalogue->obtenirTousProduits();

    for (const Produit& produit : produits) {
        QStandardItem* itemSku = new QStandardItem(produit.getCodeSku());
        itemSku->setEditable(false);

        QStandardItem* itemNom = new QStandardItem(produit.getNom());
        itemNom->setEditable(false);

        CategorieProduit categorie = m_gestionnaireCatalogue->obtenirCategorie(
            produit.getCategorieProduitId()
        );
        QStandardItem* itemCategorie = new QStandardItem(categorie.getNom());
        itemCategorie->setEditable(false);

        QStandardItem* itemPrix = new QStandardItem(QString::number(produit.getPrixUnitaire(), 'f', 2));
        itemPrix->setEditable(false);

        QStandardItem* itemStockMin = new QStandardItem(QString::number(produit.getStockMinimum()));
        itemStockMin->setEditable(false);

        QString statut = produit.estActif() ? "✓ Actif" : "✕ Inactif";
        QStandardItem* itemStatut = new QStandardItem(statut);
        itemStatut->setEditable(false);
        if (!produit.estActif()) {
            itemStatut->setBackground(QColor(255, 200, 200));
        } else {
            itemStatut->setBackground(QColor(200, 255, 200));
        }

        QStandardItem* itemActions = new QStandardItem(produit.getProduitId().toString());
        itemActions->setEditable(false);
        itemActions->setBackground(QColor(240, 240, 240));

        QList<QStandardItem*> row;
        row << itemSku << itemNom << itemCategorie << itemPrix << itemStockMin << itemStatut << itemActions;

        m_modeleTableau->appendRow(row);
    }

    m_tableauProduits->resizeRowsToContents();
}

void TableauCatalogue::chargerCategories()
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    m_modeleCategories->removeRows(0, m_modeleCategories->rowCount());

    QList<CategorieProduit> categories = m_gestionnaireCatalogue->obtenirTousCategoriesProduits();

    for (const CategorieProduit& categorie : categories) {
        QStandardItem* itemCode = new QStandardItem(categorie.getCodeCategorie());
        itemCode->setEditable(false);

        QStandardItem* itemNom = new QStandardItem(categorie.getNom());
        itemNom->setEditable(false);

        QStandardItem* itemDescription = new QStandardItem(categorie.getDescription());
        itemDescription->setEditable(false);

        QStandardItem* itemOrdre = new QStandardItem(QString::number(categorie.getOrdreAffichage()));
        itemOrdre->setEditable(false);

        QString statut = categorie.estActif() ? "✓ Actif" : "✕ Inactif";
        QStandardItem* itemStatut = new QStandardItem(statut);
        itemStatut->setEditable(false);
        if (!categorie.estActif()) {
            itemStatut->setBackground(QColor(255, 200, 200));
        } else {
            itemStatut->setBackground(QColor(200, 255, 200));
        }

        QStandardItem* itemActions = new QStandardItem(categorie.getCategorieProduitId().toString());
        itemActions->setEditable(false);
        itemActions->setBackground(QColor(240, 240, 240));

        QList<QStandardItem*> row;
        row << itemCode << itemNom << itemDescription << itemOrdre << itemStatut << itemActions;

        m_modeleCategories->appendRow(row);
    }

    m_tableauCategories->resizeRowsToContents();
}

void TableauCatalogue::onRecherche(const QString& texte)
{
    m_proxyModele->setFilterWildcard(texte);
    m_proxyModeleCategories->setFilterWildcard(texte);
}

void TableauCatalogue::onAjouterProduit()
{
    if (!m_gestionnaireCatalogue) {
        QMessageBox::warning(this, "Erreur", "Gestionnaire non initialisé");
        return;
    }

    BoiteDialogProduit dialog(m_gestionnaireCatalogue, this);
    if (dialog.exec() == QDialog::Accepted) {
        rafraichir();
        emit produitAjoute();
    }
}

void TableauCatalogue::onAjouterCategorie()
{
    if (!m_gestionnaireCatalogue) {
        QMessageBox::warning(this, "Erreur", "Gestionnaire non initialisé");
        return;
    }

    BoiteDialogCategorie dialog(m_gestionnaireCatalogue, this);
    if (dialog.exec() == QDialog::Accepted) {
        rafraichir();
        emit categorieAjoutee();
    }
}

void TableauCatalogue::onMenuContextuel(const QPoint& position)
{
    QTableView* tableauSource = qobject_cast<QTableView*>(sender());
    if (!tableauSource) {
        return;
    }

    QModelIndex index = tableauSource->indexAt(position);
    if (!index.isValid()) {
        return;
    }

    QSortFilterProxyModel* proxyModel = qobject_cast<QSortFilterProxyModel*>(tableauSource->model());
    if (!proxyModel) {
        return;
    }

    QStandardItemModel* sourceModel = qobject_cast<QStandardItemModel*>(proxyModel->sourceModel());
    if (!sourceModel) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    int lastColumn = sourceModel->columnCount() - 1;
    QString idStr = sourceModel->item(sourceIndex.row(), lastColumn)->text();

    QMenu menu;
    QAction* actionModifier = menu.addAction("✎ Modifier");
    QAction* actionBascule = menu.addAction("Changer statut");
    menu.addSeparator();
    QAction* actionSupprimer = menu.addAction("✕ Supprimer");

    QAction* actionSelectionnee = menu.exec(tableauSource->mapToGlobal(position));

    if (tableauSource == m_tableauProduits) {
        QUuid produitId(idStr);
        if (actionSelectionnee == actionModifier) {
            onModifierProduit(produitId);
        } else if (actionSelectionnee == actionBascule) {
            onBasculeStatut(produitId);
        } else if (actionSelectionnee == actionSupprimer) {
            onSupprimerProduit(produitId);
        }
    } else if (tableauSource == m_tableauCategories) {
        QUuid categorieId(idStr);
        if (actionSelectionnee == actionModifier) {
            onModifierCategorie(categorieId);
        } else if (actionSelectionnee == actionBascule) {
            onBasculeStatutCategorie(categorieId);
        } else if (actionSelectionnee == actionSupprimer) {
            onSupprimerCategorie(categorieId);
        }
    }
}

void TableauCatalogue::onModifierProduit(const QUuid& produitId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    Produit produit = m_gestionnaireCatalogue->obtenirProduit(produitId);
    if (produit.getProduitId().isNull()) {
        QMessageBox::warning(this, "Erreur", "Produit non trouvé");
        return;
    }

    BoiteDialogProduit dialog(m_gestionnaireCatalogue, this);
    dialog.chargerProduit(produit);
    if (dialog.exec() == QDialog::Accepted) {
        rafraichir();
        emit produitModifie();
    }
}

void TableauCatalogue::onBasculeStatut(const QUuid& produitId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    Produit produit = m_gestionnaireCatalogue->obtenirProduit(produitId);
    if (produit.getProduitId().isNull()) {
        QMessageBox::warning(this, "Erreur", "Produit non trouvé");
        return;
    }

    bool nouvelEtat = !produit.estActif();
    if (m_gestionnaireCatalogue->changerStatutProduit(produitId, nouvelEtat)) {
        rafraichir();
        QString message = nouvelEtat ? "Produit réactivé" : "Produit désactivé";
        QMessageBox::information(this, "Succès", message);
    } else {
        QMessageBox::warning(this, "Erreur", m_gestionnaireCatalogue->obtenirDernierErreur());
    }
}

void TableauCatalogue::onSupprimerProduit(const QUuid& produitId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    QMessageBox::StandardButton reponse = QMessageBox::question(
        this,
        "Confirmation suppression",
        "Êtes-vous sûr de vouloir supprimer ce produit ?\n(Cette action ne peut pas être annulée)",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reponse == QMessageBox::Yes) {
        if (m_gestionnaireCatalogue->supprimerProduit(produitId)) {
            rafraichir();
            QMessageBox::information(this, "Succès", "Produit supprimé");
            emit produitSupprime();
        } else {
            QMessageBox::warning(this, "Erreur", m_gestionnaireCatalogue->obtenirDernierErreur());
        }
    }
}

void TableauCatalogue::onModifierCategorie(const QUuid& categorieId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    CategorieProduit categorie = m_gestionnaireCatalogue->obtenirCategorie(categorieId);
    if (categorie.getCategorieProduitId().isNull()) {
        QMessageBox::warning(this, "Erreur", "Catégorie non trouvée");
        return;
    }

    BoiteDialogCategorie dialog(m_gestionnaireCatalogue, this);
    dialog.chargerCategorie(categorie);
    if (dialog.exec() == QDialog::Accepted) {
        rafraichir();
        emit categorieModifiee();
    }
}

void TableauCatalogue::onBasculeStatutCategorie(const QUuid& categorieId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    CategorieProduit categorie = m_gestionnaireCatalogue->obtenirCategorie(categorieId);
    if (categorie.getCategorieProduitId().isNull()) {
        QMessageBox::warning(this, "Erreur", "Catégorie non trouvée");
        return;
    }

    bool nouvelEtat = !categorie.estActif();
    if (m_gestionnaireCatalogue->changerStatutCategorie(categorieId, nouvelEtat)) {
        rafraichir();
        QString message = nouvelEtat ? "Catégorie réactivée" : "Catégorie désactivée";
        QMessageBox::information(this, "Succès", message);
    } else {
        QMessageBox::warning(this, "Erreur", m_gestionnaireCatalogue->obtenirDernierErreur());
    }
}

void TableauCatalogue::onSupprimerCategorie(const QUuid& categorieId)
{
    if (!m_gestionnaireCatalogue) {
        return;
    }

    // Vérifier si la catégorie a des produits
    QList<Produit> produits = m_gestionnaireCatalogue->obtenirProduitsParCategorie(categorieId);
    if (!produits.isEmpty()) {
        QMessageBox::warning(this, "Erreur", 
            "Impossible de supprimer cette catégorie car elle contient " + 
            QString::number(produits.count()) + " produit(s)");
        return;
    }

    QMessageBox::StandardButton reponse = QMessageBox::question(
        this,
        "Confirmation suppression",
        "Êtes-vous sûr de vouloir supprimer cette catégorie ?\n(Cette action ne peut pas être annulée)",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reponse == QMessageBox::Yes) {
        if (m_gestionnaireCatalogue->supprimerCategorie(categorieId)) {
            rafraichir();
            QMessageBox::information(this, "Succès", "Catégorie supprimée");
            emit categorieSupprimee();
        } else {
            QMessageBox::warning(this, "Erreur", m_gestionnaireCatalogue->obtenirDernierErreur());
        }
    }
}