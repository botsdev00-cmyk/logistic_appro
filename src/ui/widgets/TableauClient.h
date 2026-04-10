#ifndef TABLEAUCLIENT_H
#define TABLEAUCLIENT_H

#include <QTableWidget>
#include <memory>

class GestionnaireClient;

class TableauClient : public QTableWidget
{
    Q_OBJECT

public:
    TableauClient(QWidget* parent = nullptr);
    ~TableauClient();

    void chargerDonnees();
    void rafraichir();
    void filtrerParRoute(const QString& route);
    void rechercher(const QString& terme);

private slots:
    void afficherDetailsClient();
    void editerClient();
    void supprimerClient();
    void exporterEnCSV();

private:
    void creerContextMenu();
    void initialiserColonnes();
    void remplirTableau();

    std::unique_ptr<GestionnaireClient> m_gestionnaire;
};

#endif // TABLEAUCLIENT_H