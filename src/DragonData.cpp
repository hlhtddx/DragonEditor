#include "DragonData.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <boost/locale.hpp>

using namespace std;
namespace conv = boost::locale::conv;

namespace DragonData
{
wstring name_to_utf8(const char* s, const size_t len)
{
    if (s == nullptr || len == 0) return {L"Empty"};

    try
    {
        string in(s, len);
        while (!in.empty() && in.back() == '\0') in.pop_back();

        wstring out = conv::to_utf<wchar_t>(in, "BIG5");
        out.erase(
            find_if(out.rbegin(), out.rend(), [](const wchar_t ch) { return ch != L'\0' && ch != L'\u3000'; }).base(),
            out.end());
        return out;
    }
    catch (const conv::conversion_error&)
    {
        return {L"Wrong"};
    }
    catch (...)
    {
        return {L"Error"};
    }
}

Character::Character(const uint index, const Raw::Character& raw)
    : NamedElement(index, raw.name.name, std::size(raw.name.name))
      , status(CharacterStatusFromRaw(raw.status))
{
    property = raw.property;
    avatar = raw.avatar;
    alias = name_to_utf8(raw.alias.name, std::size(raw.alias.name));
    siege_ability = raw.siege_ability;
    field_ability = raw.field_ability;
    naval_ability = raw.naval_ability;
    battle_ability = raw.battle_ability;
    command = raw.command;
    politics = raw.politics;
    month_to_board = raw.month_to_board;

    force_next_index = raw.force_next;
    force_capture_index = raw.force_or_capture;
    force_before_capture_index = raw.force_origin;

    force_next = nullopt;
    force_or_capture = nullopt;
    force_before_capture = nullopt;

    to_board = false;
    is_warlord = false;
    to_suicide = false;
}

Force::Force(uint index, const Raw::Force& raw, const CharacterPtrVector& characters, const CityPtrVector& cities)
    : NamedElement(index)
{
    status = raw.status;
    warlord = characters[raw.warlord];
    name = warlord->getName();

    if (raw.advisor == 0x7f)
    {
        advisor = nullopt;
    }
    else
    {
        advisor = make_optional(characters[raw.advisor]);
    }

    capital = cities[raw.capital];

    cavalries = raw.cavalries;
    infantries = raw.infantries;
    archers = raw.archers;
    subordinates = raw.subordinates;
    money = static_cast<int32_t>(
        static_cast<uint32_t>(raw.money[0]) |
        static_cast<uint32_t>(raw.money[1]) << 8 |
        static_cast<uint32_t>(raw.money[2]) << 16
    );
    city_count = raw.city_count;

    if (raw.diplomacy_owner == 0)
    {
        diplomacy_owner = nullopt;
    }
    else
    {
        diplomacy_owner = characters[raw.diplomacy_owner];
    }
}

City::City(uint index, const Raw::City& raw, const CharacterPtrVector& characters)
    : NamedElement(index, raw.name.name, std::size(raw.name.name))
      , axis(raw.axis)
{
    force_index = raw.force;
    force = nullopt;

    max_productivity = raw.max_productivity;
    cur_productivity = raw.cur_productivity;
    increase = raw.increase;
    anti_disaster = raw.anti_disaster;
    soldiers = raw.soldiers;
    city_type = raw.city_type;

    if (raw.affairs_owner == 0)
    {
        affairs_owner = nullopt;
    }
    else
    {
        affairs_owner = characters[raw.affairs_owner];
    }
}

Legion::Legion(const uint index, const Raw::Legion& raw, const ForcePtrVector& forces,
               const CharacterPtrVector& characters,
               const CityPtrVector& cities)
    : NamedElement(index)
      , current_axis(raw.current_axis)
      , target_axis(raw.target_axis)
{
    state = raw.state;
    force = forces[raw.force];
    leader = characters[raw.leader];
    name = leader->getName();
    target_city = cities[raw.target_city];

    total_soldier = raw.total_soldier;
    morale = raw.morale;

    for (const auto& troop : raw.troops)
    {
        troops.emplace_back(troop);
    }
}

GameData::GameData(const Raw::GameData& raw)
    : cur_conscription(Conscription(raw.cur_conscription[0], raw.cur_conscription[1], raw.cur_conscription[2]))
      , next_conscription(Conscription(raw.next_conscription[0], raw.next_conscription[1], raw.next_conscription[2]))
{
    day = raw.day;
    month = raw.month;
    year = raw.year;

    force_index = raw.force;
    force = nullopt;

    trust = raw.trust;
    number = raw.number;

    cur_tax_rate = raw.cur_tax_rate;
    next_tax_rate = raw.next_tax_rate;

    total_forces = raw.total_forces;
    name = name_to_utf8(raw.name, std::size(raw.name));
}

Scenario::Scenario(const Raw::Scenario& raw)
    : game_data(raw.game_data)
{
    // characters
    for (size_t i = 0; i < std::size(raw.characters); ++i)
    {
        const auto& rawItem = raw.characters[i];
        if (rawItem.name.name[0] == 0) continue;
        auto item = make_shared<Character>(i, rawItem);
        characters.push_back(item);
    }

    // cities
    for (size_t i = 0; i < std::size(raw.cities); ++i)
    {
        const Raw::City& rawItem = raw.cities[i];
        if (rawItem.axis.x == 0 || rawItem.axis.y == 0) continue;
        cities.emplace_back(make_shared<City>(i, rawItem, characters));
    }

    // forces
    for (size_t i = 0; i < std::size(raw.forces); ++i)
    {
        const auto& rawItem = raw.forces[i];
        if (rawItem.status == 0) continue;
        forces.emplace_back(make_shared<Force>(i, rawItem, characters, cities));
    }

    // legions
    for (size_t i = 0; i < std::size(raw.legions); ++i)
    {
        const auto& rawItem = raw.legions[i];
        if (rawItem.current_axis.x == 0 || rawItem.current_axis.y == 0) continue;
        legions.emplace_back(make_shared<Legion>(i, rawItem, forces, characters, cities));
    }

    resolve();
}

constexpr auto FILE_SIZE = Raw::SCENARIO_DATA_SIZE * Raw::SCENARIO_COUNT;

bool ScenarioFile::loadFile(const fs::path& filepath)
{
    file_path = filepath;
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) throw std::runtime_error("open scenario file failed: " + filepath.string());

    std::array<char, FILE_SIZE> data{};
    ifs.read(data.data(), FILE_SIZE);

    std::vector<Scenario> result;
    const auto buffer = reinterpret_cast<const char*>(data.data());
    const auto* rawFile = reinterpret_cast<const Raw::File*>(buffer);
    try
    {
        for (const auto& scenario : rawFile->scenarios)
        {
            scenarios.emplace_back(scenario);
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool SavedScenarioFile::loadFile(const fs::path& filepath)
{
    if (!ScenarioFile::loadFile(filepath)) return false;
    timestamp = fs::last_write_time(filepath);
    return true;
}

bool DragonGameObject::openGameFolder(string& folder_path)
{
    fs::path root(folder_path);
    fs::path scenario_dir = root / "SINARIO";
    fs::path saved_dir = root / "SAVES";
    fs::path save_data_path = root / "SAVE.DAT";

    if (!fs::exists(scenario_dir) || !fs::is_directory(scenario_dir))
    {
        return false;
    }

    try
    {
        for (const auto& entry : fs::directory_iterator(scenario_dir))
        {
            if (!entry.is_regular_file()) continue;
            const auto& file_path = entry.path();
            if (auto ext = file_path.extension().string(); !(ext == ".dat" || ext == ".DAT")) continue;

            ScenarioFile item;
            if (!item.loadFile(file_path)) continue;
            scenario_files.emplace_back(std::move(item));
        }

        for (const auto& entry : fs::directory_iterator(saved_dir))
        {
            if (!entry.is_regular_file()) continue;
            const auto& file_path = entry.path();
            if (auto ext = file_path.extension().string(); !(ext == ".dat" || ext == ".DAT")) continue;

            SavedScenarioFile item;
            if (!item.loadFile(file_path)) continue;
            saved_files.emplace_back(std::move(item));
        }

        if (fs::exists(save_data_path) && fs::is_regular_file(save_data_path))
        {
            default_saved_file.loadFile(save_data_path);
        }
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "Failed to scan scenario directory: " << e.what() << endl;
        return false;
    }

    gameFolderPath = folder_path;
    return true;
}

bool DragonGameObject::applySavedFile(const SavedScenarioFile& saved_file) const
{
    const auto target_path = gameFolderPath / "SAVE.DAT";
    try
    {
        fs::copy_file(saved_file.getPath(), target_path, fs::copy_options::overwrite_existing);
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "Failed to apply saved file: " << e.what() << endl;
        return false;
    }
    return true;
}

// NamedElement（包含 name）
std::wostream& operator<<(std::wostream& os, const NamedElement& n)
{
    os << "NamedElement{name=" << n.getName() << "}"
        << endl;
    return os;
}

// Axis（x, y）
std::wostream& operator<<(std::wostream& os, const Axis& a)
{
    os << "Axis{x=" << a.x << ", y=" << a.y << "}"
        << endl;
    return os;
}

// Troop（count, type）
std::wostream& operator<<(std::wostream& os, const Troop& t)
{
    os << "Troop{count=" << t.getCount() << ", type=" << t.getTroopType() << "}"
        << endl;
    return os;
}

// Conscription（cavalry, infantry, archer）
std::wostream& operator<<(std::wostream& os, const Conscription& c)
{
    os << "Conscription{cavalry=" << c.cavalry
        << ", infantry=" << c.infantry
        << ", archer=" << c.archer << "}"
        << endl;
    return os;
}

// Character（摘要）
std::wostream& operator<<(std::wostream& os, const Character& item)
{
    os << "Character{name=" << item.getName()
        << ", alias=" << item.getAlias()
        << ", status=" << static_cast<int>(item.getStatus())
        << ", status_string=" << item.getStatusString()
        << ", month_to_board=" << static_cast<int>(item.getMonthToBoard())
        << ", to_suicide=" << (item.isToSuicide() ? "true" : "false")
        << ", is_warlord=" << (item.isWarlord() ? "true" : "false")
        << ", to_board=" << (item.isToBoard() ? "true" : "false")
        << "}"
        << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const Force& item)
{
    // diplomacy owner
    const Character* dip = item.getDiplomacyOwner();
    const Character* war = item.getWarlord();
    const Character* adv = item.getAdvisor();
    const City* cap = item.getCapital();

    os << "Force{name=" << item.getName()
        << ", status=" << static_cast<int>(item.getStatus())
        << ", warlord=" << (war ? war->getName() : L"<none>")
        << ", advisor=" << (adv ? adv->getName() : L"<none>")
        << ", capital=" << (cap ? cap->getName() : L"<none>")
        << ", cavalries=" << item.getCavalries()
        << ", infantries=" << item.getInfantries()
        << ", archers=" << item.getArchers()
        << ", subordinates=" << static_cast<int>(item.getSubordinates())
        << ", money=" << item.getMoney()
        << ", cities=" << static_cast<int>(item.getCities())
        << ", diplomacy_owner=" << (dip ? dip->getName() : L"<none>")
        << "}"
        << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const City& item)
{
    os << "City{name=" << item.getName() << "}"
        << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const Legion& item)
{
    os << "Legion{name=" << item.getName() << "}"
        << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const GameData& item)
{
    const Force* f = item.getForce();
    os << "GameData{name=" << item.getName()
        << ", date=" << item.getDay() << "-" << static_cast<int>(item.getMonth()) << "-" << item.getYear()
        << ", force=" << (f ? f->getName() : L"<none>")
        << ", trust=" << static_cast<int>(item.getTrust())
        << ", number=" << static_cast<int>(item.getNumber())
        << ", cur_tax=" << item.getCurTaxRate()
        << ", cur_conscription=" << item.getCurConscription()
        << ", next_tax=" << item.getNextTaxRate()
        << ", next_conscription=" << item.getNextConscription()
        << ", total_forces=" << static_cast<int>(item.getTotalForces())
        << "}"
        << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const Scenario& item)
{
    os << "Scenario{"
        << "game_data=" << item.getGameData()
        << ", forces_count=" << item.getForces().size()
        << ", cities_count=" << item.getCities().size()
        << ", legions_count=" << item.getLegions().size()
        << ", characters_count=" << item.getCharacters().size() << endl;

    for (const auto& force : item.getForces())
    {
        os << *force;
    }

    for (const auto& city : item.getCities())
    {
        os << *city;
    }

    for (const auto& leg : item.getLegions())
    {
        os << *leg;
    }

    for (const auto& character : item.getCharacters())
    {
        os << *character;
    }

    os << "}" << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const ScenarioFile& item)
{
    os << "ScenarioFile{path=" << item.getPath().wstring() << ", scenarios=" << item.getScenarios().size() << endl;
    for (const auto& scenario : item.getScenarios())
    {
        os << scenario;
    }
    os << "}" << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const SavedScenarioFile& item)
{
    os << "SavedFile{path=" << item.getPath().wstring() << "}" << endl;
    for (const auto& scenario : item.getScenarios())
    {
        os << scenario;
    }
    os << "}" << endl;
    return os;
}

std::wostream& operator<<(std::wostream& os, const DragonGameObject& item)
{
    os << "DragonGameObject{"
        << "gameFolderPath=" << (item.get_scenario_files().empty()
                                     ? L"<unknown>"
                                     : item.get_scenario_files().front().getPath().parent_path().wstring())
        << ", scenario_files=" << item.get_scenario_files().size()
        << ", saved_files=" << item.get_saved_files().size()
        << "}"
        << endl;
    return os;
}
}
