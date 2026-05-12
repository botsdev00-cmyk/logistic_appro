#include "GestionnaireCatalogue.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryCategorieProduit.h"
#include "../../core/entities/Produit.h"
#include "../../core/entities/CategorieProduit.h"

GestionnaireCatalogue::GestionnaireCatalogue()
    : m_repositoryProduit(nullptr), m_repositoryCategorieProduit(nullptr)
{}

GestionnaireCatalogue::~GestionnaireCatalogue() {}

void GestionnaireCatalogue::setRepositoryProduit(RepositoryProduit* repo) {
    m_repositoryProduit = repo;
}
void GestionnaireCatalogue::setRepositoryCategorieProduit(RepositoryCategorieProduit* repo) {
    m_repositoryCategorieProduit = repo;
}

bool GestionnaireCatalogue::creerCategorie(const CategorieProduit& categorie) {
    effacerErreur();
    if (!m_repositoryCategorieProduit) return false;
    bool ok = m_repositoryCategorieProduit->create(categorie);
    if (!ok) m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return ok;
}
bool GestionnaireCatalogue::mettreAJourCategorie(const CategorieProduit& categorie) {
    effacerErreur();
    if (!m_repositoryCategorieProduit) return false;
    bool ok = m_repositoryCategorieProduit->update(categorie);
    if (!ok) m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return ok;
}
bool GestionnaireCatalogue::supprimerCategorie(const QUuid& categorieId) {
    effacerErreur();
    if (!m_repositoryCategorieProduit) return false;
    bool ok = m_repositoryCategorieProduit->logicalDelete(categorieId);
    if (!ok) m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return ok;
}
bool GestionnaireCatalogue::changerStatutCategorie(const QUuid& categorieId, bool actif) {
    auto optCat = m_repositoryCategorieProduit->getById(categorieId);
    if (!optCat) return false;
    CategorieProduit cat = *optCat;
    cat.setEstActif(actif);
    return m_repositoryCategorieProduit->update(cat);
}

CategorieProduit GestionnaireCatalogue::obtenirCategorie(const QUuid& categorieId) {
    auto optCat = m_repositoryCategorieProduit->getById(categorieId);
    if (!optCat) return CategorieProduit();
    return *optCat;
}
CategorieProduit GestionnaireCatalogue::obtenirCategorieParCode(const QString& code) {
    auto optCat = m_repositoryCategorieProduit->getByCode(code);
    if (!optCat) return CategorieProduit();
    return *optCat;
}
QList<CategorieProduit> GestionnaireCatalogue::obtenirTousCategoriesProduits() {
    if (!m_repositoryCategorieProduit) return {};
    return m_repositoryCategorieProduit->getAll();
}
QList<CategorieProduit> GestionnaireCatalogue::rechercherCategories(const QString& critere) {
    if (!m_repositoryCategorieProduit) return {};
    return m_repositoryCategorieProduit->search(critere);
}

bool GestionnaireCatalogue::creerProduit(const Produit& produit) {
    effacerErreur();
    if (!m_repositoryProduit) return false;
    bool ok = m_repositoryProduit->create(produit);
    if (!ok) m_dernierErreur = m_repositoryProduit->getLastError();
    return ok;
}
bool GestionnaireCatalogue::mettreAJourProduit(const Produit& produit) {
    effacerErreur();
    if (!m_repositoryProduit) return false;
    bool ok = m_repositoryProduit->update(produit);
    if (!ok) m_dernierErreur = m_repositoryProduit->getLastError();
    return ok;
}
bool GestionnaireCatalogue::supprimerProduit(const QUuid& produitId) {
    effacerErreur();
    if (!m_repositoryProduit) return false;
    bool ok = m_repositoryProduit->remove(produitId);
    if (!ok) m_dernierErreur = m_repositoryProduit->getLastError();
    return ok;
}

bool GestionnaireCatalogue::changerStatutProduit(const QUuid& produitId, bool actif) {
    if (!m_repositoryProduit) return false;
    auto optProd = m_repositoryProduit->getById(produitId);
    if (!optProd.has_value()) return false;
    Produit prod = *optProd;
    prod.setEstActif(actif);
    return m_repositoryProduit->update(prod);
}

Produit GestionnaireCatalogue::obtenirProduit(const QUuid& produitId) {
    if (!m_repositoryProduit) return Produit();
    auto optProd = m_repositoryProduit->getById(produitId);
    if (!optProd.has_value()) return Produit();
    return *optProd;
}

Produit GestionnaireCatalogue::obtenirProduitParSKU(const QString& sku) {
    if (!m_repositoryProduit) return Produit();
    auto optProd = m_repositoryProduit->getByCodeSku(sku);
    if (!optProd.has_value()) return Produit();
    return *optProd;
}

QList<Produit> GestionnaireCatalogue::obtenirProduitsParCategorie(const QUuid& categorieId) {
    if (!m_repositoryProduit) return {};
    return m_repositoryProduit->getByCategorie(categorieId);
}


QList<Produit> GestionnaireCatalogue::obtenirTousProduits() {
    if (!m_repositoryProduit) return {};
    return m_repositoryProduit->getAll();
}

QList<Produit> GestionnaireCatalogue::rechercherProduits(const QString& critere) {
    if (!m_repositoryProduit) return {};
    return m_repositoryProduit->search(critere);
}

bool GestionnaireCatalogue::validerCatalogue() { /* ton implémentation */ return true; }
int GestionnaireCatalogue::obtenirNombreProduits() { return obtenirTousProduits().size(); }
int GestionnaireCatalogue::obtenirNombreCategories() { return obtenirTousCategoriesProduits().size(); }
bool GestionnaireCatalogue::validerProduit(const Produit&) { return true; }
bool GestionnaireCatalogue::validerCategorie(const CategorieProduit&) { return true; }
bool GestionnaireCatalogue::verifierUniciteCodeCategorie(const QString&, const QUuid&) { return true; }
bool GestionnaireCatalogue::verifierUniciteSkuProduit(const QString&, const QUuid&) { return true; }
