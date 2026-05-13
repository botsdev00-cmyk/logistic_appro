#ifndef GESTIONNAIRECATALOGUE_H
#define GESTIONNAIRECATALOGUE_H

#include <QString>
#include <QList>
#include <QUuid>
#include <memory>
#include "../../core/entities/Produit.h"
#include "../../core/entities/CategorieProduit.h"

class RepositoryProduit;
class RepositoryCategorieProduit;

class GestionnaireCatalogue
{
public:
    GestionnaireCatalogue();
    ~GestionnaireCatalogue();

    void setRepositoryProduit(RepositoryProduit* repo);
    void setRepositoryCategorieProduit(RepositoryCategorieProduit* repo);

    // ══ Catégories
    bool creerCategorie(const CategorieProduit& categorie);
    bool mettreAJourCategorie(const CategorieProduit& categorie);
    bool supprimerCategorie(const QUuid& categorieId);
    bool changerStatutCategorie(const QUuid& categorieId, bool actif);

    std::optional<CategorieProduit> obtenirCategorie(const QUuid& categorieId);
    std::optional<CategorieProduit> obtenirCategorieParCode(const QString& code);
    QList<CategorieProduit> obtenirTousCategoriesProduits();
    QList<CategorieProduit> rechercherCategories(const QString& critere);

    QList<CategorieProduit> obtenirCategoriesASynchroniser();
    QList<CategorieProduit> obtenirCategoriesDepuisVersion(int minVersion);

    // ══ Produits
    bool creerProduit(const Produit& produit);
    bool mettreAJourProduit(const Produit& produit);
    bool supprimerProduit(const QUuid& produitId);
    bool changerStatutProduit(const QUuid& produitId, bool actif);

    std::optional<Produit> obtenirProduit(const QUuid& produitId);
    std::optional<Produit> obtenirProduitParSKU(const QString& sku);
    QList<Produit> obtenirTousProduits();
    QList<Produit> obtenirProduitsParCategorie(const QUuid& categorieId);
    QList<Produit> rechercherProduits(const QString& critere);

    QList<Produit> obtenirProduitsASynchroniser();
    QList<Produit> obtenirProduitsDepuisVersion(int minVersion);

    QString obtenirDernierErreur() const { return m_dernierErreur; }
    void effacerErreur() { m_dernierErreur.clear(); }
    int obtenirNombreProduits();
    int obtenirNombreCategories();

private:
    RepositoryProduit* m_repositoryProduit;
    RepositoryCategorieProduit* m_repositoryCategorieProduit;
    QString m_dernierErreur;
};

#endif // GESTIONNAIRECATALOGUE_H
