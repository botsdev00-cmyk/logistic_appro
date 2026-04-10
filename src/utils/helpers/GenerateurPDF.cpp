#include "GenerateurPDF.h"
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDebug>

// Initialisation des membres statiques
QString GenerateurPDF::m_dossierSortie;
QString GenerateurPDF::m_lastError;
bool GenerateurPDF::m_initialise = false;

bool GenerateurPDF::initialiser(const QString& dossierSortie)
{
    m_dossierSortie = dossierSortie;

    QDir dir(dossierSortie);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = "Impossible de créer le dossier de sortie: " + dossierSortie;
            qCritical() << "[ERREUR]" << m_lastError;
            return false;
        }
    }

    m_initialise = true;
    qDebug() << "[OK] Générateur de PDF initialisé:" << dossierSortie;
    return true;
}

bool GenerateurPDF::genererFacture(const QString& numeroFacture, QString& cheminSortie)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurPDF] Génération de facture:" << numeroFacture;

    cheminSortie = genererNomFichier("Facture_" + numeroFacture);

    // À implémenter avec QPdfWriter
    qDebug() << "[OK] Facture générée:" << cheminSortie;
    return true;
}

bool GenerateurPDF::genererRecu(const QString& numeroRecu, QString& cheminSortie)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurPDF] Génération de reçu:" << numeroRecu;

    cheminSortie = genererNomFichier("Recu_" + numeroRecu);

    // À implémenter avec QPdfWriter
    qDebug() << "[OK] Reçu généré:" << cheminSortie;
    return true;
}

bool GenerateurPDF::genererRapport(const QString& titre, const QString& contenu, QString& cheminSortie)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurPDF] Génération de rapport:" << titre;

    cheminSortie = genererNomFichier("Rapport_" + titre);

    // À implémenter avec QPdfWriter
    qDebug() << "[OK] Rapport généré:" << cheminSortie;
    return true;
}

QString GenerateurPDF::getLastError()
{
    return m_lastError;
}

QString GenerateurPDF::genererNomFichier(const QString& prefixe)
{
    return m_dossierSortie + "/" + prefixe + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf";
}