#pragma once
#include <QUuid>
#include <QString>
#include <QDate>
#include <QDateTime>

class Employee
{
public:
    Employee();

    QUuid employeId() const;
    void setEmployeId(const QUuid& id);

    QString nom() const;
    void setNom(const QString& nom);

    QString prenom() const;
    void setPrenom(const QString& prenom);

    QUuid gradeId() const;
    void setGradeId(const QUuid& id);

    QDate dateNaissance() const;
    void setDateNaissance(const QDate& date);

    QString email() const;
    void setEmail(const QString& email);

    QString telephone() const;
    void setTelephone(const QString& tel);

    QDate dateEmbauche() const;
    void setDateEmbauche(const QDate& date);

    QString syncStatus() const;
    void setSyncStatus(const QString& status);

    int version() const;
    void setVersion(int version);

    QDateTime createdAt() const;
    void setCreatedAt(const QDateTime& dt);

    QDateTime updatedAt() const;
    void setUpdatedAt(const QDateTime& dt);

    QDateTime deletedAt() const;
    void setDeletedAt(const QDateTime& dt);

private:
    QUuid m_employeId;
    QString m_nom;
    QString m_prenom;
    QUuid m_gradeId;
    QDate m_dateNaissance;
    QString m_email;
    QString m_telephone;
    QDate m_dateEmbauche;
    QString m_syncStatus = "PENDING";
    int m_version = 1;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
};
