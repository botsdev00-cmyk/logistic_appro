#ifndef RETOURSTOCK_H
#define RETOURSTOCK_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class RetourStock
{
public:
    RetourStock();
    ~RetourStock();

    // Getters
    QUuid getRetourStockId() const { return m_retourStockId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantite() const { return m_quantite; }
    QUuid getRaisonRetourId() const { return m_raisonRetourId; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    QString getObservations() const { return m_observations; }
    QUuid getCreePar() const { return m_creePar; }
    QUuid getApprouvePar() const { return m_approuvePar; }
    QString getStatutValidation() const { return m_statutValidation; }
    QDateTime getDate() const { return m_date; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setRetourStockId(const QUuid& id) { m_retourStockId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantite(int q) { m_quantite = q; }
    void setRaisonRetourId(const QUuid& id) { m_raisonRetourId = id; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setObservations(const QString& obs) { m_observations = obs; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setApprouvePar(const QUuid& id) { m_approuvePar = id; }
    void setStatutValidation(const QString& statut) { m_statutValidation = statut; }

    // Validation
    bool estValide() const;

private:
    QUuid m_retourStockId;
    QUuid m_produitId;
    int m_quantite;
    QUuid m_raisonRetourId;
    QUuid m_repartitionId;
    QString m_observations;
    QUuid m_creePar;
    QUuid m_approuvePar;
    QString m_statutValidation;
    QDateTime m_date;
    QDateTime m_dateMiseAJour;
};

#endif // RETOURSTOCK_H