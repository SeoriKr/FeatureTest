#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cerrno>

struct FVector 
{ 
    float x = 0;
    float y = 0;
    float z = 0;
};

struct MtlMaterial {
    std::string Name;

    float Ns = 0.f;        // Specular exponent
    FVector Ka{ 0,0,0 };      // Ambient
    FVector Kd{ 0,0,0 };      // Diffuse
    FVector Ks{ 0,0,0 };      // Specular
    FVector Ke{ 0,0,0 };      // Emissive
    float Ni = 1.f;        // IOR

    float d = 1.f;        // Opacity (1=opaque)
    int   illum = 2;       // Shading model

    std::string map_Kd;    // Diffuse map
    std::string map_Ks;    // Specular map (optional)
    std::string map_Ke;    // Emissive map (optional)
    std::string map_Ns;    // Specular exponent map (optional)
    std::string map_d;     // Opacity map (optional)

    // Normal/Bump
    std::string map_Bump;  // bump/normal map file
    float bumpScale = 1.f; // -bm 값

    // 필요 시 추가 키들 계속 확장 가능
};

static inline void trim(std::string& t) {
    auto notsp = [](unsigned char c) { return !std::isspace(c); };
    t.erase(t.begin(), std::find_if(t.begin(), t.end(), notsp));
    t.erase(std::find_if(t.rbegin(), t.rend(), notsp).base(), t.end());
}

static std::vector<std::string> tokenize(std::string_view sv) {
    std::vector<std::string> out;
    size_t i = 0, n = sv.size();
    while (i < n) {
        while (i < n && std::isspace(static_cast<unsigned char>(sv[i]))) ++i;
        if (i >= n) break;

        if (sv[i] == '"') {
            ++i;
            size_t start = i;
            while (i < n && sv[i] != '"') ++i;
            out.emplace_back(sv.substr(start, i - start));
            if (i < n && sv[i] == '"') ++i;
        }
        else {
            size_t start = i;
            while (i < n && !std::isspace(static_cast<unsigned char>(sv[i]))) ++i;
            out.emplace_back(sv.substr(start, i - start));
        }
    }
    return out;
}

static bool parseFloat(std::string_view sv, float& out) {
    std::string tmp(sv);
    char* end = nullptr;
    errno = 0;
    float v = std::strtof(tmp.c_str(), &end);
    if (end == tmp.c_str() + tmp.size() && errno != ERANGE) { out = v; return true; }
    return false;
}

static bool parseFloat3(const std::vector<std::string>& t, size_t i, FVector& out) {
    if (i + 2 >= t.size()) return false;
    return parseFloat(t[i], out.x) && parseFloat(t[i + 1], out.y) && parseFloat(t[i + 2], out.z);
}

static void parseMapWithOptions(const std::vector<std::string>& tok, size_t start,
    std::string& outFile, float* outBm /*nullable*/) {
    // map_* 라인에서 옵션(-bm 등)과 파일명 분리
    // 예: map_Bump -bm 2.9 "House T3N.png"
    float bm = outBm ? *outBm : 1.f;
    std::string file;
    for (size_t i = start; i < tok.size(); ++i) {
        if (tok[i] == "-bm" && i + 1 < tok.size()) {
            float v;
            if (parseFloat(tok[i + 1], v)) bm = v;
            ++i; // 값 스킵
        }
        else if (!tok[i].empty()) {
            file = tok[i]; // 마지막 토큰을 파일로 간주 (공백 포함 파일은 위 토크나이저로 이미 한 토큰)
        }
    }
    if (!file.empty()) outFile = std::move(file);
    if (outBm) *outBm = bm;
}

static void parseMtl(std::istream& is, std::vector<MtlMaterial>& outMats) {
    std::string line;
    MtlMaterial cur;
    bool hasCur = false;

    auto flushCurrent = [&]() {
        if (hasCur) outMats.push_back(cur);
        cur = MtlMaterial{};
        hasCur = false;
        };

    while (std::getline(is, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

        // key + rest
        size_t sp = line.find_first_of(" \t");
        std::string key = (sp == std::string::npos) ? line : line.substr(0, sp);
        std::string rest = (sp == std::string::npos) ? "" : line.substr(sp + 1);
        trim(rest);

        auto tok = tokenize(rest);

        if (key == "newmtl") {
            // 이전 재질 flush
            flushCurrent();
            if (!tok.empty()) { cur.Name = tok[0]; hasCur = true; }
        }
        else if (key == "Ns" && !tok.empty()) { parseFloat(tok[0], cur.Ns); }
        else if (key == "Ni" && !tok.empty()) { parseFloat(tok[0], cur.Ni); }
        else if (key == "d" && !tok.empty()) { parseFloat(tok[0], cur.d); }
        else if (key == "illum" && !tok.empty()) {
            float f; if (parseFloat(tok[0], f)) cur.illum = static_cast<int>(f);
        }
        else if (key == "Ka") { FVector v; if (parseFloat3(tok, 0, v)) cur.Ka = v; }
        else if (key == "Kd") { FVector v; if (parseFloat3(tok, 0, v)) cur.Kd = v; }
        else if (key == "Ks") { FVector v; if (parseFloat3(tok, 0, v)) cur.Ks = v; }
        else if (key == "Ke") { FVector v; if (parseFloat3(tok, 0, v)) cur.Ke = v; }
        else if (key == "map_Kd") { parseMapWithOptions(tok, 0, cur.map_Kd, nullptr); }
        else if (key == "map_Ks") { parseMapWithOptions(tok, 0, cur.map_Ks, nullptr); }
        else if (key == "map_Ke") { parseMapWithOptions(tok, 0, cur.map_Ke, nullptr); }
        else if (key == "map_Ns") { parseMapWithOptions(tok, 0, cur.map_Ns, nullptr); }
        else if (key == "map_d") { parseMapWithOptions(tok, 0, cur.map_d, nullptr); }
        else if (key == "map_Bump" || key == "bump") {
            parseMapWithOptions(tok, 0, cur.map_Bump, &cur.bumpScale);
        }
        else {
            // TODO: 필요하면 로그/커스텀 속성 저장
            // std::cerr << "Unknown key: " << key << "\n";
        }
    }

    // 마지막 재질 flush
    flushCurrent();
}

int main() {
    const char* text =
        "newmtl HouseT3\n"
        "Ns 0.000000\n"
        "Ka 0.300000 0.300000 0.300000\n"
        "Ks 0.000000 0.000000 0.000000\n"
        "Ke 0.000000 0.000000 0.000000\n"
        "Ni 1.500000\n"
        "d 1.000000\n"
        "illum 1\n"
        "map_Kd HouseT3.png\n"
        "map_Bump -bm 2.900000 \"House T3N.png\"\n";

    std::istringstream iss(text);
    std::vector<MtlMaterial> mats;
    parseMtl(iss, mats);

    // 출력 확인
    for (auto& m : mats) {
        std::cout << "material: " << m.Name << "\n";
        std::cout << "  Ns=" << m.Ns << " Ni=" << m.Ni << " d=" << m.d << " illum=" << m.illum << "\n";
        std::cout << "  Ka=(" << m.Ka.x << "," << m.Ka.y << "," << m.Ka.z << ")\n";
        std::cout << "  Kd=(" << m.Kd.x << "," << m.Kd.y << "," << m.Kd.z << ")\n";
        std::cout << "  Ks=(" << m.Ks.x << "," << m.Ks.y << "," << m.Ks.z << ")\n";
        std::cout << "  Ke=(" << m.Ke.x << "," << m.Ke.y << "," << m.Ke.z << ")\n";
        std::cout << "  map_Kd=" << m.map_Kd << "\n";
        std::cout << "  map_Bump=" << m.map_Bump << " (bm=" << m.bumpScale << ")\n";
    }
}
