<?php

$cfg = require __DIR__ . '/../vendor/mediawiki/mediawiki-phan-config/src/config.php';

$cfg['suppress_issue_types'] = array_merge( $cfg['suppress_issue_types'], [
	// getDescription, getTargets, getValidTargets declared in MassMessageListContent
	'PhanUndeclaredMethod',
] );

return $cfg;
