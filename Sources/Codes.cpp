#include <optional>
#include "Codes.hpp"
#include "PKHeX.hpp"

namespace CTRPluginFramework {
    void CurrentRank(MenuEntry *entry) {
        u16 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u16>::Set(entry->Name() + ":", true, false, 3, value, 0, 1, 999, Callback<u16>)) {
            Process::Write16(AutoGameSet(0x33124D8A, 0x3306119A), value);
            OSD::Notify("Current Rank: " + to_string(value));
        }
    }

    void FCCollected(MenuEntry *entry) {
        u32 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u32>::Set(entry->Name() + ":", true, false, 7, value, 0, 0, 9999999, Callback<u32>)) {
            Process::Write32(AutoGameSet(0x33124D5C, 0x3306116C), value);
            OSD::Notify("FC Collected: " + to_string(value));
        }
    }

    void CurrentFC(MenuEntry *entry) {
        u32 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u32>::Set(entry->Name() + ":", true, false, 7, value, 0, 0, 9999999, Callback<u32>)) {
            Process::Write32(AutoGameSet(0x33124D58, 0x33061168), value);
            OSD::Notify("Current FC: " + to_string(value));
        }
    }

    void GuestsInteracted(MenuEntry *entry) {
        u32 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u32>::Set(entry->Name() + ":", true, false, 7, value, 0, 0, 9999999, Callback<u32>)) {
            Process::Write32(AutoGameSet(0x3313D740, 0x33079B50), value);
            OSD::Notify("Guests Interacted With: " + to_string(value));
        }
    }

    void FacilsHosted(MenuEntry *entry) {
        u16 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u16>::Set(entry->Name() + ":", true, false, 4, value, 0, 1, 9999, Callback<u16>)) {
            Process::Write16(AutoGameSet(0x3313D802, 0x33079C12), value);
            OSD::Notify("Facilities Hosted: " + to_string(value));
        }
    }

    void MissionsJoined(MenuEntry *entry) {
        u16 value;

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u16>::Set(entry->Name() + ":", true, false, 4, value, 0, 0, 9999, Callback<u16>)) {
            Process::Write16(AutoGameSet(0x3313D800, 0x33079C10), value);
            OSD::Notify("Missions Participated in: " + to_string(value));
        }
    }

    // Vector holding pointers to battle offsets
    static vector<u32> pointer(2);
    // Flag indicating whether to revert changes
    static bool isRevert = false;
    static int validPosition; // How many valid positions

    vector<string> GetBattleParty(vector<string> party) {
        // Lambda function to handle reading from memory
        auto readData = [&](int index) -> u32 {
            u32 offset = battleOffset[0] + (index * sizeof(u32));
            return Process::Read32(offset);
        };

        // Lambda function to handle species validity check
        auto isSpeciesValid = [&](int species) -> bool {
            return species > 0 && species <= (currGameSet == GameSet::Base ? 802 : 807);
        };

        // Lambda function to handle party member assignment
        auto assignParty = [&](int index, u32 pokemonLocation) {
            int species = Process::Read16(pokemonLocation + 0xC);

            if (isSpeciesValid(species)) {
                party[index] = speciesList[species - 1];
                ++validPosition;
            }

            else party[index] = "[Empty]";
        };

        // Lambda function to handle empty positions
        auto setEmptySlot = [&](int index) {
            party[index] = "[Empty]";
        };

        // Initialize validPosition counter
        validPosition = 0;

        // Process each party member using a traditional for loop
        for (int index = 0; index < party.size(); ++index) {
            u32 pokemonLocation = readData(index);
            if (pokemonLocation != 0)
                assignParty(index, pokemonLocation);

            else setEmptySlot(index);
        }

        return party;
    }

    static u8 position;

    void PartyPosition(MenuEntry *entry) {
        vector<string> party(6); // Initialize vector to hold party

        // Lambda to generate prompt text for party positions
        auto generatePromptText = [&](const vector<string> &party) -> string {
            string promptText = entry->Name() << Color::White <<
                "\n\n1: " + party[0] <<
                Color::White << "\n2: " + party[1] <<
                Color::White << "\n3: " + party[2] <<
                Color::White << "\n4: " + party[3] <<
                Color::White << "\n5: " + party[4] <<
                Color::White << "\n6: " + party[5] << "\n";
            return promptText;
        };

        // Lambda to update pointer offsets based on the selected position
        auto updatePointerOffsets = [&](u8 position) {
            for (int index = 0; index < pointer.size(); ++index)
                pointer[index] = battleOffset[index] + ((position - 1) * sizeof(u32));
        };

        // Check if the player is in battle
        if (IfInBattle()) {
            party = GetBattleParty(party); // Get battle party
            // Generate the prompt text for user selection
            string promptText = generatePromptText(party);

            // Handle keyboard input and update the selected position if valid
            if (KeyboardHandler<u8>::Set(promptText, true, false, 1, position, 0, 1, validPosition, Callback<u8>)) {
                updatePointerOffsets(position); // Adjust pointer offsets based on selection

                // Update the menu entry with the newly set position
                entry->Name() = "Position: " << Color::Gray << Utils::ToString(position, 0);
                OSD::Notify("Selected in-battle position: " + party[position - 1]);
            }
        }
    }

    static int statusFlag, selectedCondition;

    void StatusCondition(MenuEntry *entry) {
        static const vector<pair<string, int>> statusConditions = {{"Paralyzed", 0x28}, {"Asleep", 0x30}, {"Frozen", 0x38}, {"Burned", 0x40}, {"Poisoned", 0x48}};
        static const vector<string> statusChoice = {"None", "Affected"};
        vector<string> options;
        Keyboard keyboard;

        // Fill options with the names of each condition
        for (const auto &condition : statusConditions)
            options.push_back(condition.first);

        // Check if in battle and pointers are valid
        if (IfInBattle() && all_of(pointer.begin(), pointer.end(), [](int ptr) {return ptr != 0;})) {
            int userSelection = keyboard.Setup(entry->Name() + ":", true, statusChoice, statusFlag);

            if (userSelection != -1) {
                if (statusFlag == 0) {
                    // Clear any existing conditions
                    for (int ptr : pointer)
                        for (const auto &condition : statusConditions)
                            Process::Write8(ptr, condition.second, 0);

                    OSD::Notify("Status conditions removed");
                }

                else if (statusFlag == 1) {
                    // Apply the chosen condition
                    int conditionSelection = keyboard.Setup(entry->Name() + ":", true, options, selectedCondition);

                    if (conditionSelection != -1) {
                        for (int ptr : pointer) {
                            for (const auto &condition : statusConditions)
                                Process::Write8(ptr, condition.second, 0);

                            Process::Write8(ptr, statusConditions[selectedCondition].second, 1);
                        }

                        OSD::Notify("Applied status condition: " + statusConditions[selectedCondition].first);
                    }
                }
            }
        }
    }

    int statChoice, mainStat, statBoost;
    // Array to store main stat values for 5 different stats
    static u16 mainStatVal[5];
    // Array to store boosted stat values for 7 different stats
    static u8 statBoostVal[7];

    void Stats(MenuEntry *entry) {
        static const vector<string> statOptions = {"Base", "Boosts"};
        // Options for base stats
        static const vector<string> mainStats = {"Attack", "Defense", "Sp. Atk", "Sp. Def", "Speed"};
        // Options for boost stats: initialize with mainStats
        static const vector<string> statBoosts = []() {
            vector<string> combinedStats = mainStats;
            combinedStats.push_back("Accuracy");
            combinedStats.push_back("Evasiveness");
            return combinedStats;
        }();

        // Get user inputs
        u16 value16;
        u8 value8;

        // Check if in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            Keyboard keyboard;

            // While keyboard is not aborted()
            while (keyboard.Setup(entry->Name() + ":", true, statOptions, statChoice) != -1) {
                if (statChoice == 0) {
                    // While keyboard is not aborted()
                    while (keyboard.Setup(entry->Name() + ":", true, mainStats, mainStat) != -1) {
                        // Get and validate base stat value
                        if (KeyboardHandler<u16>::Set(mainStats[mainStat] + ":", true, false, 3, value16, 0, 1, 999, Callback<u16>)) {
                            mainStatVal[mainStat] = value16; // Store base stat value
                            entry->SetGameFunc(UpdateStats);
                            OSD::Notify("Updated " << mainStats[mainStat] << " to: " << to_string(value16));
                            break; // Exit loop after successful input
                        }
                    }
                }

                else if (statChoice == 1) {
                    // While keyboard is not aborted()
                    while (keyboard.Setup(entry->Name() + ":", true, statBoosts, statBoost) != -1) {
                        // Get and validate boost stat value
                        if (KeyboardHandler<u8>::Set(statBoosts[statBoost] + ":", true, false, 1, value8, 0, 0, 6, Callback<u8>)) {
                            statBoostVal[statBoost] = value8 + 6; // Adjust and store boost stat value
                            entry->SetGameFunc(UpdateStats);
                            OSD::Notify("Boosted " << statBoosts[statBoost] << " by: x" << to_string(value8));
                            break; // Exit loop after successful input
                        }
                    }
                }
            }
        }
    }

    void UpdateStats(MenuEntry *entry) {
        // Check if in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            // Loop through all pointers
            for (int i = 0; i < pointer.size(); i++) {
                for (int k = 0; k < sizeof(mainStatVal) / sizeof(mainStatVal[0]); k++)
                    if (mainStatVal[k] != 0)
                        Process::Write16(pointer[i], 0x1DA + (k * 2), mainStatVal[k]);

                for (int l = 0; l < sizeof(statBoostVal) / sizeof(statBoostVal[0]); l++)
                    if (statBoostVal[l] != 0)
                        Process::Write8(pointer[i], 0x1EA + (l * 1), statBoostVal[l]);
            }
        }
    }

    static u16 health;
    static u8 mana;

    void HealthAndMana(MenuEntry *entry) {
        static const vector<string> options = {"Health", "Mana"};
        static int choice;

        // Get user inputs
        u16 tempHealth;
        u8 tempMana;

        // Check if the player is in battle and if the pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            Keyboard keyboard;

            // While keyboard is not aborted()
            while (keyboard.Setup(entry->Name() + ":", true, options, choice) != -1) {
                if (choice == 0) {
                    // Prompt the user for Health input and update the health variable if valid
                    if (KeyboardHandler<u16>::Set("Health:", true, false, 3, tempHealth, 0, 1, 999, Callback<u16>)) {
                        health = tempHealth;
                        entry->SetGameFunc(UpdateHealthAndMana);
                        OSD::Notify("Health set to: " << to_string(tempHealth));
                        break; // Exit the loop after
                    }
                }

                else if (choice == 1) {
                    // Prompt the user for Mana input and update the mana variable if valid
                    if (KeyboardHandler<u8>::Set("Mana:", true, false, 2, tempMana, 0, 1, 99, Callback<u8>)) {
                        mana = tempMana;
                        entry->SetGameFunc(UpdateHealthAndMana);
                        OSD::Notify("PP set to: " << to_string(tempMana));
                        break; // Exit the loop after
                    }
                }
            }
        }
    }

    void UpdateHealthAndMana(MenuEntry *entry) {
        static const int healthPosition = 2, manaPosition = 4; // Number of positions to update for Health and Mana

        // Ensure the player is in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            // Iterate through each pointer
            for (const auto &ptr : pointer) {
                if (health != 0) {
                    for (int pos = 0; pos < healthPosition; ++pos) {
                        const u32 healthOffset = 0x10 - (pos * 2);
                        Process::Write16(ptr, healthOffset, health);
                    }
                }

                if (mana != 0) {
                    for (int pos = 0; pos < manaPosition; ++pos) {
                        const u32 manaOffset = 0x204 + (pos * 0xE);
                        const u16 manaValue = (mana << 8) | mana;
                        Process::Write16(ptr, manaOffset, manaValue);
                    }
                }
            }
        }
    }

    static u16 item;

    void HeldItem(MenuEntry *entry) {
        // Ensure the player is in a battle and pointers are valid
        if (!IfInBattle() || pointer[0] == 0 || pointer[1] == 0)
            return;

        // Prompt the user to search for the desired held item
        SearchForItem(entry);

        // If a valid item is selected, apply it to all relevant pointers
        if (heldItemName > 0) {
            for (const auto &ptr : pointer)
                Process::Write16(ptr, 0x12, heldItemName);

            OSD::Notify("Item changed: " + string(heldItemList[heldItemName - 1]));
        }
    }

    static u16 move;
    static int moveSlot; // 4 slots available, variable used to determine which slot to write to

    void Moves(MenuEntry *entry) {
        const vector<string> options = {"Move 1", "Move 2", "Move 3", "Move 4"};
        Keyboard keyboard;

        // Ensure the player is in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            while (true) {
                // Allow user to select a move slot
                int selectedSlot = keyboard.Setup(entry->Name() + ":", true, options, moveSlot);

                if (selectedSlot == -1)
                    return; // Exit if user cancels selection

                // Allow user to input the move
                SearchForMove(entry);
                move = moveName;

                // Check if the selected move is valid
                if (move > 0) {
                    // Update all relevant pointers with the selected move
                    for (const auto &ptr : pointer) {
                        const u32 offset = 0x202 + (moveSlot * 0xE);
                        Process::Write16(ptr, offset, move);
                    }

                    // Notify user of the move change
                    OSD::Notify("Move " + to_string(moveSlot + 1) + ": " + movesList[move - 1]);
                    break;
                }
            }
        }
    }

    // Default multiplier is set to 1, which is the game's default
    static u8 multiplier = 1;

    // Handle input changes for experience multiplier
    void ExpMultiplier(MenuEntry *entry) {
        // Check if in battle first
        if (IfInBattle()) {
            // Prompt user to enter a new multiplier value
            if (KeyboardHandler<u8>::Set(entry->Name() + ":", true, false, 3, multiplier, 0, 1, 100, Callback<u8>)) {
                entry->SetGameFunc(UpdateExpMultiplier);
                OSD::Notify("Exp: x" + to_string(multiplier));
            }
        }
    }

    void UpdateExpMultiplier(MenuEntry *entry) {
        static const vector<u32> address = AutoGameSet({0x597700, 0x490E74}, {0x5BBD40, 0x4ACD38});
        vector<u32> instruction = {0xE1D002B2, 0xE92D4002, 0xE3A01000 + multiplier, 0xE0000190, 0xE8BD8002};

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        // Initialize MemoryManager instances if not already done
        if (!initialized) {
            for (const auto &addr : address) {
                for (int i = 0; i < instruction.size(); ++i)
                    managers.emplace_back(addr + (i * 0x4)); // Increment address for each instruction
            }

            initialized = true;
        }

        // Update multiplier in the instruction
        instruction[2] = 0xE3A01000 + multiplier;

        // Check if in battle and entry is activated
        if (IfInBattle() && entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instruction : instruction) {
                if (index >= managers.size() || !managers[index].Write(instruction)) {
                    success = false;
                    break; // Exit loop if writing fails
                }

                ++index;
            }

            // Write to the second set of addresses if applicable
            if (success && managers.size() > address.size()) {
                if (index < managers.size() && !managers[index].Write(AutoGameSet(0xEB041A21, 0xEB043C00)))
                    return; // Exit if writing fails
            }
        }

        // Restore original values if the entry is not active
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    // Function to check if the given pointer is valid for a Pokemon
    bool IsValid(u32 pointer, PK7 *pkmn) {
        return GetPokemon(pointer, pkmn);
    }

    // Static variables to manage Pokemon info display
    static bool isInfoViewOn = false; // Flag to indicate if the Pokemon info view is active
    static int  infoViewState = 0, // State of the Pokemon info view (0: Enable, 1: Disable)
                screenDisplay = 0; // Current information screen

    // Callback function to display Pokemon information
    bool ViewInfoCallback(const Screen &screen) {
        // Static data for Pokemon and UI settings
        static PK7 *pokemon = new PK7; // Pointer to Pokemon data
        static const u32 address = AutoGameSet(0x3003035C, 0x30030544); // Base address for Pokemon data
        static u32 currentOffset = address; // Current offset in the Pokemon data
        static const vector<string> statNames = {"HP", "Atk", "Def", "SpD", "SpA", "Spd"}; // Names of stats

        // Early exit if the current screen is not the top screen
        if (!screen.IsTop)
            return false;

        // Only proceed if in battle and info view is active
        if (!IfInBattle() || !isInfoViewOn)
            return false;

        // Info for navigating the position of Pokemon infos
        const u32 stepSize = 0x1E4; // Step size for navigating through Pokemon data
        const u32 maxOffset = address + (stepSize * 0x5); // Maximum offset in Pokemon data

        // Move to the next screen if the R key is pressed and within bounds
        if (Controller::IsKeyPressed(Key::R) && (currentOffset + stepSize) <= maxOffset)
            currentOffset += stepSize;

        // Move to the previous screen if the L key is pressed and within bounds
        if (Controller::IsKeyPressed(Key::L) && (currentOffset - stepSize) >= address)
            currentOffset -= stepSize;

        // Fetch the current Pokemon data
        if (!IsValid(currentOffset, pokemon))
            return false;

        // Get UI settings for colors and positions, for screen drawing
        const FwkSettings &settings = FwkSettings::Get();
        Color textColor = Color::Gainsboro; // Text color
        Color headerColor = Color::DarkGrey; // Header color
        int xPos = 5, yPos = 5; // X and Y positions for drawing text
        int lineHeight = 12; // Line height for text spacing

        // Display basic Pokemon information for the first and default screen
        if (screenDisplay == 0) {
            const CTRPluginFramework::Screen &screen = CTRPluginFramework::OSD::GetTopScreen();
            u32 slotIndex = (currentOffset - address) / stepSize; // Calculate Pokemon slot index
            screen.DrawSysfont(headerColor << "[Slot: " << Utils::ToString(slotIndex + 1, 0) << "]", xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont("Species: " << Color(0xF2, 0xCE, 0x70) << speciesList[pokemon->species - 1], xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont("Nature: " << textColor << natureList[pokemon->nature], xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont("Item: " << (pokemon->heldItem == 0 ? Color::Gray : textColor) << (pokemon->heldItem == 0 ? "None" : heldItemList[pokemon->heldItem - 1]), xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont("Ability: " << textColor << abilityList[pokemon->ability - 1], xPos, yPos, textColor);
        }

        else if (screenDisplay == 1) {
            screen.DrawSysfont(headerColor << "[Moves]", xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each move
            for (int i = 0; i < 4; i++) {
                string moveDisplay = pokemon->move[i] > 0 ? movesList[pokemon->move[i] - 1] : "None";
                screen.DrawSysfont(to_string(i + 1) + ": " << (pokemon->move[i] > 0 ? textColor : Color::Gray) << moveDisplay, xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        // Display Pokemon IVs
        else if (screenDisplay == 2) {
            screen.DrawSysfont(headerColor << "[IV]", xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each IV value
            for (int i = 0; i < statNames.size(); i++) {
                u8 ivValue = (u8)(pokemon->iv32 >> (5 * i)) & 0x1F; // Extract IV value
                screen.DrawSysfont(statNames[i] + ": " + to_string(ivValue), xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        // Display Pokemon EVs
        else if (screenDisplay == 3) {
            screen.DrawSysfont(headerColor << "[EV]", xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each EV value
            for (int i = 0; i < statNames.size(); i++) {
                screen.DrawSysfont(statNames[i] + ": " + to_string(pokemon->EV[i]), xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        return true; // Successful execution of the callback function
    }

    void TogglePokemonInfo(MenuEntry *entry) {
        // Check if currently in battle and if entry is activated or not
        if (IfInBattle() && entry->IsActivated()) {
            // Check if the first hotkey is pressed
            if (entry->Hotkeys[0].IsPressed())
                screenDisplay = (screenDisplay + 1) % 4; // Move to the next info screen, loop back if needed

            // Run the callback function to display Pokemon info
            OSD::Run(ViewInfoCallback);
        }

        // Stop displaying Pokemon info if not in battle or if entry is not activated anymore
        else OSD::Stop(ViewInfoCallback);
    }

    void ViewPokemonInfo(MenuEntry *entry) {
        static const vector<string> options = {"Enable", "Disable"}; // Options for toggling Pokemon info view
        Keyboard keyboard;

        // Check if currently in battle
        if (IfInBattle()) {
            if (keyboard.Setup(entry->Name() + ":", false, options, infoViewState) != -1) {
                // Update the flag to enable or disable Pokemon info view based on selection
                isInfoViewOn = (infoViewState == 0);
                // Set the function to toggle Pokemon info
                entry->SetGameFunc(TogglePokemonInfo);
                OSD::Notify("Info view: " + string(options[infoViewState]));
            }
        }
    }

    void AlwaysCritHit(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x80860D8, 0x8087FF0);
        // Original values to restore if needed
        static vector<u32> reset(12);
        static bool saved = false; // Flag to check if original values are saved

        // Check if currently in battle
        if (IfInBattle()) {
            if (Process::Write32(AutoGameSet(0x5979D0, 0x5BBB90),
                {0xE59D0000, 0xE92D401E, 0xE59A1008, 0xE2813004, 0xE281401C, 0xE4932004,
                0xE1500002, 0x3A01001, 0x58D1018, 0xE1530004, 0x1AFFFFF9, 0xE8BD801E},
                reset, entry, saved)) {

                // Check if the menu entry is activated
                if (entry->IsActivated()) {
                    // If address[1] contains the original value, update it
                    if (Sudo::Read32(address) == 0xE59D0000) {
                        if (!Sudo::Write32(address, AutoGameSet<u32>(0xEBF9B23C, 0xEBF976E6)))
                            return; // Exit if write fails
                    }
                }

                // Check if the menu entry is deactivated
                if (!entry->IsActivated()) {
                    // If address[1] contains the critical hit value, restore it to the original value
                    if (Sudo::Read32(address) == AutoGameSet<u32>(0xEBF9B23C, 0xEBF976E6)) {
                        if (!Sudo::Write32(address, 0xE59D0000))
                            return; // Exit if write fails
                    }
                }
            }
        }
    }

    void AllowMultipleMega(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x80A2CE4, 0x80A6150);

        // Check if currently in battle
        if (IfInBattle()) {
            // Determine the current and new values based on activation state
            bool isActivated = entry->IsActivated();
            // Read the current value at the address
            u32 currentValue = Sudo::Read32(address);
            u32 instruction = isActivated ? 0xE3A00000 : 0xE12FFF32;
            u32 expected = isActivated ? 0xE12FFF32 : 0xE3A00000;

            // Check if current value matches the expected value
            if (currentValue == expected) {
                // Write the instruction to the address
                if (!Sudo::Write32(address, instruction))
                    return; // Exit if write fails
            }
        }
    }

    void ReuseZCrystal(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x80311D4, 0x8031E74);
        vector<u32> expected(3);

        // Check if currently in battle
        if (IfInBattle()) {
            // Determine the expected values and new values based on entry activation state
            bool isActivated = entry->IsActivated();
            vector<u32> currentValue = Sudo::Read32(address, 3);

            if (isActivated) {
                expected = {AutoGameSet(0xE1D000D5, 0xEB0195FE), 0xE3500000, 0xE320F000};
                vector<u32> instruction = {0xE3A00000, 0xE5C30005, 0xE1500000};

                if (currentValue == expected) {
                    if (!Sudo::Write32(address, instruction))
                        return; // Exit if write fails
                }
            }

            else {
                expected = {0xE3A00000, 0xE5C30005, 0xE1500000};
                vector<u32> restore = {AutoGameSet(0xE1D000D5, 0xEB0195FE), 0xE3500000, 0xE320F000};

                if (currentValue == expected) {
                    if (!Sudo::Write32(address, restore))
                        return; // Exit if write fails
                }
            }
        }
    }

    void IgnoreZCrystalReq(MenuEntry *entry) {
        // Instruction sets for modifying the Z-Crystal behavior
        static const vector<u32> instruction = {
            AutoGameSet(
                {0xE92D4005, 0xE1A00006, 0xEA000000, 0xE92D4005, 0xE59D2050, 0xE59F100C, 0xE1510002, 0x1D510B4, 0x11A01000, 0xE8BD8005, 0x78BF60},
                {0xE92D407D, 0xE1A00006, 0xEA000000, 0xE92D407D, 0xE1D510B4, 0xE59D201C, 0xE202530F, 0xE3550203, 0x18BD807D, 0xE59F300C, 0xEB000004, 0xE59F3008, 0xEB000002, 0xE8BD807D, 0x30000404, 0x300073EC, 0xE2834018, 0xE4935004, 0xE1550002, 0x1A01000, 0xE1530004, 0x1AFFFFFA, 0xE12FFF1E}
        )};

        // Store original instruction values and revert states
        static vector<u32> reset(instruction.size()), revert(2);
        static bool saved[2] = {false, false};

        // Check if currently in battle
        if (IfInBattle()) {
            // Apply new instruction
            if (Process::Write32(AutoGameSet(0x597800, 0x5BBDE0), instruction, reset, entry, saved[0])) {
                // Write revert instruction to specified addresses
                if (Process::Write32(AutoGameSet(
                    {0x314300, 0x314370},
                    {0x31C494, 0x31C504}),
                    {AutoGameSet(0xEB0A0D3E, 0xEB0A7E51), AutoGameSet(0xEB0A0D25, 0xEB0A7E38)}, revert, entry, saved[1])) {
                    // Update the final address based on hotkey status
                    entry->Hotkeys[0].IsDown() ? Process::Write32(AutoGameSet(0x36DFF4, 0x376EB0), 0xE3A00001) : Process::Write32(AutoGameSet(0x36DFF4, 0x376EB0), 0xE3A00000);
                    return; // Exit function after applying changes
                }
            }
        }
    }

    void NoWildPokemon(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x807A5E8, 0x807E5B8);
        u32 currentValue = Sudo::Read32(address);

        // Determine the desired and expected values based on entry activation state
        bool isActivated = entry->IsActivated();
        u32 expected = isActivated ? 0xE3A00064 : 0xE3A09000;
        u32 instruction = isActivated ? 0xE3A09000 : 0xE3A00064;

        // Update the value if the current value matches the expected value
        if (currentValue == expected) {
            if (!Sudo::Write32(address, instruction))
                return; // Exit if write fails
        }
    }

    static int pokemon, form;
    static u8 level;

    void UpdateWildSpawner(MenuEntry *entry) {
        static vector<u32> reset(10);
        static vector<u32> revert(6);
        static bool saved[2] = {false, false};

        // Write the new configuration to the addresses
        if (Process::Write32(AutoGameSet(0x597BA0, 0x5BBFC0),
            {0xE92D400E, 0xE59F0014, 0xE3500000, 0x18BD800E, 0xE59F000C,
            0xEBF83F51, 0xE2800001, 0xE8BD800E, 0, 0x327},
            reset, entry, saved[0])) {
            if (!Process::Write32(AutoGameSet(
                {0x597BC0, 0x399CB4, 0x399CC4, 0x399CF0, 0x399C9C, 0x399D50},
                {0x5BBFE0, AutoGame(0x3A86A0, 0x3A86A4), AutoGame(0x3A86B0, 0x3A86B4), AutoGame(0x3A86DC, 0x3A86E0), AutoGame(0x3A8688, 0x3A868C), AutoGame(0x3A873C, 0x3A8740)}
            ), {(unsigned int)pokemon, AutoGameSet<u32>(
                0xEB07F7B9, AutoGame(0xEB084E46, 0xEB084E45)),
                0xE1D400B0, 0xE1D400B0, 0xE3A00000 + level, 0xC3A00000 + form},
                revert, entry, saved[1]))
                return; // Exit if write fails
        }
    }

    void WildSpawner(MenuEntry *entry) {
        // Find and set the Pokemon species
        SearchForSpecies(entry);
        pokemon = speciesID;
        Keyboard keyboard;

        // Check if a valid Pokemon species was found
        if (pokemon > 0) {
            // Set up and handle form selection for the Pokemon
            if (keyboard.Setup("Form:", true, formList(pokemon), form) != -1) {
                // Set up and handle level input for the Pokemon
                if (KeyboardHandler<u8>::Set("Level:", true, false, 3, level, 0, 1, 100, Callback<u8>)) {
                    // Assign the function to update wild spawner configuration
                    entry->SetGameFunc(UpdateWildSpawner);
                    OSD::Notify("Spawning: " + string(speciesList[pokemon - 1]));
                }
            }
        }
    }

    void AlwaysShiny(MenuEntry *entry) {
        static MemoryManager manager(AutoGameSet(0x318AF0, 0x320CF0));

        if (entry->IsActivated()) {
            // Write the new instruction
            if (!manager.Write(0xEA00001C))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void TrainerRematch(MenuEntry *entry) {
        static const u32 address = AutoGameSet<u32>(0x49EFC8, 0x4BB03C);
        static const vector<u32> instruction = {0xE3A00000, 0xE12FFF1E};

        // Create MemoryManager instances for each address
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < instruction.size(); ++i)
                managers.emplace_back(address + (i * 0x4)); // Increment address for each instruction

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instruction : instruction) {
                if (index >= managers.size() || !managers[index].Write(instruction)) {
                    success = false;
                    break; // Exit loop if writing fails
                }

                ++index;
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    void NoTrainerBattle(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x802EBF0, 0x8032034);
        u32 currentValue = Sudo::Read32(address);

        // Determine the expected and new values based on entry activation state
        bool isActivated = entry->IsActivated();
        u32 expected = isActivated ? AutoGameSet(0xEB00FA2F, 0xEB00F53B) : 0xE3A00001;
        u32 instruction = isActivated ? 0xE3A00001 : AutoGameSet(0xEB00FA2F, 0xEB00F53B);

        // Update the value if the current value matches the expected value
        if (currentValue == expected) {
            if (!Sudo::Write32(address, instruction))
                return; // Exit if write fails
        }
    }

    void CaptureRate(MenuEntry *entry) {
        static vector<u32> reset(9);
        static u32 revert;
        static bool saved[2] = {false, false};

        // Check if the player is in battle.
        if (IfInBattle()) {
            // Write to the first address and check if successful.
            if (Process::Write32(AutoGameSet(0x59775C, 0x5BBEA0), {0xE5D00008, 0xE92D4003, 0xE59D0010, 0xE59F100C, 0xE1510000, 0x24000F8, 0x58D0010, 0xE8BD8003, AutoGameSet<u32>(0x6DA1CC, 0x70C62C)}, reset, entry, saved[0])) {
                // Write to the second address and check if successful.
                if (!Process::Write32(AutoGameSet(0x490E68, 0x4ACD2C), AutoGameSet<u32>(0xEB041A3B, 0xEB043C5B), revert, entry, saved[1]))
                    return;
            }
        }
    }

    void TrialIsCatchable(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x8076530, AutoGame(0x80B8914, 0x80B88FC)}, {0x8077CD8, 0x80BC874})};

        // Check if the player is in battle.
        if (IfInBattle()) {
            // If the entry is activated, verify and write new values.
            if (entry->IsActivated()) {
                if (Sudo::Read32(address[0]) == 0xE3A00000 && Sudo::Read32(address[1]) == 0xE1A01004) {
                    if (!Sudo::Write32(address[0], 0xE3A00001) || !Sudo::Write32(address[1], 0xE3A01000))
                        return;
                }
            }

            // If the entry is not activated, revert the values.
            else {
                if (Sudo::Read32(address[0]) == 0xE3A00001 && Sudo::Read32(address[1]) == 0xE3A01000) {
                    if (!Sudo::Write32(address[0], 0xE3A00000) || !Sudo::Write32(address[1], 0xE1A01004))
                        return;
                }
            }
        }
    }

    void CatchTrainerPokemon(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8035AA8, 0x8036F18);
        // Vectors to store expected values for validation.
        vector<u32> check(4);

        // Check if the player is in battle.
        if (IfInBattle()) {
            // If the entry is activated, set and check the expected values.
            if (entry->IsActivated()) {
                check = {AutoGameSet(0xEB019429, 0xEB01979E), 0xE3500000, 0xE320F000, 0xA00000B};

                if (Sudo::Read32(address, 4) == check) {
                    if (!Sudo::Write32(address, {0xE3A0B000, 0xE590000C, 0xE5C0B000, 0xEA00000B}))
                        return;
                }
            }

            // If the entry is not activated, revert to original values.
            else {
                check = {0xE3A0B000, 0xE590000C, 0xE5C0B000, 0xEA00000B};

                if (Sudo::Read32(address, 4) == check) {
                    if (!Sudo::Write32(address, {AutoGameSet(0xEB019429, 0xEB01979E), 0xE3500000, 0xE320F000, 0xA00000B}))
                        return;
                }
            }
        }
    }

    void UnlockEveryMount(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x330D7810, 0x330138D0);
        u32 data32;

        if (!Process::Read32(address, data32)) { // Check if reading data fails
            OSD::Notify("Failed to read data from address"); // Notify user if reading data fails
            return; // Exit function early
        }

        // Check if the lower bits are already set to 0x3FF80
        if ((data32 & ~0xFFF00000) == 0x3FF80) {
            OSD::Notify("Already unlocked all mounts"); // Notify user that mounts are already unlocked
            return; // Exit function early
        }

        // Keep the top 12 bits and set the lower bits to 0x3FF80
        data32 = (data32 & 0xFFF00000) + 0x3FF80;
        Process::Write32(address, data32); // Write the modified value back to the address
        OSD::Notify("All mounts unlocked!"); // Notify user that all mounts are unlocked
    }

    void ZoomedOutView(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x597724, 0x8037B18, 0x59773C}, {0x5BBF80, 0x803B920, 0x5BBF98})};
        // Define patch values to modify game memory
        static u32 patch[7] = {0xED940A1C, 0xED9F1A03, 0xEE300A01, 0xED840A1C, 0xE2840004, 0xE12FFF1E, 0};
        static bool notified = false; // If the notification has been sent

        // Apply patch when the menu entry is just activated
        if (entry->WasJustActivated()) {
            Process::Patch(address[0], patch, 0x1C); // Apply the patch

            // Notify user about key bindings only once
            if (!notified) {
                OSD::Notify("Zoom: L to zoom out, R to reset to default");
                notified = true; // Set the flag to true after notifying
            }
        }

        else if (!entry->IsActivated()) {
            Process::Patch(address[0], 0, 0x1C); // Revert the patch to the original state
            notified = false; // Reset the flag when deactivating
            return;
        }

        // Check ongoing activation of the menu entry
        if (entry->IsActivated()) {
            if (Sudo::Read32(address[1]) == 0xE2840004)
                Sudo::Write32(address[1], AutoGameSet<u32>(0xEBFAEB01, 0xEBFAA996)); // Update memory based on game condition

            // Change behavior based on key presses
            if (Controller::IsKeyPressed(Key::L))
                Process::Write32(address[2], 0xC4BB8000);

            if (Controller::IsKeyPressed(Key::R))
                Process::Write32(address[2], 0);
        }
    }

    void FastWalkRun(MenuEntry *entry) {
        static MemoryManager manager(AutoGameSet<u32>(0x39C394, AutoGame(0x3AAEB0, 0x3AAEB4)));

        if (entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.Write(0x3FB33333);
        }

        // When deactivating, the MemoryManager instance will be destroyed
        else manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
    }

    void WalkThroughWalls(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x39E560, 0x39E694}, {AutoGame(0x3AD228, 0x3AD22C), AutoGame(0x3AD35C, 0x3AD360)})};
        static const vector<vector<u32>> instruction = {{0xE1A00000, 0xE1A00000}, {0xE1A00000, 0xE1A00000}};

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < address.size(); ++i) {
                for (int j = 0; j < instruction[i].size(); ++j)
                    managers.emplace_back(address[i] + (j * 0x4)); // Increment address for each instruction
            }

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instrSet : instruction) {
                for (const auto &instruction : instrSet) {
                    if (index >= managers.size() || !managers[index].Write(instruction)) {
                        success = false;
                        break; // Exit loop if writing fails
                    }

                    ++index;
                }

                if (!success)
                    break; // Exit outer loop if writing fails
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    static u8 time; // Store time in-game

    void SetSunMoon(MenuEntry *entry) {
        if (KeyboardHandler<u8>::Set("Time:\n\nTime must be within range of 1-24 hours\n\n" << Color::SkyBlue << "12H" << Color::White << ": Moon\n\n" << Color::Orange << "24H" << Color::White << ": Sun\n", true, false, 2, time, 0, 1, 24, Callback<u8>)) {
            Process::Write32(AutoGameSet(0x330D9238, 0x330154B0), time * 3600); // Write the time in seconds to the address
            OSD::Notify("Updated in-game time by " + to_string(time) + " hours"); // Notify user of the update
        }
    }

    void RenameAnyPokemon(MenuEntry *entry) {
        static MemoryManager manager(AutoGameSet(0x4AA2C0, 0x4C6F64));

        if (entry->IsActivated()) {
            if (!manager.Write(0xE3A00001))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void LearnAnyTeachable(MenuEntry *entry) {
        static MemoryManager manager(AutoGameSet(0x490D34, 0x4ACBEC));

        if (entry->IsActivated()) {
            if (!manager.Write(0xE3A00001))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void InstantEgg(MenuEntry *entry) { // <W: Sun, Moon, Ultra Sun, Ultra Moon
        static const u32 address = AutoGameSet<u32>(0x4466A8, AutoGame(0x45C628, 0x45C62C));
        static const vector<u32> instruction = {0xE3A01001, 0xE5C011E0, AutoGameSet<u32>(0xEA00684B, AutoGame(0xEA007D39, 0xEA007D3A))};

        // Create MemoryManager instances for each address
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < instruction.size(); ++i)
                managers.emplace_back(address + (i * 0x4)); // Increment address for each instruction

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instruction : instruction) {
                if (index >= managers.size() || !managers[index].Write(instruction)) {
                    success = false;
                    break; // Exit loop if writing fails
                }

                ++index;
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    void InstantEggHatch(MenuEntry *entry) { // <W: Sun, Moon, Ultra Sun, Ultra Moon
        static const vector<u32> address = {AutoGameSet({0x5977C0, 0x493668}, {0x5BBE40, 0x4AF604})};
        static const vector<vector<u32>> instruction = {
            {0xE59D000C, 0xE59F500C, 0xE1500005, 0x3A00000, 0x11A00004, 0xE12FFF1E, AutoGameSet<u32>(0x6D08C0, 0x704650)},
            {AutoGameSet<u32>(0xEB041054, 0xEB04320D)}
        };

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < address.size(); ++i) {
                for (int j = 0; j < instruction[i].size(); ++j)
                    managers.emplace_back(address[i] + (j * 0x4)); // Increment address for each instruction
            }

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instrSet : instruction) {
                for (const auto &instruction : instrSet) {
                    if (index >= managers.size() || !managers[index].Write(instruction)) {
                        success = false;
                        break; // Exit loop if writing fails
                    }

                    ++index;
                }

                if (!success)
                    break; // Exit outer loop if writing fails
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    void ViewValuesInSummary(MenuEntry *entry) { // <W: Sun, Moon, Ultra Sun, Ultra Moon
        static const vector<u32> address = {
            AutoGameSet(
                {0x49369C, 0x493A08, 0x4937CC, 0x493818, 0x493A54, 0x493AA0, 0x493780},
                {0x4AF638, 0x4AF9A4, 0x4AF768, 0x4AF7B4, 0x4AF9F0, 0x4AFA3C, 0x4AF71C}
        )};

        static const vector<vector<u32>> temp = {
            {0x8010E78, 0x8010E7C, 0x8010EB0, 0x8010EB4, 0x8010EE0, 0x8010EE4, 0x8010F10,
             0x8010F14, 0x8010F40, 0x8010F44, 0x8010F70, 0x8010F74, 0x8010FA0, 0x8010FA4},
            {0xE591000C, 0xEBF772A4, 0xE596000C, 0xEBF77296, 0xE596000C, 0xEBF7732C, 0xE596000C,
             0xEBF7732B, 0xE596000C, 0xEBF773A6, 0xE596000C, 0xEBF773A5, 0xE596000C, 0xEBF772F1}
        };

        static const vector<vector<u32>> instruction = {
            AutoGameSet({0xEAFFFCFD, 0xEAFFFC22, 0xEAFFFD15, 0xEAFFFD0F, 0xEAFFFD14, 0xEAFFFD0E, 0xEAFFFD1B},
                        {0xEAFFFCE1, 0xEAFFFC06, 0xEAFFFCF9, 0xEAFFFCF3, 0xEAFFFCF8, 0xEAFFFCF2, 0xEAFFFCFF}),
            AutoGameSet({0xEAFFFCE1, 0xEAFFFC06, 0xEAFFFCF9, 0xEAFFFCF3, 0xEAFFFCF8, 0xEAFFFCF2, 0xEAFFFCFF},
                        {0xEAFFFC82, 0xEAFFFBA7, 0xEAFFFCD6, 0xEAFFFCCE, 0xEAFFFCC6, 0xEAFFFCBE, 0xEAFFFCDE})
        };

        static const vector<u32> reset(7, 0xE92D4070);

        // If first hotkey is pressed
        if (entry->Hotkeys[0].IsDown())
            Process::Write32(address, instruction[0]);

        // If second hotkey is pressed
        else if (entry->Hotkeys[1].IsDown()) {
            if (currGameSet == GameSet::Base) { // Sun and Moon
                for (int i = 0; i < temp[0].size(); i++)
                    Sudo::Write32(temp[0][i], temp[1][i]);
            }

            else Process::Write32(address, instruction[1]);
        }

        else {
            if (currGameSet == GameSet::Base) { // Sun and Moon
                const vector<u32> restore = {0xE1A00001, 0xEBFFFD13, 0xE1A00006, 0xEBFFFD07, 0xE1A00006, 0xEBFFFCFD, 0xE1A00006,
                                             0xEBFFFCF1, 0xE1A00006, 0xEBFFFCE5, 0xE1A00006, 0xEBFFFCD9, 0xE1A00006, 0xEBFFFCCD};

                for (int i = 0; i < temp[0].size(); i++)
                    Sudo::Write32(temp[0][i], restore[i]);
            }

            // Reset values to original
            Process::Write32(address, reset);
        }
    }

    void NoOutlines(MenuEntry *entry) {
        static MemoryManager manager(AutoGameSet<u32>(0x41CFCC, AutoGame(0x42FBA4, 0x42FBA8)));

        if (entry->IsActivated()) {
            // Write new instruction when activated
            if (!manager.Write(0xE320F000))
                return; // Exit if writing fails
        }

        // Restore the original value when deactivated
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void FastDialogs(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x3BFBF4, 0x3C0498}, {AutoGame(0x3D12B4, 0x3D12B8), AutoGame(0x3D1B58, 0x3D1B5C)})};
        static const vector<u32> instruction = {0xE3A04003, 0xE3A05003};

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < address.size(); ++i)
                managers.emplace_back(address[i]); // Create a MemoryManager instance for each address

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;

            // Write new instruction to the addresses
            for (int i = 0; i < address.size(); ++i) {
                if (i >= managers.size() || !managers[i].Write(instruction[i])) {
                    success = false;
                    break; // Exit loop if writing fails
                }
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }
}