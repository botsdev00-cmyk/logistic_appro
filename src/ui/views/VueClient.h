#ifndef VUECLIENT_H
#define VUECLIENT_H

#include <QWidget>
#include <memory>

class QLineEdit;
class QComboBox;
class TableauClient;

class VueClient : public QWidget
{
    Q_OBJECT

public:
    VueClient(QWidget* parent = nullptr);
    ~VueClient();

private slots:
    void rechercher();
    void ajouterClient();
    void editerClient();
    void supprimerClient();
    void afficherHistorique();
    void filtrerParRoute();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QLineEdit> m_champRecherche;
    std::unique_ptr<QComboBox> m_comboRoute;
    std::unique_ptr<TableauClient> m_tableauClient;
};

#endif // VUECLIENT_H