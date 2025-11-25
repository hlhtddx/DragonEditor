#include "DragonGame.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <boost/locale.hpp>

using namespace std;
namespace conv = boost::locale::conv;

namespace DragonData
{
wstring name_to_utf8(const char* s, size_t len)
{
    if (s == nullptr || len == 0) return {L"Empty"};

    try
    {
        string in(s, len);
        while (!in.empty() && in.back() == '\0') in.pop_back();

        wstring out = conv::to_utf<wchar_t>(in, "BIG5");
        out.erase(find_if(out.rbegin(), out.rend(), [](wchar_t ch) { return ch != L'\0' && ch != L'\u3000'; }).base(),
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

Character::Character(uint index, const Raw::Character& raw)
    : status(CharacterStatus(raw.status))
      , NamedElement(index, raw.name.name, std::size(raw.name.name))
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
    status = CharacterStatus(raw.status);
    month_to_board = raw.month_to_board;

    force_next_index = raw.force_next;
    force_capture_index = raw.force_or_capture;
    force_origin_index = raw.force_origin;

    force_next = nullopt;
    force_capture = nullopt;
    force_origin = nullopt;

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
        (static_cast<uint32_t>(raw.money[0])) |
        (static_cast<uint32_t>(raw.money[1]) << 8) |
        (static_cast<uint32_t>(raw.money[2]) << 16)
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

Legion::Legion(uint index, const Raw::Legion& raw, const ForcePtrVector& forces, const CharacterPtrVector& characters,
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
        characters.emplace_back(make_shared<Character>(i, rawItem));
    }

    // cities
    for (size_t i = 0; i < std::size(raw.cities); ++i)
    {
        const Raw::City& rawItem = raw.cities[i];
        if (rawItem.axis.x > 0 && rawItem.axis.y > 0) continue;
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
        if (rawItem.current_axis.x > 0 && rawItem.current_axis.y > 0) continue;
        legions.emplace_back(make_shared<Legion>(i, rawItem, forces, characters, cities));
    }

    resolve();
}

void Scenario::resolve()
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

std::vector<Scenario> ScenarioFile::load_file(const fs::path& filepath)
{
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) throw std::runtime_error("open scenario file failed: " + filepath.string());

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>()
    );

    std::vector<Scenario> result;
    for (size_t i = 0; i < SCENARIO_COUNT; ++i)
    {
        size_t offset = i * SCENARIO_DATA_SIZE;
        if (offset + SCENARIO_DATA_SIZE > data.size()) break;
        result.push_back(Scenario::from_bytes(data, offset));
    }
    return result;
}

ScenarioFile ScenarioFile::create(
    const std::string& name,
    const std::string& description,
    const fs::path& scenario_path,
    const fs::path& saved_path
)
{
    ScenarioFile sf;
    sf.name = name;
    sf.description = description;

    sf.scenarios = load_file(scenario_path);
    try
    {
        sf.saved = load_file(saved_path);
    }
    catch (...)
    {
        sf.saved.clear();
    }
    return sf;
}

struct GameObject
{
    fs::path filepath;
    std::vector<ScenarioInfo> file_info;
    std::vector<ScenarioFile> files;

    static GameObject create(const fs::path& root)
    {
        GameObject g;
        g.filepath = root;

        fs::path scenario_dir = root / "SINARIO";
        fs::path scenario_json = scenario_dir / "scenarios.json";

        std::ifstream ifs(scenario_json);
        if (!ifs)
        {
            throw std::runtime_error("open scenarios.json failed");
        }
        json j;
        ifs >> j;
        g.file_info = j.get<std::vector<ScenarioInfo>>();

        fs::path saved_dir = scenario_dir / "SAVE";

        for (const auto& info : g.file_info)
        {
            fs::path scenario_file_path = scenario_dir / info.filename;
            fs::path saved_file_path = saved_dir / (info.name + ".DAT");
            ScenarioFile sf = ScenarioFile::create(
                info.name, info.description,
                scenario_file_path, saved_file_path
            );
            g.files.push_back(std::move(sf));
        }

        return g;
    }

    [[nodiscard]] const std::vector<ScenarioFile>& get_all_scenario_files() const
    {
        return files;
    }

    [[nodiscard]] const Scenario* get_scenarios_file(size_t file_index, size_t slot_index, bool load_save_data) const
    {
        if (file_index >= files.size()) return nullptr;
        const ScenarioFile& sf = files[file_index];
        const std::vector<Scenario>& vec = load_save_data ? sf.saved : sf.scenarios;
        if (slot_index >= vec.size()) return nullptr;
        return &vec[slot_index];
    }

    void prepare_game(size_t file_index) const
    {
        if (file_index >= file_info.size())
        {
            throw std::runtime_error("Scenario file info not found");
        }
        const auto& info = file_info[file_index];

        fs::path scenario_dir = filepath / "SINARIO";
        fs::path saved_dir = scenario_dir / "SAVE";

        fs::path scenario_path = scenario_dir / info.filename;
        fs::path saved_path = saved_dir / (info.name + ".DAT");

        fs::path dest_scenario = filepath / "SINARIO.DAT";
        fs::copy_file(scenario_path, dest_scenario, fs::copy_options::overwrite_existing);

        fs::path dest_saved = filepath / "SAVE.DAT";
        if (fs::exists(saved_path))
        {
            fs::copy_file(saved_path, dest_saved, fs::copy_options::overwrite_existing);
        }
    }

    void save_scenario(size_t file_index) const
    {
        if (file_index >= file_info.size())
        {
            throw std::runtime_error("Scenario file info not found");
        }
        const auto& info = file_info[file_index];
        fs::path saved_dir = filepath / "SINARIO" / "SAVE";
        fs::path saved_path = saved_dir / (info.name + ".DAT");
        fs::path src_saved = filepath / "SAVE.DAT";
        fs::copy_file(src_saved, saved_path, fs::copy_options::overwrite_existing);
    }

    std::vector<uint8_t> dump_scenarios_to_json() const
    {
        json j = files; // 需要给 ScenarioFile/Scenario 等添加 to_json/from_json
        std::string s = j.dump();
        return std::vector<uint8_t>(s.begin(), s.end());
    }
};

DragonGame::DragonGame(string& gameFolderPath)
{
}

bool DragonGame::OpenGameFolder()
{
    return true;
}
}
