#ifndef ENTREESTOCK_H
#define ENTREESTOCK_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class EntreeStock
{
public:
    EntreeStock();
    ~EntreeStock();

    // Getters
    QUuid getEntreeStockId() const { return m_entreeStockId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantite() const { return m_quantite; }
    QUuid getSourceEntreeId() const { return m_sourceEntreeId; }
    QString getNumeroFacture() const { return m_numeroFacture; }
    double getPrixUnitaire() const { return m_prixUnitaire; }
    QString getNumeroLot() const { return m_numeroLot; }
    QDate getDateExpiration() const { return m_dateExpiration; }
    QUuid getCreePar() const { return m_creePar; }
    QUuid getApprouvePar() const { return m_approuvePar; }
    QString getStatutValidation() const { return m_statutValidation; }
    QDateTime getDate() const { return m_date; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setEntreeStockId(const QUuid& id) { m_entreeStockId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantite(int q) { m_quantite = q; }
    void setSourceEntreeId(const QUuid& id) { m_sourceEntreeId = id; }
    void setNumeroFacture(const QString& num) { m_numeroFacture = num; }
    void setPrixUnitaire(double prix) { m_prixUnitaire = prix; }
    void setNumeroLot(const QString& lot) { m_numeroLot = lot; }
    void setDateExpiration(const QDate& date) { m_dateExpiration = date; }
    void setDate(const QDateTime& date) { m_date = date; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setApprouvePar(const QUuid& id) { m_approuvePar = id; }
    void setStatutValidation(const QString& statut) { m_statutValidation = statut; }

    // Validation
    bool estValide() const;

private:
    QUuid m_entreeStockId;
    QUuid m_produitId;
    int m_quantite;
    QUuid m_sourceEntreeId;
    QString m_numeroFacture;
    double m_prixUnitaire;
    QString m_numeroLot;
    QDate m_dateExpiration;
    QUuid m_creePar;
    QUuid m_approuvePar;
    QString m_statutValidation; // EN_ATTENTE, APPROUVE, REJETE
    QDateTime m_date;
    QDateTime m_dateMiseAJour;
};

#endif // ENTREESTOCK_H