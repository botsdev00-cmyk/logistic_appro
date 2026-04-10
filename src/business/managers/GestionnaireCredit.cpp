#include "GestionnaireCredit.h"
#include "../exceptions/BusinessException.h"
#include "../../data/repositories/RepositoryCredit.h"
#include "../../core/entities/Credit.h"
#include <QDebug>

GestionnaireCredit::GestionnaireCredit()
{
}

QUuid GestionnaireCredit::creerCredit(const QUuid& venteId, const QUuid& clientId, 
                                      double montant, const QDate& dateEcheance)
{
    try {
        if (venteId.isNull() || clientId.isNull()) {
            m_dernierErreur = "IDs invalides";
            return QUuid();
        }
        
        if (montant <= 0 || !dateEcheance.isValid()) {
            m_dernierErreur = "Montant ou date invalide";
            return QUuid();
        }
        
        Credit credit;
        credit.setVenteId(venteId);
        credit.setClientId(clientId);
        credit.setMontant(montant);
        credit.setDateEcheance(dateEcheance);
        credit.setStatut(Credit::Statut::EnAttente);
        
        RepositoryCredit repo;
        if (!repo.create(credit)) {
            m_dernierErreur = "Erreur lors de la cr��ation : " + repo.getLastError();
            return QUuid();
        }
        
        qDebug() << "Crédit créé :" << credit.getCreditId().toString();
        return credit.getCreditId();
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireCredit::payerCredit(const QUuid& creditId, const QDate& datePaiement)
{
    try {
        RepositoryCredit repo;
        Credit credit = repo.getById(creditId);
        
        if (credit.getCreditId().isNull()) {
            m_dernierErreur = "Crédit non trouvé";
            return false;
        }
        
        credit.setStatut(Credit::Statut::Paye);
        credit.setDatePaiement(datePaiement);
        
        return repo.update(credit);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireCredit::annulerCredit(const QUuid& creditId)
{
    try {
        RepositoryCredit repo;
        return repo.remove(creditId);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

Credit GestionnaireCredit::obtenirCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    return repo.getById(creditId);
}

QList<Credit> GestionnaireCredit::obtenirCreditsClient(const QUuid& clientId)
{
    RepositoryCredit repo;
    return repo.getByClient(clientId);
}

QList<Credit> GestionnaireCredit::obtenirCreditsEnRetard()
{
    RepositoryCredit repo;
    return repo.getOverdueCredits();
}

QList<Credit> GestionnaireCredit::obtenirCreditEnAttente()
{
    RepositoryCredit repo;
    return repo.getByStatut(Credit::Statut::EnAttente);
}

double GestionnaireCredit::obtenirTotalCreditsEnAttente()
{
    RepositoryCredit repo;
    return repo.getTotalAmount(Credit::Statut::EnAttente);
}

double GestionnaireCredit::obtenirTotalCreditsEnRetard()
{
    RepositoryCredit repo;
    QList<Credit> creditsEnRetard = repo.getOverdueCredits();
    
    double total = 0.0;
    for (const auto& credit : creditsEnRetard) {
        total += credit.getMontant();
    }
    
    return total;
}

double GestionnaireCredit::obtenirTotalCreditsClient(const QUuid& clientId)
{
    RepositoryCredit repo;
    QList<Credit> credits = repo.getByClient(clientId);
    
    double total = 0.0;
    for (const auto& credit : credits) {
        if (credit.getStatut() == Credit::Statut::EnAttente) {
            total += credit.getMontant();
        }
    }
    
    return total;
}

bool GestionnaireCredit::validerCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    Credit credit = repo.getById(creditId);
    
    if (credit.getCreditId().isNull()) {
        m_dernierErreur = "Crédit non trouvé";
        return false;
    }
    
    mettreAJourStatutCredit(creditId);
    return true;
}

QList<Credit> GestionnaireCredit::obtenirCreditsAlerte()
{
    QList<Credit> creditsEnRetard = obtenirCreditsEnRetard();
    QList<Credit> creditsAlerte;
    
    for (const auto& credit : creditsEnRetard) {
        if (credit.getJoursRetard() > 5) {
            creditsAlerte.append(credit);
        }
    }
    
    return creditsAlerte;
}

void GestionnaireCredit::mettreAJourStatutCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    Credit credit = repo.getById(creditId);
    
    if (credit.estEnRetard()) {
        credit.setStatut(Credit::Statut::EnRetard);
        repo.update(credit);
    }
}