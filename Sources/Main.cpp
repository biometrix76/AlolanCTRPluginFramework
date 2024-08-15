#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <vector>

#include "Codes.hpp"
#include "PKHeX.hpp"

namespace CTRPluginFramework {
    // This function is called before main and before the game starts. Useful to do code edits safely
    void PatchProcess(FwkSettings &settings) {
        DetectGame(Process::GetTitleID());
        IsUpdateSupported(Process::GetVersion());
    }

    // This function is called when the process exits. Useful to save settings, undo patchs or clean up things
    void OnProcessExit(void);

    void InitMenu(PluginMenu &menu) {
        static const vector<string> name = {
            // Festival Plaza
            "Festival Plaza", // [FOLDER]
            "Current Rank", // <W
            "FC Collected", // <W
            "Current FC", // <W
            "Guests Interacted With", // <W
            "Facilities Hosted", // <W
            "Missions Participated In", // <W

            // Combat
            "Combat", // [FOLDER]
            "In-Battle", // [FOLDER]
            "Party Position:", // <W
            "Condition", // <W
            "Stats", // <W
            "Health & Mana", // <W
            "Item", // <W
            "Moves", // <W
            "Exp. Multiplier", // <W
            "View Pokemon Infos.", // <W
            "Always Crit. Hit", // <W
            "Allow 1+ Megas.", // <W
            "Reuse Z-Crystal", // <W
            "Ignore Z-Crystal Req.", // <W
            "Other", // [FOLDER]
            "No Wild Pokemon", // <W
            "Wild Pokemon Spawner", // <W
            "Always Shiny", // <W
            "[IGNORE]", // Place holder
            "Trainer Rematch", // <W
            "No Trainer Battle", // <TBD
            "Capture Rate 100%", // <W
            "Trial Is Catchable", // <TBD
            "Catch Trainer Pokemon", // <W

            // PKHeX
            "PKHeX", // [FOLDER]
            "Trainer", // [FOLDER]
            "Info", // [FOLDER]
            "TID & SID", // <W
            "Name", // <W
            "Time Played", // <W
            "Game Language", // <W
            "Bag", // [FOLDER]
            "Money", // <W
            "Battle Points", // <W
            "Items", // <W
            "Medicines", // <W
            "Berries", // <W
            "TMs & HMs", // <W
            "[IGNORE]", // Place holder
            "PC Box", // [FOLDER]
            "Export & Import", // <W
            "PKHeX", // [FOLDER]
            "Position", // <W
            "Main", // [FOLDER]
            "Shiny", // <W
            "Species", // <W
            "Is Nicknamed", // <W
            "Nickname", // <W
            "Level", // <W
            "Nature", // <W
            "Gender", // <W
            "Form", // <W
            "Held Item", // <W
            "Ability", // <W
            "Friendship", // <W
            "Language", // <W
            "Is Egg", // <W
            "Pokérus", // <W
            "Country", // <W
            "[IGNORE]", // Place holder
            "3DS Region", // <W
            "Origins", // [FOLDER]
            "Game From", // <W
            "[IGNORE]", // Place holder
            "Ball", // <W
            "Met Level", // <W
            "Met Date", // <W
            "Is Fateful Enc.", // <W
            "[IGNORE]", // Place holder
            "Egg Met Date", // <W
            "Stats", // [FOLDER]
            "IV", // <W
            "EV", // <W
            "Contest", // <W
            "Moves", // [FOLDER]
            "Current Moves", // <W
            "PP Ups", // <W
            "Relearn Moves", // <W
            "Misc.", // [FOLDER]
            "SID", // <W
            "TID", // <W
            "OT Name", // <W
            "Latest (not OT) Handler", // <W
            "[IGNORE]", // Place holder
            "PC Anywhere", // <W
            "31 Boxes Unlocked", // <W

            // Misc.
            "Misc.", // [FOLDER]
            "Unlock Every Mount", // <W
            "Zoomed-out View", // <W
            "Fast Walk & Run", // <W
            "Walk Through Walls", // <W
            "Set Sun & Moon", // <W
            "Other", // [FOLDER]
            "Rename Pokemon", // <W
            "Learn Any TM & HM", // <W
            "Instant D.C Egg", // <W
            "Hatch Egg Immediately", // <W
            "View IVs & EVs in Summary", // <W
            "No Outlines", // <W
            "Super Fast Dialogs", // <W
        };

        MenuFolder *plaza;
        plaza = new MenuFolder(name[0]);
        *plaza += new MenuEntry(name[1], nullptr, CurrentRank);
        *plaza += new MenuEntry(name[2], nullptr, FCCollected);
        *plaza += new MenuEntry(name[3], nullptr, CurrentFC);
        *plaza += new MenuEntry(name[4], nullptr, GuestsInteracted);
        *plaza += new MenuEntry(name[5], nullptr, FacilsHosted);
        *plaza += new MenuEntry(name[6], nullptr, MissionsJoined);
        menu += plaza;

        MenuFolder *combat, *inBattle, *encounter;
        combat = new MenuFolder(name[7]);
        *combat += new MenuEntry(name[9], nullptr, PartyPosition);
        inBattle = new MenuFolder(name[8]);
        *inBattle += new MenuEntry(name[10], nullptr, StatusCondition);
        *inBattle += new MenuEntry(name[11], nullptr, Stats);
        *inBattle += new MenuEntry(name[12], nullptr, HealthAndMana);
        *inBattle += new MenuEntry(name[13], nullptr, HeldItem);
        *inBattle += new MenuEntry(name[14], nullptr, Moves);
        *combat += inBattle;
        *combat += new MenuEntry(name[15], nullptr, ExpMultiplier);
        *combat += HotkeyEntry(new MenuEntry(name[16], nullptr, ViewPokemonInfo, ""), {Key::Start, ""});
        *combat += new MenuEntry(name[17], AlwaysCritHit);
        *combat += new MenuEntry(name[18], AllowMultipleMega);
        *combat += new MenuEntry(name[19], ReuseZCrystal);
        *combat += new MenuEntry(name[20], IgnoreZCrystalReq);
        encounter = new MenuFolder(name[21]);
        *encounter += new MenuEntry(name[22], NoWildPokemon);
        *encounter += new MenuEntry(name[23], nullptr, WildSpawner);
        *encounter += new MenuEntry(name[24], AlwaysShiny);
        // *encounter += new MenuEntry(name[25]);
        *encounter += new MenuEntry(name[26], TrainerRematch);
        *encounter += new MenuEntry(name[27], NoTrainerBattle);
        *encounter += new MenuEntry(name[28], CaptureRate);
        *encounter += new MenuEntry(name[29], TrialIsCatchable);
        *encounter += new MenuEntry(name[30], CatchTrainerPokemon);
        *combat += encounter;
        menu += combat;

        MenuFolder *pkhex, *trainer, *info, *bag, *boxes, *editor;
        pkhex = new MenuFolder(name[31]);
        trainer = new MenuFolder(name[32]);
        info = new MenuFolder(name[33]);
        *info += new MenuEntry(name[34], nullptr, Identity);
        *info += new MenuEntry(name[35], nullptr, InGameName);
        *info += new MenuEntry(name[36], nullptr, PlayTime);
        *info += new MenuEntry(name[37], nullptr, GameLanguage);
        *trainer += info;
        bag = new MenuFolder(name[38]);
        *bag += new MenuEntry(name[39], nullptr, Money);
        *bag += new MenuEntry(name[40], nullptr, BattlePoints);
        *bag += new MenuEntry(name[41], nullptr, Items);
        *bag += new MenuEntry(name[42], nullptr, Medicines);
        *bag += new MenuEntry(name[43], nullptr, Berries);
        *bag += new MenuEntry(name[44], nullptr, KeyItems);
        *trainer += bag;
        // *trainer += new MenuEntry(name[45]);
        *pkhex += trainer;

        boxes = new MenuFolder(name[46]);
        *boxes += new MenuEntry(name[47], nullptr, ExportImport);
        editor = new MenuFolder(name[48]);
        *editor += new MenuEntry(name[49], nullptr, PKHeX::Position);
        *editor += new MenuFolder(name[50], vector<MenuEntry *>({
            new MenuEntry(name[51], nullptr, PKHeX::Shiny),
            new MenuEntry(name[52], nullptr, PKHeX::Species),
            new MenuEntry(name[53], nullptr, PKHeX::IsNicknamed),
            new MenuEntry(name[54], nullptr, PKHeX::Nickname),
            new MenuEntry(name[55], nullptr, PKHeX::Level),
            new MenuEntry(name[56], nullptr, PKHeX::Nature),
            new MenuEntry(name[57], nullptr, PKHeX::Gender),
            new MenuEntry(name[58], nullptr, PKHeX::Form),
            new MenuEntry(name[59], nullptr, PKHeX::HeldItem),
            new MenuEntry(name[60], nullptr, PKHeX::Ability),
            new MenuEntry(name[61], nullptr, PKHeX::Friendship),
            new MenuEntry(name[62], nullptr, PKHeX::Language),
            new MenuEntry(name[63], nullptr, PKHeX::IsEgg),
            new MenuEntry(name[64], nullptr, PKHeX::Pokerus),
            new MenuEntry(name[65], nullptr, PKHeX::Country),
            // new MenuEntry(name[66], nullptr),
            new MenuEntry(name[67], nullptr, PKHeX::ConsoleRegion),
        }));
        *editor += new MenuFolder(name[68], vector<MenuEntry *>({
            new MenuEntry(name[69], nullptr, PKHeX::OriginGame),
            // new MenuEntry(name[70], nullptr),
            new MenuEntry(name[71], nullptr, PKHeX::Ball),
            new MenuEntry(name[72], nullptr, PKHeX::MetLevel),
            new MenuEntry(name[73], nullptr, PKHeX::MetDate),
            new MenuEntry(name[74], nullptr, PKHeX::IsFatefulEncounter),
            // new MenuEntry(name[75], nullptr),
            new MenuEntry(name[76], nullptr, PKHeX::EggMetDate),
        }));
        *editor += new MenuFolder(name[77], vector<MenuEntry *>({
            new MenuEntry(name[78], nullptr, PKHeX::IV),
            new MenuEntry(name[79], nullptr, PKHeX::EV),
            new MenuEntry(name[80], nullptr, PKHeX::Contest),
        }));
        *editor += new MenuFolder(name[81], vector<MenuEntry *>({
            new MenuEntry(name[82], nullptr, PKHeX::CurrentMove),
            new MenuEntry(name[83], nullptr, PKHeX::PPUp),
            new MenuEntry(name[84], nullptr, PKHeX::RelearnMove),
        }));
        *editor += new MenuFolder(name[85], vector<MenuEntry *>({
            new MenuEntry(name[86], nullptr, PKHeX::SID),
            new MenuEntry(name[87], nullptr, PKHeX::TID),
            new MenuEntry(name[88], nullptr, PKHeX::OTName),
            new MenuEntry(name[89], nullptr, PKHeX::LatestHandler),
            // new MenuEntry(name[90], nullptr),
        }));
        *boxes += editor;
        *boxes += new MenuEntry(name[91], PCAnywhere);
        *boxes += new MenuEntry(name[92], nullptr, BoxesUnlocked);
        *pkhex += boxes;
        menu += pkhex;

        MenuFolder *misc, *other;
        misc = new MenuFolder(name[93]);
        *misc += new MenuEntry(name[94], nullptr, UnlockEveryMount);
        *misc += new MenuEntry(name[95], ZoomedOutView);
        *misc += new MenuEntry(name[96], FastWalkRun);
        *misc += new MenuEntry(name[97], WalkThroughWalls);
        *misc += new MenuEntry(name[98], nullptr, SetSunMoon);
        other = new MenuFolder(name[99]);
        *other += new MenuEntry(name[100], RenameAnyPokemon);
        *other += new MenuEntry(name[101], LearnAnyTeachable);
        *other += new MenuEntry(name[102], InstantEgg);
        *other += new MenuEntry(name[103], InstantEggHatch);
        *misc += other;
        *misc += HotkeyEntry(new MenuEntry(name[104], ViewValuesInSummary, ""), {Hotkey(Key::L, ""), Hotkey(Key::Start, "")});
        *misc += new MenuEntry(name[105], NoOutlines);
        *misc += new MenuEntry(name[106], FastDialogs);
        menu += misc;
    }

    int main(void) {
        PluginMenu *menu = new PluginMenu("Alolan CTRPluginFramework", 0, 0, 1, "Alolan CTRPluginFramework is a 3gx plugin for Pokemon Sun, Moon, Ultra Sun, and Ultra Moon on the Nintendo 3DS console. This plugin is more so a continuation of ultraSuMoFramework with PKHeX implementations with an updated .3gx file format.\n\nRepository:\ngithub.com/biometrix76/AlolanCTRPluginFramework");
        // Enable menu synchronization with the game's frame rate
        menu->SynchronizeWithFrame(true);
        // Pause the execution for 100 milliseconds to ensure the menu is properly initialized
        Sleep(Milliseconds(100));

        // Check if the current game is recognized
        if (currGameName == GameName::None || currGameSet == GameSet::None)
            Process::ReturnToHomeMenu(); // If not recognized, return to the home menu of the console

        // Initialize the menu with entries and folders
        InitMenu(*menu);
        // Set the battle offset callback for the menu
        menu->Callback(SetBattleOffset);
        // Run the menu, allowing user interaction
        menu->Run();
        // After the menu is closed, delete the menu object to free up memory
        delete menu;
        // Return 0 to indicate successful execution
        return (0);
    }
}