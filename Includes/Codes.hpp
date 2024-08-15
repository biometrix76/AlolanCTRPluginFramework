#ifndef CHEATS_HPP
#define CHEATS_HPP

#include <CTRPluginFramework.hpp>
#include "Helpers.hpp"

namespace CTRPluginFramework {
    void CurrentRank(MenuEntry *entry);
    void FCCollected(MenuEntry *entry);
    void CurrentFC(MenuEntry *entry);
    void GuestsInteracted(MenuEntry *entry);
    void FacilsHosted(MenuEntry *entry);
    void MissionsJoined(MenuEntry *entry);

    vector<string> GetBattleParty(vector<string> party);
    void PartyPosition(MenuEntry *entry);
    void StatusCondition(MenuEntry *entry);
    void Stats(MenuEntry *entry);
    void UpdateStats(MenuEntry *entry);
    void HealthAndMana(MenuEntry *entry);
    void UpdateHealthAndMana(MenuEntry *entry);
    void HeldItem(MenuEntry *entry);
    void Moves(MenuEntry *entry);
    void ExpMultiplier(MenuEntry *entry);
    void UpdateExpMultiplier(MenuEntry *entry);
    bool IsValid(u32 pointer, PK7 *pkmn);
    bool ViewInfoCallback(const Screen &screen);
    void TogglePokemonInfo(MenuEntry *entry);
    void ViewPokemonInfo(MenuEntry *entry);
    void AlwaysCritHit(MenuEntry *entry);
    void AllowMultipleMega(MenuEntry *entry);
    void ReuseZCrystal(MenuEntry *entry);
    void IgnoreZCrystalReq(MenuEntry *entry);
    void NoWildPokemon(MenuEntry *entry);
    void UpdateWildSpawner(MenuEntry *entry);
    void WildSpawner(MenuEntry *entry);
    void AlwaysShiny(MenuEntry *entry);
    void TrainerRematch(MenuEntry *entry);
    void NoTrainerBattle(MenuEntry *entry);
    void CaptureRate(MenuEntry *entry);
    void TrialIsCatchable(MenuEntry *entry);
    void CatchTrainerPokemon(MenuEntry *entry);

    void UnlockEveryMount(MenuEntry *entry);
    void ZoomedOutView(MenuEntry *entry);
    void FastWalkRun(MenuEntry *entry);
    void WalkThroughWalls(MenuEntry *entry);
    void SetSunMoon(MenuEntry *entry);
    void RenameAnyPokemon(MenuEntry *entry);
    void LearnAnyTeachable(MenuEntry *entry);
    void InstantEgg(MenuEntry *entry);
    void InstantEggHatch(MenuEntry *entry);
    void ViewValuesInSummary(MenuEntry *entry);
    void NoOutlines(MenuEntry *entry);
    void FastDialogs(MenuEntry *entry);
}

#endif