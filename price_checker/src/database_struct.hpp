#ifndef DATABASE_STRUCT_HPP
#define DATABASE_STRUCT_HPP

#include "arcirk.hpp"

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), database_config,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
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
        (std::string, workplace)
        (std::string, price)
        (std::string, warehouse)
        (std::string, subdivision)
        (std::string, organization)
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
        (arcirk::database), pending_operations,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (std::string, operation)
        (std::string, parent)
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
        (std::string, vendor_code)
        (std::string, product)
        (std::string, parent)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), document_marked_table,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, qr_code)
        (std::string, document_ref)
        (std::string, parent)
        (int, quantity)
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
        (int, is_marked)
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
        (std::string, vendor_code)
        (std::string, first_name)
        (int, is_group)
        (int, deletion_mark)
        (int, version)
);


BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), table_info_sqlite,
        (int, cid)
        (std::string, name)
        (std::string, type)
        (int, notnull)
        (std::string, dflt_value)
        (int, bk)
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
        (std::string, parent)
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
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), subdivisions,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), warehouses,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (int, version)
);

BOOST_FUSION_DEFINE_STRUCT(
        (arcirk::database), price_types,
        (int, _id)
        (std::string, first)
        (std::string, second)
        (std::string, ref)
        (std::string, cache)
        (int, version)
);

namespace arcirk::database{

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
        tbPendingOperations,
        tbBarcodes,
        tbDocumentsMarkedTables,
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
        {tbPendingOperations, "PendingOperations"}  ,
        {tbBarcodes, "Barcodes"}  ,
        {tbDocumentsMarkedTables, "DocumentsMarkedTables"}  ,
    })

    enum views{
        dvDevicesView,
        dvDocumentsTableView,
        views_INVALID=-1,
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(views, {
        { views_INVALID, nullptr }    ,
        { dvDevicesView, "DevicesView" }  ,
        { dvDocumentsTableView, "DocumentsTableView" }  ,
    });

    enum devices_type{
        devDesktop,
        devServer,
        devPhone,
        devTablet,
        devExtendedLib,
        dev_INVALID=-1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(devices_type, {
        {dev_INVALID, nullptr},
        {devDesktop, "Desktop"},
        {devServer, "Server"},
        {devPhone, "Phone"},
        {devTablet, "Tablet"},
        {devExtendedLib, "ExtendedLib"},
    });

//    const std::string documents_table_ddl = "CREATE TABLE Documents (\n"
//                                            "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                            "    [first]         TEXT,\n"
//                                            "    second          TEXT,\n"
//                                            "    ref             TEXT (36) UNIQUE\n"
//                                            "                             NOT NULL,\n"
//                                            "    cache           TEXT      DEFAULT \"\",\n"
//                                            "    number          TEXT      DEFAULT \"\",\n"
//                                            "    date            INTEGER NOT NULL DEFAULT(0),\n"
//                                            "    xml_type        TEXT      DEFAULT \"\",\n"
//                                            "    version         INTEGER NOT NULL DEFAULT(0),\n"
//                                            "    device_id       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                            "    workplace       TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                            "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                            "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
//                                            "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL"
//                                            ");";

//    const std::string document_table_table_ddl = "CREATE TABLE DocumentsTables (\n"
//                                                 "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                                 "    [first]         TEXT,\n"
//                                                 "    second          TEXT,\n"
//                                                 "    ref             TEXT (36) UNIQUE\n"
//                                                 "                             NOT NULL,\n"
//                                                 "    cache           TEXT      DEFAULT \"\",\n"
//                                                 "    price           DOUBLE DEFAULT (0),\n"
//                                                 "    quantity        DOUBLE DEFAULT (0),\n"
//                                                 "    barcode         TEXT      DEFAULT \"\",\n"
//                                                 "    vendor_code     TEXT DEFAULT \"\",\n"
//                                                 "    product         TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                                 "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                                 "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
//                                                 "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
//                                                 "    version         INTEGER NOT NULL DEFAULT(0)\n"
//                                                 ");";

//    const std::string nomenclature_table_ddl = "CREATE TABLE Nomenclature (\n"
//                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                               "    [first]         TEXT,\n"
//                                               "    second          TEXT,\n"
//                                               "    ref             TEXT (36) UNIQUE\n"
//                                               "                             NOT NULL,\n"
//                                               "    cache           TEXT      DEFAULT \"\",\n"
//                                               "    vendor_code     TEXT DEFAULT \"\",\n"
//                                               "    trademark       TEXT DEFAULT \"\",\n"
//                                               "    unit            TEXT DEFAULT \"шт.\",\n"
//                                               "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                               "    is_marked       INTEGER   DEFAULT (0) NOT NULL,\n"
//                                               "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
//                                               "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
//                                               "    version         INTEGER NOT NULL DEFAULT(0)\n"
//                                               ");";

//    const std::string document_table_marked_table_ddl = "CREATE TABLE DocumentsMarkedTables (\n"
//                                                 "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                                 "    [first]         TEXT,\n"
//                                                 "    second          TEXT,\n"
//                                                 "    ref             TEXT (36) UNIQUE\n"
//                                                 "                             NOT NULL,\n"
//                                                 "    qr_code         TEXT,\n"
//                                                 "    document_ref    TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000] NOT NULL,\n"
//                                                 "    quantity        INTEGER DEFAULT (1),\n"
//                                                 "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000] NOT NULL,\n"
//                                                 "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
//                                                 "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
//                                                 "    version         INTEGER NOT NULL DEFAULT(0)\n"
//                                                 ");";

//    const std::string database_config_table_ddl = "CREATE TABLE DatabaseConfig (\n"
//                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                               "    [first]         TEXT,\n"
//                                               "    second          TEXT,\n"
//                                               "    ref             TEXT (36) UNIQUE\n"
//                                               "                             NOT NULL,\n"
//                                               "    version         INTEGER  DEFAULT(0)  NOT NULL\n"
//                                               ");";

//    const std::string pending_operations_table_ddl = "CREATE TABLE PendingOperations (\n"
//                                               "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                               "    [first]         TEXT,\n"
//                                               "    second          TEXT,\n"
//                                               "    ref             TEXT (36) UNIQUE\n"
//                                               "                             NOT NULL,\n"
//                                               "    operation       TEXT,\n"
//                                               "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000]\n"
//                                               ");";

//    const std::string barcodes_table_ddl = "CREATE TABLE Barcodes (\n"
//                                           "    _id             INTEGER   PRIMARY KEY AUTOINCREMENT,\n"
//                                           "    [first]         TEXT,\n"
//                                           "    second          TEXT,\n"
//                                           "    ref             TEXT (36) UNIQUE\n"
//                                           "                             NOT NULL,\n"
//                                           "    barcode         TEXT (128) DEFAULT \"\",\n"
//                                           "    vendor_code     TEXT DEFAULT \"\",\n"
//                                           "    first_name      TEXT DEFAULT \"\",\n"
//                                           "    parent          TEXT (36) DEFAULT [00000000-0000-0000-0000-000000000000],\n"
//                                           "    is_group        INTEGER   DEFAULT (0) NOT NULL,\n"
//                                           "    deletion_mark   INTEGER   DEFAULT (0) NOT NULL,\n"
//                                           "    version         INTEGER NOT NULL DEFAULT(0)\n"
//                                               ");";

    const inline std::string documents_table_view_ddl(){return "CREATE VIEW DocumentsTableView AS\n"
                                                 "    SELECT DocumentsTables._id,\n"
                                                 "           DocumentsTables.[first],\n"
                                                 "           DocumentsTables.second,\n"
                                                 "           DocumentsTables.ref,\n"
                                                 "           DocumentsTables.cache,\n"
                                                 "           DocumentsTables.price,\n"
                                                 "           DocumentsTables.quantity,\n"
                                                 "           DocumentsTables.barcode,\n"
                                                 "           DocumentsTables.product,\n"
                                                 "           DocumentsTables.parent,\n"
                                                 "           DocumentsTables.is_group,\n"
                                                 "           DocumentsTables.deletion_mark,\n"
                                                 "           DocumentsTables.version,\n"
                                                 "           IFNULL(Barcodes.parent, \"\") AS nomenclature,\n"
                                                 "           IFNULL(Nomenclature.is_marked, 0) AS is_marked,\n"
                                                 "           IFNULL(Nomenclature.trademark, \"\") AS trademark,\n"
                                                 "           IFNULL(Nomenclature.unit, \"шт.\") AS unit,\n"
                                                 "           IFNULL(Nomenclature.vendor_code, \"\") AS vendor_code,\n"
                                                 "           IFNULL(Nomenclature.[first], \"\") AS good,\n"
                                                 "           DocumentsTables.barcode || \"/\" || IFNULL(Nomenclature.vendor_code, \"\") AS representation,\n"
                                                 "           COUNT(DocumentsMarkedTables._id) AS marked_quantity\n"
                                                 "      FROM DocumentsTables AS DocumentsTables\n"
                                                 "           LEFT JOIN\n"
                                                 "           Barcodes AS Barcodes ON DocumentsTables.barcode = Barcodes.barcode\n"
                                                 "           LEFT JOIN\n"
                                                 "           Nomenclature AS Nomenclature ON Barcodes.parent = Nomenclature.ref\n"
                                                 "           LEFT JOIN\n"
                                                 "           DocumentsMarkedTables AS DocumentsMarkedTables ON DocumentsTables.ref = DocumentsMarkedTables.parent\n"
               "     GROUP BY DocumentsTables.ref;";};


//    static inline nlohmann::json table_default_json(arcirk::database::tables table) {

//        //using namespace arcirk::database;
//        switch (table) {
//            case tbUsers:{
//                break;
//            }
//            case tbMessages:{
//                break;
//            }
//            case tbOrganizations:{
//                break;
//            }
//            case tbSubdivisions:{
//                break;
//            }
//            case tbWarehouses:{
//                break;
//            }
//            case tbPriceTypes:{
//                break;
//            }
//            case tbWorkplaces:{
//                break;
//            }
//            case tbDevices:{
//                break;
//            }
//            case tbDocumentsTables: {
//                auto tbl = document_table();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.price = 0;
//                tbl.quantity = 0;
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                return pre::json::to_json(tbl);

//            }
//            case tbDocuments: {
//                auto tbl = documents();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.device_id = arcirk::uuids::nil_string_uuid();
//                tbl.date = 0;
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                tbl.is_group = 0;
//                tbl.deletion_mark = 0;
//                return pre::json::to_json(tbl);
//            }
//            case tbPendingOperations: {
//                auto tbl = pending_operations();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                return pre::json::to_json(tbl);
//            }
//            case tbNomenclature: {
//                auto tbl = nomenclature();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                tbl.is_group = 0;
//                tbl.deletion_mark = 0;
//                return pre::json::to_json(tbl);
//            }
//            case tbBarcodes: {
//                auto tbl = barcodes();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                tbl.is_group = 0;
//                tbl.deletion_mark = 0;
//                return pre::json::to_json(tbl);
//            }
//            case tbDocumentsMarkedTables: {
//                auto tbl = document_marked_table();
//                tbl.ref = arcirk::uuids::nil_string_uuid();
//                tbl.first = "Qr code";
//                tbl.second = "Qr code";
//                tbl.quantity = 1;
//                tbl.document_ref = arcirk::uuids::nil_string_uuid();
//                tbl.parent = arcirk::uuids::nil_string_uuid();
//                tbl.is_group = 0;
//                tbl.deletion_mark = 0;
//                return pre::json::to_json(tbl);
//            }
//            case tables_INVALID:{
//                break;
//            }
//            case tbDevicesType:
//                break;
//        }

//        return {};
//    }

//    template<typename T>
//    static inline T table_default_struct(arcirk::database::tables table){
//        auto j = table_default_json(table);
//        auto result = pre::json::from_json<T>(j);
//        return result;
//    }
}
#endif // DATABASE_STRUCT_HPP
