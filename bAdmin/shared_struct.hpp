#ifndef SHARED_STRUCT_HPP
#define SHARED_STRUCT_HPP

#ifdef _WINDOWS
    #pragma warning(disable:4100)
    #pragma warning(disable:4267)
#endif

#include "global/arcirk_qt.hpp"

#define CLIENT_VERSION 3

#define FAT12          "FAT12"
#define REGISTRY       "REGISTRY"
#define HDIMAGE        "HDIMAGE"
#define DATABASE       "DATABASE"
#define REMOTEBASE     "REMOTEBASE"

#define CRYPT_KEY "my_key"

#define APP_NAME "bAdmin"


//#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"
#define SHARED_CHANNEL_UUID "3e3b54bf-4319-4e73-9917-22f06cc1bfbd"
#define MESSAGES_DATE_GROUP_UUID "78cb038c-4761-442e-b2d5-01c359dc2a53"


BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::services), task_options,
    (std::string, uuid)
    (std::string, name)
    (std::string, synonum)
    (bool, predefined)
    (int, start_task)
    (int, end_task)
    (int, interval)
    (int, type_script)
    (bool, allowed)
    (std::string, days_of_week)
    (ByteArray, script)
    (std::string, script_synonum)
    (std::string, comment)
    (ByteArray, param)
)


BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), mstsc_options,
    (std::string, name)
    (std::string, address)
    (int, port)
    (bool, def_port)
    (bool, not_full_window)
    (int, width)
    (int, height)
    (bool, reset_user)
    (std::string, user_name)
    (std::string, password)
    (std::string, uuid)
)
BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), mpl_options,
    (bool, use_firefox)
    (std::string, firefox_path)
    (std::string, profiles_path)
    (ByteArray, mpl_list)
    (ByteArray, mpl_profiles)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), mpl_item,
    (std::string, profile)
    (std::string, name)
    (std::string, url)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), cryptopro_data,
    (std::string, cryptopro_path)
    (ByteArray, certs)
    (ByteArray, conts)
)


namespace arcirk::server{

    enum application_names{
        PriceChecker,
        ServerManager,
        ExtendedLib,
        ProfileManager,
        APP_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(application_names, {
        {APP_INVALID, nullptr} ,
        {PriceChecker, "PriceChecker"} ,
        {ServerManager, "ServerManager"} ,
        {ExtendedLib, "ExtendedLib"} ,
        {ProfileManager, "ProfileManager"} ,
    });

    enum device_types{
        devDesktop,
        devServer,
        devPhone,
        devTablet,
        devExtendedLib,
        DEV_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(device_types, {
       {DEV_INVALID, nullptr} ,
       {devDesktop, "Desktop"} ,
       {devServer, "Server"} ,
       {devPhone, "Phone"} ,
       {devTablet, "Tablet"} ,
       {devExtendedLib, "ExtendedLib"},
   });
}


BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), user_info,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, hash)
        (std::string, role)
        (std::string, performance)
        (std::string, parent)
        (std::string, cache)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), messages,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, message)
        (std::string, token)
        (int, date)
        (std::string, content_type)
        (int, unread_messages)
        (int, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), organizations,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), subdivisions,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), warehouses,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), price_types,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), workplaces,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, server)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), devices,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, deviceType)
        (std::string, address)
        (std::string, workplace)
        (std::string, price_type)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), devices_view,
        (std::string, ref)
        (std::string, first)
        (std::string, second)
        (std::string, device_type)
        (std::string, workplace)
        (std::string, price)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
        (std::string, address)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), documents,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, number)
        (int, date)
        (std::string, xml_type)
        (int, version)
        (std::string, device_id)
        (std::string, workplace)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_table,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (double, price)
        (double, quantity)
        (std::string, barcode)
        (std::string, product)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), nomenclature,
        (int, _id)
        (std::string, first) // Наименование
        (std::string, second)
        (std::string, ref)
        (std::string, cache) // Все остальные реквизиты
        (std::string, parent)
        (std::string, vendor_code)
        (std::string, trademark)
        (std::string, unit)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), barcodes,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, barcode)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), containers,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (ByteArray, data)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, sha1)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), certificates,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (ByteArray, data)
        (std::string, private_key)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, serial)
        (std::string, suffix)
        (std::string, sha1)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), certificates_view,
        (std::string, first)
        (std::string, ref)
        (std::string, cache)
        (std::string, private_key)
        (std::string, subject)
        (std::string, issuer)
        (std::string, not_valid_before)
        (std::string, not_valid_after)
        (std::string, parent_user)
        (std::string, serial)
        (std::string, suffix)
        (std::string, sha1)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), cert_users,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, uuid)
        (std::string, sid)
        (std::string, system_user)
        (std::string, host)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), available_certificates,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, user_uuid)
        (std::string, cert_uuid)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), registered_users_view,
        (std::string, ref)
        (std::string, first)
        (std::string, uuid)
        (std::string, parent)
        (int, is_group)
        (int, unread)
);

namespace arcirk::database {
    enum tables{
        tbUsers,
        tbMessages,
        tbOrganizations,
        tbSubdivisions,
        tbWarehouses,
        tbPriceTypes,
        tbWorkplaces,
        tbDevices,
        tbDevicesType,
        tbDocuments,
        tbDocumentsTables,
        tbNomenclature,
        tbDatabaseConfig,
        tbBarcodes,
        tbDocumentsMarkedTables,
        tbCertificates,
        tbCertUsers,
        tbContainers,
        tbAvailableCertificates,
        tables_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tables, {
        {tables_INVALID, nullptr}    ,
        {tbUsers, "Users"}  ,
        {tbMessages, "Messages"}  ,
        {tbOrganizations, "Organizations"}  ,
        {tbSubdivisions, "Subdivisions"}  ,
        {tbWarehouses, "Warehouses"}  ,
        {tbPriceTypes, "PriceTypes"}  ,
        {tbWorkplaces, "Workplaces"}  ,
        {tbDevices, "Devices"}  ,
        {tbDevicesType, "DevicesType"}  ,
        {tbDocuments, "Documents"}  ,
        {tbDocumentsTables, "DocumentsTables"}  ,
        {tbNomenclature, "Nomenclature"}  ,
        {tbDatabaseConfig, "DatabaseConfig"}  ,
        {tbBarcodes, "Barcodes"}  ,
        {tbDocumentsMarkedTables, "DocumentsMarkedTables"}  ,
        {tbCertificates, "Certificates"}  ,
        {tbCertUsers, "CertUsers"}  ,
        {tbContainers, "Containers"}  ,
        {tbAvailableCertificates, "AvailableCertificates"}  ,
    })

    enum roles{
         dbUser,
         dbAdministrator,
         roles_INVALID=-1,
     };

     NLOHMANN_JSON_SERIALIZE_ENUM(roles, {
         {roles_INVALID, nullptr}    ,
         {dbAdministrator, "admin"}  ,
         {dbUser, "user"}  ,
     })

    static inline nlohmann::json table_default_json(arcirk::database::tables table) {

          //using namespace arcirk::database;
          switch (table) {
              case tbUsers:{
                  auto usr_info = user_info();
                  usr_info.ref = arcirk::uuids::nil_string_uuid();
                  usr_info.parent = arcirk::uuids::nil_string_uuid();
                  usr_info.is_group = 0;
                  usr_info.deletion_mark = 0;
                  return pre::json::to_json(usr_info);
              }
              case tbMessages:{
                  auto tbl = messages();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.content_type ="Text";
                  tbl.parent = 0;
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
                  //std::string tbl_json = to_string(pre::json::to_json(tbl));
                  //return tbl_json;
              }
              case tbOrganizations:{
                  auto tbl = organizations();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbSubdivisions:{
                  auto tbl = subdivisions();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbWarehouses:{
                  auto tbl = warehouses();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbPriceTypes:{
                  auto tbl = price_types();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbWorkplaces:{
                  auto tbl = workplaces();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.server = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDevices:{
                  auto tbl = devices();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.deviceType = "Desktop";
                  tbl.address = "127.0.0.1";
                  tbl.workplace = arcirk::uuids::nil_string_uuid();
                  tbl.price_type = arcirk::uuids::nil_string_uuid();
                  tbl.warehouse = arcirk::uuids::nil_string_uuid();
                  tbl.subdivision = arcirk::uuids::nil_string_uuid();
                  tbl.organization = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDocumentsTables: {
                  auto tbl = document_table();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.price = 0;
                  tbl.quantity = 0;
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
    //                std::string tbl_json = to_string(pre::json::to_json(tbl));
    //                return tbl_json;
              }
              case tbDocumentsMarkedTables: {
//                  auto tbl = document_marked_table();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.quantity = 1;
//                  tbl.document_ref = arcirk::uuids::nil_string_uuid();
//                  tbl.parent = arcirk::uuids::nil_string_uuid();
//                  tbl.is_group = 0;
//                  tbl.deletion_mark = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbDocuments: {
//                  auto tbl = documents();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.device_id = arcirk::uuids::nil_string_uuid();
//                  tbl.date = date_to_seconds();
//                  tbl.parent = arcirk::uuids::nil_string_uuid();
//                  tbl.is_group = 0;
//                  tbl.deletion_mark = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbNomenclature: {
                  auto tbl = nomenclature();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  return pre::json::to_json(tbl);
              }
              case tbBarcodes: {
                  auto tbl = barcodes();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tbDatabaseConfig: {
//                  auto tbl = database_config();
//                  tbl.ref = arcirk::uuids::nil_string_uuid();
//                  tbl.version = 0;
//                  return pre::json::to_json(tbl);
                    break;
              }
              case tbCertificates: {
                  auto tbl = certificates();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  tbl._id = 0;
                  return pre::json::to_json(tbl);
              }
              case tbCertUsers: {
                  auto tbl = cert_users();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tbContainers: {
                  auto tbl = containers();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tbAvailableCertificates: {
                  auto tbl = available_certificates();
                  tbl.ref = arcirk::uuids::nil_string_uuid();
                  tbl.parent = arcirk::uuids::nil_string_uuid();
                  tbl.user_uuid = arcirk::uuids::nil_string_uuid();
                  tbl.cert_uuid = arcirk::uuids::nil_string_uuid();
                  tbl.is_group = 0;
                  tbl.deletion_mark = 0;
                  tbl.version = 0;
                  return pre::json::to_json(tbl);
              }
              case tables_INVALID:{
                  break;
              }
              case tbDevicesType:
                  //return devices_type();
                break;
          }

          return {};
      }

    template<typename T>
    static inline T table_default_struct(arcirk::database::tables table){
        auto j = table_default_json(table);
        auto result = pre::json::from_json<T>(j);
        return result;
    }

    enum text_type{
        dbText,
        dbHtmlText,
        dbXmlText,
        dbJsonText,
        text_type_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(text_type, {
        {text_type_INVALID, nullptr}    ,
        {dbText, "Text"}  ,
        {dbHtmlText, "HtmlText"}  ,
        {dbXmlText, "XmlText"}  ,
        {dbJsonText, "JsonText"}  ,
    })
}

namespace arcirk {

    static inline nlohmann::json json_keys(const nlohmann::json& object){
        if(!object.is_object())
            return nlohmann::json::array();
        else{
            auto result = nlohmann::json::array();
            auto items = object.items();
            for (auto itr = items.begin(); itr != items.end(); ++itr) {
                result.push_back(itr.key());
            }
            return result;
        }
    }
}

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), cert_info,
    (std::string, serial)
    (std::string, issuer)
    (std::string, subject)
    (std::string, not_valid_before)
    (std::string, not_valid_after)
    (ByteArray, data)
    (std::string, sha1)
    (std::string, suffix)
    (std::string, cache)
    (std::string, private_key)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), cont_info,
    (ByteArray, header_key)
    (ByteArray, masks_key)
    (ByteArray, masks2_key)
    (ByteArray, name_key)
    (ByteArray, primary_key)
    (ByteArray, primary2_key)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::cryptography), win_user_info,
    (std::string, user)
    (std::string, sid)
)

namespace arcirk::command_line {
    enum CmdCommand{
        echoSystem,
        echoUserName,
        wmicGetSID,
        echoGetEncoding,
        csptestGetConteiners,
        csptestContainerCopy,
        csptestContainerFnfo,
        csptestContainerDelete,
        csptestGetCertificates,
        certutilGetCertificateInfo,
        certmgrInstallCert,
        certmgrExportlCert,
        certmgrDeletelCert,
        certmgrGetCertificateInfo,
        cryptcpCopycert,
        mstscAddUserToConnect,
        mstscEditFile,
        quserList,
        mstscRunAsAdmin,
        cmdCD,
        cmdEXIT,
        wmicUserAccount,
        wmicUsers,
        COMMAND_INVALID=-1,
    };
}
namespace arcirk::cryptography{

    enum TypeOfStorgare{
        storgareTypeRegistry,
        storgareTypeLocalVolume,
        storgareTypeDatabase,
        storgareTypeRemoteBase,
        storgareTypeUnknown = -1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TypeOfStorgare,{
           {storgareTypeRegistry, REGISTRY},
           {storgareTypeLocalVolume, FAT12},
           {storgareTypeDatabase, DATABASE},
           {storgareTypeRemoteBase, REMOTEBASE},
    })

    inline TypeOfStorgare type_storgare(const std::string& source){
        if(source.empty())
            return storgareTypeUnknown;
        else{
            if(index_of(source, FAT12) != -1 || index_of(source, HDIMAGE) != -1)
                return storgareTypeLocalVolume;
            else if(index_of(source, REGISTRY) != -1)
                return storgareTypeRegistry;
            else if(index_of(source, DATABASE) != -1)
                return storgareTypeDatabase;
            else if(index_of(source, REMOTEBASE) != -1)
                return storgareTypeDatabase;
            else{
                return storgareTypeUnknown;
            }
        }
    }

}

#endif // SHARED_STRUCT_HPP
