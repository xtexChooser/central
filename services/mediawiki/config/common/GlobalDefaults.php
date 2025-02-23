<?php

// Emergeny Switches
$xvEmergAbuse = false;
$xvEmergSecLockdown = false;

// Testing wikis
$xvTesting = false;

// Paths
$wgServer = $wgCanonicalServer = 'https://' . $xvServerName;
$wgScript = '/';
$wgScriptPath = '';
$wgUsePathInfo = true;
$wgArticlePath = '/w/$1';
$wgMainPageIsDomainRoot = true;

// Database
$xvUseDatabaseMonto = true;

// Disable PHP session
$wgPHPSessionHandling = 'disable';

// Default Language
$wgLanguageCode = 'qqx';
$wgLocaltimezone = 'UTC';

// Global Extensions
$xvUseGlobalExtensions = true;
$xvGlobalExtensions = [
	'AbuseFilter',
	'AdvancedSearch',
	'AntiSpoof',
	'BetaFeatures',
	'CategoryTree',
	'CategoryWatch',
	'CheckUser',
	'Cite',
	'CiteThisPage',
	'cldr',
	'CodeEditor',
	'CodeMirror',
	'Description2',
	'DiscussionTools',
	'DynamicPageList3',
	'Echo',
	'Gadgets',
	'ImageMap',
	'InputBox',
	'Interwiki',
	'JsonConfig',
	'Linter',
	'LoginNotify',
	'Loops',
	'MassMessage',
	'Math',
	'MultimediaViewer',
	'NoTitle',
	'Nuke',
	'OATHAuth',
	'OAuth',
	'PageImages',
	'ParserFunctions',
	'PdfHandler',
	'Popups',
	'ProtectSite',
	'RealMe',
	'RevisionSlider',
	'Scribunto',
	'SecureLinkFixer',
	'SpamBlacklist',
	'SyntaxHighlight_GeSHi',
	'TemplateData',
	'TemplateSandbox',
	'TemplateStyles',
	'TemplateWizard',
	'TextExtracts',
	'ThrottleOverride',
	'TitleBlacklist',
	'TitleKey',
	'TwoColConflict',
	'WikiEditor',
];
xvLoadExtension('XensTweaks'); // always load XensTweaks

// Global Skins
$xvUseGlobalSkins = true;
$xvGlobalSkins = [
	'Vector',
	'MinervaNeue',
	'Lakeus',
];
$wgDefaultSkin = 'vector-2022';

// Rate Limits
$wgRateLimits['purge'] = [
	'ip' => [30, 60],
	'user' => [30, 30],
];
$wgRateLimits['linkpurge'] = [
	'ip' => [30, 60],
	'user' => [30, 30],
];
$wgRateLimits['renderfile-nonstandard'] = [
	'ip' => [70, 30],
	'user' => [100, 30],
];
$wgRateLimits['badcaptcha'] = [
	'ip' => [10, 600],
	'newbie' => [15, 300],
	'user' => [30, 300],
];
$wgRateLimits['sendemail'] = [
	'ip' => [5, 86400],
	'newbie' => [5, 86400],
	'user' => [20, 86400],
	'bureaucrat' => [20, 86400],
];

// CAPTCHA
$xvUseCaptcha = true;
$xvCaptchaType = XvCaptchaType::Turnstile;
$xvSkipCaptchaForAutoconfirmed = true;
$xvCaptchaOnEdit = true;

// VisualEditor
$xvUseVisualEditor = true;
$xvUseVisualEditorAsDefault = true;
$xvVisualEditorNamespaces = ['User', 'File', 'Category', 'Project'];

// MobileFrontend
$xvUseMobileFrontend = true;
$wgDefaultMobileSkin = 'minerva';

// Misc
$wgNamespacesWithSubpages[NS_MAIN] = true;
$wgAllowCrossOrigin = true;
$wgAllowUserCss = true;
$wgAllowUserJs = true;
$wgAllowSiteCSSOnRestrictedPages = true;
$wgDiffEngine = 'wikidiff2';
$wgUseImageMagick = false;
$wgEnableEditRecovery = true;
$wgDefaultUserOptions['editrecovery'] = 1;
$wgUseRCPatrol = true;
$wgUseNPPatrol = true;
$wgUseFilePatrol = true;
$wgFragmentMode = ['html5'];
$wgRestrictDisplayTitle = false;

$wgConditionalUserOptions['showhiddencats'] = [
	[1, [CUDCOND_USERGROUP, 'sysop']],
	[1, [CUDCOND_USERGROUP, 'staff']],
	[1, [CUDCOND_USERGROUP, 'interface-admin']],
];

// Cache
$wgMainCacheType = CACHE_ACCEL;

// User rights - staff
xvGrantPermissionsTo('staff', [
	'interwiki',
	'editinterfacesite',
	'userrights',
	'ipblock-exempt',
	'patrolmarks',
	'globalblock',
	'createaccount',
	'noratelimit',
	'renameuser',
	'renameuser-global',
]);
$wgAddGroups['staff'] = true;
$wgRemoveGroups['staff'] = true;

// User rights - bureaucrat
xvGrantPermissionsTo('bureaucrat', [
	'interwiki',
	'userrights',
	'ipblock-exempt',
]);
xvRemovePermissionsFrom('bureaucrat', [
	'renameuser',
]);
$wgAddGroups['bureaucrat'] = array_diff(array_keys($wgGroupPermissions), [
	'staff',
	'suppress',
	'steward',
	'checkuser'
]);
$wgRemoveGroups['bureaucrat'] = $wgAddGroups['bureaucrat'];

// User rights - sysop
xvGrantPermissionsTo('sysop', [
	'userrights',
	'patrolmarks',
]);
$wgAddGroups['sysop'] = ['bot', 'interface-admin'];
$wgRemoveGroups['sysop'] = ['sysop', 'bot', 'autoconfirmed', 'interface-admin'];

// User rights - autoconfirmed
$wgAutoConfirmCount = 10;
$wgAutoConfirmAge = 259200; // 3 days

// Shared Uploads
$wgUseSharedUploads = true;
$xvUseWikimediaCommons = true;
$xvUseMirahezeCommons = false;

// Email
$wgEnableEmail = true;
$wgSMTP = [
	'host' => 'smtp-relay.brevo.com',
	'IDHost' => 'w.xvnet.eu.org',
	'localhost' => $xvServerName,
	'port' => 587,
	'auth' => true,
	'username' => $xvSMTPUsername,
	'password' => $xvSMTPPassword
];
$wgEmergencyContact = $xvOpContactEmail;
$wgPasswordSender = $xvSMTPSender;
$xvUseEmailConfirmed = false;
$xvRequireEmailConfirmedToEdit = false;

// Scribunto
$wgScribuntoDefaultEngine = 'luasandbox';
$wgScribuntoEngineConf['luastandalone']['luaPath'] = '/usr/bin/lua';
$wgScribuntoEngineConf['luasandbox']['memoryLimit'] = 50 * 1024 * 1024;
$wgScribuntoEngineConf['luasandbox']['cpuLimit'] = 10;

// TextExtracts
$wgExtractsExtendOpenSearchXml = true;

// Lakeus
$wgLakeusShowStickyTOC = true;

// MultimediaViewer
$wgMediaViewerEnableByDefault = true;
$wgMediaViewerEnableByDefaultForAnonymous = true;

// JsonConfig
$wgJsonConfigEnableLuaSupport = true;

// Description2
$wgEnableMetaDescriptionFunctions = true;

// TwoColConflict
$wgTwoColConflictBetaFeature = true;

// Uploads
$wgEnableUploads = true;
$wgAllowCopyUploads = true;
$wgCopyUploadsFromSpecialUpload = true;
xvGrantPermission('upload_by_url', ['sysop', 'staff', 'autoconfirmed']);
$wgUploadDirectory = "/var/lib/mediawiki/images/$xvWikiID";
$xvUseSelfHostedImages = false;
$wgHashedUploadDirectory = true;
$wgSVGConverter = 'ImageMagick';
$wgFileExtensions = array_merge($wgFileExtensions, [
	'svg',
	'ogg',
	'ico',
]);
$wgThumbLimits = [120, 250, 300];
$wgDefaultUserOptions['thumbsize'] = 1;

// Echo
$wgNotificationSender = $xvSMTPSender;
$wgEchoUseJobQueue = true;
$wgEchoEmailFooterAddress = "Xens Wikis by Xensor V Project (https://xvnet.eu.org)";

// Centralization
$xvCentralized = true;
$xvCentralWiki = 'meta';
$xvUseGlobalBlocking = true;
$xvUseGlobalUserrights = true;
$xvUseGlobalCssJs = true;
$xvUseGlobalUserPage = true;
$xvUseGlobalPreferences = true;

// Extensions
$xvUseCargo = false;
$xvUseLockdown = false;
$xvUseContactPage = false;

// SecurePoll
$xvUseSecurePoll = false;
$wgSecurePollUseLogging = true;
$wgSecurePollUseNamespace = true;
$wgSecurePollSingleTransferableVoteEnabled = true;

// Math
$wgMathValidModes = ['source', 'mathml', 'native', 'mathjax'];
$wgDefaultUserOptions['math'] = 'mathjax';
$wgMathEnableFormulaLinks = true;

// TabberNeue
$xvUseTabberNeue = false;
