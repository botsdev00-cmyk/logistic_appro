#ifndef VUESTOCK_H
#define VUESTOCK_H

#include <QWidget>
#include <memory>

class QComboBox;
class QLineEdit;
class TableauStock;

class VueStock : public QWidget
{
    Q_OBJECT

public:
    VueStock(QWidget* parent = nullptr);
    ~VueStock();

private slots:
    void rechercher();
    void filtrer();
    void creerEntree();
    void enregistrerRetour();
    void ajustement();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QLineEdit> m_champRecherche;
    std::unique_ptr<QComboBox> m_comboCategorie;
    std::unique_ptr<TableauStock> m_tableauStock;
};

#endif // VUESTOCK_H