#pragma once
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <boost/locale/date_time.hpp>

using namespace std;

namespace DragonData
{
namespace Raw
{
    constexpr size_t SCENARIO_COUNT = 4;
    constexpr size_t SCENARIO_DATA_SIZE = 22208;

#pragma pack(push, 1)
    struct Name
    {
        char name[6];
    };

    struct Axis
    {
        uint16_t x;
        uint16_t y;
    };

    struct Troop
    {
        uint16_t count;
        uint16_t troop_type;
    };

    struct Force
    {
        uint8_t status;
        uint8_t warlord;
        uint8_t advisor;
        uint8_t capital;
        uint16_t cavalries;
        uint16_t infantries;
        uint16_t archers;
        uint8_t reserved_1[14];
        uint8_t subordinates;
        uint8_t reserved_2[7];
        uint8_t money[3]; // 24-bit
        uint8_t city_count;
        uint8_t reserved_3[22];
        uint8_t diplomacy_owner;
        uint8_t reserved_4[5];
    };

    struct City
    {
        uint8_t reserved_1;
        uint8_t force;
        Name name;
        Axis axis;
        uint16_t max_productivity;
        uint16_t cur_productivity;
        uint8_t increase;
        uint8_t anti_disaster;
        uint8_t soldiers;
        uint8_t reserved_2[4];
        uint16_t city_type;
        uint8_t affairs_owner;
        uint8_t reserved_3[6];
    };

    struct Legion
    {
        uint8_t state;
        uint8_t force;
        uint8_t leader;
        uint8_t reserved_1;
        uint16_t total_soldier;
        uint8_t morale;
        uint8_t reserved_2[7];
        Axis current_axis;
        uint8_t reserved_3[4];
        Axis target_axis;
        uint8_t reserved_4[6];
        uint8_t target_city;
        uint8_t reserved_5[7];
        Troop troops[6];
    };

    struct Character
    {
        uint8_t property;
        uint8_t avatar;
        Name name;
        Name alias;
        uint8_t siege_ability;
        uint8_t field_ability;
        uint8_t naval_ability;
        uint8_t battle_ability;
        uint8_t command;
        uint8_t politics;
        uint8_t reserved_1[3];
        uint8_t status;
        uint8_t month_to_board;
        uint8_t force_next;
        uint8_t reserved_2[2];
        uint8_t force_or_capture;
        uint8_t force_origin;
        uint8_t reserved_3[2];
    };

    struct GameData
    {
        uint8_t reserved_1[3];
        uint8_t day;
        uint8_t month;
        uint8_t reserved_2;
        uint16_t year;
        uint8_t reserved_3[7];
        uint8_t force;
        uint8_t trust;
        uint8_t number;
        uint8_t reserved_4[6];
        uint16_t cur_tax_rate;
        uint16_t cur_conscription[3];
        uint16_t next_tax_rate;
        uint16_t next_conscription[3];
        uint8_t reserved_5[18];
        uint8_t total_forces;
        uint8_t reserved_6[5];
        char name[32];
        uint8_t reserved_7[32];
    };

    struct Friendship
    {
        uint8_t friendship[24];
    };

    struct Scenario
    {
        GameData game_data;
        Force forces[24];
        Friendship friendship[24];
        City cities[192];
        uint8_t reserved_1[512];
        Legion legions[128];
        Character characters[128];
        uint8_t reserved_2[1024];
    };

    struct File
    {
        Scenario scenarios[4];
    };

#pragma pack(pop)
}

namespace fs = filesystem;

class Element
{
public:
    virtual ~Element() = default;

    explicit Element(const uint index)
        : index(index)
    {
    }

    [[nodiscard]] uint8_t getIndex() const
    {
        return index;
    }

private:
    uint index;
};

wstring name_to_utf8(const char* s, size_t len);

class NamedElement : Element
{
public:
    explicit NamedElement(const uint index)
        : Element(index)
    {
    }

    NamedElement(const uint index, const char* s, const size_t len)
        : Element(index)
          , name(name_to_utf8(s, len))
    {
    }

    NamedElement(const uint index, const NamedElement& other)
        : Element(index)
          , name(other.name)
    {
    }

protected:
    wstring name;

public:
    [[nodiscard]] virtual const wstring& getName() const
    {
        return name;
    }
};

class Axis : public Raw::Axis
{
public:
    explicit Axis(const Raw::Axis& raw) : Raw::Axis(raw)
    {
    }
};

enum TroopType : uint16_t
{
    Infantry = 0,
    Archer = 1,
    Cavalry = 2,
    Spearman = 3,
    Scout = 4,
    Cannon = 5,
    Naval = 6,
    Siege = 7
};

class Troop
{
public:
    explicit Troop(const Raw::Troop& raw)
    {
        count = raw.count;
        troop_type = static_cast<TroopType>(raw.troop_type);
    }

    [[nodiscard]] uint16_t getCount() const
    {
        return count;
    }

    [[nodiscard]] TroopType getTroopType() const
    {
        return troop_type;
    }

private:
    uint16_t count;
    TroopType troop_type;
};

class Force;
class Character;
class City;
class Legion;

typedef shared_ptr<Force> ForcePtr;
typedef shared_ptr<Character> CharacterPtr;
typedef shared_ptr<City> CityPtr;
typedef shared_ptr<Legion> LegionPtr;

typedef optional<ForcePtr> OptionalForcePtr;
typedef optional<CharacterPtr> OptionalCharacterPtr;
typedef optional<CityPtr> OptionalCityPtr;
typedef optional<LegionPtr> OptionalLegionPtr;


typedef vector<CharacterPtr> CharacterPtrVector;
typedef vector<CityPtr> CityPtrVector;
typedef vector<ForcePtr> ForcePtrVector;
typedef vector<LegionPtr> LegionPtrVector;


class Force final : public NamedElement
{
public:
    Force(uint index, const Raw::Force& raw, const CharacterPtrVector& characters, const CityPtrVector& cities);

    [[nodiscard]] uint8_t getStatus() const
    {
        return status;
    }

    [[nodiscard]] const Character* getWarlord() const
    {
        return warlord.get();
    }

    [[nodiscard]] const Character* getAdvisor() const
    {
        return advisor.value_or(nullptr).get();
    }

    [[nodiscard]] const City* getCapital() const
    {
        return capital.get();
    }

    [[nodiscard]] uint16_t getCavalries() const
    {
        return cavalries;
    }

    [[nodiscard]] uint16_t getInfantries() const
    {
        return infantries;
    }

    [[nodiscard]] uint16_t getArchers() const
    {
        return archers;
    }

    [[nodiscard]] uint8_t getSubordinates() const
    {
        return subordinates;
    }

    [[nodiscard]] int32_t getMoney() const
    {
        return money;
    }

    [[nodiscard]] uint8_t getCities() const
    {
        return city_count;
    }

    [[nodiscard]] const Character* getDiplomacyOwner() const
    {
        return diplomacy_owner.value_or(nullptr).get();
    }

private:
    uint8_t status;
    CharacterPtr warlord;
    OptionalCharacterPtr advisor;
    CityPtr capital;
    uint16_t cavalries;
    uint16_t infantries;
    uint16_t archers;
    uint8_t subordinates;
    int32_t money;
    uint8_t city_count;
    OptionalCharacterPtr diplomacy_owner;
};
enum class CharacterStatus
{
    Idle = 0,
    Commander = 1,
    InternalAffairsOfficer = 2,
    Diplomat = 3,
    DeadOrCaptured = 4,
};

inline CharacterStatus CharacterStatusFromRaw(const uint8_t v) noexcept
{
    switch (v)
    {
    case 0:
        return CharacterStatus::Idle;
    case 1:
        return CharacterStatus::Commander;
    case 2:
        return CharacterStatus::InternalAffairsOfficer;
    case 3:
        return CharacterStatus::Diplomat;
    case 4:
        return CharacterStatus::DeadOrCaptured;
    default:
        return CharacterStatus::Idle;
    }
}

class Character final : public NamedElement
{
public:
    Character(uint index, const Raw::Character& raw);

    void resolve(const ForcePtrVector& forces)
    {
        if (force_next_index < forces.size())
        {
            force_next = forces[force_next_index];
        }

        if (force_capture_index < forces.size())
        {
            force_or_capture = forces[force_capture_index];
        }

        if (force_before_capture_index < forces.size())
        {
            force_before_capture = forces[force_before_capture_index];
        }
    }

    const wstring& resolveStatus()
    {
        switch (status)
        {
        case CharacterStatus::Idle:
            if (force_or_capture.has_value())
            {
                status_string = L"待命";
            }
            else if (month_to_board > 0)
            {
                status_string = std::to_wstring(month_to_board) + L"月后登场";
            }
            else
            {
                status_string = L"流亡";
            }
            break;
        case CharacterStatus::Commander:
            status_string = L"军团长";
            break;
        case CharacterStatus::InternalAffairsOfficer:
            status_string = L"内政官";
            break;
        case CharacterStatus::Diplomat:
            status_string = L"外交官";
            break;
        case CharacterStatus::DeadOrCaptured:
            if (force_or_capture.has_value())
            {
                status_string = wstring(L"俘:") + force_or_capture.value()->getName();
            }
            else
            {
                status_string = L"死亡";
            }
            break;
        default:
            status_string = L"未知";
            break;
        }

        return status_string;
    }

    [[nodiscard]] const wstring& getStatusString() const
    {
        return status_string;
    }

    [[nodiscard]] uint8_t getProperty() const
    {
        return property;
    }

    [[nodiscard]] uint8_t getAvatar() const
    {
        return avatar;
    }

    [[nodiscard]] const wstring& getAlias() const
    {
        return alias;
    }

    [[nodiscard]] uint8_t getSiegeAbility() const
    {
        return siege_ability;
    }

    [[nodiscard]] uint8_t getFieldAbility() const
    {
        return field_ability;
    }

    [[nodiscard]] uint8_t getNavalAbility() const
    {
        return naval_ability;
    }

    [[nodiscard]] uint8_t getBattleAbility() const
    {
        return battle_ability;
    }

    [[nodiscard]] uint8_t getCommand() const
    {
        return command;
    }

    [[nodiscard]] uint8_t getPolitics() const
    {
        return politics;
    }

    [[nodiscard]] const CharacterStatus& getStatus() const
    {
        return status;
    }

    [[nodiscard]] uint8_t getMonthToBoard() const
    {
        return month_to_board;
    }

    [[nodiscard]] const Force* getForceNext() const
    {
        return force_next.value_or(nullptr).get();
    }

    [[nodiscard]] const Force* getForceCapture() const
    {
        return force_or_capture.value_or(nullptr).get();
    }

    [[nodiscard]] const Force* getForceOrigin() const
    {
        return force_before_capture.value_or(nullptr).get();
    }

    [[nodiscard]] bool isToSuicide() const
    {
        return to_suicide;
    }

    [[nodiscard]] bool isWarlord() const
    {
        return is_warlord;
    }

    [[nodiscard]] bool isToBoard() const
    {
        return to_board;
    }

private:
    uint8_t property;
    uint8_t avatar;
    wstring alias;
    uint8_t siege_ability;
    uint8_t field_ability;
    uint8_t naval_ability;
    uint8_t battle_ability;
    uint8_t command;
    uint8_t politics;
    CharacterStatus status;
    wstring status_string;
    uint8_t month_to_board;
    OptionalForcePtr force_or_capture;
    OptionalForcePtr force_before_capture;
    OptionalForcePtr force_next;
    uint8_t force_capture_index;
    uint8_t force_before_capture_index;
    uint8_t force_next_index;
    bool to_suicide;
    bool is_warlord;
    bool to_board;
};

class City final : public NamedElement
{
public:
    City(uint index, const Raw::City& raw, const CharacterPtrVector& characters);

    void resolve(const ForcePtrVector& forces)
    {
        if (force_index < forces.size())
        {
            force = forces[force_index];
        }
    }

private:
    uint8_t force_index;
    OptionalForcePtr force;
    wstring name;
    Axis axis;
    uint16_t max_productivity{};
    uint16_t cur_productivity{};
    uint8_t increase{};
    uint8_t anti_disaster{};
    uint8_t soldiers{};
    uint16_t city_type{};
    OptionalCharacterPtr affairs_owner;
};

class Legion final : public NamedElement
{
public:
    Legion(uint index, const Raw::Legion& raw, const ForcePtrVector& forces, const CharacterPtrVector& characters,
           const CityPtrVector& cities);

private:
    uint8_t state;
    ForcePtr force;
    CharacterPtr leader;
    CityPtr target_city;
    uint16_t total_soldier;
    uint8_t morale;
    Axis current_axis;
    Axis target_axis;
    vector<Troop> troops;
};

class Conscription final
{
public:
    uint16_t cavalry;
    uint16_t infantry;
    uint16_t archer;

    Conscription(const uint16_t cavalry, const uint16_t infantry, const uint16_t archer)
        : cavalry(cavalry)
          , infantry(infantry)
          , archer(archer)
    {
    }
};

class GameData
{
public:
    explicit GameData(const Raw::GameData& raw);

    void resolve(const ForcePtrVector& forces)
    {
        if (force_index < forces.size())
        {
            force = forces[force_index];
        }
    }

    [[nodiscard]] uint8_t getDay() const
    {
        return day;
    }

    [[nodiscard]] uint8_t getMonth() const
    {
        return month;
    }

    [[nodiscard]] uint16_t getYear() const
    {
        return year;
    }

    [[nodiscard]] const Force* getForce() const
    {
        return force.value_or(nullptr).get();
    }

    [[nodiscard]] uint8_t getTrust() const
    {
        return trust;
    }

    [[nodiscard]] uint8_t getNumber() const
    {
        return number;
    }

    [[nodiscard]] uint16_t getCurTaxRate() const
    {
        return cur_tax_rate;
    }

    [[nodiscard]] const Conscription& getCurConscription() const
    {
        return cur_conscription;
    }

    [[nodiscard]] uint16_t getNextTaxRate() const
    {
        return next_tax_rate;
    }

    [[nodiscard]] const Conscription& getNextConscription() const
    {
        return next_conscription;
    }

    [[nodiscard]] uint8_t getTotalForces() const
    {
        return total_forces;
    }

    [[nodiscard]] const wstring& getName() const
    {
        return name;
    }

private:
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t force_index;
    OptionalForcePtr force;
    uint8_t trust;
    uint8_t number;
    uint16_t cur_tax_rate;
    Conscription cur_conscription;
    uint16_t next_tax_rate;
    Conscription next_conscription;
    uint8_t total_forces;
    wstring name;
};

class Scenario
{
public:
    explicit Scenario(const Raw::Scenario& raw);

    void resolve()
    {
        game_data.resolve(forces);

        for (const auto& item : cities)
        {
            item->resolve(forces);
        }

        for (const auto& item : characters)
        {
            item->resolve(forces);
        }
    }

    [[nodiscard]] const GameData& getGameData() const
    {
        return game_data;
    }

    [[nodiscard]] const ForcePtrVector& getForces() const
    {
        return forces;
    }

    [[nodiscard]] const CityPtrVector& getCities() const
    {
        return cities;
    }

    [[nodiscard]] const LegionPtrVector& getLegions() const
    {
        return legions;
    }

    [[nodiscard]] const CharacterPtrVector& getCharacters() const
    {
        return characters;
    }

private:
    GameData game_data;
    ForcePtrVector forces;
    CityPtrVector cities;
    LegionPtrVector legions;
    CharacterPtrVector characters;
};

class ScenarioFile
{
public:
    virtual ~ScenarioFile() = default;
    virtual bool loadFile(const fs::path& filepath);

    [[nodiscard]] const fs::path& getPath() const
    {
        return file_path;
    }

    [[nodiscard]] const std::vector<Scenario>& getScenarios() const
    {
        return scenarios;
    }

private:
    fs::path file_path;
    std::vector<Scenario> scenarios;
};

class SavedScenarioFile final : public ScenarioFile
{
public:
    ~SavedScenarioFile() override = default;
    bool loadFile(const fs::path& filepath) override;

private:
    fs::file_time_type timestamp;
};

class DragonGameObject
{
public:
    bool openGameFolder(string& folder_path);
    [[nodiscard]] bool applySavedFile(const SavedScenarioFile& saved_file) const;

    [[nodiscard]] const std::vector<ScenarioFile>& get_scenario_files() const
    {
        return scenario_files;
    }

    [[nodiscard]] const std::vector<SavedScenarioFile>& get_saved_files() const
    {
        return saved_files;
    }

    [[nodiscard]] const SavedScenarioFile& get_default_saved_file() const
    {
        return default_saved_file;
    }

private:
    fs::path gameFolderPath;
    std::vector<ScenarioFile> scenario_files;
    std::vector<SavedScenarioFile> saved_files;
    SavedScenarioFile default_saved_file;
};


std::wostream& operator<<(std::wostream& os, const NamedElement& n);

std::wostream& operator<<(std::wostream& os, const Axis& a);

std::wostream& operator<<(std::wostream& os, const Troop& t);

std::wostream& operator<<(std::wostream& os, const Conscription& c);

std::wostream& operator<<(std::wostream& os, const Character& item);

std::wostream& operator<<(std::wostream& os, const Force& item);

std::wostream& operator<<(std::wostream& os, const City& item);

std::wostream& operator<<(std::wostream& os, const Legion& item);

std::wostream& operator<<(std::wostream& os, const GameData& item);

std::wostream& operator<<(std::wostream& os, const Scenario& item);

std::wostream& operator<<(std::wostream& os, const ScenarioFile& item);

std::wostream& operator<<(std::wostream& os, const SavedScenarioFile& item);

std::wostream& operator<<(std::wostream& os, const DragonGameObject& item);

}
