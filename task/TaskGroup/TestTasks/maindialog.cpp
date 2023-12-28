#include "maindialog.h"
#include "ui_maindialog.h"
#include "include/libcron/Cron.h"
#include <iostream>
#include <QDialogButtonBox>
#include <QPushButton>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

//    using namespace std::chrono_literals;

//    libcron::Cron cron;
//    cron.add_schedule("Hello from Cron", "* * * * * ?", [=](auto&) {
//        std::cout << "Hello from libcron!" << std::endl;
//    });

//    while(true)
//    {
//        cron.tick();
//        std::this_thread::sleep_for(500ms);
//    }

    m_task_list = new tasks::TaskListsWidget(this);
    ui->verticalLayout->addWidget(m_task_list);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MainDialog::onButtonBoxClicked);

    m_task_list->load();

    connect(m_task_list, &tasks::TaskListsWidget::taskListChanged, this, &MainDialog::onTaskListChanged);
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::onButtonBoxClicked(QAbstractButton *button)
{
    if(button == ui->buttonBox->button(QDialogButtonBox::Cancel))
        QApplication::quit();
}

void MainDialog::onTaskListChanged()
{
    m_task_list->save();
}

