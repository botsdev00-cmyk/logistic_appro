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

    // ============ GETTERS - Identifiants ============
    QUuid getSoldeId() const { return m_soldeId; }
    QUuid getProduitId() const { return m_produitId; }

    // ============ GETTERS - Données Produit (depuis v_stock_detail) ============
    QString getProduitNom() const { return m_produitNom; }
    QString getCodeSKU() const { return m_codeSKU; }
    QString getCategorie() const { return m_categorie; }
    QString getType() const { return m_type; }
    int getStockMinimum() const { return m_stockMinimum; }

    // ============ GETTERS - Quantités ============
    int getQuantiteTotal() const { return m_quantiteTotal; }
    int getQuantiteReservee() const { return m_quantiteReservee; }
    int getQuantiteDisponible() const { return m_quantiteTotal - m_quantiteReservee; }

    // ============ GETTERS - Valeurs ============
    double getValeurStock() const { return m_valeurStock; }
    double getPrixMoyen() const { return m_prixMoyen; }

    // ============ GETTERS - Dates ============
    QDateTime getDernierMouvementDate() const { return m_dernierMouvementDate; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    // ============ SETTERS - Identifiants ============
    void setSoldeId(const QUuid& id) { m_soldeId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }

    // ============ SETTERS - Données Produit ============
    void setProduitNom(const QString& nom) { m_produitNom = nom; }
    void setCodeSKU(const QString& sku) { m_codeSKU = sku; }
    void setCategorie(const QString& cat) { m_categorie = cat; }
    void setType(const QString& type) { m_type = type; }
    void setStockMinimum(int min) { m_stockMinimum = min; }

    // ============ SETTERS - Quantités ============
    void setQuantiteTotal(int q) { m_quantiteTotal = q; }
    void setQuantiteReservee(int q) { m_quantiteReservee = q; }

    // ============ SETTERS - Valeurs ============
    void setValeurStock(double v) { m_valeurStock = v; }
    void setPrixMoyen(double p) { m_prixMoyen = p; }

    // ============ SETTERS - Dates ============
    void setDernierMouvementDate(const QDateTime& dt) { m_dernierMouvementDate = dt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    // ============ MÉTHODES UTILES ============
    bool estEnRupture() const { return getQuantiteTotal() <= 0; }
    bool estBasStock(int seuil) const { return getQuantiteDisponible() > 0 && getQuantiteDisponible() < seuil; }
    bool estDisponible(int quantiteRequise) const { return getQuantiteDisponible() >= quantiteRequise; }

private:
    // Identifiants
    QUuid m_soldeId;
    QUuid m_produitId;

    // Données Produit (depuis v_stock_detail)
    QString m_produitNom;
    QString m_codeSKU;
    QString m_categorie;
    QString m_type;
    int m_stockMinimum;

    // Quantités
    int m_quantiteTotal;
    int m_quantiteReservee;

    // Valeurs
    double m_valeurStock;
    double m_prixMoyen;

    // Dates
    QDateTime m_dernierMouvementDate;
    QDateTime m_updatedAt;
};

#endif // STOCKSOLDE_H