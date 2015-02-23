; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3AC6F3EF-BE1C-4918-8DD1-010DD0A423A0}
AppName=ProtonClassicSuite
AppVersion=200215
;AppVerName=ProtonClassicSuite 060115
AppPublisher=Laboratoire de Recherche pour le Développement Local
AppPublisherURL=http://gipilab.org
AppSupportURL=http://gipilab.org
AppUpdatesURL=http://gipilab.org
DefaultDirName={pf}\ProtonClassicSuite
DefaultGroupName=ProtonClassicSuite
OutputDir=C:\Documents and Settings\Thibault\Mes documents
OutputBaseFilename=ProtonClassicSuite_200215b_setup
SolidCompression=yes
UninstallDisplayName=ProtonClassicSuite
ShowLanguageDialog=auto
Compression=lzma2/ultra
InternalCompressLevel=ultra

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Documents and Settings\Thibault\Mes documents\ProtonClassicSuite.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Thibault\Mes documents\Build_5.4.0\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\ProtonClassicSuite"; Filename: "{app}\ProtonClassicSuite.exe"
Name: "{commondesktop}\ProtonClassicSuite"; Filename: "{app}\ProtonClassicSuite.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\ProtonClassicSuite.exe"; Description: "{cm:LaunchProgram,ProtonClassicSuite}"; Flags: nowait postinstall skipifsilent

