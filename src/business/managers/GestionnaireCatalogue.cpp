#include "GestionnaireCatalogue.h"
#include "../../core/entities/Produit.h"
#include "../../core/entities/CategorieProduit.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../data/repositories/RepositoryCategorieProduit.h"
#include <QDebug>

GestionnaireCatalogue::GestionnaireCatalogue()
    : m_repositoryProduit(nullptr),
      m_repositoryCategorieProduit(nullptr)
{
    qDebug() << "[GESTIONNAIRE CATALOGUE] Initialisation";
}

GestionnaireCatalogue::~GestionnaireCatalogue()
{
    qDebug() << "[GESTIONNAIRE CATALOGUE] Destruction";
}

void GestionnaireCatalogue::setRepositoryProduit(RepositoryProduit* repo)
{
    m_repositoryProduit = repo;
    qDebug() << "[GESTIONNAIRE CATALOGUE] Repository Produit défini";
}

void GestionnaireCatalogue::setRepositoryCategorieProduit(RepositoryCategorieProduit* repo)
{
    m_repositoryCategorieProduit = repo;
    qDebug() << "[GESTIONNAIRE CATALOGUE] Repository Catégorie défini";
}

// ════════════════════════════════════════════════════════════════
// GESTION DES CATÉGORIES
// ════════════════════════════════════════════════════════════════

bool GestionnaireCatalogue::creerCategorie(const CategorieProduit& categorie)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return false;
    }

    if (!validerCategorie(categorie)) {
        return false;
    }

    if (!verifierUniciteCodeCategorie(categorie.getCodeCategorie())) {
        m_dernierErreur = "Ce code catégorie existe déjà";
        return false;
    }

    if (m_repositoryCategorieProduit->create(categorie)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Catégorie créée:" << categorie.getNom();
        return true;
    }

    m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::mettreAJourCategorie(const CategorieProduit& categorie)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return false;
    }

    if (!validerCategorie(categorie)) {
        return false;
    }

    if (!verifierUniciteCodeCategorie(categorie.getCodeCategorie(), categorie.getCategorieProduitId())) {
        m_dernierErreur = "Ce code catégorie existe déjà";
        return false;
    }

    if (m_repositoryCategorieProduit->update(categorie)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Catégorie mise à jour:" << categorie.getNom();
        return true;
    }

    m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::supprimerCategorie(const QUuid& categorieId)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return false;
    }

    // Vérifier qu'il n'y a pas de produits dans cette catégorie
    QList<Produit> produits = obtenirProduitsParCategorie(categorieId);
    if (!produits.isEmpty()) {
        m_dernierErreur = QString("Impossible de supprimer : %1 produit(s) dans cette catégorie").arg(produits.size());
        return false;
    }

    if (m_repositoryCategorieProduit->remove(categorieId)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Catégorie supprimée:" << categorieId.toString();
        return true;
    }

    m_dernierErreur = m_repositoryCategorieProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::changerStatutCategorie(const QUuid& categorieId, bool actif)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return false;
    }

    CategorieProduit categorie = obtenirCategorie(categorieId);
    if (categorie.getCategorieProduitId().isNull()) {
        m_dernierErreur = "Catégorie non trouvée";
        return false;
    }

    categorie.setEstActif(actif);
    return mettreAJourCategorie(categorie);
}

CategorieProduit GestionnaireCatalogue::obtenirCategorie(const QUuid& categorieId)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return CategorieProduit();
    }

    return m_repositoryCategorieProduit->getById(categorieId);
}

CategorieProduit GestionnaireCatalogue::obtenirCategorieParCode(const QString& code)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return CategorieProduit();
    }

    return m_repositoryCategorieProduit->getByCode(code);
}

QList<CategorieProduit> GestionnaireCatalogue::obtenirTousCategoriesProduits()
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return QList<CategorieProduit>();
    }

    return m_repositoryCategorieProduit->getAll();
}

QList<CategorieProduit> GestionnaireCatalogue::rechercherCategories(const QString& critere)
{
    if (!m_repositoryCategorieProduit) {
        m_dernierErreur = "Repository catégorie non initialisé";
        return QList<CategorieProduit>();
    }

    return m_repositoryCategorieProduit->search(critere);
}

// ════════════════════════════════════════════════════════════════
// GESTION DES PRODUITS
// ════════════════════════════════════════════════════════════════

bool GestionnaireCatalogue::creerProduit(const Produit& produit)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return false;
    }

    if (!validerProduit(produit)) {
        return false;
    }

    if (!verifierUniciteSkuProduit(produit.getCodeSku())) {
        m_dernierErreur = "Ce SKU existe déjà";
        return false;
    }

    // Vérifier que la catégorie existe
    if (!m_repositoryCategorieProduit->exists(produit.getCategorieProduitId())) {
        m_dernierErreur = "La catégorie sélectionnée n'existe pas";
        return false;
    }

    if (m_repositoryProduit->create(produit)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Produit créé:" << produit.getNom();
        return true;
    }

    m_dernierErreur = m_repositoryProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::mettreAJourProduit(const Produit& produit)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return false;
    }

    if (!validerProduit(produit)) {
        return false;
    }

    if (!verifierUniciteSkuProduit(produit.getCodeSku(), produit.getProduitId())) {
        m_dernierErreur = "Ce SKU existe déjà";
        return false;
    }

    if (m_repositoryProduit->update(produit)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Produit mis à jour:" << produit.getNom();
        return true;
    }

    m_dernierErreur = m_repositoryProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::supprimerProduit(const QUuid& produitId)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return false;
    }

    if (m_repositoryProduit->remove(produitId)) {
        qDebug() << "[GESTIONNAIRE CATALOGUE] Produit supprimé:" << produitId.toString();
        return true;
    }

    m_dernierErreur = m_repositoryProduit->getLastError();
    return false;
}

bool GestionnaireCatalogue::changerStatutProduit(const QUuid& produitId, bool actif)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return false;
    }

    Produit produit = obtenirProduit(produitId);
    if (produit.getProduitId().isNull()) {
        m_dernierErreur = "Produit non trouvé";
        return false;
    }

    produit.setEstActif(actif);
    return mettreAJourProduit(produit);
}

Produit GestionnaireCatalogue::obtenirProduit(const QUuid& produitId)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return Produit();
    }

    return m_repositoryProduit->getById(produitId);
}

Produit GestionnaireCatalogue::obtenirProduitParSKU(const QString& sku)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return Produit();
    }

    return m_repositoryProduit->getByCodeSku(sku);
}

QList<Produit> GestionnaireCatalogue::obtenirTousProduits()
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return QList<Produit>();
    }

    return m_repositoryProduit->getAll();
}

QList<Produit> GestionnaireCatalogue::obtenirProduitsParCategorie(const QUuid& categorieId)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return QList<Produit>();
    }

    return m_repositoryProduit->getByCategorie(categorieId);
}

QList<Produit> GestionnaireCatalogue::rechercherProduits(const QString& critere)
{
    if (!m_repositoryProduit) {
        m_dernierErreur = "Repository produit non initialisé";
        return QList<Produit>();
    }

    return m_repositoryProduit->search(critere);
}

// ════════════════════════════════════════════════════════════════
// VALIDATION ET VÉRIFICATIONS
// ════════════════════════════════════════════════════════════════

bool GestionnaireCatalogue::validerCatalogue()
{
    qDebug() << "[GESTIONNAIRE CATALOGUE] Validation du catalogue...";
    
    if (!m_repositoryProduit || !m_repositoryCategorieProduit) {
        m_dernierErreur = "Repositories non initialisés";
        return false;
    }

    QList<Produit> produits = obtenirTousProduits();
    for (const Produit& produit : produits) {
        if (!m_repositoryCategorieProduit->exists(produit.getCategorieProduitId())) {
            m_dernierErreur = QString("Produit '%1' référence une catégorie inexistante").arg(produit.getNom());
            return false;
        }
    }

    qDebug() << "[GESTIONNAIRE CATALOGUE] Validation réussie";
    return true;
}

int GestionnaireCatalogue::obtenirNombreProduits()
{
    return obtenirTousProduits().size();
}

int GestionnaireCatalogue::obtenirNombreCategories()
{
    return obtenirTousCategoriesProduits().size();
}

// ════════════════════════════════════════════════════════════════
// MÉTHODES PRIVÉES DE VALIDATION
// ════════════════════════════════════════════════════════════════

bool GestionnaireCatalogue::validerProduit(const Produit& produit)
{
    // Vérifier le nom
    if (produit.getNom().trimmed().isEmpty()) {
        m_dernierErreur = "Le nom du produit ne peut pas être vide";
        return false;
    }

    if (produit.getNom().length() > 100) {
        m_dernierErreur = "Le nom du produit ne peut pas dépasser 100 caractères";
        return false;
    }

    // Vérifier le SKU
    if (produit.getCodeSku().trimmed().isEmpty()) {
        m_dernierErreur = "Le SKU ne peut pas être vide";
        return false;
    }

    if (produit.getCodeSku().length() > 50) {
        m_dernierErreur = "Le SKU ne peut pas dépasser 50 caractères";
        return false;
    }

    // Vérifier le prix
    if (produit.getPrixUnitaire() <= 0) {
        m_dernierErreur = "Le prix doit être strictement positif";
        return false;
    }

    // Vérifier le stock minimum
    if (produit.getStockMinimum() < 0) {
        m_dernierErreur = "Le stock minimum ne peut pas être négatif";
        return false;
    }

    return true;
}

bool GestionnaireCatalogue::validerCategorie(const CategorieProduit& categorie)
{
    // Vérifier le nom
    if (categorie.getNom().trimmed().isEmpty()) {
        m_dernierErreur = "Le nom de la catégorie ne peut pas être vide";
        return false;
    }

    if (categorie.getNom().length() > 100) {
        m_dernierErreur = "Le nom de la catégorie ne peut pas dépasser 100 caractères";
        return false;
    }

    // Vérifier le code
    if (categorie.getCodeCategorie().trimmed().isEmpty()) {
        m_dernierErreur = "Le code catégorie ne peut pas être vide";
        return false;
    }

    if (categorie.getCodeCategorie().length() > 20) {
        m_dernierErreur = "Le code catégorie ne peut pas dépasser 20 caractères";
        return false;
    }

    return true;
}

bool GestionnaireCatalogue::verifierUniciteCodeCategorie(const QString& code, const QUuid& categorieIdExclu)
{
    if (!m_repositoryCategorieProduit) {
        return false;
    }

    CategorieProduit existante = obtenirCategorieParCode(code);
    if (!existante.getCategorieProduitId().isNull()) {
        if (categorieIdExclu.isNull() || existante.getCategorieProduitId() != categorieIdExclu) {
            return false;
        }
    }

    return true;
}

bool GestionnaireCatalogue::verifierUniciteSkuProduit(const QString& sku, const QUuid& produitIdExclu)
{
    if (!m_repositoryProduit) {
        return false;
    }

    Produit existant = obtenirProduitParSKU(sku);
    if (!existant.getProduitId().isNull()) {
        if (produitIdExclu.isNull() || existant.getProduitId() != produitIdExclu) {
            return false;
        }
    }

    return true;
}