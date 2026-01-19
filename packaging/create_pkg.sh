#!/bin/bash

# EA VT-2W macOS Installer Generation Script
# EMU AUDIO

APP_NAME="EA VT-2W"
BUNDLE_ID="com.emuaudio.ea-vt2w"
VERSION="1.0.0"

# パス設定
BUILD_DIR="../build/EA_VT_2W_artefacts/Release"
PKG_ROOT="mac_root"
SCRIPTS_DIR="mac_scripts"
OUTPUT_PKG="EA_VT-2W_Installer_macOS.pkg"

echo "========================================"
echo "  Generating macOS Installer (.pkg)"
echo "========================================"

# クリーンアップ
rm -rf "$PKG_ROOT/Library"
mkdir -p "$PKG_ROOT/Library/Audio/Plug-Ins/Components"
mkdir -p "$PKG_ROOT/Library/Audio/Plug-Ins/VST3"

# ビルド成果物のコピー
echo "Copying plugins to package root..."
cp -R "$BUILD_DIR/AU/$APP_NAME.component" "$PKG_ROOT/Library/Audio/Plug-Ins/Components/"
cp -R "$BUILD_DIR/VST3/$APP_NAME.vst3" "$PKG_ROOT/Library/Audio/Plug-Ins/VST3/"

# インストーラー作成
echo "Building package..."
pkgbuild --root "$PKG_ROOT" \
         --identifier "$BUNDLE_ID" \
         --version "$VERSION" \
         --install-location "/" \
         --scripts "$SCRIPTS_DIR" \
         "$OUTPUT_PKG"

echo "----------------------------------------"
echo "Done! Installer created: $OUTPUT_PKG"
echo "----------------------------------------"
echo "Note: To distribute this, you must sign it and notarize it."
