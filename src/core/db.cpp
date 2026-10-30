#include "core/db.hpp"

#include <fstream>
#include <sstream>

static Row split(const std::string& s) {
    Row r;
    std::stringstream ss(s);
    std::string f;
    while (std::getline(ss, f, '|')) r.push_back(f);
    return r;
}

std::vector<Row> readDb(const fs::path& p) {
    std::vector<Row> v;
    std::ifstream in(p);
    std::string l;
    while (std::getline(in, l)) {
        Row r = split(l);
        if (r.size() >= 3) v.push_back(r);
    }
    return v;
}

void writeDb(const fs::path& p, const std::vector<Row>& v) {
    std::ofstream o(p);
    for (auto& r : v) {
        for (size_t i = 0; i < r.size(); ++i) o << (i ? "|" : "") << r[i];
        o << '\n';
    }
}

std::string lower(std::string s) {
    for (auto& c : s) if (c >= 'A' && c <= 'Z') c += 32;
    return s;
}

int findRow(const std::vector<Row>& v, const std::string& name) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i][0] == name) return (int)i;
    return -1;
}

std::string col(const Row& r, size_t i) { return i < r.size() ? r[i] : "?"; }
