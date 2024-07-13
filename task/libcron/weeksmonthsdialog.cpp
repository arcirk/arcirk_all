#include "weeksmonthsdialog.h"
#include "ui_weeksmonthsdialog.h"
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include "cron_parser.hpp"

using namespace arcirk::tasks;

WeeksMonthsDialog::WeeksMonthsDialog(int type, const QString& def, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WeeksMonthsDialog)
{
    ui->setupUi(this);

    m_type = type;
    m_source = def;
    result = m_source.split(" ");

    if(m_type == 0)
        createWeeksControls();
    else
        createMonthControls();

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

}

WeeksMonthsDialog::~WeeksMonthsDialog()
{
    delete ui;
}

QString WeeksMonthsDialog::currentCronText() const
{
    if(m_type == 0)
        return result[5];
    else
        return result[3];
}

void WeeksMonthsDialog::createWeeksControls()
{
    setWindowTitle("Недельное");
    ui->groupBox->setTitle("Дни недели");

    const QList<QString> m_labels{"Пн","Вт","Ср","Чт","Пт","Сб","Вс"};

    m_values.resize(m_labels.size());

    sched::Schedule schedule = sched::parse_cron(m_source.toStdString());
    auto field = std::get<5>(schedule);

    auto is_valid_value = [] (const sched::Field& field, const int& value) {
        for (auto& item: field) {
            int v_min = static_cast<int>(std::get<0>(item));
            int v_max = static_cast<int>(std::get<1>(item));
            if(v_min == 255 && v_max == 255)
                return true;
            else{
                if(value >= v_min && value <= v_max)
                    return true;
            }
        }
        return false;
    };

    for (int i = 0; i < m_labels.size(); ++i) {

        auto lbl = new QLabel(this);
        lbl->setText(m_labels[i]);

        ui->mGrid->addWidget(lbl, 0, i, Qt::AlignCenter);

        auto chk = new QCheckBox(this);
        chk->setProperty("index", i);
        chk->setObjectName(day_names[i]);
        chk->setChecked(is_valid_value(std::get<5>(schedule), i + 1));
        m_values[i] = chk->isChecked();
        ui->mGrid->addWidget(chk, 1, i, Qt::AlignCenter);
        connect(chk, &QCheckBox::toggled, this, &WeeksMonthsDialog::onCheckToggled);
    }
}

void WeeksMonthsDialog::createMonthControls()
{
    setWindowTitle("Месячное");
    ui->groupBox->setTitle("Месяцы");

    const QList<QString> m_labels{"Январь","Февраль","Март","Апрель","Май","Июнь","Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};

    m_values.resize(m_labels.size());

    sched::Schedule schedule = sched::parse_cron(m_source.toStdString());
    auto field = std::get<3>(schedule);

    auto is_valid_value = [] (const sched::Field& field, const int& value) {
        for (auto& item: field) {
            int v_min = static_cast<int>(std::get<0>(item));
            int v_max = static_cast<int>(std::get<1>(item));
            if(v_min == 255 && v_max == 255)
                return true;
            else{
                if(value >= v_min && value <= v_max)
                    return true;
            }
        }
        return false;
    };

    int row = 0;
    int col = 0;
    for (int i = 0; i < m_labels.size(); ++i) {
        auto chk = new QCheckBox(m_labels[i], this);
        chk->setProperty("index", i);
        chk->setObjectName(month_names[i]);
        chk->setChecked(is_valid_value(std::get<3>(schedule), i + 1));
        m_values[i] = chk->isChecked();
        ui->mGrid->addWidget(chk, row, col, Qt::AlignLeft);
        connect(chk, &QCheckBox::toggled, this, &WeeksMonthsDialog::onCheckToggled);
        if(col == 3){
            row++;
            col = 0;
        }else
            col++;
    }

}

void WeeksMonthsDialog::updateCron()
{
    QStringList res;
    QList<int> vals;

    for (int i = 0; i < m_values.size(); ++i) {
        if(m_values[i]){
            vals.append(i+1);
        }else{
            vals.append(0);
        }
    }

    int start = 0;
    for (int i = 0; i < vals.size(); ++i) {
        int left = 0;
        int right = 0;
        if(i > 0){
            left = vals[i-1];
        }
        if(vals.size() > i + 1){
            right = vals[i+1];
        }

        if(vals[i] > 0){
            if(left + right == 0){
                res.append(QString::number(i + 1));
            }else if(left == 0 && right > 0){
                start = i;
            }else if(left > 0 && right == 0){
                if(start < i)
                    if(start + 1 != i)
                        res.append(QString::number(start + 1) + "-" + QString::number(i+1) );
                    else{
                        res.append(QString::number(i));
                        res.append(QString::number(i + 1));
                    }
                else
                    res.append(QString::number(i + 1));
            }
        }
    }

    auto s = res.join(",");

    if(m_type == 0){
        if(s != "1-7")
            result[5] = s;
        else
            result[5] = "*";
        qDebug() << result[5];
    }else{
        if(s != "1-12")
            result[3] = s;
        else
            result[3] = "*";
        qDebug() << result[3];
    }



}

void WeeksMonthsDialog::onCheckToggled(bool state)
{
    auto chk = qobject_cast<QCheckBox*>(sender());
    if(chk){
        auto index = chk->property("index").toInt();
        if(index >= 0 && index < m_values.size())
            m_values[index] = state;
        updateCron();
    }
}
