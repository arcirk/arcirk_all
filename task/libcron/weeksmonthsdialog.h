#ifndef WEEKSMONTHSDIALOG_H
#define WEEKSMONTHSDIALOG_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class WeeksMonthsDialog;
}

namespace arcirk::tasks {
    class WeeksMonthsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit WeeksMonthsDialog(int type, const QString& def = "* * * * * *", QWidget *parent = nullptr);
        ~WeeksMonthsDialog();

        QString cronText() const {return result.join(" ");};
        QString currentCronText() const;

    private:
        Ui::WeeksMonthsDialog *ui;
        int m_type;
        QString m_source;
        QList<bool> m_values;
        QStringList result;
        const QList<QString> month_names{ "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
        const QList<QString> day_names{ "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

        void createWeeksControls();
        void createMonthControls();

        void updateCron();

    private slots:
        void onCheckToggled(bool state);
    };
}
#endif // WEEKSMONTHSDIALOG_H
