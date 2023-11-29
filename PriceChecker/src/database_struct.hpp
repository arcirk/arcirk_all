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


//BOOST_FUSION_DEFINE_STRUCT(
//        (arcirk::database), table_info_sqlite,
//        (int, cid)
//        (std::string, name)
//        (std::string, type)
//        (int, notnull)
//        (std::string, dflt_value)
//        (int, bk)
//);

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

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), document_table_view,
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
    (std::string, nomenclature)
    (std::string, trademark)
    (std::string, unit)
    (std::string, good)
    (std::string, representation)
    (int, is_marked)
    (int, marked_quantity)
    (int, is_group)
    (int, deletion_mark)
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
                                                 "           DocumentsTables.barcode || \"/\" || IFNULL(Nomenclature.vendor_code, \"\") || \"/\" || IFNULL(Nomenclature.[first], \"\") AS representation,\n"
                                                 "           COUNT(DocumentsMarkedTables._id) AS marked_quantity\n"
                                                 "      FROM DocumentsTables AS DocumentsTables\n"
                                                 "           LEFT JOIN\n"
                                                 "           Barcodes AS Barcodes ON DocumentsTables.barcode = Barcodes.barcode\n"
                                                 "           LEFT JOIN\n"
                                                 "           Nomenclature AS Nomenclature ON Barcodes.parent = Nomenclature.ref\n"
                                                 "           LEFT JOIN\n"
                                                 "           DocumentsMarkedTables AS DocumentsMarkedTables ON DocumentsTables.ref = DocumentsMarkedTables.parent\n"
               "     GROUP BY DocumentsTables.ref;";};

}
#endif // DATABASE_STRUCT_HPP
