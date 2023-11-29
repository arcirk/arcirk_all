//
// Created by admin on 05.01.2023.
//
#include "barcode_parser.hpp"

BarcodeParser::BarcodeParser(QObject *parent)
    : QObject{parent}
{

}

void BarcodeParser::addSumbol(const int key, const QString& value)
{
    if(key == 16777270){
        m_sumbols.clear();
    }

    if(key == 16777220){
        QStringList lst;
        foreach(auto itr , m_sumbols){
            lst.append(itr);
        }

        emit barcode(lst.join(""));
        m_sumbols.clear();
    }
    else{
        if(key >= 33 && key <= 127)
            m_sumbols.push_back(value);
    }
}
