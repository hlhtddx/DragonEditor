// File: `src/DragonData_gtest.cpp`
#include "DragonData.h"
#include <fstream>
#include <sstream>

using namespace DragonData;
namespace fs = std::filesystem;

void Test01()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-01.DAT");
    const auto& scenarios = file.getScenarios();

    wstringstream ss;
    ss << file;
    auto out_str = ss.str();

    auto out_path = fs::current_path()  / "outputs" / "s1.log";
    wofstream of(out_path, ios_base::out | ios_base::trunc);
    of << "Loaded scenarios:" << out_str;
    of << out_str;
    of.close();

    // const auto& scenario_0 = scenarios[0];
    // EXPECT_EQ(scenario_0.getGameData().getName(), L"群雄逐鹿");
    // const auto& scenario_1 = scenarios[1];
    // EXPECT_EQ(scenario_0.getGameData().getName(), L"群雄逐鹿");
    // const auto& scenario_2 = scenarios[2];
    // EXPECT_EQ(scenario_0.getGameData().getName(), L"群雄逐鹿");
    // const auto& scenario_3 = scenarios[3];
    // EXPECT_EQ(scenario_0.getGameData().getName(), L"群雄逐鹿");
}

void Test02()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-02.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "s2.log";
    wofstream of(out_path, std::ios::out | std::ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

void Test03()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-03.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "s3.log";
    wofstream of(out_path, std::ios::out | std::ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

void Test04()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-04.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "s4.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

void Test05()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-05.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "s5.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

void Test06()
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-06.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "s6.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

void Test00()
{
    auto game_folder = std::getenv("GAME_FOLDER_PATH");
    auto data_path = fs::path(game_folder);
    SavedScenarioFile file;
    bool result = file.loadFile(data_path / "SAVE.DAT");
    const auto& scenarios = file.getScenarios();

    auto out_path = data_path / ".." / "outputs" / "ss.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

int main()
{
    Test01();
    Test02();
    Test03();
    Test04();
    Test05();
    Test06();
    Test00();
    return 0;
}
