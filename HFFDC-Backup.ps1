#Adjust Settings here
$HFFDC = "C:\Path\To\HFFDC.exe";
$blockSizeInBytes = "5242880";
$sourceDirectory = "C:\Copy\From";
$targetDirectory = "P:\Copy\To";

#Requires at least powershell 3.0
foreach($file in Get-ChildItem -Path $sourceDirectory -Exclude *.hffdc -Recurse -File)
{
    $sourceFile = $file.FullName;
    $targetFile = $file.FullName.Replace($sourceDirectory, $targetDirectory);
    
    if(Test-Path $targetFile)
    {
        Write-Host "Backup of $file already existing";
        
        if(!(Test-Path "$targetFile.hffdc"))
        {
            Write-Host "No checksum file for target file existing, generating...";
            & $HFFDC --generate-checksum $targetFile $blockSizeInBytes;
        }
        else
        {
            if((Get-Item "$targetFile.hffdc").LastWriteTime -lt (Get-Item $targetFile).LastWriteTime)
            {
                Write-Host "Recreating checksum file for source file...";
                & $HFFDC --generate-checksum $sourceFile $blockSizeInBytes;
            }
            else
            {
                Write-Host "Skipping source file checksum recreation, fresh enough";
            }
        }

        Write-Host "Updating target file...";
        & $HFFDC --checksum-copy $sourceFile $targetFile;
    }
    else 
    {
        Write-Host "No Backup of $file existing";
        Write-Host "Recreating checksum file for source file...";
        & $HFFDC --generate-checksum $sourceFile $blockSizeInBytes;

        Write-Host "Copy source file to destination...";
        $targetFileDirectory = Split-Path -Path $targetFile;
        New-Item -ItemType Directory -Force -Path $targetFileDirectory | Out-Null;
        Copy-Item -Path $sourceFile -Destination $targetFile

        Write-Host "Copy source file checksum file to target destination...";
        Copy-Item -Path "$sourceFile.hffdc" -Destination "$targetFile.hffdc";
    }
}