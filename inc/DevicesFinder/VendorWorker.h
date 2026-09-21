
#pragma once

#include <memory>
#include <string>
#include <sqlite3.h>

/*
    ! Интерфейс для работы с разными способами получения vendor-а по mac-адресу
*/
class IVendorWorker {
public:
    virtual std::string doFindVendor( const std::string &mac ) const = 0;
    virtual ~IVendorWorker() = default;
};

/* 
    ! Модуль, позволяющий найти поставщика устройства в базе .db 
*/
class VendorWorkerDB final : public IVendorWorker, public std::enable_shared_from_this<VendorWorkerDB> {
private:
    sqlite3 *m_data_base { nullptr };
public:
    std::string doFindVendor( const std::string &mac ) const override;
    ~VendorWorkerDB() override = default;
};

/* 
    ! Модуль, позволяющий найти поставщика устройства в файле .csv 
*/
class VendorWorkerCsv final : public IVendorWorker, public std::enable_shared_from_this<VendorWorkerCsv> {
public:
    std::string doFindVendor( const std::string &mac ) const override;
    ~VendorWorkerCsv() override = default;
};

