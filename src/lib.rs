pub mod controller;
use controller::*;

/// Rust defines the bridge between it and C++ in this file, using the
/// affordances of the `cxx` crate. At build time `cxx_build` will generate the
/// header files required by the C++ side.
#[cxx::bridge]
pub mod plugin {
    /// This voluminous struct exposes its fields because the UI implementation
    /// frequently refers to them. It is read-only for the C++ side. The values
    /// are filled out by lazily reading the layout toml file.
    #[derive(Deserialize, Serialize, Debug, Clone, Default)]
    pub struct HudLayout {
        /// Enable debug logging.
        debug: bool,
    }

    /// Knowing the icon for the item tells us *almost* everything we need to know
    /// about how to use it.
    #[derive(Serialize, Deserialize, Debug, Clone, PartialEq, Eq)]
    pub enum EntryIcon {
        Alteration,
        ArmorClothing,
        ArmorHeavy,
        ArmorLight,
        Arrow,
        AxeOneHanded,
        AxeTwoHanded,
        Bow,
        Claw,
        Conjuration,
        Crossbow,
        Dagger,
        DefaultPotion,
        DestructionFire,
        DestructionFrost,
        DestructionShock,
        Destruction,
        Food,
        Halberd,
        HandToHand,
        IconDefault,
        Illusion,
        Katana,
        Mace,
        Pike,
        PoisonDefault,
        PotionFireResist,
        PotionFrostResist,
        PotionHealth,
        PotionMagicka,
        PotionShockResist,
        PotionStamina,
        Power,
        QuarterStaff,
        Rapier,
        Restoration,
        Scroll,
        Shield,
        Shout,
        SpellDefault,
        Staff,
        SwordOneHanded,
        SwordTwoHanded,
        Torch,
        Whip,
    }

    /// Turning the key number into an enum is handy.
    #[derive(Debug, Clone)]
    pub enum Action {
        /// We do not need to do anything, possibly because the key was not one of our hotkeys.
        Irrelevant,
        /// We're acting on the power/shouts hotkey.
        Power,
        /// The left-hand cycle hotkey.
        Left,
        /// The right-hand cycle hotkey.
        Right,
        /// The utility-item cycle hotkey.
        Utility,
        /// The activate-utility-item hotkey.
        Activate,
        /// The HUD toggle hotkey.
        ShowHide,
    }

    /// I would rather not use exceptions for normal flow control.
    #[derive(Debug, Clone, PartialEq, Eq)]
    pub enum MenuEventResponse {
        Okay,
        Unhandled,
        Error,
        ItemAdded,
        ItemRemoved,
        ItemInappropriate,
        TooManyItems,
        // other responses/errors?
    }

    /// This struct passes data from controller to C++ to signal if it should
    /// start or stop a timer, and if so which timer. For complicated reasons,
    /// timers on the Rust side are impractical (they need to be async) and so I
    /// am doing them on the  C++ side. A better solution would be nice.
    #[derive(Debug, Clone, PartialEq, Eq)]
    pub struct KeyEventResponse {
        /// Did we handle this keypress?
        handled: bool,
        /// Do we need to start a timer?
        start_timer: Action,
        /// Do we need to stop a timer?
        stop_timer: Action,
    }

    extern "Rust" {
        // Zero or more opaque types which both languages can pass around
        // but only Rust can see the fields.

        /// Give access to the settings to the C++ side.
        type UserSettings;
        fn is_cycle_button(self: &UserSettings, key: u32) -> bool;
        /// Managed access to the settings object, so we can lazy-load if necessary.
        fn user_settings() -> Box<UserSettings>;

        /// This is an entry in the cycle. The UI will ask questions of it.
        type CycleEntry;
        fn icon_file(self: &CycleEntry) -> String;
        fn create_cycle_entry(
            kind: EntryIcon,
            two_handed: bool,
            has_count: bool,
            count: usize,
            name: &str,
            form_string: &str,
        ) -> Box<CycleEntry>;

        /// Managed access to the layout object, so we can lazy-load if necessary.
        fn layout() -> &'static HudLayout;
        /// After an MCM-managed change, re-read our .ini file.
        fn refresh_user_settings() -> Result<()>;
        /// Handle an incoming key press event from the game. Returns true if handled.
        fn handle_key_event(key: u32, button: &ButtonEvent) -> KeyEventResponse;
        /// Handle an in-menu event (which adds/removes items) from the game.
        fn handle_menu_event(key: u32, item: &CycleEntry) -> MenuEventResponse;
    }

    unsafe extern "C++" {
        /// Advertise to CXX what our C++ is supposed to look like.
        include!("PCH.h");
        include!("HeadersForRust.hxx"); // we don't want this handed to CMake, so we name it oddly.

        // everything in the RE namespace is from CommonLibSE
        // I can imagine auto-generating a complete bridge at some point.

        #[namespace = "RE"]
        type TESForm;
        #[namespace = "RE"]
        type BGSEquipSlot;
        #[namespace = "RE"]
        type ButtonEvent;
        #[namespace = "RE"]
        fn IsDown(self: &ButtonEvent) -> bool;
        fn IsUp(self: &ButtonEvent) -> bool;

        // the UI renderer
        #[namespace = "ui::ui_renderer"]
        fn set_fade(do_fade: bool, alpha: f64);
        #[namespace = "ui::ui_renderer"]
        fn get_fade() -> bool;
        #[namespace = "ui::ui_renderer"]
        fn toggle_show_ui();

        #[namespace = "inventory_item"]
        fn is_two_handed(item: &TESForm) -> bool;

        #[namespace = "helpers"]
        fn notify_player(message: &str);
    }
}
