#include "BoiteDialogAlerte.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QIcon>

BoiteDialogAlerte::BoiteDialogAlerte(Type type, const QString& titre, const QString& message, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(titre);
    setModal(true);
    setFixedSize(500, 300);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    creerWidgets(titre, message);
    configurerStyle(type);
}

BoiteDialogAlerte::~BoiteDialogAlerte()
{
}

void BoiteDialogAlerte::creerWidgets(const QString& titre, const QString& message)
{
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setSpacing(15);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    
    m_labelTitre = std::make_unique<QLabel>(titre);
    m_labelTitre->setStyleSheet("font-size: 16px; font-weight: bold;");
    layoutPrincipal->addWidget(m_labelTitre.get());
    
    m_textMessage = std::make_unique<QTextEdit>();
    m_textMessage->setText(message);
    m_textMessage->setReadOnly(true);
    m_textMessage->setMaximumHeight(150);
    layoutPrincipal->addWidget(m_textMessage.get());
    
    layoutPrincipal->addStretch();
    
    QHBoxLayout* layoutBoutons = new QHBoxLayout();
    layoutBoutons->addStretch();
    
    m_boutonOK = std::make_unique<QPushButton>("OK");
    m_boutonOK->setMinimumWidth(120);
    layoutBoutons->addWidget(m_boutonOK.get());
    
    layoutPrincipal->addLayout(layoutBoutons);
    
    connect(m_boutonOK.get(), &QPushButton::clicked, this, &QDialog::accept);
}

void BoiteDialogAlerte::configurerStyle(Type type)
{
    QString couleur = "#2c3e50";
    
    switch (type) {
        case Warning:
            couleur = "#f39c12";
            setWindowIcon(QIcon(":/images/warning.png"));
            break;
        case Error:
            couleur = "#e74c3c";
            setWindowIcon(QIcon(":/images/error.png"));
            break;
        case Info:
            couleur = "#3498db";
            setWindowIcon(QIcon(":/images/info.png"));
            break;
    }
    
    m_labelTitre->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(couleur));
    m_boutonOK->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold; padding: 5px;").arg(couleur));
}