#include "GestionnaireCredit.h"
#include "../../data/repositories/RepositoryCredit.h"
#include <QDebug>

GestionnaireCredit::GestionnaireCredit() {}

QUuid GestionnaireCredit::creerCredit(const QUuid& venteId, const QUuid& clientId, double montant, const QDate& dateEcheance, const QString& notes)
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
        credit.setNotes(notes);
        credit.setSyncStatus(Credit::SyncStatus::PENDING);
        credit.setVersion(1);

        RepositoryCredit repo;
        if (!repo.create(credit)) {
            m_dernierErreur = "Erreur lors de la création : " + repo.getLastError();
            return QUuid();
        }
        return credit.getCreditId();
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireCredit::payerCredit(const QUuid& creditId, const QDate& datePaiement)
{
    RepositoryCredit repo;
    auto optCredit = repo.getById(creditId);
    if (!optCredit) {
        m_dernierErreur = "Crédit non trouvé";
        return false;
    }
    Credit credit = *optCredit;
    credit.setStatut(Credit::Statut::Paye);
    credit.setDatePaiement(datePaiement);
    credit.setSyncStatus(Credit::SyncStatus::PENDING);
    credit.setVersion(credit.getVersion() + 1);
    return repo.update(credit);
}

bool GestionnaireCredit::annulerCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    return repo.logicalDelete(creditId);
}
Credit GestionnaireCredit::obtenirCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    auto optCredit = repo.getById(creditId);
    if (!optCredit) return Credit();
    return *optCredit;
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
QList<Credit> GestionnaireCredit::obtenirPendingSync()
{
    RepositoryCredit repo;
    return repo.getPendingSync();
}
QList<Credit> GestionnaireCredit::obtenirDepuisVersion(int version)
{
    RepositoryCredit repo;
    return repo.getSinceVersion(version);
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
    for (const auto& credit : creditsEnRetard)
        total += credit.getMontant();
    return total;
}
double GestionnaireCredit::obtenirTotalCreditsClient(const QUuid& clientId)
{
    RepositoryCredit repo;
    QList<Credit> credits = repo.getByClient(clientId);
    double total = 0.0;
    for (const auto& credit : credits)
        if (credit.getStatut() == Credit::Statut::EnAttente)
            total += credit.getMontant();
    return total;
}
bool GestionnaireCredit::validerCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    auto optCredit = repo.getById(creditId);
    if (!optCredit) {
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
    for (const auto& credit : creditsEnRetard)
        if (credit.getJoursRetard() > 5)
            creditsAlerte.append(credit);
    return creditsAlerte;
}
void GestionnaireCredit::mettreAJourStatutCredit(const QUuid& creditId)
{
    RepositoryCredit repo;
    auto optCredit = repo.getById(creditId);
    if (!optCredit) return;
    Credit credit = *optCredit;
    if (credit.estEnRetard()) {
        credit.setStatut(Credit::Statut::EnRetard);
        credit.setSyncStatus(Credit::SyncStatus::PENDING);
        credit.setVersion(credit.getVersion() + 1);
        repo.update(credit);
    }
}
