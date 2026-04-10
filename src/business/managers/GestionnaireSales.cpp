#include "GestionnaireSales.h"
#include "../exceptions/BusinessException.h"
#include "../../data/repositories/RepositorySales.h"
#include "../../core/entities/Vente.h"
#include <QDebug>

GestionnaireSales::GestionnaireSales()
{
}

QUuid GestionnaireSales::enregistrerVente(const QUuid& repartitionId, const QUuid& produitId, 
                                         const QUuid& clientId, int quantite, 
                                         const QString& typeVente, double prixUnitaire)
{
    try {
        if (repartitionId.isNull() || produitId.isNull() || clientId.isNull()) {
            m_dernierErreur = "IDs invalides";
            return QUuid();
        }
        
        if (quantite <= 0 || prixUnitaire < 0) {
            m_dernierErreur = "Quantité ou prix invalide";
            return QUuid();
        }
        
        Vente vente;
        vente.setRepartitionId(repartitionId);
        vente.setProduitId(produitId);
        vente.setClientId(clientId);
        vente.setQuantite(quantite);
        vente.setTypeVente(Vente::stringToTypeVente(typeVente));
        vente.setPrixUnitaire(prixUnitaire);
        
        RepositorySales repo;
        if (!repo.create(vente)) {
            m_dernierErreur = "Erreur lors de l'enregistrement : " + repo.getLastError();
            return QUuid();
        }
        
        qDebug() << "Vente enregistrée :" << vente.getVenteId().toString();
        return vente.getVenteId();
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireSales::modifierVente(const QUuid& venteId, int nouvelleQuantite, double nouveauPrix)
{
    try {
        if (venteId.isNull() || nouvelleQuantite <= 0 || nouveauPrix < 0) {
            m_dernierErreur = "Paramètres invalides";
            return false;
        }
        
        RepositorySales repo;
        Vente vente = repo.getById(venteId);
        
        if (vente.getVenteId().isNull()) {
            m_dernierErreur = "Vente non trouvée";
            return false;
        }
        
        vente.setQuantite(nouvelleQuantite);
        vente.setPrixUnitaire(nouveauPrix);
        
        return repo.update(vente);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireSales::supprimerVente(const QUuid& venteId)
{
    try {
        RepositorySales repo;
        return repo.remove(venteId);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

Vente GestionnaireSales::obtenirVente(const QUuid& venteId)
{
    RepositorySales repo;
    return repo.getById(venteId);
}

QList<Vente> GestionnaireSales::obtenirVentesRepartition(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getByRepartition(repartitionId);
}

QList<Vente> GestionnaireSales::obtenirVentesClient(const QUuid& clientId)
{
    RepositorySales repo;
    return repo.getByClient(clientId);
}

double GestionnaireSales::obtenirTotalVentes(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getTotalVentes(repartitionId);
}

double GestionnaireSales::obtenirTotalVentesCash(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getTotalCash(repartitionId);
}

double GestionnaireSales::obtenirTotalVentesCredit(const QUuid& repartitionId)
{
    return obtenirTotalVentes(repartitionId) - obtenirTotalVentesCash(repartitionId);
}

int GestionnaireSales::obtenirNombreVentes(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getByRepartition(repartitionId).count();
}

bool GestionnaireSales::validerVente(const QUuid& repartitionId, const QUuid& produitId, int quantite)
{
    // À implémenter avec vérification du stock
    return quantite > 0;
}