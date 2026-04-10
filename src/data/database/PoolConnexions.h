#ifndef POOLCONNEXIONS_H
#define POOLCONNEXIONS_H

#include <QString>
#include <QSqlDatabase>
#include <QQueue>
#include <QMutex>
#include <memory>

/**
 * @class PoolConnexions
 * @brief Gère un pool de connexions PostgreSQL pour améliorer les performances
 */
class PoolConnexions
{
public:
    /**
     * @brief Constructeur du pool de connexions
     * @param maxConnexions Nombre maximum de connexions à maintenir
     */
    PoolConnexions(int maxConnexions = 5);
    
    /**
     * @brief Destructeur - ferme toutes les connexions
     */
    ~PoolConnexions();

    /**
     * @brief Initialise le pool avec les paramètres de connexion
     * @param host Serveur PostgreSQL
     * @param port Port PostgreSQL
     * @param database Nom de la base de données
     * @param user Utilisateur PostgreSQL
     * @param password Mot de passe
     * @return true si succès, false sinon
     */
    bool initialiser(
        const QString& host,
        int port,
        const QString& database,
        const QString& user,
        const QString& password
    );

    /**
     * @brief Obtient une connexion disponible du pool
     * @return QSqlDatabase une connexion prête à l'emploi
     */
    QSqlDatabase obtenirConnexion();

    /**
     * @brief Libère une connexion et la remet dans le pool
     * @param connexion La connexion à libérer
     */
    void libererConnexion(const QSqlDatabase& connexion);

    /**
     * @brief Ferme toutes les connexions du pool
     */
    void fermerTout();

    /**
     * @brief Obtient le nombre de connexions actives
     * @return Nombre de connexions en cours d'utilisation
     */
    int getNombreConnexionsActives() const;

    /**
     * @brief Obtient le nombre de connexions disponibles
     * @return Nombre de connexions dans le pool
     */
    int getNombreConnexionsDisponibles() const;

    /**
     * @brief Obtient le dernier message d'erreur
     * @return Message d'erreur
     */
    QString getLastError() const { return m_lastError; }

private:
    /**
     * @brief Crée une nouvelle connexion
     * @return QSqlDatabase la nouvelle connexion
     */
    QSqlDatabase creerConnexion();

    /**
     * @brief Teste une connexion
     * @param connexion La connexion à tester
     * @return true si la connexion est valide
     */
    bool testerConnexion(const QSqlDatabase& connexion);

    // Paramètres de connexion
    QString m_host;
    int m_port;
    QString m_database;
    QString m_user;
    QString m_password;

    // Pool de connexions
    QQueue<QSqlDatabase> m_poolConnexions;
    int m_maxConnexions;
    int m_compteurConnexions;

    // Synchronisation thread-safe
    mutable QMutex m_mutex;

    // Messages d'erreur
    QString m_lastError;

    // Compteur pour les noms uniques de connexions
    static int m_compteurInstances;
};

#endif // POOLCONNEXIONS_H