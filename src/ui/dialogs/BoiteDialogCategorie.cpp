#include "BoiteDialogCategorie.h"
#include "../../business/managers/GestionnaireCatalogue.h"
#include "../../core/entities/CategorieProduit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

BoiteDialogCategorie::BoiteDialogCategorie(GestionnaireCatalogue* gestionnaire, QWidget* parent)
    : QDialog(parent),
      m_gestionnaireCatalogue(gestionnaire),
      m_champNom(nullptr),
      m_champCode(nullptr),
      m_champDescription(nullptr),
      m_spinOrdre(nullptr),
      m_btnValider(nullptr),
      m_btnAnnuler(nullptr),
      m_modeModification(false),
      m_categorieId(QUuid())
{
    initializeUI();
    setWindowTitle("Nouvelle Catégorie");
    setModal(true);
    setMinimumWidth(450);
}

BoiteDialogCategorie::~BoiteDialogCategorie() {}

void BoiteDialogCategorie::initializeUI()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);

    QLabel* labelNom = new QLabel("Nom de la catégorie :");
    m_champNom = new QLineEdit();
    m_champNom->setPlaceholderText("Ex: Électronique, Alimentaire, etc.");
    m_champNom->setMaxLength(100);

    QLabel* labelCode = new QLabel("Code catégorie (unique) :");
    m_champCode = new QLineEdit();
    m_champCode->setPlaceholderText("Ex: ELEC, ALIM (auto-majuscule)");
    m_champCode->setMaxLength(20);
    connect(m_champCode, &QLineEdit::textChanged, this, &BoiteDialogCategorie::onCodeChanged);

    QLabel* labelDescription = new QLabel("Description :");
    m_champDescription = new QTextEdit();
    m_champDescription->setPlaceholderText("Description détaillée de la catégorie...");
    m_champDescription->setMaximumHeight(80);

    QLabel* labelOrdre = new QLabel("Ordre d'affichage :");
    m_spinOrdre = new QSpinBox();
    m_spinOrdre->setMinimum(0);
    m_spinOrdre->setMaximum(9999);
    m_spinOrdre->setValue(0);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(labelNom, m_champNom);
    formLayout->addRow(labelCode, m_champCode);
    formLayout->addRow(labelDescription, m_champDescription);
    formLayout->addRow(labelOrdre, m_spinOrdre);
    layoutPrincipal->addLayout(formLayout);

    m_btnValider = new QPushButton("✓ Enregistrer");
    connect(m_btnValider, &QPushButton::clicked, this, &BoiteDialogCategorie::onValider);

    m_btnAnnuler = new QPushButton("✕ Annuler");
    connect(m_btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout* boutonsLayout = new QHBoxLayout;
    boutonsLayout->addStretch();
    boutonsLayout->addWidget(m_btnValider);
    boutonsLayout->addWidget(m_btnAnnuler);
    layoutPrincipal->addLayout(boutonsLayout);
}

void BoiteDialogCategorie::chargerCategorie(const CategorieProduit& categorie)
{
    m_modeModification = true;
    m_categorieId = categorie.getCategorieProduitId();
    m_champNom->setText(categorie.getNom());
    m_champCode->setText(categorie.getCodeCategorie());
    m_champDescription->setText(categorie.getDescription());
    m_spinOrdre->setValue(categorie.getOrdreAffichage());
    m_champCode->setReadOnly(true);
    setWindowTitle("Modifier Catégorie");
}

void BoiteDialogCategorie::onCodeChanged(const QString& texte)
{
    QString codeFormate = texte.trimmed().toUpper();
    codeFormate.remove(QRegularExpression("[^A-Z0-9_-]"));
    if (codeFormate != texte) {
        m_champCode->blockSignals(true);
        m_champCode->setText(codeFormate);
        m_champCode->blockSignals(false);
    }
}

void BoiteDialogCategorie::onValider()
{
    QString nom = m_champNom->text().trimmed();
    QString code = m_champCode->text().trimmed().toUpper();
    QString description = m_champDescription->toPlainText().trimmed();
    int ordre = m_spinOrdre->value();

    // ----- Debug étape essentielle -----
    qDebug() << "[DEBUG-CAT] onValider code saisi =" << code;
    qDebug() << "[DEBUG-CAT] m_modeModification=" << m_modeModification << " m_categorieId=" << m_categorieId;

    // ----- Validation -----
    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Le nom de la catégorie ne peut pas être vide");
        m_champNom->setFocus();
        return;
    }
    if (code.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Le code catégorie ne peut pas être vide");
        m_champCode->setFocus();
        return;
    }
    if (nom.length() > 100) {
        QMessageBox::warning(this, "Erreur de validation", "Le nom ne peut pas dépasser 100 caractères");
        m_champNom->setFocus();
        return;
    }
    if (code.length() > 20) {
        QMessageBox::warning(this, "Erreur de validation", "Le code ne peut pas dépasser 20 caractères");
        m_champCode->setFocus();
        return;
    }

    // ----- Vérification d'unicité du code -----
    CategorieProduit existante = m_gestionnaireCatalogue->obtenirCategorieParCode(code);
    qDebug() << "[DEBUG-CAT] existante.id=" << existante.getCategorieProduitId();

    if (!existante.getCategorieProduitId().isNull()
        && (!m_modeModification || existante.getCategorieProduitId() != m_categorieId)) {
        QMessageBox::warning(this, "Erreur de validation", "Ce code catégorie existe déjà");
        m_champCode->setFocus();
        return;
    }

    // ----- (Création/Modification) -----
    CategorieProduit categorie;
    if (m_modeModification) {
        categorie.setCategorieProduitId(m_categorieId);
    }
    categorie.setNom(nom);
    categorie.setCodeCategorie(code);
    categorie.setDescription(description);
    categorie.setOrdreAffichage(ordre);

    bool succes = false;
    if (m_modeModification) {
        succes = m_gestionnaireCatalogue->mettreAJourCategorie(categorie);
        if (succes) {
            QMessageBox::information(this, "Succès", "Catégorie mise à jour avec succès");
        }
    } else {
        succes = m_gestionnaireCatalogue->creerCategorie(categorie);
        if (succes) {
            QMessageBox::information(this, "Succès", "Catégorie créée avec succès");
        }
    }

    if (!succes) {
        QString erreur = m_gestionnaireCatalogue->obtenirDernierErreur();
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'enregistrement :\n" + erreur);
        qDebug() << "[DEBUG-CAT] Erreur lors de l'enregistrement catégorie:" << erreur;
        return;
    }

    accept();
}