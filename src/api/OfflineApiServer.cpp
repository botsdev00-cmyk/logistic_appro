#include "OfflineApiServer.h"
#include <QMutex>
#include <QMutexLocker>

// ------------------- Singleton ---------------------

OfflineApiServer* OfflineApiServer::s_instance = nullptr;

OfflineApiServer* OfflineApiServer::instance() {
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (!s_instance) s_instance = new OfflineApiServer();
    return s_instance;
}

OfflineApiServer::OfflineApiServer(QObject* parent)
    : QObject(parent),
    m_lastErreur(),
    m_repoClient(),
    m_repoCategorie(),
    m_repoEquipe(),
    m_repoArticle()
{}

// ================= CLIENT =================

QUuid OfflineApiServer::creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                                    const QString& email, const QUuid& routeId, const QString& conditionsPaiement)
{
    GestionnaireClient gest;
    QUuid clientId = gest.creerClient(nom, adresse, telephone, email, routeId, conditionsPaiement);
    m_lastErreur = gest.getDernierErreur();
    return clientId;
}

bool OfflineApiServer::modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse)
{
    GestionnaireClient gest;
    bool ok = gest.modifierClient(clientId, nom, adresse);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::supprimerClient(const QUuid& clientId)
{
    GestionnaireClient gest;
    bool ok = gest.desactiverClient(clientId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

std::optional<Client> OfflineApiServer::getClient(const QUuid& clientId) const
{
    auto client = m_repoClient.getById(clientId);
    m_lastErreur = m_repoClient.getLastError();
    return client;
}

QList<Client> OfflineApiServer::getClientsByRoute(const QUuid& routeId) const
{
    auto list = m_repoClient.getByRoute(routeId);
    m_lastErreur = m_repoClient.getLastError();
    return list;
}

QList<Client> OfflineApiServer::getAllClients() const
{
    auto list = m_repoClient.getAll();
    m_lastErreur = m_repoClient.getLastError();
    return list;
}

QList<Client> OfflineApiServer::searchClients(const QString& nom) const
{
    auto list = m_repoClient.search(nom);
    m_lastErreur = m_repoClient.getLastError();
    return list;
}

QList<Client> OfflineApiServer::getPendingClients() const
{
    auto list = m_repoClient.getPendingSync();
    m_lastErreur = m_repoClient.getLastError();
    return list;
}

QList<Client> OfflineApiServer::getClientsSinceVersion(int minVersion) const
{
    auto list = m_repoClient.getSinceVersion(minVersion);
    m_lastErreur = m_repoClient.getLastError();
    return list;
}

// ============= CATEGORIE PRODUIT =============

bool OfflineApiServer::creerCategorieProduit(const CategorieProduit& cat)
{
    bool ok = m_repoCategorie.create(cat);
    m_lastErreur = m_repoCategorie.getLastError();
    return ok;
}

bool OfflineApiServer::modifierCategorieProduit(const CategorieProduit& cat)
{
    bool ok = m_repoCategorie.update(cat);
    m_lastErreur = m_repoCategorie.getLastError();
    return ok;
}

bool OfflineApiServer::supprimerCategorieProduit(const QUuid& id)
{
    bool ok = m_repoCategorie.logicalDelete(id);
    m_lastErreur = m_repoCategorie.getLastError();
    return ok;
}

std::optional<CategorieProduit> OfflineApiServer::getCategorieProduit(const QUuid& id) const
{
    auto cat = m_repoCategorie.getById(id);
    m_lastErreur = m_repoCategorie.getLastError();
    return cat;
}

std::optional<CategorieProduit> OfflineApiServer::getCategorieProduitByCode(const QString& code) const
{
    auto cat = m_repoCategorie.getByCode(code);
    m_lastErreur = m_repoCategorie.getLastError();
    return cat;
}

QList<CategorieProduit> OfflineApiServer::getAllCategoriesProduit() const
{
    auto list = m_repoCategorie.getAll();
    m_lastErreur = m_repoCategorie.getLastError();
    return list;
}

QList<CategorieProduit> OfflineApiServer::rechercherCategoriesProduit(const QString& critere) const
{
    auto list = m_repoCategorie.search(critere);
    m_lastErreur = m_repoCategorie.getLastError();
    return list;
}

QList<CategorieProduit> OfflineApiServer::getPendingCategoriesProduit() const
{
    auto list = m_repoCategorie.getPendingSync();
    m_lastErreur = m_repoCategorie.getLastError();
    return list;
}

QList<CategorieProduit> OfflineApiServer::getCategoriesProduitSinceVersion(int minVersion) const
{
    auto list = m_repoCategorie.getSinceVersion(minVersion);
    m_lastErreur = m_repoCategorie.getLastError();
    return list;
}

// ==================== EQUIPE (exemple minimal) ====================

QUuid OfflineApiServer::creerEquipe(const QString& nom, const QString& nomChef, /*const QString& description,*/ const QUuid& createdBy)
{
    GestionnaireEquipe gest;
    QUuid equipeId = gest.creerEquipe(nom, nomChef, /*description,*/ createdBy);
    m_lastErreur = gest.getDernierErreur();
    return equipeId;
}

QList<Equipe> OfflineApiServer::getAllEquipes() const
{
    auto list = m_repoEquipe.getAll();
    m_lastErreur = m_repoEquipe.getLastError();
    return list;
}

QList<Equipe> OfflineApiServer::getPendingEquipes() const
{
    auto list = m_repoEquipe.getPendingEquipes();
    m_lastErreur = m_repoEquipe.getLastError();
    return list;
}

// ================ ARTICLE REPARTITION ==================

QList<ArticleRepartition> OfflineApiServer::getArticlesRepartition(const QUuid& repartitionId) const
{
    auto list = m_repoArticle.getByRepartitionId(repartitionId);
    m_lastErreur = m_repoArticle.getLastError();
    return list;
}

// =============== ERREURS ==============
QString OfflineApiServer::getLastErreur() const
{
    return m_lastErreur;
}
