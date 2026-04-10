#include "GestionnaireStock.h"
#include "../exceptions/BusinessException.h"
#include "../../data/repositories/RepositoryStock.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../../core/entities/EntreeStock.h"
#include "../../core/entities/Produit.h"
#include "../../core/entities/Utilisateur.h"
#include <QDebug>

GestionnaireStock::GestionnaireStock()
{
}

bool GestionnaireStock::ajouterStock(const QUuid& produitId, int quantite, const QString& source)
{
    try {
        if (!Utilisateur::stringToRole(source).isNull() || quantite <= 0) {
            m_dernierErreur = "Quantité invalide";
            return false;
        }
        
        EntreeStock entree;
        entree.setProduitId(produitId);
        entree.setQuantite(quantite);
        
        RepositoryStock repo;
        if (!repo.create(entree)) {
            m_dernierErreur = "Erreur lors de l'ajout au stock : " + repo.getLastError();
            return false;
        }
        
        qDebug() << "Stock ajouté :" << quantite << "pour le produit" << produitId.toString();
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireStock::retirerStock(const QUuid& produitId, int quantite)
{
    try {
        if (quantite <= 0) {
            m_dernierErreur = "Quantité invalide";
            return false;
        }
        
        if (!verifierDisponibilite(produitId, quantite)) {
            m_dernierErreur = "Stock insuffisant";
            return false;
        }
        
        EntreeStock entree;
        entree.setProduitId(produitId);
        entree.setQuantite(-quantite);
        
        RepositoryStock repo;
        if (!repo.create(entree)) {
            m_dernierErreur = "Erreur lors du retrait du stock";
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireStock::retournerStock(const QUuid& produitId, int quantite, const QString& raison)
{
    try {
        if (quantite <= 0) {
            m_dernierErreur = "Quantité invalide";
            return false;
        }
        
        EntreeStock entree;
        entree.setProduitId(produitId);
        entree.setQuantite(quantite);
        entree.setSource(EntreeStock::Source::Retour);
        entree.setNotes("Raison : " + raison);
        
        RepositoryStock repo;
        if (!repo.create(entree)) {
            m_dernierErreur = "Erreur lors du retour : " + repo.getLastError();
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

int GestionnaireStock::obtenirStockActuel(const QUuid& produitId)
{
    RepositoryStock repo;
    return repo.getTotalQuantiteByProduit(produitId);
}

QList<Produit> GestionnaireStock::obtenirProduitsEnRupture()
{
    RepositoryProduit repo;
    QList<Produit> tousLeProduits = repo.getAll();
    QList<Produit> produitsEnRupture;
    
    for (const auto& produit : tousLeProduits) {
        if (obtenirStockActuel(produit.getProduitId()) <= 0) {
            produitsEnRupture.append(produit);
        }
    }
    
    return produitsEnRupture;
}

QList<Produit> GestionnaireStock::obtenirProduitsBasStock()
{
    RepositoryProduit repo;
    QList<Produit> tousLeProduits = repo.getAll();
    QList<Produit> produitsBasStock;
    
    for (const auto& produit : tousLeProduits) {
        int stockActuel = obtenirStockActuel(produit.getProduitId());
        if (stockActuel > 0 && stockActuel < produit.getStockMinimum()) {
            produitsBasStock.append(produit);
        }
    }
    
    return produitsBasStock;
}

bool GestionnaireStock::ajusterStock(const QUuid& produitId, int nouvelleQuantite, const QString& raison)
{
    try {
        int stockActuel = obtenirStockActuel(produitId);
        int difference = nouvelleQuantite - stockActuel;
        
        if (difference == 0) {
            return true;
        }
        
        EntreeStock entree;
        entree.setProduitId(produitId);
        entree.setQuantite(difference);
        entree.setSource(EntreeStock::Source::Ajustement);
        entree.setNotes("Ajustement : " + raison);
        
        RepositoryStock repo;
        if (!repo.create(entree)) {
            m_dernierErreur = "Erreur lors de l'ajustement";
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireStock::validerQuantiteDisponible(const QUuid& produitId, int quantiteRequise)
{
    return verifierDisponibilite(produitId, quantiteRequise);
}

bool GestionnaireStock::verifierDisponibilite(const QUuid& produitId, int quantite)
{
    int stockActuel = obtenirStockActuel(produitId);
    return stockActuel >= quantite;
}

void GestionnaireStock::enregistrerMouvement(const QUuid& produitId, int quantite, const QString& type)
{
    qDebug() << "Mouvement stock :" << type << "Produit :" << produitId.toString() << "Quantité :" << quantite;
}