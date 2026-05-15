#pragma once

#include <QObject>
#include <QList>
#include <QUuid>
#include <QString>
#include <optional>
#include "../core/entities/Client.h"
#include "../core/entities/CategorieProduit.h"
#include "../core/entities/Produit.h"
#include "../core/entities/Equipe.h"
#include "../core/entities/ArticleRepartition.h"
#include "../core/entities/ReceptionCaisse.h"
#include "../core/entities/Credit.h"
#include "../data/repositories/RepositoryClient.h"
#include "../data/repositories/RepositoryCategorieProduit.h"
#include "../data/repositories/RepositoryProduit.h"
#include "../data/repositories/RepositoryEquipe.h"
#include "../data/repositories/RepositoryReceptionCaisse.h"
#include "../data/repositories/RepositoryArticleRepartition.h"
#include "../data/repositories/RepositoryCredit.h"
#include "../business/managers/GestionnaireClient.h"
#include "../business/managers/GestionnaireEquipe.h"
#include "../business/managers/GestionnaireCaisse.h"
#include "../business/managers/GestionnaireCredit.h"


class OfflineApiServer : public QObject
{
    Q_OBJECT
public:
    static OfflineApiServer* instance();

    // ====================== CLIENT  ======================
    QUuid creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                      const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId = QUuid());

    bool modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse, const QString& telephone,
                        const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId = QUuid());

    bool supprimerClient(const QUuid& clientId); // soft delete

    std::optional<Client> getClient(const QUuid& clientId) const;

    QList<Client> getClientsByRoute(const QUuid& routeId) const;
    QList<Client> getAllClients() const;
    QList<Client> searchClients(const QString& nom) const;
    QList<Client> getPendingClients() const;               // offline-first PENDING
    QList<Client> getClientsSinceVersion(int minVersion) const;
    // ============= CATALOGUE PRODUIT (Categories & Produits) =============

    // ---------- CATEGORIES ----------
    bool creerCategorieProduit(const CategorieProduit& categorie);
    bool modifierCategorieProduit(const CategorieProduit& categorie);
    bool supprimerCategorieProduit(const QUuid& categorieId); // Soft delete
    std::optional<CategorieProduit> getCategorieProduit(const QUuid& id) const;
    std::optional<CategorieProduit> getCategorieProduitByCode(const QString& code) const;
    QList<CategorieProduit> getAllCategoriesProduit() const;
    QList<CategorieProduit> rechercherCategoriesProduit(const QString& critere) const;
    QList<CategorieProduit> getPendingCategoriesProduit() const;
    QList<CategorieProduit> getCategoriesProduitSinceVersion(int minVersion) const;

    // ---------- PRODUITS ----------
    bool creerProduit(const Produit& produit);
    bool modifierProduit(const Produit& produit);
    bool supprimerProduit(const QUuid& produitId);            // Soft delete
    std::optional<Produit> getProduit(const QUuid& produitId) const;
    std::optional<Produit> getProduitBySKU(const QString& sku) const;
    QList<Produit> getAllProduits() const;
    QList<Produit> rechercherProduits(const QString& critere) const;
    QList<Produit> getProduitsByCategorie(const QUuid& categorieId) const;
    QList<Produit> getPendingProduits() const;
    QList<Produit> getProduitsSinceVersion(int minVersion) const;

    // ================= EQUIPE =================
    QUuid creerEquipe(const QString& nom, const QString& nomChef, const QUuid& createdBy, bool estActif = true);
    bool modifierEquipe(const Equipe& equipe, const QUuid& updatedBy);
    bool supprimerEquipe(const QUuid& equipeId);

    std::optional<Equipe> getEquipe(const QUuid& equipeId) const;
    QList<Equipe> getAllEquipes() const;
    QList<Equipe> searchEquipes(const QString& criterion) const;
    QList<Equipe> getPendingEquipes() const;
    QList<Equipe> getConflictEquipes() const;
    QList<Equipe> getEquipesSinceVersion(int minVersion) const;

    // ========== ARTICLE REPARTITION (exemple) ==========
    QList<ArticleRepartition> getArticlesRepartition(const QUuid& repartitionId) const;

    // =================== CAISSE =========================

    // Création/mise à jour
    QUuid creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu, const QUuid& caissierId = QUuid(), const QString& notes = "");
    bool enregistrerMontantRecu(const QUuid& receptionId, double montantRecu);
    bool validerReception(const QUuid& receptionId);
    bool supprimerReceptionCaisse(const QUuid& receptionId); // soft delete

    // Lecture/queries
    ReceptionCaisse getReceptionCaisse(const QUuid& receptionId) const;
    ReceptionCaisse getReceptionCaisseParRepartition(const QUuid& repartitionId) const;
    QList<ReceptionCaisse> getAllReceptionsCaisse() const;
    QList<ReceptionCaisse> searchReceptionsCaisse(const QString& critere) const;

    // Offline sync
    QList<ReceptionCaisse> getPendingReceptionsCaisse() const;
    QList<ReceptionCaisse> getConflictReceptionsCaisse() const;
    QList<ReceptionCaisse> getReceptionsCaisseSinceVersion(int minVersion) const;
    bool marquerReceptionCaisseSynced(const QUuid& receptionId, int nouvelleVersion);
    bool marquerReceptionCaisseConflit(const QUuid& receptionId);

    // ================= CREDIT =================
    QUuid creerCredit(const QUuid& venteId, const QUuid& clientId, double montant, const QDate& dateEcheance, const QString& notes = "");
    bool payerCredit(const QUuid& creditId, const QDate& datePaiement);
    bool annulerCredit(const QUuid& creditId); // soft delete

    std::optional<Credit> getCredit(const QUuid& creditId) const;
    QList<Credit> getAllCredits() const;
    QList<Credit> searchCredits(const QString& critere) const;
    QList<Credit> getCreditsByClient(const QUuid& clientId) const;
    QList<Credit> getOverdueCredits() const;
    QList<Credit> getPendingCredits() const;
    QList<Credit> getCreditsSinceVersion(int minVersion) const;

    double getTotalCreditsEnAttente() const;
    double getTotalCreditsEnRetard() const;
    double getTotalCreditsClient(const QUuid& clientId) const;

    QList<Credit> getCreditsEnAlerte() const;

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
