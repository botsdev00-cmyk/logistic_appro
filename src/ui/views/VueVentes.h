#ifndef VUEVENTES_H
#define VUEVENTES_H

#include <QWidget>
#include <memory>

class QComboBox;
class QLineEdit;
class TableauVentes;

class VueVentes : public QWidget
{
    Q_OBJECT

public:
    VueVentes(QWidget* parent = nullptr);
    ~VueVentes();

private slots:
    void ajouterVente();
    void filtrerParClient();
    void filtrerParType();
    void afficherDetails();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QComboBox> m_comboClient;
    std::unique_ptr<QComboBox> m_comboType;
    std::unique_ptr<TableauVentes> m_tableauVentes;
};

#endif // VUEVENTES_H