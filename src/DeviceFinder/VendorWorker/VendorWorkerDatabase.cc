#include "DevicesFinder/VendorWorker/VendorWorkerDatabase.h"

VendorWorkerDatabase::~VendorWorkerDatabase() {
    if (m_data_base) {
        sqlite3_close(m_data_base);
    }
}

bool VendorWorkerDatabase::updateFilesFromOfficialSrc() {

    return true;
}

bool VendorWorkerDatabase::loadFile( const std::string &path, bool is_default ) {
    std::cout << "Current folder: " << std::filesystem::current_path() << '\n';

    // if (sqlite3_open(vendor_db_path.c_str(), &m_data_base) != SQLITE_OK) {
    //     std::cerr << "Can't open database: " + std::string(sqlite3_errmsg(m_data_base)) << std::endl;
    // }

    return true;
}

std::string VendorWorkerDatabase::doFindVendor( const std::string &mac ) const {
    std::string vendor_str{};

    return vendor_str;
}