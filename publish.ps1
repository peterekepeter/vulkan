
if (-Not(Test-Path -Path ".\Release\*.exe"))
{
    Write-Error "You should first build the project with release configuration"
    exit 1
}

$OutDir = ".\publish\"
$OutBin = ".\publish\.bin"
if (Test-Path -Path $OutDir)
{
    Remove-Item $OutDir -Recurse -Force
}

New-Item -Path $OutDir -ItemType Directory

# gather data folders
Copy-Item -Path ".\data\*" -Destination $OutDir -Recurse
$DataFolders = Get-ChildItem -Path $OutDir


# gather binaries
New-Item -Path $OutBin -ItemType Directory
Copy-Item -Path ".\Release\*.exe" -Destination $OutBin
Copy-Item -Path ".\submodule\simple-audio\bass24\*.dll" -Destination $OutBin

# build and publish each data folder
for ($i=0; $i -lt $DataFolders.Count; $i++) {
    $OriginalDir = $PWD
    $DataFolderItem = $DataFolders[$i];
    $ExeName = $DataFolderItem.Name + ".exe";
    Write-Output "$ExeName"
    $DataFolder = $DataFolderItem.FullName
    Set-Location $DataFolder
    $BuildScript = ".\build.bat";
    if (Test-Path -Path $BuildScript)
    {
        cmd.exe /c $BuildScript
        Remove-Item $BuildScript
        Remove-Item "*.frag"
        Remove-Item "*.vert"
    }
    Out-File -FilePath ".\fullscreen.bat" -InputObject "$ExeName fullscreen" -Encoding "ASCII"
    Out-File -FilePath ".\fullscreen-1280x720.bat" -InputObject "$ExeName fullscreen 1280 720" -Encoding "ASCII"
    Out-File -FilePath ".\fullscreen-1920x1080.bat" -InputObject "$ExeName fullscreen 1920 1080" -Encoding "ASCII"
    Out-File -FilePath ".\fullscreen-borderless.bat" -InputObject "$ExeName borderless" -Encoding "ASCII"
    Set-Location $OriginalDir;
    Copy-Item -Path "$OutBin\*" -Destination "$DataFolder\"
    Rename-Item "$DataFolder\vulkan.exe" $ExeName
    Compress-Archive -Path "$DataFolder\" -DestinationPath "$DataFolder.zip" -CompressionLevel Optimal 
}