#ifndef GESTIONNAIRE_STOCK_H
#define GESTIONNAIRE_STOCK_H

#include <QList>
#include <QUuid>
#include <QString>
#include <QMap>
#include <QDate>
#include <memory>

class EntreeStock;
class RetourStock;
class StockSolde;
class RepositoryEntreeStock;
class RepositoryRetourStock;
class RepositoryStockSoldes;
class RepositoryStockMouvements;
class ServicePermissions;

// ============================================================================
// STRUCTURES DE DONNÉES ENRICHIES
// ============================================================================

struct StockInfo {
    QUuid produitId;
    QString produitNom;
    QString codeSKU;
    QString categorie;
    QString type;
    int quantiteTotal;
    int quantiteReservee;
    int quantiteDisponible;
    double valeurStock;
    double prixMoyen;
    int stockMinimum;
    QString statut; // OK, FAIBLE, CRITIQUE, RUPTURE
    QDateTime dernierMouvement;
};

struct Mouvement {
    QString type;               // ENTREE, SORTIE, RETOUR, AJUSTEMENT
    QString raison;
    QString nomProduit;
    QString codeSKU;
    int quantiteDelta;
    int quantiteApres;
    QString nomUtilisateur;
    QString dateCreation;
    QString source;
};

struct Alerte {
    QUuid produitId;
    QString produitNom;
    QString codeSKU;
    int quantiteDisponible;
    int stockMinimum;
    QString severite;           // LOW, CRITICAL, RUPTURE
    double valeurStock;
    QString action;             // Recommandation
};

struct StatistiquesStock {
    int nombreProduitsTotal;
    int nombreProduitsEnStock;
    int nombreProduitsEnRupture;
    int nombreProduitsBasStock;
    double valeurTotalStock;
    double valeurMoyenneProduit;
    int quantiteTotalUnites;
    int quantiteReserveeUnites;
    int quantiteDisponibleUnites;
};

// ✅ NOUVEAU: Structure pour stocks par location
struct StockParLocation {
    QUuid produitId;
    QString produitNom;
    int warehouse;
    int inTransit;
    int returned;
    int total;
};

// ✅ NOUVEAU: Structure pour réconciliation
struct ReconciliationResult {
    QUuid produitId;
    QString produitNom;
    int stockFromMovements;
    int stockInSoldes;
    int difference;
    bool isConsistent;
    QString status;
};

// ============================================================================
// GESTIONNAIRE PRINCIPAL
// ============================================================================

class GestionnaireStock
{
public:
    GestionnaireStock();
    ~GestionnaireStock();

    // ====== SETTERS REPOSITORIES ======
    void setRepositoryEntreeStock(RepositoryEntreeStock* repo);
    void setRepositoryRetourStock(RepositoryRetourStock* repo);
    void setRepositoryStockSoldes(RepositoryStockSoldes* repo);
    void setRepositoryStockMouvements(RepositoryStockMouvements* repo);
    void setServicePermissions(ServicePermissions* service);

    // ====== GESTION DES ENTRÉES ======
    bool creerEntreeStock(const EntreeStock& entree);
    bool creerEntreeStockAvecPermission(const EntreeStock& entree, const QUuid& utilisateurId);
    bool approuverEntree(const QUuid& entreeId, const QUuid& utilisateurId);
    bool approuverEntreeAvecPermission(const QUuid& entreeId, const QUuid& utilisateurId);
    bool rejeterEntree(const QUuid& entreeId);
    QList<EntreeStock> obtenirEntreesEnAttente();
    QList<EntreeStock> obtenirHistoriqueEntrees(const QUuid& produitId = QUuid(), int joursArriere = 30);
    EntreeStock obtenirEntree(const QUuid& entreeId);
    bool supprimerEntree(const QUuid& entreeId);

    // ====== GESTION DES RETOURS ======
    bool creerRetourStock(const RetourStock& retour);
    bool creerRetourStockAvecPermission(const RetourStock& retour, const QUuid& utilisateurId);
    bool approuverRetour(const QUuid& retourId, const QUuid& utilisateurId);
    bool approuverRetourAvecPermission(const QUuid& retourId, const QUuid& utilisateurId);
    bool rejeterRetour(const QUuid& retourId);
    QList<RetourStock> obtenirRetoursEnAttente();
    QList<RetourStock> obtenirHistoriqueRetours(const QUuid& produitId = QUuid(), int joursArriere = 30);
    RetourStock obtenirRetour(const QUuid& retourId);
    bool supprimerRetour(const QUuid& retourId);

    // ====== CONSULTATION STOCK ======
    int obtenirQuantiteDisponible(const QUuid& produitId);
    int obtenirQuantiteReservee(const QUuid& produitId);
    int obtenirQuantiteTotal(const QUuid& produitId);
    StockInfo obtenirStockDetail(const QUuid& produitId);
    QList<StockInfo> obtenirTousLesStocks();
    QList<StockInfo> obtenirStocksPaginee(int page = 1, int parPage = 50);
    QList<StockInfo> rechercherStocks(const QString& critere);
    QList<StockInfo> filtrerStocksParStatut(const QString& statut);
    QList<StockInfo> filtrerStocksParCategorie(const QUuid& categorieId);

    // ✅ NOUVEAU: Stocks par location
    StockParLocation obtenirStockParLocation(const QUuid& produitId);
    QList<StockParLocation> obtenirTousStocksParLocation();

    // ====== MOUVEMENTS ======
    QList<Mouvement> obtenirMouvementsRecents(const QUuid& produitId = QUuid(), int jours = 30);
    QList<Mouvement> obtenirHistoriqueProduit(const QUuid& produitId);
    QList<Mouvement> obtenirMouvementsParType(const QString& type);
    QList<Mouvement> obtenirMouvementsParDateRange(const QDate& dateDebut, const QDate& dateFin);
    int obtenirNombreMouvements(const QUuid& produitId = QUuid());

    // ====== ALERTES ======
    QList<Alerte> obtenirAlertes();
    QList<Alerte> obtenirAlertesCritiques();
    QList<Alerte> obtenirAlertesBasStock();
    QList<Alerte> obtenirAlerteRupture();
    bool genererAlertes();
    int obtenirNombreAlertes();
    int obtenirNombreAlertesComme(const QString& severite);

    // ====== VALIDATION ======
    bool validerDisponibilite(const QUuid& produitId, int quantiteRequise);
    bool validerEntree(const EntreeStock& entree);
    bool validerRetour(const RetourStock& retour);
    QString obtenirErreurValidation();

    // ✅ NOUVEAU: Réconciliation stock
    QList<ReconciliationResult> verifierIntegriteStock();
    bool repairerIntegriteStock();
    ReconciliationResult verifierIntegriteProduit(const QUuid& produitId);

    // ====== TRANSACTIONS ======
    bool reserverStock(const QUuid& repartitionId, const QUuid& produitId, int quantite);
    bool confirmerConsommation(const QUuid& repartitionId, const QUuid& produitId, int quantite);
    bool annulerReservation(const QUuid& repartitionId, const QUuid& produitId);
    bool synchroniserStockSoldes();

    // ====== STATISTIQUES ======
    StatistiquesStock obtenirStatistiques();
    QMap<QString, int> obtenirStatutistiquesParCategorie();
    QMap<QString, double> obtenirValeurParCategorie();
    double obtenirValeurTotalStock();
    double obtenirValeurMoyenneProduit();
    int obtenirRotationStock(const QUuid& produitId, int jours = 30);

    // ====== RAPPORTS ======
    QString genererRapportStock();
    QString genererRapportAlertes();
    QString genererRapportMouvements(const QDate& dateDebut, const QDate& dateFin);
    QString genererRapportReconciliation();

    // ====== GESTION DES ERREURS ======
    QString obtenirDernierErreur() const { return m_dernierErreur; }
    void effacerDernierErreur() { m_dernierErreur.clear(); }

private:
    // Repositories
    RepositoryEntreeStock* m_repoEntrees;
    RepositoryRetourStock* m_repoRetours;
    RepositoryStockSoldes* m_repoSoldes;
    RepositoryStockMouvements* m_repoMouvements;
    ServicePermissions* m_servicePermissions;

    // Cache des erreurs
    QString m_dernierErreur;

    // ====== MÉTHODES INTERNES ======
    bool validerQuantite(int quantite);
    bool validerProduit(const QUuid& produitId);
    QString determinerStatutStock(int quantiteDisponible, int stockMinimum);
    double calculerValeurStock(int quantite, double prixMoyen);
    QString genererRecommandation(const Alerte& alerte);
};

#endif // GESTIONNAIRE_STOCK_H