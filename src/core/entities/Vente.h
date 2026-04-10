#ifndef VENTE_H
#define VENTE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Vente
{
public:
    enum class TypeVente {
        Cash,
        Credit
    };

    Vente();
    ~Vente();

    // Getters
    QUuid getVenteId() const { return m_venteId; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    QUuid getProduitId() const { return m_produitId; }
    QUuid getClientId() const { return m_clientId; }
    int getQuantite() const { return m_quantite; }
    TypeVente getTypeVente() const { return m_typeVente; }
    double getPrixUnitaire() const { return m_prixUnitaire; }
    double getMontantTotal() const { return m_quantite * m_prixUnitaire; }
    QString getNotes() const { return m_notes; }
    QDateTime getDateVente() const { return m_dateVente; }
    QUuid getCreePar() const { return m_creePar; }

    // Setters
    void setVenteId(const QUuid& id) { m_venteId = id; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setClientId(const QUuid& id) { m_clientId = id; }
    void setQuantite(int qty) { m_quantite = qty; }
    void setTypeVente(TypeVente type) { m_typeVente = type; }
    void setPrixUnitaire(double prix) { m_prixUnitaire = prix; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setDateVente(const QDateTime& date) { m_dateVente = date; }
    void setCreePar(const QUuid& id) { m_creePar = id; }

    // Utility methods
    static QString typeVenteToString(TypeVente type);
    static TypeVente stringToTypeVente(const QString& str);
    QString getTypeVenteLabel() const;

private:
    QUuid m_venteId;
    QUuid m_repartitionId;
    QUuid m_produitId;
    QUuid m_clientId;
    int m_quantite;
    TypeVente m_typeVente;
    double m_prixUnitaire;
    QString m_notes;
    QDateTime m_dateVente;
    QUuid m_creePar;
};

#endif // VENTE_H