#pragma once

#include "Core/DevicesFinder/VendorWorker/VendorWorker.h"

// -> Модуль, позволяющий найти поставщика устройства в файле .csv
class VendorWorkerCsv : public IVendorWorker, public std::enable_shared_from_this<VendorWorkerCsv> {
public:
    ~VendorWorkerCsv() override = default;
    bool updateFilesFromOfficialSrc() override;
    bool loadFile( const std::string &path, bool is_default ) override;
    std::string doFindVendor( const std::string &mac ) const override;
};