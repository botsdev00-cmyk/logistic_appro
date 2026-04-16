#include "BoiteDialogEntreeStock.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../core/entities/EntreeStock.h"
#include "../../core/entities/Produit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QTextDocument>
#include <QPrinter>
#include <QPainter>
#include "../../data/database/ConnexionBaseDonnees.h"

// ============================================================================
// CONSTRUCTEUR
// ============================================================================

BoiteDialogEntreeStock::BoiteDialogEntreeStock(GestionnaireStock* gestionnaire,
                                               const QUuid& utilisateurId,
                                               QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(gestionnaire),
      m_utilisateurId(utilisateurId)
{
    qDebug() << "[ENTREE STOCK] ═══════════════════════════════════════════════";
    qDebug() << "[ENTREE STOCK] Initialisation";
    
    setWindowTitle("📥 Gestion des Entrées de Stock");
    setMinimumSize(1400, 700);
    setModal(true);

    initializeUI();
    connectSignals();
    chargerProduits();
    chargerSources();

    qDebug() << "[ENTREE STOCK] ✓ Initialisation OK";
    qDebug() << "[ENTREE STOCK] ═══════════════════════════════════════════════";
}

BoiteDialogEntreeStock::~BoiteDialogEntreeStock()
{
    qDebug() << "[ENTREE STOCK] Destruction";
}

// ============================================================================
// INITIALISATION UI
// ============================================================================

void BoiteDialogEntreeStock::initializeUI()
{
    qDebug() << "[ENTREE STOCK] Initialisation UI";

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ====== HEADER ======
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* titleLabel = new QLabel("📥 Approvisionnement du Stock");
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2; "
        "padding: 10px; background-color: #E3F2FD; border-radius: 5px;"
    );
    
    m_labelNbLignes = new QLabel("Lignes: 0");
    m_labelNbLignes->setStyleSheet("font-weight: bold; color: #666;");
    
    m_labelTotal = new QLabel("Total: 0,00 €");
    m_labelTotal->setStyleSheet("font-weight: bold; color: #2196F3; font-size: 13px;");
    
    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(m_labelNbLignes);
    headerLayout->addWidget(m_labelTotal);
    mainLayout->addLayout(headerLayout);

    // ====== INSTRUCTIONS ======
    QLabel* instructionsLabel = new QLabel(
        "📋 Sélectionnez les produits à approvisionner. "
        "Vous pouvez ajouter plusieurs articles en une seule opération. "
        "Cliquez sur la colonne 'Produit' pour sélectionner."
    );
    instructionsLabel->setStyleSheet(
        "color: #555; font-size: 11px; padding: 8px; "
        "background-color: #FFF9C4; border-left: 4px solid #FBC02D; border-radius: 3px;"
    );
    mainLayout->addWidget(instructionsLabel);

    // ====== TABLE PRINCIPALE ======
    m_tableWidget = new QTableWidget();
    m_tableWidget->setColumnCount(10);
    m_tableWidget->setHorizontalHeaderLabels(QStringList()
        << "Produit" << "SKU" << "Catégorie" << "Qté" << "Prix Unit."
        << "Montant" << "Facture" << "Lot" << "Expiration" << "Source");
    
    m_tableWidget->horizontalHeader()->setStretchLastSection(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setStyleSheet(
        "QTableWidget { gridline-color: #E0E0E0; border: 1px solid #BDBDBD; } "
        "QHeaderView::section { "
        "  background-color: #1976D2; color: white; padding: 6px; "
        "  border: none; font-weight: bold; "
        "} "
        "QTableWidget::item { padding: 4px; }"
    );

    mainLayout->addWidget(m_tableWidget, 1);

    // ====== TOOLBAR ACTIONS ======
    QHBoxLayout* toolbarLayout = new QHBoxLayout();

    m_btnAjouter = new QPushButton("➕ Ajouter une ligne");
    m_btnAjouter->setMinimumWidth(140);
    m_btnAjouter->setIcon(QIcon(":/icons/add.png"));
    
    m_btnSupprimer = new QPushButton("🗑️ Supprimer la ligne");
    m_btnSupprimer->setMinimumWidth(140);
    m_btnSupprimer->setIcon(QIcon(":/icons/delete.png"));

    QLabel* separateur1 = new QLabel(" | ");
    
    m_btnImprimer = new QPushButton("🖨️ Imprimer");
    m_btnImprimer->setMinimumWidth(100);
    
    m_btnExporter = new QPushButton("📊 Exporter (CSV)");
    m_btnExporter->setMinimumWidth(120);

    toolbarLayout->addWidget(m_btnAjouter);
    toolbarLayout->addWidget(m_btnSupprimer);
    toolbarLayout->addWidget(separateur1);
    toolbarLayout->addWidget(m_btnImprimer);
    toolbarLayout->addWidget(m_btnExporter);
    toolbarLayout->addStretch();

    mainLayout->addLayout(toolbarLayout);

    // ====== STATUT ======
    m_labelStatut = new QLabel("✓ Prêt à saisir");
    m_labelStatut->setStyleSheet(
        "color: #4CAF50; font-size: 11px; padding: 6px; "
        "background-color: #E8F5E9; border-radius: 3px;"
    );
    mainLayout->addWidget(m_labelStatut);

    // ====== BOUTONS FINAUX ======
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_btnValider = new QPushButton("✓ Valider & Créer les Entrées");
    m_btnValider->setMinimumWidth(200);
    m_btnValider->setMinimumHeight(35);
    m_btnValider->setStyleSheet(
        "background-color: #4CAF50; color: white; font-weight: bold; "
        "padding: 8px; border-radius: 5px; font-size: 12px;"
    );
    
    m_btnAnnuler = new QPushButton("✗ Annuler");
    m_btnAnnuler->setMinimumWidth(120);
    m_btnAnnuler->setMinimumHeight(35);

    buttonLayout->addWidget(m_btnValider);
    buttonLayout->addWidget(m_btnAnnuler);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    qDebug() << "[ENTREE STOCK] ✓ UI initialisée";
}

// ============================================================================
// CONNEXION DES SIGNAUX
// ============================================================================

void BoiteDialogEntreeStock::connectSignals()
{
    qDebug() << "[ENTREE STOCK] Connexion des signaux";

    connect(m_btnAjouter, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onAjouterLigne);
    connect(m_btnSupprimer, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onSupprimerLigne);
    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onValider);
    connect(m_btnAnnuler, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onAnnuler);
    connect(m_btnImprimer, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onImprimer);
    connect(m_btnExporter, &QPushButton::clicked, this, &BoiteDialogEntreeStock::onExporter);
    connect(m_tableWidget, &QTableWidget::cellClicked, this, &BoiteDialogEntreeStock::onSelectionnerProduit);
    connect(m_tableWidget, &QTableWidget::cellChanged, this, &BoiteDialogEntreeStock::onCellChanged);

    qDebug() << "[ENTREE STOCK] ✓ Signaux connectés";
}

// ============================================================================
// CHARGEMENT DES DONNÉES
// ============================================================================

void BoiteDialogEntreeStock::chargerProduits()
{
    qDebug() << "[ENTREE STOCK] Chargement des produits...";

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT p.produit_id, p.nom, p.code_sku, p.prix_unitaire, "
        "       cp.nom as categorie "
        "FROM produits p "
        "LEFT JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "WHERE p.est_actif = true "
        "ORDER BY p.nom ASC"
    );

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            QUuid id(query.value("produit_id").toString());
            QString nom = query.value("nom").toString();
            QString sku = query.value("code_sku").toString();
            
            m_produitsMap[nom + " (" + sku + ")"] = id;
            count++;
        }
        qDebug() << "[ENTREE STOCK] ✓ Produits chargés:" << count;
    } else {
        qWarning() << "[ENTREE STOCK] Erreur:" << query.lastError().text();
    }
}

void BoiteDialogEntreeStock::chargerSources()
{
    qDebug() << "[ENTREE STOCK] Chargement des sources...";

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT source_entree_id, code, nom FROM sources_entree ORDER BY nom ASC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            QUuid id(query.value("source_entree_id").toString());
            QString code = query.value("code").toString();
            
            m_sourcesMap[code] = id;
            count++;
        }
        qDebug() << "[ENTREE STOCK] ✓ Sources chargées:" << count;
    } else {
        qWarning() << "[ENTREE STOCK] Erreur:" << query.lastError().text();
    }
}

// ============================================================================
// GESTION DES LIGNES
// ============================================================================

void BoiteDialogEntreeStock::ajouterLigneVide()
{
    qDebug() << "[ENTREE STOCK] Ajout d'une ligne";

    int row = m_tableWidget->rowCount();
    m_tableWidget->insertRow(row);

    // Produit (combo)
    QComboBox* comboProduit = new QComboBox();
    comboProduit->addItem("-- Sélectionner --");
    for (const auto& nom : m_produitsMap.keys()) {
        comboProduit->addItem(nom);
    }
    comboProduit->setStyleSheet(
        "QComboBox { border: 1px solid #BDBDBD; padding: 4px; border-radius: 3px; } "
        "QComboBox::drop-down { border: none; }"
    );
    // Connecter le signal pour charger les détails du produit
    connect(comboProduit, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, [this, row](const QString &text) {
        if (text != "-- Sélectionner --") {
            onSelectionnerProduit(row, 0);
        }
    });
    m_tableWidget->setCellWidget(row, 0, comboProduit);

    // SKU (lecture seule)
    QTableWidgetItem* skuItem = new QTableWidgetItem();
    skuItem->setFlags(skuItem->flags() & ~Qt::ItemIsEditable);
    skuItem->setBackground(QColor(245, 245, 245));
    m_tableWidget->setItem(row, 1, skuItem);

    // Catégorie (lecture seule)
    QTableWidgetItem* catItem = new QTableWidgetItem();
    catItem->setFlags(catItem->flags() & ~Qt::ItemIsEditable);
    catItem->setBackground(QColor(245, 245, 245));
    m_tableWidget->setItem(row, 2, catItem);

    // Quantité
    QSpinBox* spinQte = new QSpinBox();
    spinQte->setMinimum(0);
    spinQte->setMaximum(100000);
    spinQte->setValue(0);
    spinQte->setStyleSheet(
        "QSpinBox { border: 1px solid #BDBDBD; padding: 4px; border-radius: 3px; } "
    );
    // Connecter pour recalculer montant
    connect(spinQte, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this, row](int) { onCellChanged(row, 3); });
    m_tableWidget->setCellWidget(row, 3, spinQte);

    // Prix unitaire
    QDoubleSpinBox* spinPrix = new QDoubleSpinBox();
    spinPrix->setMinimum(0);
    spinPrix->setMaximum(999999);
    spinPrix->setDecimals(2);
    spinPrix->setValue(0);
    spinPrix->setStyleSheet(
        "QDoubleSpinBox { border: 1px solid #BDBDBD; padding: 4px; border-radius: 3px; } "
    );
    // Connecter pour recalculer montant
    connect(spinPrix, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, row](double) { onCellChanged(row, 4); });
    m_tableWidget->setCellWidget(row, 4, spinPrix);

    // Montant (lecture seule)
    QTableWidgetItem* montantItem = new QTableWidgetItem("0,00");
    montantItem->setFlags(montantItem->flags() & ~Qt::ItemIsEditable);
    montantItem->setBackground(QColor(245, 245, 245));
    montantItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_tableWidget->setItem(row, 5, montantItem);

    // Numéro facture
    QTableWidgetItem* factureItem = new QTableWidgetItem();
    m_tableWidget->setItem(row, 6, factureItem);

    // Numéro lot
    QTableWidgetItem* lotItem = new QTableWidgetItem();
    m_tableWidget->setItem(row, 7, lotItem);

    // Date expiration
    QDateEdit* dateExp = new QDateEdit();
    dateExp->setDate(QDate::currentDate().addMonths(12));
    dateExp->setCalendarPopup(true);
    dateExp->setStyleSheet(
        "QDateEdit { border: 1px solid #BDBDBD; padding: 4px; border-radius: 3px; } "
    );
    m_tableWidget->setCellWidget(row, 8, dateExp);

    // Source
    QComboBox* comboSource = new QComboBox();
    comboSource->addItem("-- Sélectionner --");
    for (const auto& code : m_sourcesMap.keys()) {
        comboSource->addItem(code);
    }
    comboSource->setStyleSheet(
        "QComboBox { border: 1px solid #BDBDBD; padding: 4px; border-radius: 3px; } "
    );
    m_tableWidget->setCellWidget(row, 9, comboSource);

    // Mettre à jour le compteur
    m_labelNbLignes->setText(QString("Lignes: %1").arg(m_tableWidget->rowCount()));

    qDebug() << "[ENTREE STOCK] ✓ Ligne" << row << "ajoutée";
}

void BoiteDialogEntreeStock::onAjouterLigne()
{
    qDebug() << "[ENTREE STOCK] Ajout de ligne";
    ajouterLigneVide();
}

void BoiteDialogEntreeStock::onSupprimerLigne()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "⚠️ Attention", "Sélectionnez une ligne à supprimer");
        return;
    }

    qDebug() << "[ENTREE STOCK] Suppression ligne" << row;
    m_tableWidget->removeRow(row);
    m_labelNbLignes->setText(QString("Lignes: %1").arg(m_tableWidget->rowCount()));
}

// ============================================================================
// SÉLECTION DE PRODUIT
// ============================================================================

void BoiteDialogEntreeStock::onSelectionnerProduit(int row, int col)
{
    if (col != 0) return;

    qDebug() << "[ENTREE STOCK] Sélection produit ligne" << row;

    QComboBox* combo = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(row, 0));
    if (!combo || combo->currentText() == "-- Sélectionner --") return;

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    QUuid produitId = m_produitsMap[combo->currentText()];
    query.prepare(
        "SELECT p.code_sku, p.prix_unitaire, cp.nom as categorie "
        "FROM produits p "
        "LEFT JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "WHERE p.produit_id = ?"
    );
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        // SKU
        m_tableWidget->item(row, 1)->setText(query.value("code_sku").toString());
        
        // Catégorie
        m_tableWidget->item(row, 2)->setText(query.value("categorie").toString());

        // Prix
        QDoubleSpinBox* spinPrix = qobject_cast<QDoubleSpinBox*>(m_tableWidget->cellWidget(row, 4));
        if (spinPrix) {
            spinPrix->setValue(query.value("prix_unitaire").toDouble());
        }

        qDebug() << "[ENTREE STOCK] ✓ Produit sélectionné:" << combo->currentText();
    }
}

// ============================================================================
// CALCUL DU MONTANT
// ============================================================================

void BoiteDialogEntreeStock::onCellChanged(int row, int col)
{
    if (col == 3 || col == 4) {
        QSpinBox* spinQte = qobject_cast<QSpinBox*>(m_tableWidget->cellWidget(row, 3));
        QDoubleSpinBox* spinPrix = qobject_cast<QDoubleSpinBox*>(m_tableWidget->cellWidget(row, 4));

        if (spinQte && spinPrix) {
            double montant = spinQte->value() * spinPrix->value();
            m_tableWidget->item(row, 5)->setText(QString::number(montant, 'f', 2));
        }

        // Recalculer total
        double totalGeneral = 0;
        for (int r = 0; r < m_tableWidget->rowCount(); ++r) {
            QTableWidgetItem* montantItem = m_tableWidget->item(r, 5);
            if (montantItem) {
                totalGeneral += montantItem->text().replace(',', '.').toDouble();
            }
        }
        m_labelTotal->setText(QString("Total: %1 €").arg(totalGeneral, 0, 'f', 2));
    }
}

// ============================================================================
// VALIDATION & SAUVEGARDE
// ============================================================================

bool BoiteDialogEntreeStock::validerLignes()
{
    qDebug() << "[ENTREE STOCK] Validation des lignes...";

    int lignesValides = 0;
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QComboBox* comboProduit = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(row, 0));
        if (!comboProduit || comboProduit->currentText() == "-- Sélectionner --") continue;

        if (!validerLigne(row)) {
            m_labelStatut->setText(QString("❌ Erreur ligne %1").arg(row + 1));
            m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");
            return false;
        }
        lignesValides++;
    }

    if (lignesValides == 0) {
        m_labelStatut->setText("❌ Aucune ligne à valider");
        m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");
        return false;
    }

    qDebug() << "[ENTREE STOCK] ✓ Validation OK:" << lignesValides << "lignes";
    return true;
}

bool BoiteDialogEntreeStock::validerLigne(int row)
{
    QComboBox* comboProduit = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(row, 0));
    QSpinBox* spinQte = qobject_cast<QSpinBox*>(m_tableWidget->cellWidget(row, 3));
    QDoubleSpinBox* spinPrix = qobject_cast<QDoubleSpinBox*>(m_tableWidget->cellWidget(row, 4));

    if (!comboProduit || !spinQte || !spinPrix) return false;
    if (comboProduit->currentText() == "-- Sélectionner --") return false;
    if (spinQte->value() <= 0) return false;
    if (spinPrix->value() < 0) return false;

    return true;
}

bool BoiteDialogEntreeStock::sauvegarderEntrees()
{
    qDebug() << "[ENTREE STOCK] Sauvegarde des entrées...";
    
    // Vérifier que l'utilisateur est valide
    if (m_utilisateurId.isNull()) {
        m_labelStatut->setText("❌ Erreur: Utilisateur non défini");
        m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");
        QMessageBox::critical(this, "❌ Erreur", "L'ID utilisateur n'est pas valide");
        return false;
    }

    qDebug() << "[ENTREE STOCK] Utilisateur créateur ID:" << m_utilisateurId.toString();

    int count = 0;
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        // Récupérer les widgets de la ligne
        QComboBox* comboProduit = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(row, 0));
        QSpinBox* spinQte = qobject_cast<QSpinBox*>(m_tableWidget->cellWidget(row, 3));
        QDoubleSpinBox* spinPrix = qobject_cast<QDoubleSpinBox*>(m_tableWidget->cellWidget(row, 4));
        QTableWidgetItem* factureItem = m_tableWidget->item(row, 6);
        QTableWidgetItem* lotItem = m_tableWidget->item(row, 7);
        QDateEdit* dateExp = qobject_cast<QDateEdit*>(m_tableWidget->cellWidget(row, 8));
        QComboBox* comboSource = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(row, 9));

        // Vérifier que le produit est sélectionné
        if (!comboProduit || comboProduit->currentText() == "-- Sélectionner --") {
            continue;
        }

        // Vérifier que la source est sélectionnée
        if (!comboSource || comboSource->currentText() == "-- Sélectionner --") {
            m_labelStatut->setText(QString("❌ Erreur ligne %1: Source manquante").arg(row + 1));
            m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");
            QMessageBox::critical(this, "❌ Erreur",
                QString("Ligne %1: Vous devez sélectionner une source").arg(row + 1));
            return false;
        }

        // Créer l'entrée
        EntreeStock entree;
        entree.setEntreeStockId(QUuid::createUuid());
        entree.setProduitId(m_produitsMap[comboProduit->currentText()]);
        entree.setQuantite(spinQte ? spinQte->value() : 0);
        entree.setPrixUnitaire(spinPrix ? spinPrix->value() : 0.0);
        entree.setNumeroFacture(factureItem ? factureItem->text() : "");
        entree.setNumeroLot(lotItem ? lotItem->text() : "");
        entree.setDateExpiration(dateExp ? dateExp->date() : QDate::currentDate());
        entree.setCreePar(m_utilisateurId);  // ✅ Utiliser l'UUID de l'utilisateur courant
        entree.setSourceEntreeId(m_sourcesMap[comboSource->currentText()]);
        entree.setStatutValidation("EN_ATTENTE");
        entree.setDate(QDateTime::currentDateTime());

        qDebug() << "[ENTREE STOCK] Ligne" << row + 1 
                 << "- Produit:" << comboProduit->currentText()
                 << "- Quantité:" << entree.getQuantite()
                 << "- CreePar:" << m_utilisateurId.toString();

        // Sauvegarder
        if (m_gestionnaire->creerEntreeStock(entree)) {
            count++;
            qDebug() << "[ENTREE STOCK] ✓ Entrée" << count << "créée";
        } else {
            QString erreur = m_gestionnaire->obtenirDernierErreur();
            qWarning() << "[ENTREE STOCK] ❌ Erreur:" << erreur;
            
            m_labelStatut->setText(QString("❌ Erreur création ligne %1").arg(row + 1));
            m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");

            QMessageBox::critical(this, "❌ Erreur",
                QString("Erreur création entrée ligne %1:\n%2")
                .arg(row + 1)
                .arg(erreur));
            return false;
        }
    }

    if (count == 0) {
        m_labelStatut->setText("❌ Aucune ligne complète n'a pu être sauvegardée");
        m_labelStatut->setStyleSheet("color: #F44336; font-size: 11px; padding: 6px; background-color: #FFEBEE; border-radius: 3px;");
        QMessageBox::warning(this, "⚠️ Attention", "Aucune ligne n'a été complètement remplie");
        return false;
    }

    qDebug() << "[ENTREE STOCK] ✓ Sauvegarde OK:" << count << "entrées";
    return true;
}

// ============================================================================
// ACTIONS FINALES
// ============================================================================

void BoiteDialogEntreeStock::onValider()
{
    qDebug() << "[ENTREE STOCK] Validation demandée";

    m_labelStatut->setText("🔄 Validation et création en cours...");
    m_labelStatut->setStyleSheet("color: #FF9800; font-size: 11px; padding: 6px; background-color: #FFF3E0; border-radius: 3px;");

    if (!validerLignes()) return;
    if (!sauvegarderEntrees()) return;

    m_labelStatut->setText("✓ Toutes les entrées créées avec succès!");
    m_labelStatut->setStyleSheet("color: #4CAF50; font-size: 11px; padding: 6px; background-color: #E8F5E9; border-radius: 3px;");

    QMessageBox::information(this, "✓ Succès",
        "Tous les approvisionnements ont été enregistrés avec succès!");

    accept();
}

void BoiteDialogEntreeStock::onAnnuler()
{
    qDebug() << "[ENTREE STOCK] Annulation";
    reject();
}

void BoiteDialogEntreeStock::onImprimer()
{
    qDebug() << "[ENTREE STOCK] Impression demandée";
    QMessageBox::information(this, "Impression",
        "Fonctionnalité d'impression à implémenter");
}

void BoiteDialogEntreeStock::onExporter()
{
    qDebug() << "[ENTREE STOCK] Export demandé";
    QMessageBox::information(this, "Export",
        "Fonctionnalité d'export CSV à implémenter");
}