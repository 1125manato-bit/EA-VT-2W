; EA VT-2W Windows Installer Script
; EMU AUDIO

[Setup]
AppName=EA VT-2W
AppVersion=1.0.0
DefaultDirName={commoncf}\VST3\EMU AUDIO
DefaultGroupName=EMU AUDIO
OutputBaseFilename=EA_VT-2W_Installer_Windows
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "..\build\EA_VT_2W_artefacts\Release\VST3\EA VT-2W.vst3\*"; DestDir: "{commoncf}\VST3\EA VT-2W.vst3"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\EA VT-2W"; Filename: "{commoncf}\VST3\EA VT-2W.vst3\Contents\x86_64-win\EA VT-2W.vst3"

[Messages]
WelcomeLabel2=This will install EA VT-2W on your computer.
