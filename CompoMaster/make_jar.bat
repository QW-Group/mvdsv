@rem $Id: make_jar.bat,v 1.2 2006/11/27 15:15:39 vvd0 Exp $

@echo off
set OUT_DIR=bin
set OUT_DIR_FULL=..\%OUT_DIR%\
set IN_DIR=source
rd /S /Q %OUT_DIR%
cd %IN_DIR%
set OUT_NIL_DIR=nilzorlib\diverse\
mkdir %OUT_DIR_FULL%META-INF
mkdir %OUT_DIR_FULL%%OUT_NIL_DIR% > nul
copy META-INF\MANIFEST.MF.Viewer %OUT_DIR_FULL%META-INF\MANIFEST.MF
copy %OUT_NIL_DIR%*.class %OUT_DIR_FULL%%OUT_NIL_DIR% > nul

echo.
echo =============== Compiling com\borland\jbcl\layout ===============
javac -d %OUT_DIR_FULL% com\borland\jbcl\layout\XYConstraints.java com\borland\jbcl\layout\XYLayout.java

echo.
echo =============== Compiling Data ===============
javac -d %OUT_DIR_FULL% Data\Comparable.java Data\CupStructure.java Data\Data.java Data\DataInterface.java Data\DeathMatch.java Data\Demo.java Data\DMRound.java Data\DoubleRound.java Data\EditListener.java Data\FDECup.java Data\FFAGroup.java Data\FFAGroups.java Data\Group.java Data\GroupStructure.java Data\LeagueGroup.java Data\LeagueGroups.java Data\LoserOf.java Data\Match.java Data\MatchClickListener.java Data\MatchDetails.java Data\MatchList.java Data\MatchPlacingInfo.java Data\MatchPlayer.java Data\NetInterface.java Data\Player.java Data\ProgressIndicator.java Data\Rankable.java Data\RankOf.java Data\RealPlayer.java Data\Team.java Data\Walkover.java Data\WinnerOf.java

echo.
echo =============== Compiling CompoViewer ===============
javac -d %OUT_DIR_FULL% CompoViewer\CompoDrawing.java CompoViewer\CompoSelecter.java CompoViewer\CompoViewer.java CompoViewer\EditInvoker.java

cd %OUT_DIR_FULL%
pkzip25 -Add -Maximum -Directories CompoViewer *.*
move CompoViewer.zip ..\CompoViewer.jar

cd ..\%IN_DIR%
copy META-INF\MANIFEST.MF %OUT_DIR%META-INF

echo.
echo =============== Compiling Data ===============
javac -d %OUT_DIR_FULL% Data\BufferedReaderTap.java Data\DataInterface.java Data\NetInterface.java

echo.
echo =============== Compiling CompoMaster ===============
javac -Xlint:unchecked -d %OUT_DIR_FULL% CompoMaster\AdminFrame.java CompoMaster\CmpFilter.java CompoMaster\CompoMaster.java CompoMaster\HtmlCode.java CompoMaster\JCompoDrawing.java CompoMaster\Menu.java CompoMaster\MultipleSlider.java CompoMaster\ScoreRegDM.java CompoMaster\ScoreRegFFA.java CompoMaster\SignupAdmin.java CompoMaster\TextFieldLimiter.java

copy META-INF\MANIFEST.MF %OUT_DIR_FULL%META-INF\MANIFEST.MF
copy cm_icon.gif %OUT_DIR_FULL%

cd %OUT_DIR_FULL%
pkzip25 -Add -Maximum -Directories CompoMaster *.*
move CompoMaster.zip ..\CompoMaster.jar

cd ..