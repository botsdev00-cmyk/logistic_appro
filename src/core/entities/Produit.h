#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Produit
{
public:
    enum class SyncStatus { PENDING, SYNCED, CONFLICT };

    Produit();
    ~Produit();

    // Getters
    QUuid getProduitId() const { return m_produitId; }
    QUuid getCategorieProduitId() const { return m_categorieProduitId; }
    QUuid getTypeProduitId() const { return m_typeProduitId; }
    QString getNom() const { return m_nom; }
    QString getDescription() const { return m_description; }
    QString getCodeSku() const { return m_codeSku; }
    double getPrixUnitaire() const { return m_prixUnitaire; }
    int getStockMinimum() const { return m_stockMinimum; }
    bool estActif() const { return m_estActif; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    int getVersion() const { return m_version; }
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    QDateTime getDeletedAt() const { return m_deletedAt; }

    // Setters
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setCategorieProduitId(const QUuid& id) { m_categorieProduitId = id; }
    void setTypeProduitId(const QUuid& id) { m_typeProduitId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setCodeSku(const QString& code) { m_codeSku = code; }
    void setPrixUnitaire(double prix) { m_prixUnitaire = prix; }
    void setStockMinimum(int stock) { m_stockMinimum = stock; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }
    void setVersion(int v) { m_version = v; }
    void setSyncStatus(SyncStatus status) { m_syncStatus = status; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }

    bool isDeleted() const { return m_deletedAt.isValid(); }
    bool needsSync() const { return m_syncStatus != SyncStatus::SYNCED; }
    QString syncStatusString() const;
    static SyncStatus fromString(const QString& v);

private:
    QUuid m_produitId;
    QUuid m_categorieProduitId;
    QUuid m_typeProduitId;
    QString m_nom;
    QString m_description;
    QString m_codeSku;
    double m_prixUnitaire;
    int m_stockMinimum;
    bool m_estActif;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
    int m_version;
    SyncStatus m_syncStatus;
    QDateTime m_deletedAt;
};

#endif // PRODUIT_H
