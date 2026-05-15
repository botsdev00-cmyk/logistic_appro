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
                                    const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId)
{
    GestionnaireClient gest;
    QUuid clientId = gest.creerClient(nom, adresse, telephone, email, routeId, conditionPaiementId, grilleId);
    m_lastErreur = gest.getDernierErreur();
    return clientId;
}

bool OfflineApiServer::modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse, const QString& telephone,
                                      const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId)
{
    GestionnaireClient gest;
    bool ok = gest.modifierClient(clientId, nom, adresse, telephone, email, routeId, conditionPaiementId, grilleId);
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
    // On ne set pas m_lastErreur car const, gestion mutabilité possible si besoin futur
    return client;
}

QList<Client> OfflineApiServer::getClientsByRoute(const QUuid& routeId) const
{
    return m_repoClient.getByRoute(routeId);
}

QList<Client> OfflineApiServer::getAllClients() const
{
    return m_repoClient.getAll();
}

QList<Client> OfflineApiServer::searchClients(const QString& nom) const
{
    return m_repoClient.search(nom);
}

QList<Client> OfflineApiServer::getPendingClients() const
{
    return m_repoClient.getPendingSync();
}

QList<Client> OfflineApiServer::getClientsSinceVersion(int minVersion) const
{
    return m_repoClient.getSinceVersion(minVersion);
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

// ================= EQUIPE =================

QUuid OfflineApiServer::creerEquipe(const QString& nom, const QString& nomChef, const QUuid& createdBy, bool estActif)
{
    GestionnaireEquipe gest;
    QUuid equipeId = gest.creerEquipe(nom, nomChef, createdBy, estActif);
    m_lastErreur = gest.getDernierErreur();
    return equipeId;
}

bool OfflineApiServer::modifierEquipe(const Equipe& equipe, const QUuid& updatedBy)
{
    GestionnaireEquipe gest;
    bool ok = gest.modifierEquipe(equipe, updatedBy);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::supprimerEquipe(const QUuid& equipeId)
{
    GestionnaireEquipe gest;
    bool ok = gest.supprimerEquipe(equipeId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

std::optional<Equipe> OfflineApiServer::getEquipe(const QUuid& equipeId) const
{
    RepositoryEquipe repo;
    auto eq = repo.getById(equipeId);
    // pas de set m_lastErreur ici (const)
    return eq;
}

QList<Equipe> OfflineApiServer::getAllEquipes() const
{
    return m_repoEquipe.getAll();
}

QList<Equipe> OfflineApiServer::searchEquipes(const QString& criterion) const
{
    return m_repoEquipe.search(criterion);
}

QList<Equipe> OfflineApiServer::getPendingEquipes() const
{
    return m_repoEquipe.getPendingEquipes();
}

QList<Equipe> OfflineApiServer::getConflictEquipes() const
{
    return m_repoEquipe.getConflictEquipes();
}

QList<Equipe> OfflineApiServer::getEquipesSinceVersion(int minVersion) const
{
    return m_repoEquipe.getSinceVersion(minVersion);
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

// ================= CREDIT =================

QUuid OfflineApiServer::creerCredit(const QUuid& venteId, const QUuid& clientId, double montant, const QDate& dateEcheance, const QString& notes)
{
    GestionnaireCredit gest;
    QUuid creditId = gest.creerCredit(venteId, clientId, montant, dateEcheance, notes);
    m_lastErreur = gest.getDernierErreur();
    return creditId;
}

bool OfflineApiServer::payerCredit(const QUuid& creditId, const QDate& datePaiement)
{
    GestionnaireCredit gest;
    bool ok = gest.payerCredit(creditId, datePaiement);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

bool OfflineApiServer::annulerCredit(const QUuid& creditId)
{
    GestionnaireCredit gest;
    bool ok = gest.annulerCredit(creditId);
    m_lastErreur = gest.getDernierErreur();
    return ok;
}

std::optional<Credit> OfflineApiServer::getCredit(const QUuid& creditId) const
{
    RepositoryCredit repo;
    auto credit = repo.getById(creditId);
    // pas de m_lastErreur car méthode const, sauf usage mutable
    return credit;
}

QList<Credit> OfflineApiServer::getAllCredits() const
{
    RepositoryCredit repo;
    return repo.getAll();
}

QList<Credit> OfflineApiServer::searchCredits(const QString& critere) const
{
    RepositoryCredit repo;
    return repo.search(critere);
}

QList<Credit> OfflineApiServer::getCreditsByClient(const QUuid& clientId) const
{
    RepositoryCredit repo;
    return repo.getByClient(clientId);
}

QList<Credit> OfflineApiServer::getOverdueCredits() const
{
    RepositoryCredit repo;
    return repo.getOverdueCredits();
}

QList<Credit> OfflineApiServer::getPendingCredits() const
{
    RepositoryCredit repo;
    return repo.getPendingSync();
}

QList<Credit> OfflineApiServer::getCreditsSinceVersion(int minVersion) const
{
    RepositoryCredit repo;
    return repo.getSinceVersion(minVersion);
}

double OfflineApiServer::getTotalCreditsEnAttente() const
{
    RepositoryCredit repo;
    return repo.getTotalAmount(Credit::Statut::EnAttente);
}

double OfflineApiServer::getTotalCreditsEnRetard() const
{
    RepositoryCredit repo;
    QList<Credit> creditsEnRetard = repo.getOverdueCredits();
    double total = 0.0;
    for (const auto& credit : creditsEnRetard)
        total += credit.getMontant();
    return total;
}

double OfflineApiServer::getTotalCreditsClient(const QUuid& clientId) const
{
    RepositoryCredit repo;
    QList<Credit> credits = repo.getByClient(clientId);
    double total = 0.0;
    for (const auto& credit : credits)
        if (credit.getStatut() == Credit::Statut::EnAttente)
            total += credit.getMontant();
    return total;
}

QList<Credit> OfflineApiServer::getCreditsEnAlerte() const
{
    GestionnaireCredit gest;
    return gest.obtenirCreditsAlerte();
}
