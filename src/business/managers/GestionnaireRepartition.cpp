#include "GestionnaireRepartition.h"
#include "../exceptions/BusinessException.h"
#include "../../data/repositories/RepositoryRepartition.h"
#include "../../data/repositories/RepositorySales.h"
#include "../../core/entities/Repartition.h"
#include "../../core/entities/ArticleRepartition.h"
#include <QDebug>

GestionnaireRepartition::GestionnaireRepartition()
{
}

QUuid GestionnaireRepartition::creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& dateRepartition)
{
    try {
        if (equipeId.isNull() || routeId.isNull() || !dateRepartition.isValid()) {
            m_dernierErreur = "Paramètres invalides";
            return QUuid();
        }
        
        Repartition repartition;
        repartition.setEquipeId(equipeId);
        repartition.setRouteId(routeId);
        repartition.setDateRepartition(dateRepartition);
        repartition.setStatut(Repartition::Statut::EnAttente);
        
        RepositoryRepartition repo;
        if (!repo.create(repartition)) {
            m_dernierErreur = "Erreur lors de la création : " + repo.getLastError();
            return QUuid();
        }
        
        qDebug() << "Répartition créée :" << repartition.getRepartitionId().toString();
        return repartition.getRepartitionId();
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireRepartition::ajouterArticle(const QUuid& repartitionId, const QUuid& produitId, 
                                             int quantiteVente, int quantiteCadeau, int quantiteDegustation)
{
    try {
        if (repartitionId.isNull() || produitId.isNull()) {
            m_dernierErreur = "IDs invalides";
            return false;
        }
        
        if (quantiteVente < 0 || quantiteCadeau < 0 || quantiteDegustation < 0) {
            m_dernierErreur = "Quantités invalides";
            return false;
        }
        
        ArticleRepartition article;
        article.setRepartitionId(repartitionId);
        article.setProduitId(produitId);
        article.setQuantiteVente(quantiteVente);
        article.setQuantiteCadeau(quantiteCadeau);
        article.setQuantiteDegustation(quantiteDegustation);
        
        // Note: Vous aurez besoin d'une méthode create pour ArticleRepartition
        // Pour l'instant, on simule
        
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireRepartition::marquerEnCours(const QUuid& repartitionId)
{
    try {
        RepositoryRepartition repo;
        Repartition repartition = repo.getById(repartitionId);
        
        if (repartition.getRepartitionId().isNull()) {
            m_dernierErreur = "Répartition non trouvée";
            return false;
        }
        
        repartition.setStatut(Repartition::Statut::EnCours);
        return repo.update(repartition);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireRepartition::marquerCompletee(const QUuid& repartitionId)
{
    try {
        RepositoryRepartition repo;
        Repartition repartition = repo.getById(repartitionId);
        
        if (repartition.getRepartitionId().isNull()) {
            m_dernierErreur = "Répartition non trouvée";
            return false;
        }
        
        repartition.setStatut(Repartition::Statut::Completee);
        repartition.setDateRetour(QDate::currentDate());
        return repo.update(repartition);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireRepartition::annulerRepartition(const QUuid& repartitionId)
{
    try {
        RepositoryRepartition repo;
        return repo.remove(repartitionId);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

Repartition GestionnaireRepartition::obtenirRepartition(const QUuid& repartitionId)
{
    RepositoryRepartition repo;
    return repo.getById(repartitionId);
}

QList<Repartition> GestionnaireRepartition::obtenirRepartitionsEnCours()
{
    RepositoryRepartition repo;
    return repo.getByStatut(Repartition::Statut::EnCours);
}

QList<Repartition> GestionnaireRepartition::obtenirRepartitionsParEquipe(const QUuid& equipeId)
{
    RepositoryRepartition repo;
    return repo.getByEquipe(equipeId);
}

int GestionnaireRepartition::obtenirNombreClientsVisites(const QUuid& repartitionId)
{
    // À implémenter avec une requête SQL
    return 0;
}

double GestionnaireRepartition::obtenirMontantVentes(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getTotalVentes(repartitionId);
}

double GestionnaireRepartition::obtenirMontantCash(const QUuid& repartitionId)
{
    RepositorySales repo;
    return repo.getTotalCash(repartitionId);
}

bool GestionnaireRepartition::validerRepartition(const QUuid& repartitionId)
{
    try {
        RepositoryRepartition repo;
        Repartition repartition = repo.getById(repartitionId);
        
        if (repartition.getRepartitionId().isNull()) {
            m_dernierErreur = "Répartition non trouvée";
            return false;
        }
        
        return verifierQuantitesDisponibles(repartitionId);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireRepartition::verifierQuantitesDisponibles(const QUuid& repartitionId)
{
    // À implémenter avec vérification du stock
    return true;
}