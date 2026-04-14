#include "ConnexionBaseDonnees.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>

std::unique_ptr<ConnexionBaseDonnees> ConnexionBaseDonnees::m_instance = nullptr;

ConnexionBaseDonnees::ConnexionBaseDonnees()
{
}

ConnexionBaseDonnees::~ConnexionBaseDonnees()
{
    fermer();
}

ConnexionBaseDonnees& ConnexionBaseDonnees::getInstance()
{
    if (!m_instance) {
        m_instance = std::unique_ptr<ConnexionBaseDonnees>(new ConnexionBaseDonnees());
    }
    return *m_instance;
}

bool ConnexionBaseDonnees::initialiser(const QString& host, const QString& port,
                                       const QString& database, const QString& user,
                                       const QString& password)
{
    m_database = QSqlDatabase::addDatabase("QPSQL");
    m_database.setHostName(host);
    m_database.setPort(port.toInt());
    m_database.setDatabaseName(database);
    m_database.setUserName(user);
    m_database.setPassword(password);

    if (!m_database.open()) {
        m_dernierErreur = "Erreur de connexion : " + m_database.lastError().text();
        qDebug() << m_dernierErreur;
        return false;
    }

    // Ajoute ces lignes :
    qDebug() << "[DEBUG DB] Connexion à la base de données RÉUSSIE !";
    qDebug() << "[DEBUG DB] DB =" << m_database.databaseName()
             << "Host =" << m_database.hostName()
             << "Port =" << m_database.port()
             << "User =" << m_database.userName();

    return true;
}

bool ConnexionBaseDonnees::estConnecte() const
{
    return m_database.isOpen();
}

void ConnexionBaseDonnees::fermer()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "Connexion fermée";
    }
}

bool ConnexionBaseDonnees::executerMigration(const QString& cheminSQL)
{
    QFile fichier(cheminSQL);
    if (!fichier.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_dernierErreur = "Impossible d'ouvrir le fichier : " + cheminSQL;
        qDebug() << m_dernierErreur;
        return false;
    }

    QString contenu = fichier.readAll();
    fichier.close();

    QSqlQuery query(m_database);
    if (!query.exec(contenu)) {
        m_dernierErreur = "Erreur de migration : " + query.lastError().text();
        qDebug() << m_dernierErreur;
        return false;
    }

    qDebug() << "Migration exécutée avec succès";
    return true;
}