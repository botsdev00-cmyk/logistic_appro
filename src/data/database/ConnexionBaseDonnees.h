#ifndef CONNEXIONBASEDONNEES_H
#define CONNEXIONBASEDONNEES_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <memory>

class ConnexionBaseDonnees
{
public:
    static ConnexionBaseDonnees& getInstance();

    bool initialiser(const QString& host, const QString& port, 
                    const QString& database, const QString& user, 
                    const QString& password);
    
    bool estConnecte() const;
    void fermer();
    
    QSqlDatabase getDatabase() const { return m_database; }
    QString getDernierErreur() const { return m_dernierErreur; }

    bool executerMigration(const QString& cheminSQL);
    ~ConnexionBaseDonnees();


private:
    ConnexionBaseDonnees();
    
    // Empêcher la copie
    ConnexionBaseDonnees(const ConnexionBaseDonnees&) = delete;
    ConnexionBaseDonnees& operator=(const ConnexionBaseDonnees&) = delete;

    static std::unique_ptr<ConnexionBaseDonnees> m_instance;
    QSqlDatabase m_database;
    QString m_dernierErreur;
};

#endif // CONNEXIONBASEDONNEES_H