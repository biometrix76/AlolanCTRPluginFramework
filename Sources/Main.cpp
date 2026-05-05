#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <vector>

#include "Codes.hpp"
#include "Parser.hpp"
#include "PKHeX.hpp"

namespace CTRPluginFramework {
    ///< Language & translation WIP
    TextFileParser *getLanguage = nullptr;

	static const vector<string> languageText {
        PATH_ENGLISH_TEXT,
        PATH_FRENCH_TEXT,
        PATH_ITALIAN_TEXT,
        PATH_SPANISH_TEXT,
        PATH_GERMAN_TEXT,
        PATH_JAPANESE_TEXT
    };

    enum Language {NONE = 0, ENG, FRE, ITA, SPA, GER, JAP};
    Language currentLanguage = Language::NONE;

    static const vector<string> name_EN = {
        "Festival Plaza","Current Rank","FC Collected","Current FC","Guests Interacted With","Facilities Hosted","Missions Participated In",
        "Combat","In-Battle","Party Position:","Condition","Stats","Health & Mana","Item","Moves","Exp. Multiplier","View Pokemon Infos.","Always Crit. Hit","Allow 1+ Megas.","Reuse Z-Crystal","Ignore Z-Crystal Req.","Other","No Wild Pokemon","Wild Pokemon Spawner","Always Shiny","[IGNORE]","Trainer Rematch","No Trainer Battle","Capture Rate 100%","Trial Is Catchable","Catch Trainer Pokemon",
        "PKHeX","Trainer","Info","TID & SID","Name","Time Played","Game Language","Bag","Money","Battle Points","Items","Medicines","Berries","TMs & HMs","[IGNORE]","PC Box","Export & Import","PKHeX","Position","Main","Shiny","Species","Is Nicknamed","Nickname","Level","Nature","Gender","Form","Held Item","Ability","Friendship","Language","Is Egg","Pokérus","Country","[IGNORE]","3DS Region","Origins","Game From","[IGNORE]","Ball","Met Level","Met Date","Is Fateful Enc.","[IGNORE]","Egg Met Date","Stats","IV","EV","Contest","Moves","Current Moves","PP Ups","Relearn Moves","Misc.","SID","TID","OT Name","Latest (not OT) Handler","[IGNORE]","PC Anywhere","31 Boxes Unlocked",
        "Misc.","Unlock Every Mount","Zoomed-out View","Fast Walk & Run","Walk Through Walls","Set Sun & Moon","Other","Rename Pokemon","Learn Any TM & HM","Instant D.C Egg","Hatch Egg Immediately","View IVs & EVs in Summary","No Outlines","Super Fast Dialogs"
    };

    static const vector<string> name_FR = {
        "Place Festival","Rang actuel","FC collectés","FC actuels","Invités rencontrés","Installations hébergées","Missions participées",
        "Combat","En combat","Position de l'équipe :","État","Statistiques","Santé & Mana","Objet","Capacités","Multiplicateur Exp.","Voir infos Pokémon","Toujours critique","Autoriser 1+ Méga","Réutiliser Cristal Z","Ignorer conditions Z","Autre","Aucun Pokémon sauvage","Générateur sauvage","Toujours chromatique","[IGNORE]","Revanche dresseur","Pas de combat dresseur","Capture 100%","Épreuve capturable","Capturer Pokémon dresseur",
        "PKHeX","Dresseur","Infos","TID & SID","Nom","Temps de jeu","Langue du jeu","Sac","Argent","Points Combat","Objets","Médicaments","Baies","CT & CS","[IGNORE]","Boîte PC","Exporter & Importer","PKHeX","Position","Principal","Chromatique","Espèce","A surnom","Surnom","Niveau","Nature","Genre","Forme","Objet tenu","Talent","Amitié","Langue","Est œuf","Pokérus","Pays","[IGNORE]","Région 3DS","Origines","Jeu d'origine","[IGNORE]","Poké Ball","Niveau capture","Date capture","Rencontre fatidique","[IGNORE]","Date œuf","Statistiques","IV","EV","Concours","Capacités","Capacités actuelles","PP Plus","Réapprendre capacités","Divers","SID","TID","Nom OT","Dernier dresseur","[IGNORE]","PC partout","31 boîtes débloquées",
        "Divers","Débloquer montures","Vue éloignée","Marche & course rapides","Traverser murs","Définir Soleil & Lune","Autre","Renommer Pokémon","Apprendre toutes CT/CS","Œuf instantané","Éclosion immédiate","Voir IV/EV résumé","Sans contours","Dialogues rapides"
    };

    static const vector<string> name_IT = {
        "Piazza Festival","Grado attuale","FC raccolti","FC attuali","Ospiti incontrati","Strutture ospitate","Missioni completate",
        "Combattimento","In battaglia","Posizione squadra:","Condizione","Statistiche","Salute e Mana","Strumento","Mosse","Moltiplicatore Exp.","Info Pokémon","Critico sempre","Permetti 1+ Mega","Riusa Cristallo Z","Ignora requisiti Z","Altro","Nessun Pokémon selvatico","Generatore selvatici","Sempre cromatico","[IGNORE]","Rivincita Allenatore","No lotta Allenatore","Cattura 100%","Prova catturabile","Cattura Pokémon allenatore",
        "PKHeX","Allenatore","Info","TID & SID","Nome","Tempo di gioco","Lingua gioco","Borsa","Denaro","Punti Lotta","Strumenti","Medicine","Bacche","MT & MN","[IGNORE]","Box PC","Esporta & Importa","PKHeX","Posizione","Principale","Cromatico","Specie","Ha soprannome","Soprannome","Livello","Natura","Genere","Forma","Strumento tenuto","Abilità","Amicizia","Lingua","È uovo","Pokérus","Paese","[IGNORE]","Regione 3DS","Origini","Gioco origine","[IGNORE]","Poké Ball","Livello incontro","Data incontro","Incontro fatidico","[IGNORE]","Data uovo","Statistiche","IV","EV","Gara","Mosse","Mosse attuali","PP Plus","Reimpara mosse","Varie","SID","TID","Nome OT","Ultimo allenatore","[IGNORE]","PC ovunque","31 Box sbloccati",
        "Varie","Sblocca cavalcature","Vista zoom out","Cammina & corri veloce","Attraversa muri","Imposta Sole & Luna","Altro","Rinomina Pokémon","Impara tutte MT/MN","Uovo istantaneo","Schiusa immediata","Vedi IV/EV","Senza contorni","Dialoghi veloci"
    };

    static const vector<string> name_SP = {
        "Plaza Festival","Rango actual","FC recolectados","FC actuales","Invitados interactuados","Instalaciones","Misiones participadas",
        "Combate","En batalla","Posición equipo:","Estado","Estadísticas","Salud y Maná","Objeto","Movimientos","Multiplicador Exp.","Ver info Pokémon","Siempre crítico","Permitir 1+ Mega","Reusar Cristal Z","Ignorar requisitos Z","Otro","Sin Pokémon salvajes","Generador salvaje","Siempre variocolor","[IGNORE]","Revancha entrenador","Sin combate entrenador","Captura 100%","Prueba capturable","Capturar Pokémon entrenador",
        "PKHeX","Entrenador","Info","TID & SID","Nombre","Tiempo jugado","Idioma del juego","Bolsa","Dinero","Puntos Batalla","Objetos","Medicinas","Bayas","MT & MO","[IGNORE]","Caja PC","Exportar & Importar","PKHeX","Posición","Principal","Variocolor","Especie","Tiene apodo","Apodo","Nivel","Naturaleza","Género","Forma","Objeto equipado","Habilidad","Amistad","Idioma","Es huevo","Pokérus","País","[IGNORE]","Región 3DS","Orígenes","Juego origen","[IGNORE]","Poké Ball","Nivel encuentro","Fecha encuentro","Encuentro fatídico","[IGNORE]","Fecha huevo","Estadísticas","IV","EV","Concurso","Movimientos","Movimientos actuales","PP Plus","Reaprender movimientos","Varios","SID","TID","Nombre OT","Último entrenador","[IGNORE]","PC en cualquier lugar","31 cajas desbloqueadas",
        "Varios","Desbloquear monturas","Vista alejada","Caminar y correr rápido","Atravesar paredes","Configurar Sol y Luna","Otro","Renombrar Pokémon","Aprender todas MT/MO","Huevo instantáneo","Eclosión inmediata","Ver IV/EV","Sin contornos","Diálogos rápidos"
    };

    static const vector<string> name_GE = {
        "Festival-Plaza","Aktueller Rang","FC gesammelt","Aktuelle FC","Gäste getroffen","Einrichtungen","Missionen teilgenommen",
        "Kampf","Im Kampf","Teamposition:","Status","Werte","Gesundheit & Mana","Item","Attacken","EP-Multiplikator","Pokémon Infos","Immer kritisch","Erlaube 1+ Mega","Z-Kristall wiederverwenden","Z-Bedingungen ignorieren","Andere","Keine wilden Pokémon","Spawner","Immer Shiny","[IGNORE]","Trainer-Rückkampf","Kein Trainerkampf","Fangrate 100%","Prüfung fangbar","Trainer Pokémon fangen",
        "PKHeX","Trainer","Info","TID & SID","Name","Spielzeit","Spielsprache","Beutel","Geld","Kampfpunkte","Items","Medizin","Beeren","TM & VM","[IGNORE]","PC-Box","Export & Import","PKHeX","Position","Haupt","Shiny","Spezies","Hat Spitzname","Spitzname","Level","Wesen","Geschlecht","Form","Getragenes Item","Fähigkeit","Freundschaft","Sprache","Ist Ei","Pokérus","Land","[IGNORE]","3DS Region","Herkunft","Ursprungsspiel","[IGNORE]","Pokéball","Level erhalten","Datum erhalten","Schicksalhaft","[IGNORE]","Ei Datum","Werte","IV","EV","Wettbewerb","Attacken","Aktuelle Attacken","PP Plus","Attacken lernen","Verschiedenes","SID","TID","OT Name","Letzter Trainer","[IGNORE]","PC überall","31 Boxen freigeschaltet",
        "Verschiedenes","Alle Reittiere freischalten","Zoom Ansicht","Schnell laufen","Durch Wände","Sonne & Mond setzen","Andere","Pokémon umbenennen","Alle TM/VM lernen","Sofort Ei","Sofort schlüpfen","IV/EV anzeigen","Keine Umrisse","Schnelle Dialoge"
    };

    static const vector<string> name_JP = {
        "フェスサークル","現在のランク","集めたFC","現在のFC","交流したゲスト","設置施設","参加ミッション",
        "バトル","戦闘中","手持ち位置:","状態","ステータス","体力とマナ","アイテム","技","経験値倍率","ポケモン情報","常に急所","メガ進化許可","Zクリスタル再使用","Z条件無視","その他","野生ポケモンなし","野生生成","常に色違い","[IGNORE]","トレーナー再戦","トレーナー戦なし","捕獲率100%","試練捕獲可","トレーナーポケモン捕獲",
        "PKHeX","トレーナー","情報","TID & SID","名前","プレイ時間","ゲーム言語","バッグ","お金","BP","アイテム","回復","きのみ","わざマシン","[IGNORE]","ボックス","エクスポート","PKHeX","位置","メイン","色違い","種族","ニックネーム有","ニックネーム","レベル","性格","性別","フォルム","持ち物","特性","なつき度","言語","タマゴ","ポケルス","国","[IGNORE]","3DS地域","出身","元のゲーム","[IGNORE]","ボール","出会いレベル","出会い日","特別遭遇","[IGNORE]","タマゴ日","ステータス","IV","EV","コンテスト","技","現在の技","PPアップ","思い出し","その他","SID","TID","親名","最新トレーナー","[IGNORE]","どこでもPC","31ボックス解放",
        "その他","乗り物解放","ズームアウト","高速移動","壁抜け","サンムーン設定","その他","名前変更","全技習得","即タマゴ","即孵化","IV/EV表示","輪郭なし","高速テキスト"
    };

    // Pointer to current language
    const vector<string>* name = &name_EN;

    void ApplyLanguageToList(f_Language lang) {
        switch (lang) {
            case f_Language::l_English:
                name = &name_EN;
                break;
            case f_Language::l_French:
                name = &name_FR;
                break;
            case f_Language::l_Italian:
                name = &name_IT;
                break;
            case f_Language::l_Spanish:
                name = &name_SP;
                break;
            case f_Language::l_German:
                name = &name_GE;
                break;
            case f_Language::l_Japanese:
                name = &name_JP;
                break;
            default:
                name = &name_EN;
                break;
        }
    }

    void SetLanguage(bool SetInMenu) {
        vector<string> languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese"};
        f_Language initLanguage = f_Language::l_None; // Initialize to "no language"
        int pos;

        File file(PATH_LANGUAGE_SETTINGS);
        string language;

        if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
            LineReader readFile(file);
            readFile(language);

            for (int i = 0; i < languageOption.size(); i++) {
                if (language == languageOption[i]) {
                    currentLanguage = static_cast<Language>(i + 1);
                    initLanguage = static_cast<f_Language>(currentLanguage);
                    ApplyLanguageToList(initLanguage);
                    pos = initLanguage - 1;
                    goto saved;
                }
            }
        }

        if (File::Exists(PATH_LANGUAGE_SETTINGS) == 0)
            File::Create(PATH_LANGUAGE_SETTINGS);

		// If initLanguage is invalid, reset it
        if (initLanguage >= f_Language::l_Undefined)
            initLanguage = f_Language::l_None;

        if (initLanguage == f_Language::l_None || SetInMenu) { // If byte has no mode or re-choose in menu let user choose one
            static const string s_Language = "Choose your language configuration file.\n\nNote: English-only lists (still needs translation): speciesList[], formList[], abilityList[], heldItemList[], movesList[], natureList[], originList[], countryList[], ballList[], iconList[], modelList[]\n\n" << Color(0xFF1C1CFF) << "Red " << "= language file does not exist!\n" << Color(0x2CFF12FF) << "Green " << "= language file does exist!";
            int found = 0;

            if (System::IsCitra()) {
                Sleep(Milliseconds(1000));
                MessageBox("Note: Citra/emulators aren't fully supported yet (need test contributors). Not all codes will work flawlessly.\n\nUse at your own risk. Report bugs/issues on GitHub", DialogType::DialogOk, ClearScreen::Both)();
            }

            // If is missing it gets red if not it gets green
            for (size_t i = 0; i < languageOption.size(); ++i) {
                if (!File::Exists(languageText[i]))
                    languageOption[i] = Color(0xFF1C1CFF) << languageOption[i];

                else {
                    languageOption[i] = Color(0x2CFF12FF) << languageOption[i];
                    found++;
                }
            }

            Keyboard keyboard(s_Language, languageOption);
            keyboard.CanAbort(false);
            Sleep(Milliseconds(1000));
            int selection = keyboard.Open();

            switch (selection) {
                case 0: initLanguage = f_Language::l_English; break;
                case 1: initLanguage = f_Language::l_French; break;
                case 2: initLanguage = f_Language::l_Italian; break;
                case 3: initLanguage = f_Language::l_Spanish; break;
                case 4: initLanguage = f_Language::l_German; break;
                case 5: initLanguage = f_Language::l_Japanese; break;
            }

            currentLanguage = static_cast<Language>(initLanguage);
            ApplyLanguageToList(initLanguage);
            languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese"};

            if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
                LineWriter writeFile(file);
                writeFile << languageOption[selection];
                writeFile.Flush();
                writeFile.Close();
            }
        }

        pos = initLanguage - 1;

        if (pos < 0 || pos >= languageText.size()) {
            OSD::Notify("Invalid language index!");
            pos = 0;
        }

        saved:

        if (!File::Exists(languageText[pos])) {
            Sleep(Milliseconds(1000));
            MessageBox(Utils::Format("You need the correct language text file for the plugin to work. Please download and make sure installation is proper.\nSubmit an issue ticket over at GitHub if there is a bug: %s", GITHUB)).SetClear(ClearScreen::Top)();
            SetLanguage(true); // Redo language choosing
        }

        if (!getLanguage) // Only create once
            getLanguage = new TextFileParser();

        if (!getLanguage->Parse(languageText[pos]))
            OSD::Notify("Failed to load previously selected language file.");
    }

    void Configuration(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("CONFIG_LANGUAGE")};
        Keyboard keyboard(entry->Name(), options);
        int selection = keyboard.Open();

        switch (selection) {
            case 0:
                Keyboard kb(options[selection] + ":", {getLanguage->Get("CONFIG_LANGUAGE_RESET")});
                int resetLanguage = kb.Open();

                if (resetLanguage != -1 && resetLanguage == 0) {
                    if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
                        File::Remove(PATH_LANGUAGE_SETTINGS);
                        MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS") + " " + getLanguage->Get("PLUGIN_HOME_MENU")), DialogType::DialogOk, ClearScreen::Both)();
                        Process::ReturnToHomeMenu();
                    }
                }

                break;
        }
    }

    // This function is called before main and before the game starts. Useful to do code edits safely
    void PatchProcess(FwkSettings &settings) {
        DetectGame(Process::GetTitleID());
        IsUpdateSupported(Process::GetVersion());
    }

    // This function is called when the process exits. Useful to save settings, undo patchs or clean up things
    void OnProcessExit(void);

    void InitMenu(PluginMenu &menu) {
        SetLanguage(false);

        switch (currentLanguage) {
            case ENG: name = &name_EN; break;
            case FRE: name = &name_FR; break;
            case ITA: name = &name_IT; break;
            case SPA: name = &name_SP; break;
            case GER: name = &name_GE; break;
            case JAP: name = &name_JP; break;
            default:  name = &name_EN; break;
        }

        if (name->size() != name_EN.size())
            OSD::Notify("Language list size mismatch!");

        // Use the instance that SetLanguage() parsed | Ensure language file was parsed before using Get()
        if (!getLanguage)
            getLanguage = new TextFileParser();

        MenuFolder *plaza;
        plaza = new MenuFolder((*name)[0]);
        *plaza += new MenuEntry((*name)[1], nullptr, CurrentRank);
        *plaza += new MenuEntry((*name)[2], nullptr, FCCollected);
        *plaza += new MenuEntry((*name)[3], nullptr, CurrentFC);
        *plaza += new MenuEntry((*name)[4], nullptr, GuestsInteracted);
        *plaza += new MenuEntry((*name)[5], nullptr, FacilsHosted);
        *plaza += new MenuEntry((*name)[6], nullptr, MissionsJoined);
        menu += plaza;

        MenuFolder *combat, *inBattle, *encounter;
        combat = new MenuFolder((*name)[7]);
        *combat += new MenuEntry((*name)[9], nullptr, PartyPosition);
        inBattle = new MenuFolder((*name)[8]);
        *inBattle += new MenuEntry((*name)[10], nullptr, StatusCondition);
        *inBattle += new MenuEntry((*name)[11], nullptr, Stats);
        *inBattle += new MenuEntry((*name)[12], nullptr, HealthAndMana);
        *inBattle += new MenuEntry((*name)[13], nullptr, HeldItem);
        *inBattle += new MenuEntry((*name)[14], nullptr, Moves);
        *combat += inBattle;
        *combat += new MenuEntry((*name)[15], nullptr, ExpMultiplier);
        *combat += HotkeyEntry(new MenuEntry((*name)[16], nullptr, ViewPokemonInfo, ""), {Key::Start, ""});
        *combat += new MenuEntry((*name)[17], AlwaysCritHit);
        *combat += new MenuEntry((*name)[18], AllowMultipleMega);
        *combat += new MenuEntry((*name)[19], ReuseZCrystal);
        *combat += new MenuEntry((*name)[20], IgnoreZCrystalReq);
        encounter = new MenuFolder((*name)[21]);
        *encounter += new MenuEntry((*name)[22], NoWildPokemon);
        *encounter += new MenuEntry((*name)[23], nullptr, WildSpawner);
        *encounter += new MenuEntry((*name)[24], AlwaysShiny);
        // *encounter += new MenuEntry((*name)[25]);
        *encounter += new MenuEntry((*name)[26], TrainerRematch);
        *encounter += new MenuEntry((*name)[27], NoTrainerBattle);
        *encounter += new MenuEntry((*name)[28], CaptureRate);
        *encounter += new MenuEntry((*name)[29], TrialIsCatchable);
        *encounter += new MenuEntry((*name)[30], CatchTrainerPokemon);
        *combat += encounter;
        menu += combat;

        MenuFolder *pkhex, *trainer, *info, *bag, *boxes, *editor;
        pkhex = new MenuFolder((*name)[31]);
        trainer = new MenuFolder((*name)[32]);
        info = new MenuFolder((*name)[33]);
        *info += new MenuEntry((*name)[34], nullptr, Identity);
        *info += new MenuEntry((*name)[35], nullptr, InGameName);
        *info += new MenuEntry((*name)[36], nullptr, PlayTime);
        *info += new MenuEntry((*name)[37], nullptr, GameLanguage);
        *trainer += info;
        bag = new MenuFolder((*name)[38]);
        *bag += new MenuEntry((*name)[39], nullptr, Money);
        *bag += new MenuEntry((*name)[40], nullptr, BattlePoints);
        *bag += new MenuEntry((*name)[41], nullptr, Items);
        *bag += new MenuEntry((*name)[42], nullptr, Medicines);
        *bag += new MenuEntry((*name)[43], nullptr, Berries);
        *bag += new MenuEntry((*name)[44], nullptr, KeyItems);
        *trainer += bag;
        // *trainer += new MenuEntry((*name)[45]);
        *pkhex += trainer;

        boxes = new MenuFolder((*name)[46]);
        *boxes += new MenuEntry((*name)[47], nullptr, ExportImport);
        editor = new MenuFolder((*name)[48]);
        *editor += new MenuEntry((*name)[49], nullptr, PKHeX::Position);
        *editor += new MenuFolder((*name)[50], vector<MenuEntry *>({
            new MenuEntry((*name)[51], nullptr, PKHeX::Shiny),
            new MenuEntry((*name)[52], nullptr, PKHeX::Species),
            new MenuEntry((*name)[53], nullptr, PKHeX::IsNicknamed),
            new MenuEntry((*name)[54], nullptr, PKHeX::Nickname),
            new MenuEntry((*name)[55], nullptr, PKHeX::Level),
            new MenuEntry((*name)[56], nullptr, PKHeX::Nature),
            new MenuEntry((*name)[57], nullptr, PKHeX::Gender),
            new MenuEntry((*name)[58], nullptr, PKHeX::Form),
            new MenuEntry((*name)[59], nullptr, PKHeX::HeldItem),
            new MenuEntry((*name)[60], nullptr, PKHeX::Ability),
            new MenuEntry((*name)[61], nullptr, PKHeX::Friendship),
            new MenuEntry((*name)[62], nullptr, PKHeX::Language),
            new MenuEntry((*name)[63], nullptr, PKHeX::IsEgg),
            new MenuEntry((*name)[64], nullptr, PKHeX::Pokerus),
            new MenuEntry((*name)[65], nullptr, PKHeX::Country),
            // new MenuEntry((*name)[66], nullptr),
            new MenuEntry((*name)[67], nullptr, PKHeX::ConsoleRegion),
        }));
        *editor += new MenuFolder((*name)[68], vector<MenuEntry *>({
            new MenuEntry((*name)[69], nullptr, PKHeX::OriginGame),
            // new MenuEntry((*name)[70], nullptr),
            new MenuEntry((*name)[71], nullptr, PKHeX::Ball),
            new MenuEntry((*name)[72], nullptr, PKHeX::MetLevel),
            new MenuEntry((*name)[73], nullptr, PKHeX::MetDate),
            new MenuEntry((*name)[74], nullptr, PKHeX::IsFatefulEncounter),
            // new MenuEntry((*name)[75], nullptr),
            new MenuEntry((*name)[76], nullptr, PKHeX::EggMetDate),
        }));
        *editor += new MenuFolder((*name)[77], vector<MenuEntry *>({
            new MenuEntry((*name)[78], nullptr, PKHeX::IV),
            new MenuEntry((*name)[79], nullptr, PKHeX::EV),
            new MenuEntry((*name)[80], nullptr, PKHeX::Contest),
        }));
        *editor += new MenuFolder((*name)[81], vector<MenuEntry *>({
            new MenuEntry((*name)[82], nullptr, PKHeX::CurrentMove),
            new MenuEntry((*name)[83], nullptr, PKHeX::PPUp),
            new MenuEntry((*name)[84], nullptr, PKHeX::RelearnMove),
        }));
        *editor += new MenuFolder((*name)[85], vector<MenuEntry *>({
            new MenuEntry((*name)[86], nullptr, PKHeX::SID),
            new MenuEntry((*name)[87], nullptr, PKHeX::TID),
            new MenuEntry((*name)[88], nullptr, PKHeX::OTName),
            new MenuEntry((*name)[89], nullptr, PKHeX::LatestHandler),
            // new MenuEntry((*name)[90], nullptr),
        }));
        *boxes += editor;
        *boxes += new MenuEntry((*name)[91], PCAnywhere);
        *boxes += new MenuEntry((*name)[92], nullptr, BoxesUnlocked);
        *pkhex += boxes;
        menu += pkhex;

        MenuFolder *misc, *other;
        misc = new MenuFolder((*name)[93]);
        *misc += new MenuEntry((*name)[94], nullptr, UnlockEveryMount);
        *misc += new MenuEntry((*name)[95], ZoomedOutView);
        *misc += new MenuEntry((*name)[96], FastWalkRun);
        *misc += HotkeyEntry(new MenuEntry((*name)[97], WalkThroughWalls), {Key::Start, ""});
        *misc += new MenuEntry((*name)[98], nullptr, SetSunMoon);
        other = new MenuFolder((*name)[99]);
        *other += new MenuEntry((*name)[100], RenameAnyPokemon);
        *other += new MenuEntry((*name)[101], LearnAnyTeachable);
        *other += new MenuEntry((*name)[102], InstantEgg);
        *other += new MenuEntry((*name)[103], InstantEggHatch);
        *misc += other;
        *misc += HotkeyEntry(new MenuEntry((*name)[104], ViewValuesInSummary, ""), {Hotkey(Key::L, ""), Hotkey(Key::Start, "")});
        *misc += new MenuEntry((*name)[105], NoOutlines);
        *misc += new MenuEntry((*name)[106], FastDialogs);
        menu += misc;

        menu += new MenuEntry(getLanguage->Get("PLUGIN_SETTING"), nullptr, Configuration);
    }

    int main(void) {
        PluginMenu *menu = new PluginMenu("Alolan CTRPluginFramework", 0, 1, 2, "Alolan CTRPluginFramework is a 3gx plugin for Pokemon Sun, Moon, Ultra Sun, and Ultra Moon on the Nintendo 3DS console. This plugin is more so a continuation of ultraSuMoFramework with PKHeX implementations with an updated .3gx file format.\n\nRepository:\ngithub.com/biometrix76/AlolanCTRPluginFramework");
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