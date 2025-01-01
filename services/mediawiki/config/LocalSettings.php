<?php
if (!defined('MEDIAWIKI')) {
	die('Not an entry point.');
}

// set environment locale
setlocale(LC_ALL, 'en_US.UTF-8');

// set error reporting
error_reporting(E_ALL);

// config utilities
$xvConfigDirectory = __DIR__;
require_once "$xvConfigDirectory/common/ConfigTypes.php";
require_once "$xvConfigDirectory/common/ConfigUtils.php";

// read site list
$xvWikis = xvLoadJson('sites.json');

// extract wiki ID
if (defined('MW_DB')) {
	$xvWikiID = MW_DB;
	$xvMaintScript = true;
} else if ($_SERVER['MW_WIKI'] ?? false) {
	$xvWikiID = $_SERVER['MW_WIKI'];
	$xvMaintScript = false;
} else {
	die('Unknown wiki.');
}

$xvServerName = $xvWikis[$xvWikiID]['domain'];
$xvHttpHost = $_SERVER['HTTP_HOST'] ?? $xvServerName;

// development mode
$xvDebug = boolval($_SERVER['MW_DEBUG'] ?? false)
	|| boolval($_SERVER['HTTP_X_XENS_WIKIS_DEBUG'] ?? false)
	|| $xvMaintScript;
if ($xvDebug) {
	header('X-Xens-Wikis-Debug: true');
	$wgDevelopmentWarnings = true;
	ini_set('display_errors', true);
}

// request timeouts
if (PHP_SAPI === 'cli')
	$wgRequestTimeLimit = 0;
elseif ($xvMaintScript)
	$wgRequestTimeLimit = 86400;
elseif ($_SERVER['REQUEST_METHOD'] === 'POST')
	$wgRequestTimeLimit = 200;
else
	$wgRequestTimeLimit = 60;

// user agent
ini_set('user_agent', "Xens Wikis, $xvWikiID (op@xvnet0.eu.org)");

// force to use default messages in maintenance scripts
if ($xvMaintScript)
	$wgUseDatabaseMessages = false;

require_once '/srv/secrets/mw/Secrets.php';

// read only mode
if (boolval($_SERVER['HTTP_X_XENS_WIKIS_READ_ONLY'] ?? false)) {
	header('X-Xens-Wikis-Read-Only: true');
	$wgReadOnly = 'X-Xens-Wikis-Read-Only';
}
if (PHP_SAPI === 'cli')
	$wgReadOnly = null;
if ($wgReadOnly != null)
	$wgIgnoreImageErrors = true;

require_once "$xvConfigDirectory/common/GlobalDefaults.php";
require_once "$xvConfigDirectory/sites/SiteSettings.$xvWikiID.php";
