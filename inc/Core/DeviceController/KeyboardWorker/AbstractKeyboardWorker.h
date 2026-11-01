#pragma once

#include <functional>
#include <thread>
#include <iostream>
#include <atomic>

struct KeyCombo {
    std::vector<std::string> keys{};
    std::string modifier{};
    static std::vector<char> doSerialize(const KeyCombo& combo) {
        std::vector<char> out;
        auto put32 = [&](uint32_t v) {
            out.push_back(v & 0xFF); out.push_back((v >> 8) & 0xFF);
            out.push_back((v >> 16) & 0xFF); out.push_back((v >> 24) & 0xFF);
        };
        auto putStr = [&](const std::string& s) {
            put32((uint32_t)s.size());
            out.insert(out.end(), s.begin(), s.end());
        };
        put32((uint32_t)combo.keys.size());
        for (auto& k : combo.keys) putStr(k);
        putStr(combo.modifier);
        return out;
    }
    static KeyCombo doDeserialize(const std::vector<char>& in) {
        KeyCombo combo;
        auto it = in.cbegin();
        auto last = in.cend();

        auto get32 = [&](uint32_t& v) -> bool {
            if (last - it < 4) return false;
            v = (uint8_t)it[0] | ((uint8_t)it[1] << 8) |
                ((uint8_t)it[2] << 16) | ((uint8_t)it[3] << 24);
            it += 4;
            return true;
        };
        auto getStr = [&](std::string& s) -> bool {
            uint32_t n;
            if (!get32(n) || (size_t)(last - it) < n) return false;
            s.assign(it, it + n);
            it += n;
            return true;
        };

        uint32_t n;
        if (!get32(n)) return combo;
        combo.keys.reserve(n);
        for (uint32_t i = 0; i < n; ++i) {
            std::string k;
            if (!getStr(k)) return combo;
            combo.keys.push_back(std::move(k));
        }
        getStr(combo.modifier);
        return combo;
    }
};

class AbstractKeyboardWorker {
protected:
    std::thread m_run_thread;
    std::atomic<bool> m_flag_run{false};
    std::atomic<bool> m_block_input{false};
public:
    virtual ~AbstractKeyboardWorker() = default;
    void setBlockInput( const bool &f ) {
        (f) ? m_block_input.store(true) : m_block_input.store(false);
    }
    virtual void startScan() = 0;
    virtual void stopScan() = 0;
};