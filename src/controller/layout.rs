//! The layout for the HUD is read from a TOML file. This data is shared between
//! languages the same way that the user settings are. The Rust side reads the
//! toml; the C++ side uses the data in layout. The majority of the implementation
//! is filing in defaults.

use std::fs;
use std::io::Write;
use std::path::PathBuf;
use std::sync::Mutex;

use anyhow::Result;
use once_cell::sync::Lazy;

use crate::plugin::{Color, HudElement, HudLayout, Point, SlotLayout};

static LAYOUT_PATH: &str = "./data/SKSE/Plugins/SoulsyHUD_Layout.toml";

/// There can be only one. Not public because we want access managed.
static LAYOUT: Lazy<Mutex<HudLayout>> = Lazy::new(|| Mutex::new(HudLayout::refresh()));

/// Read our layout data from the file, or fall back to defaults if the file
/// is not present or is invalid TOML.
pub fn layout() -> HudLayout {
    let layout = LAYOUT.lock().unwrap();
    layout.clone()
}

impl HudLayout {
    /// Read a settings object from a toml file.
    pub fn read_from_file() -> Result<Self> {
        let path = std::path::Path::new(LAYOUT_PATH);
        if !path.exists() {
            // No file? We write out defaults.
            let layout = HudLayout::default();
            let buf = toml::to_string(&layout)?;
            let mut fp = fs::File::create(path)?;
            write!(fp, "{buf}")?;
            Ok(HudLayout::default())
        } else if let Ok(buf) = fs::read_to_string(PathBuf::from(LAYOUT_PATH)) {
            match toml::from_str::<HudLayout>(&buf) {
                Ok(v) => {
                    log::info!("successfully read a hud layout file");
                    Ok(v)
                }
                Err(e) => {
                    // We are *not* overwriting a bad TOML file, but we are logging it.
                    log::warn!("bad toml in hud layout; {e:?}");
                    Ok(HudLayout::default())
                }
            }
        } else {
            log::warn!(
                "unable to read any data from {}! falling back to defaults",
                LAYOUT_PATH
            );
            Ok(HudLayout::default())
        }
    }

    /// Refresh the layout from the file, to take an out-of-band update and apply it in-game.
    pub fn refresh() -> HudLayout {
        match HudLayout::read_from_file() {
            Ok(v) => {
                log::info!("successfully refreshed HUD layout");
                v
            }
            Err(e) => {
                log::warn!("Failed to read layout file; continuing with defaults; {e:?}");
                HudLayout::default()
            }
        }
    }
}

impl Default for HudLayout {
    fn default() -> Self {
        // Yes, this is annoyingly complex. Obviously this solution is wrong.
        let power_default = SlotLayout {
            element: HudElement::Power,
            name: "Shouts/Powers".to_string(),
            offset: Point { x: 0.0, y: -125.0 },
            size: Point { x: 125.0, y: 125.0 },
            bg_color: Color::default(),
            icon_color: Color {
                r: 255,
                g: 255,
                b: 120,
                a: 255,
            },
            icon_size: Point { x: 125.0, y: 125.0 },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 10.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point { x: 0.0, y: 0.0 },
            count_font_size: 20.0,
            count_color: Color::default(),
            name_color: Color::default(),
            name_offset: Point { x: 0.0, y: 20.0 },
            name_font_size: 30.0,
        };
        let utility_default = SlotLayout {
            element: HudElement::Utility,
            name: "Consumables".to_string(),
            offset: Point { x: 0.0, y: 125.0 },
            size: Point { x: 125.0, y: 125.0 },
            bg_color: Color::default(),
            icon_color: Color {
                r: 255,
                g: 120,
                b: 120,
                a: 255,
            },
            icon_size: Point { x: 125.0, y: 125.0 },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 10.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point { x: 0.0, y: 0.0 },
            count_font_size: 20.0,
            count_color: Color::default(),
            name_color: Color::default(),
            name_offset: Point { x: 0.0, y: 20.0 },
            name_font_size: 30.0,
        };
        let left_default = SlotLayout {
            element: HudElement::Left,
            name: "Left Hand".to_string(),
            offset: Point { x: -125.0, y: 0.0 },
            size: Point { x: 125.0, y: 125.0 },
            bg_color: Color::default(),
            icon_color: Color {
                r: 120,
                g: 255,
                b: 120,
                a: 255,
            },
            icon_size: Point { x: 125.0, y: 125.0 },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 10.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point { x: 0.0, y: 0.0 },
            count_font_size: 20.0,
            count_color: Color::default(),
            name_color: Color::default(),
            name_offset: Point { x: 0.0, y: 20.0 },
            name_font_size: 30.0,
        };
        let right_default = SlotLayout {
            element: HudElement::Right,
            name: "Right Hand".to_string(),
            offset: Point { x: 125.0, y: 0.0 },
            size: Point { x: 125.0, y: 125.0 },
            bg_color: Color::default(),
            icon_color: Color {
                r: 0,
                g: 255,
                b: 255,
                a: 255,
            },
            icon_size: Point { x: 125.0, y: 125.0 },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 10.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point { x: 0.0, y: 0.0 },
            count_font_size: 20.0,
            count_color: Color::default(),
            name_color: Color::default(),
            name_offset: Point { x: 0.0, y: 20.0 },
            name_font_size: 30.0,
        };
        let ammo_default = SlotLayout {
            element: HudElement::Ammo,
            name: "Ammo".to_string(),
            offset: Point { x: 62.0, y: 62.0 },
            size: Point { x: 62.0, y: 62.0 },
            bg_color: Color::default(),
            icon_color: Color {
                r: 255,
                g: 0,
                b: 255,
                a: 255,
            },
            icon_size: Point { x: 62.0, y: 62.0 },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 10.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point { x: 0.0, y: 0.0 },
            count_font_size: 20.0,
            count_color: Color::default(),
            name_color: Color::default(),
            name_offset: Point { x: 0.0, y: 20.0 },
            name_font_size: 30.0,
        };

        let layouts = vec![
            power_default,
            utility_default,
            left_default,
            right_default,
            ammo_default,
        ];
        Self {
            anchor: Point {
                x: 200.0,
                y: 1300.0,
            },
            size: Point { x: 300.0, y: 300.0 },
            bg_color: Color::default(),
            layouts,
            debug: false,
            animation_alpha: 51,
            animation_duration: 0.1,
        }
    }
}

impl SlotLayout {
    pub fn default_for_element(element: HudElement, name: &str) -> Self {
        Self {
            element,
            name: name.to_string(),
            ..Default::default()
        }
    }
}

impl Default for SlotLayout {
    fn default() -> Self {
        Self {
            element: HudElement { repr: 1 },
            name: "unknown".to_string(),
            offset: Point::default(),
            size: Point { x: 150.0, y: 150.0 },
            bg_color: Color::default(),
            icon_size: Point { x: 150.0, y: 150.0 },
            icon_color: Color {
                r: 255,
                g: 255,
                b: 255,
                a: 125,
            },
            hotkey_color: Color::default(),
            hotkey_offset: Point { x: 20.0, y: 0.0 },
            hotkey_size: Point { x: 30.0, y: 30.0 },
            hotkey_bg_color: Color::default(),
            text_offset: Point::default(),
            count_font_size: 20.0,
            count_color: Color::default(),
            name_offset: Point::default(),
            name_font_size: 20.0,
            name_color: Color::default(),
        }
    }
}

impl Default for Color {
    fn default() -> Self {
        Self {
            r: 255,
            g: 255,
            b: 255,
            a: 255,
        }
    }
}

impl Point {
    pub fn offset_by(&self, offset: Point) -> Point {
        Point {
            x: self.x + offset.x,
            y: self.y + offset.y,
        }
    }
}
