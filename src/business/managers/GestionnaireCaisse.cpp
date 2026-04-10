#include "GestionnaireCaisse.h"
#include "../exceptions/BusinessException.h"
#include "../../data/repositories/RepositoryReceptionCaisse.h"
#include "../../core/entities/ReceptionCaisse.h"
#include <QDebug>
#include <QDateTime>

GestionnaireCaisse::GestionnaireCaisse()
{
}

QUuid GestionnaireCaisse::creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu)
{
    try {
        if (repartitionId.isNull()) {
            m_dernierErreur = "ID répartition invalide";
            return QUuid();
        }
        
        if (montantAttendu < 0) {
            m_dernierErreur = "Montant invalide";
            return QUuid();
        }
        
        ReceptionCaisse reception;
        reception.setRepartitionId(repartitionId);
        reception.setMontantAttendu(montantAttendu);
        reception.setNumeroRecu(genererNumeroRecu());
        reception.setStatut(ReceptionCaisse::Statut::EnAttente);
        
        RepositoryReceptionCaisse repo;
        if (!repo.create(reception)) {
            m_dernierErreur = "Erreur lors de la création : " + repo.getLastError();
            return QUuid();
        }
        
        qDebug() << "Réception caisse créée :" << reception.getReceptionCaisseId().toString();
        return reception.getReceptionCaisseId();
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireCaisse::enregistrerMontantRecu(const QUuid& receptionId, double montantRecu)
{
    try {
        if (receptionId.isNull() || montantRecu < 0) {
            m_dernierErreur = "Paramètres invalides";
            return false;
        }
        
        RepositoryReceptionCaisse repo;
        ReceptionCaisse reception = repo.getById(receptionId);
        
        if (reception.getReceptionCaisseId().isNull()) {
            m_dernierErreur = "Réception non trouvée";
            return false;
        }
        
        reception.setMontantRecu(montantRecu);
        reception.setDateReception(QDateTime::currentDateTime());
        
        if (reception.hasDiscrepancy()) {
            reception.setStatut(ReceptionCaisse::Statut::Discrepance);
        } else {
            reception.setStatut(ReceptionCaisse::Statut::Recu);
        }
        
        return repo.update(reception);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireCaisse::validerReception(const QUuid& receptionId)
{
    try {
        RepositoryReceptionCaisse repo;
        ReceptionCaisse reception = repo.getById(receptionId);
        
        if (reception.getReceptionCaisseId().isNull()) {
            m_dernierErreur = "Réception non trouvée";
            return false;
        }
        
        if (reception.hasDiscrepancy()) {
            m_dernierErreur = "Discordance détectée : " + 
                             QString::number(reception.getEcart(), 'f', 2);
            reception.setStatut(ReceptionCaisse::Statut::Discrepance);
        } else {
            reception.setStatut(ReceptionCaisse::Statut::Valide);
        }
        
        return repo.update(reception);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireCaisse::genererRecu(const QUuid& receptionId)
{
    try {
        RepositoryReceptionCaisse repo;
        ReceptionCaisse reception = repo.getById(receptionId);
        
        if (reception.getReceptionCaisseId().isNull()) {
            m_dernierErreur = "Réception non trouvée";
            return false;
        }
        
        // Générer le reçu PDF ou imprimé
        qDebug() << "Génération du reçu :" << reception.getNumeroRecu();
        
        return true;
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

ReceptionCaisse GestionnaireCaisse::obtenirReception(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    return repo.getById(receptionId);
}

ReceptionCaisse GestionnaireCaisse::obtenirReceptionRepartition(const QUuid& repartitionId)
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

double GestionnaireCaisse::obtenirTotalCashRecu()
{
    RepositoryReceptionCaisse repo;
    QList<ReceptionCaisse> receptions = repo.getByStatut(ReceptionCaisse::Statut::Valide);
    
    double total = 0.0;
    for (const auto& reception : receptions) {
        total += reception.getMontantRecu();
    }
    
    return total;
}

double GestionnaireCaisse::obtenirTotalEcarts()
{
    RepositoryReceptionCaisse repo;
    QList<ReceptionCaisse> receptions = repo.getWithDiscrepancies();
    
    double total = 0.0;
    for (const auto& reception : receptions) {
        total += qAbs(reception.getEcart());
    }
    
    return total;
}

int GestionnaireCaisse::obtenirNombreDiscrepances()
{
    return obtenirReceptionsAvecDiscrepance().count();
}

bool GestionnaireCaisse::validerDiscrepance(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    ReceptionCaisse reception = repo.getById(receptionId);
    
    if (reception.getReceptionCaisseId().isNull()) {
        m_dernierErreur = "Réception non trouvée";
        return false;
    }
    
    reception.setStatut(ReceptionCaisse::Statut::Discrepance);
    return repo.update(reception);
}

QList<ReceptionCaisse> GestionnaireCaisse::obtenirAlertesDiscrepance()
{
    return obtenirReceptionsAvecDiscrepance();
}

QString GestionnaireCaisse::genererNumeroRecu()
{
    QString date = QDateTime::currentDateTime().toString("yyMMdd");
    QString heure = QDateTime::currentDateTime().toString("hhmmss");
    return QString("RC-%1-%2").arg(date, heure);
}

bool GestionnaireCaisse::detecterDiscrepance(const QUuid& receptionId)
{
    RepositoryReceptionCaisse repo;
    ReceptionCaisse reception = repo.getById(receptionId);
    
    return reception.hasDiscrepancy();
}