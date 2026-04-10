#include "VueRapport.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

VueRapport::VueRapport(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #ecf0f1;");
    creerWidgets();
    initialiserConnexions();
}

VueRapport::~VueRapport()
{
}

void VueRapport::creerWidgets()
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(15);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    // Titre
    QLabel* labelTitre = new QLabel("Rapports et Analytics");
    labelTitre->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    layoutPrincipal->addWidget(labelTitre);
    
    // Groupes de rapports
    QGroupBox* groupRapportsPre = new QGroupBox("Rapports prédéfinis");
    QHBoxLayout* layoutRapportsPre = new QHBoxLayout(groupRapportsPre);
    
    QPushButton* btnJour = new QPushButton("Rapport du jour");
    connect(btnJour, &QPushButton::clicked, this, &VueRapport::genererRapportJour);
    layoutRapportsPre->addWidget(btnJour);
    
    QPushButton* btnMois = new QPushButton("Rapport du mois");
    connect(btnMois, &QPushButton::clicked, this, &VueRapport::genererRapportMois);
    layoutRapportsPre->addWidget(btnMois);
    
    layoutPrincipal->addWidget(groupRapportsPre);
    
    // Rapport personnalisé
    QGroupBox* groupRapportPerso = new QGroupBox("Rapport personnalisé");
    QGridLayout* layoutRapportPerso = new QGridLayout(groupRapportPerso);
    
    layoutRapportPerso->addWidget(new QLabel("Type:"), 0, 0);
    m_comboType = std::make_unique<QComboBox>();
    m_comboType->addItem("Ventes");
    m_comboType->addItem("Stock");
    m_comboType->addItem("Crédits");
    m_comboType->addItem("Caisse");
    m_comboType->addItem("Performance équipes");
    layoutRapportPerso->addWidget(m_comboType.get(), 0, 1);
    
    layoutRapportPerso->addWidget(new QLabel("Début:"), 1, 0);
    m_dateDebut = std::make_unique<QDateEdit>();
    m_dateDebut->setDate(QDate::currentDate());
    layoutRapportPerso->addWidget(m_dateDebut.get(), 1, 1);
    
    layoutRapportPerso->addWidget(new QLabel("Fin:"), 2, 0);
    m_dateFin = std::make_unique<QDateEdit>();
    m_dateFin->setDate(QDate::currentDate());
    layoutRapportPerso->addWidget(m_dateFin.get(), 2, 1);
    
    QPushButton* btnGenerer = new QPushButton("Générer");
    connect(btnGenerer, &QPushButton::clicked, this, &VueRapport::genererRapportPersonnalise);
    layoutRapportPerso->addWidget(btnGenerer, 3, 0, 1, 2);
    
    layoutPrincipal->addWidget(groupRapportPerso);
    
    layoutPrincipal->addStretch();
    
    // Export
    QGroupBox* groupExport = new QGroupBox("Export");
    QHBoxLayout* layoutExport = new QHBoxLayout(groupExport);
    
    QPushButton* btnPDF = new QPushButton("Exporter en PDF");
    connect(btnPDF, &QPushButton::clicked, this, &VueRapport::exporterEnPDF);
    layoutExport->addWidget(btnPDF);
    
    QPushButton* btnCSV = new QPushButton("Exporter en CSV");
    connect(btnCSV, &QPushButton::clicked, this, &VueRapport::exporterEnCSV);
    layoutExport->addWidget(btnCSV);
    
    layoutPrincipal->addWidget(groupExport);
}

void VueRapport::initialiserConnexions()
{
    // Connexions à implémenter
}

void VueRapport::genererRapportJour()
{
    QMessageBox::information(this, "Rapport", "Rapport du jour généré");
}

void VueRapport::genererRapportMois()
{
    QMessageBox::information(this, "Rapport", "Rapport du mois généré");
}

void VueRapport::genererRapportPersonnalise()
{
    QMessageBox::information(this, "Rapport", "Rapport personnalisé généré");
}

void VueRapport::exporterEnPDF()
{
    QMessageBox::information(this, "Export", "Rapport exporté en PDF");
}

void VueRapport::exporterEnCSV()
{
    QMessageBox::information(this, "Export", "Rapport exporté en CSV");
}