#include "GestionnaireCatalogue.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryCategorieProduit.h"
#include "../../core/entities/Produit.h"
#include "../../core/entities/CategorieProduit.h"

GestionnaireCatalogue::GestionnaireCatalogue()
    : m_repositoryProduit(nullptr), m_repositoryCategorieProduit(nullptr)
{ }
GestionnaireCatalogue::~GestionnaireCatalogue() {}

void GestionnaireCatalogue::setRepositoryProduit(RepositoryProduit* repo) { m_repositoryProduit = repo; }
void GestionnaireCatalogue::setRepositoryCategorieProduit(RepositoryCategorieProduit* repo) { m_repositoryCategorieProduit = repo; }

// Catégories
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
    cat.setVersion(cat.getVersion() + 1);
    cat.setSyncStatus(CategorieProduit::SyncStatus::PENDING);
    return m_repositoryCategorieProduit->update(cat);
}
std::optional<CategorieProduit> GestionnaireCatalogue::obtenirCategorie(const QUuid& categorieId) {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->getById(categorieId) : std::nullopt;
}
std::optional<CategorieProduit> GestionnaireCatalogue::obtenirCategorieParCode(const QString& code) {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->getByCode(code) : std::nullopt;
}
QList<CategorieProduit> GestionnaireCatalogue::obtenirTousCategoriesProduits() {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->getAll() : QList<CategorieProduit>{};
}
QList<CategorieProduit> GestionnaireCatalogue::rechercherCategories(const QString& critere) {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->search(critere) : QList<CategorieProduit>{};
}
QList<CategorieProduit> GestionnaireCatalogue::obtenirCategoriesASynchroniser() {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->getPendingSync() : QList<CategorieProduit>{};
}
QList<CategorieProduit> GestionnaireCatalogue::obtenirCategoriesDepuisVersion(int minVersion) {
    return m_repositoryCategorieProduit ? m_repositoryCategorieProduit->getSinceVersion(minVersion) : QList<CategorieProduit>{};
}

// Produits
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
    bool ok = m_repositoryProduit->logicalDelete(produitId);
    if (!ok) m_dernierErreur = m_repositoryProduit->getLastError();
    return ok;
}

bool GestionnaireCatalogue::changerStatutProduit(const QUuid& produitId, bool actif) {
    if (!m_repositoryProduit) return false;
    auto optProd = m_repositoryProduit->getById(produitId);
    if (!optProd.has_value()) return false;
    Produit prod = *optProd;
    prod.setEstActif(actif);
    prod.setVersion(prod.getVersion() + 1);
    prod.setSyncStatus(Produit::SyncStatus::PENDING);
    return m_repositoryProduit->update(prod);
}

std::optional<Produit> GestionnaireCatalogue::obtenirProduit(const QUuid& produitId) {
    return m_repositoryProduit ? m_repositoryProduit->getById(produitId) : std::nullopt;
}
std::optional<Produit> GestionnaireCatalogue::obtenirProduitParSKU(const QString& sku) {
    return m_repositoryProduit ? m_repositoryProduit->getByCodeSku(sku) : std::nullopt;
}
QList<Produit> GestionnaireCatalogue::obtenirTousProduits() {
    return m_repositoryProduit ? m_repositoryProduit->getAll() : QList<Produit>{};
}
QList<Produit> GestionnaireCatalogue::obtenirProduitsParCategorie(const QUuid& categorieId) {
    return m_repositoryProduit ? m_repositoryProduit->getByCategorie(categorieId) : QList<Produit>{};
}
QList<Produit> GestionnaireCatalogue::rechercherProduits(const QString& critere) {
    return m_repositoryProduit ? m_repositoryProduit->search(critere) : QList<Produit>{};
}
QList<Produit> GestionnaireCatalogue::obtenirProduitsASynchroniser() {
    return m_repositoryProduit ? m_repositoryProduit->getPendingSync() : QList<Produit>{};
}
QList<Produit> GestionnaireCatalogue::obtenirProduitsDepuisVersion(int minVersion) {
    return m_repositoryProduit ? m_repositoryProduit->getSinceVersion(minVersion) : QList<Produit>{};
}

int GestionnaireCatalogue::obtenirNombreProduits() {
    return obtenirTousProduits().size();
}
int GestionnaireCatalogue::obtenirNombreCategories() {
    return obtenirTousCategoriesProduits().size();
}
