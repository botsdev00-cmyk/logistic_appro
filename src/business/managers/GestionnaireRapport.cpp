#include "GestionnaireRapport.h"
#include "../../data/repositories/RepositorySales.h"
#include "../../data/repositories/RepositoryCredit.h"
#include "../../data/repositories/RepositoryReceptionCaisse.h"
#include "../../data/repositories/RepositoryStock.h"
#include <QDebug>

GestionnaireRapport::GestionnaireRapport()
{
}

QMap<QString, double> GestionnaireRapport::obtenirRapportJournalier(const QDate& date)
{
    QMap<QString, double> rapport;
    
    try {
        // À implémenter avec les statistiques du jour
        rapport["Total Ventes"] = 0.0;
        rapport["Total Cash"] = 0.0;
        rapport["Total Crédit"] = 0.0;
        rapport["Nombre Ventes"] = 0.0;
        
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, double> GestionnaireRapport::obtenirRapportEquipe(const QUuid& equipeId, 
                                                               const QDate& dateDebut, 
                                                               const QDate& dateFin)
{
    QMap<QString, double> rapport;
    
    try {
        rapport["Nombre Repartitions"] = 0.0;
        rapport["Total Ventes"] = 0.0;
        rapport["Total Cash"] = 0.0;
        rapport["Performance"] = 0.0;
        
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, double> GestionnaireRapport::obtenirRapportVentes(const QDate& dateDebut, 
                                                               const QDate& dateFin)
{
    QMap<QString, double> rapport;
    
    try {
        rapport["Total Ventes"] = 0.0;
        rapport["Ventes Cash"] = 0.0;
        rapport["Ventes Crédit"] = 0.0;
        rapport["Panier Moyen"] = 0.0;
        
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, double> GestionnaireRapport::obtenirRapportVentesParCategorie(const QDate& dateDebut, 
                                                                             const QDate& dateFin)
{
    QMap<QString, double> rapport;
    
    try {
        // À implémenter
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, double> GestionnaireRapport::obtenirRapportCredits()
{
    QMap<QString, double> rapport;
    
    try {
        RepositoryCredit repo;
        
        rapport["Total Crédits En Attente"] = repo.getTotalAmount(Credit::Statut::EnAttente);
        rapport["Crédits En Retard"] = 0.0;  // À calculer
        rapport["Crédits Payés"] = repo.getTotalAmount(Credit::Statut::Paye);
        
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, double> GestionnaireRapport::obtenirRapportCaisse(const QDate& dateDebut, 
                                                               const QDate& dateFin)
{
    QMap<QString, double> rapport;
    
    try {
        RepositoryReceptionCaisse repo;
        
        QList<ReceptionCaisse> receptions = repo.getAll();
        
        double totalRecu = 0.0;
        double totalDiscrepance = 0.0;
        int nombreDiscrepances = 0;
        
        for (const auto& reception : receptions) {
            totalRecu += reception.getMontantRecu();
            if (reception.hasDiscrepancy()) {
                totalDiscrepance += qAbs(reception.getEcart());
                nombreDiscrepances++;
            }
        }
        
        rapport["Total Reçu"] = totalRecu;
        rapport["Total Discrepances"] = totalDiscrepance;
        rapport["Nombre Discrepances"] = nombreDiscrepances;
        
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QMap<QString, int> GestionnaireRapport::obtenirRapportStock()
{
    QMap<QString, int> rapport;
    
    try {
        // À implémenter
        return rapport;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapport;
    }
}

QList<QMap<QString, QVariant>> GestionnaireRapport::obtenirRapportClients()
{
    QList<QMap<QString, QVariant>> rapports;
    
    try {
        // À implémenter
        return rapports;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return rapports;
    }
}