#include "GenerateurRapport.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

// Initialisation des membres statiques
QString GenerateurRapport::m_dossierSortie;
QString GenerateurRapport::m_lastError;
bool GenerateurRapport::m_initialise = false;

bool GenerateurRapport::initialiser(const QString& dossierSortie)
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
    qDebug() << "[OK] Générateur de rapports initialisé:" << dossierSortie;
    return true;
}

QString GenerateurRapport::genererRapportVentes(
    const QDate& dateDebut,
    const QDate& dateFin,
    FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport de ventes";

    QString nomFichier = genererNomFichier("Rapport_Ventes_" + dateDebut.toString("yyyyMMdd"),
                                           formatToExtension(format));

    // À implémenter avec les vraies données
    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        qCritical() << "[ERREUR]" << m_lastError;
        return "";
    }

    QTextStream out(&fichier);
    out << "Rapport de Ventes\n";
    out << "Période: " << dateDebut.toString("dd/MM/yyyy") << " à " << dateFin.toString("dd/MM/yyyy") << "\n";
    out << "Généré le: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n";

    fichier.close();
    qDebug() << "[OK] Rapport généré:" << nomFichier;
    return nomFichier;
}

QString GenerateurRapport::genererRapportStock(FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport de stock";

    QString nomFichier = genererNomFichier("Rapport_Stock",
                                           formatToExtension(format));

    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        return "";
    }

    QTextStream out(&fichier);
    out << "Rapport de Stock\n";
    out << "Généré le: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n";

    fichier.close();
    return nomFichier;
}

QString GenerateurRapport::genererRapportCredits(
    const QDate& dateDebut,
    const QDate& dateFin,
    FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport de crédits";

    QString nomFichier = genererNomFichier("Rapport_Credits_" + dateDebut.toString("yyyyMMdd"),
                                           formatToExtension(format));

    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        return "";
    }

    QTextStream out(&fichier);
    out << "Rapport de Crédits\n";
    out << "Période: " << dateDebut.toString("dd/MM/yyyy") << " à " << dateFin.toString("dd/MM/yyyy") << "\n";

    fichier.close();
    return nomFichier;
}

QString GenerateurRapport::genererRapportCaisse(
    const QDate& dateDebut,
    const QDate& dateFin,
    FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport de caisse";

    QString nomFichier = genererNomFichier("Rapport_Caisse_" + dateDebut.toString("yyyyMMdd"),
                                           formatToExtension(format));

    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        return "";
    }

    QTextStream out(&fichier);
    out << "Rapport de Caisse\n";
    out << "Période: " << dateDebut.toString("dd/MM/yyyy") << " à " << dateFin.toString("dd/MM/yyyy") << "\n";

    fichier.close();
    return nomFichier;
}

QString GenerateurRapport::genererRapportPerformanceEquipes(
    const QDate& dateDebut,
    const QDate& dateFin,
    FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport de performance";

    QString nomFichier = genererNomFichier("Rapport_Performance_" + dateDebut.toString("yyyyMMdd"),
                                           formatToExtension(format));

    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        return "";
    }

    QTextStream out(&fichier);
    out << "Rapport de Performance par Équipe\n";
    out << "Période: " << dateDebut.toString("dd/MM/yyyy") << " à " << dateFin.toString("dd/MM/yyyy") << "\n";

    fichier.close();
    return nomFichier;
}

QString GenerateurRapport::genererRapportPersonnalise(
    const QString& titre,
    const QMap<QString, QVariant>& donnees,
    FormatRapport format)
{
    if (!m_initialise) {
        initialiser();
    }

    qDebug() << "[GenerateurRapport] Génération du rapport personnalisé:" << titre;

    QString nomFichier = genererNomFichier("Rapport_" + titre,
                                           formatToExtension(format));

    QFile fichier(nomFichier);
    if (!fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Impossible de créer le fichier: " + nomFichier;
        return "";
    }

    QTextStream out(&fichier);
    out << titre << "\n";
    out << "Généré le: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n\n";

    // Écrire les données
    for (auto it = donnees.begin(); it != donnees.end(); ++it) {
        out << it.key() << ": " << it.value().toString() << "\n";
    }

    fichier.close();
    return nomFichier;
}

QString GenerateurRapport::getLastError()
{
    return m_lastError;
}

bool GenerateurRapport::ouvrirRapport(const QString& cheminFichier)
{
    qDebug() << "[GenerateurRapport] Ouverture du rapport:" << cheminFichier;
    return QDesktopServices::openUrl(QUrl::fromLocalFile(cheminFichier));
}

QString GenerateurRapport::genererNomFichier(const QString& prefixe, const QString& extension)
{
    return m_dossierSortie + "/" + prefixe + "_" + QDateTime::currentDateTime().toString("hhmmss") + extension;
}

QString GenerateurRapport::formatToExtension(FormatRapport format)
{
    switch (format) {
        case FormatRapport::HTML:
            return ".html";
        case FormatRapport::CSV:
            return ".csv";
        case FormatRapport::JSON:
            return ".json";
        case FormatRapport::Texte:
        default:
            return ".txt";
    }
}