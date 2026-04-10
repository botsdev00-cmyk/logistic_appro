#include "BoiteDialogConnexion.h"
#include "../../business/services/ServiceAuthentification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QPixmap>
#include <QIcon>

BoiteDialogConnexion::BoiteDialogConnexion(QWidget* parent)
    : QDialog(parent),
      m_authService(std::make_unique<ServiceAuthentification>())
{
    setWindowTitle("SEMULIKI ERP - Connexion");
    setWindowIcon(QIcon(":/images/icon.png"));
    setModal(true);
    setFixedSize(400, 350);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    creerWidgets();
    initialiserConnexions();
    configurerStyleSheet();
}

BoiteDialogConnexion::~BoiteDialogConnexion()
{
}

void BoiteDialogConnexion::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(20);
    layoutPrincipal->setContentsMargins(30, 30, 30, 30);
    
    // Logo/Titre
    QLabel* labelTitre = new QLabel("SEMULIKI ERP");
    labelTitre->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    labelTitre->setAlignment(Qt::AlignCenter);
    layoutPrincipal->addWidget(labelTitre);
    
    QLabel* labelSous = new QLabel("Système de Gestion Logistique");
    labelSous->setAlignment(Qt::AlignCenter);
    labelSous->setStyleSheet("color: #7f8c8d;");
    layoutPrincipal->addWidget(labelSous);
    
    layoutPrincipal->addSpacing(20);
    
    // Utilisateur
    QLabel* labelUtilisateur = new QLabel("Nom d'utilisateur:");
    layoutPrincipal->addWidget(labelUtilisateur);
    
    m_champUtilisateur = std::make_unique<QLineEdit>();
    m_champUtilisateur->setPlaceholderText("Entrez votre nom d'utilisateur");
    m_champUtilisateur->setMinimumHeight(40);
    layoutPrincipal->addWidget(m_champUtilisateur.get());
    
    // Mot de passe
    QLabel* labelMotDePasse = new QLabel("Mot de passe:");
    layoutPrincipal->addWidget(labelMotDePasse);
    
    QHBoxLayout* layoutMotDePasse = new QHBoxLayout();
    m_champMotDePasse = std::make_unique<QLineEdit>();
    m_champMotDePasse->setPlaceholderText("Entrez votre mot de passe");
    m_champMotDePasse->setEchoMode(QLineEdit::Password);
    m_champMotDePasse->setMinimumHeight(40);
    layoutMotDePasse->addWidget(m_champMotDePasse.get());
    
    m_boutonAfficherMotDePasse = std::make_unique<QPushButton>("👁");
    m_boutonAfficherMotDePasse->setMaximumWidth(40);
    m_boutonAfficherMotDePasse->setMinimumHeight(40);
    layoutMotDePasse->addWidget(m_boutonAfficherMotDePasse.get());
    
    layoutPrincipal->addLayout(layoutMotDePasse);
    
    // Label erreur
    m_labelErreur = std::make_unique<QLabel>();
    m_labelErreur->setStyleSheet("color: red; font-weight: bold;");
    m_labelErreur->setVisible(false);
    layoutPrincipal->addWidget(m_labelErreur.get());
    
    layoutPrincipal->addSpacing(10);
    
    // Boutons
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->setSpacing(10);
    
    m_boutonConnecter = std::make_unique<QPushButton>("Se connecter");
    m_boutonConnecter->setMinimumHeight(45);
    m_boutonConnecter->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; font-weight: bold; border-radius: 5px; }"
        "QPushButton:hover { background-color: #229954; }"
    );
    layoutBoutons->addWidget(m_boutonConnecter.get());
    
    m_boutonAnnuler = std::make_unique<QPushButton>("Annuler");
    m_boutonAnnuler->setMinimumHeight(45);
    m_boutonAnnuler->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; font-weight: bold; border-radius: 5px; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );
    layoutBoutons->addWidget(m_boutonAnnuler.get());
    
    layoutPrincipal->addLayout(layoutBoutons);
}

void BoiteDialogConnexion::initialiserConnexions()
{
    connect(m_boutonConnecter.get(), &QPushButton::clicked, this, &BoiteDialogConnexion::seConnecter);
    connect(m_boutonAnnuler.get(), &QPushButton::clicked, this, &QDialog::reject);
    connect(m_boutonAfficherMotDePasse.get(), &QPushButton::clicked, this, &BoiteDialogConnexion::afficherMotDePasse);
    connect(m_champUtilisateur.get(), &QLineEdit::returnPressed, this, &BoiteDialogConnexion::seConnecter);
    connect(m_champMotDePasse.get(), &QLineEdit::returnPressed, this, &BoiteDialogConnexion::seConnecter);
}

void BoiteDialogConnexion::configurerStyleSheet()
{
    setStyleSheet(
        "QDialog { background-color: #ecf0f1; }"
        "QLineEdit { border: 2px solid #bdc3c7; border-radius: 5px; padding: 5px; }"
        "QLineEdit:focus { border: 2px solid #3498db; }"
        "QLabel { color: #2c3e50; }"
    );
}

void BoiteDialogConnexion::seConnecter()
{
    QString utilisateur = m_champUtilisateur->text();
    QString motDePasse = m_champMotDePasse->text();
    
    if (utilisateur.isEmpty() || motDePasse.isEmpty()) {
        m_labelErreur->setText("Veuillez entrer tous les champs");
        m_labelErreur->setVisible(true);
        return;
    }
    
    if (m_authService->authenticate(utilisateur, motDePasse)) {
        m_labelErreur->setVisible(false);
        accept();
    } else {
        m_labelErreur->setText("Erreur: " + m_authService->getLastError());
        m_labelErreur->setVisible(true);
        m_champMotDePasse->clear();
    }
}

void BoiteDialogConnexion::afficherMotDePasse()
{
    if (m_champMotDePasse->echoMode() == QLineEdit::Password) {
        m_champMotDePasse->setEchoMode(QLineEdit::Normal);
        m_boutonAfficherMotDePasse->setText("🔒");
    } else {
        m_champMotDePasse->setEchoMode(QLineEdit::Password);
        m_boutonAfficherMotDePasse->setText("👁");
    }
}

void BoiteDialogConnexion::reinitialiser()
{
    m_champUtilisateur->clear();
    m_champMotDePasse->clear();
    m_labelErreur->setVisible(false);
    m_champUtilisateur->setFocus();
}