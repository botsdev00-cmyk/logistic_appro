#include "Enregistreur.h"
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <iostream>

// Initialisation des membres statiques
QString Enregistreur::m_cheminFichier;
QFile Enregistreur::m_fichierLog;
QMutex Enregistreur::m_mutex;
bool Enregistreur::m_affichageConsole = true;
bool Enregistreur::m_initialise = false;

bool Enregistreur::initialiser(const QString& cheminFichier)
{
    QMutexLocker locker(&m_mutex);

    m_cheminFichier = cheminFichier;

    // Créer le répertoire s'il n'existe pas
    QDir dir(QFileInfo(cheminFichier).absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            std::cerr << "[ERREUR] Impossible de créer le répertoire pour les logs\n";
            return false;
        }
    }

    // Ouvrir le fichier en mode ajout
    m_fichierLog.setFileName(cheminFichier);
    if (!m_fichierLog.open(QIODevice::Append | QIODevice::Text)) {
        std::cerr << "[ERREUR] Impossible d'ouvrir le fichier de log: " 
                  << cheminFichier.toStdString() << "\n";
        return false;
    }

    m_initialise = true;

    // Log de démarrage
    info("═══════════════════════════════════════════════════════════");
    info("  SEMULIKI ERP - Démarrage du système de logging");
    info("  Fichier: " + cheminFichier);
    info("═══════════════════════════════════════════════════════════");

    return true;
}

void Enregistreur::debug(const QString& message)
{
    enregistrer(NiveauLog::DEBUG, message);
}

void Enregistreur::info(const QString& message)
{
    enregistrer(NiveauLog::INFO, message);
}

void Enregistreur::warning(const QString& message)
{
    enregistrer(NiveauLog::WARNING, message);
}

void Enregistreur::critical(const QString& message)
{
    enregistrer(NiveauLog::CRITICAL, message);
}

void Enregistreur::error(const QString& message)
{
    enregistrer(NiveauLog::ERROR, message);
}

void Enregistreur::enregistrer(NiveauLog niveau, const QString& message)
{
    if (!m_initialise) {
        initialiser();
    }

    QMutexLocker locker(&m_mutex);

    QString messageFormate = formaterMessage(niveau, message);

    // Afficher dans la console
    if (m_affichageConsole) {
        switch (niveau) {
            case NiveauLog::DEBUG:
                qDebug().noquote() << messageFormate;
                break;
            case NiveauLog::INFO:
                qInfo().noquote() << messageFormate;
                break;
            case NiveauLog::WARNING:
                qWarning().noquote() << messageFormate;
                break;
            case NiveauLog::CRITICAL:
            case NiveauLog::ERROR:
                qCritical().noquote() << messageFormate;
                break;
        }
    }

    // Écrire dans le fichier
    if (m_fichierLog.isOpen()) {
        QTextStream out(&m_fichierLog);
        out << messageFormate << "\n";
        out.flush();
    }
}

void Enregistreur::fermer()
{
    QMutexLocker locker(&m_mutex);

    if (m_fichierLog.isOpen()) {
        info("═══════════════════════════════════════════════════════════");
        info("  SEMULIKI ERP - Arrêt du système de logging");
        info("═══════════════════════════════════════════════════════════");
        m_fichierLog.close();
    }

    m_initialise = false;
}

void Enregistreur::vider()
{
    QMutexLocker locker(&m_mutex);

    if (m_fichierLog.isOpen()) {
        m_fichierLog.close();
    }

    QFile::remove(m_cheminFichier);
    m_fichierLog.setFileName(m_cheminFichier);
    m_fichierLog.open(QIODevice::WriteOnly | QIODevice::Text);

    info("Fichier de log vidé");
}

QString Enregistreur::getCheminFichier()
{
    QMutexLocker locker(&m_mutex);
    return QFileInfo(m_cheminFichier).absoluteFilePath();
}

void Enregistreur::setAffichageConsole(bool afficher)
{
    QMutexLocker locker(&m_mutex);
    m_affichageConsole = afficher;
}

QString Enregistreur::niveauToString(NiveauLog niveau)
{
    switch (niveau) {
        case NiveauLog::DEBUG:
            return "DEBUG";
        case NiveauLog::INFO:
            return "INFO";
        case NiveauLog::WARNING:
            return "WARNING";
        case NiveauLog::CRITICAL:
            return "CRITICAL";
        case NiveauLog::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

QString Enregistreur::formaterMessage(NiveauLog niveau, const QString& message)
{
    QDateTime dateHeure = QDateTime::currentDateTime();
    QString timestamp = dateHeure.toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString niveau_str = niveauToString(niveau);

    return QString("[%1] [%-8s] %2")
        .arg(timestamp)
        .arg(niveau_str)
        .arg(message);
}