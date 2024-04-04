<?php

namespace MediaWiki\CheckUser\Logging;

use Linker;
use LogFormatter;
use MediaWiki\MediaWikiServices;
use Message;

class TemporaryAccountLogFormatter extends LogFormatter {
	/**
	 * @inheritDoc
	 */
	protected function getMessageParameters() {
		$params = parent::getMessageParameters();

		// Update the logline depending on if the user had their access enabled or disabled
		if ( $this->entry->getSubtype() === 'change-access' ) {
			// Message keys used:
			// - 'checkuser-temporary-account-change-access-level-enable'
			// - 'checkuser-temporary-account-change-access-level-disable'
			$params[3] = $this->msg( 'checkuser-temporary-account-change-access-level-' . $params[3], $params[1] );
		} elseif ( $this->entry->getSubtype() === 'view-ips' ) {
			// Replace temporary user page link with contributions page link.
			// Don't use LogFormatter::makeUserLink, because that adds tools links.
			$tempUserName = $this->entry->getTarget()->getText();
			$userFactory = MediaWikiServices::getInstance()->getUserFactory();
			$params[2] = Message::rawParam(
				Linker::userLink( 0, $userFactory->newUnsavedTempUser( $tempUserName ) )
			);
		}

		return $params;
	}
}
