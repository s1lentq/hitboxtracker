@echo OFF
::
:: Post-build auto-deploy script
:: Create and fill PublishPath.ini file with path to deployment folder
:: I.e. PublishPath.ini should contain one line with a folder path
:: Call it so:
:: IF EXIST "$(ProjectDir)PostBuild.bat" (CALL "$(ProjectDir)PostBuild.bat" "$(TargetDir)" "$(TargetName)" "$(TargetExt)" "$(ProjectDir)")
::

SET targetDir=%~1
SET targetName=%~2
SET targetExt=%~3
SET projectDir=%~4
SET destination=

IF NOT EXIST "%projectDir%\PublishPath.ini" (
	ECHO 	No deployment path specified. Create PublishPath.ini near PostBuild.bat with paths on separate lines for auto deployment.
	exit /B 0
)

FOR /f "tokens=* delims= usebackq eol=;" %%a IN ("%projectDir%\PublishPath.ini") DO (
	ECHO Deploying to: %%a
	IF NOT "%%a" == "" (
		copy /Y "%targetDir%%targetName%%targetExt%" "%%a"
		IF NOT ERRORLEVEL 1 (
			IF EXIST "%targetDir%%targetName%.pdb" (
				copy /Y "%targetDir%%targetName%.pdb" "%%a"
			)
		) ELSE (
			ECHO PostBuild.bat ^(27^) : warning : Can't copy '%targetName%%targetExt%' to deploy path '%%a'
		)
	)
)

IF "%%a" == "" (
	ECHO 	No deployment path specified.
)

exit /B 0