#ifndef ENREGISTREUR_H
#define ENREGISTREUR_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

/**
 * @class Enregistreur
 * @brief Classe singleton pour la gestion centralisée des logs
 * 
 * Fournit des méthodes statiques pour enregistrer des messages de debug,
 * info, avertissement et erreur critique dans le fichier de log et la console.
 */
class Enregistreur
{
public:
    /**
     * @enum NiveauLog
     * @brief Énumération des niveaux de log
     */
    enum class NiveauLog {
        DEBUG,      ///< Messages de débogage
        INFO,       ///< Messages informatifs
        WARNING,    ///< Avertissements
        CRITICAL,   ///< Erreurs critiques
        ERROR       ///< Erreurs générales
    };

    /**
     * @brief Initialise le système de logging
     * @param cheminFichier Chemin du fichier de log (par défaut: logistic_appro.log)
     * @return true si succès, false sinon
     */
    static bool initialiser(const QString& cheminFichier = "logistic_appro.log");

    /**
     * @brief Enregistre un message de débogage
     * @param message Le message à enregistrer
     */
    static void debug(const QString& message);

    /**
     * @brief Enregistre un message d'information
     * @param message Le message à enregistrer
     */
    static void info(const QString& message);

    /**
     * @brief Enregistre un avertissement
     * @param message Le message à enregistrer
     */
    static void warning(const QString& message);

    /**
     * @brief Enregistre une erreur critique
     * @param message Le message à enregistrer
     */
    static void critical(const QString& message);

    /**
     * @brief Enregistre une erreur générale
     * @param message Le message à enregistrer
     */
    static void error(const QString& message);

    /**
     * @brief Enregistre un message personnalisé avec un niveau spécifique
     * @param niveau Le niveau de log
     * @param message Le message à enregistrer
     */
    static void enregistrer(NiveauLog niveau, const QString& message);

    /**
     * @brief Ferme le fichier de log
     */
    static void fermer();

    /**
     * @brief Vide le fichier de log
     */
    static void vider();

    /**
     * @brief Obtient le chemin du fichier de log courant
     * @return Le chemin complet du fichier de log
     */
    static QString getCheminFichier();

    /**
     * @brief Active/désactive l'affichage dans la console
     * @param afficher true pour afficher, false pour masquer
     */
    static void setAffichageConsole(bool afficher);

private:
    /**
     * @brief Constructeur privé (classe singleton)
     */
    Enregistreur() = default;

    /**
     * @brief Convertit un niveau de log en chaîne de caractères
     * @param niveau Le niveau de log
     * @return La chaîne correspondante ("DEBUG", "INFO", etc.)
     */
    static QString niveauToString(NiveauLog niveau);

    /**
     * @brief Formate un message de log avec timestamp et niveau
     * @param niveau Le niveau de log
     * @param message Le message
     * @return Le message formaté
     */
    static QString formaterMessage(NiveauLog niveau, const QString& message);

    // Membres statiques
    static QString m_cheminFichier;
    static QFile m_fichierLog;
    static QMutex m_mutex;
    static bool m_affichageConsole;
    static bool m_initialise;
};

#endif // ENREGISTREUR_H