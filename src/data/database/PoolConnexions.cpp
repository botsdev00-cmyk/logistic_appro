#include "PoolConnexions.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QMutexLocker>

int PoolConnexions::m_compteurInstances = 0;

PoolConnexions::PoolConnexions(int maxConnexions)
    : m_port(5432),
      m_maxConnexions(maxConnexions),
      m_compteurConnexions(0)
{
    qDebug() << "[PoolConnexions] Initialisation avec" << maxConnexions << "connexions max";
}

PoolConnexions::~PoolConnexions()
{
    qDebug() << "[PoolConnexions] Fermeture du pool";
    fermerTout();
}

bool PoolConnexions::initialiser(
    const QString& host,
    int port,
    const QString& database,
    const QString& user,
    const QString& password)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "[PoolConnexions] Initialisation:" << host << ":" << port << "/" << database;

    m_host = host;
    m_port = port;
    m_database = database;
    m_user = user;
    m_password = password;

    // Créer les connexions initiales
    for (int i = 0; i < m_maxConnexions; ++i) {
        QSqlDatabase connexion = creerConnexion();
        if (!connexion.isValid() || !connexion.open()) {
            m_lastError = "Impossible de créer la connexion " + QString::number(i + 1);
            qCritical() << "[ERREUR]" << m_lastError;
            return false;
        }
        m_poolConnexions.enqueue(connexion);
    }

    qDebug() << "[OK] Pool initialisé avec" << m_poolConnexions.size() << "connexions";
    return true;
}

QSqlDatabase PoolConnexions::obtenirConnexion()
{
    QMutexLocker locker(&m_mutex);

    if (m_poolConnexions.isEmpty()) {
        m_lastError = "Aucune connexion disponible dans le pool";
        qWarning() << "[WARNING]" << m_lastError;
        return QSqlDatabase();
    }

    QSqlDatabase connexion = m_poolConnexions.dequeue();

    // Vérifier que la connexion est encore valide
    if (!testerConnexion(connexion)) {
        qWarning() << "[WARNING] Connexion invalide, création d'une nouvelle";
        connexion = creerConnexion();
        if (!connexion.isValid() || !connexion.open()) {
            m_lastError = "Impossible de créer une nouvelle connexion";
            qCritical() << "[ERREUR]" << m_lastError;
            return QSqlDatabase();
        }
    }

    m_compteurConnexions++;
    qDebug() << "[OK] Connexion obtenue du pool. Connexions actives:" << m_compteurConnexions;

    return connexion;
}

void PoolConnexions::libererConnexion(const QSqlDatabase& connexion)
{
    QMutexLocker locker(&m_mutex);

    if (!connexion.isValid()) {
        qWarning() << "[WARNING] Tentative de libérer une connexion invalide";
        return;
    }

    m_poolConnexions.enqueue(connexion);
    m_compteurConnexions--;

    qDebug() << "[OK] Connexion libérée. Connexions actives:" << m_compteurConnexions;
}

void PoolConnexions::fermerTout()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "[PoolConnexions] Fermeture de toutes les connexions";

    while (!m_poolConnexions.isEmpty()) {
        QSqlDatabase connexion = m_poolConnexions.dequeue();
        if (connexion.isOpen()) {
            connexion.close();
        }
    }

    qDebug() << "[OK] Toutes les connexions fermées";
}

int PoolConnexions::getNombreConnexionsActives() const
{
    QMutexLocker locker(&m_mutex);
    return m_compteurConnexions;
}

int PoolConnexions::getNombreConnexionsDisponibles() const
{
    QMutexLocker locker(&m_mutex);
    return m_poolConnexions.size();
}

QSqlDatabase PoolConnexions::creerConnexion()
{
    QString nomConnexion = "SEMULIKI_DB_" + QString::number(m_compteurInstances++);

    QSqlDatabase connexion = QSqlDatabase::addDatabase("QPSQL", nomConnexion);
    connexion.setHostName(m_host);
    connexion.setPort(m_port);
    connexion.setDatabaseName(m_database);
    connexion.setUserName(m_user);
    connexion.setPassword(m_password);
    connexion.setConnectOptions("application_name=SEMULIKI_ERP_POOL");

    qDebug() << "[OK] Nouvelle connexion créée:" << nomConnexion;

    return connexion;
}

bool PoolConnexions::testerConnexion(const QSqlDatabase& connexion)
{
    if (!connexion.isOpen()) {
        return false;
    }

    QSqlQuery query(connexion);
    if (!query.exec("SELECT 1")) {
        qWarning() << "[WARNING] Test de connexion échoué:" << query.lastError().text();
        return false;
    }

    return true;
}