#define MyAppName "TrebleMaker"
#define MyAppPublisher "LeoCodes"
#define MyAppURL "https://github.com/leonardonapoless/treblemaker"

[Setup]
AppId={{E6805872-9E95-46B4-82D1-344422501000}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
OutputBaseFilename=TrebleMaker_Windows_Installer_{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 (Folder)
Source: "Builds\Windows\TrebleMaker_artefacts\Release\VST3\TrebleMaker.vst3\*"; DestDir: "{commoncf64}\VST3\TrebleMaker.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

; CLAP (Single File)
Source: "Builds\Windows\TrebleMaker_artefacts\Release\CLAP\TrebleMaker.clap"; DestDir: "{commoncf64}\CLAP"; Flags: ignoreversion skipifsourcedoesntexist

[Messages]
SetupAppTitle=TrebleMaker Installer
SetupWindowTitle=TrebleMaker Installer