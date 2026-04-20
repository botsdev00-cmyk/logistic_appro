#ifndef TABLEAUREPARTITION_H
#define TABLEAUREPARTITION_H

#include <QTableWidget>
#include <memory>

class GestionnaireRepartition;

class TableauRepartition : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableauRepartition(QWidget* parent = nullptr);
    ~TableauRepartition();

    void chargerDonnees();
    void rafraichir();
    void filtrerParStatut(const QString& statut);

private slots:
    void afficherDetailsRepartition();
    void changerStatut();
    void afficherArticles();
    void imprimerBon(); // BONUS : génération PDF

private:
    void creerContextMenu();
    void initialiserColonnes();
    void remplirTableau();
    void mettreEnEvidence();
    QUuid repartitionIdFromRow(int row) const;

    std::unique_ptr<GestionnaireRepartition> m_gestionnaire;
};

#endif // TABLEAUREPARTITION_H