#ifndef GESTIONNAIRECATALOGUE_H
#define GESTIONNAIRECATALOGUE_H

#include <QString>
#include <QList>
#include <QUuid>
#include <memory>

class Produit;
class CategorieProduit;
class RepositoryProduit;
class RepositoryCategorieProduit;

class GestionnaireCatalogue
{
public:
    GestionnaireCatalogue();
    ~GestionnaireCatalogue();

    // Setters pour les repositories
    void setRepositoryProduit(RepositoryProduit* repo);
    void setRepositoryCategorieProduit(RepositoryCategorieProduit* repo);

    // ════════════════════════════════════════════════════════════════
    // GESTION DES CATÉGORIES
    // ════════════════════════════════════════════════════════════════

    bool creerCategorie(const CategorieProduit& categorie);
    bool mettreAJourCategorie(const CategorieProduit& categorie);
    bool supprimerCategorie(const QUuid& categorieId);
    bool changerStatutCategorie(const QUuid& categorieId, bool actif);

    CategorieProduit obtenirCategorie(const QUuid& categorieId);
    CategorieProduit obtenirCategorieParCode(const QString& code);
    QList<CategorieProduit> obtenirTousCategoriesProduits();
    QList<CategorieProduit> rechercherCategories(const QString& critere);

    // ════════════════════════════════════════════════════════════════
    // GESTION DES PRODUITS
    // ════════════════════════════════════════════════════════════════

    bool creerProduit(const Produit& produit);
    bool mettreAJourProduit(const Produit& produit);
    bool supprimerProduit(const QUuid& produitId);
    bool changerStatutProduit(const QUuid& produitId, bool actif);

    Produit obtenirProduit(const QUuid& produitId);
    Produit obtenirProduitParSKU(const QString& sku);
    QList<Produit> obtenirTousProduits();
    QList<Produit> obtenirProduitsParCategorie(const QUuid& categorieId);
    QList<Produit> rechercherProduits(const QString& critere);

    // ════════════════════════════════════════════════════════════════
    // VALIDATION ET VÉRIFICATIONS
    // ════════════════════════════════════════════════════════════════

    bool validerCatalogue();
    int obtenirNombreProduits();
    int obtenirNombreCategories();

    // ════════════════════════════════════════════════════════════════
    // GESTION DES ERREURS
    // ════════════════════════════════════════════════════════════════

    QString obtenirDernierErreur() const { return m_dernierErreur; }
    void effacerErreur() { m_dernierErreur.clear(); }

private:
    RepositoryProduit* m_repositoryProduit;
    RepositoryCategorieProduit* m_repositoryCategorieProduit;
    QString m_dernierErreur;

    // Méthodes privées de validation
    bool validerProduit(const Produit& produit);
    bool validerCategorie(const CategorieProduit& categorie);
    bool verifierUniciteCodeCategorie(const QString& code, const QUuid& categorieIdExclu = QUuid());
    bool verifierUniciteSkuProduit(const QString& sku, const QUuid& produitIdExclu = QUuid());
};

#endif // GESTIONNAIRECATALOGUE_H