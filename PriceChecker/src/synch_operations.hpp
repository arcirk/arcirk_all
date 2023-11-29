#ifndef SYNCH_OPERATIONS_HPP
#define SYNCH_OPERATIONS_HPP

#include <QRunnable>
#include <QDebug>
#include <QDateTime>

template<class SynchClass>
class SynhOperations : public QRunnable{

public:
    explicit SynhOperations(SynchClass * synhClass)
    {
        synchClass_ = synhClass;
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "auto delete:" << autoDelete();
    }

    void run() override{
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
        synchronize();
    }

    void synchronize(){
        synchClass_->synchronize();
    }


private:
    SynchClass * synchClass_;

};

#endif // SYNCH_OPERATIONS_HPP
