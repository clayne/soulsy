set windows-shell := ["pwsh.exe", "-Command"]
set shell := ["bash", "-uc"]
set dotenv-load := true

SPRIGGIT := "~/bin/spriggit"
TESTMOD := "/mnt/g/MO2Skyrim/SoulsyHUD fomod test/"

# List available recipes.
help:
    just -l

# Build everything from a clean repo. One-stop shop.
full-build: tools cmake build archive

# Install required tools.
@tools:
    rustup install nightly
    cargo install cargo-nextest
    cargo install tomato-toml

# Run initial cmake to generate project files. Requires Windows.
cmake:
    cmake --preset vs2022-windows

# Use cargo & cmake to compile the mod in release mode. Requires Windows.
@build:
    # if (test-path build/Release/SoulsyHUD.dll) { rm build/Release/SoulsyHUD.dll }
    cargo build --release
    cmake --build --preset vs2022-windows --config Release

# Format both Rust & C++. Can run anywhere.
@format:
    cargo +nightly fmt
    find src -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i

# Clippy.
@lint:
	cargo clippy --all-targets --no-deps

# Run rust tests. Cannot run on Windows (yet; use Mac or WSL Ubuntu for now).
@test:
    cargo nextest run -E 'not test(/.*pack_complete/)'

# Run icon checks.
@test-icons:
	cargo nextest run -- soulsy_pack_complete thicc_pack_complete

# Generate source files list for CMake. Requires bash. Use a *nix.
sources:
    #!/bin/bash
    set -e
    echo "set(headers \${headers}" > test.txt
    headers=$(find ./src -name \*\.h | sort)
    echo "${headers}" >> test.txt
    echo ")" >> test.txt
    echo "set(sources \${sources}" >> test.txt
    echo "    \${headers}" >> test.txt
    cpps=$(find ./src -name \*\.cpp | sort)
    echo "${cpps}" >> test.txt
    echo ")" >> test.txt
    sed -e 's/^\.\//    /' test.txt > cmake/sourcelist.cmake
    rm test.txt

# Set the crate version and tag the repo to match. Requires bash.
tag VERSION:
    #!/usr/bin/env bash
    set -e
    tomato set package.version {{VERSION}} Cargo.toml
    # update the version header for the plugin
    sed -i -e 's/set(VERSION [0-9][0-9]*\.[0-9]*\.[0-9]*\(\.[0-9]*\)/set(VERSION {{VERSION}}\1/' CMakeLists.txt
    # update the lock file
    #cargo check
    git commit CMakeLists.txt Cargo.toml Cargo.lock -m "v{{VERSION}}"
    git tag "v{{VERSION}}"
    echo "Release tagged for version v{{VERSION}}"

# Copy the built mod files to my test mod.
install:
    #!/usr/bin/env bash
    echo "copying to live mod for testing..."
    outdir="{{TESTMOD}}"
    rsync -a installer/core/ "$outdir"
    cp -p build/Release/SoulsyHUD.{dll,pdb} "$outdir"/SKSE/plugins/

# Copy English translation to other translation files.
translations:
    #!/usr/bin/env bash
    declare -a langs=(czech french german italian japanese polish russian spanish)
    for lang in "${langs[@]}"; do
        cp -p installer/core/Interface/Translations/SoulsyHUD_english.txt installer/core/Interface/Translations/SoulsyHUD_$lang.txt
    done

# check that all $ strings in config have matching translation strings
check-translations:
    #!/usr/bin/env bash
    converted=$(iconv -f utf-16 -t utf-8 data/Interface/Translations/SoulsyHUD_english.txt > tmp.txt)

    # I am too lazy to figure out how to get jq to do all of it.
    keys=$(cat data/mcm/config/SoulsyHUD/config.json | jq '.pages[] | .content[] | .[]' -r | grep "\\$" | tr -d '," $' | sort | uniq)
    for k in $keys; do
        cmd="grep $k tmp.txt"
        suppressed=$(sh -c "$cmd")
        exit=$?
        if [ $exit != '0' ]; then
            echo "missing translation: $k"
        fi
    done
    rm tmp.txt

# Create a mod archive and 7zip it. Requires bash.
[unix]
archive:
    #!/usr/bin/env bash
    set -e
    version=$(tomato get package.version Cargo.toml)
    release_name=SoulsyHUD_v${version}
    mkdir -p "releases/$release_name"
    cp -rp installer/* "releases/${release_name}/"
    cp -p build/Release/SoulsyHUD.dll "releases/${release_name}/core/SKSE/plugins/SoulsyHUD.dll"
    cp -p build/Release/SoulsyHUD.pdb "releases/${release_name}/core/SKSE/plugins/SoulsyHUD.pdb"
    rm "releases/${release_name}/core/scripts/source/TESV_Papyrus_Flags.flg"
    cd releases
    rm -f "$release_name"_fomod.7z
    7z a "$release_name"_fomod.7z "$release_name"
    rm -rf "$release_name"
    cd ..
    echo "Mod archive for v${version} ready at releases/${release_name}.7z"

# Use spriggit to dump the plugin to text.
plugin-ser:
    {{SPRIGGIT}} serialize --InputPath ./data/SoulsyHUD.esl --OutputPath ./plugin/ --GameRelease SkyrimSE --PackageName Spriggit.Json

# Use spriggit to rehydrate the plugin.
@plugin-de:
    {{SPRIGGIT}} deserialize --InputPath ./plugin --OutputPath ./SoulsyHUD_test.esl

# Remove archive files.
@clean:
    rm -rf releases/

# Remove archive files & all build artifacts.
spotless: clean
    cargo clean
    rm -rf build

# The rest of these are stubs so windows doesn't just hork.

[windows]
@archive:
    echo "Run this recipe in a bash shell."
