<?php
$wgSitename = "Xensor V Wiki";
$wgMetaNamespace = "XvWiki";

$xvTesting = true;

$wgLanguageCode = 'en';
$wgLocaltimezone = 'UTC';

$wgRightsUrl = "https://creativecommons.org/licenses/by-sa/4.0/";
$wgRightsText = "Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)";
$wgRightsIcon = "$wgResourceBasePath/resources/assets/licenses/cc-by-sa.png";

$xvUseGlobalSkins = false;
xvLoadSkin('Lakeus');
$wgDefaultSkin = $wgDefaultMobileSkin = 'lakeus';

$wgLocalInterwikis[] = 'xvn';

$xvUseEmailConfirmed = true;
$xvRequireEmailConfirmedToEdit = true;

$xvUseCargo = true;

require_once "$xvConfigDirectory/common/GlobalSettings.php";
