#pragma once

#include "DevicesFinder/VendorWorker/VendorWorker.h"
#include <sqlite3.h>
#include <iostream>
#include <filesystem>

#define DEFAULT_NAME "macvendor.db"

// -> Модуль, позволяющий найти поставщика устройства в базе .db
class VendorWorkerDatabase : public IVendorWorker, public std::enable_shared_from_this<VendorWorkerDatabase> {
private:
    sqlite3 *m_data_base{ nullptr };
public:
    ~VendorWorkerDatabase() override;
    bool updateFilesFromOfficialSrc() override;
    bool loadFile( const std::string &path, bool is_default ) override;
    std::string doFindVendor( const std::string &mac ) const override;
};