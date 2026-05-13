#include "GestionnaireCaisse.h"
#include "../../data/repositories/RepositoryReceptionCaisse.h"
#include <QDateTime>
#include <QDebug>

GestionnaireCaisse::GestionnaireCaisse() : m_dernierErreur() {}

QUuid GestionnaireCaisse::creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu, const QUuid& caissierId, const QString& notes)
{
    if (repartitionId.isNull() || montantAttendu < 0) {
        m_dernierErreur = "Paramètres invalides";
        return QUuid();
    }

    ReceptionCaisse reception;
    reception.setRepartitionId(repartitionId);
    reception.setMontantAttendu(montantAttendu);
    reception.setNumeroRecu(genererNumeroRecu());
    reception.setStatut(ReceptionCaisse::Statut::EnAttente);
    reception.setVersion(1);
    reception.setSyncStatus(ReceptionCaisse::SyncStatus::PENDING);
    reception.setDateCreation(QDateTime::currentDateTime());
    reception.setDateMiseAJour(QDateTime::currentDateTime());
    reception.setCaissierId(caissierId);
    reception.setNotes(notes);

    RepositoryReceptionCaisse repo;
    if (!repo.create(reception)) {
        m_dernierErreur = "Erreur lors de la création : " + repo.getLastError();
        return QUuid();
    }
    return reception.getReceptionCaisseId();
}

bool GestionnaireCaisse::enregistrerMontantRecu(const QUuid& receptionId, double montantRecu)
{
    if (receptionId.isNull() || montantRecu < 0) {
        m_dernierErreur = "Paramètres invalides";
        return false;
    }

    RepositoryReceptionCaisse repo;
    ReceptionCaisse rc = repo.getById(receptionId);
    if (rc.getReceptionCaisseId().isNull()) {
        m_dernierErreur = "Réception non trouvée";
        return false;
    }

    rc.setMontantRecu(montantRecu);
    rc.setDateReception(QDateTime::currentDateTime());
    rc.setDateMiseAJour(QDateTime::currentDateTime());
    rc.setVersion(rc.getVersion() + 1);
    rc.setSyncStatus(ReceptionCaisse::SyncStatus::PENDING);

    if (rc.hasDiscrepancy()) {
        rc.setStatut(ReceptionCaisse::Statut::Discrepance);
    } else {
        rc.setStatut(ReceptionCaisse::Statut::Recu);
    }

    return repo.update(rc);
}

bool GestionnaireCaisse::validerReception(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    ReceptionCaisse rc = repo.getById(receptionId);
    if (rc.getReceptionCaisseId().isNull()) {
        m_dernierErreur = "Réception non trouvée";
        return false;
    }

    rc.setDateMiseAJour(QDateTime::currentDateTime());
    rc.setVersion(rc.getVersion() + 1);
    rc.setSyncStatus(ReceptionCaisse::SyncStatus::PENDING);

    if (rc.hasDiscrepancy()) {
        m_dernierErreur = "Discordance détectée : " + QString::number(rc.getEcart(), 'f', 2);
        rc.setStatut(ReceptionCaisse::Statut::Discrepance);
    } else {
        rc.setStatut(ReceptionCaisse::Statut::Valide);
    }

    return repo.update(rc);
}

bool GestionnaireCaisse::softDeleteReception(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    // soft-delete = update de la colonne deleted_at, sync_status = PENDING (déjà géré dans repo)
    return repo.remove(receptionId);
}

ReceptionCaisse GestionnaireCaisse::obtenirReception(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    return repo.getById(receptionId);
}

ReceptionCaisse GestionnaireCaisse::obtenirReceptionParRepartition(const QUuid& repartitionId)
{
    RepositoryReceptionCaisse repo;
    return repo.getByRepartition(repartitionId);
}

QList<ReceptionCaisse> GestionnaireCaisse::obtenirReceptionsEnAttente()
{
    RepositoryReceptionCaisse repo;
    return repo.getByStatut(ReceptionCaisse::Statut::EnAttente);
}

QList<ReceptionCaisse> GestionnaireCaisse::obtenirReceptionsAvecDiscrepance()
{
    RepositoryReceptionCaisse repo;
    return repo.getWithDiscrepancies();
}

double GestionnaireCaisse::obtenirTotalCashValide()
{
    RepositoryReceptionCaisse repo;
    QList<ReceptionCaisse> List = repo.getByStatut(ReceptionCaisse::Statut::Valide);
    double total = 0.0;
    for (const auto& rc : List) {
        total += rc.getMontantRecu();
    }
    return total;
}

double GestionnaireCaisse::obtenirTotalEcarts()
{
    RepositoryReceptionCaisse repo;
    QList<ReceptionCaisse> List = repo.getWithDiscrepancies();
    double total = 0.0;
    for (const auto& rc : List) {
        total += std::abs(rc.getEcart());
    }
    return total;
}

int GestionnaireCaisse::obtenirNombreDiscrepances()
{
    return obtenirReceptionsAvecDiscrepance().count();
}

QList<ReceptionCaisse> GestionnaireCaisse::obtenirReceptionsASynchroniser()
{
    RepositoryReceptionCaisse repo;
    return repo.getBySyncStatus(ReceptionCaisse::SyncStatus::PENDING);
}

QList<ReceptionCaisse> GestionnaireCaisse::obtenirReceptionsEnConflit()
{
    RepositoryReceptionCaisse repo;
    return repo.getBySyncStatus(ReceptionCaisse::SyncStatus::CONFLICT);
}

bool GestionnaireCaisse::marquerSynced(const QUuid& receptionId, int nouvelleVersion)
{
    RepositoryReceptionCaisse repo;
    ReceptionCaisse rc = repo.getById(receptionId);
    if (rc.getReceptionCaisseId().isNull()) return false;
    rc.setSyncStatus(ReceptionCaisse::SyncStatus::SYNCED);
    rc.setVersion(nouvelleVersion);
    rc.setDateMiseAJour(QDateTime::currentDateTime());
    return repo.update(rc);
}

bool GestionnaireCaisse::marquerConflit(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    ReceptionCaisse rc = repo.getById(receptionId);
    if (rc.getReceptionCaisseId().isNull()) return false;
    rc.setSyncStatus(ReceptionCaisse::SyncStatus::CONFLICT);
    rc.setDateMiseAJour(QDateTime::currentDateTime());
    rc.setVersion(rc.getVersion() + 1);
    return repo.update(rc);
}

QString GestionnaireCaisse::genererNumeroRecu() const
{
    const QString date = QDateTime::currentDateTime().toString("yyMMdd");
    const QString heure = QDateTime::currentDateTime().toString("hhmmss");
    return QString("RC-%1-%2").arg(date, heure);
}
