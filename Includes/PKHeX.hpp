  #ifndef PKHEX_HPP
  #define PKHEX_HPP

  namespace CTRPluginFramework {
   void Identity(MenuEntry *entry);
   void InGameName(MenuEntry *entry);
   void PlayTime(MenuEntry *entry);
   void UpdatePlayTime(MenuEntry *entry);
   void GameLanguage(MenuEntry *entry);
   void UpdateLanguage(MenuEntry *entry);
   void Money(MenuEntry *entry);
   void ApplyMoney(MenuEntry *entry);
   void BattlePoints(MenuEntry *entry);
   void ApplyBattlePoints(MenuEntry *entry);

   void Items(MenuEntry *entry);
   void UpdateItems(MenuEntry *entry);
   void Medicines(MenuEntry *entry);
   void UpdateMedicines(MenuEntry *entry);
   void Berries(MenuEntry *entry);
   void UpdateBerries(MenuEntry *entry);
   void UnlockTMsAndHMs(MenuEntry *entry);
   void UnlockKeyItems(MenuEntry *entry);
   void KeyItems(MenuEntry *entry);

   struct PK7 {
      u32 encryptionConstant; // Encryption constant for data security
      u16 sanity; // Sanity check value
      u16 checksum; // Data checksum for integrity verification

      // Block A
      u16 species; // Pokemon species ID
      u16 heldItem; // Held item ID
      u16 TID; // Trainer ID
      u16 SID; // Secret ID
      u32 exp; // Experience points
      u8 ability; // Ability ID
      u8 abilityNumber; // Ability number
      u8 markValue; // Mark value
      u32 PID; // Pokemon ID
      u8 nature; // Nature ID
      u8 fatefulEncounterGenderForm; // Gender/Form for fateful encounters
      u8 EV[6]; // Effort Values for stats
      u8 contest[6]; // Contest stats
      u8 resortEventStatus; // Resort event status
      u8 infected; // Infection status (e.g., Pokerus)
      u8 st1; // Stat 1
      u8 st2; // Stat 2
      u8 st3; // Stat 3
      u8 st4; // Stat 4
      u8 ribbons[6]; // Ribbons
      u8 unused1[2]; // Unused bytes
      u8 ribbonCountMemoryContest; // Ribbon count memory for contests
      u8 ribbonCountMemoryBattle; // Ribbon count memory for battles
      u8 superTrainingFlags; // Flags for super training
      u8 unused2[5]; // Unused bytes

      // Block B
      u8 nickname[26]; // Nickname of the Pokemon
      u16 move[4]; // Move IDs
      u8 movePP[4]; // Move Power Points
      u8 movePPUp[4]; // Move Power Point Ups
      u16 relearn[4]; // Relearnable moves
      u8 secretSuperTrainingUnlockled; // Secret super training unlocked status
      u8 unused3; // Unused byte
      u32 iv32; // Individual values (IVs) packed into 32 bits

      // Block C
      u8 hiddenTrainerName[26]; // Hidden trainer's name
      u8 hiddenTrainerGender; // Hidden trainer's gender
      u8 currentHandler; // Current handler (e.g., for events)
      u8 geo1Region; // Geographic region 1
      u8 geo1Country; // Geographic country 1
      u8 geo2Region; // Geographic region 2
      u8 geo2Country; // Geographic country 2
      u8 geo3Region; // Geographic region 3
      u8 geo3Country; // Geographic country 3
      u8 geo4Region; // Geographic region 4
      u8 geo4Country; // Geographic country 4
      u8 geo5Region; // Geographic region 5
      u8 geo5Country; // Geographic country 5
      u8 unused4[4]; // Unused bytes
      u8 hiddenTrainerFriendship; // Hidden trainer's friendship level
      u8 hiddenTrainerAffection; // Hidden trainer's affection level
      u8 hiddenTrainerIntensity; // Hidden trainer's intensity level
      u8 hiddenTrainerMemory; // Hidden trainer's memory
      u8 hiddenTrainerFeeling; // Hidden trainer's feeling
      u8 unused5; // Unused byte
      u16 hiddenTrainerTextVar; // Hidden trainer's text variable
      u8 unused6[4]; // Unused bytes
      u8 fullness; // Fullness level
      u8 enjoyment; // Enjoyment level

      // Block D
      u8 originalTrainerName[26]; // Original trainer's name
      u8 originalTrainerFriendship; // Original trainer's friendship level
      u8 originalTrainerAffection; // Original trainer's affection level
      u8 originalTrainerIntensity; // Original trainer's intensity level
      u8 originalTrainerMemory; // Original trainer's memory
      u16 originalTrainerTextVar; // Original trainer's text variable
      u8 originalTrainerFeeling; // Original trainer's feeling
      u8 eggDate[3]; // Egg date (year, month, day)
      u8 metDate[3]; // Met date (year, month, day)
      u8 unused7; // Unused byte
      u16 eggLocation; // Egg location ID
      u16 metLocation; // Met location ID
      u8 ball; // Poké Ball used
      u8 metLevel; // Level at which the Pokemon was met
      u8 hyperTrainFlags; // Hyper training flags
      u8 version; // Version of the game
      u8 country; // Country of origin
      u8 region; // Region of origin
      u8 consoleRegion; // Console region
      u8 language; // Language
      u8 unused8[4]; // Unused bytes
   };

   /* Template function to retrieve Pokemon data using a given pointer.
   The function takes a pointer to the Pokemon data and a reference to a data object where the retrieved data will be stored.
   It returns true if the data retrieval is successful, otherwise false. */
   template <class Pokemon>
   bool GetPokemon(u32 pokePointer, Pokemon data);

   /* External variable declaration for the pointer to the species data.
   This pointer is used to access the Pokemon species information in memory. */
   extern u32 speciesPointer;

   /* Function to retrieve the pointer to the species data in memory.
   This function returns the value of the speciesPointer, which points to the Pokemon species information. */
   u32 GetSpeciesPointer(void);

   namespace PKHeX {
      void Position(MenuEntry *entry);
      void Shiny(MenuEntry *entry);
      void Species(MenuEntry *entry);
      void IsNicknamed(MenuEntry *entry);
      void Nickname(MenuEntry *entry);
      void Level(MenuEntry *entry);
      void Nature(MenuEntry *entry);
      void Gender(MenuEntry *entry);
      void Form(MenuEntry *entry);
      void HeldItem(MenuEntry *entry);
      void Ability(MenuEntry *entry);
      void Friendship(MenuEntry *entry);
      void Language(MenuEntry *entry);
      void IsEgg(MenuEntry *entry);
      void Pokerus(MenuEntry *entry);
      void Country(MenuEntry *entry);
      void ConsoleRegion(MenuEntry *entry);
      void OriginGame(MenuEntry *entry);
      void Ball(MenuEntry *entry);
      void MetLevel(MenuEntry *entry);
      void MetDate(MenuEntry *entry);
      void IsFatefulEncounter(MenuEntry *entry);
      void EggMetDate(MenuEntry *entry);
      void IV(MenuEntry *entry);
      void EV(MenuEntry *entry);
      void Contest(MenuEntry *entry);
      void CurrentMove(MenuEntry *entry);
      void PPUp(MenuEntry *entry);
      void RelearnMove(MenuEntry *entry);
      void SID(MenuEntry *entry);
      void TID(MenuEntry *entry);
      void OTName(MenuEntry *entry);
      void LatestHandler(MenuEntry *entry);
   }

   void ExportImport(MenuEntry *entry);
   void PCAnywhere(MenuEntry *entry);
   void BoxesUnlocked(MenuEntry *entry);
}

#endif // PKHEX_HPP