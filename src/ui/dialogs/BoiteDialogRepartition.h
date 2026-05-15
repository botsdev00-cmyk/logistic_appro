#ifndef BOITEDIALOGREPARTITION_H
#define BOITEDIALOGREPARTITION_H

#include <QDialog>
#include <QDate>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QPushButton>
#include <memory>
#include <QUuid>
#include <QList>
#include "../../core/entities/ArticleRepartition.h"

class BoiteDialogRepartition : public QDialog
{
    Q_OBJECT
public:
    explicit BoiteDialogRepartition(QWidget* parent = nullptr);
    ~BoiteDialogRepartition();
    QUuid getRepartitionId() const { return m_repartitionId; }

private:
    void creerWidgets();
    void initialiserConnexions();
    void chargerDonnees();

    void remplirComboEquipes();
    void remplirComboRoutes();
    void remplirTableProduits();

private slots:
    void onNouvelleEquipe();
    void onNouvelleRoute();
    void creerRepartition();
    void ajouterArticle();
    void supprimerArticle();
    void mettreAJourArticles();

private:
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

    QUuid m_repartitionId;

    std::unique_ptr<QPushButton> m_btnNouvelleEquipe;
    std::unique_ptr<QPushButton> m_btnNouvelleRoute;
};

#endif // BOITEDIALOGREPARTITION_H
