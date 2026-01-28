#!/bin/bash
set -e
set -o pipefail

if command -v zenity &> /dev/null; then
    gui_mode=true
else
    gui_mode=false
    echo "Zenity not found. Running in text mode."
fi

progress() {
    local percent=$1
    local message=$2
    if [ "$gui_mode" = true ]; then
        echo "$percent"
        echo "# $message"
    else
        echo "[$percent%] $message"
    fi
}

install_steps() {
    progress 10 "Initializing..."
    VST3_DIR="$HOME/.vst3"
    CLAP_DIR="$HOME/.clap"

    mkdir -p "$VST3_DIR" "$CLAP_DIR"
    
    local count=0

    if [ -d "TrebleMaker.vst3" ]; then
        progress 30 "Installing VST3..."
        rm -rf "$VST3_DIR/TrebleMaker.vst3"
        cp -r "TrebleMaker.vst3" "$VST3_DIR/"
        ((count++))
    fi


    if [ -f "TrebleMaker.clap" ]; then
        progress 80 "Installing CLAP..."
        cp "TrebleMaker.clap" "$CLAP_DIR/"
        ((count++))
    fi

    progress 100 "Finishing..."
    
    if [ "$count" -eq 0 ]; then
        echo "ERR:NO_FILES"
        return 1
    fi
}

if [ "$gui_mode" = true ]; then
    if output=$(install_steps | zenity --progress --title="Installing TrebleMaker" --auto-close --no-cancel 2>&1); then
        if [[ "$output" == *"ERR:NO_FILES"* ]]; then
             zenity --error --text="No plugin files found.\nRun this script from the released folder."
             exit 1
        else
             zenity --info --text="Installation Complete!\nYou may need to rescan plugins in your DAW." --title="Success"
        fi
    else
        exit 1
    fi
else
    if ! install_steps | grep -v "ERR:NO_FILES"; then
        echo "Error: No plugin files found. Check your directory."
        exit 1
    fi
    echo "Success! Installation Complete."
fi
