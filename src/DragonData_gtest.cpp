// File: `src/DragonData_gtest.cpp`
#include "DragonData.h"
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

using namespace DragonData;
namespace fs = std::filesystem;


TEST(DragonData, CompileAndLink)
{
    (void)sizeof(DragonGameObject);
    SUCCEED();
}

TEST(DragonData, LoadScenario_01)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-01.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    wstringstream ss;
    ss << file;
    auto out_str = ss.str();
    SCOPED_TRACE(out_str);

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

TEST(DragonData, LoadScenario_02)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-02.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "s2.log";
    wofstream of(out_path, std::ios::out | std::ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

TEST(DragonData, LoadScenario_03)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-03.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "s3.log";
    wofstream of(out_path, std::ios::out | std::ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

TEST(DragonData, LoadScenario_04)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-04.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "s4.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

TEST(DragonData, LoadScenario_05)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-05.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "s5.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

TEST(DragonData, LoadScenario_06)
{
    auto data_path = fs::current_path() / "tests";
    ScenarioFile file;
    bool result = file.loadFile(data_path / "SINARIO-06.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "s6.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}

TEST(DragonData, LoadSaveFile)
{
    auto game_folder = std::getenv("GAME_FOLDER_PATH");
    auto data_path = fs::path(game_folder);
    SavedScenarioFile file;
    bool result = file.loadFile(data_path / "SAVE.DAT");
    EXPECT_TRUE(result);
    const auto& scenarios = file.getScenarios();
    EXPECT_EQ(scenarios.size(), 4);

    auto out_path = data_path / ".." / "outputs" / "ss.log";
    wofstream of(out_path, ios::trunc);
    of << "Loaded scenarios:" << std::endl;
    of << file;
    of.close();
}
