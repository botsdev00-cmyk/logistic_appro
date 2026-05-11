#pragma once

#include <QObject>
#include <QList>
#include <QUuid>
#include <QString>
#include <optional>
#include "../core/entities/Client.h"
#include "../core/entities/CategorieProduit.h"
#include "../core/entities/Equipe.h"
#include "../core/entities/ArticleRepartition.h"
#include "../data/repositories/RepositoryClient.h"
#include "../data/repositories/RepositoryCategorieProduit.h"
#include "../data/repositories/RepositoryEquipe.h"
#include "../data/repositories/RepositoryArticleRepartition.h"
#include "../business/managers/GestionnaireClient.h"
#include "../business/managers/GestionnaireEquipe.h"

class OfflineApiServer : public QObject
{
    Q_OBJECT
public:
    static OfflineApiServer* instance();

    // ================= CLIENT =================
    QUuid creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                      const QString& email, const QUuid& routeId, const QString& conditionsPaiement);
    bool modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse);
    bool supprimerClient(const QUuid& clientId); // soft delete
    std::optional<Client> getClient(const QUuid& clientId) const;
    QList<Client> getClientsByRoute(const QUuid& routeId) const;
    QList<Client> getAllClients() const;
    QList<Client> searchClients(const QString& nom) const;
    QList<Client> getPendingClients() const;
    QList<Client> getClientsSinceVersion(int minVersion) const;

    // ============= CATEGORIE PRODUIT =============
    bool creerCategorieProduit(const CategorieProduit& cat);
    bool modifierCategorieProduit(const CategorieProduit& cat);
    bool supprimerCategorieProduit(const QUuid& id);
    std::optional<CategorieProduit> getCategorieProduit(const QUuid& id) const;
    std::optional<CategorieProduit> getCategorieProduitByCode(const QString& code) const;
    QList<CategorieProduit> getAllCategoriesProduit() const;
    QList<CategorieProduit> rechercherCategoriesProduit(const QString& critere) const;
    QList<CategorieProduit> getPendingCategoriesProduit() const;
    QList<CategorieProduit> getCategoriesProduitSinceVersion(int minVersion) const;

    // ================= EQUIPE (exemple) =================
    QUuid creerEquipe(const QString& nom, const QUuid& chefId, const QList<QUuid>& membres, const QString& telephoneChef = "");
    QList<Equipe> getAllEquipes() const;
    QList<Equipe> getPendingEquipes() const;

    // ========== ARTICLE REPARTITION (exemple) ==========
    QList<ArticleRepartition> getArticlesRepartition(const QUuid& repartitionId) const;

    // =============== COMMUN ERROR ====================
    QString getLastErreur() const;

private:
    explicit OfflineApiServer(QObject* parent = nullptr);
    mutable QString m_lastErreur;
    RepositoryClient m_repoClient;
    RepositoryCategorieProduit m_repoCategorie;
    RepositoryEquipe m_repoEquipe;
    RepositoryArticleRepartition m_repoArticle;
    static OfflineApiServer* s_instance;
};
