#ifndef STOCKSOLDE_H
#define STOCKSOLDE_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class StockSolde
{
public:
    StockSolde();
    ~StockSolde();

    // Getters
    QUuid getSoldeId() const { return m_soldeId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantiteTotal() const { return m_quantiteTotal; }
    int getQuantiteReservee() const { return m_quantiteReservee; }
    int getQuantiteDisponible() const { return m_quantiteTotal - m_quantiteReservee; }
    double getValeurStock() const { return m_valeurStock; }
    double getPrixMoyen() const { return m_prixMoyen; }
    QDateTime getDernierMouvementDate() const { return m_dernierMouvementDate; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    // Setters
    void setSoldeId(const QUuid& id) { m_soldeId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantiteTotal(int q) { m_quantiteTotal = q; }
    void setQuantiteReservee(int q) { m_quantiteReservee = q; }
    void setValeurStock(double v) { m_valeurStock = v; }
    void setPrixMoyen(double p) { m_prixMoyen = p; }
    void setDernierMouvementDate(const QDateTime& dt) { m_dernierMouvementDate = dt; }

private:
    QUuid m_soldeId;
    QUuid m_produitId;
    int m_quantiteTotal;
    int m_quantiteReservee;
    double m_valeurStock;
    double m_prixMoyen;
    QDateTime m_dernierMouvementDate;
    QDateTime m_updatedAt;
};

#endif // STOCKSOLDE_H