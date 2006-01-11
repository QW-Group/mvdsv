@echo off
cd source
set OUT_DIR=..\bin\
set OUT_NIL_DIR=nilzorlib\diverse\
mkdir %OUT_DIR%META-INF
mkdir %OUT_DIR%%OUT_NIL_DIR% > nul
copy META-INF\MANIFEST.MF %OUT_DIR%META-INF
copy %OUT_NIL_DIR%*.class %OUT_DIR%%OUT_NIL_DIR% > nul
copy cm_icon.gif %OUT_DIR%

echo.
echo =============== Compiling com\borland\jbcl\layout ===============
javac -d %OUT_DIR% com\borland\jbcl\layout\XYConstraints.java com\borland\jbcl\layout\XYLayout.java

echo.
echo =============== Compiling Data ===============
javac -d %OUT_DIR% Data\BufferedReaderTap.java Data\Comparable.java Data\CupStructure.java Data\Data.java Data\DataInterface.java Data\DeathMatch.java Data\Demo.java Data\DMRound.java Data\DoubleRound.java Data\EditListener.java Data\FDECup.java Data\FFAGroup.java Data\FFAGroups.java Data\Group.java Data\GroupStructure.java Data\LeagueGroup.java Data\LeagueGroups.java Data\LoserOf.java Data\Match.java Data\MatchClickListener.java Data\MatchDetails.java Data\MatchList.java Data\MatchPlacingInfo.java Data\MatchPlayer.java Data\NetInterface.java Data\Player.java Data\ProgressIndicator.java Data\Rankable.java Data\RankOf.java Data\RealPlayer.java Data\Team.java Data\Walkover.java Data\WinnerOf.java

echo.
echo =============== Compiling CompoMaster ===============
javac -d %OUT_DIR% CompoMaster\AdminFrame.java CompoMaster\CmpFilter.java CompoMaster\CompoMaster.java CompoMaster\HtmlCode.java CompoMaster\JCompoDrawing.java CompoMaster\Menu.java CompoMaster\MultipleSlider.java CompoMaster\ScoreRegDM.java CompoMaster\ScoreRegFFA.java CompoMaster\SignupAdmin.java CompoMaster\TextFieldLimiter.java

echo.
echo =============== Compiling CompoViewer ===============
javac -d %OUT_DIR% CompoViewer\CompoDrawing.java CompoViewer\CompoSelecter.java CompoViewer\CompoViewer.java CompoViewer\EditInvoker.java

cd %OUT_DIR%
pkzip25 -Add -Maximum -Directories CompoMaster *.*
move CompoMaster.zip ..\CompoMaster.jar