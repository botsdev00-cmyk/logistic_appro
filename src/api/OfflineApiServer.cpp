#include "OfflineApiServer.h"
#include "../business/managers/GestionnaireCaisse.h"
#include "../data/repositories/RepositoryReceptionCaisse.h"
#include "../core/entities/ReceptionCaisse.h"
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

// ======== CAISSE ========

QUuid OfflineApiServer::creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu, const QUuid& caissierId, const QString& notes)
{
    GestionnaireCaisse gest;
    QUuid id = gest.creerReceptionCaisse(repartitionId, montantAttendu, caissierId, notes);
    m_lastErreur = gest.getDernierErreur();
    return id;
}

bool OfflineApiServer::enregistrerMontantRecu(const QUuid& receptionId, double montantRecu)
{
    GestionnaireCaisse gest;
    bool ok = gest.enregistrerMontantRecu(receptionId, montantRecu);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::validerReception(const QUuid& receptionId)
{
    GestionnaireCaisse gest;
    bool ok = gest.validerReception(receptionId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::supprimerReceptionCaisse(const QUuid& receptionId)
{
    GestionnaireCaisse gest;
    bool ok = gest.softDeleteReception(receptionId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

ReceptionCaisse OfflineApiServer::getReceptionCaisse(const QUuid& receptionId) const
{
    GestionnaireCaisse gest;
    ReceptionCaisse rc = gest.obtenirReception(receptionId);
    // pas de set m_lastErreur ici car const, sinon utiliser mutable
    return rc;
}

ReceptionCaisse OfflineApiServer::getReceptionCaisseParRepartition(const QUuid& repartitionId) const
{
    GestionnaireCaisse gest;
    ReceptionCaisse rc = gest.obtenirReceptionParRepartition(repartitionId);
    return rc;
}

QList<ReceptionCaisse> OfflineApiServer::getAllReceptionsCaisse() const
{
    RepositoryReceptionCaisse repo;
    return repo.getAll();
}

QList<ReceptionCaisse> OfflineApiServer::searchReceptionsCaisse(const QString& critere) const
{
    RepositoryReceptionCaisse repo;
    return repo.search(critere);
}

QList<ReceptionCaisse> OfflineApiServer::getPendingReceptionsCaisse() const
{
    RepositoryReceptionCaisse repo;
    return repo.getBySyncStatus(ReceptionCaisse::SyncStatus::PENDING);
}

QList<ReceptionCaisse> OfflineApiServer::getConflictReceptionsCaisse() const
{
    RepositoryReceptionCaisse repo;
    return repo.getBySyncStatus(ReceptionCaisse::SyncStatus::CONFLICT);
}

QList<ReceptionCaisse> OfflineApiServer::getReceptionsCaisseSinceVersion(int minVersion) const
{
    RepositoryReceptionCaisse repo;
    return repo.getSinceVersion(minVersion);
}

bool OfflineApiServer::marquerReceptionCaisseSynced(const QUuid& receptionId, int nouvelleVersion)
{
    GestionnaireCaisse gest;
    bool ok = gest.marquerSynced(receptionId, nouvelleVersion);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::marquerReceptionCaisseConflit(const QUuid& receptionId)
{
    GestionnaireCaisse gest;
    bool ok = gest.marquerConflit(receptionId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}
