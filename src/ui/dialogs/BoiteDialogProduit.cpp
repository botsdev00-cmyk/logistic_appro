#include "BoiteDialogProduit.h"
#include "../../business/managers/GestionnaireCatalogue.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFormLayout>
#include <QRegularExpression>

BoiteDialogProduit::BoiteDialogProduit(GestionnaireCatalogue* gestionnaire, QWidget* parent)
    : QDialog(parent),
      m_gestionnaire(gestionnaire),
      m_modeModification(false),
      m_produitId(QUuid())
{
    initializeUI();
    chargerCategories();
    setWindowTitle("Nouveau Produit");
    setModal(true);
    setMinimumWidth(500);
}

BoiteDialogProduit::~BoiteDialogProduit()
{
}

void BoiteDialogProduit::initializeUI()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    m_editNom = new QLineEdit();
    m_editSku = new QLineEdit();
    m_editSku->setPlaceholderText("Ex: ELEC-001 (auto-majuscule)");
    m_editSku->setMaxLength(50);
    connect(m_editSku, &QLineEdit::textChanged, this, &BoiteDialogProduit::onSkuChanged);

    m_comboCategorie = new QComboBox();

    m_spinPrix = new QDoubleSpinBox();
    m_spinPrix->setMinimum(0.01);
    m_spinPrix->setMaximum(999999.99);
    m_spinPrix->setDecimals(2);
    m_spinPrix->setValue(0.00);

    m_spinStockMin = new QSpinBox();
    m_spinStockMin->setMinimum(0);
    m_spinStockMin->setMaximum(999999);
    m_spinStockMin->setValue(10);

    m_editDescription = new QTextEdit();
    m_editDescription->setMaximumHeight(100);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Nom :", m_editNom);
    formLayout->addRow("SKU :", m_editSku);
    formLayout->addRow("Catégorie :", m_comboCategorie);
    formLayout->addRow("Prix unitaire (€) :", m_spinPrix);
    formLayout->addRow("Stock minimum :", m_spinStockMin);
    formLayout->addRow("Description :", m_editDescription);
    layoutPrincipal->addLayout(formLayout);

    m_btnValider = new QPushButton("✓ Enregistrer");
    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogProduit::onValiderClicked);

    m_btnAnnuler = new QPushButton("✕ Annuler");
    connect(m_btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout* boutonsLayout = new QHBoxLayout;
    boutonsLayout->addStretch();
    boutonsLayout->addWidget(m_btnValider);
    boutonsLayout->addWidget(m_btnAnnuler);
    layoutPrincipal->addLayout(boutonsLayout);
}

void BoiteDialogProduit::chargerCategories()
{
    if (!m_gestionnaire)
        return;

    m_comboCategorie->clear();
    m_categories = m_gestionnaire->obtenirTousCategoriesProduits();
    for (const auto& cat : m_categories)
        m_comboCategorie->addItem(cat.getNom(), cat.getCategorieProduitId().toString());
    if (m_categories.isEmpty())
        QMessageBox::warning(this, "Attention", "Aucune catégorie disponible. Créez d'abord une catégorie.");
}

void BoiteDialogProduit::chargerProduit(const Produit& produit)
{
    m_modeModification = true;
    m_produit = produit;
    m_produitId = produit.getProduitId();
    m_editNom->setText(produit.getNom());
    m_editSku->setText(produit.getCodeSku());
    m_spinPrix->setValue(produit.getPrixUnitaire());
    m_spinStockMin->setValue(produit.getStockMinimum());
    m_editDescription->setText(produit.getDescription());
    int idx = m_comboCategorie->findData(produit.getCategorieProduitId().toString());
    if (idx >= 0) m_comboCategorie->setCurrentIndex(idx);
    m_editSku->setReadOnly(true);
    setWindowTitle("Modifier Produit");
}

void BoiteDialogProduit::onSkuChanged(const QString& texte)
{
    QString skuFormate = texte.toUpper();
    skuFormate.remove(QRegularExpression("[^A-Z0-9_-]"));

    if (skuFormate != texte) {
        m_editSku->blockSignals(true);
        m_editSku->setText(skuFormate);
        m_editSku->blockSignals(false);
    }
}

void BoiteDialogProduit::onValiderClicked()
{
    QString nom = m_editNom->text().trimmed();
    QString sku = m_editSku->text().trimmed();
    double prix = m_spinPrix->value();
    int stockMin = m_spinStockMin->value();
    QString description = m_editDescription->toPlainText().trimmed();

    if (nom.isEmpty() || sku.isEmpty() || m_comboCategorie->currentIndex()<0 || prix<=0) {
        QMessageBox::warning(this, "Erreur", "Champs obligatoires manquants ou invalides.");
        return;
    }
    QUuid categorieId(m_comboCategorie->currentData().toString());
    Produit produit;
    produit.setNom(nom);
    produit.setCodeSku(sku);
    produit.setCategorieProduitId(categorieId);
    produit.setPrixUnitaire(prix);
    produit.setStockMinimum(stockMin);
    produit.setDescription(description);

    bool ok = false;
    if (m_modeModification) {
        produit.setProduitId(m_produitId);
        ok = m_gestionnaire->mettreAJourProduit(produit);
    } else {
        ok = m_gestionnaire->creerProduit(produit);
    }
    if (!ok) {
        QMessageBox::critical(this, "Erreur", m_gestionnaire->obtenirDernierErreur());
        return;
    }
    accept();
}