#ifndef CATEGORIEPRODUIT_H
#define CATEGORIEPRODUIT_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class CategorieProduit
{
public:
    enum class SyncStatus { PENDING, SYNCED, CONFLICT };

    CategorieProduit();
    ~CategorieProduit();

    // Getters
    QUuid getCategorieProduitId() const { return m_categorieProduitId; }
    QString getNom() const { return m_nom; }
    QString getDescription() const { return m_description; }
    QString getCodeCategorie() const { return m_codeCategorie; }
    bool estActif() const { return m_estActif; }
    int getOrdreAffichage() const { return m_ordreAffichage; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    int getVersion() const { return m_version; }
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    QDateTime getDeletedAt() const { return m_deletedAt; }

    // Setters
    void setCategorieProduitId(const QUuid& id) { m_categorieProduitId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setCodeCategorie(const QString& code) { m_codeCategorie = code; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setOrdreAffichage(int ordre) { m_ordreAffichage = ordre; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }
    void setVersion(int v) { m_version = v; }
    void setSyncStatus(SyncStatus status) { m_syncStatus = status; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }

    // Utilitaires
    bool isDeleted() const { return m_deletedAt.isValid(); }
    bool needsSync() const { return m_syncStatus != SyncStatus::SYNCED; }
    QString syncStatusString() const;
    static SyncStatus fromString(const QString& v);

private:
    QUuid m_categorieProduitId;
    QString m_nom;
    QString m_description;
    QString m_codeCategorie;
    bool m_estActif;
    int m_ordreAffichage;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
    int m_version;
    SyncStatus m_syncStatus;
    QDateTime m_deletedAt;
};

#endif // CATEGORIEPRODUIT_H
