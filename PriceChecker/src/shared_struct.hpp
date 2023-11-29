#ifndef ARCIRK_SHARED_MODULE_HPP
#define ARCIRK_SHARED_MODULE_HPP

#include "arcirk.hpp"

//#define ARCIRK_VERSION "1.1.0"
//#define ARCIRK_SERVER_CONF "server_conf.json"
//#define CLIENT_VERSION 2

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), checker_conf,
        (std::string, typePriceRef)
        (std::string, stockRef)
        (std::string, typePrice)
        (std::string, stock)
)
BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), job_progress,
        (int, total)
        (int, done)
        (int, percent)
        (std::string, comment)
)
namespace arcirk::client {

    enum client_commands{
        GetForms,
        GetDocumentConent,
        FormOpen,
        FormLeave,
        UpdateDocumentRow,
        DeleteDocumentRow,
        AddDocumentRow,
        DisconnectForm,
        SetQrCode,
        ErrorResponse,
        DeleteQrCodeRow,
        Import1CDocument,
        StartingLongOberation,
        EndingLongOperation,
        JobProgress,
        CancelLongOperation,
        CMD_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(client_commands, {
        {CMD_INVALID, nullptr}    ,
        {GetForms, "GetForms"}  ,                                    
        {GetDocumentConent, "GetDocumentConent"}  ,
        {FormOpen, "FormOpen"}  ,
        {FormLeave, "FormLeave"}  ,
        {UpdateDocumentRow, "UpdateDocumentRow"}  ,
        {DeleteDocumentRow, "DeleteDocumentRow"}  ,
        {AddDocumentRow, "AddDocumentRow"}  ,
        {DisconnectForm, "DisconnectForm"}  ,
        {SetQrCode, "SetQrCode"}  ,
        {ErrorResponse, "ErrorResponse"}  ,
        {DeleteQrCodeRow, "DeleteQrCodeRow"}  ,
        {Import1CDocument, "Import1CDocument"}  ,
        {StartingLongOberation, "StartingLongOberation"}  ,
        {EndingLongOperation, "EndingLongOperation"}  ,
        {JobProgress, "JobProgress"}  ,
        {CancelLongOperation, "CancelLongOperation"}  ,
    });
}

namespace arcirk::server{

    enum application_names{
        PriceChecker,
        ServerManager,
        ExtendedLib,
        APP_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(application_names, {
        {APP_INVALID, nullptr} ,
        {PriceChecker, "PriceChecker"} ,
        {ServerManager, "ServerManager"} ,
        {ExtendedLib, "ExtendedLib"} ,
    });
}

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::client), client_private_config,
        (bool, showImage)
        (bool, keyboardInputMode)
        (bool, priceCheckerMode)
        (std::string, deviceId)
        (std::string, deviceName)
);

#endif
