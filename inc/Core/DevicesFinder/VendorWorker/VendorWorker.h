
#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <cstdint>

enum class VendorMode : uint32_t {
    Database = 0,
    Csv
};

// -> интерфейс для работы с разными способами получения vendor-а по mac-адресу
class IVendorWorker {
public:
    virtual ~IVendorWorker() = default;
    virtual bool updateFilesFromOfficialSrc() = 0;
    virtual bool loadFile( const std::string &path, bool is_default ) = 0;
    virtual std::string doFindVendor( const std::string &mac ) const = 0;
};
