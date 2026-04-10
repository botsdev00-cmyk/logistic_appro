#ifndef BOITEDIALOGPARTITION_H
#define BOITEDIALOGPARTITION_H

#include <QDialog>
#include <QUuid>
#include <memory>

class QComboBox;
class QDateEdit;
class QPushButton;
class QTableWidget;
class QSpinBox;
class QLabel;
class GestionnaireRepartition;

class BoiteDialogRepartition : public QDialog
{
    Q_OBJECT

public:
    BoiteDialogRepartition(QWidget* parent = nullptr);
    ~BoiteDialogRepartition();
    
    QUuid getRepartitionId() const { return m_repartitionId; }

private slots:
    void creerRepartition();
    void ajouterArticle();
    void supprimerArticle();
    void mettreAJourArticles();

private:
    void creerWidgets();
    void initialiserConnexions();
    void chargerDonnees();
    void remplirComboEquipes();
    void remplirComboRoutes();
    void remplirTableProduits();

    std::unique_ptr<QComboBox> m_comboEquipe;
    std::unique_ptr<QComboBox> m_comboRoute;
    std::unique_ptr<QDateEdit> m_dateRepartition;
    std::unique_ptr<QComboBox> m_comboProduit;
    std::unique_ptr<QSpinBox> m_spinVente;
    std::unique_ptr<QSpinBox> m_spinCadeau;
    std::unique_ptr<QSpinBox> m_spinDegustation;
    std::unique_ptr<QTableWidget> m_tableArticles;
    std::unique_ptr<QPushButton> m_boutonAjouter;
    std::unique_ptr<QPushButton> m_boutonSupprimer;
    std::unique_ptr<QPushButton> m_boutonCreer;
    std::unique_ptr<QPushButton> m_boutonAnnuler;
    
    std::unique_ptr<GestionnaireRepartition> m_gestionnaire;
    QUuid m_repartitionId;
};

#endif // BOITEDIALOGPARTITION_H